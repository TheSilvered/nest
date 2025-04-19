#include <cstdlib>
#include "nest_co.h"

static Nst_Obj *t_Coroutine;
static Nst_Obj *state_suspended;
static Nst_Obj *state_running;
static Nst_Obj *state_paused;
static Nst_Obj *state_ended;
static Nst_ValueStack co_c_stack;

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(create_, 1),
    Nst_FUNCDECLR(call_, 2),
    Nst_FUNCDECLR(yield_, 1),
    Nst_FUNCDECLR(get_state_, 1),
    Nst_FUNCDECLR(generator_, 2),
    Nst_FUNCDECLR(_get_co_type_obj_, 0),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    t_Coroutine = Nst_cont_type_new(
        "Coroutine",
        (Nst_ObjDstr)coroutine_destroy,
        (Nst_ObjTrav)coroutine_traverse);

    state_suspended = Nst_int_new(FLAG_CO_SUSPENDED);
    state_running   = Nst_int_new(FLAG_CO_RUNNING);
    state_paused    = Nst_int_new(FLAG_CO_PAUSED);
    state_ended     = Nst_int_new(FLAG_CO_ENDED);
    Nst_vstack_init(&co_c_stack);

    return Nst_error_occurred() ? nullptr : obj_list_;
}

void lib_quit()
{
    Nst_dec_ref(t_Coroutine);
    Nst_dec_ref(state_suspended);
    Nst_dec_ref(state_running);
    Nst_dec_ref(state_paused);
    Nst_dec_ref(state_ended);

    for (usize i = 0, n = co_c_stack.len; i < n; i++)
        Nst_dec_ref(NstOBJ(co_c_stack.stack[i]));
    Nst_free(co_c_stack.stack);
}

static void co_c_stack_push(CoroutineObj *co)
{
    Nst_vstack_push(&co_c_stack, NstOBJ(co));
}

static void co_c_stack_pop()
{
    Nst_dec_ref(Nst_vstack_pop(&co_c_stack));
}

static CoroutineObj *co_c_stack_peek()
{
    return (CoroutineObj *)Nst_vstack_peek(&co_c_stack);
}

static Nst_Obj *call_coroutine(CoroutineObj *co, usize arg_num, Nst_Obj **args)
{
    if (Nst_HAS_FLAG(co, FLAG_CO_RUNNING)) {
        Nst_error_setc_call("the coroutine is already running");
        return nullptr;
    }

    bool is_paused = Nst_HAS_FLAG(co, FLAG_CO_PAUSED);

    Nst_CLEAR_FLAGS(co);
    Nst_SET_FLAG(co, FLAG_CO_RUNNING);

    const Nst_InterpreterState *state = Nst_state();
    co->call_stack_size = state->f_stack.len;
    co_c_stack_push(co);

    Nst_Obj *result = nullptr;
    if (is_paused) {
        Nst_Obj **stack = co->stack;
        co->stack = NULL;
        Nst_VarTable vt = co->vt;
        co->vt.vars = NULL;
        co->vt.global_table = NULL;
        result = Nst_coroutine_resume(
            co->func,
            co->idx + 1,
            stack, co->stack_size,
            vt);
    } else
        result = Nst_func_call(co->func, (i32)arg_num, args);

    co_c_stack_pop();

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

typedef struct _GeneratorData {
    CoroutineObj *co;
    Nst_Obj *co_args;
} GeneratorData;

static void destroy_generator_data(Nst_Obj *data_obj)
{
    GeneratorData *data = (GeneratorData *)Nst_obj_custom_data(data_obj);
    Nst_dec_ref(NstOBJ(data->co));
    Nst_dec_ref(data->co_args);
}

static Nst_Obj *NstC generator_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    GeneratorData *data = (GeneratorData *)Nst_obj_custom_data(args[0]);
    CoroutineObj *co = data->co;

    if (Nst_HAS_FLAG(co, FLAG_CO_PAUSED)) {
        if (co->stack != NULL) {
            // Reset the state of the coroutine to start from the beginning
            for (usize i = 0, n = co->stack_size; i < n; i++)
                Nst_dec_ref(co->stack[i]);
            Nst_free(co->stack);
        }
        Nst_vt_destroy(&co->vt);

        Nst_DEL_FLAG(co, FLAG_CO_PAUSED);
        Nst_SET_FLAG(co, FLAG_CO_SUSPENDED);
    }

    return Nst_null_ref();
}

static Nst_Obj *NstC generator_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    GeneratorData *data = (GeneratorData *)Nst_obj_custom_data(args[0]);

    CoroutineObj *co = data->co;
    Nst_Obj *co_args = data->co_args;
    Nst_Obj *obj = call_coroutine(
        co,
        Nst_seq_len(co_args),
        _Nst_seq_objs(co_args));

    if (obj == nullptr)
        return nullptr;

    if (Nst_HAS_FLAG(co, FLAG_CO_ENDED)) {
        Nst_dec_ref(obj);
        return Nst_iend_ref();
    }

    if (obj == Nst_iend()) {
        Nst_dec_ref(obj);
        return Nst_null_ref();
    }
    return obj;
}

Nst_Obj *coroutine_new(Nst_Obj *func)
{
    CoroutineObj *co = Nst_obj_alloc(CoroutineObj, t_Coroutine);
    if (co == nullptr)
        return nullptr;

    Nst_GGC_OBJ_INIT(co);

    co->func = func;
    co->vt.vars = NULL;
    co->vt.global_table = NULL;
    co->stack_size = 0;
    co->idx = -1;

    Nst_SET_FLAG(co, FLAG_CO_SUSPENDED);
    Nst_SET_FLAG(func, FLAG_FUNC_IS_CO);

    return NstOBJ(co);
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

    if (co->vt.vars != NULL)
        Nst_ggc_obj_reachable(co->vt.vars);
    if (co->vt.global_table != NULL)
        Nst_ggc_obj_reachable(co->vt.global_table);
}

void coroutine_destroy(CoroutineObj *co)
{
    Nst_dec_ref(co->func);

    if (!Nst_HAS_FLAG(co, FLAG_CO_PAUSED))
        return;

    Nst_vt_destroy(&co->vt);

    for (usize i = 0, n = co->stack_size; i < n; i++) {
        if (co->stack[i] != nullptr)
            Nst_dec_ref(co->stack[i]);
    }

    Nst_free(co->stack);
}

Nst_Obj *NstC create_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *func;

    if (!Nst_extract_args("f", arg_num, args, &func))
        return nullptr;

    if (Nst_FUNC_IS_C(func)) {
        Nst_error_setc_type("cannot create a coroutine from a C function");
        return nullptr;
    }

    Nst_inc_ref(func);
    return coroutine_new(func);
}

Nst_Obj *NstC call_(usize arg_num, Nst_Obj **args)
{
    CoroutineObj *co;
    Nst_Obj *co_args;

    if (!Nst_extract_args("# ?A", arg_num, args, t_Coroutine, &co, &co_args))
        return nullptr;

    if (co_args == Nst_null())
        return call_coroutine(co, 0, nullptr);
    else
        return call_coroutine(co, Nst_seq_len(co_args), _Nst_seq_objs(co_args));
}

Nst_Obj *NstC yield_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *return_value;

    if (!Nst_extract_args("o", arg_num, args, &return_value))
        return nullptr;

    const Nst_InterpreterState *state = Nst_state();
    Nst_FuncCall call = Nst_fstack_peek((Nst_CallStack *)&state->f_stack);
    CoroutineObj *co = co_c_stack_peek();

    if (co == nullptr
        || call.func != co->func
        || state->f_stack.len - 1 != co->call_stack_size
        || !Nst_HAS_FLAG(co, FLAG_CO_RUNNING))
    {
        Nst_error_setc_call("the function was not called with 'call'");
        return nullptr;
    }

    Nst_coroutine_yield(nullptr, &co->stack_size, nullptr, nullptr);
    co->stack = Nst_malloc_c(co->stack_size, Nst_Obj *);
    if (co->stack == nullptr)
        return nullptr;
    Nst_coroutine_yield(co->stack, &co->stack_size, &co->idx, &co->vt);

    Nst_CLEAR_FLAGS(co);
    Nst_SET_FLAG(co, FLAG_CO_PAUSED);

    return Nst_inc_ref(return_value);
}

Nst_Obj *NstC get_state_(usize arg_num, Nst_Obj **args)
{
    CoroutineObj *co;

    if (!Nst_extract_args("#", arg_num, args, t_Coroutine, &co))
        return nullptr;

    if (Nst_HAS_FLAG(co, FLAG_CO_SUSPENDED))
        return Nst_inc_ref(state_suspended);

    if (Nst_HAS_FLAG(co, FLAG_CO_RUNNING))
        return Nst_inc_ref(state_running);

    if (Nst_HAS_FLAG(co, FLAG_CO_PAUSED))
        return Nst_inc_ref(state_paused);

    return Nst_inc_ref(state_ended);
}

Nst_Obj *NstC generator_(usize arg_num, Nst_Obj **args)
{
    CoroutineObj *co;
    Nst_Obj *co_args;

    if (!Nst_extract_args("# ?A", arg_num, args, t_Coroutine, &co, &co_args))
        return nullptr;

    usize co_func_arg_num = Nst_func_arg_num(co->func);
    if (co_args != Nst_null() && Nst_seq_len(co_args) > co_func_arg_num) {
        Nst_error_setf_call(
            "the coroutine expects at most %zi arguments but %zi were given",
            co_func_arg_num,
            Nst_seq_len(co_args));
        return nullptr;
    }

    if (co_args == Nst_null())
        co_args = Nst_array_new(0);
    else
        Nst_inc_ref(co_args);

    if (co_args == nullptr)
        return nullptr;

    GeneratorData data = {
        .co = (CoroutineObj *)Nst_inc_ref(NstOBJ(co)),
        .co_args = Nst_inc_ref(co_args)
    };
    Nst_Obj *co_data = Nst_obj_custom_ex(
        GeneratorData,
        &data,
        destroy_generator_data);
    if (co_data == nullptr) {
        Nst_dec_ref(NstOBJ(co));
        Nst_dec_ref(co_args);
        return nullptr;
    }

    return Nst_iter_new(
        Nst_func_new_c(1, generator_start),
        Nst_func_new_c(1, generator_next),
        co_data);
}

Nst_Obj *NstC _get_co_type_obj_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(t_Coroutine);
}
