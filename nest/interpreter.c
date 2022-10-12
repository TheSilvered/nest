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

#define SET_ERROR(err_macro, start, end, message) \
    do { \
        err_macro(&(nst_state.traceback->error), start, end, message); \
        *nst_state.error_occurred = true; \
    } while ( 0 )

#define SET_OP_ERROR(start_pos, end_pos, op_err) \
    do { \
        if ( nst_state.traceback->error.start.filename == NULL ) \
        { \
            LList_append(nst_state.traceback->positions, &(start_pos), false); \
            LList_append(nst_state.traceback->positions, &(end_pos), false); \
            break; \
        } \
        nst_state.traceback->error.start = start_pos; \
        nst_state.traceback->error.end = end_pos; \
        nst_state.traceback->error.name = (char *)op_err.name; \
        nst_state.traceback->error.message = (char *)op_err.message; \
        *nst_state.error_occurred = true; \
    } while ( 0 )

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

static Nst_SeqObj *make_argv(int argc, char **argv, char *filename);
static Nst_StrObj *make_cwd(char *file_path);

void nst_run(Nst_FuncObj *main_func, int argc, char **argv, char *filename, int opt_lvl)
{
    opt_level = opt_lvl;

    // nst_state global variable initialization
    char *cwd_buf = (char *)malloc(sizeof(char) * PATH_MAX);
    Nst_VarTable **vt = (Nst_VarTable **)malloc(sizeof(Nst_VarTable *));
    Nst_StrObj **curr_path = (Nst_StrObj **)malloc(sizeof(Nst_StrObj *));
    if ( cwd_buf        == NULL ||
         vt             == NULL ||
         curr_path      == NULL )
        return;

    bool error_occurred = false;
    Nst_StrObj *cwd = AS_STR(nst_new_string_raw(_getcwd(cwd_buf, PATH_MAX), true));
    Nst_Traceback tb;
    tb.error.start = nst_no_pos();
    tb.error.end = nst_no_pos();
    tb.error.name = NULL;
    tb.error.message = NULL;
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

    Nst_SeqObj *argv_obj = make_argv(argc, argv, filename);
    nst_state.traceback = &tb;
    nst_state.vt = vt;
    *nst_state.vt = nst_new_var_table(NULL, cwd, argv_obj);
    nst_state.idx = &idx;
    nst_state.error_occurred = &error_occurred;
    nst_state.curr_path = curr_path;
    *nst_state.curr_path = cwd;
    nst_state.argv = argv_obj;
    nst_state.v_stack = nst_new_val_stack();
    nst_state.f_stack = nst_new_call_stack();
    nst_state.loaded_libs = LList_new();
    nst_state.lib_paths = LList_new();
    nst_state.lib_handles = LList_new();

    nst_push_func(
        nst_state.f_stack,
        main_func,
        nst_no_pos(),
        nst_no_pos(),
        NULL,
        0
    );

    complete_function(0);

    if ( *(nst_state.error_occurred) )
    {
        Nst_FuncCall *calls = nst_state.f_stack->stack;
        for ( Nst_Int i = 0, n = nst_state.f_stack->current_size - 1; i < n; i++ )
        {
            if ( calls[n - i].start.filename == NULL ) // i.e. there is no valid position
                continue;

            LList_append(nst_state.traceback->positions, &(calls[n - i].start), false);
            LList_append(nst_state.traceback->positions, &(calls[n - i].start), false);
        }

        nst_print_traceback(*nst_state.traceback);
        free(&(nst_state.traceback->error));

        nst_dec_ref((*nst_state.vt)->vars);
        free(*nst_state.vt);
    }

    // Freeing nst_state
    LList_destroy(nst_state.traceback->positions, NULL);
    free(vt);
    free(curr_path);
    nst_dec_ref(cwd);
    nst_dec_ref(argv_obj);
    nst_destroy_v_stack(nst_state.v_stack);
    nst_destroy_f_stack(nst_state.f_stack);
    for ( LLNode *n = nst_state.loaded_libs->head; n != NULL; n = n->next )
    {
        void (*free_lib_func)() = (void (*)())dlsym(n->value, "free_lib");
        if ( free_lib_func != NULL )
            free_lib_func();
    }
    LList_destroy(nst_state.loaded_libs, (LList_item_destructor)dlclose);
    LList_destroy(nst_state.lib_paths, free);
    LList_destroy(nst_state.lib_handles, (LList_item_destructor)nst_destroy_lib_handle);

    delete_objects(&ggc);
}

static void complete_function(size_t final_stack_size)
{
    if ( nst_state.f_stack->current_size == 0 )
        return;

    Nst_InstructionList *curr_inst_ls = nst_peek_func(nst_state.f_stack).func->body;

    for ( ; nst_state.f_stack->current_size > final_stack_size; (*nst_state.idx)++ )
    {
        if ( *nst_state.idx >= (Nst_Int)curr_inst_ls->total_size )
        {
            // Free the function call
            Nst_FuncCall call = nst_pop_func(nst_state.f_stack);
            Nst_MapObj *vars_to_del = (*nst_state.vt)->vars;

            nst_dec_ref(nst_map_drop_str(vars_to_del, "_vars_"));
            nst_dec_ref(vars_to_del);

            free(*nst_state.vt);
            nst_dec_ref(call.func);
            *nst_state.vt = call.vt;
            *nst_state.idx = call.idx;
            Nst_FuncObj *func = nst_peek_func(nst_state.f_stack).func;
            curr_inst_ls = func == NULL ? NULL : func->body;
            continue;
        }

        Nst_RuntimeInstruction *inst = curr_inst_ls->instructions + *nst_state.idx;
        int inst_id = inst->id;
        run_instruction(inst);

        if ( *(nst_state.error_occurred) )
            break;

        // only OP_CALL and FOR_* can push a function on the call stack
        if ( inst_id == NST_IC_OP_CALL     || inst_id == NST_IC_FOR_START   ||
             inst_id == NST_IC_FOR_IS_DONE || inst_id == NST_IC_FOR_GET_VAL ||
             inst_id == NST_IC_FOR_ADVANCE )
            curr_inst_ls = nst_peek_func(nst_state.f_stack).func->body;
    }
}

int nst_run_module(char *filename, char **lib_text)
{
    // Compile and optimize the imported module

    // The file is guaranteed to exist
    LList *tokens = nst_ftokenize(filename, lib_text);
    Nst_Node *ast = nst_parse(tokens);
    if ( ast != NULL && opt_level >= 1 )
        ast = nst_optimize_ast(ast);

    if ( ast == NULL )
        return -1;

    Nst_InstructionList *inst_ls = nst_compile(ast, true);
    if ( opt_level >= 2 )
        inst_ls = nst_optimize_bytecode(inst_ls, opt_level == 3);
    if ( inst_ls == NULL )
        return -1;

    Nst_FuncObj *mod_func = AS_FUNC(new_func(0));
    mod_func->body = inst_ls;

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
    *nst_state.vt = nst_new_var_table(NULL, path_str, nst_state.argv);

    _nst_set_global_vt(mod_func, (*nst_state.vt)->vars);

    complete_function(nst_state.f_stack->current_size - 1);
    *nst_state.curr_path = prev_path;
    nst_dec_ref(path_str);
    nst_dec_ref(mod_func);

    res = _chdir(prev_path->value);
    assert(res == 0);

    if ( *nst_state.error_occurred )
    {
        Nst_FuncCall call = nst_pop_func(nst_state.f_stack);
        nst_dec_ref(nst_map_drop_str((*nst_state.vt)->vars, "_vars_"));
        nst_dec_ref((*nst_state.vt)->vars);
        free(*nst_state.vt);
        nst_dec_ref(call.func);
        *nst_state.vt = call.vt;
        *nst_state.idx = call.idx;
        return -1;
    }
    else
        return 0;
}

Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err)
{
    if ( func->cbody != NULL )
        return func->cbody(func->arg_num, args, err);

    assert(func->body != NULL);

    Nst_VarTable *new_vt;
    if ( func->mod_globals != NULL )
        new_vt = nst_new_var_table(func->mod_globals, NULL, NULL);
    if ( (*nst_state.vt)->global_table == NULL )
        new_vt = nst_new_var_table((*nst_state.vt)->vars, NULL, NULL);
    else
        new_vt = nst_new_var_table((*nst_state.vt)->global_table, NULL, NULL);

    for ( size_t i = 0, n = func->arg_num; i < n; i++ )
    {
        nst_set_val(new_vt, func->args[i], args[i]);
        nst_dec_ref(args[i]);
    }

    nst_push_val(nst_state.v_stack, NULL);
    nst_push_func(
        nst_state.f_stack,
        func,
        nst_no_pos(),
        nst_no_pos(),
        *nst_state.vt,
        *nst_state.idx - 1
    );
    *nst_state.idx = 0;
    *nst_state.vt = new_vt;
    complete_function(nst_state.f_stack->current_size - 1);

    if ( *nst_state.error_occurred )
        return NULL;

    return nst_pop_val(nst_state.v_stack);
}

static inline void run_instruction(Nst_RuntimeInstruction *inst)
{
    switch ( inst->id )
    {
    case NST_IC_POP_VAL:      exe_pop_val();              break;
    case NST_IC_FOR_START:    exe_for_start(inst);        break;
    case NST_IC_FOR_IS_DONE:  exe_for_is_done(inst);      break;
    case NST_IC_FOR_GET_VAL:  exe_for_get_val(inst);      break;
    case NST_IC_FOR_ADVANCE:  exe_for_advance(inst);      break;
    case NST_IC_RETURN_VAL:   exe_return_val();           break;
    case NST_IC_RETURN_VARS:  exe_return_vars();          break;
    case NST_IC_SET_VAL_LOC:  exe_set_val_loc(inst);      break;
    case NST_IC_JUMP:         exe_jump(inst);             break;
    case NST_IC_JUMPIF_T:     exe_jumpif_t(inst);         break;
    case NST_IC_JUMPIF_F:     exe_jumpif_f(inst);         break;
    case NST_IC_JUMPIF_ZERO:  exe_jumpif_zero(inst);      break;
    case NST_IC_TYPE_CHECK:   exe_type_check(inst);       break;
    case NST_IC_HASH_CHECK:   exe_hash_check(inst);       break;
    case NST_IC_SET_VAL:      exe_set_val(inst);          break;
    case NST_IC_GET_VAL:      exe_get_val(inst);          break;
    case NST_IC_PUSH_VAL:     exe_push_val(inst);         break;
    case NST_IC_SET_CONT_VAL: exe_set_cont_val(inst);     break;
    case NST_IC_OP_CALL:      exe_op_call(inst);          break;
    case NST_IC_OP_CAST:      exe_op_cast(inst);          break;
    case NST_IC_OP_RANGE:     exe_op_range(inst);         break;
    case NST_IC_STACK_OP:     exe_stack_op(inst);         break;
    case NST_IC_LOCAL_OP:     exe_local_op(inst);         break;
    case NST_IC_OP_IMPORT:    exe_op_import(inst);        break;
    case NST_IC_OP_EXTRACT:   exe_op_extract(inst);       break;
    case NST_IC_DEC_INT:      exe_dec_int();              break;
    case NST_IC_NEW_OBJ:      exe_new_obj();              break;
    case NST_IC_DUP:          exe_dup();                  break;
    case NST_IC_MAKE_ARR:
    case NST_IC_MAKE_VEC:     exe_make_seq(inst);         break;
    case NST_IC_MAKE_ARR_REP:
    case NST_IC_MAKE_VEC_REP: exe_make_seq_rep(inst);     break;
    case NST_IC_MAKE_MAP:     exe_make_map(inst);         break;
    }
}

static inline void exe_no_op() {}

static inline void exe_pop_val()
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_pop_val(nst_state.v_stack);
    if (obj == NULL)
        printf("hi\n");
    nst_dec_ref(obj);
}

static inline void exe_for_inst(Nst_RuntimeInstruction *inst, Nst_IterObj *iter, Nst_FuncObj *func)
{
    if ( func->cbody != NULL )
    {
        Nst_OpErr err = { NULL, NULL };
        Nst_Obj *res = func->cbody((size_t)inst->int_val, &iter->value, &err);

        if ( res == NULL )
        {
            SET_ERROR(
                _NST_SET_GENERAL_ERROR,
                inst->start,
                inst->end,
                err.message
            );
            nst_state.traceback->error.name = (char *)err.name;
        }
        else
        {
            nst_push_val(nst_state.v_stack, res);
            nst_dec_ref(res);
        }
    }
    else
    {
        nst_push_val(nst_state.v_stack, NULL);
        nst_push_val(nst_state.v_stack, iter->value);
        nst_push_val(nst_state.v_stack, func);
        exe_op_call(inst);
    }
}

static inline void exe_for_start(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = AS_ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->start);
}

static inline void exe_for_is_done(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = AS_ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->is_done);
}

static inline void exe_for_get_val(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = AS_ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->get_val);
}

static inline void exe_for_advance(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = AS_ITER(nst_peek_val(nst_state.v_stack));
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
    *nst_state.idx = nst_peek_func(nst_state.f_stack).func->body->total_size;
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
    *nst_state.idx = nst_peek_func(nst_state.f_stack).func->body->total_size;
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
    Nst_Obj *result = nst_obj_cast(top_val, nst_t_bool, NULL);
    nst_dec_ref(top_val);

    if ( result == nst_true )
        *nst_state.idx = inst->int_val - 1;
    nst_dec_ref(result);
}

static inline void exe_jumpif_f(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *result = nst_obj_cast(top_val, nst_t_bool, NULL);
    nst_dec_ref(top_val);

    if ( result == nst_false )
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
    if ( obj->type != inst->val )
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            inst->start,
            inst->end,
            _nst_format_types_error(
                EXPECTED_TYPES,
                AS_STR(inst->val)->value,
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
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            inst->start,
            inst->end,
            _nst_format_type_error(UNHASHABLE_TYPE, TYPE_NAME(obj))
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
        nst_push_val(nst_state.v_stack, nst_null);
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

    if ( cont->type == nst_t_arr || cont->type == nst_t_vect )
    {
        if ( idx->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                inst->start,
                inst->end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), TYPE_NAME(idx))
            );

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        bool res = nst_set_value_seq(cont, AS_INT(idx), val);

        if ( !res )
        {
            SET_ERROR(
                _NST_SET_VALUE_ERROR,
                inst->start,
                inst->end,
                _nst_format_idx_error(
                    cont->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                            : INDEX_OUT_OF_BOUNDS("Vector"),
                    AS_INT(idx),
                    AS_SEQ(cont)->len
                )
            );
        }

        nst_dec_ref(cont);
        nst_dec_ref(idx);
    }
    else if ( cont->type == nst_t_map )
    {
        bool res = nst_map_set(cont, idx, val);
        if ( !res )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                inst->start,
                inst->end,
                _nst_format_type_error(UNHASHABLE_TYPE, TYPE_NAME(idx))
            );
        }

        nst_dec_ref(cont);
        nst_dec_ref(idx);
    }
    else
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            inst->start,
            inst->end,
            _nst_format_type_error(
                EXPECTED_TYPE("Array', 'Vector', 'Map' or 'Str"),
                TYPE_NAME(cont)
            )
        );
}

static inline void exe_op_call(Nst_RuntimeInstruction *inst)
{
    Nst_Int arg_num = inst->int_val;

    CHECK_V_STACK_SIZE(arg_num + 1);
    Nst_FuncObj *func = AS_FUNC(nst_pop_val(nst_state.v_stack));

    if ( (Nst_Int)(func->arg_num) != arg_num )
    {
        SET_ERROR(
            _NST_SET_CALL_ERROR,
            inst->start,
            inst->end,
            arg_num > (Nst_Int)func->arg_num ? TOO_MANY_ARGS_FUNC
                                             : TOO_FEW_ARGS_FUNC
        );

        nst_dec_ref(func);
        return;
    }

    if ( func->cbody != NULL )
    {
        Nst_OpErr err = { "", "" };
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
        nst_pop_val(nst_state.v_stack); // removes the NULL separator for the call

        Nst_Obj *res = func->cbody((size_t)arg_num, args, &err);

        for ( Nst_Int i = 0; i < arg_num; i++ )
            nst_dec_ref(args[i]);

        if ( args_allocated )
            free(args);

        if ( res == NULL )
        {
            if ( !*nst_state.error_occurred )
            {
                SET_ERROR(
                    _NST_SET_GENERAL_ERROR,
                    inst->start,
                    inst->end,
                    err.message
                );
                nst_state.traceback->error.name = (char *)err.name;
            }
            else
            {
                LList_append(nst_state.traceback->positions, &inst->start, false);
                LList_append(nst_state.traceback->positions, &inst->end, false);
            }
        }
        else
        {
            nst_push_val(nst_state.v_stack, res);
            nst_dec_ref(res);
        }

        nst_dec_ref(func);
        return;
    }

    assert(func->body != NULL);

    bool res = nst_push_func(
        nst_state.f_stack,
        func,
        inst->start,
        inst->end,
        *nst_state.vt,
        *nst_state.idx
    );
    *nst_state.idx = -1;

    if ( !res )
    {
        SET_ERROR(
            _NST_SET_CALL_ERROR,
            inst->start,
            inst->end,
            CALL_STACK_SIZE_EXCEEDED
        );

        nst_dec_ref(func);
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
    *nst_state.vt = new_vt;
}

static inline void exe_op_cast(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *type = nst_pop_val(nst_state.v_stack);

    Nst_OpErr err = { "", "" };
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
        step = nst_pop_val(nst_state.v_stack);
        start = nst_pop_val(nst_state.v_stack);
    }
    else
    {
        start = nst_pop_val(nst_state.v_stack);

        if ( AS_INT(start) <= AS_INT(stop) )
            step = nst_new_int(1);
        else
            step = nst_new_int(-1);
    }

    Nst_Obj *idx = nst_new_int(0);

    Nst_Obj *data_seq = nst_new_array(4);
    nst_set_value_seq(data_seq, 0, idx);
    nst_set_value_seq(data_seq, 1, start);
    nst_set_value_seq(data_seq, 2, stop);
    nst_set_value_seq(data_seq, 3, step);

    nst_dec_ref(idx);
    nst_dec_ref(start);
    nst_dec_ref(stop);
    nst_dec_ref(step);

    Nst_Obj *iter = nst_new_iter(
        AS_FUNC(new_cfunc(1, nst_num_iter_start)),
        AS_FUNC(new_cfunc(1, nst_num_iter_advance)),
        AS_FUNC(new_cfunc(1, nst_num_iter_is_done)),
        AS_FUNC(new_cfunc(1, nst_num_iter_get_val)),
        data_seq
    );

    nst_push_val(nst_state.v_stack, iter);
    nst_dec_ref(iter);
}

static inline void exe_stack_op(Nst_RuntimeInstruction *inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *ob2 = nst_pop_val(nst_state.v_stack);
    Nst_Obj *ob1 = nst_pop_val(nst_state.v_stack);
    Nst_Obj *res = NULL;
    Nst_OpErr err = { "", "" };

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
    Nst_OpErr err = { "", "" };

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
    Nst_OpErr err = { "", "" };
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

    if ( cont->type == nst_t_arr || cont->type == nst_t_vect )
    {
        if ( idx->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                inst->start,
                inst->end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), TYPE_NAME(idx))
            );

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        res = nst_get_value_seq(cont, AS_INT(idx));

        if ( res == NULL )
        {
            SET_ERROR(
                _NST_SET_VALUE_ERROR,
                inst->start,
                inst->end,
                _nst_format_idx_error(
                    cont->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                            : INDEX_OUT_OF_BOUNDS("Vector"),
                    AS_INT(idx),
                    AS_SEQ(cont)->len
                )
            );

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        nst_push_val(nst_state.v_stack, res);
    }
    else if ( cont->type == nst_t_map )
    {
        res = nst_map_get(cont, idx);

        if ( res == NULL )
        {
            if ( idx->hash != -1 )
                nst_push_val(nst_state.v_stack, nst_null);
            else
            {
                SET_ERROR(
                    _NST_SET_VALUE_ERROR,
                    inst->start,
                    inst->start,
                    _nst_format_type_error(UNHASHABLE_TYPE, TYPE_NAME(idx))
                );
            }

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }
        else
            nst_push_val(nst_state.v_stack, res);
    }
    else if ( cont->type == nst_t_str )
    {
        if ( idx->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                inst->start,
                inst->end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), TYPE_NAME(idx))
            );

            nst_dec_ref(cont);
            nst_dec_ref(idx);
            return;
        }

        res = nst_string_get_idx(cont, AS_INT(idx));

        if ( res == NULL )
        {
            SET_ERROR(
                _NST_SET_VALUE_ERROR,
                inst->start,
                inst->end,
                _nst_format_idx_error(
                    INDEX_OUT_OF_BOUNDS("Str"),
                    AS_INT(idx),
                    AS_SEQ(cont)->len
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
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            inst->start,
            inst->end,
            _nst_format_type_error(
                EXPECTED_TYPE("Array', 'Vector' or 'Map"),
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

size_t nst_get_full_path(char *file_path, char **buf, char **file_part)
#if defined(_WIN32) || defined(WIN32)

{
    char *path = (char *)malloc(sizeof(char) * PATH_MAX);
    if ( path == NULL )
        return 0;

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
}

#else

{
    char *path = (char *)malloc(sizeof(char) * PATH_MAX);
    if ( path == NULL )
        return 0;

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
}

#endif

static Nst_SeqObj *make_argv(int argc, char **argv, char *filename)
{
    Nst_SeqObj *args = AS_SEQ(nst_new_array(argc + 1));

    Nst_Obj *val = nst_new_string_raw(filename, false);
    nst_set_value_seq(args, 0, val);
    nst_dec_ref(val);

    for ( int i = 0; i < argc; i++ )
    {
        val = nst_new_string_raw(argv[i], false);
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

    return AS_STR(nst_new_string(path, file_part - path, true));
}

void nst_destroy_lib_handle(Nst_LibHandle *handle)
{
    if ( handle->text != NULL )
        free(handle->text);
    free(handle);
}
