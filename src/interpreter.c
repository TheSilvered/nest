#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include "nest.h"

#ifdef Nst_MSVC

#include <windows.h>
#include <direct.h>

#ifdef Nst_DBG_KEEP_DYN_LIBS
#define dlclose (void)
#else
#define dlclose FreeLibrary
#endif

#define dlsym GetProcAddress
#define PATH_MAX 4096

typedef HMODULE lib_t;

#else

#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

typedef void *lib_t;

#ifdef Nst_DBG_KEEP_DYN_LIBS
#define dlclose (void)
#endif

#endif // !Nst_MSVC

#include "mem.h"

#define CHECK_V_STACK Nst_es_assert_vstack_size_at_least(Nst_state.es, 1)
#define CHECK_V_STACK_SIZE(size)                                              \
    Nst_es_assert_vstack_size_at_least(Nst_state.es, size)
#define UNTIL_CURRENT_FUNC_FINISHES Nst_state.es->f_stack.len - 1
#define UNTIL_ALL_FUNCS_FINISH 0
#define POP_TOP_VALUE Nst_vstack_pop(&Nst_state.es->v_stack)
#define GET_CURRENT_INST_LS                                                   \
    Nst_func_nest_body(Nst_fstack_peek(&Nst_state.es->f_stack).func)
#define FAST_TOP_VAL                                                          \
    (Nst_state.es->v_stack.stack[Nst_state.es->v_stack.len - 1])

typedef enum _InstResult {
    INST_FAILED = -1,
    INST_SUCCESS = 0,
    INST_NEW_FUNC = 1,
    INST_SUCCESS_ALREADY_ADVANCED = 2
} InstResult;

Nst_IntrState Nst_state;
static bool state_init = false;

static void complete_function(usize final_stack_size);
static bool type_check(Nst_Obj *obj, Nst_Obj *type);

Nst_Inst *inst;
static volatile bool interrupt = false;

static InstResult exe_no_op();
static InstResult exe_pop_val();
static InstResult exe_for_start();
static InstResult exe_jumpif_iend();
static InstResult exe_for_next();
static InstResult exe_return_val();
static InstResult exe_return_vars();
static InstResult exe_set_val_loc();
static InstResult exe_set_cont_loc();
static InstResult exe_jump();
static InstResult exe_jumpif_t();
static InstResult exe_jumpif_f();
static InstResult exe_jumpif_zero();
static InstResult exe_hash_check();
static InstResult exe_set_val();
static InstResult exe_get_val();
static InstResult exe_push_val();
static InstResult exe_set_cont_val();
static InstResult exe_op_call();
static InstResult exe_op_cast();
static InstResult exe_op_range();
static InstResult exe_throw_err();
static InstResult exe_stack_op();
static InstResult exe_local_op();
static InstResult exe_op_import();
static InstResult exe_op_extract();
static InstResult exe_dec_int();
static InstResult exe_new_int();
static InstResult exe_dup();
static InstResult exe_rot();
static InstResult exe_make_arr();
static InstResult exe_make_vec();
static InstResult exe_make_arr_rep();
static InstResult exe_make_vec_rep();
static InstResult exe_make_map();
static InstResult exe_make_func();
static InstResult exe_push_catch();
static InstResult exe_pop_catch();
static InstResult exe_save_error();
static InstResult exe_unpack_seq();

static InstResult call_c_func(bool is_seq_call, usize arg_num,
                              Nst_Obj *args_seq, Nst_Obj *func);
static void loaded_libs_destructor(lib_t lib);
static void source_text_destructor(Nst_SourceText *src);

InstResult (*inst_func[])() = {
    [Nst_IC_NO_OP]        = exe_no_op,
    [Nst_IC_POP_VAL]      = exe_pop_val,
    [Nst_IC_FOR_START]    = exe_for_start,
    [Nst_IC_FOR_NEXT]     = exe_for_next,
    [Nst_IC_RETURN_VAL]   = exe_return_val,
    [Nst_IC_RETURN_VARS]  = exe_return_vars,
    [Nst_IC_SET_VAL_LOC]  = exe_set_val_loc,
    [Nst_IC_SET_CONT_LOC] = exe_set_cont_loc,
    [Nst_IC_JUMP]         = exe_jump,
    [Nst_IC_JUMPIF_T]     = exe_jumpif_t,
    [Nst_IC_JUMPIF_F]     = exe_jumpif_f,
    [Nst_IC_JUMPIF_ZERO]  = exe_jumpif_zero,
    [Nst_IC_JUMPIF_IEND]  = exe_jumpif_iend,
    [Nst_IC_PUSH_CATCH]   = exe_push_catch,
    [Nst_IC_HASH_CHECK]   = exe_hash_check,
    [Nst_IC_THROW_ERR]    = exe_throw_err,
    [Nst_IC_POP_CATCH]    = exe_pop_catch,
    [Nst_IC_SET_VAL]      = exe_set_val,
    [Nst_IC_GET_VAL]      = exe_get_val,
    [Nst_IC_PUSH_VAL]     = exe_push_val,
    [Nst_IC_SET_CONT_VAL] = exe_set_cont_val,
    [Nst_IC_OP_CALL]      = exe_op_call,
    [Nst_IC_OP_CAST]      = exe_op_cast,
    [Nst_IC_OP_RANGE]     = exe_op_range,
    [Nst_IC_STACK_OP]     = exe_stack_op,
    [Nst_IC_LOCAL_OP]     = exe_local_op,
    [Nst_IC_OP_IMPORT]    = exe_op_import,
    [Nst_IC_OP_EXTRACT]   = exe_op_extract,
    [Nst_IC_DEC_INT]      = exe_dec_int,
    [Nst_IC_NEW_INT]      = exe_new_int,
    [Nst_IC_DUP]          = exe_dup,
    [Nst_IC_ROT]          = exe_rot,
    [Nst_IC_MAKE_ARR]     = exe_make_arr,
    [Nst_IC_MAKE_ARR_REP] = exe_make_arr_rep,
    [Nst_IC_MAKE_VEC]     = exe_make_vec,
    [Nst_IC_MAKE_VEC_REP] = exe_make_vec_rep,
    [Nst_IC_MAKE_MAP]     = exe_make_map,
    [Nst_IC_MAKE_FUNC]    = exe_make_func,
    [Nst_IC_SAVE_ERROR]   = exe_save_error,
    [Nst_IC_UNPACK_SEQ]   = exe_unpack_seq
};

Nst_Obj *(*stack_op_func[])(Nst_Obj *, Nst_Obj *) = {
    [Nst_TT_ADD]      = Nst_obj_add,
    [Nst_TT_SUB]      = Nst_obj_sub,
    [Nst_TT_MUL]      = Nst_obj_mul,
    [Nst_TT_DIV]      = Nst_obj_div,
    [Nst_TT_POW]      = Nst_obj_pow,
    [Nst_TT_MOD]      = Nst_obj_mod,
    [Nst_TT_B_AND]    = Nst_obj_bwand,
    [Nst_TT_B_OR]     = Nst_obj_bwor,
    [Nst_TT_B_XOR]    = Nst_obj_bwxor,
    [Nst_TT_LSHIFT]   = Nst_obj_bwls,
    [Nst_TT_RSHIFT]   = Nst_obj_bwrs,
    [Nst_TT_CONCAT]   = Nst_obj_concat,
    [Nst_TT_L_AND]    = Nst_obj_lgand,
    [Nst_TT_L_OR]     = Nst_obj_lgor,
    [Nst_TT_L_XOR]    = Nst_obj_lgxor,
    [Nst_TT_GT]       = Nst_obj_gt,
    [Nst_TT_LT]       = Nst_obj_lt,
    [Nst_TT_EQ]       = Nst_obj_eq,
    [Nst_TT_NEQ]      = Nst_obj_ne,
    [Nst_TT_GTE]      = Nst_obj_ge,
    [Nst_TT_LTE]      = Nst_obj_le,
    [Nst_TT_CONTAINS] = Nst_obj_contains
};

Nst_Obj *(*local_op_func[])(Nst_Obj *) = {
    [Nst_TT_LEN      - Nst_TT_LEN] = Nst_obj_len,
    [Nst_TT_L_NOT    - Nst_TT_LEN] = Nst_obj_lgnot,
    [Nst_TT_B_NOT    - Nst_TT_LEN] = Nst_obj_bwnot,
    [Nst_TT_STDOUT   - Nst_TT_LEN] = Nst_obj_stdout,
    [Nst_TT_STDIN    - Nst_TT_LEN] = Nst_obj_stdin,
    [Nst_TT_IMPORT   - Nst_TT_LEN] = NULL,
    [Nst_TT_LOC_CALL - Nst_TT_LEN] = NULL,
    [Nst_TT_NEG      - Nst_TT_LEN] = Nst_obj_neg,
    [Nst_TT_TYPEOF   - Nst_TT_LEN] = Nst_obj_typeof
};

static void interrupt_handler(int sig)
{
    Nst_UNUSED(sig);
    // if the process is stuck in a loop outside of complete_function this
    // allows the user to send Ctrl-C twice to force exit
    if (interrupt)
        exit(1);
    else {
        interrupt = true;
        Nst_error_set(Nst_inc_ref(Nst_s.e_Interrupt), Nst_null_ref());
    }
}

bool Nst_init(Nst_CLArgs *args)
{
    if (state_init)
        return true;

    Nst_error_set_color(Nst_supports_color());

    Nst_state.es = NULL;
    Nst_tb_init(&Nst_state.global_traceback);

    if (!_Nst_globals_init()) {
        fprintf(stderr, "Memory Error - memory allocation failed...\n");
        return false;
    }

    _Nst_ggc_init();

    if (args != NULL)
        Nst_state.opt_level = args->opt_level;
    else
        Nst_state.opt_level = 3;
    Nst_state.loaded_libs = Nst_llist_new();
    Nst_state.lib_paths = Nst_llist_new();
    Nst_state.lib_srcs = Nst_llist_new();
    Nst_state.lib_handles = Nst_map_new();

    if (Nst_error_occurred())
        goto cleanup;

    state_init = true;

    return true;
cleanup:
    if (Nst_error_occurred())
        Nst_tb_print(&Nst_state.global_traceback);

    Nst_quit();
    return false;
}

void Nst_quit(void)
{
    if (!state_init)
        return;

    state_init = false;

    Nst_tb_destroy(&Nst_state.global_traceback);
    if (Nst_state.lib_paths != NULL) {
        Nst_llist_destroy(
            Nst_state.lib_paths,
            (Nst_Destructor)Nst_dec_ref);
    }
#if !defined(_DEBUG) || !defined(Nst_DBG_TRACK_OBJ_INIT_POS)
    if (Nst_state.lib_srcs != NULL) {
        Nst_llist_destroy(
            Nst_state.lib_srcs,
            (Nst_Destructor)source_text_destructor);
    }
#else
    // in debug mode the libraries are not unloaded to allow for debug checks
    // after quitting libnest
    if (Nst_state.lib_srcs != NULL)
        Nst_llist_destroy(Nst_state.lib_srcs, NULL);
#endif
    // call the destructors for any loaded library
    if (Nst_state.loaded_libs != NULL) {
        for (Nst_LLIST_ITER(lib, Nst_state.loaded_libs)) {
            void (*lib_quit_func)(void) =
                (void (*)(void))dlsym((lib_t)lib->value, "lib_quit");
            if (lib_quit_func != NULL)
                lib_quit_func();
        }
    }

    Nst_dec_ref(Nst_state.lib_handles);
    _Nst_ggc_delete_objs();
    _Nst_globals_quit();

    // the libraries are freed only after the objects because I/O streams might
    // contain objects that are created inside a libraray and are deleted when
    // that library is unloaded
    if (Nst_state.loaded_libs != NULL) {
        Nst_llist_destroy(
            Nst_state.loaded_libs,
            (Nst_Destructor)loaded_libs_destructor);
    }
}

bool Nst_was_init(void)
{
    return state_init;
}

i32 Nst_run(Nst_Obj *main_func)
{
    Nst_assert(main_func->type == Nst_t.Func);

    signal(SIGINT, interrupt_handler);

    // Push the path of the current file on the import stack
    if (Nst_state.es->curr_path != NULL) {
        bool result = Nst_llist_push(
            Nst_state.lib_paths,
            Nst_state.es->curr_path,
            false);
        if (!result)
            return 1;
    }

    // Execute main function
    Nst_FuncCall call = {
        .func = main_func,
        .span = Nst_span_empty(),
        .vt = NULL,
        .cstack_len = 0,
        .idx = 0,
        .cwd = NULL
    };
    Nst_fstack_push(&Nst_state.es->f_stack, call);
    _Nst_func_set_mod_globals(main_func, Nst_state.es->vt->vars);

    complete_function(UNTIL_ALL_FUNCS_FINISH);
    Nst_dec_ref(main_func);

    signal(SIGINT, SIG_DFL);

    // Check for errors
    i32 exit_code = 0;
    if (Nst_error_occurred()) {
        Nst_Traceback *error = Nst_error_get();
        if (error->error_name != Nst_c.Null_null)
            exit_code = 1;
        else {
            exit_code = (i32)Nst_int_i64(error->error_msg);
            Nst_error_clear();
        }
    }

    return exit_code;
}

Nst_ExecutionState *Nst_state_get_es(void)
{
    return Nst_state.es;
}

Nst_ExecutionState *Nst_state_set_es(Nst_ExecutionState *es)
{
    Nst_ExecutionState *prev_es = Nst_state.es;
    Nst_state.es = es;
    if (es != NULL && es->curr_path != NULL)
        Nst_chdir(es->curr_path);
    return prev_es;
}

static inline void destroy_call(Nst_FuncCall *call)
{
    while (Nst_state.es->c_stack.len > call->cstack_len)
        Nst_cstack_pop(&Nst_state.es->c_stack);

    Nst_state.es->idx = call->idx;
    if (Nst_state.es->vt == call->vt)
        return;

    if (Nst_state.es->vt != NULL)
        Nst_vt_destroy(Nst_state.es->vt);
    Nst_dec_ref(call->func);
    Nst_state.es->vt = call->vt;
    if (call->cwd != NULL) {
        Nst_chdir(call->cwd);
        Nst_dec_ref(call->cwd);
    }
}

static inline void set_global_error(usize final_stack_size, Nst_Span span)
{
    Nst_error_add_span(span);

    Nst_CatchFrame top_catch = Nst_cstack_peek(&Nst_state.es->c_stack);
    if (Nst_error_get()->error_name == Nst_c.Null_null
        || Nst_error_get()->error_msg == Nst_c.Null_null)
    {
        top_catch.f_stack_len = 0;
        top_catch.v_stack_len = 0;
        top_catch.inst_idx = -1;
    }

    Nst_Obj *obj;

    usize end_size = top_catch.f_stack_len;
    if (end_size < final_stack_size)
        end_size = final_stack_size;

    while (Nst_state.es->f_stack.len > end_size) {
        Nst_FuncCall call = Nst_fstack_pop(&Nst_state.es->f_stack);
        destroy_call(&call);
        obj = POP_TOP_VALUE;

        while (obj != NULL) {
            Nst_dec_ref(obj);
            obj = POP_TOP_VALUE;
        }

        Nst_error_add_span(call.span);
    }

    if (end_size == final_stack_size)
        return;

    while (Nst_state.es->v_stack.len > top_catch.v_stack_len) {
        obj = POP_TOP_VALUE;
        if (obj != NULL)
            Nst_dec_ref(obj);
    }
    Nst_state.es->idx = top_catch.inst_idx;
}

static void complete_function(usize final_stack_size)
{
    if (Nst_state.es->f_stack.len <= final_stack_size)
        return;

    Nst_InstList *curr_inst_ls = GET_CURRENT_INST_LS;
    Nst_Inst *instructions = curr_inst_ls->instructions;

    while (Nst_state.es->f_stack.len > final_stack_size) {
        i64 idx = Nst_state.es->idx;
        if (idx >= (i64)curr_inst_ls->total_size) {
            // Free the function call
            Nst_FuncCall call = Nst_fstack_pop(&Nst_state.es->f_stack);
            destroy_call(&call);
            if (Nst_state.es->f_stack.len <= final_stack_size)
                return;
            idx = ++Nst_state.es->idx;
            curr_inst_ls = GET_CURRENT_INST_LS;
            instructions = curr_inst_ls->instructions;
        }

        inst = &instructions[idx];
        InstResult result = inst_func[inst->id]();

        if (Nst_state.ggc.gen1.len > _Nst_GEN1_MAX)
            Nst_ggc_collect();

        if (interrupt) {
            interrupt = false;
            result = INST_FAILED;
        }

        if (result == INST_SUCCESS) {
            Nst_state.es->idx++;
            continue;
        } else if (result == INST_FAILED) {
            set_global_error(final_stack_size, instructions[idx].span);
            if (Nst_state.es->f_stack.len <= final_stack_size)
                return;
        }
        curr_inst_ls = GET_CURRENT_INST_LS;
        instructions = curr_inst_ls->instructions;
    }
}

Nst_Inst *Nst_current_inst(void)
{
    if (Nst_state.es == NULL || Nst_state.es->f_stack.len == 0)
        return NULL;

    Nst_InstList *inst_list = GET_CURRENT_INST_LS;
    if (Nst_state.es->idx >= (i64)inst_list->total_size
        || Nst_state.es->idx < 0)
    {
        return NULL;
    }
    return &inst_list->instructions[Nst_state.es->idx];
}

bool Nst_run_module(const char *filename, Nst_SourceText *lib_src)
{
    if (!Nst_es_push_module(Nst_state.es, filename, lib_src))
        return false;

    Nst_chdir(Nst_state.es->curr_path);
    complete_function(UNTIL_CURRENT_FUNC_FINISHES);

    if (Nst_error_occurred())
        return false;
    else
        return true;
}

Nst_Obj *Nst_func_call(Nst_Obj *func, usize arg_num, Nst_Obj **args)
{
    Nst_assert(func->type == Nst_t.Func);
    if (Nst_FUNC_IS_C(func)) {
        usize tot_args = Nst_func_arg_num(func);

        if (tot_args < arg_num) {
            Nst_error_set_call(
                _Nst_WRONG_ARG_NUM(tot_args, arg_num));
            return NULL;
        }

        usize null_args = tot_args - arg_num;

        Nst_Obj **all_args = NULL;
        Nst_Obj *stack_args[10]; // for up to 10 arguments this array is used

        if (tot_args == 0)
            all_args = NULL;
        else if (null_args == 0)
            all_args = args;
        else if (tot_args <= 10) {
            memcpy(stack_args, args, arg_num * sizeof(Nst_Obj *));
            all_args = stack_args;
        } else {
            all_args = Nst_malloc_c(tot_args, Nst_Obj *);
            if (args == NULL)
                return NULL;
            memcpy(all_args, args, arg_num * sizeof(Nst_Obj *));
        }

        for (usize i = 0; all_args && i < null_args; i++)
            all_args[arg_num + i] = Nst_inc_ref(Nst_c.Null_null);

        Nst_Obj *res = Nst_func_c_body(func)(tot_args, all_args);
        if (all_args != stack_args && all_args != args)
            Nst_free(all_args);

        return res;
    }

    bool result = Nst_es_push_func(
        Nst_state.es,
        func,
        Nst_span_empty(),
        arg_num, args);
    if (!result)
        return NULL;

    complete_function(UNTIL_CURRENT_FUNC_FINISHES);

    if (Nst_error_occurred())
        return NULL;

    return POP_TOP_VALUE;
}

Nst_Obj *Nst_run_paused_coroutine(Nst_Obj *func, i64 idx, Nst_VarTable *vt)
{
    Nst_assert(func->type == Nst_t.Func);
    bool result = Nst_es_push_paused_coroutine(
        Nst_state.es,
        func,
        Nst_span_empty(),
        idx,
        vt);

    if (!result)
        return NULL;

    complete_function(UNTIL_CURRENT_FUNC_FINISHES);

    if (Nst_error_occurred())
        return NULL;

    return POP_TOP_VALUE;
}

static bool type_check(Nst_Obj *obj, Nst_Obj *type)
{
    if (obj->type != type) {
        Nst_error_setf_type(
            "expected type '%s', got '%s' instead",
            Nst_type_name(type).value,
            Nst_type_name(obj->type).value);
        return false;
    }
    return true;
}

static InstResult exe_no_op()
{
    return INST_SUCCESS;
}

static InstResult exe_pop_val()
{
    CHECK_V_STACK;
    Nst_Obj *obj = POP_TOP_VALUE;
    Nst_dec_ref(obj);
    return INST_SUCCESS;
}

static InstResult exe_for_inst(Nst_Obj *iter, Nst_Obj *func)
{
    Nst_assert(iter->type == Nst_t.Iter);
    if (Nst_FUNC_IS_C(func)) {
        Nst_Obj *iter_value = Nst_iter_value(iter);
        Nst_Obj *res = Nst_func_c_body(func)(
            (usize)inst->int_val,
            &iter_value);
        if (res == NULL)
            return INST_FAILED;

        Nst_vstack_push(&Nst_state.es->v_stack, res);
        Nst_dec_ref(res);
        return INST_SUCCESS;
    }
    Nst_vstack_push(&Nst_state.es->v_stack, Nst_iter_value(iter));
    Nst_vstack_push(&Nst_state.es->v_stack, func);
    exe_op_call(inst);
    return INST_NEW_FUNC;
}

static InstResult exe_for_start()
{
    CHECK_V_STACK;
    Nst_Obj *iterable = Nst_vstack_peek(&Nst_state.es->v_stack);
    Nst_Obj *iter = Nst_obj_cast(iterable, Nst_t.Iter);
    if (iter == NULL) {
        Nst_error_setf_type(
            "invalid type cast from '%s' to 'Iter'",
            Nst_type_name(iterable->type).value);
        return INST_FAILED;
    }
    Nst_state.es->v_stack.stack[Nst_state.es->v_stack.len - 1] = iter;
    Nst_dec_ref(iterable);
    return exe_for_inst(iter, Nst_iter_start_func(iter));
}

static InstResult exe_for_next()
{
    CHECK_V_STACK;
    Nst_Obj *iter = FAST_TOP_VAL;
    return exe_for_inst(iter, Nst_iter_next_func(iter));
}

static InstResult exe_jumpif_iend()
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *iter_val = FAST_TOP_VAL;
    if (iter_val == Nst_c.IEnd_iend) {
        Nst_dec_ref(POP_TOP_VALUE);
        Nst_state.es->idx = inst->int_val - 1;
    }
    return INST_SUCCESS;
}

static InstResult exe_return_val()
{
    Nst_Obj *result = POP_TOP_VALUE;
    Nst_Obj *obj = POP_TOP_VALUE;

    while (obj != NULL) {
        Nst_dec_ref(obj);
        obj = POP_TOP_VALUE;
    }

    Nst_vstack_push(&Nst_state.es->v_stack, result);
    Nst_es_force_function_end(Nst_state.es);
    Nst_dec_ref(result);
    return INST_NEW_FUNC;
}

static InstResult exe_return_vars()
{
    Nst_Obj *vars = Nst_state.es->vt->vars;
    Nst_Obj *obj = POP_TOP_VALUE;

    while (obj != NULL) {
        Nst_dec_ref(obj);
        obj = POP_TOP_VALUE;
    }

    Nst_vstack_push(&Nst_state.es->v_stack, vars);
    Nst_es_force_function_end(Nst_state.es);
    return INST_NEW_FUNC;
}

static InstResult exe_set_val_loc()
{
    CHECK_V_STACK;
    Nst_Obj *val = POP_TOP_VALUE;
    bool res = Nst_vt_set(Nst_state.es->vt, inst->val, val);
    Nst_dec_ref(val);
    return res ? INST_SUCCESS : INST_FAILED;
}

static InstResult exe_set_cont_loc()
{
    i32 res = exe_set_cont_val(inst);
    if (res == INST_FAILED)
        return INST_FAILED;

    CHECK_V_STACK;
    Nst_dec_ref(POP_TOP_VALUE);
    return res;
}

static InstResult exe_jump()
{
    Nst_state.es->idx = inst->int_val - 1;
    return INST_SUCCESS;
}

static InstResult exe_jumpif_t()
{
    CHECK_V_STACK;
    Nst_Obj *top_val = POP_TOP_VALUE;
    if (Nst_obj_to_bool(top_val))
        Nst_state.es->idx = inst->int_val - 1;
    Nst_dec_ref(top_val);
    return INST_SUCCESS;
}

static InstResult exe_jumpif_f()
{
    CHECK_V_STACK;
    Nst_Obj *top_val = POP_TOP_VALUE;
    if (!Nst_obj_to_bool(top_val))
        Nst_state.es->idx = inst->int_val - 1;

    Nst_dec_ref(top_val);
    return INST_SUCCESS;
}

static InstResult exe_jumpif_zero()
{
    CHECK_V_STACK;
    if (Nst_int_i64(FAST_TOP_VAL) == 0)
        Nst_state.es->idx = inst->int_val - 1;

    return INST_SUCCESS;
}

static InstResult exe_hash_check()
{
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_peek(&Nst_state.es->v_stack);
    Nst_obj_hash(obj);
    if (obj->hash == -1) {
        Nst_error_setf_type(
            "type '%s' is not hashable",
            Nst_type_name(obj->type).value);
        return INST_FAILED;
    }
    return INST_SUCCESS;
}

static InstResult exe_set_val()
{
    CHECK_V_STACK;
    Nst_vt_set(
        Nst_state.es->vt,
        inst->val,
        Nst_vstack_peek(&Nst_state.es->v_stack));
    return Nst_error_occurred() ? INST_FAILED : INST_SUCCESS;
}

static InstResult exe_get_val()
{
    Nst_Obj *obj = Nst_vt_get(Nst_state.es->vt, inst->val);
    if (obj == NULL)
        Nst_vstack_push(&Nst_state.es->v_stack, Nst_c.Null_null);
    else {
        Nst_vstack_push(&Nst_state.es->v_stack, obj);
        Nst_dec_ref(obj);
    }
    return INST_SUCCESS;
}

static InstResult exe_push_val()
{
    bool res = Nst_vstack_push(&Nst_state.es->v_stack, inst->val);
    return res ? INST_SUCCESS : INST_FAILED;
}

static InstResult exe_set_cont_val()
{
    CHECK_V_STACK_SIZE(3);
    Nst_Obj *idx = POP_TOP_VALUE;
    Nst_Obj *cont = POP_TOP_VALUE;
    Nst_Obj *val = Nst_vstack_peek(&Nst_state.es->v_stack);
    i32 return_value = INST_SUCCESS;

    if (cont->type == Nst_t.Array || cont->type == Nst_t.Vector) {
        if (idx->type != Nst_t.Int) {
            Nst_error_setf_type(
                "expected type 'Int', got '%s' instead",
                Nst_type_name(idx->type).value);

            return_value = INST_FAILED;
            goto end;
        }

        if (!Nst_seq_set(cont, Nst_int_i64(idx), val))
            return_value = INST_FAILED;
        goto end;
    } else if (cont->type == Nst_t.Map) {
        if (!Nst_map_set(cont, idx, val))
            return_value = INST_FAILED;
        goto end;
    }

    Nst_error_setf_type(
        "expected type 'Array', 'Vector', or 'Map', got '%s' instead",
        Nst_type_name(cont->type).value);
    return_value = INST_FAILED;

end:
    Nst_dec_ref(cont);
    Nst_dec_ref(idx);
    return return_value;
}

static InstResult call_c_func(bool is_seq_call, usize arg_num,
                              Nst_Obj *args_seq, Nst_Obj *func)
{
    usize tot_args = Nst_func_arg_num(func);

    if (tot_args < arg_num) {
        Nst_error_set_call(
            _Nst_WRONG_ARG_NUM(tot_args, arg_num));
        Nst_dec_ref(func);
        Nst_ndec_ref(args_seq);
        return INST_FAILED;
    }

    usize null_args = tot_args - arg_num;

    Nst_Obj **args;
    Nst_Obj *stack_args[10]; // for up to 10 arguments this array is used

    if (tot_args == 0)
        args = NULL;
    else if (is_seq_call && null_args == 0)
        args = _Nst_seq_objs(args_seq);
    else if (is_seq_call && tot_args <= 10) {
        memcpy(stack_args, _Nst_seq_objs(args_seq), arg_num * sizeof(Nst_Obj *));
        args = stack_args;
    } else if (is_seq_call) {
        args = Nst_malloc_c(tot_args, Nst_Obj *);
        if (args == NULL) {
            Nst_dec_ref(args_seq);
            Nst_dec_ref(func);
            return INST_FAILED;
        }
        memcpy(args, _Nst_seq_objs(args_seq), arg_num * sizeof(Nst_Obj *));
    } else if (tot_args <= 10) {
        for (i64 i = (i64)arg_num - 1; i >= 0; i--)
            stack_args[i] = POP_TOP_VALUE;
        args = stack_args;
    } else {
        args = Nst_malloc_c(tot_args, Nst_Obj *);
        if (args == NULL) {
            Nst_dec_ref(func);
            return INST_FAILED;
        }
        for (i64 i = (i64)arg_num - 1; i >= 0; i--)
            args[i] = POP_TOP_VALUE;
    }

    for (usize i = 0; i < null_args; i++)
        args[arg_num + i] = Nst_inc_ref(Nst_c.Null_null);

    Nst_Obj *res = Nst_func_c_body(func)(tot_args, args);

    if (!is_seq_call) {
        for (usize i = 0; i < tot_args; i++)
            Nst_dec_ref(args[i]);
    } else {
        for (usize i = 0; i < null_args; i++)
            Nst_dec_ref(args[arg_num + i]);
    }

    if (args != stack_args && !is_seq_call)
        Nst_free(args);

    if (res == NULL) {
        Nst_dec_ref(func);
        return INST_FAILED;
    } else {
        Nst_vstack_push(&Nst_state.es->v_stack, res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(func);
    return INST_SUCCESS;
}

static InstResult exe_op_call()
{
#ifdef _DEBUG
    if (inst->int_val == -1)
        CHECK_V_STACK_SIZE(2);
    else
        CHECK_V_STACK_SIZE((u64)inst->int_val + 1);
#endif // !_DEBUG

    i64 arg_num = inst->int_val;
    bool is_seq_call = false;
    Nst_Obj *func = POP_TOP_VALUE;

    if (!type_check(func, Nst_t.Func)) {
        Nst_dec_ref(func);
        return INST_FAILED;
    }

    Nst_Obj *args_seq;

    if (arg_num == -1) {
        args_seq = POP_TOP_VALUE;
        if (args_seq->type != Nst_t.Array && args_seq->type != Nst_t.Vector) {
            Nst_error_setf_type(
                "expected type 'Array' or 'Vector', got '%s' instead",
                Nst_type_name(args_seq->type).value);

            Nst_dec_ref(args_seq);
            Nst_dec_ref(func);
            return INST_FAILED;
        }
        is_seq_call = true;
        arg_num = Nst_seq_len(args_seq);
    } else
        args_seq = NULL;

    if (Nst_FUNC_IS_C(func))
        return call_c_func(is_seq_call, arg_num, args_seq, func);

    bool result = Nst_es_push_func(
        Nst_state.es,
        func,
        inst->span,
        arg_num,
        is_seq_call ? _Nst_seq_objs(args_seq) : NULL);

    Nst_ndec_ref(args_seq);
    return result ? INST_NEW_FUNC : INST_FAILED;
}

static InstResult exe_op_cast()
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *val = POP_TOP_VALUE;
    Nst_Obj *type = POP_TOP_VALUE;

    if (!type_check(type, Nst_t.Type)) {
        Nst_dec_ref(val);
        Nst_dec_ref(type);
        return INST_FAILED;
    }

    Nst_Obj *res = Nst_obj_cast(val, type);

    if (res != NULL) {
        Nst_vstack_push(&Nst_state.es->v_stack, res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(val);
    Nst_dec_ref(type);
    return res == NULL ? INST_FAILED : INST_SUCCESS;
}

static InstResult exe_op_range()
{
    CHECK_V_STACK_SIZE((u64)inst->int_val);
    Nst_Obj *stop = POP_TOP_VALUE;
    Nst_Obj *step = NULL;
    Nst_Obj *start = NULL;

    if (!type_check(stop, Nst_t.Int)) {
        Nst_dec_ref(stop);
        return INST_FAILED;
    }

    if (inst->int_val == 3) {
        start = POP_TOP_VALUE;
        step  = POP_TOP_VALUE;

        if (!type_check(start, Nst_t.Int) || !type_check(step, Nst_t.Int)) {
            Nst_dec_ref(stop);
            Nst_dec_ref(step);
            Nst_dec_ref(start);
            return INST_FAILED;
        }
    } else {
        start = POP_TOP_VALUE;

        if (!type_check(start, Nst_t.Int)) {
            Nst_dec_ref(stop);
            Nst_dec_ref(start);
            return INST_FAILED;
        }

        if (Nst_int_i64(start) <= Nst_int_i64(stop))
            step = Nst_inc_ref(Nst_c.Int_1);
        else
            step = Nst_inc_ref(Nst_c.Int_neg1);
    }

    Nst_Obj *iter = Nst_obj_range(start, stop, step);

    Nst_dec_ref(start);
    Nst_dec_ref(stop);
    Nst_dec_ref(step);

    if (iter == NULL)
        return INST_FAILED;
    Nst_vstack_push(&Nst_state.es->v_stack, iter);
    Nst_dec_ref(iter);
    return INST_SUCCESS;
}

static InstResult exe_throw_err()
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *message = POP_TOP_VALUE;
    Nst_Obj *name = POP_TOP_VALUE;

    Nst_Obj *message_str = Nst_obj_cast(message, Nst_t.Str);
    Nst_Obj *name_str = Nst_obj_cast(name, Nst_t.Str);

    Nst_dec_ref(name);
    Nst_dec_ref(message);

    if (message_str == NULL || name_str == NULL) {
        Nst_ndec_ref(name_str);
        Nst_ndec_ref(message_str);
        return INST_FAILED;
    }

    Nst_error_set(name_str, message_str);
    return INST_FAILED;
}

static InstResult exe_stack_op()
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *ob2 = POP_TOP_VALUE;
    Nst_Obj *ob1 = POP_TOP_VALUE;
    Nst_Obj *res = stack_op_func[inst->int_val](ob1, ob2);

    if (res != NULL) {
        Nst_vstack_push(&Nst_state.es->v_stack, res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(ob1);
    Nst_dec_ref(ob2);
    return res == NULL ? INST_FAILED : INST_SUCCESS;
}

static InstResult exe_local_op()
{
    CHECK_V_STACK;
    Nst_Obj *obj = POP_TOP_VALUE;
    Nst_Obj *res = local_op_func[inst->int_val - Nst_TT_LEN](obj);

    if (res != NULL) {
        Nst_vstack_push(&Nst_state.es->v_stack, res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(obj);
    return res == NULL ? INST_FAILED : INST_SUCCESS;
}

static InstResult exe_op_import()
{
    CHECK_V_STACK;
    Nst_Obj *name = POP_TOP_VALUE;

    if (!type_check(name, Nst_t.Str)) {
        Nst_dec_ref(name);
        return INST_FAILED;
    }

    Nst_Obj *res = Nst_obj_import(name);

    if (res != NULL) {
        Nst_vstack_push(&Nst_state.es->v_stack, res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(name);
    return res == NULL ? INST_FAILED : INST_SUCCESS;
}

static InstResult exe_op_extract()
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *idx = POP_TOP_VALUE;
    Nst_Obj *cont = POP_TOP_VALUE;
    Nst_Obj *res = NULL;
    i32 return_value = INST_SUCCESS;

    if (cont->type == Nst_t.Array || cont->type == Nst_t.Vector) {
        if (idx->type != Nst_t.Int) {
            Nst_error_setf_type(
                "expected type 'Int', got '%s' instead",
                Nst_type_name(idx->type).value);

            return_value = INST_FAILED;
            goto end;
        }

        res = Nst_seq_get(cont, Nst_int_i64(idx));

        if (res == NULL)
            return_value = INST_FAILED;
        else
            Nst_vstack_push(&Nst_state.es->v_stack, res);
    } else if (cont->type == Nst_t.Map) {
        res = Nst_map_get(cont, idx);

        if (res != NULL)
            Nst_vstack_push(&Nst_state.es->v_stack, res);
        else if (idx->hash != -1)
            Nst_vstack_push(&Nst_state.es->v_stack, Nst_c.Null_null);
        else {
            Nst_error_setf_type(
                "type '%s' is not hashable",
                Nst_type_name(idx->type).value);
            return_value = INST_FAILED;
        }
    } else if (cont->type == Nst_t.Str) {
        if (idx->type != Nst_t.Int) {
            Nst_error_setf_type(
                "expected type 'Int', got '%s' instead",
                Nst_type_name(idx->type).value);

            return_value = INST_FAILED;
            goto end;
        }

        res = Nst_str_get_obj(cont, Nst_int_i64(idx));

        if (res == NULL)
            return_value = INST_FAILED;
        else
            Nst_vstack_push(&Nst_state.es->v_stack, res);
    } else {
        Nst_error_setf_type(
            "expected type 'Array', 'Vector', 'Map' or 'Str', got '%s' instead",
            Nst_type_name(cont->type).value);
        return_value = INST_FAILED;
    }

end:
    Nst_ndec_ref(res);
    Nst_dec_ref(cont);
    Nst_dec_ref(idx);
    return return_value;
}

static InstResult exe_dec_int()
{
    CHECK_V_STACK;
    _Nst_counter_dec(FAST_TOP_VAL);
    return INST_SUCCESS;
}

static InstResult exe_new_int()
{
    CHECK_V_STACK;
    Nst_Obj *obj = POP_TOP_VALUE;
    if (!type_check(obj, Nst_t.Int)) {
        Nst_dec_ref(obj);
        return INST_FAILED;
    }

    Nst_Obj *new_obj = Nst_int_new(Nst_int_i64(obj));
    Nst_dec_ref(obj);
    if (new_obj == NULL)
        return INST_FAILED;
    Nst_vstack_push(&Nst_state.es->v_stack, new_obj);
    Nst_dec_ref(new_obj);
    return INST_SUCCESS;
}

static InstResult exe_dup()
{
    CHECK_V_STACK;
    Nst_vstack_push(
        &Nst_state.es->v_stack,
        Nst_vstack_peek(&Nst_state.es->v_stack));
    return INST_SUCCESS;
}

static InstResult exe_rot()
{
    CHECK_V_STACK_SIZE((u64)inst->int_val);

    Nst_Obj *obj = Nst_vstack_peek(&Nst_state.es->v_stack);
    usize stack_size = Nst_state.es->v_stack.len - 1;
    Nst_Obj **stack = Nst_state.es->v_stack.stack;
    for (i64 i = 1, n = inst->int_val; i < n; i++)
        stack[stack_size - i + 1] = stack[stack_size - i];

    stack[stack_size - inst->int_val + 1] = obj;
    return INST_SUCCESS;
}

static i32 complete_seq(Nst_Obj *seq)
{
    if (seq == NULL)
        return INST_FAILED;

    for (i64 i = 1; i <= inst->int_val; i++) {
        Nst_Obj *curr_obj = POP_TOP_VALUE;
        Nst_seq_set(seq, inst->int_val - i, curr_obj);
        Nst_dec_ref(curr_obj);
    }

    Nst_vstack_push(&Nst_state.es->v_stack, seq);
    Nst_dec_ref(seq);
    return INST_SUCCESS;
}

static InstResult exe_make_arr()
{
    CHECK_V_STACK_SIZE((u64)inst->int_val);
    return complete_seq(Nst_array_new((usize)inst->int_val));
}

static InstResult exe_make_vec()
{
    CHECK_V_STACK_SIZE((u64)inst->int_val);
    return complete_seq(Nst_vector_new((usize)inst->int_val));
}

static i32 complete_seq_rep(Nst_Obj *seq, i64 size)
{
    if (seq == NULL)
        return INST_FAILED;
    Nst_Obj *val = POP_TOP_VALUE;

    for (i64 i = 0; i < size; i++)
        Nst_seq_set(seq, i, val);

    Nst_vstack_push(&Nst_state.es->v_stack, seq);
    Nst_dec_ref(seq);
    Nst_dec_ref(val);
    return INST_SUCCESS;
}

static InstResult exe_make_arr_rep()
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *size_obj = POP_TOP_VALUE;

    if (!type_check(size_obj, Nst_t.Int)) {
        Nst_dec_ref(size_obj);
        return INST_FAILED;
    }

    i64 size = Nst_int_i64(size_obj);
    if (size < 0) {
        Nst_error_setc_value("the length of the array cannot be negative");
        Nst_dec_ref(size_obj);
        return INST_FAILED;
    }

    Nst_dec_ref(size_obj);
    Nst_Obj *seq = Nst_array_new((usize)size);
    return complete_seq_rep(seq, size);
}

static InstResult exe_make_vec_rep()
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *size_obj = POP_TOP_VALUE;

    if (!type_check(size_obj, Nst_t.Int)) {
        Nst_dec_ref(size_obj);
        return INST_FAILED;
    }

    i64 size = Nst_int_i64(size_obj);
    if (size < 0) {
        Nst_error_setc_value("the length of the vector cannot be negative");
        Nst_dec_ref(size_obj);
        return INST_FAILED;
    }

    Nst_dec_ref(size_obj);
    Nst_Obj *seq = Nst_array_new((usize)size);
    return complete_seq_rep(seq, size);
}

static InstResult exe_make_map()
{
    CHECK_V_STACK_SIZE((u64)inst->int_val);
    i64 map_size = inst->int_val;
    Nst_assert(map_size >= 0);
    Nst_Obj *map = Nst_map_new();
    if (map == NULL)
        return INST_FAILED;

    usize stack_size = Nst_state.es->v_stack.len;
    Nst_Obj **v_stack = Nst_state.es->v_stack.stack;

    for (i64 i = 0; i < map_size; i++) {
        Nst_Obj *key = v_stack[stack_size - map_size + i];
        i++;
        Nst_Obj *val = v_stack[stack_size - map_size + i];
        if (!Nst_map_set(map, key, val)) {
            Nst_dec_ref(val);
            Nst_dec_ref(key);
            Nst_dec_ref(map);
            return INST_FAILED;
        }
        Nst_dec_ref(val);
        Nst_dec_ref(key);
    }
    Nst_state.es->v_stack.len -= (usize)map_size;
    Nst_vstack_push(&Nst_state.es->v_stack, map);
    Nst_dec_ref(map);
    return INST_SUCCESS;
}

static InstResult exe_make_func()
{
    if (Nst_state.es->vt->global_table == NULL)
        return INST_SUCCESS;

    Nst_Obj *vars_copy = Nst_map_copy(Nst_state.es->vt->vars);
    if (vars_copy == NULL)
        return INST_FAILED;

    Nst_ndec_ref(Nst_map_drop(vars_copy, Nst_s.o__vars_));
    if (Nst_map_len(vars_copy) != 0) {
        Nst_Obj *func = POP_TOP_VALUE;
        Nst_Obj *new_func = _Nst_func_new_outer_vars(func, vars_copy);
        Nst_dec_ref(func);
        if (new_func == NULL) {
            Nst_dec_ref(vars_copy);
            return INST_FAILED;
        }
        Nst_vstack_push(&Nst_state.es->v_stack, new_func);
        Nst_dec_ref(new_func);
    }
    Nst_dec_ref(vars_copy);

    return INST_SUCCESS;
}

static InstResult exe_push_catch()
{
    Nst_CatchFrame cf = {
        .inst_idx = inst->int_val,
        .v_stack_len = Nst_state.es->v_stack.len,
        .f_stack_len = Nst_state.es->f_stack.len
    };
    Nst_cstack_push(&Nst_state.es->c_stack, cf);
    return INST_SUCCESS;
}

static InstResult exe_pop_catch()
{
    Nst_cstack_pop(&Nst_state.es->c_stack);
    return INST_SUCCESS;
}

static InstResult exe_save_error()
{
    Nst_assert(Nst_error_occurred());

    Nst_Obj *err_map = Nst_map_new();
    Nst_Traceback *tb = Nst_error_get();
    Nst_map_set_str(err_map, "name", tb->error_name);
    Nst_map_set_str(err_map, "message", tb->error_msg);
    Nst_error_clear();

    Nst_vstack_push(&Nst_state.es->v_stack, err_map);
    Nst_dec_ref(err_map);

    // Remove the source of any libraries that failed to load
    while (Nst_state.lib_srcs->len > Nst_map_len(Nst_state.lib_handles)) {
        Nst_SourceText *txt = (Nst_SourceText *)Nst_llist_pop(
            Nst_state.lib_srcs);
        source_text_destructor(txt);
    }
    return INST_SUCCESS;
}

static InstResult exe_unpack_seq()
{
    CHECK_V_STACK;
    Nst_Obj *seq = POP_TOP_VALUE;

    if (seq->type != Nst_t.Array && seq->type != Nst_t.Vector) {
        Nst_error_setf_type(
            "expected type 'Array' or 'Vector', got '%s' instead",
            Nst_type_name(seq->type).value);
        Nst_dec_ref(seq);
        return INST_FAILED;
    }

    if ((i64)Nst_seq_len(seq) != inst->int_val) {
        Nst_error_setf_value(
            "expected %"PRIi64" items to unpack but the sequence contains %zi",
            inst->int_val, Nst_seq_len(seq));
        Nst_dec_ref(seq);
        return INST_FAILED;
    }

    for (i64 i = Nst_seq_len(seq) - 1; i >= 0; i--) {
        Nst_Obj *obj = Nst_seq_getf(seq, i);
        Nst_vstack_push(&Nst_state.es->v_stack, obj);
        Nst_dec_ref(obj);
    }

    Nst_dec_ref(seq);
    return INST_SUCCESS;
}

usize Nst_get_full_path(const char *file_path, char **buf, char **file_part)
{
    *buf = NULL;
    if (file_part != NULL)
        *file_part = NULL;

#ifdef Nst_MSVC
    wchar_t *wide_full_path = Nst_malloc_c(PATH_MAX, wchar_t);
    if (wide_full_path == NULL)
        return 0;
    wchar_t *wide_file_path = Nst_char_to_wchar_t(file_path, 0);
    if (wide_file_path == NULL) {
        Nst_free(wide_full_path);
        return 0;
    }

    DWORD full_path_len = GetFullPathNameW(
        wide_file_path,
        PATH_MAX,
        wide_full_path,
        NULL);

    if (full_path_len == 0) {
        Nst_free(wide_full_path);
        Nst_free(wide_file_path);
        Nst_error_setf_value("file '%.4096s' not found", file_path);
        return 0;
    }

    if (full_path_len > PATH_MAX) {
        Nst_free(wide_full_path);
        wide_full_path = Nst_malloc_c(full_path_len + 1, wchar_t);
        if (wide_full_path == NULL) {
            Nst_free(wide_file_path);
            return 0;
        }
        full_path_len = GetFullPathNameW(
            wide_file_path,
            full_path_len + 1,
            wide_full_path,
            NULL);

        if (full_path_len == 0) {
            Nst_free(wide_full_path);
            Nst_free(wide_file_path);
            Nst_error_setf_value("file '%.4096s' not found", file_path);
            return 0;
        }
    }
    Nst_free(wide_file_path);

    char *full_path = Nst_wchar_t_to_char(wide_full_path, full_path_len);
    Nst_free(wide_full_path);
    if (full_path == NULL)
        return 0;

    if (file_part != NULL) {
        *file_part = strrchr(full_path, '\\');

        if (*file_part == NULL)
            *file_part = full_path;
        else
            (*file_part)++;
    }

    *buf = full_path;
    return full_path_len;

#else

    char *path = Nst_malloc_c(PATH_MAX, char);
    if (path == NULL)
        return 0;

    char *result = realpath(file_path, path);

    if (result == NULL) {
        Nst_free(path);
        return 0;
    }

    if (file_part != NULL) {
        *file_part = strrchr(path, '/');

        if (*file_part == NULL)
            *file_part = path;
        else
            (*file_part)++;
    }

    *buf = path;
    return strlen(path);
#endif // !Nst_MSVC
}

Nst_IntrState *Nst_state_get(void)
{
    return &Nst_state;
}

i32 Nst_chdir(Nst_Obj *str)
{
#ifdef Nst_MSVC
    wchar_t *wide_cwd = Nst_char_to_wchar_t(
        (char *)Nst_str_value(str),
        Nst_str_len(str));
    if (wide_cwd == NULL)
        return -1;
    i32 res = _wchdir(wide_cwd);
    Nst_free(wide_cwd);
#else
    i32 res = chdir((char *)Nst_str_value(str));
#endif // !Nst_MSVC

    if (res != 0)
        Nst_error_setc_call("failed to change the current working directory");
    else if (Nst_state.es != NULL) {
        Nst_inc_ref(str);
        Nst_es_set_cwd(Nst_state.es, str);
    }
    return res != 0 ? -1 : 0;
}

Nst_Obj *Nst_getcwd(void)
{
#ifdef Nst_MSVC
    wchar_t *wide_cwd = Nst_malloc_c(PATH_MAX, wchar_t);
    if (wide_cwd == NULL)
        return NULL;
    wchar_t *result = _wgetcwd(wide_cwd, PATH_MAX);
    if (result == NULL) {
        Nst_free(wide_cwd);
        Nst_error_setc_call("failed to get the current working directory");
        return NULL;
    }
    u8 *cwd_buf = (u8 *)Nst_wchar_t_to_char(wide_cwd, 0);
    Nst_free(wide_cwd);
    if (cwd_buf == NULL)
        return NULL;
    return Nst_str_new_allocated(cwd_buf, strlen((char *)cwd_buf));
#else
    u8 *cwd_buf = Nst_malloc_c(PATH_MAX, u8);
    if (cwd_buf == NULL)
        return NULL;
    u8 *cwd_result = (u8 *)getcwd((char *)cwd_buf, PATH_MAX);
    if (cwd_result == NULL) {
        Nst_free(cwd_buf);
        Nst_error_setc_call("failed to get the current working directory");
        return NULL;
    }
    return Nst_str_new_allocated(cwd_buf, strlen((char *)cwd_buf));
#endif // !Nst_MSVC
}

static void loaded_libs_destructor(lib_t lib)
{
    dlclose(lib);
}

void source_text_destructor(Nst_SourceText *src)
{
    Nst_source_text_destroy(src);
    Nst_free(src);
}
