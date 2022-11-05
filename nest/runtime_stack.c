#include "runtime_stack.h"

Nst_ValueStack *nst_new_val_stack()
{
    Nst_ValueStack *v_stack = (Nst_ValueStack *)malloc(sizeof(Nst_ValueStack));
    Nst_Obj **objs = (Nst_Obj **)malloc(32 * sizeof(Nst_Obj *));
    if ( v_stack == NULL || objs == NULL )
        return NULL;

    v_stack->stack = objs;
    v_stack->current_size = 0;
    v_stack->max_size = 32;

    return v_stack;
}

bool _nst_push_val(Nst_ValueStack *v_stack, Nst_Obj *obj)
{
    size_t max_size = v_stack->max_size;
    if ( v_stack->current_size == max_size )
    {
        Nst_Obj **new_objs = (Nst_Obj **)realloc(
            v_stack->stack,
            max_size * 2 * sizeof(Nst_Obj *)
        );
        if ( new_objs == NULL )
            return false;

        v_stack->stack = new_objs;
        v_stack->max_size = max_size * 2;
    }

    v_stack->stack[v_stack->current_size++] = obj != NULL ? nst_inc_ref(obj) : NULL;
    return true;
}

Nst_Obj *nst_pop_val(Nst_ValueStack *v_stack)
{
    if ( v_stack->current_size == 0 )
        return NULL;

    Nst_Obj *val = v_stack->stack[--v_stack->current_size];
    return val;
}


Nst_Obj *nst_peek_val(Nst_ValueStack *v_stack)
{
    if ( v_stack->current_size == 0 )
        return NULL;

    return v_stack->stack[v_stack->current_size - 1];
}

bool nst_dup_val(Nst_ValueStack *v_stack)
{
    if ( v_stack->current_size != 0 )
        return nst_push_val(v_stack, nst_peek_val(v_stack));
    return true;
}

void nst_destroy_v_stack(Nst_ValueStack *v_stack)
{
    for ( Nst_Int i = 0; i < (Nst_Int)v_stack->current_size; i++ )
    {
        if ( v_stack->stack[i] != NULL )
            nst_dec_ref(v_stack->stack[i]);
    }

    free(v_stack);
}

Nst_CallStack *nst_new_call_stack()
{
    Nst_CallStack *f_stack = (Nst_CallStack *)malloc(sizeof(Nst_CallStack));
    Nst_FuncCall *calls = (Nst_FuncCall *)malloc(125 * sizeof(Nst_FuncCall));
    if ( f_stack == NULL || calls == NULL )
        return NULL;

    f_stack->stack = calls;
    f_stack->current_size = 0;
    f_stack->max_size = 125;

    return f_stack;
}

bool _nst_push_func(Nst_CallStack *f_stack,
                    Nst_FuncObj *func,
                    Nst_Pos call_start,
                    Nst_Pos call_end,
                    Nst_VarTable *vt,
                    Nst_Int idx)
{
    size_t max_size = f_stack->max_size;

    if ( f_stack->current_size == max_size )
    {
        if ( max_size == 1000 )
            return false;

        Nst_FuncCall *new_calls = (Nst_FuncCall *)realloc(
            f_stack->stack,
            max_size * 2 * sizeof(Nst_FuncCall)
        );
        if ( new_calls == NULL )
            return false;

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

Nst_FuncCall nst_pop_func(Nst_CallStack *f_stack)
{
    if ( f_stack->current_size == 0 )
    {
        Nst_FuncCall call = {
            NULL,
            nst_no_pos(),
            nst_no_pos(),
        };
        return call;
    }
    return f_stack->stack[--f_stack->current_size];
}

Nst_FuncCall nst_peek_func(Nst_CallStack *f_stack)
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

void nst_destroy_f_stack(Nst_CallStack *f_stack)
{
    for ( Nst_Int i = 0; i < (Nst_Int)f_stack->current_size; i++ )
    {
        if ( f_stack->stack[i].func != NULL )
            nst_dec_ref(f_stack->stack[i].func);

        if ( f_stack->stack[i].vt != NULL )
        {
            nst_dec_ref(f_stack->stack[i].vt->vars);
            free(f_stack->stack[i].vt);
        }
    }

    free(f_stack);
}
