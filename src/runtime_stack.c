#include <assert.h>
#include "mem.h"
#include "runtime_stack.h"
#include "interpreter.h"

#define V_STACK_MIN_SIZE 32
#define F_STACK_MIN_SIZE 125
#define C_STACK_MIN_SIZE 4

void Nst_stack_shrink(Nst_GenericStack *g_stack, usize min_size,
                      usize unit_size)
{
    if (g_stack->cap <= min_size)
        return;

    if (g_stack->cap >> 2 < g_stack->len)
        return;

    assert(g_stack->len <= g_stack->cap);

    void *new_stack = Nst_realloc(
        g_stack->stack,
        (g_stack->cap >> 1),
        unit_size,
        g_stack->cap);

    g_stack->cap >>= 1;
    g_stack->stack = new_stack;
}

bool Nst_stack_init(Nst_GenericStack *g_stack, usize unit_size,
                    usize starting_size)
{
    void *stack = Nst_malloc(starting_size, unit_size);
    if (stack == NULL) {
        g_stack->stack = NULL;
        g_stack->len = 0;
        g_stack->cap = 0;
        return false;
    }

    g_stack->stack = stack;
    g_stack->len = 0;
    g_stack->cap = starting_size;

    return true;
}

bool Nst_stack_expand(Nst_GenericStack *g_stack, usize unit_size)
{
    if (g_stack->len < g_stack->cap)
        return true;

    void *new_stack = Nst_realloc(
        g_stack->stack,
        g_stack->cap * 2,
        unit_size,
        0);

    if (new_stack == NULL)
        return false;

    g_stack->stack = new_stack;
    g_stack->cap *= 2;

    return true;
}

bool Nst_vstack_init(void)
{
    return Nst_stack_init(
        (Nst_GenericStack *)&Nst_state.v_stack,
        sizeof(Nst_Obj *),
        V_STACK_MIN_SIZE);
}

bool _Nst_vstack_push(Nst_Obj *obj)
{
    bool result = Nst_stack_expand(
        (Nst_GenericStack *)&Nst_state.v_stack,
        sizeof(Nst_Obj *));
    if (!result)
        return false;

    usize idx = Nst_state.v_stack.len++;
    Nst_state.v_stack.stack[idx] = Nst_ninc_ref(obj);
    return true;
}

Nst_Obj *Nst_vstack_pop(void)
{
    if (Nst_state.v_stack.len == 0)
        return NULL;

    Nst_Obj *val = Nst_state.v_stack.stack[--Nst_state.v_stack.len];
    Nst_stack_shrink(
        (Nst_GenericStack *)&Nst_state.v_stack,
        V_STACK_MIN_SIZE,
        sizeof(Nst_Obj *));

    return val;
}

Nst_Obj *Nst_vstack_peek(void)
{
    if (Nst_state.v_stack.len == 0)
        return NULL;

    return Nst_state.v_stack.stack[Nst_state.v_stack.len - 1];
}

bool Nst_vstack_dup(void)
{
    if (Nst_state.v_stack.len != 0)
        return Nst_vstack_push(Nst_vstack_peek());
    return true;
}

void Nst_vstack_destroy(void)
{
    if (Nst_state.v_stack.stack == NULL)
        return;

    for (i64 i = 0; i < (i64)Nst_state.v_stack.len; i++) {
        if (Nst_state.v_stack.stack[i] != NULL)
            Nst_dec_ref(Nst_state.v_stack.stack[i]);
    }

    Nst_free(Nst_state.v_stack.stack);
}

bool Nst_fstack_init(void)
{
    return Nst_stack_init(
        (Nst_GenericStack *)&Nst_state.f_stack,
        sizeof(Nst_FuncCall),
        F_STACK_MIN_SIZE);
}

bool _Nst_fstack_push(Nst_FuncObj  *func, Nst_Pos call_start, Nst_Pos call_end,
                      Nst_VarTable *vt, i64 idx, usize cstack_size)
{
    usize max_size = Nst_state.f_stack.cap;

    if (Nst_state.f_stack.len == max_size && max_size == 1000)
        return false;

    bool result = Nst_stack_expand(
        (Nst_GenericStack *)&Nst_state.f_stack,
        sizeof(Nst_FuncCall));
    if (!result)
        return false;

    usize stack_idx = Nst_state.f_stack.len++;
    Nst_FuncCall *call = &Nst_state.f_stack.stack[stack_idx];

    call->func = FUNC(Nst_inc_ref(func));
    call->start = call_start;
    call->end = call_end;
    call->vt = vt;
    call->idx = idx;
    call->cstack_len = cstack_size;
    return true;
}

Nst_FuncCall Nst_fstack_pop(void)
{
    Nst_FuncCall call = {
        NULL,
        Nst_no_pos(),
        Nst_no_pos(),
        NULL,
        0, 0
    };

    if (Nst_state.f_stack.len == 0)
        return call;

    call = Nst_state.f_stack.stack[--Nst_state.f_stack.len];
    Nst_stack_shrink(
        (Nst_GenericStack *)&Nst_state.f_stack,
        F_STACK_MIN_SIZE,
        sizeof(Nst_FuncCall));
    return call;
}

Nst_FuncCall Nst_fstack_peek(void)
{
    if (Nst_state.f_stack.len == 0) {
        Nst_FuncCall ret_val = {
            NULL,
            Nst_no_pos(),
            Nst_no_pos(),
            NULL,
            0, 0
        };
        return ret_val;
    }

    return Nst_state.f_stack.stack[Nst_state.f_stack.len - 1];
}

void Nst_fstack_destroy(void)
{
    if (Nst_state.v_stack.stack == NULL)
        return;

    for (i64 i = 0; i < (i64)Nst_state.f_stack.len; i++) {
        if (Nst_state.f_stack.stack[i].func != NULL)
            Nst_dec_ref(Nst_state.f_stack.stack[i].func);

        if (Nst_state.f_stack.stack[i].vt != NULL) {
            Nst_dec_ref(Nst_state.f_stack.stack[i].vt->vars);
            Nst_free(Nst_state.f_stack.stack[i].vt);
        }
    }

    Nst_free(Nst_state.f_stack.stack);
}

bool Nst_cstack_init(void)
{
    return Nst_stack_init(
        (Nst_GenericStack *)&Nst_state.c_stack,
        sizeof(Nst_CatchFrame),
        C_STACK_MIN_SIZE);
}

bool Nst_cstack_push(i64 inst_idx, usize v_stack_size, usize f_stack_size)
{
    bool result = Nst_stack_expand(
        (Nst_GenericStack *)&Nst_state.c_stack,
        sizeof(Nst_CatchFrame));
    if (!result)
        return false;

    usize idx = Nst_state.c_stack.len++;
    Nst_CatchFrame *frame = &Nst_state.c_stack.stack[idx];
    frame->f_stack_len = f_stack_size;
    frame->v_stack_len = v_stack_size;
    frame->inst_idx     = inst_idx;
    return true;
}

Nst_CatchFrame Nst_cstack_peek(void)
{
    if (Nst_state.c_stack.len == 0) {
        Nst_CatchFrame frame = { 0, 0, -1 };
        return frame;
    }

    return Nst_state.c_stack.stack[Nst_state.c_stack.len - 1];
}

Nst_CatchFrame Nst_cstack_pop(void)
{
    Nst_CatchFrame frame = { 0, 0, -1 };
    if (Nst_state.c_stack.len == 0)
        return frame;

    frame = Nst_state.c_stack.stack[--Nst_state.c_stack.len];
    Nst_stack_shrink(
        (Nst_GenericStack *)&Nst_state.c_stack,
        C_STACK_MIN_SIZE,
        sizeof(Nst_CatchFrame));
    return frame;
}

void Nst_cstack_destroy(void)
{
    if (Nst_state.v_stack.stack == NULL)
        return;

    Nst_free(Nst_state.c_stack.stack);
}
