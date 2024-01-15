#include <cstdlib>
#include "nest_co.h"

#define FUNC_COUNT 6

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_TypeObj *t_Coroutine;
static Nst_Obj *state_suspended;
static Nst_Obj *state_running;
static Nst_Obj *state_paused;
static Nst_Obj *state_ended;
static CoroutineCallStack co_c_stack;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(create_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(call_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(pause_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_state_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(generator_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_get_co_type_obj_, 0);

#if __LINE__ - FUNC_COUNT != 21
#error
#endif

    t_Coroutine = Nst_cont_type_new(
        "Coroutine",
        (Nst_ObjDstr)coroutine_destroy,
        (Nst_ObjTrav)coroutine_traverse);

    state_suspended = Nst_int_new(FLAG_CO_SUSPENDED);
    state_running   = Nst_int_new(FLAG_CO_RUNNING);
    state_paused    = Nst_int_new(FLAG_CO_PAUSED);
    state_ended     = Nst_int_new(FLAG_CO_ENDED);
    Nst_stack_init((Nst_GenericStack *)&co_c_stack, sizeof(CoroutineObj *), 8);

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    Nst_dec_ref(t_Coroutine);
    Nst_dec_ref(state_suspended);
    Nst_dec_ref(state_running);
    Nst_dec_ref(state_paused);
    Nst_dec_ref(state_ended);

    for (usize i = 0, n = co_c_stack.len; i < n; i++)
        Nst_dec_ref(co_c_stack.stack[i]);
    Nst_free(co_c_stack.stack);
}

static void co_c_stack_push(CoroutineObj *co)
{
    Nst_stack_expand((Nst_GenericStack *)&co_c_stack, sizeof(CoroutineObj *));
    co_c_stack.stack[co_c_stack.len] = co;
    co_c_stack.len++;
    Nst_inc_ref(co);
}

static void co_c_stack_pop()
{
    Nst_dec_ref(co_c_stack.stack[co_c_stack.len - 1]);
    co_c_stack.len--;
    Nst_stack_shrink((Nst_GenericStack *)&co_c_stack, 8, sizeof(CoroutineObj *));
}

static CoroutineObj *co_c_stack_peek()
{
    if (co_c_stack.len == 0)
        return nullptr;
    return co_c_stack.stack[co_c_stack.len - 1];
}

static Nst_Obj *call_coroutine(CoroutineObj *co, usize arg_num, Nst_Obj **args)
{
    if (Nst_HAS_FLAG(co, FLAG_CO_RUNNING)) {
        Nst_set_call_error_c("the coroutine is already running");
        return nullptr;
    }

    bool is_paused = Nst_HAS_FLAG(co, FLAG_CO_PAUSED);

    Nst_CLEAR_FLAGS(co);
    Nst_SET_FLAG(co, FLAG_CO_RUNNING);

    Nst_ExecutionState *state = Nst_state_get_es();
    co->call_stack_size = state->f_stack.len;
    co_c_stack_push(co);

    Nst_Obj *result = nullptr;
    if (is_paused) {
        Nst_vstack_push(&state->v_stack, nullptr);
        for (usize i = 0, n = co->stack_size; i < n; i++) {
            Nst_vstack_push(&state->v_stack, co->stack[i]);
            Nst_dec_ref(co->stack[i]);
        }
        Nst_free(co->stack);
        // emulates the return value of co.pause
        Nst_vstack_push(&state->v_stack, Nst_null());
        Nst_VarTable *vt = co->vt;
        co->vt = nullptr;
        result = Nst_run_paused_coroutine(co->func, co->idx + 1, vt);
    } else
        result = Nst_call_func(co->func, (i32)arg_num, args);

    co_c_stack_pop();

    // If an error occurred
    if (result == nullptr) {
        Nst_CLEAR_FLAGS(co);
        Nst_SET_FLAG(co, FLAG_CO_ENDED);
        return nullptr;
    }

    if (!Nst_HAS_FLAG(co, FLAG_CO_PAUSED)) {
        Nst_CLEAR_FLAGS(co);
        Nst_SET_FLAG(co, FLAG_CO_ENDED);
    }

    return result;
}

static Nst_FUNC_SIGN(generator_start)
{
    Nst_UNUSED(arg_num);
    CoroutineObj *co = (CoroutineObj *)(args[0]);

    if (Nst_HAS_FLAG(co, FLAG_CO_PAUSED)) {
        // Reset the state of the coroutine to start from the beginning
        for (usize i = 0, n = co->stack_size; i < n; i++)
            Nst_dec_ref(co->stack[i]);

        Nst_free(co->stack);
        if (co->vt != nullptr)
            Nst_vt_destroy(co->vt);

        Nst_DEL_FLAG(co, FLAG_CO_PAUSED);
        Nst_SET_FLAG(co, FLAG_CO_SUSPENDED);
    }

    Nst_RETURN_NULL;
}

static Nst_FUNC_SIGN(generator_get_val)
{
    Nst_UNUSED(arg_num);

    CoroutineObj *co = (CoroutineObj *)(args[0]);
    Nst_Obj *obj = call_coroutine(co, 1, (Nst_Obj **)&co);

    if (obj == nullptr)
        return nullptr;

    if (Nst_HAS_FLAG(co, FLAG_CO_ENDED)) {
        Nst_dec_ref(obj);
        return Nst_iend_ref();
    }

    if (obj == Nst_iend()) {
        Nst_dec_ref(obj);
        Nst_RETURN_NULL;
    }
    return obj;
}

Nst_Obj *coroutine_new(Nst_FuncObj *func)
{
    CoroutineObj *co = Nst_obj_alloc(CoroutineObj, t_Coroutine);
    if (co == nullptr)
        return nullptr;

    Nst_GGC_OBJ_INIT(co);

    co->func = func;
    co->vt = NULL;
    co->stack_size = 0;
    co->idx = -1;

    Nst_SET_FLAG(co, FLAG_CO_SUSPENDED);
    Nst_SET_FLAG(func, FLAG_FUNC_IS_CO);

    return OBJ(co);
}

void coroutine_traverse(CoroutineObj *co)
{
    Nst_ggc_obj_reachable(co->func);

    if (!Nst_HAS_FLAG(co, FLAG_CO_PAUSED))
        return;

    for (usize i = 0, n = co->stack_size; i < n; i++) {
        if (co->stack[i] != NULL)
            Nst_ggc_obj_reachable(co->stack[i]);
    }

    Nst_ggc_obj_reachable(co->vt->vars);
    Nst_ggc_obj_reachable(co->vt->global_table);
}

void coroutine_destroy(CoroutineObj *co)
{
    Nst_dec_ref(co->func);

    if (!Nst_HAS_FLAG(co, FLAG_CO_PAUSED))
        return;

    if (co->vt != NULL) {
        Nst_vt_destroy(co->vt);
    }

    for (usize i = 0, n = co->stack_size; i < n; i++) {
        if (co->stack[i] != nullptr)
            Nst_dec_ref(co->stack[i]);
    }

    Nst_free(co->stack);
}

Nst_FUNC_SIGN(create_)
{
    Nst_FuncObj *func;

    Nst_DEF_EXTRACT("f", &func);

    if (Nst_HAS_FLAG(func, Nst_FLAG_FUNC_IS_C)) {
        Nst_set_type_error_c("cannot create a coroutine from a C function");
        return nullptr;
    }

    Nst_inc_ref(func);
    return coroutine_new(func);
}

Nst_FUNC_SIGN(call_)
{
    CoroutineObj *co;
    Nst_Obj *co_args;

    Nst_DEF_EXTRACT("# ?A", t_Coroutine, &co, &co_args);

    if (co_args == Nst_null())
        return call_coroutine(co, 0, nullptr);
    else
        return call_coroutine(co, SEQ(co_args)->len, SEQ(co_args)->objs);
}

Nst_FUNC_SIGN(pause_)
{
    Nst_Obj *return_value;

    Nst_DEF_EXTRACT("o", &return_value);
    Nst_ExecutionState *state = Nst_state_get_es();
    Nst_FuncCall call = Nst_fstack_peek(&state->f_stack);
    CoroutineObj *co = co_c_stack_peek();

    if (co == nullptr
        || call.func != co->func
        || state->f_stack.len - 1 != co->call_stack_size
        || !Nst_HAS_FLAG(co, FLAG_CO_RUNNING))
    {
        Nst_set_call_error_c("the function was not called with 'call'");
        return nullptr;
    }

    co->vt = state->vt;
    co->idx = state->idx;
    Nst_es_force_function_end(state);
    // safe because the function finishes before any instructions are executed
    state->vt = NULL;

    usize stack_size = 0;
    Nst_Obj **v_stack_objs = state->v_stack.stack;
    for (i64 i = (i64)state->v_stack.len - 1; i >= 0; i--) {
        if (v_stack_objs[i] == nullptr)
            break;
        stack_size++;
    }

    co->stack = Nst_malloc_c(stack_size, Nst_Obj *);

    for (usize i = stack_size; i > 0; i--)
        co->stack[i - 1] = Nst_vstack_pop(&state->v_stack);

    Nst_CLEAR_FLAGS(co);
    Nst_SET_FLAG(co, FLAG_CO_PAUSED);

    return Nst_inc_ref(return_value);
}

Nst_FUNC_SIGN(get_state_)
{
    CoroutineObj *co;

    Nst_DEF_EXTRACT("#", t_Coroutine, &co);

    if (Nst_HAS_FLAG(co, FLAG_CO_SUSPENDED))
        return Nst_inc_ref(state_suspended);

    if (Nst_HAS_FLAG(co, FLAG_CO_RUNNING))
        return Nst_inc_ref(state_running);

    if (Nst_HAS_FLAG(co, FLAG_CO_PAUSED))
        return Nst_inc_ref(state_paused);

    return Nst_inc_ref(state_ended);
}

Nst_FUNC_SIGN(generator_)
{
    CoroutineObj *co;

    Nst_DEF_EXTRACT("#", t_Coroutine, &co);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, generator_start)),
        FUNC(Nst_func_new_c(1, generator_get_val)),
        Nst_inc_ref(co));
}

Nst_FUNC_SIGN(_get_co_type_obj_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(t_Coroutine);
}
