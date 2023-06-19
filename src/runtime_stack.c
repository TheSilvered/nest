#include <assert.h>
#include "mem.h"
#include "runtime_stack.h"
#include "interpreter.h"

#define V_STACK_MIN_SIZE 32
#define F_STACK_MIN_SIZE 125
#define C_STACK_MIN_SIZE 4

void nst_stack_shrink(Nst_GenericStack *g_stack,
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

    void *new_stack = nst_realloc(
        g_stack->stack,
        (g_stack->max_size >> 1),
        unit_size,
        g_stack->max_size, NULL);

    g_stack->max_size >>= 1;
    g_stack->stack = new_stack;
}

bool nst_stack_init(Nst_GenericStack *g_stack,
                    usize             unit_size,
                    usize             starting_size,
                    Nst_OpErr        *err)
{
    void *stack = nst_malloc(starting_size, unit_size, err);
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

bool nst_stack_expand(Nst_GenericStack *g_stack, usize unit_size)
{
    if ( g_stack->current_size < g_stack->max_size )
    {
        return true;
    }

    void *new_stack = nst_realloc(
        g_stack->stack,
        g_stack->max_size * 2,
        unit_size,
        0, NULL);

    if ( new_stack == NULL )
    {
        return false;
    }

    g_stack->stack = new_stack;
    g_stack->max_size *= 2;

    return true;
}

bool nst_vstack_init(Nst_OpErr *err)
{
    return nst_stack_init(
        (Nst_GenericStack *)&nst_state.v_stack,
        sizeof(Nst_Obj *),
        V_STACK_MIN_SIZE, err);
}

bool _nst_vstack_push(Nst_Obj *obj)
{
    if ( !nst_stack_expand((Nst_GenericStack *)&nst_state.v_stack, sizeof(Nst_Obj *)) )
    {
        return false;
    }

    nst_state.v_stack.stack[nst_state.v_stack.current_size++] =
        obj != NULL ? nst_inc_ref(obj) : NULL;
    return true;
}

Nst_Obj *nst_vstack_pop()
{
    if ( nst_state.v_stack.current_size == 0 )
    {
        return NULL;
    }

    Nst_Obj *val = nst_state.v_stack.stack[--nst_state.v_stack.current_size];
    nst_stack_shrink(
        (Nst_GenericStack *)&nst_state.v_stack,
        V_STACK_MIN_SIZE,
        sizeof(Nst_Obj *));

    return val;
}

Nst_Obj *nst_vstack_peek()
{
    if ( nst_state.v_stack.current_size == 0 )
    {
        return NULL;
    }

    return nst_state.v_stack.stack[nst_state.v_stack.current_size - 1];
}

bool nst_vstack_dup()
{
    if ( nst_state.v_stack.current_size != 0 )
    {
        return nst_vstack_push(nst_vstack_peek());
    }
    return true;
}

void nst_vstack_destroy()
{
    for ( Nst_Int i = 0; i < (Nst_Int)nst_state.v_stack.current_size; i++ )
    {
        if ( nst_state.v_stack.stack[i] != NULL )
        {
            nst_dec_ref(nst_state.v_stack.stack[i]);
        }
    }

    nst_free(nst_state.v_stack.stack);
}

bool nst_fstack_init(Nst_OpErr *err)
{
    return nst_stack_init(
        (Nst_GenericStack *)&nst_state.f_stack,
        sizeof(Nst_FuncCall),
        F_STACK_MIN_SIZE, err);
}

bool _nst_fstack_push(Nst_FuncObj  *func,
                      Nst_Pos       call_start,
                      Nst_Pos       call_end,
                      Nst_VarTable *vt,
                      Nst_Int       idx)
{
    usize max_size = nst_state.f_stack.max_size;

    if ( nst_state.f_stack.current_size == max_size && max_size == 1000 )
    {
        return false;
    }

    if ( !nst_stack_expand((Nst_GenericStack *)&nst_state.f_stack, sizeof(Nst_FuncCall)) )
    {
        return false;
    }

    nst_state.f_stack.stack[nst_state.f_stack.current_size].func = FUNC(nst_inc_ref(func));
    nst_state.f_stack.stack[nst_state.f_stack.current_size].start = call_start;
    nst_state.f_stack.stack[nst_state.f_stack.current_size].end = call_end;
    nst_state.f_stack.stack[nst_state.f_stack.current_size].vt = vt;
    nst_state.f_stack.stack[nst_state.f_stack.current_size++].idx = idx;
    return true;
}

Nst_FuncCall nst_fstack_pop()
{
    Nst_FuncCall call = {
        NULL,
        nst_no_pos(),
        nst_no_pos(),
    };

    if ( nst_state.f_stack.current_size == 0 )
    {
        return call;
    }

    call = nst_state.f_stack.stack[--nst_state.f_stack.current_size];
    nst_stack_shrink(
        (Nst_GenericStack *)&nst_state.f_stack,
        F_STACK_MIN_SIZE,
        sizeof(Nst_FuncCall));
    return call;
}

Nst_FuncCall nst_fstack_peek()
{
    if ( nst_state.f_stack.current_size == 0 )
    {
        Nst_FuncCall ret_val = {
            NULL,
            nst_no_pos(),
            nst_no_pos(),
            NULL,
            0
        };
        return ret_val;
    }

    return nst_state.f_stack.stack[nst_state.f_stack.current_size - 1];
}

void nst_fstack_destroy()
{
    for ( Nst_Int i = 0; i < (Nst_Int)nst_state.f_stack.current_size; i++ )
    {
        if ( nst_state.f_stack.stack[i].func != NULL )
        {
            nst_dec_ref(nst_state.f_stack.stack[i].func);
        }

        if ( nst_state.f_stack.stack[i].vt != NULL )
        {
            nst_dec_ref(nst_state.f_stack.stack[i].vt->vars);
            nst_free(nst_state.f_stack.stack[i].vt);
        }
    }

    nst_free(nst_state.f_stack.stack);
}

bool nst_cstack_init(Nst_OpErr *err)
{
    return nst_stack_init(
        (Nst_GenericStack *)&nst_state.c_stack,
        sizeof(Nst_CatchFrame),
        C_STACK_MIN_SIZE, err);
}

bool nst_cstack_push(Nst_Int inst_idx,
                     usize   v_stack_size,
                     usize   f_stack_size)
{
    if ( !nst_stack_expand((Nst_GenericStack *)&nst_state.c_stack, sizeof(Nst_CatchFrame)) )
    {
        return false;
    }

    nst_state.c_stack.stack[nst_state.c_stack.current_size].f_stack_size = f_stack_size;
    nst_state.c_stack.stack[nst_state.c_stack.current_size].v_stack_size = v_stack_size;
    nst_state.c_stack.stack[nst_state.c_stack.current_size++].inst_idx   = inst_idx;
    return true;
}

Nst_CatchFrame nst_cstack_peek()
{
    if ( nst_state.c_stack.current_size == 0 )
    {
        Nst_CatchFrame frame = { 0, 0, -1 };
        return frame;
    }

    return nst_state.c_stack.stack[nst_state.c_stack.current_size - 1];
}

Nst_CatchFrame nst_cstack_pop()
{
    Nst_CatchFrame frame = { 0, 0, -1 };
    if ( nst_state.c_stack.current_size == 0 )
    {
        return frame;
    }

    frame = nst_state.c_stack.stack[--nst_state.c_stack.current_size];
    nst_stack_shrink(
        (Nst_GenericStack *)&nst_state.c_stack,
        C_STACK_MIN_SIZE,
        sizeof(Nst_CatchFrame));
    return frame;
}

void nst_cstack_destroy()
{
    nst_free(nst_state.c_stack.stack);
}
