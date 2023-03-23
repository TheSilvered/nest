#include <assert.h>
#include "mem.h"
#include "runtime_stack.h"

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

Nst_GenericStack *nst_stack_new(usize unit_size, usize starting_size, Nst_OpErr *err)
{
    Nst_GenericStack *g_stack =
        (Nst_GenericStack *)nst_malloc(1, sizeof(Nst_GenericStack), err);
    void *stack = nst_malloc(starting_size, unit_size, err);
    if ( g_stack == NULL || stack == NULL )
    {
        if ( g_stack ) nst_free(g_stack);
        if ( stack ) nst_free(stack);
        return NULL;
    }

    g_stack->stack = stack;
    g_stack->current_size = 0;
    g_stack->max_size = starting_size;

    return g_stack;
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

Nst_ValueStack *nst_vstack_new(Nst_OpErr *err)
{
    return (Nst_ValueStack *)nst_stack_new(sizeof(Nst_Obj *), V_STACK_MIN_SIZE, err);
}

bool _nst_vstack_push(Nst_ValueStack *v_stack, Nst_Obj *obj)
{
    if ( !nst_stack_expand((Nst_GenericStack *)v_stack, sizeof(Nst_Obj *)) )
    {
        return false;
    }

    v_stack->stack[v_stack->current_size++] =
        obj != NULL ? nst_inc_ref(obj) : NULL;
    return true;
}

Nst_Obj *nst_vstack_pop(Nst_ValueStack *v_stack)
{
    if ( v_stack->current_size == 0 )
    {
        return NULL;
    }

    Nst_Obj *val = v_stack->stack[--v_stack->current_size];
    nst_stack_shrink((Nst_GenericStack *)v_stack, V_STACK_MIN_SIZE, sizeof(Nst_Obj *));

    return val;
}

Nst_Obj *nst_vstack_peek(Nst_ValueStack *v_stack)
{
    if ( v_stack->current_size == 0 )
    {
        return NULL;
    }

    return v_stack->stack[v_stack->current_size - 1];
}

bool nst_vstack_dup(Nst_ValueStack *v_stack)
{
    if ( v_stack->current_size != 0 )
    {
        return nst_vstack_push(v_stack, nst_vstack_peek(v_stack));
    }
    return true;
}

void nst_vstack_destroy(Nst_ValueStack *v_stack)
{
    for ( Nst_Int i = 0; i < (Nst_Int)v_stack->current_size; i++ )
    {
        if ( v_stack->stack[i] != NULL )
        {
            nst_dec_ref(v_stack->stack[i]);
        }
    }

    nst_free(v_stack->stack);
    nst_free(v_stack);
}

Nst_CallStack *nst_fstack_new(Nst_OpErr *err)
{
    return (Nst_CallStack *)nst_stack_new(sizeof(Nst_FuncCall), F_STACK_MIN_SIZE, err);
}

bool _nst_fstack_push(Nst_CallStack *f_stack,
                      Nst_FuncObj   *func,
                      Nst_Pos        call_start,
                      Nst_Pos        call_end,
                      Nst_VarTable  *vt,
                      Nst_Int        idx)
{
    usize max_size = f_stack->max_size;

    if ( f_stack->current_size == max_size && max_size == 1000 )
    {
        return false;
    }

    if ( !nst_stack_expand((Nst_GenericStack *)f_stack, sizeof(Nst_FuncCall)) )
    {
        return false;
    }

    f_stack->stack[f_stack->current_size].func = FUNC(nst_inc_ref(func));
    f_stack->stack[f_stack->current_size].start = call_start;
    f_stack->stack[f_stack->current_size].end = call_end;
    f_stack->stack[f_stack->current_size].vt = vt;
    f_stack->stack[f_stack->current_size++].idx = idx;
    return true;
}

Nst_FuncCall nst_fstack_pop(Nst_CallStack *f_stack)
{
    Nst_FuncCall call = {
        NULL,
        nst_no_pos(),
        nst_no_pos(),
    };

    if ( f_stack->current_size == 0 )
    {
        return call;
    }

    call = f_stack->stack[--f_stack->current_size];
    nst_stack_shrink(
        (Nst_GenericStack *)f_stack,
        F_STACK_MIN_SIZE,
        sizeof(Nst_FuncCall));
    return call;
}

Nst_FuncCall nst_fstack_peek(Nst_CallStack *f_stack)
{
    if ( f_stack->current_size == 0 )
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

    return f_stack->stack[f_stack->current_size - 1];
}

void nst_fstack_destroy(Nst_CallStack *f_stack)
{
    for ( Nst_Int i = 0; i < (Nst_Int)f_stack->current_size; i++ )
    {
        if ( f_stack->stack[i].func != NULL )
        {
            nst_dec_ref(f_stack->stack[i].func);
        }

        if ( f_stack->stack[i].vt != NULL )
        {
            nst_dec_ref(f_stack->stack[i].vt->vars);
            nst_free(f_stack->stack[i].vt);
        }
    }

    nst_free(f_stack->stack);
    nst_free(f_stack);
}

Nst_CatchStack *nst_cstack_new(Nst_OpErr *err)
{
    return (Nst_CatchStack *)nst_stack_new(sizeof(Nst_CatchFrame), C_STACK_MIN_SIZE, err);
}

bool nst_cstack_push(Nst_CatchStack *c_stack,
                     Nst_Int         inst_idx,
                     usize           v_stack_size,
                     usize           f_stack_size)
{
    if ( !nst_stack_expand((Nst_GenericStack *)c_stack, sizeof(Nst_CatchFrame)) )
    {
        return false;
    }

    c_stack->stack[c_stack->current_size].f_stack_size = f_stack_size;
    c_stack->stack[c_stack->current_size].v_stack_size = v_stack_size;
    c_stack->stack[c_stack->current_size++].inst_idx   = inst_idx;
    return true;
}

Nst_CatchFrame nst_cstack_peek(Nst_CatchStack *c_stack)
{
    if ( c_stack->current_size == 0 )
    {
        Nst_CatchFrame frame = { 0, 0, -1 };
        return frame;
    }

    return c_stack->stack[c_stack->current_size - 1];
}

Nst_CatchFrame nst_cstack_pop(Nst_CatchStack *c_stack)
{
    Nst_CatchFrame frame = { 0, 0, -1 };
    if ( c_stack->current_size == 0 )
    {
        return frame;
    }

    frame = c_stack->stack[--c_stack->current_size];
    nst_stack_shrink(
        (Nst_GenericStack *)c_stack,
        C_STACK_MIN_SIZE,
        sizeof(Nst_CatchFrame));
    return frame;
}

void nst_cstack_destroy(Nst_CatchStack *c_stack)
{
    nst_free(c_stack->stack);
    nst_free(c_stack);
}
