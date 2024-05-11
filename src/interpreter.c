#include <errno.h>
#include <stdlib.h>
#include "obj_ops.h"
#include "hash.h"
#include "tokens.h"
#include "iter.h"
#include "optimizer.h"
#include "parser.h"
#include "lexer.h"
#include "format.h"
#include "encoding.h"

#ifdef Nst_WIN

#include <windows.h>
#include <direct.h>

#define dlclose FreeLibrary
#define dlsym(lib, name) GetProcAddress((HMODULE)lib, name)
#define PATH_MAX 4096

#define C_LIB_TYPE HMODULE

#else

#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#define C_LIB_TYPE void *

#endif // !Nst_WIN

#include "mem.h"

#define INST_FAILED -1
#define INST_SUCCESS 0
#define INST_NEW_FUNC 1
#define INST_SUCCESS_ALREADY_ADVANCED 2

#define CHECK_V_STACK Nst_es_assert_vstack_size_at_least(Nst_state.es, 1)
#define CHECK_V_STACK_SIZE(size)                                              \
    Nst_es_assert_vstack_size_at_least(Nst_state.es, size)
#define UNTIL_CURRENT_FUNC_FINISHES Nst_state.es->f_stack.len - 1
#define UNTIL_ALL_FUNCS_FINISH 0
#define POP_TOP_VALUE Nst_vstack_pop(&Nst_state.es->v_stack)
#define GET_CURRENT_INST_LS                                                   \
    Nst_fstack_peek(&Nst_state.es->f_stack).func->body.bytecode

Nst_IntrState Nst_state;
static bool state_init = false;

static void complete_function(usize final_stack_size);
static bool type_check(Nst_Obj *obj, Nst_TypeObj *type);

static i32 exe_no_op(Nst_Inst *inst);
static i32 exe_pop_val(Nst_Inst *inst);
static i32 exe_for_start(Nst_Inst *inst);
static i32 exe_jumpif_iend(Nst_Inst *inst);
static i32 exe_for_get_val(Nst_Inst *inst);
static i32 exe_return_val(Nst_Inst *inst);
static i32 exe_return_vars(Nst_Inst *inst);
static i32 exe_set_val_loc(Nst_Inst *inst);
static i32 exe_set_cont_loc(Nst_Inst *inst);
static i32 exe_jump(Nst_Inst *inst);
static i32 exe_jumpif_t(Nst_Inst *inst);
static i32 exe_jumpif_f(Nst_Inst *inst);
static i32 exe_jumpif_zero(Nst_Inst *inst);
static i32 exe_type_check(Nst_Inst *inst);
static i32 exe_hash_check(Nst_Inst *inst);
static i32 exe_set_val(Nst_Inst *inst);
static i32 exe_get_val(Nst_Inst *inst);
static i32 exe_push_val(Nst_Inst *inst);
static i32 exe_set_cont_val(Nst_Inst *inst);
static i32 exe_op_call(Nst_Inst *inst);
static i32 exe_op_cast(Nst_Inst *inst);
static i32 exe_op_range(Nst_Inst *inst);
static i32 exe_throw_err(Nst_Inst *inst);
static i32 exe_stack_op(Nst_Inst *inst);
static i32 exe_local_op(Nst_Inst *inst);
static i32 exe_op_import(Nst_Inst *inst);
static i32 exe_op_extract(Nst_Inst *inst);
static i32 exe_dec_int(Nst_Inst *inst);
static i32 exe_new_int(Nst_Inst *inst);
static i32 exe_dup(Nst_Inst *inst);
static i32 exe_rot(Nst_Inst *inst);
static i32 exe_make_seq(Nst_Inst *inst);
static i32 exe_make_seq_rep(Nst_Inst *inst);
static i32 exe_make_map(Nst_Inst *inst);
static i32 exe_push_catch(Nst_Inst *inst);
static i32 exe_pop_catch(Nst_Inst *inst);
static i32 exe_save_error(Nst_Inst *inst);
static i32 exe_unpack_seq(Nst_Inst *inst);

static i32 call_c_func(bool is_seq_call, i64 arg_num, Nst_SeqObj *args_seq,
                       Nst_FuncObj *func);
static void loaded_libs_destructor(C_LIB_TYPE lib);
static void source_text_destructor(Nst_SourceText *src);

i32 (*inst_func[])(Nst_Inst *) = {
    [Nst_IC_NO_OP]        = exe_no_op,
    [Nst_IC_POP_VAL]      = exe_pop_val,
    [Nst_IC_FOR_START]    = exe_for_start,
    [Nst_IC_FOR_GET_VAL]  = exe_for_get_val,
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
    [Nst_IC_TYPE_CHECK]   = exe_type_check,
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
    [Nst_IC_MAKE_ARR]     = exe_make_seq,
    [Nst_IC_MAKE_ARR_REP] = exe_make_seq_rep,
    [Nst_IC_MAKE_VEC]     = exe_make_seq,
    [Nst_IC_MAKE_VEC_REP] = exe_make_seq_rep,
    [Nst_IC_MAKE_MAP]     = exe_make_map,
    [Nst_IC_SAVE_ERROR]   = exe_save_error,
    [Nst_IC_UNPACK_SEQ]   = exe_unpack_seq
};

Nst_Obj *(*stack_op_func[])(Nst_Obj *, Nst_Obj *) = {
    [Nst_TT_ADD]      = _Nst_obj_add,
    [Nst_TT_SUB]      = _Nst_obj_sub,
    [Nst_TT_MUL]      = _Nst_obj_mul,
    [Nst_TT_DIV]      = _Nst_obj_div,
    [Nst_TT_POW]      = _Nst_obj_pow,
    [Nst_TT_MOD]      = _Nst_obj_mod,
    [Nst_TT_B_AND]    = _Nst_obj_bwand,
    [Nst_TT_B_OR]     = _Nst_obj_bwor,
    [Nst_TT_B_XOR]    = _Nst_obj_bwxor,
    [Nst_TT_LSHIFT]   = _Nst_obj_bwls,
    [Nst_TT_RSHIFT]   = _Nst_obj_bwrs,
    [Nst_TT_CONCAT]   = _Nst_obj_concat,
    [Nst_TT_L_AND]    = _Nst_obj_lgand,
    [Nst_TT_L_OR]     = _Nst_obj_lgor,
    [Nst_TT_L_XOR]    = _Nst_obj_lgxor,
    [Nst_TT_GT]       = _Nst_obj_gt,
    [Nst_TT_LT]       = _Nst_obj_lt,
    [Nst_TT_EQ]       = _Nst_obj_eq,
    [Nst_TT_NEQ]      = _Nst_obj_ne,
    [Nst_TT_GTE]      = _Nst_obj_ge,
    [Nst_TT_LTE]      = _Nst_obj_le,
    [Nst_TT_CONTAINS] = _Nst_obj_contains
};

Nst_Obj *(*local_op_func[])(Nst_Obj *) = {
    [Nst_TT_LEN      - Nst_TT_LEN] = _Nst_obj_len,
    [Nst_TT_L_NOT    - Nst_TT_LEN] = _Nst_obj_lgnot,
    [Nst_TT_B_NOT    - Nst_TT_LEN] = _Nst_obj_bwnot,
    [Nst_TT_STDOUT   - Nst_TT_LEN] = _Nst_obj_stdout,
    [Nst_TT_STDIN    - Nst_TT_LEN] = _Nst_obj_stdin,
    [Nst_TT_IMPORT   - Nst_TT_LEN] = NULL,
    [Nst_TT_LOC_CALL - Nst_TT_LEN] = NULL,
    [Nst_TT_NEG      - Nst_TT_LEN] = _Nst_obj_neg,
    [Nst_TT_TYPEOF   - Nst_TT_LEN] = _Nst_obj_typeof
};

bool Nst_init(Nst_CLArgs *args)
{
    Nst_set_color(Nst_supports_color());

    // these need to be set to allow _Nst_init_objects to be called
    Nst_state.es = NULL;
    Nst_state.global_traceback.error_occurred = false;
    Nst_state.global_traceback.positions = NULL;

    if (!_Nst_init_objects()) {
        fprintf(stderr, "Memory Error - memory allocation failed...\n");
        return false;
    }

    if (!Nst_traceback_init(&Nst_state.global_traceback)) {
        fprintf(stderr, "Memory Error - memory allocation failed...\n");
        Nst_error_clear();
        goto cleanup;
    }
    Nst_ggc_init();

    if (args != NULL)
        Nst_state.opt_level = args->opt_level;
    else
        Nst_state.opt_level = 3;
    Nst_state.loaded_libs = Nst_llist_new();
    Nst_state.lib_paths = Nst_llist_new();
    Nst_state.lib_srcs = Nst_llist_new();
    Nst_state.lib_handles = MAP(Nst_map_new());

    if (Nst_error_occurred())
        goto cleanup;

    state_init = true;

    return true;
cleanup:
    if (Nst_error_occurred())
        Nst_print_traceback(&Nst_state.global_traceback);

    Nst_quit();
    return false;
}

void Nst_quit(void)
{
    state_init = false;

    if (Nst_state.global_traceback.positions != NULL)
        Nst_traceback_destroy(&Nst_state.global_traceback);
    if (Nst_state.lib_paths != NULL) {
        Nst_llist_destroy(
            Nst_state.lib_paths,
            (Nst_LListDestructor)_Nst_dec_ref);
    }
#if !defined(_DEBUG) || !defined(Nst_TRACK_OBJ_INIT_POS)
    if (Nst_state.lib_srcs != NULL) {
        Nst_llist_destroy(
            Nst_state.lib_srcs,
            (Nst_LListDestructor)source_text_destructor);
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
            void (*free_lib_func)(void) =
                (void (*)(void))dlsym(lib->value, "free_lib");
            if (free_lib_func != NULL)
                free_lib_func();
        }
    }

    Nst_dec_ref(Nst_state.lib_handles);
    _Nst_ggc_delete_objs();
    _Nst_del_objects();

    // the libraries are freed only after the objects because I/O streams might
    // contain objects that are created inside a libraray and are deleted when
    // that library is unloaded
    if (Nst_state.loaded_libs != NULL) {
        Nst_llist_destroy(
            Nst_state.loaded_libs,
            (Nst_LListDestructor)loaded_libs_destructor);
    }
}

bool Nst_was_init(void)
{
    return state_init;
}

i32 Nst_run(Nst_FuncObj *main_func)
{
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
        .start = Nst_no_pos(),
        .end = Nst_no_pos(),
        .vt = NULL,
        .cstack_len = 0,
        .idx = 0,
        .cwd = NULL
    };
    Nst_fstack_push(&Nst_state.es->f_stack, call);
    Nst_func_set_vt(main_func, Nst_state.es->vt->vars);

    complete_function(UNTIL_ALL_FUNCS_FINISH);
    Nst_dec_ref(main_func);

    // Check for errors
    i32 exit_code = 0;
    if (Nst_error_occurred()) {
        Nst_Traceback *error = Nst_error_get();
        if (OBJ(error->error_name) != Nst_c.Null_null)
            exit_code = 1;
        else {
            exit_code = (i32)AS_INT(error->error_msg);
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

static inline void set_global_error(usize final_stack_size, Nst_Inst *inst)
{
    Nst_error_add_positions(Nst_error_get(), inst->start, inst->end);

    Nst_CatchFrame top_catch = Nst_cstack_peek(&Nst_state.es->c_stack);
    if (OBJ(Nst_error_get()->error_name) == Nst_c.Null_null) {
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
        obj = Nst_vstack_pop(&Nst_state.es->v_stack);

        while (obj != NULL) {
            Nst_dec_ref(obj);
            obj = Nst_vstack_pop(&Nst_state.es->v_stack);
        }

        Nst_error_add_positions(Nst_error_get(), call.start, call.end);
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
        if (Nst_state.es->idx >= (i64)curr_inst_ls->total_size) {
            // Free the function call
            Nst_FuncCall call = Nst_fstack_pop(&Nst_state.es->f_stack);
            destroy_call(&call);
            if (Nst_state.es->f_stack.len <= final_stack_size)
                return;
            Nst_state.es->idx++;
            curr_inst_ls = GET_CURRENT_INST_LS;
            instructions = curr_inst_ls->instructions;
        }

        Nst_Inst *inst = &instructions[Nst_state.es->idx];
        Nst_InstID inst_id = inst->id;
        i32 result = inst_func[inst_id](inst);

        if (Nst_state.ggc.gen1.len > _Nst_GEN1_MAX)
            Nst_ggc_collect();

        if (result == INST_SUCCESS) {
            Nst_state.es->idx++;
            continue;
        } else if (result == INST_FAILED) {
            set_global_error(final_stack_size, inst);
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

    Nst_InstList *instructions =
        Nst_fstack_peek(&Nst_state.es->f_stack).func->body.bytecode;
    if (Nst_state.es->idx >= (i64)instructions->total_size
        || Nst_state.es->idx < 0)
    {
        return NULL;
    }
    return &instructions->instructions[Nst_state.es->idx];
}

bool Nst_run_module(i8 *filename, Nst_SourceText *lib_src)
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

Nst_Obj *Nst_call_func(Nst_FuncObj *func, i32 arg_num, Nst_Obj **args)
{
    if (Nst_HAS_FLAG(func, Nst_FLAG_FUNC_IS_C)) {
        i64 tot_args = func->arg_num;
        i64 null_args = (i64)func->arg_num - arg_num;

        if (tot_args < arg_num) {
            Nst_set_call_error(
                _Nst_EM_WRONG_ARG_NUM_FMT((usize)tot_args, (i64)arg_num));
            return NULL;
        }

        Nst_Obj **all_args;
        Nst_Obj *stack_args[10]; // for up to 10 arguments this array is used

        if (tot_args == 0)
            all_args = NULL;
        else if (null_args == 0)
            all_args = args;
        else if (tot_args <= 10) {
            memcpy(stack_args, args, (usize)arg_num * sizeof(Nst_Obj *));
            all_args = stack_args;
        } else {
            all_args = Nst_malloc_c((usize)tot_args, Nst_Obj *);
            if (args == NULL) {
                return NULL;
            }
            memcpy(all_args, args, (usize)arg_num * sizeof(Nst_Obj *));
        }

        for (i64 i = 0; i < null_args; i++)
            all_args[arg_num + i] = Nst_inc_ref(Nst_c.Null_null);

        Nst_Obj *res = func->body.c_func((usize)tot_args, all_args);
        if (all_args != stack_args && all_args != args)
            Nst_free(all_args);

        return res;
    }

    Nst_es_push_func(
        Nst_state.es,
        func,
        Nst_no_pos(), Nst_no_pos(),
        arg_num, args);
    complete_function(UNTIL_CURRENT_FUNC_FINISHES);

    if (Nst_error_occurred())
        return NULL;

    return POP_TOP_VALUE;
}

Nst_Obj *Nst_run_paused_coroutine(Nst_FuncObj *func, i64 idx, Nst_VarTable *vt)
{
    bool result = Nst_es_push_paused_coroutine(
        Nst_state.es,
        func,
        Nst_no_pos(),
        Nst_no_pos(),
        idx,
        vt);

    if (!result)
        return NULL;

    complete_function(UNTIL_CURRENT_FUNC_FINISHES);

    if (Nst_error_occurred())
        return NULL;

    return POP_TOP_VALUE;
}

static bool type_check(Nst_Obj *obj, Nst_TypeObj *type)
{
    if (obj->type != type) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_EXPECTED_TYPES,
            Nst_TYPE_STR(type)->value,
            TYPE_NAME(obj)));
        return false;
    }
    return true;
}

static i32 exe_no_op(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    return INST_SUCCESS;
}

static i32 exe_pop_val(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *obj = POP_TOP_VALUE;
    Nst_dec_ref(obj);
    return INST_SUCCESS;
}

static i32 exe_for_inst(Nst_Inst *inst, Nst_IterObj *iter,
                        Nst_FuncObj *func)
{
    if (Nst_HAS_FLAG(func, Nst_FLAG_FUNC_IS_C)) {
        Nst_Obj *res = func->body.c_func((usize)inst->int_val, &iter->value);
        if (res == NULL)
            return INST_FAILED;

        Nst_vstack_push(&Nst_state.es->v_stack, res);
        Nst_dec_ref(res);
        return INST_SUCCESS;
    }
    Nst_vstack_push(&Nst_state.es->v_stack, iter->value);
    Nst_vstack_push(&Nst_state.es->v_stack, func);
    exe_op_call(inst);
    return INST_NEW_FUNC;
}

static i32 exe_for_start(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *iterable = Nst_vstack_peek(&Nst_state.es->v_stack);
    Nst_IterObj *iter = ITER(Nst_obj_cast(iterable, Nst_t.Iter));
    if (iter == NULL) {
        Nst_set_type_errorf(_Nst_EM_BAD_CAST("Iter"), TYPE_NAME(iterable));
        return INST_FAILED;
    }
    Nst_state.es->v_stack.stack[Nst_state.es->v_stack.len - 1] = OBJ(iter);
    Nst_dec_ref(iterable);
    return exe_for_inst(inst, iter, iter->start);
}

static i32 exe_for_get_val(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(Nst_vstack_peek(&Nst_state.es->v_stack));
    return exe_for_inst(inst, iter, iter->get_val);
}

static i32 exe_jumpif_iend(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *iter_val = Nst_vstack_peek(&Nst_state.es->v_stack);
    if (iter_val == Nst_c.IEnd_iend) {
        Nst_dec_ref(Nst_vstack_pop(&Nst_state.es->v_stack));
        Nst_state.es->idx = inst->int_val - 1;
    }
    return INST_SUCCESS;
}

static i32 exe_return_val(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    Nst_Obj *result = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *obj = Nst_vstack_pop(&Nst_state.es->v_stack);

    while (obj != NULL) {
        Nst_dec_ref(obj);
        obj = Nst_vstack_pop(&Nst_state.es->v_stack);
    }

    Nst_vstack_push(&Nst_state.es->v_stack, result);
    Nst_es_force_function_end(Nst_state.es);
    Nst_dec_ref(result);
    return INST_NEW_FUNC;
}

static i32 exe_return_vars(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    Nst_MapObj *vars = Nst_state.es->vt->vars;
    Nst_Obj *obj = Nst_vstack_pop(&Nst_state.es->v_stack);

    while (obj != NULL) {
        Nst_dec_ref(obj);
        obj = Nst_vstack_pop(&Nst_state.es->v_stack);
    }

    Nst_vstack_push(&Nst_state.es->v_stack, vars);
    Nst_es_force_function_end(Nst_state.es);
    return INST_NEW_FUNC;
}

static i32 exe_set_val_loc(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *val = Nst_vstack_pop(&Nst_state.es->v_stack);
    bool res = Nst_vt_set(Nst_state.es->vt, inst->val, val);
    Nst_dec_ref(val);
    return res ? INST_SUCCESS : INST_FAILED;
}

static i32 exe_set_cont_loc(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    i32 res = exe_set_cont_val(inst);
    if (res == INST_FAILED)
        return INST_FAILED;

    CHECK_V_STACK;
    Nst_dec_ref(Nst_vstack_pop(&Nst_state.es->v_stack));
    return res;
}

static i32 exe_jump(Nst_Inst *inst)
{
    Nst_state.es->idx = inst->int_val - 1;
    return INST_SUCCESS;
}

static i32 exe_jumpif_t(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = Nst_vstack_pop(&Nst_state.es->v_stack);
    if (Nst_obj_to_bool(top_val))
        Nst_state.es->idx = inst->int_val - 1;
    Nst_dec_ref(top_val);
    return INST_SUCCESS;
}

static i32 exe_jumpif_f(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = Nst_vstack_pop(&Nst_state.es->v_stack);
    if (!Nst_obj_to_bool(top_val))
        Nst_state.es->idx = inst->int_val - 1;

    Nst_dec_ref(top_val);
    return INST_SUCCESS;
}

static i32 exe_jumpif_zero(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = Nst_vstack_peek(&Nst_state.es->v_stack);
    if (AS_INT(val) == 0)
        Nst_state.es->idx = inst->int_val - 1;

    return INST_SUCCESS;
}

static i32 exe_type_check(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_peek(&Nst_state.es->v_stack);
    if (obj->type != TYPE(inst->val)) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_EXPECTED_TYPES,
            Nst_TYPE_STR(inst->val)->value,
            TYPE_NAME(obj)));
        return INST_FAILED;
    }
    return INST_SUCCESS;
}

static i32 exe_hash_check(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_peek(&Nst_state.es->v_stack);
    Nst_obj_hash(obj);
    if (obj->hash == -1) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_UNHASHABLE_TYPE,
            TYPE_NAME(obj)));
        return INST_FAILED;
    }
    return INST_SUCCESS;
}

static i32 exe_set_val(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_vt_set(
        Nst_state.es->vt,
        inst->val,
        Nst_vstack_peek(&Nst_state.es->v_stack));
    return Nst_error_occurred() ? INST_FAILED : INST_SUCCESS;
}

static i32 exe_get_val(Nst_Inst *inst)
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

static i32 exe_push_val(Nst_Inst *inst)
{
    bool res = Nst_vstack_push(&Nst_state.es->v_stack, inst->val);
    return res ? INST_SUCCESS : INST_FAILED;
}

static i32 exe_set_cont_val(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK_SIZE(3);
    Nst_Obj *idx = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *cont = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *val = Nst_vstack_peek(&Nst_state.es->v_stack);
    i32 return_value = INST_SUCCESS;

    if (cont->type == Nst_t.Array || cont->type == Nst_t.Vector) {
        if (idx->type != Nst_t.Int) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_EXPECTED_TYPE("Int"),
                TYPE_NAME(idx)));

            return_value = INST_FAILED;
            goto end;
        }

        if (!Nst_seq_set(cont, AS_INT(idx), val))
            return_value = INST_FAILED;
        goto end;
    } else if (cont->type == Nst_t.Map) {
        if (!Nst_map_set(cont, idx, val))
            return_value = INST_FAILED;
        goto end;
    }

    Nst_set_type_error(Nst_sprintf(
        _Nst_EM_EXPECTED_TYPE("Array', 'Vector', or 'Map"),
        TYPE_NAME(cont)));
    return_value = INST_FAILED;

end:
    Nst_dec_ref(cont);
    Nst_dec_ref(idx);
    return return_value;
}

static i32 call_c_func(bool is_seq_call, i64 arg_num, Nst_SeqObj *args_seq,
                       Nst_FuncObj *func)
{
    i64 tot_args = func->arg_num;
    i64 null_args = (i64)func->arg_num - arg_num;

    if (tot_args < arg_num) {
        Nst_set_call_error(
            _Nst_EM_WRONG_ARG_NUM_FMT((usize)tot_args, arg_num));
        Nst_dec_ref(func);
        Nst_ndec_ref(args_seq);
        return INST_FAILED;
    }

    Nst_Obj **args;
    Nst_Obj *stack_args[10]; // for up to 10 arguments this array is used

    if (tot_args == 0)
        args = NULL;
    else if (is_seq_call && null_args == 0)
        args = args_seq->objs;
    else if (is_seq_call && tot_args <= 10) {
        memcpy(stack_args, args_seq->objs, (usize)arg_num * sizeof(Nst_Obj *));
        args = stack_args;
    } else if (is_seq_call) {
        args = Nst_malloc_c((usize)tot_args, Nst_Obj *);
        if (args == NULL) {
            Nst_dec_ref(args_seq);
            Nst_dec_ref(func);
            return INST_FAILED;
        }
        memcpy(args, args_seq->objs, (usize)arg_num * sizeof(Nst_Obj *));
    } else if (tot_args <= 10) {
        for (i64 i = arg_num - 1; i >= 0; i--)
            stack_args[i] = Nst_vstack_pop(&Nst_state.es->v_stack);
        args = stack_args;
    } else {
        args = Nst_malloc_c((usize)tot_args, Nst_Obj *);
        if (args == NULL) {
            Nst_dec_ref(func);
            return INST_FAILED;
        }
        for (i64 i = arg_num - 1; i >= 0; i--)
            args[i] = Nst_vstack_pop(&Nst_state.es->v_stack);
    }

    for (i64 i = 0; i < null_args; i++)
        args[arg_num + i] = Nst_inc_ref(Nst_c.Null_null);

    Nst_Obj *res = func->body.c_func((usize)tot_args, args);

    if (!is_seq_call) {
        for (i64 i = 0; i < tot_args; i++)
            Nst_dec_ref(args[i]);
    } else {
        for (i64 i = 0; i < null_args; i++)
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

static i32 exe_op_call(Nst_Inst *inst)
{
#ifdef _DEBUG
    if (inst->int_val == -1)
        CHECK_V_STACK_SIZE(2);
    else
        CHECK_V_STACK_SIZE((u64)inst->int_val + 1);
#endif // !_DEBUG

    i64 arg_num = inst->int_val;
    bool is_seq_call = false;
    Nst_FuncObj *func = FUNC(Nst_vstack_pop(&Nst_state.es->v_stack));

    if (!type_check(OBJ(func), Nst_t.Func)) {
        Nst_dec_ref(func);
        return INST_FAILED;
    }

    Nst_SeqObj *args_seq;

    if (arg_num == -1) {
        args_seq = SEQ(Nst_vstack_pop(&Nst_state.es->v_stack));
        if (args_seq->type != Nst_t.Array && args_seq->type != Nst_t.Vector) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_EXPECTED_TYPE("Array' or 'Vector"),
                TYPE_NAME(args_seq)));

            Nst_dec_ref(args_seq);
            Nst_dec_ref(func);
            return INST_FAILED;
        }
        is_seq_call = true;
        arg_num = args_seq->len;
    } else
        args_seq = NULL;

    if (Nst_HAS_FLAG(func, Nst_FLAG_FUNC_IS_C))
        return call_c_func(is_seq_call, arg_num, args_seq, func);

    bool result;
    if (is_seq_call) {
        result = Nst_es_push_func(
            Nst_state.es,
            func,
            inst->start,
            inst->end,
            arg_num,
            args_seq->objs);
    } else {
        result = Nst_es_push_func(
            Nst_state.es,
            func,
            inst->start,
            inst->end,
            arg_num,
            NULL);
    }
    Nst_ndec_ref(args_seq);
    return result ? INST_NEW_FUNC : INST_FAILED;
}

static i32 exe_op_cast(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *val = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *type = Nst_vstack_pop(&Nst_state.es->v_stack);

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

static i32 exe_op_range(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE((u64)inst->int_val);
    Nst_Obj *stop = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *step = NULL;
    Nst_Obj *start = NULL;

    if (!type_check(stop, Nst_t.Int)) {
        Nst_dec_ref(stop);
        return INST_FAILED;
    }

    if (inst->int_val == 3) {
        start = Nst_vstack_pop(&Nst_state.es->v_stack);
        step  = Nst_vstack_pop(&Nst_state.es->v_stack);

        if (!type_check(start, Nst_t.Int) || !type_check(step, Nst_t.Int)) {
            Nst_dec_ref(stop);
            Nst_dec_ref(step);
            Nst_dec_ref(start);
            return INST_FAILED;
        }
    } else {
        start = Nst_vstack_pop(&Nst_state.es->v_stack);

        if (!type_check(start, Nst_t.Int)) {
            Nst_dec_ref(stop);
            Nst_dec_ref(start);
            return INST_FAILED;
        }

        if (AS_INT(start) <= AS_INT(stop))
            step = Nst_inc_ref(Nst_c.Int_1);
        else
            step = Nst_inc_ref(Nst_c.Int_neg1);
    }

    Nst_Obj *iter = _Nst_obj_range(start, stop, step);

    Nst_dec_ref(start);
    Nst_dec_ref(stop);
    Nst_dec_ref(step);

    if (iter == NULL)
        return INST_FAILED;
    Nst_vstack_push(&Nst_state.es->v_stack, iter);
    Nst_dec_ref(iter);
    return INST_SUCCESS;
}

static i32 exe_throw_err(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *message = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *name = Nst_vstack_pop(&Nst_state.es->v_stack);

    Nst_Obj *message_str = Nst_obj_cast(message, Nst_t.Str);
    Nst_Obj *name_str = Nst_obj_cast(name, Nst_t.Str);

    Nst_dec_ref(name);
    Nst_dec_ref(message);

    if (message_str == NULL || name_str == NULL) {
        Nst_ndec_ref(name_str);
        Nst_ndec_ref(message_str);
        return INST_FAILED;
    }

    Nst_set_error(name_str, message_str);
    return INST_FAILED;
}

static i32 exe_stack_op(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *ob2 = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *ob1 = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *res = stack_op_func[inst->int_val](ob1, ob2);

    if (res != NULL) {
        Nst_vstack_push(&Nst_state.es->v_stack, res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(ob1);
    Nst_dec_ref(ob2);
    return res == NULL ? INST_FAILED : INST_SUCCESS;
}

static i32 exe_local_op(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *res = local_op_func[inst->int_val - Nst_TT_LEN](obj);

    if (res != NULL) {
        Nst_vstack_push(&Nst_state.es->v_stack, res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(obj);
    return res == NULL ? INST_FAILED : INST_SUCCESS;
}

static i32 exe_op_import(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *name = Nst_vstack_pop(&Nst_state.es->v_stack);

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

static i32 exe_op_extract(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *idx = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *cont = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_Obj *res = NULL;
    i32 return_value = INST_SUCCESS;

    if (cont->type == Nst_t.Array || cont->type == Nst_t.Vector) {
        if (idx->type != Nst_t.Int) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_EXPECTED_TYPE("Int"),
                TYPE_NAME(idx)));

            return_value = INST_FAILED;
            goto end;
        }

        res = Nst_seq_get(cont, AS_INT(idx));

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
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_UNHASHABLE_TYPE,
                TYPE_NAME(idx)));
            return_value = INST_FAILED;
        }
    } else if (cont->type == Nst_t.Str) {
        if (idx->type != Nst_t.Int) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_EXPECTED_TYPE("Int"),
                TYPE_NAME(idx)));

            return_value = INST_FAILED;
            goto end;
        }

        res = Nst_string_get(cont, AS_INT(idx));

        if (res == NULL)
            return_value = INST_FAILED;
        else
            Nst_vstack_push(&Nst_state.es->v_stack, res);
    } else {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_EXPECTED_TYPE("Array', 'Vector', 'Map' or 'Str"),
            TYPE_NAME(cont)));
        return_value = INST_FAILED;
    }

end:
    Nst_ndec_ref(res);
    Nst_dec_ref(cont);
    Nst_dec_ref(idx);
    return return_value;
}

static i32 exe_dec_int(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_peek(&Nst_state.es->v_stack);
    AS_INT(obj) -= 1;
    return INST_SUCCESS;
}

static i32 exe_new_int(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_pop(&Nst_state.es->v_stack);
    if (!type_check(obj, Nst_t.Int)) {
        Nst_dec_ref(obj);
        return INST_FAILED;
    }

    Nst_Obj *new_obj = Nst_int_new(AS_INT(obj));
    Nst_dec_ref(obj);
    if (new_obj == NULL)
        return INST_FAILED;
    Nst_vstack_push(&Nst_state.es->v_stack, new_obj);
    Nst_dec_ref(new_obj);
    return INST_SUCCESS;
}

static i32 exe_dup(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_vstack_push(
        &Nst_state.es->v_stack,
        Nst_vstack_peek(&Nst_state.es->v_stack));
    return INST_SUCCESS;
}

static i32 exe_rot(Nst_Inst *inst)
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

static i32 exe_make_seq(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE((u64)inst->int_val);

    i64 seq_size = inst->int_val;
    Nst_Obj *seq = inst->id == Nst_IC_MAKE_ARR
        ? Nst_array_new((usize)seq_size)
        : Nst_vector_new((usize)seq_size);
    if (Nst_error_occurred())
        return INST_FAILED;

    for (i64 i = 1; i <= seq_size; i++) {
        Nst_Obj *curr_obj = Nst_vstack_pop(&Nst_state.es->v_stack);
        Nst_seq_set(seq, seq_size - i, curr_obj);
        Nst_dec_ref(curr_obj);
    }

    Nst_vstack_push(&Nst_state.es->v_stack, seq);
    Nst_dec_ref(seq);
    return INST_SUCCESS;
}

static i32 exe_make_seq_rep(Nst_Inst *inst)
{
    i64 seq_size = inst->int_val;
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *size_obj = Nst_vstack_pop(&Nst_state.es->v_stack);

    if (!type_check(size_obj, Nst_t.Int)) {
        Nst_dec_ref(size_obj);
        return INST_FAILED;
    }

    Nst_Obj *val = Nst_vstack_pop(&Nst_state.es->v_stack);

    i64 size = AS_INT(size_obj);
    Nst_dec_ref(size_obj);
    Nst_Obj *seq = inst->id == Nst_IC_MAKE_ARR_REP
        ? Nst_array_new((usize)size)
        : Nst_vector_new((usize)size);
    if (seq == NULL) {
        Nst_dec_ref(val);
        return INST_FAILED;
    }

    for (i64 i = 1; i <= size; i++)
        Nst_seq_set(seq, seq_size - i, val);

    Nst_vstack_push(&Nst_state.es->v_stack, seq);
    Nst_dec_ref(seq);
    Nst_dec_ref(val);
    return INST_SUCCESS;
}

static i32 exe_make_map(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE((u64)inst->int_val);
    i64 map_size = inst->int_val;
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

static i32 exe_push_catch(Nst_Inst *inst)
{
    Nst_CatchFrame cf = {
        .inst_idx = inst->int_val,
        .v_stack_len = Nst_state.es->v_stack.len,
        .f_stack_len = Nst_state.es->f_stack.len
    };
    Nst_cstack_push(&Nst_state.es->c_stack, cf);
    return INST_SUCCESS;
}

static i32 exe_pop_catch(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    Nst_cstack_pop(&Nst_state.es->c_stack);
    return INST_SUCCESS;
}

static i32 exe_save_error(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    Nst_assert(Nst_error_occurred());

    Nst_Obj *err_map = Nst_map_new();
    Nst_Traceback *tb = Nst_error_get();
    Nst_map_set_str(err_map, "name", tb->error_name);
    Nst_map_set_str(err_map, "message", tb->error_msg);
    Nst_error_clear();

    Nst_vstack_push(&Nst_state.es->v_stack, err_map);
    Nst_dec_ref(err_map);

    // Remove the source of any libraries that failed to load
    while (Nst_state.lib_srcs->len > Nst_state.lib_handles->len) {
        Nst_SourceText *txt = (Nst_SourceText *)Nst_llist_pop(
            Nst_state.lib_srcs);
        Nst_source_text_destroy(txt);
    }
    return INST_SUCCESS;
}

static i32 exe_unpack_seq(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_SeqObj *seq = SEQ(Nst_vstack_pop(&Nst_state.es->v_stack));

    if (seq->type != Nst_t.Array && seq->type != Nst_t.Vector) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_EXPECTED_TYPE("Array' or 'Vector"),
            TYPE_NAME(seq)));
        Nst_dec_ref(seq);
        return INST_FAILED;
    }

    if ((i64)seq->len != inst->int_val) {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_WRONG_UNPACK_LENGTH,
            inst->int_val, seq->len));
        Nst_dec_ref(seq);
        return INST_FAILED;
    }

    for (i64 i = seq->len - 1; i >= 0; i--)
        Nst_vstack_push(&Nst_state.es->v_stack, seq->objs[i]);

    Nst_dec_ref(seq);
    return INST_SUCCESS;
}

usize Nst_get_full_path(i8 *file_path, i8 **buf, i8 **file_part)
{
    *buf = NULL;
    if (file_part != NULL)
        *file_part = NULL;

#ifdef Nst_WIN
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
        Nst_set_value_error(Nst_sprintf(_Nst_EM_FILE_NOT_FOUND, file_path));
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
            Nst_set_value_error(Nst_sprintf(_Nst_EM_FILE_NOT_FOUND, file_path));
            return 0;
        }
    }
    Nst_free(wide_file_path);

    i8 *full_path = Nst_wchar_t_to_char(wide_full_path, full_path_len);
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

    i8 *path = Nst_malloc_c(PATH_MAX, i8);
    if (path == NULL)
        return 0;

    i8 *result = realpath(file_path, path);

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
#endif // !Nst_WIN
}

Nst_IntrState *Nst_get_state(void)
{
    return &Nst_state;
}

i32 Nst_chdir(Nst_StrObj *str)
{
#ifdef Nst_WIN
    wchar_t *wide_cwd = Nst_char_to_wchar_t(str->value, str->len);
    if (wide_cwd == NULL)
        return -1;
    i32 res = _wchdir(wide_cwd);
    Nst_free(wide_cwd);
#else
    i32 res = chdir(str->value);
#endif // !Nst_WIN

    if (res != 0)
        Nst_set_call_error_c(_Nst_EM_FAILED_CHDIR);
    else if (Nst_state.es != NULL) {
        Nst_inc_ref(str);
        Nst_es_set_cwd(Nst_state.es, str);
    }
    return res != 0 ? -1 : 0;
}

Nst_StrObj *Nst_getcwd(void)
{
#ifdef Nst_WIN
    wchar_t *wide_cwd = Nst_malloc_c(PATH_MAX, wchar_t);
    if (wide_cwd == NULL)
        return NULL;
    wchar_t *result = _wgetcwd(wide_cwd, PATH_MAX);
    if (result == NULL) {
        Nst_free(wide_cwd);
        Nst_set_call_error_c(_Nst_EM_FAILED_GETCWD);
        return NULL;
    }
    i8 *cwd_buf = Nst_wchar_t_to_char(wide_cwd, 0);
    Nst_free(wide_cwd);
    if (cwd_buf == NULL)
        return NULL;
    return STR(Nst_string_new_allocated(cwd_buf, strlen(cwd_buf)));
#else
    i8 *cwd_buf = Nst_malloc_c(PATH_MAX, i8);
    if (cwd_buf == NULL)
        return NULL;
    i8 *cwd_result = getcwd(cwd_buf, PATH_MAX);
    if (cwd_result == NULL) {
        Nst_free(cwd_buf);
        Nst_set_call_error_c(_Nst_EM_FAILED_GETCWD);
        return NULL;
    }
    return STR(Nst_string_new_allocated(cwd_buf, strlen(cwd_buf)));
#endif // !Nst_WIN
}

static void loaded_libs_destructor(C_LIB_TYPE lib)
{
    dlclose(lib);
}

void source_text_destructor(Nst_SourceText *src)
{
    Nst_source_text_destroy(src);
    Nst_free(src);
}
