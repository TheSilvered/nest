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
static CoroutineCallStack *co_c_stack;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(create_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(call_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(pause_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_state_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(generator_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_co_type_obj_, 0);

#if __LINE__ - FUNC_COUNT != 21
#error
#endif

    t_Coroutine = nst_type_new("Coroutine", 9);

    state_suspended = nst_int_new(FLAG_CO_SUSPENDED);
    state_running   = nst_int_new(FLAG_CO_RUNNING);
    state_paused    = nst_int_new(FLAG_CO_PAUSED);
    state_ended     = nst_int_new(FLAG_CO_ENDED);
    co_c_stack = (CoroutineCallStack *)nst_stack_new(sizeof(CoroutineObj *), 8);

    lib_init_ = true;
    return true;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    if ( !lib_init_ )
    {
        return;
    }

    nst_dec_ref(t_Coroutine);
    nst_dec_ref(state_suspended);
    nst_dec_ref(state_running);
    nst_dec_ref(state_paused);
    nst_dec_ref(state_ended);

    for ( usize i = 0, n = co_c_stack->current_size; i < n; i++ )
    {
        nst_dec_ref(co_c_stack->stack[i]);
    }
    free(co_c_stack->stack);
    free(co_c_stack);
}

static void co_c_stack_push(CoroutineObj *co)
{
    nst_stack_expand((Nst_GenericStack *)co_c_stack, sizeof(CoroutineObj *));
    co_c_stack->stack[co_c_stack->current_size] = co;
    co_c_stack->current_size++;
    nst_inc_ref(co);
}

static void co_c_stack_pop()
{
    nst_dec_ref(co_c_stack->stack[co_c_stack->current_size - 1]);
    co_c_stack->current_size--;
    nst_stack_shrink((Nst_GenericStack *)co_c_stack, 8, sizeof(CoroutineObj *));
}

static CoroutineObj *co_c_stack_peek()
{
    if ( co_c_stack->current_size == 0 )
    {
        return nullptr;
    }
    return co_c_stack->stack[co_c_stack->current_size - 1];
}

static NST_FUNC_SIGN(generator_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    CoroutineObj *co = (CoroutineObj *)(objs[1]);

    if ( NST_FLAG_HAS(co, FLAG_CO_PAUSED) )
    {
        // Reset the state of the coroutine to start from the beginning
        for ( usize i = 0, n = co->stack_size; i < n; i++ )
        {
            nst_dec_ref(co->stack[i]);
        }

        nst_free(co->stack);
        nst_dec_ref(co->vars);
        nst_dec_ref(co->globals);

        NST_FLAG_DEL(co, FLAG_CO_PAUSED);
        NST_FLAG_SET(co, FLAG_CO_SUSPENDED);
    }

    Nst_Obj *obj = call_(2, objs, err);

    if ( obj == nullptr )
    {
        return nullptr;
    }

    if ( NST_FLAG_HAS(co, FLAG_CO_ENDED) )
    {
        nst_seq_set(args[0], 3, nst_true());
    }
    else
    {
        nst_seq_set(args[0], 2, obj);
    }
    nst_dec_ref(obj);

    NST_RETURN_NULL;
}

static NST_FUNC_SIGN(generator_is_done)
{
    return nst_inc_ref(SEQ(args[0])->objs[3]);
}

static NST_FUNC_SIGN(generator_get_val)
{
    Nst_Obj *return_ob = nst_inc_ref(SEQ(args[0])->objs[2]);

    CoroutineObj *co = (CoroutineObj *)(SEQ(args[0])->objs[1]);
    Nst_Obj *obj = call_(2, SEQ(args[0])->objs, err);

    if ( obj == nullptr )
    {
        return nullptr;
    }

    if ( NST_FLAG_HAS(co, FLAG_CO_ENDED) )
    {
        nst_seq_set(args[0], 3, nst_true());
    }
    else
    {
        nst_seq_set(args[0], 2, obj);
    }
    nst_dec_ref(obj);

    return return_ob;
}

Nst_Obj *coroutine_new(Nst_FuncObj *func)
{
    CoroutineObj *co = (CoroutineObj *)nst_obj_alloc(
        sizeof(CoroutineObj),
        t_Coroutine,
        coroutine_destroy);

    co->func = func;
    co->vars = NULL;
    co->globals = NULL;
    co->stack = NULL;
    co->stack_size = 0;
    co->idx = -1;

    NST_FLAG_SET(co, FLAG_CO_SUSPENDED);
    NST_FLAG_SET(func, FLAG_FUNC_IS_CO);

    NST_GGC_OBJ_INIT(co, coroutine_traverse, coroutine_track);

    return OBJ(co);
}

void coroutine_traverse(CoroutineObj *co)
{
    NST_FLAG_SET(co->func, NST_FLAG_GGC_REACHABLE);

    if ( !NST_FLAG_HAS(co, FLAG_CO_PAUSED) )
    {
        return;
    }

    for ( usize i = 0, n = co->stack_size; i < n; i++ )
    {
        if ( co->stack[i] != NULL )
        {
            NST_FLAG_SET(co->stack[i], NST_FLAG_GGC_REACHABLE);
        }
    }

    NST_FLAG_SET(co->vars,    NST_FLAG_GGC_REACHABLE);
    NST_FLAG_SET(co->globals, NST_FLAG_GGC_REACHABLE);
}

void coroutine_track(CoroutineObj *co)
{
    nst_ggc_track_obj(GGC_OBJ(co->func));

    if ( !NST_FLAG_HAS(co, FLAG_CO_PAUSED) )
    {
        return;
    }

    for ( usize i = 0, n = co->stack_size; i < n; i++ )
    {
        if ( co->stack[i] != nullptr &&
             NST_FLAG_HAS(co->stack[i], NST_FLAG_GGC_IS_SUPPORTED))
        {
            nst_ggc_track_obj((Nst_GGCObj*)co->stack[i]);
        }
    }

    nst_ggc_track_obj(GGC_OBJ(co->vars));
    nst_ggc_track_obj(GGC_OBJ(co->globals));
}

void coroutine_destroy(CoroutineObj *co)
{
    nst_dec_ref(co->func);

    if ( !NST_FLAG_HAS(co, FLAG_CO_PAUSED) )
    {
        return;
    }

    if ( co->vars != NULL && !NST_FLAG_HAS(co->vars, NST_FLAG_GGC_DELETED) )
    {
        nst_dec_ref(nst_map_drop_str(co->vars, "_vars_"));
        nst_dec_ref(co->vars);
    }

    if ( co->globals != NULL )
    {
        nst_dec_ref(co->globals);
    }

    for ( usize i = 0, n = co->stack_size; i < n; i++ )
    {
        if ( co->stack[i] != nullptr)
            nst_dec_ref(co->stack[i]);
    }

    nst_free(co->stack);
}

NST_FUNC_SIGN(create_)
{
    Nst_FuncObj *func;

    NST_DEF_EXTRACT("f", &func);

    if ( NST_FLAG_HAS(func, NST_FLAG_FUNC_IS_C) )
    {
        NST_SET_RAW_TYPE_ERROR("cannot create a coroutine from a C function");
        return nullptr;
    }

    nst_inc_ref(func);
    return coroutine_new(func);
}

NST_FUNC_SIGN(call_)
{
    Nst_Obj *co_args;
    CoroutineObj *co;

    NST_DEF_EXTRACT("?A#", &co_args, t_Coroutine, &co);

    if ( co_args == nst_null() )
    {
        co_args = nst_array_new(0);
    }
    else
    {
        nst_inc_ref(co_args);
    }

    if ( !NST_FLAG_HAS(co, FLAG_CO_PAUSED) &&
         co->func->arg_num != SEQ(co_args)->len )
    {
        NST_SET_CALL_ERROR(nst_format_error(
            _NST_EM_WRONG_ARG_NUM, "usis",
            co->func->arg_num, co->func->arg_num == 1 ? "" : "s",
            SEQ(co_args)->len, SEQ(co_args)->len == 1 ? "was" : "were"));
        nst_dec_ref(co_args);
        return nullptr;
    }

    if ( NST_FLAG_HAS(co, FLAG_CO_RUNNING) )
    {
        NST_SET_RAW_CALL_ERROR("the coroutine is already running");
        nst_dec_ref(co_args);
        return nullptr;
    }

    bool is_paused = NST_FLAG_HAS(co, FLAG_CO_PAUSED);

    NST_FLAG_DEL(co, FLAG_CO_SUSPENDED);
    NST_FLAG_DEL(co, FLAG_CO_PAUSED);
    NST_FLAG_DEL(co, FLAG_CO_ENDED);
    NST_FLAG_SET(co, FLAG_CO_RUNNING);

    Nst_ExecutionState *state = nst_get_state();
    co->call_stack_size = state->f_stack->current_size;
    co_c_stack_push(co);

    Nst_Obj *result = nullptr;
    if ( is_paused )
    {
        nst_vstack_push(state->v_stack, nullptr);
        for ( usize i = 0, n = co->stack_size; i < n; i++ )
        {
            nst_vstack_push(state->v_stack, co->stack[i]);
            nst_dec_ref(co->stack[i]);
        }
        nst_free(co->stack);
        // emulates the return value of co.pause
        nst_vstack_push(state->v_stack, nst_null());
        result = nst_run_func_context(
            co->func,
            co->idx + 1,
            co->vars,
            co->globals);
        nst_dec_ref(co->vars);
        nst_dec_ref(co->globals);
    }
    else
    {
        result = nst_call_func(co->func, SEQ(co_args)->objs, err);
    }

    nst_dec_ref(co_args);
    co_c_stack_pop();

    // If an error occurred
    if ( result == nullptr )
    {
        NST_FLAG_DEL(co, FLAG_CO_SUSPENDED);
        NST_FLAG_DEL(co, FLAG_CO_RUNNING);
        NST_FLAG_DEL(co, FLAG_CO_PAUSED);
        NST_FLAG_SET(co, FLAG_CO_ENDED);
        return nullptr;
    }

    if ( !NST_FLAG_HAS(co, FLAG_CO_PAUSED) )
    {
        NST_FLAG_DEL(co, FLAG_CO_SUSPENDED);
        NST_FLAG_DEL(co, FLAG_CO_RUNNING);
        NST_FLAG_SET(co, FLAG_CO_ENDED);
    }

    return result;
}

NST_FUNC_SIGN(pause_)
{
    Nst_Obj *return_value;

    NST_DEF_EXTRACT("o", &return_value);
    Nst_ExecutionState *state = nst_get_state();
    Nst_FuncCall call = nst_fstack_peek(state->f_stack);
    
    CoroutineObj *co = co_c_stack_peek();
    
    if ( co == nullptr || call.func != co->func )
    {
        NST_SET_RAW_CALL_ERROR(
            "the top function does not match the coroutine");
        return nullptr;
    }

    if ( state->f_stack->current_size - 1 != co->call_stack_size ||
         !NST_FLAG_HAS(co, FLAG_CO_RUNNING) )
    {
        NST_SET_RAW_CALL_ERROR("the function was not called with 'call'");
        return nullptr;
    }

    // Now I'm sure that the function was called with co.call
    call = nst_fstack_pop(state->f_stack);

    co->vars = state->vt->vars;
    co->globals = state->vt->global_table;
    co->idx = state->idx;
    nst_free(state->vt);

    nst_dec_ref(call.func);

    state->vt = call.vt;
    state->idx = call.idx;

    usize stack_size = 0;
    Nst_Obj **v_stack_objs = state->v_stack->stack;
    for ( Nst_Int i = (Nst_Int)state->v_stack->current_size - 1;
          i >= 0;
          i-- )
    {
        if ( v_stack_objs[i] == nullptr )
        {
            break;
        }
        stack_size++;
    }

    co->stack = (Nst_Obj **)nst_malloc(stack_size, sizeof(Nst_Obj *));

    for ( usize i = stack_size; i > 0; i-- )
    {
        co->stack[i - 1] = nst_vstack_pop(state->v_stack);
    }

    nst_vstack_pop(state->v_stack); // remove NULL from the stack

    NST_FLAG_DEL(co, FLAG_CO_SUSPENDED);
    NST_FLAG_DEL(co, FLAG_CO_RUNNING);
    NST_FLAG_DEL(co, FLAG_CO_ENDED);
    NST_FLAG_SET(co, FLAG_CO_PAUSED);

    return nst_inc_ref(return_value);
}

NST_FUNC_SIGN(get_state_)
{
    CoroutineObj *co;

    NST_DEF_EXTRACT("#", t_Coroutine, &co);

    if ( NST_FLAG_HAS(co, FLAG_CO_SUSPENDED))
        return nst_inc_ref(state_suspended);

    if ( NST_FLAG_HAS(co, FLAG_CO_RUNNING))
        return nst_inc_ref(state_running);

    if ( NST_FLAG_HAS(co, FLAG_CO_PAUSED))
        return nst_inc_ref(state_paused);

    return nst_inc_ref(state_ended);
}

NST_FUNC_SIGN(generator_)
{
    CoroutineObj *co;

    NST_DEF_EXTRACT("#", t_Coroutine, &co);

    // layout co_args, co, obj, is_done
    Nst_SeqObj *arr = SEQ(nst_array_new(4));
    arr->objs[0] = nst_array_new(1);
    arr->objs[1] = nst_inc_ref(co);
    arr->objs[2] = nst_inc_ref(nst_null());
    arr->objs[3] = nst_inc_ref(nst_false());

    SEQ(arr->objs[0])->objs[0] = nst_inc_ref(co);

    return nst_iter_new(
        FUNC(nst_func_new_c(1, generator_start)),
        FUNC(nst_func_new_c(1, generator_is_done)),
        FUNC(nst_func_new_c(1, generator_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(_get_co_type_obj_)
{
    return nst_inc_ref(t_Coroutine);
}
