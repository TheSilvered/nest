#include <assert.h>
#include <errno.h>
#include "error_internal.h"
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

#define ERROR_OCCURRED (Nst_state.traceback.error.occurred)
#define GLOBAL_ERROR (&(Nst_state.traceback.error))

#define CHECK_V_STACK assert(Nst_state.v_stack.current_size != 0)
#define CHECK_V_STACK_SIZE(size) \
    assert((Nst_Int)(Nst_state.v_stack.current_size) >= size)

Nst_ExecutionState Nst_state;

static void complete_function(usize final_stack_size);

static i32 exe_no_op(Nst_Inst *inst);
static i32 exe_pop_val(Nst_Inst *inst);
static i32 exe_for_start(Nst_Inst *inst);
static i32 exe_for_is_done(Nst_Inst *inst);
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
static i32 exe_new_obj(Nst_Inst *inst);
static i32 exe_dup(Nst_Inst *inst);
static i32 exe_rot(Nst_Inst *inst);
static i32 exe_make_seq(Nst_Inst *inst);
static i32 exe_make_seq_rep(Nst_Inst *inst);
static i32 exe_make_map(Nst_Inst *inst);
static i32 exe_push_catch(Nst_Inst *inst);
static i32 exe_pop_catch(Nst_Inst *inst);
static i32 exe_save_error(Nst_Inst *inst);
static i32 exe_unpack_seq(Nst_Inst *inst);

static Nst_SeqObj *make_argv(i32 argc, i8 **argv, i8 *filename);
static Nst_StrObj *make_cwd(i8 *file_path);
static inline void add_positions(Nst_Pos start, Nst_Pos end);
static inline void change_vt(Nst_VarTable *new_vt);
static void loaded_libs_destructor(C_LIB_TYPE lib);

i32 (*inst_func[])(Nst_Inst *) = {
    exe_no_op, exe_pop_val, exe_for_start, exe_for_is_done, exe_for_get_val,
    exe_return_val, exe_return_vars, exe_set_val_loc, exe_set_cont_loc,
    exe_jump, exe_jumpif_t, exe_jumpif_f, exe_jumpif_zero, exe_push_catch,
    exe_type_check, exe_hash_check, exe_throw_err, exe_pop_catch, exe_set_val,
    exe_get_val, exe_push_val, exe_set_cont_val, exe_op_call, exe_op_cast,
    exe_op_range, exe_stack_op, exe_local_op, exe_op_import, exe_op_extract,
    exe_dec_int, exe_new_obj, exe_dup, exe_rot, exe_make_seq,exe_make_seq_rep,
    exe_make_seq, exe_make_seq_rep, exe_make_map, exe_save_error,
    exe_unpack_seq
};

Nst_Obj *(*stack_op_func[])(Nst_Obj *, Nst_Obj *) = {
    _Nst_obj_add, _Nst_obj_sub, _Nst_obj_mul, _Nst_obj_div, _Nst_obj_pow,
    _Nst_obj_mod, _Nst_obj_bwand, _Nst_obj_bwor, _Nst_obj_bwxor, _Nst_obj_bwls,
    _Nst_obj_bwrs, _Nst_obj_concat, _Nst_obj_lgand, _Nst_obj_lgor,
    _Nst_obj_lgxor, _Nst_obj_gt, _Nst_obj_lt, _Nst_obj_eq, _Nst_obj_ne,
    _Nst_obj_ge, _Nst_obj_le
};

Nst_Obj *(*local_op_func[])(Nst_Obj *) = {
    _Nst_obj_len, _Nst_obj_lgnot, _Nst_obj_bwnot, _Nst_obj_stdout,
    _Nst_obj_stdin, NULL, /* import */ NULL, /* loc_call */ _Nst_obj_neg,
    _Nst_obj_typeof
};

i32 Nst_run(Nst_FuncObj *main_func, i32 argc, i8 **argv, i8 *filename,
            i32 opt_level, bool no_default)
{
    // Init global state
    bool state_init = Nst_state_init(
        argc, argv,
        filename,
        opt_level,
        no_default);

    if (!state_init) {
        fprintf(stderr, "Failed allocation\n");
        return -1;
    }

    // Execute main function
    Nst_fstack_push(
        main_func,
        Nst_no_pos(),
        Nst_no_pos(),
        NULL,
        0, 0);

    Nst_func_set_vt(main_func, Nst_state.vt->vars);
    Nst_ggc_track_obj(GGC_OBJ(Nst_state.vt->vars));

    complete_function(0);
    Nst_dec_ref(main_func);

    // Check for errors
    int exit_code = 0;
    if (ERROR_OCCURRED) {
        // When the error name is of type Null it signals an exit call
        if (OBJ(Nst_state.traceback.error.name) != Nst_c.Null_null) {
            Nst_print_traceback(Nst_state.traceback);
            exit_code = 1;
        } else
            exit_code = (int)AS_INT(Nst_state.traceback.error.message);
    }

    Nst_state_free();
    return exit_code;
}

bool Nst_state_init(i32 argc, i8 **argv, i8 *filename, i32 opt_level,
                    bool no_default)
{
    if (!Nst_traceback_init()) {
        return false;
    }
    Nst_ggc_init();
    Nst_state.curr_path = Nst_getcwd();
    if (Nst_state.curr_path == NULL) {
        Nst_error_clear();
        Nst_state_free();
        return false;
    }
    Nst_state.argv = make_argv(
        argc, argv,
        filename == NULL ? (i8 *)"-c" : filename);

    if (Nst_state.argv == NULL) {
        Nst_traceback_delete();
        Nst_state_free();
        return false;
    }

    Nst_state.vt = Nst_vt_new(
        NULL,
        Nst_state.curr_path,
        Nst_state.argv,
        no_default);

    if (Nst_state.vt == NULL) {
        Nst_error_clear();
        Nst_state_free();
        return false;
    }

    Nst_state.idx = 0;
    Nst_state.opt_level = opt_level;
    Nst_vstack_init();
    Nst_fstack_init();
    Nst_cstack_init();
    Nst_state.loaded_libs = Nst_llist_new();
    Nst_state.lib_paths = Nst_llist_new();
    Nst_state.lib_handles = MAP(Nst_map_new());
    Nst_state.lib_srcs = Nst_llist_new();

    if (Nst_error_occurred()) {
        Nst_error_clear();
        Nst_state_free();
        return false;
    }

    if (filename != NULL) {
        i8 *path_main_file;
        usize path_len = Nst_get_full_path(filename, &path_main_file, NULL);
        if (path_main_file == NULL) {
            Nst_error_clear();
            Nst_state_free();
            return false;
        }
        Nst_Obj *path_str = Nst_string_new(path_main_file, path_len, true);
        if (path_str == NULL) {
            Nst_error_clear();
            Nst_free(path_main_file);
            Nst_state_free();
            return false;
        }
        if (!Nst_llist_append(Nst_state.lib_paths, path_str, true)) {
            Nst_error_clear();
            Nst_dec_ref(path_str);
            Nst_state_free();
            return false;
        }
    }

    return true;
}

void Nst_state_free(void)
{
    Nst_traceback_delete();
    Nst_ndec_ref(Nst_state.curr_path);
    Nst_ndec_ref(Nst_state.argv);
    Nst_ndec_ref(Nst_state.lib_handles);
    Nst_vstack_destroy();
    Nst_fstack_destroy();
    Nst_cstack_destroy();
    if (Nst_state.lib_paths != NULL) {
        Nst_llist_destroy(
            Nst_state.lib_paths,
            (Nst_LListDestructor)_Nst_dec_ref);
    }
    if (Nst_state.lib_srcs != NULL) {
        Nst_llist_destroy(
            Nst_state.lib_srcs,
            (Nst_LListDestructor)Nst_free_src_text);
    }
    if (Nst_state.loaded_libs == NULL)
        goto free_ggc;

    for (Nst_LLIST_ITER(lib, Nst_state.loaded_libs)) {
        void (*free_lib_func)() = (void (*)())dlsym(lib->value, "free_lib");
        if (free_lib_func != NULL)
            free_lib_func();
    }
free_ggc:
    Nst_ggc_delete_objs();
}

void _Nst_unload_libs(void)
{
    if (Nst_state.loaded_libs != NULL) {
        Nst_llist_destroy(
            Nst_state.loaded_libs,
            (Nst_LListDestructor)loaded_libs_destructor);
    }
}

static inline void destroy_call(Nst_FuncCall *call, Nst_Int offset)
{
    while (Nst_state.c_stack.current_size > call->cstack_size)
        Nst_cstack_pop();

    Nst_state.idx = call->idx + offset;
    if (Nst_state.vt == call->vt)
        return;

    Nst_vt_destroy(Nst_state.vt);
    Nst_dec_ref(call->func);
    Nst_state.vt = call->vt;
}

static inline void set_global_error(usize final_stack_size, Nst_Inst *inst)
{
    if (Nst_state.traceback.error.occurred)
        add_positions(inst->start, inst->end);
    else
        _Nst_SET_ERROR_FROM_OP_ERR(GLOBAL_ERROR, inst->start, inst->end);

    Nst_CatchFrame top_catch = Nst_cstack_peek();
    if (OBJ(Nst_state.traceback.error.name) == Nst_c.Null_null) {
        top_catch.f_stack_size = 0;
        top_catch.v_stack_size = 0;
        top_catch.inst_idx = -1;
    }

    Nst_Obj *obj;

    usize end_size = top_catch.f_stack_size;
    if (end_size < final_stack_size) {
        end_size = final_stack_size;
    }

    while (Nst_state.f_stack.current_size > end_size) {
        Nst_FuncCall call = Nst_fstack_pop();
        destroy_call(&call, 1);
        obj = Nst_vstack_pop();

        while (obj != NULL) {
            Nst_dec_ref(obj);
            obj = Nst_vstack_pop();
        }

        add_positions(call.start, call.end);
    }

    if (end_size == final_stack_size)
        return;

    while (Nst_state.v_stack.current_size > top_catch.v_stack_size) {
        obj = Nst_vstack_pop();
        if (obj != NULL)
            Nst_dec_ref(obj);
    }
    Nst_state.idx = top_catch.inst_idx - 1;
}

static void complete_function(usize final_stack_size)
{
    if (Nst_state.f_stack.current_size == 0)
        return;

    Nst_InstList *curr_inst_ls = Nst_fstack_peek().func->body.bytecode;
    Nst_Inst *instructions = curr_inst_ls->instructions;

    for (; Nst_state.f_stack.current_size > final_stack_size;
         Nst_state.idx++)
    {
        if (Nst_state.idx >= (Nst_Int)curr_inst_ls->total_size) {
            // Free the function call
            Nst_FuncCall call = Nst_fstack_pop();
            destroy_call(&call, 0);
            if (Nst_state.f_stack.current_size == 0)
                return;
            curr_inst_ls = Nst_fstack_peek().func->body.bytecode;
            instructions = curr_inst_ls->instructions;
            continue;
        }

        Nst_Inst *inst = &instructions[Nst_state.idx];
        Nst_InstID inst_id = inst->id;
        i32 result = inst_func[inst_id](inst);

        if (result == 0)
            continue;
        else if (result == -1) {
            set_global_error(final_stack_size, inst);
            if (Nst_state.f_stack.current_size == final_stack_size)
                return;
        }
        curr_inst_ls = Nst_fstack_peek().func->body.bytecode;
        instructions = curr_inst_ls->instructions;
    }
}

i32 Nst_run_module(i8 *filename, Nst_SourceText *lib_src)
{
    // Compile and optimize the imported module
    i32 opt_level = Nst_state.opt_level;

    i32 file_opt_lvl;
    bool no_default;

    // The file is guaranteed to exist
    Nst_LList *tokens = Nst_tokenizef(
        filename,
        Nst_CP_UNKNOWN,
        &file_opt_lvl,
        &no_default,
        lib_src,
        GLOBAL_ERROR);

    if (file_opt_lvl < opt_level)
        opt_level = file_opt_lvl;

    if (tokens == NULL)
        return -1;

    Nst_Node *ast = Nst_parse(tokens, GLOBAL_ERROR);
    if (ast != NULL && opt_level >= 1)
        ast = Nst_optimize_ast(ast, GLOBAL_ERROR);

    if (ast == NULL)
        return -1;

    Nst_InstList *inst_ls = Nst_compile(ast, true, GLOBAL_ERROR);
    if (opt_level >= 2 && inst_ls != NULL) {
        inst_ls = Nst_optimize_bytecode(
            inst_ls,
            opt_level == 3 && !no_default,
            GLOBAL_ERROR);
    }
    if (inst_ls == NULL)
        return -1;

    Nst_FuncObj *mod_func = FUNC(Nst_func_new(0, inst_ls));
    if (mod_func == NULL) {
        Nst_inst_list_destroy(inst_ls);
        inst_ls = Nst_fstack_peek().func->body.bytecode;
        _Nst_SET_ERROR_FROM_OP_ERR(
            GLOBAL_ERROR,
            inst_ls->instructions[Nst_state.idx].start,
            inst_ls->instructions[Nst_state.idx].end);
        return -1;
    }

    // Change the cwd
    Nst_StrObj *prev_path = Nst_state.curr_path;

    Nst_StrObj *path_str = make_cwd(filename);
    if (path_str == NULL) {
        Nst_dec_ref(mod_func);
        inst_ls = Nst_fstack_peek().func->body.bytecode;
        _Nst_SET_ERROR_FROM_OP_ERR(
            GLOBAL_ERROR,
            inst_ls->instructions[Nst_state.idx].start,
            inst_ls->instructions[Nst_state.idx].end);
        return -1;
    }
    Nst_state.curr_path = path_str;

    if (Nst_chdir(path_str) != 0) {
        Nst_dec_ref(mod_func);
        Nst_dec_ref(path_str);
        Nst_state.curr_path = prev_path;
        inst_ls = Nst_fstack_peek().func->body.bytecode;
        _Nst_SET_ERROR_FROM_OP_ERR(
            GLOBAL_ERROR,
            inst_ls->instructions[Nst_state.idx].start,
            inst_ls->instructions[Nst_state.idx].end);
        return -1;
    }

    Nst_vstack_push(NULL);
    Nst_fstack_push(
        mod_func,
        Nst_no_pos(),
        Nst_no_pos(),
        Nst_state.vt,
        Nst_state.idx - 1,
        Nst_state.c_stack.current_size);
    Nst_state.idx = 0;
    Nst_VarTable *vt = Nst_vt_new(NULL, path_str, Nst_state.argv, no_default);
    if (vt == NULL) {
        Nst_dec_ref(mod_func);
        Nst_dec_ref(Nst_state.curr_path);
        Nst_state.curr_path = prev_path;
        inst_ls = Nst_fstack_peek().func->body.bytecode;
        _Nst_SET_ERROR_FROM_OP_ERR(
            GLOBAL_ERROR,
            inst_ls->instructions[Nst_state.idx].start,
            inst_ls->instructions[Nst_state.idx].end);
        return -1;
    }

    change_vt(vt);

    Nst_func_set_vt(mod_func, Nst_state.vt->vars);

    complete_function(Nst_state.f_stack.current_size - 1);
    Nst_state.curr_path = prev_path;
    Nst_dec_ref(path_str);
    Nst_dec_ref(mod_func);

    if (Nst_chdir(prev_path) != 0) {
        inst_ls = Nst_fstack_peek().func->body.bytecode;
        _Nst_SET_ERROR_FROM_OP_ERR(
            GLOBAL_ERROR,
            inst_ls->instructions[Nst_state.idx].start,
            inst_ls->instructions[Nst_state.idx].end);
        return -1;
    }

    if (ERROR_OCCURRED)
        return -1;
    else
        return 0;
}

Nst_Obj *Nst_call_func(Nst_FuncObj *func, Nst_Obj **args)
{
    if (Nst_FLAG_HAS(func, Nst_FLAG_FUNC_IS_C))
        return func->body.c_func(func->arg_num, args);

    Nst_vstack_push(NULL);
    Nst_fstack_push(
        func,
        Nst_no_pos(),
        Nst_no_pos(),
        Nst_state.vt,
        Nst_state.idx - 1,
        Nst_state.c_stack.current_size);

    Nst_VarTable *new_vt = Nst_vt_from_func(func);
    if (new_vt == NULL)
        return NULL;

    for (usize i = 0, n = func->arg_num; i < n; i++) {
        if (!Nst_vt_set(new_vt, func->args[i], args[i])) {
            Nst_dec_ref(new_vt->global_table);
            Nst_map_drop(new_vt->vars, Nst_s.o__vars_);
            Nst_dec_ref(new_vt->vars);
            Nst_free(new_vt);
            return NULL;
        }
    }

    Nst_state.idx = 0;
    change_vt(new_vt);
    complete_function(Nst_state.f_stack.current_size - 1);

    if (ERROR_OCCURRED)
        return NULL;

    return Nst_vstack_pop();
}

Nst_Obj *Nst_run_func_context(Nst_FuncObj *func, Nst_Int idx, Nst_MapObj *vars,
                              Nst_MapObj *globals)
{
    assert(!Nst_FLAG_HAS(func, Nst_FLAG_FUNC_IS_C));

    Nst_fstack_push(
        func,
        Nst_no_pos(),
        Nst_no_pos(),
        Nst_state.vt,
        Nst_state.idx - 1,
        Nst_state.c_stack.current_size);

    Nst_VarTable *new_vt = Nst_malloc_c(1, Nst_VarTable);
    if (new_vt == NULL) {
        _Nst_SET_ERROR_FROM_OP_ERR(
            GLOBAL_ERROR,
            func->body.bytecode->instructions[0].start,
            func->body.bytecode->instructions[0].end);
        return NULL;
    }

    new_vt->vars = MAP(Nst_inc_ref(vars));

    if (globals == NULL) {
        if (func->mod_globals != NULL)
            new_vt->global_table = MAP(Nst_inc_ref(func->mod_globals));
        else if (Nst_state.vt->global_table == NULL)
            new_vt->global_table = MAP(Nst_inc_ref(Nst_state.vt->vars));
        else
            new_vt->global_table = MAP(Nst_inc_ref(Nst_state.vt->global_table));
    } else
        new_vt->global_table = MAP(Nst_inc_ref(globals));
    change_vt(new_vt);
    Nst_state.idx = idx;
    complete_function(Nst_state.f_stack.current_size - 1);

    if (ERROR_OCCURRED)
        return NULL;

    return Nst_vstack_pop();
}

static i32 exe_no_op(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    return 0;
}

static i32 exe_pop_val(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_pop();
    Nst_dec_ref(obj);
    return 0;
}

static i32 exe_for_inst(Nst_Inst *inst, Nst_IterObj *iter,
                        Nst_FuncObj *func)
{
    if (Nst_FLAG_HAS(func, Nst_FLAG_FUNC_IS_C)) {
        Nst_Obj *res = func->body.c_func(
            (usize)inst->int_val,
            &iter->value);

        if (res == NULL)
            return -1;
        Nst_vstack_push(res);
        Nst_dec_ref(res);
        return 0;
    }
    Nst_vstack_push(iter->value);
    Nst_vstack_push(func);
    exe_op_call(inst);
    return 1;
}

static i32 exe_for_start(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(Nst_vstack_peek());
    return exe_for_inst(inst, iter, iter->start);
}

static i32 exe_for_is_done(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(Nst_vstack_peek());
    return exe_for_inst(inst, iter, iter->is_done);
}

static i32 exe_for_get_val(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(Nst_vstack_peek());
    return exe_for_inst(inst, iter, iter->get_val);
}

static i32 exe_return_val(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    Nst_Obj *result = Nst_vstack_pop();
    Nst_Obj *obj = Nst_vstack_pop();

    while (obj != NULL) {
        Nst_dec_ref(obj);
        obj = Nst_vstack_pop();
    }

    Nst_vstack_push(result);
    Nst_state.idx = Nst_fstack_peek().func->body.bytecode->total_size;
    Nst_dec_ref(result);
    return 0;
}

static i32 exe_return_vars(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    Nst_MapObj *vars = Nst_state.vt->vars;
    Nst_Obj *obj = Nst_vstack_pop();

    while (obj != NULL) {
        Nst_dec_ref(obj);
        obj = Nst_vstack_pop();
    }

    Nst_vstack_push(vars);
    Nst_state.idx = Nst_fstack_peek().func->body.bytecode->total_size;
    return 0;
}

static i32 exe_set_val_loc(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *val = Nst_vstack_pop();
    i32 res = Nst_vt_set(Nst_state.vt, inst->val, val);
    Nst_dec_ref(val);
    return res;
}

static i32 exe_set_cont_loc(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    i32 res = exe_set_cont_val(inst);
    CHECK_V_STACK;
    Nst_vstack_pop();
    return res;
}

static i32 exe_jump(Nst_Inst *inst)
{
    Nst_state.idx = inst->int_val - 1;
    return 0;
}

static i32 exe_jumpif_t(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = Nst_vstack_pop();
    if (Nst_obj_to_bool(top_val)) {
        Nst_state.idx = inst->int_val - 1;
    }
    Nst_dec_ref(top_val);
    return 0;
}

static i32 exe_jumpif_f(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = Nst_vstack_pop();
    if (!Nst_obj_to_bool(top_val)) {
        Nst_state.idx = inst->int_val - 1;
    }
    Nst_dec_ref(top_val);
    return 0;
}

static i32 exe_jumpif_zero(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = Nst_vstack_peek();
    if (AS_INT(val) == 0) {
        Nst_state.idx = inst->int_val - 1;
    }
    return 0;
}

static i32 exe_type_check(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_peek();
    if (obj->type != TYPE(inst->val)) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_EXPECTED_TYPES,
            STR(inst->val)->value,
            TYPE_NAME(obj)));
        return -1;
    }
    return 0;
}

static i32 exe_hash_check(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_peek();
    Nst_obj_hash(obj);
    if (obj->hash == -1) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_UNHASHABLE_TYPE,
            TYPE_NAME(obj)));
        return -1;
    }
    return 0;
}

static i32 exe_set_val(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_vt_set(
        Nst_state.vt,
        inst->val,
        Nst_vstack_peek());
    return Nst_error_occurred() ? -1 : 0;
}

static i32 exe_get_val(Nst_Inst *inst)
{
    Nst_Obj *obj = Nst_vt_get(Nst_state.vt, inst->val);
    if (obj == NULL)
        Nst_vstack_push(Nst_c.Null_null);
    else {
        Nst_vstack_push(obj);
        Nst_dec_ref(obj);
    }
    return 0;
}

static i32 exe_push_val(Nst_Inst *inst)
{
    Nst_vstack_push(inst->val);
    return 0;
}

static i32 exe_set_cont_val(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK_SIZE(3);
    Nst_Obj *idx = Nst_vstack_pop();
    Nst_Obj *cont = Nst_vstack_pop();
    Nst_Obj *val = Nst_vstack_peek();
    i32 return_value = 0;

    if (cont->type == Nst_t.Array || cont->type == Nst_t.Vector) {
        if (idx->type != Nst_t.Int) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_EXPECTED_TYPE("Int"),
                TYPE_NAME(idx)));

            return_value = -1;
            goto end;
        }

        if (!Nst_seq_set(cont, AS_INT(idx), val))
            return_value = -1;
        goto end;
    } else if (cont->type == Nst_t.Map) {
        if (!Nst_map_set(cont, idx, val))
            return_value = -1;
        goto end;
    }

    Nst_set_type_error(Nst_sprintf(
        _Nst_EM_EXPECTED_TYPE("Array', 'Vector', or 'Map"),
        TYPE_NAME(cont)));
    return_value = -1;

end:
    Nst_dec_ref(cont);
    Nst_dec_ref(idx);
    Nst_dec_ref(val);
    return return_value;
}

static i32 call_c_func(bool         is_seq_call,
                       Nst_Int      tot_args,
                       Nst_Int      arg_num,
                       Nst_Int      null_args,
                       Nst_SeqObj  *args_seq,
                       Nst_FuncObj *func)
{
    Nst_Obj **args;
    Nst_Obj *stack_args[10]; // for up to 10 arguments this array is used
    bool args_allocated = false;

    if (is_seq_call && null_args == 0)
        args = args_seq->objs;
    else if (is_seq_call && tot_args <= 10) {
        memcpy(
            stack_args,
            args_seq->objs,
            (usize)arg_num * sizeof(Nst_Obj *));
        args = stack_args;
    } else if (is_seq_call) {
        args = Nst_malloc_c((usize)tot_args, Nst_Obj *);
        if (args == NULL) {
            Nst_dec_ref(args_seq);
            Nst_dec_ref(func);
            return -1;
        }

        memcpy(
            stack_args,
            args_seq->objs,
            (usize)arg_num * sizeof(Nst_Obj *));
        args = stack_args;
    } else if (tot_args == 0)
        args = NULL;
    else if (tot_args <= 10) {
        for (Nst_Int i = arg_num - 1; i >= 0; i--)
            stack_args[i] = Nst_vstack_pop();
        args = stack_args;
    } else {
        args = Nst_malloc_c((usize)tot_args, Nst_Obj *);
        if (args == NULL) {
            Nst_dec_ref(func);
            return -1;
        }

        for (Nst_Int i = arg_num - 1; i >= 0; i--)
            args[i] = Nst_vstack_pop();
        args_allocated = true;
    }

    for (Nst_Int i = 0; i < null_args; i++)
        args[arg_num + i] = Nst_inc_ref(Nst_c.Null_null);

    Nst_Obj *res = func->body.c_func((usize)tot_args, args);

    if (!is_seq_call) {
        for (Nst_Int i = 0; i < tot_args; i++)
            Nst_dec_ref(args[i]);
    } else {
        for (Nst_Int i = 0; i < null_args; i++)
            Nst_dec_ref(args[arg_num + i]);
    }

    if (args_allocated)
        Nst_free(args);

    if (res == NULL) {
        Nst_dec_ref(func);
        return -1;
    } else {
        Nst_vstack_push(res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(func);
    return 0;
}

static i32 exe_op_call(Nst_Inst *inst)
{
#ifdef _DEBUG
    if (inst->int_val == -1)
        CHECK_V_STACK_SIZE(2);
    else
        CHECK_V_STACK_SIZE(inst->int_val + 1);
#endif // !_DEBUG

    Nst_Int arg_num = inst->int_val;
    bool is_seq_call = false;
    Nst_FuncObj *func = FUNC(Nst_vstack_pop());
    Nst_SeqObj *args_seq;

    if (arg_num == -1) {
        args_seq = SEQ(Nst_vstack_pop());
        if (args_seq->type != Nst_t.Array && args_seq->type != Nst_t.Vector) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_EXPECTED_TYPE("Array' or 'Vector"),
                TYPE_NAME(args_seq)));

            Nst_dec_ref(args_seq);
            Nst_dec_ref(func);
            return -1;
        }
        is_seq_call = true;
        arg_num = args_seq->len;
    } else
        args_seq = NULL;

    if ((Nst_Int)(func->arg_num) < arg_num) {
        Nst_set_call_error(Nst_sprintf(
            _Nst_EM_WRONG_ARG_NUM,
            func->arg_num, func->arg_num == 1 ? "" : "s",
            arg_num, arg_num == 1 ? "was" : "were"));

        Nst_dec_ref(func);
        Nst_ndec_ref(args_seq);
        return -1;
    }

    Nst_Int null_args = (Nst_Int)func->arg_num - arg_num;
    Nst_Int tot_args = func->arg_num;

    if (Nst_FLAG_HAS(func, Nst_FLAG_FUNC_IS_C)) {
        return call_c_func(
            is_seq_call,
            tot_args,
            arg_num,
            null_args,
            args_seq,
            func);
    }

    bool res = Nst_fstack_push(
        func,
        inst->start,
        inst->end,
        Nst_state.vt,
        Nst_state.idx,
        Nst_state.c_stack.current_size);
    Nst_state.idx = -1;
    Nst_dec_ref(func);

    if (!res) {
        Nst_set_call_error_c(_Nst_EM_CALL_STACK_SIZE_EXCEEDED);
        Nst_ndec_ref(args_seq);
        return -1;
    }

    Nst_VarTable *new_vt = Nst_vt_from_func(func);
    if (new_vt == NULL) {
        Nst_ndec_ref(args_seq);
        return -1;
    }

    change_vt(new_vt);

    for (Nst_Int i = 0; i < arg_num; i++) {
        Nst_Obj *val;
        if (is_seq_call)
            val = Nst_inc_ref(args_seq->objs[i]);
        else
            val = Nst_vstack_pop();

        Nst_vt_set(new_vt, func->args[is_seq_call ? i : arg_num - i - 1], val);
        Nst_dec_ref(val);
        if (Nst_error_occurred()) {
            Nst_ndec_ref(args_seq);
            return -1;
        }
    }
    Nst_ndec_ref(args_seq);

    for (Nst_Int i = arg_num; i < tot_args; i++) {
        Nst_vt_set(new_vt, func->args[i], Nst_c.Null_null);
        if (Nst_error_occurred())
            return -1;
    }

    Nst_vstack_push(NULL);
    return 1;
}

static i32 exe_op_cast(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *val = Nst_vstack_pop();
    Nst_Obj *type = Nst_vstack_pop();

    Nst_Obj *res = Nst_obj_cast(val, type);

    if (res != NULL) {
        Nst_vstack_push(res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(val);
    Nst_dec_ref(type);
    return res == NULL ? -1 : 0;
}

static i32 exe_op_range(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(inst->int_val);
    Nst_Obj *stop = Nst_vstack_pop();
    Nst_Obj *step = NULL;
    Nst_Obj *start = NULL;

    if (inst->int_val == 3) {
        start = Nst_vstack_pop();
        step  = Nst_vstack_pop();
    } else {
        start = Nst_vstack_pop();

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
        return -1;
    Nst_vstack_push(iter);
    Nst_dec_ref(iter);
    return 0;
}

static i32 exe_throw_err(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *message = Nst_vstack_pop();
    Nst_Obj *name = Nst_vstack_pop();

    Nst_set_error(name, message);
    return -1;
}

static i32 exe_stack_op(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *ob2 = Nst_vstack_pop();
    Nst_Obj *ob1 = Nst_vstack_pop();
    Nst_Obj *res = stack_op_func[inst->int_val](ob1, ob2);

    if (res != NULL) {
        Nst_vstack_push(res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(ob1);
    Nst_dec_ref(ob2);
    return res == NULL ? -1 : 0;
}

static i32 exe_local_op(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_pop();
    Nst_Obj *res = local_op_func[inst->int_val - Nst_TT_LEN](obj);

    if (res != NULL)
    {
        Nst_vstack_push(res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(obj);
    return res == NULL ? -1 : 0;
}

static i32 exe_op_import(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *name = Nst_vstack_pop();
    Nst_Obj *res = Nst_obj_import(name);

    if (res != NULL) {
        Nst_vstack_push(res);
        Nst_dec_ref(res);
    }

    Nst_dec_ref(name);
    return res == NULL ? -1 : 0;
}

static i32 exe_op_extract(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *idx = Nst_vstack_pop();
    Nst_Obj *cont = Nst_vstack_pop();
    Nst_Obj *res = NULL;
    i32 return_value = 0;

    if (cont->type == Nst_t.Array || cont->type == Nst_t.Vector) {
        if (idx->type != Nst_t.Int) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_EXPECTED_TYPE("Int"),
                TYPE_NAME(idx)));

            return_value = -1;
            goto end;
        }

        res = Nst_seq_get(cont, AS_INT(idx));

        if (res == NULL)
            return_value = -1;
        else
            Nst_vstack_push(res);
    } else if (cont->type == Nst_t.Map) {
        res = Nst_map_get(cont, idx);

        if (res != NULL)
            Nst_vstack_push(res);
        else if ( idx->hash != -1 )
            Nst_vstack_push(Nst_c.Null_null);
        else {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_UNHASHABLE_TYPE,
                TYPE_NAME(idx)));
            return_value = -1;
        }
    } else if (cont->type == Nst_t.Str) {
        if (idx->type != Nst_t.Int) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_EXPECTED_TYPE("Int"),
                TYPE_NAME(idx)));

            return_value = -1;
            goto end;
        }

        res = Nst_string_get(cont, AS_INT(idx));

        if (res == NULL)
            return_value = -1;
        else
            Nst_vstack_push(res);
    } else {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_EXPECTED_TYPE("Array', 'Vector', 'Map' or 'Str"),
            TYPE_NAME(cont)));
        return_value = -1;
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
    Nst_Obj *obj = Nst_vstack_peek();
    AS_INT(obj) -= 1;
    return 0;
}

static i32 exe_new_obj(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_Obj *obj = Nst_vstack_peek();
    Nst_Obj *new_obj = Nst_int_new(AS_INT(obj));
    if (Nst_error_occurred())
        return -1;
    Nst_vstack_push(new_obj);
    Nst_dec_ref(new_obj);
    return 0;
}

static i32 exe_dup(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    CHECK_V_STACK;
    Nst_vstack_push(Nst_vstack_peek());
    return 0;
}

static i32 exe_rot(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(inst->int_val);

    Nst_Obj *obj = Nst_vstack_peek();
    usize stack_size = Nst_state.v_stack.current_size - 1;
    Nst_Obj **stack = Nst_state.v_stack.stack;
    for (Nst_Int i = 1, n = inst->int_val; i < n; i++)
        stack[stack_size - i + 1] = stack[stack_size - i];

    stack[stack_size - inst->int_val + 1] = obj;
    return 0;
}

static i32 exe_make_seq(Nst_Inst *inst)
{
    Nst_Int seq_size = inst->int_val;
    Nst_Obj *seq = inst->id == Nst_IC_MAKE_ARR
        ? Nst_array_new((usize)seq_size)
        : Nst_vector_new((usize)seq_size);
    if (Nst_error_occurred())
        return -1;

    CHECK_V_STACK_SIZE(seq_size);

    for (Nst_Int i = 1; i <= seq_size; i++) {
        Nst_Obj *curr_obj = Nst_vstack_pop();
        Nst_seq_set(seq, seq_size - i, curr_obj);
        Nst_dec_ref(curr_obj);
    }

    Nst_vstack_push(seq);
    Nst_dec_ref(seq);
    return 0;
}

static i32 exe_make_seq_rep(Nst_Inst *inst)
{
    Nst_Int seq_size = inst->int_val;
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *size_obj = Nst_vstack_pop();
    Nst_Obj *val = Nst_vstack_pop();

    Nst_Int size = AS_INT(size_obj);
    Nst_dec_ref(size_obj);
    Nst_Obj *seq = inst->id == Nst_IC_MAKE_ARR_REP
        ? Nst_array_new((usize)size)
        : Nst_vector_new((usize)size);
    if (Nst_error_occurred())
        return -1;

    for (Nst_Int i = 1; i <= size; i++)
        Nst_seq_set(seq, seq_size - i, val);

    Nst_vstack_push(seq);
    Nst_dec_ref(seq);
    Nst_dec_ref(val);
    return 0;
}

static i32 exe_make_map(Nst_Inst *inst)
{
    Nst_Int map_size = inst->int_val;
    Nst_Obj *map = Nst_map_new();
    if (Nst_error_occurred())
        return -1;

    CHECK_V_STACK_SIZE(map_size);
    usize stack_size = Nst_state.v_stack.current_size;
    Nst_Obj **v_stack = Nst_state.v_stack.stack;

    for (Nst_Int i = 0; i < map_size; i++) {
        Nst_Obj *key = v_stack[stack_size - map_size + i];
        i++;
        Nst_Obj *val = v_stack[stack_size - map_size + i];
        if (!Nst_map_set(map, key, val)) {
            Nst_dec_ref(val);
            Nst_dec_ref(key);
            Nst_dec_ref(map);
            return -1;
        }
        Nst_dec_ref(val);
        Nst_dec_ref(key);
    }
    Nst_state.v_stack.current_size -= (usize)map_size;
    Nst_vstack_push(map);
    Nst_dec_ref(map);
    return 0;
}

static i32 exe_push_catch(Nst_Inst *inst)
{
    Nst_cstack_push(
        inst->int_val,
        Nst_state.v_stack.current_size,
        Nst_state.f_stack.current_size);
    return 0;
}

static i32 exe_pop_catch(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    Nst_cstack_pop();
    return 0;
}

static i32 exe_save_error(Nst_Inst *inst)
{
    Nst_UNUSED(inst);
    assert(GLOBAL_ERROR->occurred);

    Nst_Obj *err_map = Nst_map_new();
    Nst_map_set_str(err_map, "name", GLOBAL_ERROR->name);
    Nst_map_set_str(err_map, "message", GLOBAL_ERROR->message);
    Nst_error_clear();

    Nst_state.traceback.error.occurred = false;
    Nst_dec_ref(GLOBAL_ERROR->name);
    Nst_dec_ref(GLOBAL_ERROR->message);

    Nst_llist_empty(Nst_state.traceback.positions, Nst_free);

    Nst_vstack_push(err_map);
    Nst_dec_ref(err_map);

    // Remove the source of any libraries that failed to load
    while (Nst_state.lib_srcs->size > Nst_state.lib_handles->item_count) {
        Nst_SourceText *txt = (Nst_SourceText *)Nst_llist_pop(
            Nst_state.lib_srcs);
        Nst_free_src_text(txt);
    }
    return 0;
}

static i32 exe_unpack_seq(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_SeqObj *seq = SEQ(Nst_vstack_pop());

    if (seq->type != Nst_t.Array && seq->type != Nst_t.Vector) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_EXPECTED_TYPE("Array' or 'Vector"),
            TYPE_NAME(seq)));
        Nst_dec_ref(seq);
        return -1;
    }

    if ((Nst_Int)seq->len != inst->int_val) {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_WRONG_UNPACK_LENGTH,
            inst->int_val, seq->len));
        Nst_dec_ref(seq);
        return -1;
    }

    for (Nst_Int i = seq->len - 1; i >= 0; i--)
        Nst_vstack_push(seq->objs[i]);

    Nst_dec_ref(seq);
    return 0;
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

static Nst_SeqObj *make_argv(i32 argc, i8 **argv, i8 *filename)
{
    Nst_SeqObj *args = SEQ(Nst_array_new(argc + 1));
    if (args == NULL) {
        Nst_error_clear();
        return NULL;
    }

    Nst_Obj *val = Nst_string_new_c_raw(filename, false);
    if (val == NULL) {
        Nst_error_clear();
        args->len = 0;
        Nst_dec_ref(args);
        return NULL;
    }
    Nst_seq_set(args, 0, val);
    Nst_dec_ref(val);

    for (i32 i = 0; i < argc; i++) {
        val = Nst_string_new_c_raw(argv[i], false);
        if (val == NULL) {
            Nst_error_clear();
            args->len = i + 1;
            Nst_dec_ref(args);
            return NULL;
        }
        Nst_seq_set(args, i + 1, val);
        Nst_dec_ref(val);
    }

    return args;
}

static Nst_StrObj *make_cwd(i8 *file_path)
{
    i8 *path = NULL;
    i8 *file_part = NULL;

    Nst_get_full_path(file_path, &path, &file_part);
    if (path == NULL) {
        Nst_error_clear();
        return NULL;
    }

    *(file_part - 1) = 0;
    Nst_StrObj *str = STR(Nst_string_new(path, file_part - path - 1, true));
    if (str == NULL) {
        Nst_error_clear();
        Nst_free(file_path);
        return NULL;
    }
    return str;
}

Nst_ExecutionState *Nst_get_state(void)
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
    if (res != 0)
        Nst_set_call_error_c(_Nst_EM_FAILED_CHDIR);
    return res;
#else
    i32 res = chdir(str->value);
    if (res != 0)
        Nst_set_call_error_c(_Nst_EM_FAILED_CHDIR);
    return res != 0 ? -1 : 0;
#endif // !Nst_WIN
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
    Nst_Obj *return_str = Nst_string_new_c_raw((const i8 *)cwd_buf, true);
    if (return_str == NULL) {
        Nst_free(cwd_buf);
        return NULL;
    }
    return STR(return_str);
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
    Nst_Obj *return_str = Nst_string_new_c_raw((const i8 *)cwd_buf, true);
    if (return_str == NULL) {
        Nst_free(cwd_buf);
        return NULL;
    }
    return STR(return_str);
#endif // !Nst_WIN
}

static inline void add_positions(Nst_Pos start, Nst_Pos end)
{
    Nst_Pos *positions = Nst_raw_malloc(2 * sizeof(Nst_Pos));
    if (positions == NULL) {
        return;
    }
    Nst_OpErr prev_err = { NULL, NULL };
    if (Nst_error_occurred()) {
        prev_err.name    = STR(Nst_inc_ref(Nst_error_get()->name));
        prev_err.message = STR(Nst_inc_ref(Nst_error_get()->message));
    }

    positions[0] = start;
    positions[1] = end;
    bool result = Nst_llist_push(
        Nst_state.traceback.positions,
        positions + 1,
        false);
    if (!result) {
        Nst_error_clear();
        if (prev_err.name != NULL)
            Nst_set_error(prev_err.name, prev_err.message);
        Nst_free(positions);
        return;
    }
    result = Nst_llist_push(
        Nst_state.traceback.positions,
        positions,
        true);
    if (!result) {
        Nst_error_clear();
        if (prev_err.name != NULL)
            Nst_set_error(prev_err.name, prev_err.message);
        Nst_llist_pop(Nst_state.traceback.positions);
        Nst_free(positions);
    }
}

static inline void change_vt(Nst_VarTable *new_vt)
{
    Nst_state.vt = new_vt;
    Nst_ggc_track_obj(GGC_OBJ(Nst_state.vt->vars));
    if (Nst_state.vt->global_table != NULL)
        Nst_ggc_track_obj(GGC_OBJ(Nst_state.vt->global_table));
}

static void loaded_libs_destructor(C_LIB_TYPE lib)
{
    dlclose(lib);
}
