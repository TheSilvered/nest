#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "interpreter.h"
#include "error_internal.h"
#include "obj_ops.h"
#include "hash.h"
#include "tokens.h"
#include "iter.h"
#include "compiler.h"
#include "optimizer.h"
#include "parser.h"
#include "lexer.h"
#include "global_consts.h"

#if defined(_WIN32) || defined(WIN32)

#include <windows.h>
#include <direct.h>

#define dlclose FreeLibrary
#define dlsym GetProcAddress
#define PATH_MAX MAX_PATH

#else

#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#define _chdir chdir
#define _getcwd getcwd

#endif

#define ADD_POSITIONS(start_pos, end_pos) do { \
        Nst_Pos *positions = (Nst_Pos *)malloc(sizeof(Nst_Pos) * 2); \
        if ( positions == NULL ) \
            break; \
        positions[0] = start_pos; \
        positions[1] = end_pos; \
        LList_push(nst_state.traceback->positions, positions + 1, false); \
        LList_push(nst_state.traceback->positions, positions,     true); \
    } while ( 0 )

#define SET_OP_ERROR(start_pos, end_pos, op_err) do { \
        if ( nst_state.traceback->error.occurred ) \
        { \
            ADD_POSITIONS(start_pos, end_pos);\
            break; \
        } \
        _NST_SET_ERROR(GLOBAL_ERROR, start_pos, end_pos, op_err.name, op_err.message); \
        nst_dec_ref(op_err.name); \
        nst_dec_ref(op_err.message); \
    } while ( 0 )

#define CHANGE_VT(new_vt) do { \
    *nst_state.vt = new_vt; \
    nst_add_tracked_object((Nst_GGCObj *)(*nst_state.vt)->vars); \
    } while ( 0 )

#define ERROR_OCCURRED (nst_state.traceback->error.occurred)
#define GLOBAL_ERROR (&(nst_state.traceback->error))

#define CHECK_V_STACK assert(nst_state.v_stack->current_size != 0)
#define CHECK_V_STACK_SIZE(size) assert((Nst_Int)(nst_state.v_stack->current_size) >= size)
#define CHECK_F_STACK assert(nst_state.f_stack->current_size != 0)

Nst_ExecutionState nst_state;
static int opt_level;

static void complete_function(size_t final_stack_size);
static inline void run_instruction(Nst_RuntimeInstruction *inst);
static inline void exe_no_op();
static inline void exe_pop_val();
static inline void exe_for_start(Nst_RuntimeInstruction *inst);
static inline void exe_for_is_done(Nst_RuntimeInstruction *inst);
static inline void exe_for_get_val(Nst_RuntimeInstruction *inst);
static inline void exe_for_advance(Nst_RuntimeInstruction *inst);
static inline void exe_return_val();
static inline void exe_return_vars();
static inline void exe_set_val_loc(Nst_RuntimeInstruction *inst);
static inline void exe_jump(Nst_RuntimeInstruction *inst);
static inline void exe_jumpif_t(Nst_RuntimeInstruction *inst);
static inline void exe_jumpif_f(Nst_RuntimeInstruction *inst);
static inline void exe_jumpif_zero(Nst_RuntimeInstruction *inst);
static inline void exe_type_check(Nst_RuntimeInstruction *inst);
static inline void exe_hash_check(Nst_RuntimeInstruction *inst);
static inline void exe_set_val(Nst_RuntimeInstruction *inst);
static inline void exe_get_val(Nst_RuntimeInstruction *inst);
static inline void exe_push_val(Nst_RuntimeInstruction *inst);
static inline void exe_set_cont_val(Nst_RuntimeInstruction *inst);
static inline void exe_op_call(Nst_RuntimeInstruction *inst);
static inline void exe_op_cast(Nst_RuntimeInstruction *inst);
static inline void exe_op_range(Nst_RuntimeInstruction *inst);
static inline void exe_throw_err(Nst_RuntimeInstruction *inst);
static inline void exe_stack_op(Nst_RuntimeInstruction *inst);
static inline void exe_local_op(Nst_RuntimeInstruction *inst);
static inline void exe_op_import(Nst_RuntimeInstruction *inst);
static inline void exe_op_extract(Nst_RuntimeInstruction *inst);
static inline void exe_dec_int();
static inline void exe_new_obj();
static inline void exe_dup();
static inline void exe_make_seq(Nst_RuntimeInstruction *inst);
static inline void exe_make_seq_rep(Nst_RuntimeInstruction *inst);
static inline void exe_make_map(Nst_RuntimeInstruction *inst);
static inline void exe_push_catch(Nst_RuntimeInstruction *inst);
static inline void exe_pop_catch();
static inline void exe_save_error();

static Nst_SeqObj *make_argv(int argc, char **argv, char *filename);
static Nst_StrObj *make_cwd(char *file_path);

void nst_run(Nst_FuncObj *main_func, int argc, char **argv, char *filename, int opt_lvl)
{
    opt_level = opt_lvl;

    char *cwd_buf = (char *)malloc(sizeof(char) * PATH_MAX);
    if ( cwd_buf == NULL )
        return;

    Nst_Traceback tb;
    tb.error.start = nst_no_pos();
    tb.error.end = nst_no_pos();
    tb.error.name = NULL;
    tb.error.message = NULL;
    tb.error.occurred = false;
    tb.positions = LList_new();
    Nst_Int idx = 0;

    // Create the garbage collector
    Nst_GGCList gen1 = { NULL, NULL, 0 };
    Nst_GGCList gen2 = { NULL, NULL, 0 };
    Nst_GGCList gen3 = { NULL, NULL, 0 };
    Nst_GGCList old_gen = { NULL, NULL, 0 };
    Nst_GarbageCollector ggc = { gen1, gen2, gen3, old_gen, 0 };

    // make_argv creates a tracked object
    nst_state.ggc = &ggc;

    Nst_StrObj *cwd = STR(nst_new_cstring_raw(_getcwd(cwd_buf, PATH_MAX), true));
    Nst_SeqObj *argv_obj = make_argv(argc, argv, filename == NULL ? (char *)"-c"
                                                                  : filename);
    Nst_VarTable *vt = nst_new_var_table(NULL, cwd, argv_obj);

    nst_state.traceback = &tb;
    nst_state.vt = &vt;
    nst_state.idx = &idx;
    nst_state.curr_path = &cwd;
    nst_state.argv = argv_obj;
    nst_state.v_stack = nst_new_val_stack();
    nst_state.f_stack = nst_new_call_stack();
    nst_state.c_stack = nst_new_catch_stack();
    nst_state.loaded_libs = LList_new();
    nst_state.lib_paths = LList_new();
    nst_state.lib_handles = LList_new();

    if ( filename != NULL )
    {
        char *path_main_file = NULL;
        nst_get_full_path(filename, &path_main_file, NULL);

        for ( char *p = path_main_file; *p; p++ )
        {
            if ( *p == '\\' )
                *p = '/';
        }

        LList_append(nst_state.lib_paths, path_main_file, true);
    }

    nst_push_func(
        nst_state.f_stack,
        main_func,
        nst_no_pos(),
        nst_no_pos(),
        NULL,
        0
    );
    nst_set_vt_func(main_func, vt->vars);
    nst_add_tracked_object((Nst_GGCObj *)vt->vars);

    complete_function(0);

    nst_dec_ref(main_func);
    if ( ERROR_OCCURRED )
    {
        nst_print_traceback(*nst_state.traceback);
        nst_dec_ref(nst_state.traceback->error.name);
        nst_dec_ref(nst_state.traceback->error.message);
    }

    // Freeing nst_state
    LList_destroy(nst_state.traceback->positions, free);
    nst_dec_ref(cwd);
    nst_dec_ref(argv_obj);
    nst_destroy_v_stack(nst_state.v_stack);
    nst_destroy_f_stack(nst_state.f_stack);
    nst_destroy_c_stack(nst_state.c_stack);
    for ( LLNode *n = nst_state.loaded_libs->head; n != NULL; n = n->next )
    {
        void (*free_lib_func)() = (void (*)())dlsym(n->value, "free_lib");
        if ( free_lib_func != NULL )
            free_lib_func();
    }
    LList_destroy(nst_state.loaded_libs, (LList_item_destructor)dlclose);
    LList_destroy(nst_state.lib_paths, free);
    LList_destroy(nst_state.lib_handles, (LList_item_destructor)nst_destroy_lib_handle);

    nst_delete_objects(&ggc);
}

static inline void destroy_call(Nst_FuncCall *call, Nst_Int offset)
{
    nst_dec_ref(nst_map_drop_str((*nst_state.vt)->vars, "_vars_"));
    nst_dec_ref((*nst_state.vt)->vars);
    if ( (*nst_state.vt)->global_table != NULL )
        nst_dec_ref((*nst_state.vt)->global_table);
    free(*nst_state.vt);

    nst_dec_ref(call->func);

    *nst_state.vt = call->vt;
    *nst_state.idx = call->idx + offset;
}

static void complete_function(size_t final_stack_size)
{
    if ( nst_state.f_stack->current_size == 0 )
        return;

    Nst_InstructionList *curr_inst_ls = nst_peek_func(nst_state.f_stack).func->body.bytecode;

    for ( ; nst_state.f_stack->current_size > final_stack_size; (*nst_state.idx)++ )
    {
        assert(curr_inst_ls != NULL);
        if ( *nst_state.idx >= (Nst_Int)curr_inst_ls->total_size )
        {
            // Free the function call
            Nst_FuncCall call = nst_pop_func(nst_state.f_stack);

            destroy_call(&call, 0);

            Nst_FuncObj *func = nst_peek_func(nst_state.f_stack).func;
            curr_inst_ls = func == NULL ? NULL : func->body.bytecode;
            continue;
        }

        Nst_RuntimeInstruction *inst = curr_inst_ls->instructions + *nst_state.idx;
        int inst_id = inst->id;
        run_instruction(inst);

        if ( ERROR_OCCURRED )
        {
            Nst_CatchFrame top_catch = nst_peek_catch(nst_state.c_stack);
            Nst_Obj *obj;

            size_t end_size = top_catch.f_stack_size;
            if ( end_size < final_stack_size )
                end_size = final_stack_size;

            while ( nst_state.f_stack->current_size > end_size )
            {
                Nst_FuncCall call = nst_pop_func(nst_state.f_stack);

                destroy_call(&call, 1);

                obj = nst_pop_val(nst_state.v_stack);
                while ( obj != NULL )
                {
                    nst_dec_ref(obj);
                    obj = nst_pop_val(nst_state.v_stack);
                }

                ADD_POSITIONS(call.start, call.end);
            }

            if ( end_size == final_stack_size )
                return;

            while ( nst_state.v_stack->current_size > top_catch.v_stack_size )
            {
                obj = nst_pop_val(nst_state.v_stack);
                if ( obj != NULL )
                    nst_dec_ref(obj);
            }
            *nst_state.idx = top_catch.inst_idx - 1;
            curr_inst_ls = nst_peek_func(nst_state.f_stack).func->body.bytecode;
        }

        // only OP_CALL and FOR_* can push a function on the call stack
        if ( inst_id == NST_IC_OP_CALL     || inst_id == NST_IC_FOR_START   ||
             inst_id == NST_IC_FOR_IS_DONE || inst_id == NST_IC_FOR_GET_VAL ||
             inst_id == NST_IC_FOR_ADVANCE )
            curr_inst_ls = nst_peek_func(nst_state.f_stack).func->body.bytecode;
    }
}

int nst_run_module(char *filename, Nst_SourceText *lib_src)
{
    // Compile and optimize the imported module

    Nst_Error error = { false, nst_no_pos(), nst_no_pos(), NULL, NULL };

    // The file is guaranteed to exist
    LList *tokens = nst_ftokenize(filename, lib_src, &error);

    if ( tokens == NULL )
    {
        _NST_SET_ERROR(
            GLOBAL_ERROR,
            error.start,
            error.end,
            error.name,
            error.message
        );
        return -1;
    }

    Nst_Node *ast = nst_parse(tokens, &error);
    if ( ast != NULL && opt_level >= 1 )
        ast = nst_optimize_ast(ast, &error);

    if ( ast == NULL )
    {
        _NST_SET_ERROR(
            GLOBAL_ERROR,
            error.start,
            error.end,
            error.name,
            error.message
        );
        return -1;
    }

    Nst_InstructionList *inst_ls = nst_compile(ast, true);
    if ( opt_level >= 2 )
        inst_ls = nst_optimize_bytecode(inst_ls, opt_level == 3, &error);
    if ( inst_ls == NULL )
    {
        _NST_SET_ERROR(
            GLOBAL_ERROR,
            error.start,
            error.end,
            error.name,
            error.message
        );
        return -1;
    }

    Nst_FuncObj *mod_func = FUNC(new_func(0, inst_ls));

    // Change the cwd
    Nst_StrObj *prev_path = *nst_state.curr_path;

    Nst_StrObj *path_str = make_cwd(filename);
    *nst_state.curr_path = path_str;

    int res = _chdir(path_str->value);
    assert(res == 0);

    nst_push_val(nst_state.v_stack, NULL);
    nst_push_func(
        nst_state.f_stack,
        mod_func,
        nst_no_pos(),
        nst_no_pos(),
        *nst_state.vt,
        *nst_state.idx - 1
    );
    *nst_state.idx = 0;
    CHANGE_VT(nst_new_var_table(NULL, path_str, nst_state.argv));

    nst_set_vt_func(mod_func, (*nst_state.vt)->vars);

    complete_function(nst_state.f_stack->current_size - 1);
    *nst_state.curr_path = prev_path;
    nst_dec_ref(path_str);
    nst_dec_ref(mod_func);

    res = _chdir(prev_path->value);
    assert(res == 0);

    if ( ERROR_OCCURRED )
        return -1;
    else
        return 0;
}

Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err)
{
    if ( NST_HAS_FLAG(func, NST_FLAG_FUNC_IS_C) )
        return func->body.c_func(func->arg_num, args, err);

    nst_push_val(nst_state.v_stack, NULL);
    nst_push_func(
        nst_state.f_stack,
        func,
        nst_no_pos(),
        nst_no_pos(),
        *nst_state.vt,
        *nst_state.idx - 1
    );

    Nst_VarTable *new_vt;
    if ( func->mod_globals != NULL )
        new_vt = nst_new_var_table(func->mod_globals, NULL, NULL);
    else if ( (*nst_state.vt)->global_table == NULL )
        new_vt = nst_new_var_table((*nst_state.vt)->vars, NULL, NULL);
    else
        new_vt = nst_new_var_table((*nst_state.vt)->global_table, NULL, NULL);

    for ( size_t i = 0, n = func->arg_num; i < n; i++ )
        nst_set_val(new_vt, func->args[i], args[i]);

    *nst_state.idx = 0;
    CHANGE_VT(new_vt);
    complete_function(nst_state.f_stack->current_size - 1);

    if ( ERROR_OCCURRED )
        return NULL;

    return nst_pop_val(nst_state.v_stack);
}

Nst_Obj *nst_run_func_context(Nst_FuncObj *func,
                              Nst_Int idx,
                              Nst_MapObj *vars,
                              Nst_MapObj *globals)
{
    assert(!NST_HAS_FLAG(func, NST_FLAG_FUNC_IS_C));

    nst_push_func(
        nst_state.f_stack,
        func,
        nst_no_pos(),
        nst_no_pos(),
        *nst_state.vt,
        *nst_state.idx - 1
    );

    Nst_VarTable *new_vt = (Nst_VarTable *)malloc(sizeof(Nst_VarTable));
    if ( new_vt == NULL ) return NULL;

    new_vt->vars = MAP(nst_inc_ref(vars));

    if ( globals == NULL )
    {
        if ( func->mod_globals != NULL )
            new_vt->global_table = func->mod_globals;
        else if ( (*nst_state.vt)->global_table == NULL )
            new_vt->global_table = (*nst_state.vt)->vars;
        else
            new_vt->global_table = (*nst_state.vt)->global_table;
    }
    else
        new_vt->global_table = globals;
    CHANGE_VT(new_vt);
    *nst_state.idx = idx;
    complete_function(nst_state.f_stack->current_size - 1);

    if ( ERROR_OCCURRED )
        return NULL;

    return nst_pop_val(nst_state.v_stack);
}

static inline void run_instruction(Nst_RuntimeInstruction *inst)
{
    switch ( inst->id )
    {
    case NST_IC_POP_VAL:      exe_pop_val();           break;
    case NST_IC_FOR_START:    exe_for_start(inst);     break;
    case NST_IC_FOR_IS_DONE:  exe_for_is_done(inst);   break;
    case NST_IC_FOR_GET_VAL:  exe_for_get_val(inst);   break;
    case NST_IC_FOR_ADVANCE:  exe_for_advance(inst);   break;
    case NST_IC_RETURN_VAL:   exe_return_val();        break;
    case NST_IC_RETURN_VARS:  exe_return_vars();       break;
    case NST_IC_SET_VAL_LOC:  exe_set_val_loc(inst);   break;
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
    case NST_IC_MAKE_ARR:
    case NST_IC_MAKE_VEC:     exe_make_seq(inst);      break;
    case NST_IC_MAKE_ARR_REP:
    case NST_IC_MAKE_VEC_REP: exe_make_seq_rep(inst);  break;
    case NST_IC_MAKE_MAP:     exe_make_map(inst);      break;
    case NST_IC_PUSH_CATCH:   exe_push_catch(inst);    break;
    case NST_IC_POP_CATCH:    exe_pop_catch();         break;
    case NST_IC_SAVE_ERROR:   exe_save_error();        break;
    }
}

static inline void exe_no_op() {}

static inline void exe_pop_val()
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_pop_val(nst_state.v_stack);
    nst_dec_ref(obj);
}

static inline void exe_for_inst(Nst_RuntimeInstruction *inst, Nst_IterObj *iter, Nst_FuncObj *func)
{
    if ( NST_HAS_FLAG(func, NST_FLAG_FUNC_IS_C) )
    {
        Nst_OpErr err = { NULL, NULL };
        Nst_Obj *res = func->body.c_func((size_t)inst->int_val, &iter->value, &err);

        if ( res == NULL )
        {
            _NST_SET_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                err.name,
                err.message
            );
        }
        else
        {
            nst_push_val(nst_state.v_stack, res);
            nst_dec_ref(res);
        }
    }
    else
    {
        nst_push_val(nst_state.v_stack, iter->value);
        nst_push_val(nst_state.v_stack, func);
        exe_op_call(inst);
    }
}

static inline void exe_for_start(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->start);
}

static inline void exe_for_is_done(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->is_done);
}

static inline void exe_for_get_val(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->get_val);
}

static inline void exe_for_advance(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->advance);
}

static inline void exe_return_val()
{
    Nst_Obj *result = nst_pop_val(nst_state.v_stack);
    Nst_Obj *obj = nst_pop_val(nst_state.v_stack);

    while ( obj != NULL )
    {
        nst_dec_ref(obj);
        obj = nst_pop_val(nst_state.v_stack);
    }

    nst_push_val(nst_state.v_stack, result);
    *nst_state.idx = nst_peek_func(nst_state.f_stack).func->body.bytecode->total_size;
    nst_dec_ref(result);
}

static inline void exe_return_vars()
{
    Nst_MapObj *vars = (*nst_state.vt)->vars;
    Nst_Obj *obj = nst_pop_val(nst_state.v_stack);

    while ( obj != NULL )
    {
        nst_dec_ref(obj);
        obj = nst_pop_val(nst_state.v_stack);
    }

    nst_push_val(nst_state.v_stack, vars);
    *nst_state.idx = nst_peek_func(nst_state.f_stack).func->body.bytecode->total_size;
}

static inline void exe_set_val_loc(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = nst_pop_val(nst_state.v_stack);
    nst_set_val(*nst_state.vt, inst->val, val);
    nst_dec_ref(val);
}

static inline void exe_jump(Nst_RuntimeInstruction *inst)
{
    *nst_state.idx = inst->int_val - 1;
}

static inline void exe_jumpif_t(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *result = nst_obj_cast(top_val, nst_t.Bool, NULL);
    nst_dec_ref(top_val);

    if ( result == nst_c.b_true )
        *nst_state.idx = inst->int_val - 1;
    nst_dec_ref(result);
}

static inline void exe_jumpif_f(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *result = nst_obj_cast(top_val, nst_t.Bool, NULL);
    nst_dec_ref(top_val);

    if ( result == nst_c.b_false )
        *nst_state.idx = inst->int_val - 1;
    nst_dec_ref(result);
}

static inline void exe_jumpif_zero(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = nst_peek_val(nst_state.v_stack);
    if ( AS_INT(val) == 0 )
        *nst_state.idx = inst->int_val - 1;
}

static inline void exe_type_check(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_peek_val(nst_state.v_stack);
    if ( obj->type != TYPE(inst->val) )
        _NST_SET_TYPE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            _nst_format_error(
                _NST_EM_EXPECTED_TYPES, "ss",
                STR(inst->val)->value,
                TYPE_NAME(obj)
            )
        );
}

static inline void exe_hash_check(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_peek_val(nst_state.v_stack);
    nst_hash_obj(obj);
    if ( obj->hash == -1 )
        _NST_SET_TYPE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            _nst_format_error(_NST_EM_UNHASHABLE_TYPE, "s", TYPE_NAME(obj))
        );
}

static inline void exe_set_val(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    nst_set_val(
        *nst_state.vt,
        inst->val,
        nst_peek_val(nst_state.v_stack)
    );
}

static inline void exe_get_val(Nst_RuntimeInstruction *inst)
{
    Nst_Obj *obj = nst_get_val(*nst_state.vt, inst->val);
    if ( obj == NULL )
        nst_push_val(nst_state.v_stack, nst_c.null);
    else
    {
        nst_push_val(nst_state.v_stack, obj);
        nst_dec_ref(obj);
    }
}

static inline void exe_push_val(Nst_RuntimeInstruction *inst)
{
    nst_push_val(nst_state.v_stack, inst->val);
}

static inline void exe_set_cont_val(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK_SIZE(3);
    Nst_Obj *idx = nst_pop_val(nst_state.v_stack);
    Nst_Obj *cont = nst_pop_val(nst_state.v_stack);
    Nst_Obj *val = nst_peek_val(nst_state.v_stack);

    if ( cont->type == nst_t.Array || cont->type == nst_t.Vector )
    {
        if ( idx->type != nst_t.Int )
        {
            _NST_SET_TYPE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                _nst_format_error(_NST_EM_EXPECTED_TYPE("Int"), "s", TYPE_NAME(idx))
            );

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        bool res = nst_set_value_seq(cont, AS_INT(idx), val);

        if ( !res )
        {
            _NST_SET_VALUE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                _nst_format_error(
                    cont->type == nst_t.Array ? _NST_EM_INDEX_OUT_OF_BOUNDS("Array")
                                            : _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
                    "iu",
                    AS_INT(idx),
                    SEQ(cont)->len
                )
            );
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
                _nst_format_error(_NST_EM_UNHASHABLE_TYPE, "s", TYPE_NAME(idx))
            );
        }

        nst_dec_ref(cont);
        nst_dec_ref(idx);
    }
    else
        _NST_SET_TYPE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            _nst_format_error(
                _NST_EM_EXPECTED_TYPE("Array', 'Vector', or 'Map"),
                "s",
                TYPE_NAME(cont)
            )
        );
}

static inline void exe_op_call(Nst_RuntimeInstruction *inst)
{
    Nst_Int arg_num = inst->int_val;

    CHECK_V_STACK_SIZE(arg_num + 1);
    Nst_FuncObj *func = FUNC(nst_pop_val(nst_state.v_stack));

    if ( (Nst_Int)(func->arg_num) != arg_num )
    {
        _NST_SET_CALL_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            _nst_format_error(
                _NST_EM_WRONG_ARG_NUM, "usis",
                func->arg_num, func->arg_num == 1 ? "" : "s",
                arg_num, arg_num == 1 ? "was" : "were"
            )
        );

        nst_dec_ref(func);
        return;
    }

    if ( NST_HAS_FLAG(func, NST_FLAG_FUNC_IS_C) )
    {
        Nst_OpErr err = { NULL, NULL };
        Nst_Obj **args;
        Nst_Obj *arg;
        bool args_allocated = false;

        if ( arg_num == 0 )
            args = NULL;
        else if ( arg_num == 1 )
        {
            arg = nst_pop_val(nst_state.v_stack);
            args = &arg;
        }
        else
        {
            args = (Nst_Obj **)malloc((size_t)(sizeof(Nst_Obj *) * arg_num));
            if ( args == NULL )
                return;

            for ( Nst_Int i = arg_num - 1; i >= 0; i-- )
                args[i] = nst_pop_val(nst_state.v_stack);
            args_allocated = true;
        }

        Nst_Obj *res = func->body.c_func((size_t)arg_num, args, &err);

        for ( Nst_Int i = 0; i < arg_num; i++ )
            nst_dec_ref(args[i]);

        if ( args_allocated )
            free(args);

        if ( res == NULL )
            SET_OP_ERROR(inst->start, inst->end, err);
        else
        {
            nst_push_val(nst_state.v_stack, res);
            nst_dec_ref(res);
        }

        nst_dec_ref(func);
        return;
    }

    bool res = nst_push_func(
        nst_state.f_stack,
        func,
        inst->start,
        inst->end,
        *nst_state.vt,
        *nst_state.idx
    );
    *nst_state.idx = -1;
    nst_dec_ref(func);

    if ( !res )
    {
        _NST_SET_RAW_CALL_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            _NST_EM_CALL_STACK_SIZE_EXCEEDED
        );

        return;
    }

    Nst_VarTable *new_vt;

    if ( func->mod_globals != NULL )
        new_vt = nst_new_var_table(func->mod_globals, NULL, NULL);
    else if ( (*nst_state.vt)->global_table == NULL )
        new_vt = nst_new_var_table((*nst_state.vt)->vars, NULL, NULL);
    else
        new_vt = nst_new_var_table((*nst_state.vt)->global_table, NULL, NULL);

    for ( Nst_Int i = 0; i < arg_num; i++ )
    {
        Nst_Obj *val = nst_pop_val(nst_state.v_stack);
        nst_set_val(new_vt, func->args[arg_num - i - 1], val);
        nst_dec_ref(val);
    }
    CHANGE_VT(new_vt);
    nst_push_val(nst_state.v_stack, NULL);
}

static inline void exe_op_cast(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *type = nst_pop_val(nst_state.v_stack);

    Nst_OpErr err = { NULL, NULL };
    Nst_Obj *res = nst_obj_cast(val, type, &err);

    if ( res == NULL )
        SET_OP_ERROR(inst->start, inst->end, err);
    else
    {
        nst_push_val(nst_state.v_stack, res);
        nst_dec_ref(res);
    }

    nst_dec_ref(val);
    nst_dec_ref(type);
}

static inline void exe_op_range(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK_SIZE(inst->int_val);
    Nst_Obj *stop = nst_pop_val(nst_state.v_stack);
    Nst_Obj *step = NULL;
    Nst_Obj *start = NULL;

    if ( inst->int_val == 3 )
    {
        start = nst_pop_val(nst_state.v_stack);
        step  = nst_pop_val(nst_state.v_stack);
    }
    else
    {
        start = nst_pop_val(nst_state.v_stack);

        if ( AS_INT(start) <= AS_INT(stop) )
            step = nst_new_int(1);
        else
            step = nst_new_int(-1);
    }

    Nst_OpErr err = { NULL, NULL };
    Nst_Obj *iter = _nst_obj_range(start, stop, step, &err);

    nst_dec_ref(start);
    nst_dec_ref(stop);
    nst_dec_ref(step);

    if ( iter == NULL )
        SET_OP_ERROR(inst->start, inst->end, err);
    else
    {
        nst_push_val(nst_state.v_stack, iter);
        nst_dec_ref(iter);
    }
}

static inline void exe_throw_err(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *message = nst_pop_val(nst_state.v_stack);
    Nst_Obj *name = nst_pop_val(nst_state.v_stack);

    _NST_SET_ERROR(GLOBAL_ERROR, inst->start, inst->end, name, message);

    nst_dec_ref(name);
    nst_dec_ref(message);
}

static inline void exe_stack_op(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *ob2 = nst_pop_val(nst_state.v_stack);
    Nst_Obj *ob1 = nst_pop_val(nst_state.v_stack);
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
        SET_OP_ERROR(inst->start, inst->end, err);
    else
    {
        nst_push_val(nst_state.v_stack, res);
        nst_dec_ref(res);
    }

    nst_dec_ref(ob1);
    nst_dec_ref(ob2);
}

static inline void exe_local_op(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_pop_val(nst_state.v_stack);
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

    nst_push_val(nst_state.v_stack, res);
    nst_dec_ref(obj);
    nst_dec_ref(res);
}

static inline void exe_op_import(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_Obj *name = nst_pop_val(nst_state.v_stack);
    Nst_OpErr err = { NULL, NULL };
    Nst_Obj *res = nst_obj_import(name, &err);

    if ( res == NULL )
    {
        SET_OP_ERROR(inst->start, inst->end, err);
        nst_dec_ref(name);
        return;
    }

    nst_push_val(nst_state.v_stack, res);
    nst_dec_ref(name);
    nst_dec_ref(res);
}

static inline void exe_op_extract(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *idx = nst_pop_val(nst_state.v_stack);
    Nst_Obj *cont = nst_pop_val(nst_state.v_stack);
    Nst_Obj *res = NULL;

    if ( cont->type == nst_t.Array || cont->type == nst_t.Vector )
    {
        if ( idx->type != nst_t.Int )
        {
            _NST_SET_TYPE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                _nst_format_error(_NST_EM_EXPECTED_TYPE("Int"), "s", TYPE_NAME(idx))
            );

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        res = nst_get_value_seq(cont, AS_INT(idx));

        if ( res == NULL )
        {
            _NST_SET_VALUE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                _nst_format_error(
                    cont->type == nst_t.Array ? _NST_EM_INDEX_OUT_OF_BOUNDS("Array")
                                              : _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
                    "iu",
                    AS_INT(idx),
                    SEQ(cont)->len
                )
            );

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        nst_push_val(nst_state.v_stack, res);
    }
    else if ( cont->type == nst_t.Map )
    {
        res = nst_map_get(cont, idx);

        if ( res == NULL )
        {
            if ( idx->hash != -1 )
                nst_push_val(nst_state.v_stack, nst_c.null);
            else
            {
                _NST_SET_VALUE_ERROR(
                    GLOBAL_ERROR,
                    inst->start,
                    inst->start,
                    _nst_format_error(_NST_EM_UNHASHABLE_TYPE, "s", TYPE_NAME(idx))
                );
            }

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }
        else
            nst_push_val(nst_state.v_stack, res);
    }
    else if ( cont->type == nst_t.Str )
    {
        if ( idx->type != nst_t.Int )
        {
            _NST_SET_TYPE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                _nst_format_error(_NST_EM_EXPECTED_TYPE("Int"), "s", TYPE_NAME(idx))
            );

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        res = nst_string_get_idx(cont, AS_INT(idx));

        if ( res == NULL )
        {
            _NST_SET_VALUE_ERROR(
                GLOBAL_ERROR,
                inst->start,
                inst->end,
                _nst_format_error(
                    _NST_EM_INDEX_OUT_OF_BOUNDS("Str"),
                    "iu",
                    AS_INT(idx),
                    STR(cont)->len
                )
            );

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }
        else
            nst_push_val(nst_state.v_stack, res);
    }
    else
    {
        _NST_SET_TYPE_ERROR(
            GLOBAL_ERROR,
            inst->start,
            inst->end,
            _nst_format_error(
                _NST_EM_EXPECTED_TYPE("Array', 'Vector', 'Map' or 'Str"),
                "s",
                TYPE_NAME(cont)
            )
        );

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
    Nst_Obj *obj = nst_peek_val(nst_state.v_stack);
    AS_INT(obj) -= 1;
}

static inline void exe_new_obj()
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_peek_val(nst_state.v_stack);
    Nst_Obj *new_obj = nst_new_int(AS_INT(obj));
    nst_push_val(nst_state.v_stack, new_obj);
    nst_dec_ref(new_obj);
}

static inline void exe_dup()
{
    CHECK_V_STACK;
    nst_push_val(nst_state.v_stack, nst_peek_val(nst_state.v_stack));
}

static inline void exe_make_seq(Nst_RuntimeInstruction *inst)
{
    Nst_Int seq_size = inst->int_val;
    Nst_Obj *seq = inst->id == NST_IC_MAKE_ARR ? nst_new_array((size_t)seq_size)
                                              : nst_new_vector((size_t)seq_size);
    CHECK_V_STACK_SIZE(seq_size);

    for ( Nst_Int i = 1; i <= seq_size; i++ )
    {
        Nst_Obj *curr_obj = nst_pop_val(nst_state.v_stack);
        nst_set_value_seq(seq, seq_size - i, curr_obj);
        nst_dec_ref(curr_obj);
    }

    nst_push_val(nst_state.v_stack, seq);
    nst_dec_ref(seq);
}

static inline void exe_make_seq_rep(Nst_RuntimeInstruction *inst)
{
    Nst_Int seq_size = inst->int_val;
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *size_obj = nst_pop_val(nst_state.v_stack);
    Nst_Obj *val = nst_pop_val(nst_state.v_stack);

    Nst_Int size = AS_INT(size_obj);
    nst_dec_ref(size_obj);
    Nst_Obj *seq = inst->id == NST_IC_MAKE_ARR_REP ? nst_new_array((size_t)size)
                                                  : nst_new_vector((size_t)size);

    for ( Nst_Int i = 1; i <= size; i++ )
        nst_set_value_seq(seq, seq_size - i, val);

    nst_push_val(nst_state.v_stack, seq);
    nst_dec_ref(seq);
    nst_dec_ref(val);
}

static inline void exe_make_map(Nst_RuntimeInstruction *inst)
{
    Nst_Int map_size = inst->int_val;
    Nst_Obj *map = nst_new_map();
    CHECK_V_STACK_SIZE(map_size);
    size_t stack_size = nst_state.v_stack->current_size;
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
    nst_state.v_stack->current_size -= (size_t)map_size;
    nst_push_val(nst_state.v_stack, map);
    nst_dec_ref(map);
}

static inline void exe_push_catch(Nst_RuntimeInstruction* inst)
{
    nst_push_catch(nst_state.c_stack,
                   inst->int_val,
                   nst_state.v_stack->current_size,
                   nst_state.f_stack->current_size);
}

static inline void exe_pop_catch()
{
    nst_pop_catch(nst_state.c_stack);
}

static inline void exe_save_error()
{
    assert(GLOBAL_ERROR->occurred);

    Nst_Obj *err_map = nst_new_map();
    nst_map_set_str(err_map, "name", GLOBAL_ERROR->name);
    nst_map_set_str(err_map, "message", GLOBAL_ERROR->message);

    nst_state.traceback->error.occurred = false;
    nst_dec_ref(GLOBAL_ERROR->name);
    nst_dec_ref(GLOBAL_ERROR->message);

    LList_empty(nst_state.traceback->positions, free);

    nst_push_val(nst_state.v_stack, err_map);
    nst_dec_ref(err_map);
}

size_t nst_get_full_path(char *file_path, char **buf, char **file_part)
{
    char *path = (char *)malloc(sizeof(char) * PATH_MAX);
    if ( path == NULL )
        return 0;

#if defined(_WIN32) || defined(WIN32)

    DWORD path_len = GetFullPathNameA(file_path, PATH_MAX, path, file_part);
    if ( path_len > PATH_MAX )
    {
        free(path);
        path = (char *)malloc(sizeof(char) * path_len);
        if ( path == NULL )
            return 0;
        path_len = GetFullPathNameA(file_path, path_len, path, file_part);
    }

    *buf = path;
    return path_len;

#else

    path = realpath(file_path, path);

    if ( file_part != NULL )
    {
        *file_part = strrchr(path, '/');

        if ( !*file_part )
            *file_part = path;
        else
            (*file_part)++;
    }

    *buf = path;
    return strlen(path);
#endif
}


static Nst_SeqObj *make_argv(int argc, char **argv, char *filename)
{
    Nst_SeqObj *args = SEQ(nst_new_array(argc + 1));

    Nst_Obj *val = nst_new_cstring_raw(filename, false);
    nst_set_value_seq(args, 0, val);
    nst_dec_ref(val);

    for ( int i = 0; i < argc; i++ )
    {
        val = nst_new_cstring_raw(argv[i], false);
        nst_set_value_seq(args, i + 1, val);
        nst_dec_ref(val);
    }

    return args;
}

static Nst_StrObj *make_cwd(char *file_path)
{
    char *path = NULL;
    char *file_part = NULL;

    nst_get_full_path(file_path, &path, &file_part);

    *(file_part - 1) = 0;

    return STR(nst_new_string(path, file_part - path - 1, true));
}

void nst_destroy_lib_handle(Nst_LibHandle *handle)
{
    if ( handle->text != NULL )
    {
        free(handle->text->text);
        free(handle->text->lines);
        free(handle->text);
    }
    free(handle->path);
    free(handle);
}
