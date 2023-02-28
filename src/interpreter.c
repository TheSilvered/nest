#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "error_internal.h"
#include "obj_ops.h"
#include "hash.h"
#include "tokens.h"
#include "iter.h"
#include "optimizer.h"
#include "parser.h"
#include "lexer.h"

#if defined(_WIN32) || defined(WIN32)

#include <windows.h>
#include <direct.h>

#define dlclose FreeLibrary
#define dlsym(lib, name) GetProcAddress((HMODULE)lib, name)
#define PATH_MAX MAX_PATH

#define C_LIB_TYPE HMODULE

#else

#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#define _chdir chdir
#define _getcwd getcwd

#define C_LIB_TYPE void *

#endif

#define ADD_POSITIONS(start_pos, end_pos) do { \
        Nst_Pos *positions = (Nst_Pos *)malloc(sizeof(Nst_Pos) * 2); \
        if ( positions == NULL ) \
            break; \
        positions[0] = start_pos; \
        positions[1] = end_pos; \
        nst_llist_push(nst_state.traceback->positions, positions + 1, false); \
        nst_llist_push(nst_state.traceback->positions, positions,     true); \
    } while ( 0 )

#define SET_OP_ERROR(start_pos, end_pos, op_err) do { \
        if ( nst_state.traceback->error.occurred ) \
        { \
            ADD_POSITIONS(start_pos, end_pos);\
            break; \
        } \
        _NST_SET_ERROR( \
            GLOBAL_ERROR, \
            start_pos, \
            end_pos, \
            op_err.name, \
            op_err.message); \
        nst_dec_ref(op_err.name); \
        nst_dec_ref(op_err.message); \
    } while ( 0 )

#define CHANGE_VT(new_vt) do { \
    *nst_state.vt = new_vt; \
    nst_ggc_track_obj(GGC_OBJ((*nst_state.vt)->vars)); \
    if ( (*nst_state.vt)->global_table != NULL ) \
        nst_ggc_track_obj(GGC_OBJ((*nst_state.vt)->global_table)); \
    } while ( 0 )

#define ERROR_OCCURRED (nst_state.traceback->error.occurred)
#define GLOBAL_ERROR (&(nst_state.traceback->error))

#define CHECK_V_STACK assert(nst_state.v_stack->current_size != 0)
#define CHECK_V_STACK_SIZE(size) \
    assert((Nst_Int)(nst_state.v_stack->current_size) >= size)
#define CHECK_F_STACK assert(nst_state.f_stack->current_size != 0)

Nst_ExecutionState nst_state;

static void complete_function(usize final_stack_size);

static inline void run_instruction(Nst_Inst *inst);
static inline void exe_pop_val();
static inline void exe_for_start(Nst_Inst *inst);
static inline void exe_for_is_done(Nst_Inst *inst);
static inline void exe_for_get_val(Nst_Inst *inst);
static inline void exe_return_val();
static inline void exe_return_vars();
static inline void exe_set_val_loc(Nst_Inst *inst);
static inline void exe_set_cont_loc(Nst_Inst *inst);
static inline void exe_jump(Nst_Inst *inst);
static inline void exe_jumpif_t(Nst_Inst *inst);
static inline void exe_jumpif_f(Nst_Inst *inst);
static inline void exe_jumpif_zero(Nst_Inst *inst);
static inline void exe_type_check(Nst_Inst *inst);
static inline void exe_hash_check(Nst_Inst *inst);
static inline void exe_set_val(Nst_Inst *inst);
static inline void exe_get_val(Nst_Inst *inst);
static inline void exe_push_val(Nst_Inst *inst);
static inline void exe_set_cont_val(Nst_Inst *inst);
static inline void exe_op_call(Nst_Inst *inst);
static inline void exe_op_cast(Nst_Inst *inst);
static inline void exe_op_range(Nst_Inst *inst);
static inline void exe_throw_err(Nst_Inst *inst);
static inline void exe_stack_op(Nst_Inst *inst);
static inline void exe_local_op(Nst_Inst *inst);
static inline void exe_op_import(Nst_Inst *inst);
static inline void exe_op_extract(Nst_Inst *inst);
static inline void exe_dec_int();
static inline void exe_new_obj();
static inline void exe_dup();
static inline void exe_rot(Nst_Inst *inst);
static inline void exe_make_seq(Nst_Inst *inst);
static inline void exe_make_seq_rep(Nst_Inst *inst);
static inline void exe_make_map(Nst_Inst *inst);
static inline void exe_push_catch(Nst_Inst *inst);
static inline void exe_pop_catch();
static inline void exe_save_error();
static inline void exe_unpack_seq(Nst_Inst *inst);

static Nst_SeqObj *make_argv(i32 argc, i8 **argv, i8 *filename);
static Nst_StrObj *make_cwd(i8 *file_path);

i32 nst_run(Nst_FuncObj *main_func,
            i32          argc,
            i8         **argv,
            i8          *filename,
            i32          opt_level,
            bool         no_default)
{
    i8 *cwd_buf = (i8 *)malloc(sizeof(i8) * PATH_MAX);
    if ( cwd_buf == NULL )
    {
        return -1;
    }

    Nst_Traceback tb;
    tb.error.start = nst_no_pos();
    tb.error.end = nst_no_pos();
    tb.error.name = NULL;
    tb.error.message = NULL;
    tb.error.occurred = false;
    tb.positions = nst_llist_new();
    Nst_Int idx = 0;

    // Create the garbage collector
    Nst_GGCList gen1 = { NULL, NULL, 0 };
    Nst_GGCList gen2 = { NULL, NULL, 0 };
    Nst_GGCList gen3 = { NULL, NULL, 0 };
    Nst_GGCList old_gen = { NULL, NULL, 0 };
    Nst_GarbageCollector ggc = { gen1, gen2, gen3, old_gen, 0 };

    // make_argv creates a tracked object
    nst_state.ggc = &ggc;

    Nst_StrObj *cwd = STR(nst_string_new_c_raw(
        _getcwd(cwd_buf, PATH_MAX),
        true));
    Nst_SeqObj *argv_obj = make_argv(
        argc,
        argv,
        filename == NULL ? (i8 *)"-c" : filename);
    Nst_VarTable *vt = nst_vt_new(
        no_default ? MAP(nst_c.Null_null) : NULL,
        cwd,
        argv_obj);

    if ( no_default )
    {
        nst_dec_ref(nst_c.Null_null);
        vt->global_table = NULL;
        nst_dec_ref(nst_map_drop(vt->vars, nst_s.o__globals_));
    }

    nst_state.traceback = &tb;
    nst_state.vt = &vt;
    nst_state.idx = &idx;
    nst_state.curr_path = &cwd;
    nst_state.argv = argv_obj;
    nst_state.opt_level = &opt_level;
    nst_state.v_stack = nst_vstack_new();
    nst_state.f_stack = nst_fstack_new();
    nst_state.c_stack = nst_cstack_new();
    nst_state.loaded_libs = nst_llist_new();
    nst_state.lib_paths = nst_llist_new();
    nst_state.lib_handles = MAP(nst_map_new());
    nst_state.lib_srcs = nst_llist_new();

    if ( filename != NULL )
    {
        i8 *path_main_file = NULL;
        usize path_len = nst_get_full_path(filename, &path_main_file, NULL);

        nst_llist_append(
            nst_state.lib_paths,
            nst_string_new(path_main_file, path_len, true),
            true);
    }

    nst_fstack_push(
        nst_state.f_stack,
        main_func,
        nst_no_pos(),
        nst_no_pos(),
        NULL,
        0);

    nst_func_set_vt(main_func, vt->vars);
    nst_ggc_track_obj(GGC_OBJ(vt->vars));

    complete_function(0);

    nst_dec_ref(main_func);
    if ( ERROR_OCCURRED )
    {
        nst_print_traceback(*nst_state.traceback);
        nst_dec_ref(nst_state.traceback->error.name);
        nst_dec_ref(nst_state.traceback->error.message);
    }

    nst_state_free();
    return ERROR_OCCURRED ? 1 : 0;
}

void nst_state_free()
{
    nst_llist_destroy(nst_state.traceback->positions, free);
    nst_dec_ref(*nst_state.curr_path);
    nst_dec_ref(nst_state.argv);
    nst_vstack_destroy(nst_state.v_stack);
    nst_fstack_destroy(nst_state.f_stack);
    nst_cstack_destroy(nst_state.c_stack);
    nst_llist_destroy(nst_state.lib_paths, (nst_llist_destructor)_nst_dec_ref);
    nst_dec_ref(nst_state.lib_handles);
    nst_llist_destroy(nst_state.lib_srcs, (nst_llist_destructor)nst_free_src_text);
    for ( NST_LLIST_ITER(lib, nst_state.loaded_libs) )
    {
        void (*free_lib_func)() = (void (*)())dlsym(lib->value, "free_lib");
        if ( free_lib_func != NULL )
        {
            free_lib_func();
        }
    }
    nst_ggc_delete_objs(nst_state.ggc);
}

void _nst_unload_libs()
{
    nst_llist_destroy(nst_state.loaded_libs, (nst_llist_destructor)dlclose);
}

static inline void destroy_call(Nst_FuncCall *call, Nst_Int offset)
{
    nst_dec_ref(nst_map_drop_str((*nst_state.vt)->vars, "_vars_"));
    nst_dec_ref((*nst_state.vt)->vars);
    if ( (*nst_state.vt)->global_table != NULL )
    {
        nst_dec_ref((*nst_state.vt)->global_table);
    }
    free(*nst_state.vt);

    nst_dec_ref(call->func);

    *nst_state.vt = call->vt;
    *nst_state.idx = call->idx + offset;
}

static void complete_function(usize final_stack_size)
{
    if ( nst_state.f_stack->current_size == 0 )
    {
        return;
    }

    Nst_InstList *curr_inst_ls =
        nst_fstack_peek(nst_state.f_stack).func->body.bytecode;

    for ( ; nst_state.f_stack->current_size > final_stack_size;
          (*nst_state.idx)++ )
    {
        assert(curr_inst_ls != NULL);
        if ( *nst_state.idx >= (Nst_Int)curr_inst_ls->total_size )
        {
            // Free the function call
            Nst_FuncCall call = nst_fstack_pop(nst_state.f_stack);

            destroy_call(&call, 0);

            Nst_FuncObj *func = nst_fstack_peek(nst_state.f_stack).func;
            curr_inst_ls = func == NULL ? NULL : func->body.bytecode;
            continue;
        }

        Nst_Inst *inst =
            curr_inst_ls->instructions + *nst_state.idx;
        i32 inst_id = inst->id;
        run_instruction(inst);

        if ( ERROR_OCCURRED )
        {
            Nst_CatchFrame top_catch = nst_cstack_peek(nst_state.c_stack);
            Nst_Obj *obj;

            usize end_size = top_catch.f_stack_size;
            if ( end_size < final_stack_size )
            {
                end_size = final_stack_size;
            }

            while ( nst_state.f_stack->current_size > end_size )
            {
                Nst_FuncCall call = nst_fstack_pop(nst_state.f_stack);

                destroy_call(&call, 1);

                obj = nst_vstack_pop(nst_state.v_stack);
                while ( obj != NULL )
                {
                    nst_dec_ref(obj);
                    obj = nst_vstack_pop(nst_state.v_stack);
                }

                ADD_POSITIONS(call.start, call.end);
            }

            if ( end_size == final_stack_size )
            {
                return;
            }

            while ( nst_state.v_stack->current_size > top_catch.v_stack_size )
            {
                obj = nst_vstack_pop(nst_state.v_stack);
                if ( obj != NULL )
                {
                    nst_dec_ref(obj);
                }
            }
            *nst_state.idx = top_catch.inst_idx - 1;
            curr_inst_ls = nst_fstack_peek(nst_state.f_stack).func->body.bytecode;
        }

        // only OP_CALL and FOR_* can push a function on the call stack
        if ( inst_id == NST_IC_OP_CALL     || inst_id == NST_IC_FOR_START   ||
             inst_id == NST_IC_FOR_IS_DONE || inst_id == NST_IC_FOR_GET_VAL )
            curr_inst_ls = nst_fstack_peek(nst_state.f_stack).func->body.bytecode;
    }
}

i32 nst_run_module(i8 *filename, Nst_SourceText *lib_src)
{
    // Compile and optimize the imported module

    i32 opt_level = *nst_state.opt_level;
    Nst_Error error = { false, nst_no_pos(), nst_no_pos(), NULL, NULL };

    i32 file_opt_lvl;
    bool no_default;

    // The file is guaranteed to exist
    Nst_LList *tokens = nst_tokenizef(
        filename,
        false,
        &file_opt_lvl,
        &no_default,
        lib_src,
        &error);

    if ( file_opt_lvl < opt_level )
    {
        opt_level = file_opt_lvl;
    }

    if ( tokens == NULL )
    {
        _NST_SET_ERROR(
            GLOBAL_ERROR,
            error.start,
            error.end,
            error.name,
            error.message);
        return -1;
    }

    Nst_Node *ast = nst_parse(tokens, &error);
    if ( ast != NULL && opt_level >= 1 )
    {
        ast = nst_optimize_ast(ast, &error);
    }

    if ( ast == NULL )
    {
        _NST_SET_ERROR(
            GLOBAL_ERROR,
            error.start,
            error.end,
            error.name,
            error.message);
        return -1;
    }

    Nst_InstList *inst_ls = nst_compile(ast, true);
    if ( opt_level >= 2 )
    {
        inst_ls = nst_optimize_bytecode(
            inst_ls,
            opt_level == 3 && !no_default,
            &error);
    }
    if ( inst_ls == NULL )
    {
        _NST_SET_ERROR(
            GLOBAL_ERROR,
            error.start,
            error.end,
            error.name,
            error.message);
        return -1;
    }

    Nst_FuncObj *mod_func = FUNC(nst_func_new(0, inst_ls));

    // Change the cwd
    Nst_StrObj *prev_path = *nst_state.curr_path;

    Nst_StrObj *path_str = make_cwd(filename);
    *nst_state.curr_path = path_str;

    i32 res = _chdir(path_str->value);
    assert(res == 0);

    nst_vstack_push(nst_state.v_stack, NULL);
    nst_fstack_push(
        nst_state.f_stack,
        mod_func,
        nst_no_pos(),
        nst_no_pos(),
        *nst_state.vt,
        *nst_state.idx - 1);
    *nst_state.idx = 0;
    Nst_VarTable *vt = nst_vt_new(
        no_default ? MAP(nst_c.Null_null) : NULL,
        path_str,
        nst_state.argv);

    if ( no_default )
    {
        nst_dec_ref(nst_c.Null_null);
        vt->global_table = NULL;
        nst_dec_ref(nst_map_drop(vt->vars, nst_s.o__globals_));
    }

    CHANGE_VT(vt);

    nst_func_set_vt(mod_func, (*nst_state.vt)->vars);

    complete_function(nst_state.f_stack->current_size - 1);
    *nst_state.curr_path = prev_path;
    nst_dec_ref(path_str);
    nst_dec_ref(mod_func);

    res = _chdir(prev_path->value);
    assert(res == 0);

    if ( ERROR_OCCURRED )
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err)
{
    if ( NST_FLAG_HAS(func, NST_FLAG_FUNC_IS_C) )
    {
        return func->body.c_func(func->arg_num, args, err);
    }

    nst_vstack_push(nst_state.v_stack, NULL);
    nst_fstack_push(
        nst_state.f_stack,
        func,
        nst_no_pos(),
        nst_no_pos(),
        *nst_state.vt,
        *nst_state.idx - 1);

    Nst_VarTable *new_vt;
    if ( func->mod_globals != NULL )
    {
        new_vt = nst_vt_new(func->mod_globals, NULL, NULL);
    }
    else if ( (*nst_state.vt)->global_table == NULL )
    {
        new_vt = nst_vt_new((*nst_state.vt)->vars, NULL, NULL);
    }
    else
    {
        new_vt = nst_vt_new((*nst_state.vt)->global_table, NULL, NULL);
    }

    for ( usize i = 0, n = func->arg_num; i < n; i++ )
    {
        nst_vt_set(new_vt, func->args[i], args[i]);
    }

    *nst_state.idx = 0;
    CHANGE_VT(new_vt);
    complete_function(nst_state.f_stack->current_size - 1);

    if ( ERROR_OCCURRED )
    {
        return NULL;
    }

    return nst_vstack_pop(nst_state.v_stack);
}

Nst_Obj *nst_run_func_context(Nst_FuncObj *func,
                              Nst_Int idx,
                              Nst_MapObj *vars,
                              Nst_MapObj *globals)
{
    assert(!NST_FLAG_HAS(func, NST_FLAG_FUNC_IS_C));

    nst_fstack_push(
        nst_state.f_stack,
        func,
        nst_no_pos(),
        nst_no_pos(),
        *nst_state.vt,
        *nst_state.idx - 1);

    Nst_VarTable *new_vt = (Nst_VarTable *)malloc(sizeof(Nst_VarTable));
    if ( new_vt == NULL )
    {
        return NULL;
    }

    new_vt->vars = MAP(nst_inc_ref(vars));

    if ( globals == NULL )
    {
        if ( func->mod_globals != NULL )
        {
            new_vt->global_table = MAP(nst_inc_ref(func->mod_globals));
        }
        else if ( (*nst_state.vt)->global_table == NULL )
        {
            new_vt->global_table = MAP(nst_inc_ref((*nst_state.vt)->vars));
        }
        else
        {
            new_vt->global_table =
                MAP(nst_inc_ref((*nst_state.vt)->global_table));
        }
    }
    else
    {
        new_vt->global_table = MAP(nst_inc_ref(globals));
    }
    CHANGE_VT(new_vt);
    *nst_state.idx = idx;
    complete_function(nst_state.f_stack->current_size - 1);

    if ( ERROR_OCCURRED )
    {
        return NULL;
    }

    return nst_vstack_pop(nst_state.v_stack);
}

static inline void run_instruction(Nst_Inst *inst)
{
    switch ( inst->id )
    {
    case NST_IC_POP_VAL:      exe_pop_val();           break;
    case NST_IC_FOR_START:    exe_for_start(inst);     break;
    case NST_IC_FOR_IS_DONE:  exe_for_is_done(inst);   break;
    case NST_IC_FOR_GET_VAL:  exe_for_get_val(inst);   break;
    case NST_IC_RETURN_VAL:   exe_return_val();        break;
    case NST_IC_RETURN_VARS:  exe_return_vars();       break;
    case NST_IC_SET_VAL_LOC:  exe_set_val_loc(inst);   break;
    case NST_IC_SET_CONT_LOC: exe_set_cont_loc(inst);  break;
    case NST_IC_JUMP:         exe_jump(inst);          break;
    case NST_IC_JUMPIF_T:     exe_jumpif_t(inst);      break;
    case NST_IC_JUMPIF_F:     exe_jumpif_f(inst);      break;
    case NST_IC_JUMPIF_ZERO:  exe_jumpif_zero(inst);   break;
    case NST_IC_TYPE_CHECK:   exe_type_check(inst);    break;
    case NST_IC_HASH_CHECK:   exe_hash_check(inst);    break;
    case NST_IC_SET_VAL:      exe_set_val(inst);       break;
    case NST_IC_GET_VAL:      exe_get_val(inst);       break;
    case NST_IC_PUSH_VAL:     exe_push_val(inst);      break;
    case NST_IC_SET_CONT_VAL: exe_set_cont_val(inst);  break;
    case NST_IC_OP_CALL:      exe_op_call(inst);       break;
    case NST_IC_OP_CAST:      exe_op_cast(inst);       break;
    case NST_IC_OP_RANGE:     exe_op_range(inst);      break;
    case NST_IC_THROW_ERR:    exe_throw_err(inst);     break;
    case NST_IC_STACK_OP:     exe_stack_op(inst);      break;
    case NST_IC_LOCAL_OP:     exe_local_op(inst);      break;
    case NST_IC_OP_IMPORT:    exe_op_import(inst);     break;
    case NST_IC_OP_EXTRACT:   exe_op_extract(inst);    break;
    case NST_IC_DEC_INT:      exe_dec_int();           break;
    case NST_IC_NEW_OBJ:      exe_new_obj();           break;
    case NST_IC_DUP:          exe_dup();               break;
    case NST_IC_ROT:          exe_rot(inst);           break;
    case NST_IC_MAKE_ARR:
    case NST_IC_MAKE_VEC:     exe_make_seq(inst);      break;
    case NST_IC_MAKE_ARR_REP:
    case NST_IC_MAKE_VEC_REP: exe_make_seq_rep(inst);  break;
    case NST_IC_MAKE_MAP:     exe_make_map(inst);      break;
    case NST_IC_PUSH_CATCH:   exe_push_catch(inst);    break;
    case NST_IC_POP_CATCH:    exe_pop_catch();         break;
    case NST_IC_SAVE_ERROR:   exe_save_error();        break;
    case NST_IC_UNPACK_SEQ:   exe_unpack_seq(inst);    break;
    case NST_IC_NO_OP:                                 break;
    default: assert(false);
    }
}

static inline void exe_pop_val()
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_vstack_pop(nst_state.v_stack);
    nst_dec_ref(obj);
}

static inline void exe_for_inst(Nst_Inst *inst,
                                Nst_IterObj     *iter,
                                Nst_FuncObj     *func)
{
    if ( NST_FLAG_HAS(func, NST_FLAG_FUNC_IS_C) )
    {
        Nst_OpErr err = { NULL, NULL };
        Nst_Obj *res = func->body.c_func(
            (usize)inst->int_val,
            &iter->value,
            &err);

        if ( res == NULL )
        {
            SET_OP_ERROR(inst->start, inst->end, err);
        }
        else
        {
            nst_vstack_push(nst_state.v_stack, res);
            nst_dec_ref(res);
        }
    }
    else
    {
        nst_vstack_push(nst_state.v_stack, iter->value);
        nst_vstack_push(nst_state.v_stack, func);
        exe_op_call(inst);
    }
}

static inline void exe_for_start(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(nst_vstack_peek(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->start);
}

static inline void exe_for_is_done(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(nst_vstack_peek(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->is_done);
}

static inline void exe_for_get_val(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(nst_vstack_peek(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->get_val);
}

static inline void exe_return_val()
{
    Nst_Obj *result = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *obj = nst_vstack_pop(nst_state.v_stack);

    while ( obj != NULL )
    {
        nst_dec_ref(obj);
        obj = nst_vstack_pop(nst_state.v_stack);
    }

    nst_vstack_push(nst_state.v_stack, result);
    *nst_state.idx = nst_fstack_peek(nst_state.f_stack)
        .func->body
        .bytecode->total_size;
    nst_dec_ref(result);
}

static inline void exe_return_vars()
{
    Nst_MapObj *vars = (*nst_state.vt)->vars;
    Nst_Obj *obj = nst_vstack_pop(nst_state.v_stack);

    while ( obj != NULL )
    {
        nst_dec_ref(obj);
        obj = nst_vstack_pop(nst_state.v_stack);
    }

    nst_vstack_push(nst_state.v_stack, vars);
    *nst_state.idx = nst_fstack_peek(nst_state.f_stack)
        .func->body
        .bytecode->total_size;
}

static inline void exe_set_val_loc(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = nst_vstack_pop(nst_state.v_stack);
    nst_vt_set(*nst_state.vt, inst->val, val);
    nst_dec_ref(val);
}

static inline void exe_set_cont_loc(Nst_Inst *inst)
{
    exe_set_cont_val(inst);
    CHECK_V_STACK;
    nst_vstack_pop(nst_state.v_stack);
}

static inline void exe_jump(Nst_Inst *inst)
{
    *nst_state.idx = inst->int_val - 1;
}

static inline void exe_jumpif_t(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *result = nst_obj_cast(top_val, nst_t.Bool, NULL);
    nst_dec_ref(top_val);

    if ( result == nst_c.Bool_true )
    {
        *nst_state.idx = inst->int_val - 1;
    }
    nst_dec_ref(result);
}

static inline void exe_jumpif_f(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *result = nst_obj_cast(top_val, nst_t.Bool, NULL);
    nst_dec_ref(top_val);

    if ( result == nst_c.Bool_false )
    {
        *nst_state.idx = inst->int_val - 1;
    }
    nst_dec_ref(result);
}

static inline void exe_jumpif_zero(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = nst_vstack_peek(nst_state.v_stack);
    if ( AS_INT(val) == 0 )
    {
        *nst_state.idx = inst->int_val - 1;
    }
}

static inline void exe_type_check(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_vstack_peek(nst_state.v_stack);
    if ( obj->type != TYPE(inst->val) )
    {
        _NST_SET_TYPE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            nst_format_error(
                _NST_EM_EXPECTED_TYPES, "ss",
                STR(inst->val)->value,
                TYPE_NAME(obj)));
    }
}

static inline void exe_hash_check(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_vstack_peek(nst_state.v_stack);
    nst_obj_hash(obj);
    if ( obj->hash == -1 )
    {
        _NST_SET_TYPE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            nst_format_error(_NST_EM_UNHASHABLE_TYPE, "s", TYPE_NAME(obj)));
    }
}

static inline void exe_set_val(Nst_Inst *inst)
{
    CHECK_V_STACK;
    nst_vt_set(
        *nst_state.vt,
        inst->val,
        nst_vstack_peek(nst_state.v_stack));
}

static inline void exe_get_val(Nst_Inst *inst)
{
    Nst_Obj *obj = nst_vt_get(*nst_state.vt, inst->val);
    if ( obj == NULL )
    {
        nst_vstack_push(nst_state.v_stack, nst_c.Null_null);
    }
    else
    {
        nst_vstack_push(nst_state.v_stack, obj);
        nst_dec_ref(obj);
    }
}

static inline void exe_push_val(Nst_Inst *inst)
{
    nst_vstack_push(nst_state.v_stack, inst->val);
}

static inline void exe_set_cont_val(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(3);
    Nst_Obj *idx = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *cont = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *val = nst_vstack_peek(nst_state.v_stack);

    if ( cont->type == nst_t.Array || cont->type == nst_t.Vector )
    {
        if ( idx->type != nst_t.Int )
        {
            _NST_SET_TYPE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                nst_format_error(
                    _NST_EM_EXPECTED_TYPE("Int"),
                    "s",
                    TYPE_NAME(idx)));

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        bool res = nst_seq_set(cont, AS_INT(idx), val);

        if ( !res )
        {
            _NST_SET_VALUE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                nst_format_error(
                    cont->type == nst_t.Array ?
                        _NST_EM_INDEX_OUT_OF_BOUNDS("Array")
                      : _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
                    "iu",
                    AS_INT(idx),
                    SEQ(cont)->len));
        }

        nst_dec_ref(cont);
        nst_dec_ref(idx);
    }
    else if ( cont->type == nst_t.Map )
    {
        bool res = nst_map_set(cont, idx, val);
        if ( !res )
        {
            _NST_SET_TYPE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                nst_format_error(
                    _NST_EM_UNHASHABLE_TYPE,
                    "s",
                    TYPE_NAME(idx)));
        }

        nst_dec_ref(cont);
        nst_dec_ref(idx);
    }
    else
    {
        _NST_SET_TYPE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            nst_format_error(
                _NST_EM_EXPECTED_TYPE("Array', 'Vector', or 'Map"),
                "s",
                TYPE_NAME(cont)));
    }
}

static inline void exe_op_call(Nst_Inst *inst)
{
    Nst_Int arg_num = inst->int_val;
    bool is_seq_call = false;

    if ( arg_num == -1 )
    {
        CHECK_V_STACK_SIZE(2);
    }
    else
    {
        CHECK_V_STACK_SIZE(arg_num + 1);
    }
    Nst_FuncObj *func = FUNC(nst_vstack_pop(nst_state.v_stack));
    Nst_SeqObj *args_seq;

    if ( arg_num == -1 )
    {
        args_seq = SEQ(nst_vstack_pop(nst_state.v_stack));
        if ( args_seq->type != nst_t.Array && args_seq->type != nst_t.Vector )
        {
            _NST_SET_TYPE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                nst_format_error(
                    _NST_EM_EXPECTED_TYPE("Array' or 'Vector"),
                    "s",
                    TYPE_NAME(args_seq)));

            nst_dec_ref(args_seq);
            nst_dec_ref(func);
            return;
        }
        is_seq_call = true;
        arg_num = args_seq->len;
    }
    else
    {
        args_seq = NULL;
    }

    if ( (Nst_Int)(func->arg_num) < arg_num )
    {
        _NST_SET_CALL_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            nst_format_error(
                _NST_EM_WRONG_ARG_NUM, "usis",
                func->arg_num, func->arg_num == 1 ? "" : "s",
                arg_num, arg_num == 1 ? "was" : "were"));

        nst_dec_ref(func);
        if ( is_seq_call )
        {
            nst_dec_ref(args_seq);
        }

        return;
    }

    Nst_Int null_args = (Nst_Int)func->arg_num - arg_num;
    Nst_Int tot_args = arg_num + null_args;

    if ( NST_FLAG_HAS(func, NST_FLAG_FUNC_IS_C) )
    {
        Nst_OpErr err = { NULL, NULL };
        Nst_Obj **args;
        Nst_Obj *stack_args[10]; // up to 10 arguments this array is used
        bool args_allocated = false;

        if ( is_seq_call && null_args == 0 )
        {
            args = args_seq->objs;
        }
        else if ( is_seq_call && tot_args <= 10 )
        {
            memcpy(
                stack_args,
                args_seq->objs,
                (usize)arg_num * sizeof(Nst_Obj *));
            args = stack_args;
        }
        else if ( is_seq_call )
        {
            args = (Nst_Obj **)malloc((usize)(sizeof(Nst_Obj *) * tot_args));
            if ( args == NULL )
            {
                return;
            }

            memcpy(
                stack_args,
                args_seq->objs,
                (usize)arg_num * sizeof(Nst_Obj *));
            args = stack_args;
        }
        else if ( tot_args == 0 )
        {
            args = NULL;
        }
        else if ( tot_args <= 10 )
        {
            for ( Nst_Int i = arg_num - 1; i >= 0; i-- )
            {
                stack_args[i] = nst_vstack_pop(nst_state.v_stack);
            }
            args = stack_args;
        }
        else
        {
            args = (Nst_Obj **)malloc((usize)(sizeof(Nst_Obj *) * tot_args));
            if ( args == NULL )
            {
                return;
            }

            for ( Nst_Int i = arg_num - 1; i >= 0; i-- )
            {
                args[i] = nst_vstack_pop(nst_state.v_stack);
            }
            args_allocated = true;
        }

        for ( Nst_Int i = 0; i < null_args; i++ )
        {
            args[arg_num + i] = nst_inc_ref(nst_c.Null_null);
        }

        Nst_Obj *res = func->body.c_func((usize)tot_args, args, &err);

        if ( !is_seq_call )
        {
            for ( Nst_Int i = 0; i < tot_args; i++ )
            {
                nst_dec_ref(args[i]);
            }
        }
        else
        {
            for ( Nst_Int i = 0; i < null_args; i++ )
            {
                nst_dec_ref(args[arg_num + i]);
            }
        }

        if ( args_allocated )
        {
            free(args);
        }

        if ( res == NULL )
        {
            SET_OP_ERROR(inst->start, inst->end, err);
        }
        else
        {
            nst_vstack_push(nst_state.v_stack, res);
            nst_dec_ref(res);
        }

        nst_dec_ref(func);
        return;
    }

    bool res = nst_fstack_push(
        nst_state.f_stack,
        func,
        inst->start,
        inst->end,
        *nst_state.vt,
        *nst_state.idx);
    *nst_state.idx = -1;
    nst_dec_ref(func);

    if ( !res )
    {
        _NST_SET_RAW_CALL_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            _NST_EM_CALL_STACK_SIZE_EXCEEDED);

        return;
    }

    Nst_VarTable *new_vt;

    if ( func->mod_globals != NULL )
    {
        new_vt = nst_vt_new(func->mod_globals, NULL, NULL);
    }
    else if ( (*nst_state.vt)->global_table == NULL )
    {
        new_vt = nst_vt_new((*nst_state.vt)->vars, NULL, NULL);
    }
    else
    {
        new_vt = nst_vt_new((*nst_state.vt)->global_table, NULL, NULL);
    }

    for ( Nst_Int i = 0; i < arg_num; i++ )
    {
        Nst_Obj *val;
        if ( is_seq_call )
        {
            val = nst_inc_ref(args_seq->objs[i]);
        }
        else
        {
            val = nst_vstack_pop(nst_state.v_stack);
        }

        nst_vt_set(new_vt, func->args[arg_num - i - 1], val);
        nst_dec_ref(val);
    }

    for ( Nst_Int i = arg_num; i < tot_args; i++ )
    {
        nst_vt_set(new_vt, func->args[i], nst_c.Null_null);
    }

    CHANGE_VT(new_vt);
    nst_vstack_push(nst_state.v_stack, NULL);
}

static inline void exe_op_cast(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *val = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *type = nst_vstack_pop(nst_state.v_stack);

    Nst_OpErr err = { NULL, NULL };
    Nst_Obj *res = nst_obj_cast(val, type, &err);

    if ( res == NULL )
    {
        SET_OP_ERROR(inst->start, inst->end, err);
    }
    else
    {
        nst_vstack_push(nst_state.v_stack, res);
        nst_dec_ref(res);
    }

    nst_dec_ref(val);
    nst_dec_ref(type);
}

static inline void exe_op_range(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(inst->int_val);
    Nst_Obj *stop = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *step = NULL;
    Nst_Obj *start = NULL;

    if ( inst->int_val == 3 )
    {
        start = nst_vstack_pop(nst_state.v_stack);
        step  = nst_vstack_pop(nst_state.v_stack);
    }
    else
    {
        start = nst_vstack_pop(nst_state.v_stack);

        if ( AS_INT(start) <= AS_INT(stop) )
        {
            step = nst_inc_ref(nst_c.Int_1);
        }
        else
        {
            step = nst_inc_ref(nst_c.Int_neg1);
        }
    }

    Nst_OpErr err = { NULL, NULL };
    Nst_Obj *iter = _nst_obj_range(start, stop, step, &err);

    nst_dec_ref(start);
    nst_dec_ref(stop);
    nst_dec_ref(step);

    if ( iter == NULL )
    {
        SET_OP_ERROR(inst->start, inst->end, err);
    }
    else
    {
        nst_vstack_push(nst_state.v_stack, iter);
        nst_dec_ref(iter);
    }
}

static inline void exe_throw_err(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *message = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *name = nst_vstack_pop(nst_state.v_stack);

    _NST_SET_ERROR(GLOBAL_ERROR, inst->start, inst->end, name, message);

    nst_dec_ref(name);
    nst_dec_ref(message);
}

static inline void exe_stack_op(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *ob2 = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *ob1 = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *res = NULL;
    Nst_OpErr err = { NULL, NULL };

    switch ( inst->int_val )
    {
    case NST_TT_ADD:    res = nst_obj_add(ob1, ob2, &err);   break;
    case NST_TT_SUB:    res = nst_obj_sub(ob1, ob2, &err);   break;
    case NST_TT_MUL:    res = nst_obj_mul(ob1, ob2, &err);   break;
    case NST_TT_DIV:    res = nst_obj_div(ob1, ob2, &err);   break;
    case NST_TT_POW:    res = nst_obj_pow(ob1, ob2, &err);   break;
    case NST_TT_MOD:    res = nst_obj_mod(ob1, ob2, &err);   break;
    case NST_TT_B_AND:  res = nst_obj_bwand(ob1, ob2, &err); break;
    case NST_TT_B_OR:   res = nst_obj_bwor(ob1, ob2, &err);  break;
    case NST_TT_B_XOR:  res = nst_obj_bwxor(ob1, ob2, &err); break;
    case NST_TT_LSHIFT: res = nst_obj_bwls(ob1, ob2, &err);  break;
    case NST_TT_RSHIFT: res = nst_obj_bwrs(ob1, ob2, &err);  break;
    case NST_TT_CONCAT: res = nst_obj_concat(ob1, ob2, &err);break;
    case NST_TT_L_AND:  res = nst_obj_lgand(ob1, ob2, &err); break;
    case NST_TT_L_OR:   res = nst_obj_lgor(ob1, ob2, &err);  break;
    case NST_TT_L_XOR:  res = nst_obj_lgxor(ob1, ob2, &err); break;
    case NST_TT_GT:     res = nst_obj_gt(ob1, ob2, &err);    break;
    case NST_TT_LT:     res = nst_obj_lt(ob1, ob2, &err);    break;
    case NST_TT_EQ:     res = nst_obj_eq(ob1, ob2, &err);    break;
    case NST_TT_NEQ:    res = nst_obj_ne(ob1, ob2, &err);    break;
    case NST_TT_GTE:    res = nst_obj_ge(ob1, ob2, &err);    break;
    case NST_TT_LTE:    res = nst_obj_le(ob1, ob2, &err);    break;
    }

    if ( res == NULL )
    {
        SET_OP_ERROR(inst->start, inst->end, err);
    }
    else
    {
        nst_vstack_push(nst_state.v_stack, res);
        nst_dec_ref(res);
    }

    nst_dec_ref(ob1);
    nst_dec_ref(ob2);
}

static inline void exe_local_op(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *res = NULL;
    Nst_OpErr err = { NULL, NULL };

    switch ( inst->int_val )
    {
    case NST_TT_LEN:    res = nst_obj_len(obj, &err);   break;
    case NST_TT_L_NOT:  res = nst_obj_lgnot(obj, &err); break;
    case NST_TT_B_NOT:  res = nst_obj_bwnot(obj, &err); break;
    case NST_TT_STDOUT: res = nst_obj_stdout(obj, &err);break;
    case NST_TT_STDIN:  res = nst_obj_stdin(obj, &err); break;
    case NST_TT_NEG:    res = nst_obj_neg(obj, &err);   break;
    case NST_TT_TYPEOF: res = nst_obj_typeof(obj, &err);break;
    }

    if ( res == NULL )
    {
        SET_OP_ERROR(inst->start, inst->end, err);
        nst_dec_ref(obj);
        return;
    }

    nst_vstack_push(nst_state.v_stack, res);
    nst_dec_ref(obj);
    nst_dec_ref(res);
}

static inline void exe_op_import(Nst_Inst *inst)
{
    CHECK_V_STACK;
    Nst_Obj *name = nst_vstack_pop(nst_state.v_stack);
    Nst_OpErr err = { NULL, NULL };
    Nst_Obj *res = nst_obj_import(name, &err);

    if ( res == NULL )
    {
        SET_OP_ERROR(inst->start, inst->end, err);
        nst_dec_ref(name);
        return;
    }

    nst_vstack_push(nst_state.v_stack, res);
    nst_dec_ref(name);
    nst_dec_ref(res);
}

static inline void exe_op_extract(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *idx = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *cont = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *res = NULL;

    if ( cont->type == nst_t.Array || cont->type == nst_t.Vector )
    {
        if ( idx->type != nst_t.Int )
        {
            _NST_SET_TYPE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                nst_format_error(
                    _NST_EM_EXPECTED_TYPE("Int"),
                    "s",
                    TYPE_NAME(idx)));

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        res = nst_seq_get(cont, AS_INT(idx));

        if ( res == NULL )
        {
            _NST_SET_VALUE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                nst_format_error(
                    cont->type == nst_t.Array ?
                        _NST_EM_INDEX_OUT_OF_BOUNDS("Array")
                      : _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
                    "iu",
                    AS_INT(idx),
                    SEQ(cont)->len));

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        nst_vstack_push(nst_state.v_stack, res);
    }
    else if ( cont->type == nst_t.Map )
    {
        res = nst_map_get(cont, idx);

        if ( res == NULL )
        {
            if ( idx->hash != -1 )
            {
                nst_vstack_push(nst_state.v_stack, nst_c.Null_null);
            }
            else
            {
                _NST_SET_VALUE_ERROR(
                    GLOBAL_ERROR,
                    inst->start,
                    inst->start,
                    nst_format_error(
                        _NST_EM_UNHASHABLE_TYPE,
                        "s",
                        TYPE_NAME(idx)));
            }

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }
        else
        {
            nst_vstack_push(nst_state.v_stack, res);
        }
    }
    else if ( cont->type == nst_t.Str )
    {
        if ( idx->type != nst_t.Int )
        {
            _NST_SET_TYPE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                nst_format_error(
                    _NST_EM_EXPECTED_TYPE("Int"),
                    "s",
                    TYPE_NAME(idx)));

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        res = nst_string_get(cont, AS_INT(idx));

        if ( res == NULL )
        {
            _NST_SET_VALUE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                nst_format_error(
                    _NST_EM_INDEX_OUT_OF_BOUNDS("Str"),
                    "iu",
                    AS_INT(idx),
                    STR(cont)->len));

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }
        else
        {
            nst_vstack_push(nst_state.v_stack, res);
        }
    }
    else
    {
        _NST_SET_TYPE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            nst_format_error(
                _NST_EM_EXPECTED_TYPE("Array', 'Vector', 'Map' or 'Str"),
                "s",
                TYPE_NAME(cont)));

        nst_dec_ref(idx);
        nst_dec_ref(cont);
        return;
    }

    nst_dec_ref(res);
    nst_dec_ref(cont);
    nst_dec_ref(idx);
}

static inline void exe_dec_int()
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_vstack_peek(nst_state.v_stack);
    AS_INT(obj) -= 1;
}

static inline void exe_new_obj()
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_vstack_peek(nst_state.v_stack);
    Nst_Obj *new_obj = nst_int_new(AS_INT(obj));
    nst_vstack_push(nst_state.v_stack, new_obj);
    nst_dec_ref(new_obj);
}

static inline void exe_dup()
{
    CHECK_V_STACK;
    nst_vstack_push(nst_state.v_stack, nst_vstack_peek(nst_state.v_stack));
}

static inline void exe_rot(Nst_Inst *inst)
{
    CHECK_V_STACK_SIZE(inst->int_val);

    Nst_Obj *obj = nst_vstack_peek(nst_state.v_stack);
    usize stack_size = nst_state.v_stack->current_size - 1;
    Nst_Obj **stack = nst_state.v_stack->stack;
    for ( Nst_Int i = 1, n = inst->int_val; i < n; i++ )
    {
        stack[stack_size - i + 1] = stack[stack_size - i];
    }

    stack[stack_size - inst->int_val + 1] = obj;
}

static inline void exe_make_seq(Nst_Inst *inst)
{
    Nst_Int seq_size = inst->int_val;
    Nst_Obj *seq = inst->id == NST_IC_MAKE_ARR
        ? nst_array_new((usize)seq_size)
        : nst_vector_new((usize)seq_size);
    CHECK_V_STACK_SIZE(seq_size);

    for ( Nst_Int i = 1; i <= seq_size; i++ )
    {
        Nst_Obj *curr_obj = nst_vstack_pop(nst_state.v_stack);
        nst_seq_set(seq, seq_size - i, curr_obj);
        nst_dec_ref(curr_obj);
    }

    nst_vstack_push(nst_state.v_stack, seq);
    nst_dec_ref(seq);
}

static inline void exe_make_seq_rep(Nst_Inst *inst)
{
    Nst_Int seq_size = inst->int_val;
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *size_obj = nst_vstack_pop(nst_state.v_stack);
    Nst_Obj *val = nst_vstack_pop(nst_state.v_stack);

    Nst_Int size = AS_INT(size_obj);
    nst_dec_ref(size_obj);
    Nst_Obj *seq = inst->id == NST_IC_MAKE_ARR_REP
        ? nst_array_new((usize)size)
        : nst_vector_new((usize)size);

    for ( Nst_Int i = 1; i <= size; i++ )
    {
        nst_seq_set(seq, seq_size - i, val);
    }

    nst_vstack_push(nst_state.v_stack, seq);
    nst_dec_ref(seq);
    nst_dec_ref(val);
}

static inline void exe_make_map(Nst_Inst *inst)
{
    Nst_Int map_size = inst->int_val;
    Nst_Obj *map = nst_map_new();
    CHECK_V_STACK_SIZE(map_size);
    usize stack_size = nst_state.v_stack->current_size;
    Nst_Obj **v_stack = nst_state.v_stack->stack;

    for ( Nst_Int i = 0; i < map_size; i++ )
    {
        Nst_Obj *key = v_stack[stack_size - map_size + i];
        i++;
        Nst_Obj *val = v_stack[stack_size - map_size + i];
        nst_map_set(map, key, val);
        nst_dec_ref(val);
        nst_dec_ref(key);
    }
    nst_state.v_stack->current_size -= (usize)map_size;
    nst_vstack_push(nst_state.v_stack, map);
    nst_dec_ref(map);
}

static inline void exe_push_catch(Nst_Inst* inst)
{
    nst_cstack_push(nst_state.c_stack,
                   inst->int_val,
                   nst_state.v_stack->current_size,
                   nst_state.f_stack->current_size);
}

static inline void exe_pop_catch()
{
    nst_cstack_pop(nst_state.c_stack);
}

static inline void exe_save_error()
{
    assert(GLOBAL_ERROR->occurred);

    Nst_Obj *err_map = nst_map_new();
    nst_map_set_str(err_map, "name", GLOBAL_ERROR->name);
    nst_map_set_str(err_map, "message", GLOBAL_ERROR->message);

    nst_state.traceback->error.occurred = false;
    nst_dec_ref(GLOBAL_ERROR->name);
    nst_dec_ref(GLOBAL_ERROR->message);

    nst_llist_empty(nst_state.traceback->positions, free);

    nst_vstack_push(nst_state.v_stack, err_map);
    nst_dec_ref(err_map);

    // Remove the source of any libraries that failed to load
    while ( nst_state.lib_srcs->size != nst_state.lib_handles->item_count )
    {
        Nst_SourceText *txt = (Nst_SourceText *)nst_llist_pop(nst_state.lib_srcs);
        nst_free_src_text(txt);
    }
}

static inline void exe_unpack_seq(Nst_Inst* inst)
{
    CHECK_V_STACK;
    Nst_SeqObj *seq = SEQ(nst_vstack_pop(nst_state.v_stack));

    if ( seq->type != nst_t.Array && seq->type != nst_t.Vector )
    {
        _NST_SET_TYPE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            nst_format_error(
                _NST_EM_EXPECTED_TYPE("Array' or 'Vector"),
                "s", TYPE_NAME(seq)));
        nst_dec_ref(seq);
        return;
    }

    if ( (Nst_Int)seq->len != inst->int_val )
    {
        _NST_SET_VALUE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            nst_format_error(
                _NST_EM_WRONG_UNPACK_LENGTH,
                "iu", inst->int_val, seq->len));
        nst_dec_ref(seq);
        return;
    }

    for ( Nst_Int i = seq->len - 1; i >= 0; i-- )
    {
        nst_vstack_push(nst_state.v_stack, seq->objs[i]);
    }

    nst_dec_ref(seq);
}

usize nst_get_full_path(i8 *file_path, i8 **buf, i8 **file_part)
{
    i8 *path = (i8 *)malloc(sizeof(i8) * PATH_MAX);
    if ( path == NULL )
    {
        return 0;
    }

#if defined(_WIN32) || defined(WIN32)

    DWORD path_len = GetFullPathNameA(file_path, PATH_MAX, path, file_part);

    if ( path_len == 0 )
    {
        free(path);
        *buf = NULL;
        return 0;
    }

    if ( path_len > PATH_MAX )
    {
        free(path);
        path = (i8 *)malloc(sizeof(i8) * path_len);
        if ( path == NULL )
        {
            return 0;
        }
        path_len = GetFullPathNameA(file_path, path_len, path, file_part);

        if ( path_len == 0 )
        {
            free(path);
            *buf = NULL;
            return 0;
        }
    }

    *buf = path;
    return path_len;

#else

    i8 *result = realpath(file_path, path);

    if ( result == NULL )
    {
        free(path);
        *buf = NULL;
        return 0;
    }

    if ( file_part != NULL )
    {
        *file_part = strrchr(path, '/');

        if ( !*file_part )
        {
            *file_part = path;
        }
        else
        {
            (*file_part)++;
        }
    }

    *buf = path;
    return strlen(path);
#endif
}

static Nst_SeqObj *make_argv(i32 argc, i8 **argv, i8 *filename)
{
    Nst_SeqObj *args = SEQ(nst_array_new(argc + 1));

    Nst_Obj *val = nst_string_new_c_raw(filename, false);
    nst_seq_set(args, 0, val);
    nst_dec_ref(val);

    for ( i32 i = 0; i < argc; i++ )
    {
        val = nst_string_new_c_raw(argv[i], false);
        nst_seq_set(args, i + 1, val);
        nst_dec_ref(val);
    }

    return args;
}

static Nst_StrObj *make_cwd(i8 *file_path)
{
    i8 *path = NULL;
    i8 *file_part = NULL;

    nst_get_full_path(file_path, &path, &file_part);

    *(file_part - 1) = 0;

    return STR(nst_string_new(path, file_part - path - 1, true));
}

Nst_ExecutionState *nst_get_state()
{
    return &nst_state;
}
