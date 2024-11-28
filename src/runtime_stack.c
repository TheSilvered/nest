#include "mem.h"
#include "runtime_stack.h"
#include "interpreter.h"
#include "format.h"

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

    Nst_assert(g_stack->len <= g_stack->cap);

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

bool Nst_vstack_init(Nst_ValueStack *v_stack)
{
    return Nst_stack_init(
        (Nst_GenericStack *)v_stack,
        sizeof(Nst_Obj *),
        V_STACK_MIN_SIZE);
}

bool _Nst_vstack_push(Nst_ValueStack *v_stack, Nst_Obj *obj)
{
    bool result = Nst_stack_expand(
        (Nst_GenericStack *)v_stack,
        sizeof(Nst_Obj *));
    if (!result)
        return false;

    usize idx = v_stack->len++;
    v_stack->stack[idx] = Nst_ninc_ref(obj);
    return true;
}

Nst_Obj *Nst_vstack_pop(Nst_ValueStack *v_stack)
{
    if (v_stack->len == 0)
        return NULL;

    Nst_Obj *val = v_stack->stack[--v_stack->len];
    Nst_stack_shrink(
        (Nst_GenericStack *)v_stack,
        V_STACK_MIN_SIZE,
        sizeof(Nst_Obj *));

    return val;
}

Nst_Obj *Nst_vstack_peek(Nst_ValueStack *v_stack)
{
    if (v_stack->len == 0)
        return NULL;

    return v_stack->stack[v_stack->len - 1];
}

bool Nst_vstack_dup(Nst_ValueStack *v_stack)
{
    if (v_stack->len != 0)
        return Nst_vstack_push(v_stack, Nst_vstack_peek(v_stack));
    return true;
}

void Nst_vstack_destroy(Nst_ValueStack *v_stack)
{
    if (v_stack->stack == NULL)
        return;

    for (usize i = 0, n = v_stack->len; i < n; i++) {
        if (v_stack->stack[i] != NULL)
            Nst_dec_ref(v_stack->stack[i]);
    }

    Nst_free(v_stack->stack);
    v_stack->stack = NULL;
}

bool Nst_fstack_init(Nst_CallStack *f_stack)
{
    f_stack->max_recursion_depth = 1000;
    return Nst_stack_init(
        (Nst_GenericStack *)f_stack,
        sizeof(Nst_FuncCall),
        F_STACK_MIN_SIZE);
}

bool _Nst_fstack_push(Nst_CallStack *f_stack, Nst_FuncCall call)
{
    usize max_size = f_stack->cap;

    if (f_stack->len == max_size && max_size == f_stack->max_recursion_depth) {
        Nst_set_call_errorf(
            _Nst_EM_CALL_STACK_SIZE_EXCEEDED,
            f_stack->max_recursion_depth);
        return false;
    }

    bool result = Nst_stack_expand(
        (Nst_GenericStack *)f_stack,
        sizeof(Nst_FuncCall));
    if (!result)
        return false;

    usize stack_idx = f_stack->len++;
    f_stack->stack[stack_idx] = call;
    Nst_ninc_ref(call.func);

    return true;
}

Nst_FuncCall Nst_fstack_pop(Nst_CallStack *f_stack)
{
    Nst_FuncCall call = {
        NULL,
        NULL,
        Nst_no_pos(),
        Nst_no_pos(),
        NULL,
        0, 0
    };

    if (f_stack->len == 0)
        return call;

    call = f_stack->stack[--f_stack->len];
    Nst_stack_shrink(
        (Nst_GenericStack *)f_stack,
        F_STACK_MIN_SIZE,
        sizeof(Nst_FuncCall));
    return call;
}

Nst_FuncCall Nst_fstack_peek(Nst_CallStack *f_stack)
{
    if (f_stack->len == 0) {
        Nst_FuncCall ret_val = {
            NULL,
            NULL,
            Nst_no_pos(),
            Nst_no_pos(),
            NULL,
            0, 0
        };
        return ret_val;
    }

    return f_stack->stack[f_stack->len - 1];
}

void Nst_fstack_destroy(Nst_CallStack *f_stack)
{
    if (f_stack->stack == NULL)
        return;

    for (usize i = 0, n = f_stack->len; i < n; i++) {
        Nst_ndec_ref(f_stack->stack[i].func);
        Nst_ndec_ref(f_stack->stack[i].cwd);

        if (f_stack->stack[i].vt != NULL) {
            Nst_dec_ref(f_stack->stack[i].vt->vars);
            Nst_free(f_stack->stack[i].vt);
        }
    }

    Nst_free(f_stack->stack);
    f_stack->stack = NULL;
}

bool Nst_cstack_init(Nst_CatchStack *c_stack)
{
    return Nst_stack_init(
        (Nst_GenericStack *)c_stack,
        sizeof(Nst_CatchFrame),
        C_STACK_MIN_SIZE);
}

bool Nst_cstack_push(Nst_CatchStack *c_stack, Nst_CatchFrame frame)
{
    bool result = Nst_stack_expand(
        (Nst_GenericStack *)c_stack,
        sizeof(Nst_CatchFrame));
    if (!result)
        return false;

    usize idx = c_stack->len++;
    c_stack->stack[idx] = frame;
    return true;
}

Nst_CatchFrame Nst_cstack_peek(Nst_CatchStack *c_stack)
{
    if (c_stack->len == 0) {
        Nst_CatchFrame frame = { 0, 0, -1 };
        return frame;
    }

    return c_stack->stack[c_stack->len - 1];
}

Nst_CatchFrame Nst_cstack_pop(Nst_CatchStack *c_stack)
{
    Nst_CatchFrame frame = { 0, 0, -1 };
    if (c_stack->len == 0)
        return frame;

    frame = c_stack->stack[--c_stack->len];
    Nst_stack_shrink(
        (Nst_GenericStack *)c_stack,
        C_STACK_MIN_SIZE,
        sizeof(Nst_CatchFrame));
    return frame;
}

void Nst_cstack_destroy(Nst_CatchStack *c_stack)
{
    if (c_stack->stack == NULL)
        return;

    Nst_free(c_stack->stack);
    c_stack->stack = NULL;
}
