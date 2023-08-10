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

    t_Coroutine = Nst_type_new("Coroutine");

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

static Nst_FUNC_SIGN(generator_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    CoroutineObj *co = (CoroutineObj *)(objs[0]);

    if (Nst_FLAG_HAS(co, FLAG_CO_PAUSED)) {
        // Reset the state of the coroutine to start from the beginning
        for (usize i = 0, n = co->stack_size; i < n; i++)
            Nst_dec_ref(co->stack[i]);

        Nst_free(co->stack);
        Nst_dec_ref(co->vars);
        Nst_dec_ref(co->globals);

        Nst_FLAG_DEL(co, FLAG_CO_PAUSED);
        Nst_FLAG_SET(co, FLAG_CO_SUSPENDED);
    }

    Nst_Obj *obj = call_(2, objs);

    if (obj == nullptr)
        return nullptr;

    if (Nst_FLAG_HAS(co, FLAG_CO_ENDED))
        Nst_seq_set(args[0], 3, Nst_true());
    else
        Nst_seq_set(args[0], 2, obj);
    Nst_dec_ref(obj);

    Nst_RETURN_NULL;
}

static Nst_FUNC_SIGN(generator_is_done)
{
    Nst_UNUSED(arg_num);
    return Nst_inc_ref(SEQ(args[0])->objs[3]);
}

static Nst_FUNC_SIGN(generator_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj *return_ob = Nst_inc_ref(SEQ(args[0])->objs[2]);

    CoroutineObj *co = (CoroutineObj *)(SEQ(args[0])->objs[0]);
    Nst_Obj *obj = call_(2, SEQ(args[0])->objs);

    if (obj == nullptr)
        return nullptr;

    if (Nst_FLAG_HAS(co, FLAG_CO_ENDED))
        Nst_seq_set(args[0], 3, Nst_true());
    else
        Nst_seq_set(args[0], 2, obj);
    Nst_dec_ref(obj);

    return return_ob;
}

Nst_Obj *coroutine_new(Nst_FuncObj *func)
{
    CoroutineObj *co = Nst_obj_alloc(
        CoroutineObj,
        t_Coroutine,
        coroutine_destroy);

    if (co == nullptr)
        return nullptr;

    co->func = func;
    co->vars = NULL;
    co->globals = NULL;
    co->stack = NULL;
    co->stack_size = 0;
    co->idx = -1;

    Nst_FLAG_SET(co, FLAG_CO_SUSPENDED);
    Nst_FLAG_SET(func, FLAG_FUNC_IS_CO);

    Nst_GGC_OBJ_INIT(co, coroutine_traverse, coroutine_track);

    return OBJ(co);
}

void coroutine_traverse(CoroutineObj *co)
{
    Nst_FLAG_SET(co->func, Nst_FLAG_GGC_REACHABLE);

    if (!Nst_FLAG_HAS(co, FLAG_CO_PAUSED))
        return;

    for (usize i = 0, n = co->stack_size; i < n; i++) {
        if (co->stack[i] != NULL)
            Nst_FLAG_SET(co->stack[i], Nst_FLAG_GGC_REACHABLE);
    }

    Nst_FLAG_SET(co->vars,    Nst_FLAG_GGC_REACHABLE);
    Nst_FLAG_SET(co->globals, Nst_FLAG_GGC_REACHABLE);
}

void coroutine_track(CoroutineObj *co)
{
    Nst_ggc_track_obj(GGC_OBJ(co->func));

    if (!Nst_FLAG_HAS(co, FLAG_CO_PAUSED))
        return;

    for (usize i = 0, n = co->stack_size; i < n; i++) {
        if (co->stack[i] != nullptr
            && Nst_FLAG_HAS(co->stack[i], Nst_FLAG_GGC_IS_SUPPORTED))
        {
            Nst_ggc_track_obj((Nst_GGCObj*)co->stack[i]);
        }
    }

    Nst_ggc_track_obj(GGC_OBJ(co->vars));
    Nst_ggc_track_obj(GGC_OBJ(co->globals));
}

void coroutine_destroy(CoroutineObj *co)
{
    Nst_dec_ref(co->func);

    if (!Nst_FLAG_HAS(co, FLAG_CO_PAUSED))
        return;

    if (co->vars != NULL && !Nst_FLAG_HAS(co->vars, Nst_FLAG_GGC_DELETED)) {
        Nst_dec_ref(Nst_map_drop_str(co->vars, "_vars_"));
        Nst_dec_ref(co->vars);
    }

    if (co->globals != NULL)
        Nst_dec_ref(co->globals);

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

    if (Nst_FLAG_HAS(func, Nst_FLAG_FUNC_IS_C)) {
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
        co_args = Nst_array_new(0);
    else
        Nst_inc_ref(co_args);

    if (!Nst_FLAG_HAS(co, FLAG_CO_PAUSED)
        && co->func->arg_num != SEQ(co_args)->len)
    {
        Nst_set_call_error(Nst_sprintf(
            _Nst_EM_WRONG_ARG_NUM,
            co->func->arg_num, co->func->arg_num == 1 ? "" : "s",
            i64(SEQ(co_args)->len), SEQ(co_args)->len == 1 ? "was" : "were"));
        Nst_dec_ref(co_args);
        return nullptr;
    }

    if (Nst_FLAG_HAS(co, FLAG_CO_RUNNING)) {
        Nst_set_call_error_c("the coroutine is already running");
        Nst_dec_ref(co_args);
        return nullptr;
    }

    bool is_paused = Nst_FLAG_HAS(co, FLAG_CO_PAUSED);

    Nst_FLAG_DEL(co, FLAG_CO_SUSPENDED);
    Nst_FLAG_DEL(co, FLAG_CO_PAUSED);
    Nst_FLAG_DEL(co, FLAG_CO_ENDED);
    Nst_FLAG_SET(co, FLAG_CO_RUNNING);

    Nst_ExecutionState *state = Nst_get_state();
    co->call_stack_size = state->f_stack.len;
    co_c_stack_push(co);

    Nst_Obj *result = nullptr;
    if (is_paused) {
        Nst_vstack_push(nullptr);
        for (usize i = 0, n = co->stack_size; i < n; i++) {
            Nst_vstack_push(co->stack[i]);
            Nst_dec_ref(co->stack[i]);
        }
        Nst_free(co->stack);
        // emulates the return value of co.pause
        Nst_vstack_push(Nst_null());
        result = Nst_run_func_context(
            co->func,
            co->idx + 1,
            co->vars,
            co->globals);
        Nst_dec_ref(co->vars);
        Nst_dec_ref(co->globals);
    } else
        result = Nst_call_func(co->func, SEQ(co_args)->objs);

    Nst_dec_ref(co_args);
    co_c_stack_pop();

    // If an error occurred
    if (result == nullptr) {
        Nst_FLAG_DEL(co, FLAG_CO_SUSPENDED);
        Nst_FLAG_DEL(co, FLAG_CO_RUNNING);
        Nst_FLAG_DEL(co, FLAG_CO_PAUSED);
        Nst_FLAG_SET(co, FLAG_CO_ENDED);
        return nullptr;
    }

    if (!Nst_FLAG_HAS(co, FLAG_CO_PAUSED)) {
        Nst_FLAG_DEL(co, FLAG_CO_SUSPENDED);
        Nst_FLAG_DEL(co, FLAG_CO_RUNNING);
        Nst_FLAG_SET(co, FLAG_CO_ENDED);
    }

    return result;
}

Nst_FUNC_SIGN(pause_)
{
    Nst_Obj *return_value;

    Nst_DEF_EXTRACT("o", &return_value);
    Nst_ExecutionState *state = Nst_get_state();
    Nst_FuncCall call = Nst_fstack_peek();

    CoroutineObj *co = co_c_stack_peek();

    if (co == nullptr || call.func != co->func) {
        Nst_set_call_error_c(
            "the top function does not match the coroutine");
        return nullptr;
    }

    if (state->f_stack.len - 1 != co->call_stack_size
        || !Nst_FLAG_HAS(co, FLAG_CO_RUNNING))
    {
        Nst_set_call_error_c("the function was not called with 'call'");
        return nullptr;
    }

    // Now I'm sure that the function was called with co.call
    call = Nst_fstack_pop();

    co->vars = state->vt->vars;
    co->globals = state->vt->global_table;
    co->idx = state->idx;
    Nst_free(state->vt);

    Nst_dec_ref(call.func);

    state->vt = call.vt;
    state->idx = call.idx;

    usize stack_size = 0;
    Nst_Obj **v_stack_objs = state->v_stack.stack;
    for (i64 i = (i64)state->v_stack.len - 1; i >= 0; i--) {
        if (v_stack_objs[i] == nullptr)
            break;
        stack_size++;
    }

    co->stack = Nst_malloc_c(stack_size, Nst_Obj *);

    for (usize i = stack_size; i > 0; i--)
        co->stack[i - 1] = Nst_vstack_pop();

    Nst_vstack_pop(); // remove NULL from the stack

    Nst_FLAG_DEL(co, FLAG_CO_SUSPENDED);
    Nst_FLAG_DEL(co, FLAG_CO_RUNNING);
    Nst_FLAG_DEL(co, FLAG_CO_ENDED);
    Nst_FLAG_SET(co, FLAG_CO_PAUSED);

    return Nst_inc_ref(return_value);
}

Nst_FUNC_SIGN(get_state_)
{
    CoroutineObj *co;

    Nst_DEF_EXTRACT("#", t_Coroutine, &co);

    if (Nst_FLAG_HAS(co, FLAG_CO_SUSPENDED))
        return Nst_inc_ref(state_suspended);

    if (Nst_FLAG_HAS(co, FLAG_CO_RUNNING))
        return Nst_inc_ref(state_running);

    if (Nst_FLAG_HAS(co, FLAG_CO_PAUSED))
        return Nst_inc_ref(state_paused);

    return Nst_inc_ref(state_ended);
}

Nst_FUNC_SIGN(generator_)
{
    CoroutineObj *co;

    Nst_DEF_EXTRACT("#", t_Coroutine, &co);

    // layout co_args, co, obj, is_done
    Nst_Obj *arr = Nst_array_create_c(
        "Oonb",
        co, Nst_array_create_c("O", co), nullptr, false);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, generator_start)),
        FUNC(Nst_func_new_c(1, generator_is_done)),
        FUNC(Nst_func_new_c(1, generator_get_val)),
        arr);
}

Nst_FUNC_SIGN(_get_co_type_obj_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(t_Coroutine);
}
