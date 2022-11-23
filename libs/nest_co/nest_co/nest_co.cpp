#include "nest_co.h"

#define FUNC_COUNT 5 // Set this to the number of functions in your module

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;
static Nst_TypeObj *t_Coroutine;
static Nst_Obj *state_suspended;
static Nst_Obj *state_running;
static Nst_Obj *state_paused;
static Nst_Obj *state_ended;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(create_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(call_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(pause_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_state_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_co_type_obj_, 0);

    t_Coroutine = nst_new_type_obj("Coroutine", 9);

    state_suspended = nst_new_int(FLAG_CO_SUSPENDED);
    state_running   = nst_new_int(FLAG_CO_RUNNING);
    state_paused    = nst_new_int(FLAG_CO_PAUSED);
    state_ended     = nst_new_int(FLAG_CO_ENDED);

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

void free_lib()
{
    if ( !lib_init_ )
        return;

    nst_dec_ref(t_Coroutine);
    nst_dec_ref(state_suspended);
    nst_dec_ref(state_running);
    nst_dec_ref(state_paused);
    nst_dec_ref(state_ended);
}

Nst_Obj *new_coroutine(Nst_FuncObj *func)
{
    CorutineObj *co = (CorutineObj *)nst_alloc_obj(
        sizeof(CorutineObj),
        t_Coroutine,
        destroy_coroutine
    );

    co->func = func;
    co->vars = NULL;
    co->globals = NULL;
    co->stack = NULL;
    co->stack_size = 0;
    co->idx = -1;
    size_t call_stack_size = 0;

    NST_SET_FLAG(co, FLAG_CO_SUSPENDED);
    NST_SET_FLAG(func, FLAG_FUNC_IS_CO);

    NST_GGC_SUPPORT_INIT(co, traverse_coroutine, track_coroutine);

    return OBJ(co);
}

void traverse_coroutine(CorutineObj *co)
{
    NST_SET_FLAG(co->func, NST_FLAG_GGC_REACHABLE);

    if ( !NST_HAS_FLAG(co, FLAG_CO_PAUSED) )
        return;

    for ( size_t i = 0, n = co->stack_size; i < n; i++ )
    {
        if ( co->stack[i] != NULL )
            NST_SET_FLAG(co->stack[i], NST_FLAG_GGC_REACHABLE);
    }

    NST_SET_FLAG(co->vars,    NST_FLAG_GGC_REACHABLE);
    NST_SET_FLAG(co->globals, NST_FLAG_GGC_REACHABLE);
}

void track_coroutine(CorutineObj *co)
{
    nst_add_tracked_object((Nst_GGCObj *)co->func);

    if ( !NST_HAS_FLAG(co, FLAG_CO_PAUSED) )
        return;

    for ( size_t i = 0, n = co->stack_size; i < n; i++ )
    {
        if ( co->stack[i] != nullptr && NST_HAS_FLAG(co->stack[i], NST_FLAG_GGC_IS_SUPPORTED))
            nst_add_tracked_object((Nst_GGCObj *)co->stack[i]);
    }

    nst_add_tracked_object((Nst_GGCObj *)co->vars);
    nst_add_tracked_object((Nst_GGCObj *)co->globals);
}

void destroy_coroutine(CorutineObj *co)
{
    nst_dec_ref(co->func);

    if ( co->vars != NULL )
    {
        nst_dec_ref(nst_map_drop_str(co->vars, "_vars_"));
        nst_dec_ref(co->vars);
    }

    if ( co->globals != NULL )
        nst_dec_ref(co->globals);

    if ( !NST_HAS_FLAG(co, FLAG_CO_PAUSED) )
        return;

    for ( size_t i = 0, n = co->stack_size; i < n; i++ )
    {
        if ( co->stack[i] != nullptr)
            nst_dec_ref(co->stack[i]);
    }

    delete[] co->stack;
}

NST_FUNC_SIGN(create_)
{
    Nst_FuncObj *func;

    if ( !nst_extract_arg_values("f", arg_num, args, err, &func))
        return nullptr;

    if ( NST_HAS_FLAG(func, NST_FLAG_FUNC_IS_C) )
    {
        NST_SET_RAW_TYPE_ERROR("cannot create a coroutine from a C function");
        return nullptr;
    }

    nst_inc_ref(func);
    return new_coroutine(func);
}

NST_FUNC_SIGN(call_)
{
    Nst_Obj *co_args = args[0];
    CorutineObj *co = (CorutineObj *)args[1];

    if ( co->type != t_Coroutine )
    {
        NST_SET_TYPE_ERROR(_nst_format_error(
            "expected type 'Coroutine' for argument 2, got type '%s' instead",
            "s",
            TYPE_NAME(co)
        ));

        return nullptr;
    }

    if ( !NST_HAS_FLAG(co, FLAG_CO_PAUSED) &&
         co->func->arg_num != SEQ(co_args)->len )
    {
        NST_SET_CALL_ERROR(_nst_format_error(
            "the function of the coroutine expected %zi args but %zi were given",
            "uu",
            co->func->arg_num, SEQ(co_args)->len
        ));

        return nullptr;
    }

    bool is_paused = NST_HAS_FLAG(co, FLAG_CO_PAUSED);

    NST_UNSET_FLAG(co, FLAG_CO_SUSPENDED);
    NST_UNSET_FLAG(co, FLAG_CO_PAUSED);
    NST_UNSET_FLAG(co, FLAG_CO_ENDED);
    NST_SET_FLAG(co, FLAG_CO_RUNNING);
    co->call_stack_size = nst_state.f_stack->current_size;
    
    Nst_Obj *result = nullptr;
    if ( is_paused )
    {
        nst_push_val(nst_state.v_stack, nullptr);
        for ( size_t i = 0, n = co->stack_size; i < n; i++ )
        {
            nst_push_val(nst_state.v_stack, co->stack[i]);
            nst_dec_ref(co->stack[i]);
        }
        delete[] co->stack;
        nst_push_val(nst_state.v_stack, nst_c.null); // emulates the return value of co.pause
        result = nst_run_func_context(co->func, co->idx + 1, co->vars, co->globals);
    }
    else
        result = nst_call_func(co->func, SEQ(co_args)->objs, err);

    // If an error occurred
    if ( result == nullptr )
        return nullptr;
    
    if ( !NST_HAS_FLAG(co, FLAG_CO_PAUSED) )
    {
        NST_UNSET_FLAG(co, FLAG_CO_SUSPENDED);
        NST_UNSET_FLAG(co, FLAG_CO_RUNNING);
        NST_SET_FLAG(co, FLAG_CO_ENDED);
    }

    return result;
}

NST_FUNC_SIGN(pause_)
{
    CorutineObj *co = (CorutineObj *)args[0];
    Nst_Obj *return_value = args[1];

    if ( co->type != t_Coroutine )
    {
        NST_SET_TYPE_ERROR(_nst_format_error(
            "expected type 'Coroutine' for argument 1, got type '%s' instead",
            "s",
            TYPE_NAME(co)
        ));

        return nullptr;
    }

    Nst_FuncCall call = nst_peek_func(nst_state.f_stack);
    if ( call.func != co->func )
    {
        NST_SET_RAW_CALL_ERROR("the top function does not match the coroutine");
        return nullptr;
    }

    if ( nst_state.f_stack->current_size - 1 != co->call_stack_size )
    {
        NST_SET_RAW_CALL_ERROR("the function was not called with 'call'");
        return nullptr;
    }

    // Now I'm sure that the function was called with co.call
    call = nst_pop_func(nst_state.f_stack);

    co->vars = (*nst_state.vt)->vars;
    co->globals = (*nst_state.vt)->global_table;
    co->idx = *nst_state.idx;
    delete *nst_state.vt;

    nst_dec_ref(call.func);

    *nst_state.vt = call.vt;
    *nst_state.idx = call.idx;

    size_t stack_size = 0;
    Nst_Obj **v_stack_objs = nst_state.v_stack->stack;
    for ( Nst_Int i = (Nst_Int)nst_state.v_stack->current_size - 1; i >= 0; i-- )
    {
        if ( v_stack_objs[i] == nullptr )
            break;
        stack_size++;
    }

    co->stack = new Nst_Obj *[stack_size];

    for ( size_t i = stack_size; i > 0; i-- )
        co->stack[i - 1] = nst_pop_val(nst_state.v_stack);

    nst_pop_val(nst_state.v_stack); // remove NULL from the stack

    NST_UNSET_FLAG(co, FLAG_CO_SUSPENDED);
    NST_UNSET_FLAG(co, FLAG_CO_RUNNING);
    NST_UNSET_FLAG(co, FLAG_CO_ENDED);
    NST_SET_FLAG(co, FLAG_CO_PAUSED);

    return nst_inc_ref(return_value);
}

NST_FUNC_SIGN(get_state_)
{
    CorutineObj *co = (CorutineObj *)args[0];

    if ( co->type != t_Coroutine )
    {
        NST_SET_TYPE_ERROR(_nst_format_error(
            "expected type 'Coroutine' for argument 1, got type '%s' instead",
            "s",
            TYPE_NAME(co)
        ));

        return nullptr;
    }

    if ( NST_HAS_FLAG(co, FLAG_CO_SUSPENDED))
        return nst_inc_ref(state_suspended);

    if ( NST_HAS_FLAG(co, FLAG_CO_RUNNING))
        return nst_inc_ref(state_running);

    if ( NST_HAS_FLAG(co, FLAG_CO_PAUSED))
        return nst_inc_ref(state_paused);

    return nst_inc_ref(state_ended);
}

NST_FUNC_SIGN(_get_co_type_obj_)
{
    return nst_inc_ref(t_Coroutine);
}
