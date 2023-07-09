#include <assert.h>
#include "mem.h"
#include "runtime_stack.h"
#include "interpreter.h"

#define V_STACK_MIN_SIZE 32
#define F_STACK_MIN_SIZE 125
#define C_STACK_MIN_SIZE 4

void Nst_stack_shrink(Nst_GenericStack *g_stack,
                      usize             min_size,
                      usize             unit_size)
{
    if ( g_stack->max_size <= min_size )
    {
        return;
    }

    if ( g_stack->max_size >> 2 < g_stack->current_size )
    {
        return;
    }

    assert(g_stack->current_size <= g_stack->max_size);

    void *new_stack = Nst_realloc(
        g_stack->stack,
        (g_stack->max_size >> 1),
        unit_size,
        g_stack->max_size);

    g_stack->max_size >>= 1;
    g_stack->stack = new_stack;
}

bool Nst_stack_init(Nst_GenericStack *g_stack,
                    usize             unit_size,
                    usize             starting_size)
{
    void *stack = Nst_malloc(starting_size, unit_size);
    if ( stack == NULL )
    {
        g_stack->stack = NULL;
        g_stack->current_size = 0;
        g_stack->max_size = 0;
        return false;
    }

    g_stack->stack = stack;
    g_stack->current_size = 0;
    g_stack->max_size = starting_size;

    return true;
}

bool Nst_stack_expand(Nst_GenericStack *g_stack, usize unit_size)
{
    if ( g_stack->current_size < g_stack->max_size )
    {
        return true;
    }

    void *new_stack = Nst_realloc(
        g_stack->stack,
        g_stack->max_size * 2,
        unit_size,
        0);

    if ( new_stack == NULL )
    {
        return false;
    }

    g_stack->stack = new_stack;
    g_stack->max_size *= 2;

    return true;
}

bool Nst_vstack_init()
{
    return Nst_stack_init(
        (Nst_GenericStack *)&Nst_state.v_stack,
        sizeof(Nst_Obj *),
        V_STACK_MIN_SIZE);
}

bool _Nst_vstack_push(Nst_Obj *obj)
{
    if ( !Nst_stack_expand((Nst_GenericStack *)&Nst_state.v_stack, sizeof(Nst_Obj *)) )
    {
        return false;
    }

    Nst_state.v_stack.stack[Nst_state.v_stack.current_size++] =
        obj != NULL ? Nst_inc_ref(obj) : NULL;
    return true;
}

Nst_Obj *Nst_vstack_pop()
{
    if ( Nst_state.v_stack.current_size == 0 )
    {
        return NULL;
    }

    Nst_Obj *val = Nst_state.v_stack.stack[--Nst_state.v_stack.current_size];
    Nst_stack_shrink(
        (Nst_GenericStack *)&Nst_state.v_stack,
        V_STACK_MIN_SIZE,
        sizeof(Nst_Obj *));

    return val;
}

Nst_Obj *Nst_vstack_peek()
{
    if ( Nst_state.v_stack.current_size == 0 )
    {
        return NULL;
    }

    return Nst_state.v_stack.stack[Nst_state.v_stack.current_size - 1];
}

bool Nst_vstack_dup()
{
    if ( Nst_state.v_stack.current_size != 0 )
    {
        return Nst_vstack_push(Nst_vstack_peek());
    }
    return true;
}

void Nst_vstack_destroy()
{
    if ( Nst_state.v_stack.stack == NULL )
    {
        return;
    }

    for ( Nst_Int i = 0; i < (Nst_Int)Nst_state.v_stack.current_size; i++ )
    {
        if ( Nst_state.v_stack.stack[i] != NULL )
        {
            Nst_dec_ref(Nst_state.v_stack.stack[i]);
        }
    }

    Nst_free(Nst_state.v_stack.stack);
}

bool Nst_fstack_init()
{
    return Nst_stack_init(
        (Nst_GenericStack *)&Nst_state.f_stack,
        sizeof(Nst_FuncCall),
        F_STACK_MIN_SIZE);
}

bool _Nst_fstack_push(Nst_FuncObj  *func,
                      Nst_Pos       call_start,
                      Nst_Pos       call_end,
                      Nst_VarTable *vt,
                      Nst_Int       idx,
                      usize         cstack_size)
{
    usize max_size = Nst_state.f_stack.max_size;

    if ( Nst_state.f_stack.current_size == max_size && max_size == 1000 )
    {
        return false;
    }

    if ( !Nst_stack_expand((Nst_GenericStack *)&Nst_state.f_stack, sizeof(Nst_FuncCall)) )
    {
        return false;
    }

    Nst_FuncCall *call = &Nst_state.f_stack.stack[Nst_state.f_stack.current_size++];

    call->func = FUNC(Nst_inc_ref(func));
    call->start = call_start;
    call->end = call_end;
    call->vt = vt;
    call->idx = idx;
    call->cstack_size = cstack_size;
    return true;
}

Nst_FuncCall Nst_fstack_pop()
{
    Nst_FuncCall call = {
        NULL,
        Nst_no_pos(),
        Nst_no_pos(),
    };

    if ( Nst_state.f_stack.current_size == 0 )
    {
        return call;
    }

    call = Nst_state.f_stack.stack[--Nst_state.f_stack.current_size];
    Nst_stack_shrink(
        (Nst_GenericStack *)&Nst_state.f_stack,
        F_STACK_MIN_SIZE,
        sizeof(Nst_FuncCall));
    return call;
}

Nst_FuncCall Nst_fstack_peek()
{
    if ( Nst_state.f_stack.current_size == 0 )
    {
        Nst_FuncCall ret_val = {
            NULL,
            Nst_no_pos(),
            Nst_no_pos(),
            NULL,
            0
        };
        return ret_val;
    }

    return Nst_state.f_stack.stack[Nst_state.f_stack.current_size - 1];
}

void Nst_fstack_destroy()
{
    if ( Nst_state.v_stack.stack == NULL )
    {
        return;
    }

    for ( Nst_Int i = 0; i < (Nst_Int)Nst_state.f_stack.current_size; i++ )
    {
        if ( Nst_state.f_stack.stack[i].func != NULL )
        {
            Nst_dec_ref(Nst_state.f_stack.stack[i].func);
        }

        if ( Nst_state.f_stack.stack[i].vt != NULL )
        {
            Nst_dec_ref(Nst_state.f_stack.stack[i].vt->vars);
            Nst_free(Nst_state.f_stack.stack[i].vt);
        }
    }

    Nst_free(Nst_state.f_stack.stack);
}

bool Nst_cstack_init()
{
    return Nst_stack_init(
        (Nst_GenericStack *)&Nst_state.c_stack,
        sizeof(Nst_CatchFrame),
        C_STACK_MIN_SIZE);
}

bool Nst_cstack_push(Nst_Int inst_idx,
                     usize   v_stack_size,
                     usize   f_stack_size)
{
    if ( !Nst_stack_expand((Nst_GenericStack *)&Nst_state.c_stack, sizeof(Nst_CatchFrame)) )
    {
        return false;
    }

    Nst_state.c_stack.stack[Nst_state.c_stack.current_size].f_stack_size = f_stack_size;
    Nst_state.c_stack.stack[Nst_state.c_stack.current_size].v_stack_size = v_stack_size;
    Nst_state.c_stack.stack[Nst_state.c_stack.current_size++].inst_idx   = inst_idx;
    return true;
}

Nst_CatchFrame Nst_cstack_peek()
{
    if ( Nst_state.c_stack.current_size == 0 )
    {
        Nst_CatchFrame frame = { 0, 0, -1 };
        return frame;
    }

    return Nst_state.c_stack.stack[Nst_state.c_stack.current_size - 1];
}

Nst_CatchFrame Nst_cstack_pop()
{
    Nst_CatchFrame frame = { 0, 0, -1 };
    if ( Nst_state.c_stack.current_size == 0 )
    {
        return frame;
    }

    frame = Nst_state.c_stack.stack[--Nst_state.c_stack.current_size];
    Nst_stack_shrink(
        (Nst_GenericStack *)&Nst_state.c_stack,
        C_STACK_MIN_SIZE,
        sizeof(Nst_CatchFrame));
    return frame;
}

void Nst_cstack_destroy()
{
    if ( Nst_state.v_stack.stack == NULL )
    {
        return;
    }

    Nst_free(Nst_state.c_stack.stack);
}
