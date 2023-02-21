#include <assert.h>
#include "runtime_stack.h"

#define V_STACK_MIN_SIZE 32
#define F_STACK_MIN_SIZE 125
#define C_STACK_MIN_SIZE 4

typedef struct _GenericStack
{
    void *stack;
    usize current_size;
    usize max_size;
}
GenericStack;

static void shrink_stack(GenericStack *g_stack,
                         usize         min_size,
                         usize         unit_size)
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

    void *new_stack = realloc(
        g_stack->stack, unit_size * (g_stack->max_size >> 1));
    if ( new_stack == NULL )
    {
        return;
    }
    g_stack->max_size >>= 1;
    g_stack->stack = new_stack;
}

Nst_ValueStack *nst_vstack_new()
{
    Nst_ValueStack *v_stack = (Nst_ValueStack *)malloc(sizeof(Nst_ValueStack));
    Nst_Obj **objs = (Nst_Obj **)malloc(V_STACK_MIN_SIZE * sizeof(Nst_Obj *));
    if ( v_stack == NULL || objs == NULL )
    {
        return NULL;
    }

    v_stack->stack = objs;
    v_stack->current_size = 0;
    v_stack->max_size = V_STACK_MIN_SIZE;

    return v_stack;
}

bool _nst_vstack_push(Nst_ValueStack *v_stack, Nst_Obj *obj)
{
    usize max_size = v_stack->max_size;
    if ( v_stack->current_size == max_size )
    {
        Nst_Obj **new_objs = (Nst_Obj **)realloc(
            v_stack->stack,
            max_size * 2 * sizeof(Nst_Obj *));
        if ( new_objs == NULL )
        {
            return false;
        }

        v_stack->stack = new_objs;
        v_stack->max_size = max_size * 2;
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
    shrink_stack((GenericStack *)v_stack, V_STACK_MIN_SIZE, sizeof(Nst_Obj *));

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

    free(v_stack->stack);
    free(v_stack);
}

Nst_CallStack *nst_fstack_new()
{
    Nst_CallStack *f_stack = (Nst_CallStack *)malloc(sizeof(Nst_CallStack));
    Nst_FuncCall *calls = (Nst_FuncCall *)malloc(
        F_STACK_MIN_SIZE * sizeof(Nst_FuncCall));
    if ( f_stack == NULL || calls == NULL )
    {
        return NULL;
    }

    f_stack->stack = calls;
    f_stack->current_size = 0;
    f_stack->max_size = F_STACK_MIN_SIZE;

    return f_stack;
}

bool _nst_fstack_push(Nst_CallStack *f_stack,
                      Nst_FuncObj   *func,
                      Nst_Pos        call_start,
                      Nst_Pos        call_end,
                      Nst_VarTable  *vt,
                      Nst_Int        idx)
{
    usize max_size = f_stack->max_size;

    if ( f_stack->current_size == max_size )
    {
        if ( max_size == 1000 )
        {
            return false;
        }

        Nst_FuncCall *new_calls = (Nst_FuncCall *)realloc(
            f_stack->stack,
            max_size * 2 * sizeof(Nst_FuncCall));
        if ( new_calls == NULL )
        {
            return false;
        }

        f_stack->stack = new_calls;
        f_stack->max_size = max_size * 2;
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
    shrink_stack(
        (GenericStack *)f_stack,
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
            free(f_stack->stack[i].vt);
        }
    }

    free(f_stack->stack);
    free(f_stack);
}

Nst_CatchStack *nst_cstack_new()
{
    Nst_CatchStack *c_stack = (Nst_CatchStack *)malloc(sizeof(Nst_CatchStack));
    Nst_CatchFrame *frames = (Nst_CatchFrame *)malloc(
        C_STACK_MIN_SIZE * sizeof(Nst_CatchFrame));
    if ( c_stack == NULL || frames == NULL )
    {
        return NULL;
    }

    c_stack->stack = frames;
    c_stack->current_size = 0;
    c_stack->max_size = C_STACK_MIN_SIZE;

    return c_stack;
}

bool nst_cstack_push(Nst_CatchStack *c_stack,
                    Nst_Int         inst_idx,
                    usize           v_stack_size,
                    usize           f_stack_size)
{
    usize max_size = c_stack->max_size;

    if ( c_stack->current_size == max_size )
    {
        Nst_CatchFrame *new_frames = (Nst_CatchFrame *)realloc(
            c_stack->stack,
            max_size * 2 * sizeof(Nst_CatchFrame));
        if ( new_frames == NULL )
        {
            return false;
        }

        c_stack->stack = new_frames;
        c_stack->max_size = max_size * 2;
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
    shrink_stack(
        (GenericStack *)c_stack,
        C_STACK_MIN_SIZE,
        sizeof(Nst_CatchFrame));
    return frame;
}

void nst_cstack_destroy(Nst_CatchStack *c_stack)
{
    free(c_stack->stack);
    free(c_stack);
}
