#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "nest.h"
#include "mem.h"

#ifdef Nst_MSVC

#include <direct.h>
#define PATH_MAX 4096

#else

#include <unistd.h>

#endif // !Nst_MSVC

#define CHECK_V_STACK(size) Nst_assert(i_state.v_stack.len >= size)
#define FAST_TOP (i_state.v_stack.stack[i_state.v_stack.len - 1])
#define OP_OBJ (op_objs[op_arg])

typedef enum _InstResult {
    INST_FAILED = -1,
    INST_SUCCESS = 0,
    INST_NEW_FUNC = 1
} OpResult;

static void complete_function(usize final_stack_size);
static bool type_check(Nst_Obj *obj, Nst_Obj *type);

static inline void destroy_call(Nst_FuncCall *call);
static inline void unwind_error(usize final_stack_size, Nst_Span span);

static bool push_func(Nst_Obj *func, Nst_Span span, usize arg_num,
                      Nst_Obj **args, Nst_VarTable *vt);
static Nst_Obj *make_cwd(const char *file_path);
static Nst_Bytecode *compile_file(Nst_CLArgs *args);
static bool push_module(const char *filename);

static inline Nst_Obj *pop_val(void);
static inline void pop_and_destroy(void);
static inline bool push_val(Nst_Obj *obj);

static OpResult exe_pop_val(void);
static OpResult exe_for_start(void);
static OpResult exe_for_next(void);
static OpResult exe_return_val(void);
static OpResult exe_return_vars(void);
static OpResult exe_set_val_loc(void);
static OpResult exe_set_cont_loc(void);
static OpResult exe_throw_err(void);
static OpResult exe_pop_catch(void);
static OpResult exe_set_val(void);
static OpResult exe_get_val(void);
static OpResult exe_push_val(void);
static OpResult exe_set_cont_val(void);
static OpResult exe_op_call(void);
static OpResult exe_op_seq_call(void);
static OpResult exe_op_cast(void);
static OpResult exe_op_range(void);
static OpResult exe_stack_op(void);
static OpResult exe_local_op(void);
static OpResult exe_op_import(void);
static OpResult exe_op_extract(void);
static OpResult exe_dec_int(void);
static OpResult exe_new_int(void);
static OpResult exe_dup(void);
static OpResult exe_rot2(void);
static OpResult exe_rot3(void);
static OpResult exe_make_arr(void);
static OpResult exe_make_arr_rep(void);
static OpResult exe_make_vec(void);
static OpResult exe_make_vec_rep(void);
static OpResult exe_make_map(void);
static OpResult exe_make_func(void);
static OpResult exe_save_error(void);
static OpResult exe_unpack_seq(void);
static OpResult exe_jump(void);
static OpResult exe_jumpif_t(void);
static OpResult exe_jumpif_f(void);
static OpResult exe_jumpif_zero(void);
static OpResult exe_jumpif_iend(void);
static OpResult exe_push_catch(void);

static OpResult (*inst_func[])(void) = {
    [Nst_OP_POP_VAL]      = exe_pop_val,
    [Nst_OP_FOR_START]    = exe_for_start,
    [Nst_OP_FOR_NEXT]     = exe_for_next,
    [Nst_OP_RETURN_VAL]   = exe_return_val,
    [Nst_OP_RETURN_VARS]  = exe_return_vars,
    [Nst_OP_SET_VAL_LOC]  = exe_set_val_loc,
    [Nst_OP_SET_CONT_LOC] = exe_set_cont_loc,
    [Nst_OP_THROW_ERR]    = exe_throw_err,
    [Nst_OP_POP_CATCH]    = exe_pop_catch,
    [Nst_OP_SET_VAL]      = exe_set_val,
    [Nst_OP_GET_VAL]      = exe_get_val,
    [Nst_OP_PUSH_VAL]     = exe_push_val,
    [Nst_OP_SET_CONT_VAL] = exe_set_cont_val,
    [Nst_OP_CALL]         = exe_op_call,
    [Nst_OP_SEQ_CALL]     = exe_op_seq_call,
    [Nst_OP_CAST]         = exe_op_cast,
    [Nst_OP_RANGE]        = exe_op_range,
    [Nst_OP_STACK]        = exe_stack_op,
    [Nst_OP_LOCAL]        = exe_local_op,
    [Nst_OP_IMPORT]       = exe_op_import,
    [Nst_OP_EXTRACT]      = exe_op_extract,
    [Nst_OP_DEC_INT]      = exe_dec_int,
    [Nst_OP_NEW_INT]      = exe_new_int,
    [Nst_OP_DUP]          = exe_dup,
    [Nst_OP_ROT_2]        = exe_rot2,
    [Nst_OP_ROT_3]        = exe_rot3,
    [Nst_OP_MAKE_ARR]     = exe_make_arr,
    [Nst_OP_MAKE_ARR_REP] = exe_make_arr_rep,
    [Nst_OP_MAKE_VEC]     = exe_make_vec,
    [Nst_OP_MAKE_VEC_REP] = exe_make_vec_rep,
    [Nst_OP_MAKE_MAP]     = exe_make_map,
    [Nst_OP_MAKE_FUNC]    = exe_make_func,
    [Nst_OP_SAVE_ERROR]   = exe_save_error,
    [Nst_OP_UNPACK_SEQ]   = exe_unpack_seq,
    [Nst_OP_EXTEND_ARG]   = NULL,
    [Nst_OP_JUMP]         = exe_jump,
    [Nst_OP_JUMPIF_T]     = exe_jumpif_t,
    [Nst_OP_JUMPIF_F]     = exe_jumpif_f,
    [Nst_OP_JUMPIF_ZERO]  = exe_jumpif_zero,
    [Nst_OP_JUMPIF_IEND]  = exe_jumpif_iend,
    [Nst_OP_PUSH_CATCH]   = exe_push_catch,
};

static Nst_Obj *(*stack_op_func[])(Nst_Obj *, Nst_Obj *) = {
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

static Nst_Obj *(*local_op_func[])(Nst_Obj *) = {
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

static volatile bool interrupt = false;

Nst_InterpreterState i_state;
bool extend_arg;
u64 op_arg;
Nst_Bytecode *bc;
Nst_Obj **op_objs;

static bool state_init = false;

bool Nst_init(void)
{
    if (state_init)
        return true;

    Nst_error_set_color(Nst_supports_color());
    _Nst_error_init();

    if (!_Nst_globals_init())
        goto cleanup;

    if (!_Nst_source_loader_init())
        goto cleanup;

    _Nst_ggc_init();
    _Nst_import_init();

    // Initialize the internal state to allow for a correct cleanup at any
    // error that might occur during initialization
    i_state.v_stack.stack = NULL;
    i_state.f_stack.stack = NULL;
    i_state.c_stack.stack = NULL;
    i_state.func = NULL;
    i_state.vt.vars = NULL;
    i_state.vt.global_table = NULL;
    i_state.idx = -1;
    i_state.prog = NULL;
    op_arg = 0;
    extend_arg = false;
    op_objs = NULL;
    bc = NULL;

    if (!Nst_vstack_init(&i_state.v_stack))
        goto cleanup;
    if (!Nst_fstack_init(&i_state.f_stack))
        goto cleanup;
    if (!Nst_cstack_init(&i_state.c_stack))
        goto cleanup;

    state_init = true;

    return true;
cleanup:
    if (Nst_error_occurred())
        Nst_error_print();

    Nst_quit();
    return false;
}

void Nst_quit(void)
{
    if (!state_init)
        return;

    state_init = false;

    Nst_error_clear();

    Nst_vstack_destroy(&i_state.v_stack);
    Nst_fstack_destroy(&i_state.f_stack);
    Nst_cstack_destroy(&i_state.c_stack);
    Nst_ndec_ref(i_state.func);
    Nst_vt_destroy(&i_state.vt);
    i_state.idx = 0;
    i_state.prog = NULL;
    op_arg = 0;
    extend_arg = false;
    op_objs = NULL;
    bc = NULL;

    _Nst_import_quit();
    _Nst_source_loader_quit();
    _Nst_ggc_quit();
    _Nst_globals_quit();
    _Nst_import_close_libs();
}

bool Nst_was_init(void)
{
    return state_init;
}

static void interrupt_handler(int sig)
{
    Nst_UNUSED(sig);
    // if the process is stuck in a loop outside of complete_function this
    // allows the user to send Ctrl-C twice to force exit
    if (interrupt)
        exit(1);
    else
        interrupt = true;
}

i32 Nst_run(Nst_Program *prog)
{
    _Nst_import_clear_paths();
    Nst_error_clear();

    i_state.prog = prog;
    if (prog->main_func == NULL) {
        Nst_error_setc_value("Nst_run: invalid program");
        return 1;
    }

    Nst_assert(prog->main_func->type == Nst_t.Func);

    if (prog->source_path != NULL) {
        if (!_Nst_import_push_path(Nst_inc_ref(prog->source_path)))
            return 1;
    }

    Nst_VarTable prog_vt = {
        .vars = Nst_ninc_ref(Nst_func_mod_globals(prog->main_func)),
        .global_table = NULL
    };
    if (!push_func(
            prog->main_func,
            Nst_span_empty(),
            0, NULL,
            prog_vt.vars == NULL ? NULL : &prog_vt))
    {
        _Nst_import_pop_path();
        return 1;
    }

    signal(SIGINT, interrupt_handler);
    complete_function(0);
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

Nst_Span Nst_state_span(void)
{
    if (i_state.idx < 0 || i_state.func == NULL)
        return Nst_span_empty();
    Nst_Bytecode *func_bc = Nst_func_nest_body(i_state.func);
    if ((usize)i_state.idx >= func_bc->len)
        return Nst_span_empty();
    return func_bc->positions[i_state.idx];
}

const Nst_InterpreterState *Nst_state(void)
{
    return (const Nst_InterpreterState *)&i_state;
}

static bool push_func(Nst_Obj *func, Nst_Span span, usize arg_num,
                      Nst_Obj **args, Nst_VarTable *vt)
{
    usize func_arg_num = Nst_func_arg_num(func);
    if (func_arg_num < arg_num) {
        Nst_error_set_call(_Nst_WRONG_ARG_NUM(func_arg_num, arg_num));
        return false;
    }

    bool success;
    Nst_VarTable new_vt;

    if (vt != NULL) {
        new_vt = *vt;
    } else {
        Nst_Obj *func_globals = Nst_func_mod_globals(func);
        if (func_globals != NULL)
            success = Nst_vt_init(&new_vt, func_globals, NULL, false);
        else if (i_state.vt.global_table == NULL)
            success = Nst_vt_init(&new_vt, i_state.vt.vars, NULL, false);
        else {
            success = Nst_vt_init(
                &new_vt,
                i_state.vt.global_table,
                NULL, false);
        }

        if (!success)
            return false;

        // add the outer variables if needed
        Nst_Obj *outer_vars = Nst_func_outer_vars(func);
        if (vt == NULL && outer_vars != NULL) {
            Nst_Obj *key;
            Nst_Obj *val;
            for (isize i = Nst_map_next(-1, outer_vars, &key, &val);
                 i != -1;
                 i = Nst_map_next(i, outer_vars, &key, &val))
            {
                if (!Nst_vt_set(new_vt, key, val)) {
                    Nst_vt_destroy(&new_vt);
                    return false;
                }
            }
        }

        // add the given arguments
        Nst_Obj **func_args = Nst_func_args(func);
        if (vt == NULL && args != NULL) {
            for (usize i = 0; i < arg_num; i++) {
                if (!Nst_vt_set(new_vt, func_args[i], args[i])) {
                    Nst_vt_destroy(&new_vt);
                    return false;
                }
            }
        } else if (vt == NULL) {
            for (usize i = 0; i < arg_num; i++) {
                Nst_Obj *arg = pop_val();
                if (!Nst_vt_set(new_vt, func_args[arg_num - i - 1], arg)) {
                    Nst_dec_ref(arg);
                    Nst_vt_destroy(&new_vt);
                    return false;
                }
                Nst_dec_ref(arg);
            }
        }

        // fill the remaining ones with `null`
        for (usize i = arg_num; i < func_arg_num; i++) {
            if (!Nst_vt_set(new_vt, func_args[i], Nst_null())) {
                Nst_vt_destroy(&new_vt);
                return false;
            }
        }

        if (!push_val(NULL)) {
            Nst_vt_destroy(&new_vt);
            return false;
        }
    }

    if (i_state.func != NULL) {
        Nst_FuncCall call = {
            .func = i_state.func,
            .cwd = Nst_getcwd(),
            .span = span,
            .vt = i_state.vt,
            .idx = i_state.idx,
            .cstack_len = i_state.c_stack.len
        };

        if (!Nst_fstack_push(&i_state.f_stack, call)) {
            Nst_vt_destroy(&new_vt);
            Nst_dec_ref(call.cwd);
            pop_and_destroy();
            return false;
        }
    }

    i_state.func = Nst_inc_ref(func);
    i_state.vt = new_vt;
    i_state.idx = 0;
    op_arg = 0;
    extend_arg = false;
    op_objs = NULL;
    bc = NULL;

    return true;
}

static Nst_Obj *make_cwd(const char *file_path)
{
    char *path = NULL;
    char *file_part = NULL;

    Nst_get_full_path(file_path, &path, &file_part);
    if (path == NULL) {
        Nst_error_clear();
        return NULL;
    }

    *(file_part - 1) = 0;
    Nst_Obj *str = Nst_str_new((u8 *)path, file_part - path - 1, true);
    if (str == NULL) {
        Nst_error_clear();
        Nst_free(path);
        return NULL;
    }
    return str;
}

static Nst_Bytecode *compile_file(Nst_CLArgs *args)
{
    Nst_SourceText *src = Nst_source_load(args);
    if (src == NULL)
        return NULL;

    Nst_DynArray tokens = Nst_tokenize(src);
    if (tokens.len == 0)
        return NULL;

    Nst_Node *ast = Nst_parse(&tokens);
    Nst_da_clear(&tokens, (Nst_Destructor)Nst_tok_destroy);
    if (ast != NULL && args->opt_level >= 1)
        ast = Nst_optimize_ast(ast);

    if (ast == NULL)
        return NULL;

    Nst_InstList inst_ls = Nst_compile(ast, true);
    Nst_node_destroy(ast);

    if (Nst_ilist_len(&inst_ls) == 0)
        return NULL;

    if (args->opt_level >= 2) {
        Nst_optimize_ilist(
            &inst_ls,
            args->opt_level == 3 && !args->no_default);
    }

    Nst_Bytecode *file_bc = Nst_assemble(&inst_ls);
    Nst_ilist_destroy(&inst_ls);

    return file_bc;
}

static bool push_module(const char *filename)
{
    Nst_Obj *mod_func = NULL;
    Nst_Obj *path_str = NULL;

    Nst_CLArgs args;
    Nst_cl_args_init(&args, 0, NULL);
    args.filename = (char *)filename;

    Nst_Bytecode *module_bc = compile_file(&args);
    if (module_bc == NULL)
        return false;

    mod_func = _Nst_func_new(NULL, 0, module_bc);
    if (mod_func == NULL) {
        Nst_bc_destroy(module_bc);
        return false;
    }

    Nst_VarTable mod_vt;
    bool success = Nst_vt_init(
        &mod_vt, NULL,
        i_state.prog->argv,
        args.no_default);
    if (!success) {
        Nst_dec_ref(mod_func);
        return false;
    }
    _Nst_func_set_mod_globals(mod_func, mod_vt.vars);

    path_str = make_cwd(filename);
    if (path_str == NULL) {
        Nst_dec_ref(mod_func);
        return false;
    }
    i32 chdir_result = Nst_chdir(path_str);
    Nst_dec_ref(path_str);
    if (chdir_result != 0) {
        Nst_dec_ref(mod_func);
        return false;
    }

    if (!push_func(mod_func, Nst_span_empty(), 0, NULL, &mod_vt)) {
        Nst_dec_ref(mod_func);
        return false;
    }

    Nst_dec_ref(mod_func);
    return true;
}

static void complete_function(usize final_stack_size)
{
    if (i_state.f_stack.len < final_stack_size)
        return;

    bc = Nst_func_nest_body(i_state.func);
    usize bc_len = bc->len;
    Nst_Op *ops = bc->bytecode;
    op_objs = bc->objects;

    while (i_state.f_stack.len >= final_stack_size) {
        if (i_state.idx >= (isize)bc_len) {
            if (i_state.f_stack.len == 0)
                return;

            // Free the function call if there is one
            Nst_FuncCall call = Nst_fstack_pop(&i_state.f_stack);
            destroy_call(&call);
            if (i_state.f_stack.len < final_stack_size)
                return;
            i_state.idx++;
            bc = Nst_func_nest_body(i_state.func);
            bc_len = bc->len;
            ops = bc->bytecode;
            op_objs = bc->objects;
        }

        Nst_Op op = ops[i_state.idx];
        op_arg = ((op_arg<<8) * extend_arg)
                       | Nst_OP_ARG(op);
        if (Nst_OP_CODE(op) == Nst_OP_EXTEND_ARG) {
            extend_arg = true;
            i_state.idx++;
            continue;
        }
        extend_arg = false;
        OpResult result = inst_func[Nst_OP_CODE(op)]();

        if (interrupt) {
            interrupt = false;
            Nst_error_set(Nst_inc_ref(Nst_s.e_Interrupt), Nst_null_ref());
            result = INST_FAILED;
        }

        Nst_ggc_collect();

        if (result == INST_SUCCESS) {
            i_state.idx++;
            continue;
        } else if (result == INST_FAILED) {
            bc = Nst_func_nest_body(i_state.func);
            unwind_error(final_stack_size, bc->positions[i_state.idx]);
            if (i_state.f_stack.len <= final_stack_size)
                return;
        }
        bc = Nst_func_nest_body(i_state.func);
        bc_len = bc->len;
        ops = bc->bytecode;
        op_objs = bc->objects;
    }
}

static inline void destroy_call(Nst_FuncCall *call)
{
    while (i_state.c_stack.len > call->cstack_len)
        Nst_cstack_pop(&i_state.c_stack);

    Nst_vt_destroy(&i_state.vt);
    Nst_dec_ref(i_state.func);
    i_state.func = call->func;
    i_state.vt = call->vt;
    i_state.idx = call->idx;
    if (call->cwd != NULL) {
        Nst_chdir(call->cwd);
        Nst_dec_ref(call->cwd);
    }
}

static inline void unwind_error(usize final_stack_size, Nst_Span span)
{
    Nst_error_add_span(span);

    Nst_CatchFrame top_catch = Nst_cstack_peek(&i_state.c_stack);
    if (Nst_error_get()->error_name == Nst_c.Null_null
        || Nst_error_get()->error_msg == Nst_c.Null_null)
    {
        top_catch.f_stack_len = 0;
        top_catch.v_stack_len = 0;
        top_catch.idx = -1;
    }

    usize end_size = top_catch.f_stack_len;
    if (end_size < final_stack_size)
        end_size = final_stack_size;

    while (i_state.f_stack.len > end_size) {
        Nst_FuncCall call = Nst_fstack_pop(&i_state.f_stack);
        Nst_error_add_span(call.span);

        destroy_call(&call);
        Nst_Obj *obj = pop_val();

        while (obj != NULL) {
            Nst_dec_ref(obj);
            obj = pop_val();
        }
    }

    if (end_size == final_stack_size)
        return;

    while (i_state.v_stack.len > top_catch.v_stack_len)
        Nst_ndec_ref(pop_val());
    i_state.idx = top_catch.idx;
}

static inline Nst_Obj *pop_val(void)
{
    if (i_state.v_stack.len == 0)
        return NULL;

    Nst_Obj *val = i_state.v_stack.stack[--i_state.v_stack.len];

    if (i_state.v_stack.cap <= _Nst_V_STACK_MIN_SIZE)
        return val;

    if (i_state.v_stack.cap >> 2 < i_state.v_stack.len)
        return val;

    Nst_assert(i_state.v_stack.len <= i_state.v_stack.cap);

    i_state.v_stack.stack = Nst_realloc(
        i_state.v_stack.stack,
        (i_state.v_stack.cap >> 1),
        sizeof(Nst_Obj *),
        i_state.v_stack.cap);

    i_state.v_stack.cap >>= 1;

    return val;
}

static inline void pop_and_destroy(void)
{
    Nst_ndec_ref(pop_val());
}

static inline bool push_val(Nst_Obj *obj)
{
    if (i_state.v_stack.len >= i_state.v_stack.cap) {
        void *new_stack = Nst_realloc(
            i_state.v_stack.stack,
            i_state.v_stack.cap * 2,
            sizeof(Nst_Obj *),
            0);

        if (new_stack == NULL)
            return false;

        i_state.v_stack.stack = new_stack;
        i_state.v_stack.cap *= 2;
    }

    i_state.v_stack.stack[i_state.v_stack.len++] = Nst_ninc_ref(obj);
    return true;
}

Nst_Obj *Nst_run_module(const char *filename)
{
    if (!push_module(filename))
        return false;

    complete_function(i_state.f_stack.len - 1);

    if (Nst_error_occurred())
        return NULL;
    else
        return pop_val();
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

    bool result = push_func(func, Nst_span_empty(), arg_num, args, NULL);
    if (!result)
        return NULL;

    complete_function(i_state.f_stack.len - 1);

    if (Nst_error_occurred())
        return NULL;

    return pop_val();
}

Nst_Obj *Nst_coroutine_yield(Nst_ObjRef **out_stack, usize *out_stack_size,
                             i64 *out_idx, Nst_VarTable *out_vt)
{
    usize stack_size = 0;
    for (i64 i = (i64)i_state.v_stack.len - 1; i >= 0; i--) {
        if (i_state.v_stack.stack[i] == NULL)
            break;
        stack_size++;
    }
    *out_stack_size = stack_size;

    if (out_stack == NULL)
        return NULL;

    for (usize i = 0; i < stack_size; i++)
        out_stack[stack_size - i - 1] = pop_val();
    // remove the NULL of the function call
    pop_val();

    *out_idx = i_state.idx;
    out_vt->vars = Nst_ninc_ref(i_state.vt.vars);
    out_vt->global_table = Nst_ninc_ref(i_state.vt.global_table);

    i_state.idx = bc->len;
    return i_state.func;
}

Nst_ObjRef *Nst_coroutine_resume(Nst_Obj *func, i64 idx,
                                 Nst_ObjRef **value_stack,
                                 usize value_stack_len, Nst_VarTable vt)
{
    Nst_assert(func->type == Nst_t.Func);
    Nst_assert(!Nst_FUNC_IS_C(func));

    if (!push_func(func, Nst_span_empty(), 0, NULL, &vt))
        return NULL;
    i_state.idx = idx;

    for (usize i = 0; i < value_stack_len; i++) {
        if (!push_val(value_stack[i]))
            return NULL;
    }

    complete_function(i_state.f_stack.len - 1);

    if (Nst_error_occurred())
        return NULL;

    return pop_val();
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

static OpResult exe_pop_val(void)
{
    CHECK_V_STACK(1);
    pop_and_destroy();
    return INST_SUCCESS;
}

static OpResult exe_for_inst(Nst_Obj *iter, Nst_Obj *func)
{
    Nst_assert(iter->type == Nst_t.Iter);
    if (Nst_FUNC_IS_C(func)) {
        Nst_Obj *iter_value = Nst_iter_value(iter);
        Nst_Obj *res = Nst_func_c_body(func)(1, &iter_value);
        if (res == NULL || !push_val(res)) {
            Nst_ndec_ref(res);
            return INST_FAILED;
        }
        Nst_dec_ref(res);
        return INST_SUCCESS;
    }
    if (!push_val(Nst_iter_value(iter)) || !push_val(func))
        return INST_FAILED;
    return exe_op_call();
}

static OpResult exe_for_start(void)
{
    CHECK_V_STACK(1);
    Nst_Obj *iterable = FAST_TOP;
    Nst_Obj *iter = Nst_obj_cast(iterable, Nst_t.Iter);
    if (iter == NULL) {
        Nst_error_setf_type(
            "invalid type cast from '%s' to 'Iter'",
            Nst_type_name(iterable->type).value);
        return INST_FAILED;
    }
    FAST_TOP = iter;
    Nst_dec_ref(iterable);
    return exe_for_inst(iter, Nst_iter_start_func(iter));
}

static OpResult exe_for_next(void)
{
    CHECK_V_STACK(1);
    Nst_Obj *iter = FAST_TOP;
    return exe_for_inst(iter, Nst_iter_next_func(iter));
}

static OpResult exe_jumpif_iend(void)
{
    CHECK_V_STACK(2);
    Nst_Obj *iter_val = FAST_TOP;
    if (iter_val == Nst_c.IEnd_iend) {
        pop_and_destroy();
        i_state.idx = op_arg - 1;
    }
    return INST_SUCCESS;
}

static OpResult exe_return_val(void)
{
    CHECK_V_STACK(1);

    Nst_Obj *result = pop_val();
    Nst_Obj *obj = pop_val();

    while (obj != NULL) {
        Nst_dec_ref(obj);
        obj = pop_val();
    }
    if (!push_val(result)) {
        Nst_dec_ref(result);
        return INST_FAILED;
    }
    i_state.idx = bc->len;
    Nst_dec_ref(result);
    return INST_SUCCESS;
}

static OpResult exe_return_vars(void)
{
    Nst_Obj *vars = i_state.vt.vars;
    Nst_Obj *obj = pop_val();

    while (obj != NULL) {
        Nst_dec_ref(obj);
        obj = pop_val();
    }

    if (!push_val(vars))
        return INST_FAILED;
    i_state.idx = bc->len;
    return INST_SUCCESS;
}

static OpResult exe_set_val_loc(void)
{
    CHECK_V_STACK(1);
    Nst_Obj *val = pop_val();
    bool res = Nst_vt_set(i_state.vt, OP_OBJ, val);
    Nst_dec_ref(val);
    return res ? INST_SUCCESS : INST_FAILED;
}

static OpResult exe_set_cont_loc(void)
{
    i32 res = exe_set_cont_val();
    if (res == INST_FAILED)
        return INST_FAILED;

    CHECK_V_STACK(1);
    pop_and_destroy();
    return res;
}

static OpResult exe_jump(void)
{
    i_state.idx = op_arg - 1;
    return INST_SUCCESS;
}

static OpResult exe_jumpif_t(void)
{
    CHECK_V_STACK(1);
    Nst_Obj *top_val = pop_val();
    if (Nst_obj_to_bool(top_val))
        i_state.idx = op_arg - 1;
    Nst_dec_ref(top_val);
    return INST_SUCCESS;
}

static OpResult exe_jumpif_f(void)
{
    CHECK_V_STACK(1);
    Nst_Obj *top_val = pop_val();
    if (!Nst_obj_to_bool(top_val))
        i_state.idx = op_arg - 1;

    Nst_dec_ref(top_val);
    return INST_SUCCESS;
}

static OpResult exe_jumpif_zero(void)
{
    CHECK_V_STACK(1);
    if (Nst_int_i64(FAST_TOP) == 0)
        i_state.idx = op_arg - 1;

    return INST_SUCCESS;
}

static OpResult exe_set_val(void)
{
    CHECK_V_STACK(1);
    if (Nst_vt_set(i_state.vt, OP_OBJ, FAST_TOP))
        return INST_SUCCESS;
    else
        return INST_FAILED;
}

static OpResult exe_get_val(void)
{
    Nst_Obj *obj = Nst_vt_get(i_state.vt, OP_OBJ);
    bool res;
    if (obj == NULL)
        res = push_val(Nst_c.Null_null);
    else {
        res = push_val(obj);
        Nst_dec_ref(obj);
    }
    return res ? INST_SUCCESS : INST_FAILED;
}

static OpResult exe_push_val(void)
{
    return push_val(OP_OBJ) ? INST_SUCCESS : INST_FAILED;
}

static OpResult exe_set_cont_val(void)
{
    CHECK_V_STACK(3);
    Nst_Obj *idx = pop_val();
    Nst_Obj *cont = pop_val();
    Nst_Obj *val = FAST_TOP;
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

static OpResult call_c_func(bool is_seq_call, usize arg_num, Nst_Obj *args_seq,
                            Nst_Obj *func)
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
            stack_args[i] = pop_val();
        args = stack_args;
    } else {
        args = Nst_malloc_c(tot_args, Nst_Obj *);
        if (args == NULL) {
            Nst_dec_ref(func);
            return INST_FAILED;
        }
        for (i64 i = (i64)arg_num - 1; i >= 0; i--)
            args[i] = pop_val();
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
    }

    if (!push_val(res)) {
        Nst_dec_ref(res);
        Nst_dec_ref(func);
        return INST_FAILED;
    }

    Nst_dec_ref(res);
    Nst_dec_ref(func);
    return INST_SUCCESS;
}

static OpResult exe_op_call(void)
{
    CHECK_V_STACK(op_arg + 1);

    u64 arg_num = op_arg;
    Nst_Obj *func = pop_val();

    if (!type_check(func, Nst_t.Func)) {
        Nst_dec_ref(func);
        return INST_FAILED;
    }

    if (Nst_FUNC_IS_C(func))
        return call_c_func(false, arg_num, NULL, func);

    bool result = push_func(func, Nst_state_span(), arg_num, NULL, NULL);
    return result ? INST_NEW_FUNC : INST_FAILED;
}

static OpResult exe_op_seq_call(void)
{
    CHECK_V_STACK(2);

    Nst_Obj *func = pop_val();

    if (!type_check(func, Nst_t.Func)) {
        Nst_dec_ref(func);
        return INST_FAILED;
    }

    Nst_Obj *args_seq = pop_val();

    if (args_seq->type != Nst_t.Array && args_seq->type != Nst_t.Vector) {
        Nst_error_setf_type(
            "expected type 'Array' or 'Vector', got '%s' instead",
            Nst_type_name(args_seq->type).value);

        Nst_dec_ref(args_seq);
        Nst_dec_ref(func);
        return INST_FAILED;
    }

    usize arg_num = Nst_seq_len(args_seq);
    if (Nst_FUNC_IS_C(func))
        return call_c_func(true, arg_num, args_seq, func);
    bool result = push_func(
        func,
        Nst_state_span(),
        arg_num,
        _Nst_seq_objs(args_seq),
        NULL);
    Nst_dec_ref(args_seq);
    return result ? INST_NEW_FUNC : INST_FAILED;
}

static OpResult exe_op_cast(void)
{
    CHECK_V_STACK(2);
    Nst_Obj *val = pop_val();
    Nst_Obj *type = pop_val();

    if (!type_check(type, Nst_t.Type)) {
        Nst_dec_ref(val);
        Nst_dec_ref(type);
        return INST_FAILED;
    }

    Nst_Obj *res = Nst_obj_cast(val, type);
    Nst_dec_ref(val);
    Nst_dec_ref(type);

    if (res == NULL || !push_val(res)) {
        Nst_ndec_ref(res);
        return INST_FAILED;
    }
    Nst_dec_ref(res);
    return INST_SUCCESS;
}

static OpResult exe_op_range(void)
{
    CHECK_V_STACK(op_arg);
    Nst_Obj *stop = pop_val();
    Nst_Obj *step = NULL;
    Nst_Obj *start = NULL;

    if (!type_check(stop, Nst_t.Int)) {
        Nst_dec_ref(stop);
        return INST_FAILED;
    }

    if (op_arg == 3) {
        start = pop_val();
        step  = pop_val();

        if (!type_check(start, Nst_t.Int) || !type_check(step, Nst_t.Int)) {
            Nst_dec_ref(stop);
            Nst_dec_ref(step);
            Nst_dec_ref(start);
            return INST_FAILED;
        }
    } else {
        start = pop_val();

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

    if (iter == NULL || !push_val(iter)) {
        Nst_ndec_ref(iter);
        return INST_FAILED;
    }
    Nst_dec_ref(iter);
    return INST_SUCCESS;
}

static OpResult exe_throw_err(void)
{
    CHECK_V_STACK(2);
    Nst_Obj *message = pop_val();
    Nst_Obj *name = pop_val();

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

static OpResult exe_stack_op(void)
{
    CHECK_V_STACK(2);
    Nst_Obj *ob2 = pop_val();
    Nst_Obj *ob1 = pop_val();
    Nst_Obj *res = stack_op_func[op_arg](ob1, ob2);
    Nst_dec_ref(ob1);
    Nst_dec_ref(ob2);

    if (res == NULL || !push_val(res)) {
        Nst_ndec_ref(res);
        return INST_FAILED;
    }
    Nst_dec_ref(res);
    return INST_SUCCESS;
}

static OpResult exe_local_op(void)
{
    CHECK_V_STACK(1);
    Nst_Obj *obj = pop_val();
    Nst_Obj *res = local_op_func[op_arg - Nst_TT_LEN](obj);
    Nst_dec_ref(obj);

    if (res == NULL || !push_val(res)) {
        Nst_ndec_ref(res);
        return INST_FAILED;
    }
    Nst_dec_ref(res);
    return INST_SUCCESS;
}

static OpResult exe_op_import(void)
{
    CHECK_V_STACK(1);
    Nst_Obj *name = pop_val();

    if (!type_check(name, Nst_t.Str)) {
        Nst_dec_ref(name);
        return INST_FAILED;
    }

    Nst_Obj *res = Nst_obj_import(name);
    Nst_dec_ref(name);

    if (res == NULL || !push_val(res)) {
        Nst_ndec_ref(res);
        return INST_FAILED;
    }
    Nst_dec_ref(res);
    return INST_SUCCESS;
}

static OpResult exe_op_extract(void)
{
    CHECK_V_STACK(2);
    Nst_Obj *idx = pop_val();
    Nst_Obj *cont = pop_val();
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

        if (res == NULL || !push_val(res))
            return_value = INST_FAILED;
    } else if (cont->type == Nst_t.Map) {
        res = Nst_map_get(cont, idx);

        if (res == NULL && idx->hash == -1) {
            Nst_error_setf_type(
                "type '%s' is not hashable",
                Nst_type_name(idx->type).value);
            return_value = INST_FAILED;
        } else if (res == NULL && !push_val(Nst_c.Null_null))
            return_value = INST_FAILED;
        else if (!push_val(res))
            return_value = INST_FAILED;
    } else if (cont->type == Nst_t.Str) {
        if (idx->type != Nst_t.Int) {
            Nst_error_setf_type(
                "expected type 'Int', got '%s' instead",
                Nst_type_name(idx->type).value);

            return_value = INST_FAILED;
            goto end;
        }

        res = Nst_str_get_obj(cont, Nst_int_i64(idx));

        if (res == NULL || !push_val(res))
            return_value = INST_FAILED;
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

static OpResult exe_dec_int(void)
{
    CHECK_V_STACK(1);
    _Nst_counter_dec(FAST_TOP);
    return INST_SUCCESS;
}

static OpResult exe_new_int(void)
{
    CHECK_V_STACK(1);
    Nst_Obj *obj = pop_val();
    if (!type_check(obj, Nst_t.Int)) {
        Nst_dec_ref(obj);
        return INST_FAILED;
    }

    Nst_Obj *new_obj = Nst_int_new(Nst_int_i64(obj));
    Nst_dec_ref(obj);
    if (new_obj == NULL || !push_val(new_obj)) {
        Nst_ndec_ref(new_obj);
        return INST_FAILED;
    }
    Nst_dec_ref(new_obj);
    return INST_SUCCESS;
}

static OpResult exe_dup(void)
{
    CHECK_V_STACK(1);
    return push_val(FAST_TOP) ? INST_SUCCESS : INST_FAILED;
}

static OpResult exe_rot2(void)
{
    CHECK_V_STACK(2);

    Nst_Obj *tmp = FAST_TOP;
    FAST_TOP = i_state.v_stack.stack[i_state.v_stack.len - 2];
    i_state.v_stack.stack[i_state.v_stack.len - 2] = tmp;
    return INST_SUCCESS;
}

static OpResult exe_rot3(void)
{
    CHECK_V_STACK(3);

    #define FAST_PEEK(n) (i_state.v_stack.stack[i_state.v_stack.len - (n)])

    Nst_Obj *tmp = FAST_TOP;
    FAST_TOP = FAST_PEEK(2);
    FAST_PEEK(2) = FAST_PEEK(3);
    FAST_PEEK(3) = tmp;

    #undef FAST_PEEK

    return INST_SUCCESS;
}

static OpResult complete_seq(Nst_Obj *seq)
{
    if (seq == NULL)
        return INST_FAILED;

    Nst_Obj **objs = _Nst_seq_objs(seq);
    for (u64 i = 1; i <= op_arg; i++)
        objs[op_arg - i] = pop_val();

    if (!push_val(seq)) {
        Nst_dec_ref(seq);
        return INST_FAILED;
    }
    Nst_dec_ref(seq);
    return INST_SUCCESS;
}

static OpResult exe_make_arr(void)
{
    CHECK_V_STACK(op_arg);
    return complete_seq(_Nst_array_new_empty((usize)op_arg));
}

static OpResult exe_make_vec(void)
{
    CHECK_V_STACK(op_arg);
    return complete_seq(_Nst_vector_new_empty((usize)op_arg));
}

static i32 complete_seq_rep(Nst_Obj *seq, usize size)
{
    if (seq == NULL)
        return INST_FAILED;

    Nst_Obj *val = pop_val();
    Nst_Obj **objs = _Nst_seq_objs(seq);
    for (u64 i = 0; i < size; i++)
        objs[i] = pop_val();
    Nst_dec_ref(val);

    if (!push_val(seq)) {
        Nst_dec_ref(seq);
        return INST_FAILED;
    }
    Nst_dec_ref(seq);
    return INST_SUCCESS;
}

static OpResult exe_make_arr_rep(void)
{
    CHECK_V_STACK(2);
    Nst_Obj *size_obj = pop_val();

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
    Nst_Obj *seq = _Nst_array_new_empty((usize)size);
    return complete_seq_rep(seq, (usize)size);
}

static OpResult exe_make_vec_rep(void)
{
    CHECK_V_STACK(2);
    Nst_Obj *size_obj = pop_val();

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
    Nst_Obj *seq = _Nst_vector_new_empty((usize)size);
    return complete_seq_rep(seq, (usize)size);
}

static OpResult exe_make_map(void)
{
    CHECK_V_STACK(op_arg * 2);
    u64 map_size = op_arg;
    Nst_Obj *map = Nst_map_new();
    if (map == NULL)
        return INST_FAILED;

    usize stack_size = i_state.v_stack.len;
    Nst_Obj **v_stack = i_state.v_stack.stack;

    for (u64 i = 0; i < map_size; i++) {
        Nst_Obj *key = v_stack[stack_size - map_size * 2 + i];
        Nst_Obj *val = v_stack[stack_size - map_size * 2 + i + 1];
        if (!Nst_map_set(map, key, val)) {
            Nst_dec_ref(val);
            Nst_dec_ref(key);
            Nst_dec_ref(map);
            return INST_FAILED;
        }
        Nst_dec_ref(val);
        Nst_dec_ref(key);
    }
    i_state.v_stack.len -= (usize)map_size;
    if (!push_val(map)) {
        Nst_dec_ref(map);
        return INST_FAILED;
    }
    Nst_dec_ref(map);
    return INST_SUCCESS;
}

static OpResult exe_make_func(void)
{
    Nst_Obj *func = OP_OBJ;
    if (i_state.vt.global_table == NULL) {
        _Nst_func_set_mod_globals(func, i_state.vt.vars);
        return push_val(func) ? INST_SUCCESS : INST_FAILED;
    }

    _Nst_func_set_mod_globals(func, i_state.vt.global_table);
    Nst_Obj *vars_copy = Nst_map_copy(i_state.vt.vars);
    if (vars_copy == NULL)
        return INST_FAILED;

    Nst_ndec_ref(Nst_map_drop(vars_copy, Nst_s.o__vars_));
    Nst_ndec_ref(Nst_map_drop(vars_copy, Nst_s.o__globals_));
    Nst_Obj *new_func = _Nst_func_new_outer_vars(func, vars_copy);
    Nst_dec_ref(vars_copy);
    if (new_func == NULL || !push_val(new_func)) {
        Nst_ndec_ref(new_func);
        return INST_FAILED;
    }

    return INST_SUCCESS;
}

static OpResult exe_push_catch(void)
{
    Nst_CatchFrame cf = {
        .idx = (i64)op_arg,
        .v_stack_len = i_state.v_stack.len,
        .f_stack_len = i_state.f_stack.len
    };
    Nst_cstack_push(&i_state.c_stack, cf);
    return INST_SUCCESS;
}

static OpResult exe_pop_catch(void)
{
    Nst_cstack_pop(&i_state.c_stack);
    return INST_SUCCESS;
}

static OpResult exe_save_error(void)
{
    Nst_assert(Nst_error_occurred());

    Nst_Obj *err_map = Nst_map_new();
    Nst_Traceback *tb = Nst_error_get();
    Nst_map_set_str(err_map, "name", tb->error_name);
    Nst_map_set_str(err_map, "message", tb->error_msg);
    Nst_error_clear();

    push_val(err_map);
    Nst_dec_ref(err_map);

    return INST_SUCCESS;
}

static OpResult exe_unpack_seq(void)
{
    CHECK_V_STACK(1);
    Nst_Obj *seq = pop_val();

    if (seq->type != Nst_t.Array && seq->type != Nst_t.Vector) {
        Nst_error_setf_type(
            "expected type 'Array' or 'Vector', got '%s' instead",
            Nst_type_name(seq->type).value);
        Nst_dec_ref(seq);
        return INST_FAILED;
    }

    if (Nst_seq_len(seq) != op_arg) {
        Nst_error_setf_value(
            "expected %"PRIu64" items to unpack but the sequence contains %zi",
            op_arg, Nst_seq_len(seq));
        Nst_dec_ref(seq);
        return INST_FAILED;
    }

    for (usize i = 0, n = Nst_seq_len(seq); i < n; i++) {
        if (!push_val(Nst_seq_getnf(seq, n - i - 1))) {
            Nst_dec_ref(seq);
            return INST_FAILED;
        }
    }
    Nst_dec_ref(seq);
    return INST_SUCCESS;
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
    return res != 0 ? -1 : 0;
}

Nst_ObjRef *Nst_getcwd(void)
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
