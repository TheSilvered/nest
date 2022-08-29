#include <assert.h>
#include <direct.h>
#include <errno.h>
#include <stdlib.h>
#include "error_internal.h"
#include "hash.h"
#include "interpreter_ast.h"
#include "lexer.h"
#include "lib_import.h"
#include "llist.h"
#include "nst_types.h"
#include "obj_ops.h"
#include "parser.h"
#include "tokens.h"

#define SET_ERROR(err_macro, start, end, message) \
    do { \
        Nst_Error *error = malloc(sizeof(Nst_Error)); \
        if ( error == NULL ) \
        { \
            errno = ENOMEM; \
            return; \
        } \
        err_macro(error, start, end, message); \
        _nst_state_old->traceback->error = error; \
        _nst_state_old->error_occurred = true; \
    } while ( 0 )

#define SET_VALUE(new_value) do { \
    if ( _nst_state_old->value != NULL ) \
        dec_ref(_nst_state_old->value); \
    _nst_state_old->value = (Nst_Obj *)new_value; \
    } while ( 0 )

#define SET_NULL do { \
    SET_VALUE(nst_null); \
    inc_ref(nst_null); \
    } while ( 0 )

#define exe_node(node) do { \
    switch ( node->type ) \
    { \
    case NST_NT_LONG_S:         exe_long_s(node); break; \
    case NST_NT_WHILE_L:        exe_while_l(node); break; \
    case NST_NT_DOWHILE_L:      exe_dowhile_l(node); break; \
    case NST_NT_FOR_L:          exe_for_l(node); break; \
    case NST_NT_FOR_AS_L:       exe_for_as_l(node); break; \
    case NST_NT_IF_E:           exe_if_e(node); break; \
    case NST_NT_FUNC_DECLR:     exe_func_declr(node); break; \
    case NST_NT_RETURN_S:       exe_return_s(node); break; \
    case NST_NT_STACK_OP:       exe_stack_op(node); break; \
    case NST_NT_LOCAL_STACK_OP: exe_local_stack_op(node); break; \
    case NST_NT_LOCAL_OP:       exe_local_op(node); break; \
    case NST_NT_ARR_LIT: \
    case NST_NT_VEC_LIT:        exe_arr_or_vect_lit(node); break; \
    case NST_NT_MAP_LIT:        exe_map_lit(node); break; \
    case NST_NT_VALUE:          exe_value(node); break; \
    case NST_NT_ACCESS:         exe_access(node); break; \
    case NST_NT_EXTRACT_E:      exe_extract_e(node); break; \
    case NST_NT_ASSIGN_E:       exe_assign_e(node); break; \
    case NST_NT_CONTINUE_S:     exe_continue_s(node); break; \
    case NST_NT_BREAK_S:        exe_break_s(node); break; \
    case NST_NT_SWITCH_S:       exe_switch_s(node); break; \
    } \
    } while (0)

#define HEAD_NODE(node) (NODE(node->nodes->head->value))
#define TAIL_NODE(node) (NODE(node->nodes->tail->value))
#define HEAD_TOK(node) (TOK(node->tokens->head->value))
#define TAIL_TOK(node) (TOK(node->tokens->tail->value))

// General execution functions
static void exe_long_s(Nst_Node *node);
static void exe_while_l(Nst_Node *node);
static void exe_dowhile_l(Nst_Node *node);
static void exe_for_l(Nst_Node *node);
static void exe_for_as_l(Nst_Node *node);
static void exe_if_e(Nst_Node *node);
static void exe_func_declr(Nst_Node *node);
static void exe_return_s(Nst_Node *node);
static void exe_stack_op(Nst_Node *node);
static void exe_local_stack_op(Nst_Node *node);
static void exe_local_op(Nst_Node *node);
static void exe_arr_or_vect_lit(Nst_Node *node);
static void exe_map_lit(Nst_Node *node);
static void exe_value(Nst_Node *node);
static void exe_access(Nst_Node *node);
static void exe_extract_e(Nst_Node *node);
static void exe_assign_e(Nst_Node *node);
static void exe_continue_s(Nst_Node *node);
static void exe_break_s(Nst_Node * node);
static void exe_switch_s(Nst_Node *node);

// Operation functions that need the ExecutionState
static void call_func_internal(Nst_Node *node, Nst_FuncObj *func, Nst_Obj **args);

// Uitility functions
static inline bool safe_exe(Nst_Node *node);
static Nst_StrObj *make_cwd(char *file_path);
static bool check_same_file(char *p1, char *p2);
static Nst_SeqObj *make_argv(int argc, char **argv);

_Nst_OldExecutionState *_nst_state_old;

void _nst_run_old(Nst_Node *node, int argc, char **argv)
{
    if ( node == NULL )
        return;

    _nst_state_old = malloc(sizeof(_Nst_OldExecutionState));
    if ( _nst_state_old == NULL )
        return;

    char *cwd_buf = malloc(sizeof(char) * MAX_PATH);
    if ( cwd_buf == NULL )
        return;

    Nst_StrObj *cwd = AS_STR(nst_new_string_raw(_getcwd(cwd_buf, MAX_PATH), true));
    Nst_Traceback tb = { NULL, LList_new() };

    _nst_state_old->traceback = &tb;
    _nst_state_old->value = nst_null;
    _nst_state_old->vt = nst_new_var_table(NULL, cwd, make_argv(argc, argv));
    _nst_state_old->error_occurred = false;
    _nst_state_old->must_return = false;
    _nst_state_old->must_continue = false;
    _nst_state_old->must_break = false;
    _nst_state_old->curr_path = cwd;
    _nst_state_old->loaded_libs = LList_new();
    _nst_state_old->lib_paths = LList_new();
    _nst_state_old->lib_handles = LList_new();

    inc_ref(nst_null);
    LList_append(_nst_state_old->lib_paths, node->start.filename, false);

    exe_node(node);

    if ( _nst_state_old->error_occurred && _nst_state_old->traceback->error != NULL )
        nst_print_traceback(tb);

    nst_destroy_map(_nst_state_old->vt->vars);
    free(_nst_state_old->vt);
    dec_ref(_nst_state_old->value);
    for ( LLNode *n = _nst_state_old->loaded_libs->head; n != NULL; n = n->next )
    {
        void (*free_lib_func)() = (void (*)())GetProcAddress(n->value, "free_lib");
        if ( free_lib_func != NULL )
            free_lib_func();
    }
    LList_destroy(_nst_state_old->loaded_libs, (void (*)(void *))FreeLibrary);
    LList_destroy(_nst_state_old->lib_paths, free);
    LList_destroy(_nst_state_old->lib_handles, free);
}

Nst_MapObj *_nst_run_module_old(char *file_name)
{
    Nst_Node *module_ast = nst_parse(nst_ftokenize(file_name));

    if ( module_ast == NULL )
        return NULL;

    Nst_StrObj *prev_path = _nst_state_old->curr_path;
    Nst_VarTable *prev_vt = _nst_state_old->vt;

    Nst_StrObj *path_str = make_cwd(file_name);
    _nst_state_old->curr_path = path_str;

    int res = _chdir(path_str->value);
    assert(res == 0);

    _nst_state_old->vt = nst_new_var_table(NULL, path_str, make_argv(1, &file_name));
    LList_append(_nst_state_old->lib_paths, path_str, false);

    SET_NULL;
    exe_node(module_ast);

    if ( _nst_state_old->error_occurred )
        return NULL;

    LList_pop(_nst_state_old->lib_paths);

    Nst_MapObj *var_map = _nst_state_old->vt->vars;
    free(_nst_state_old->vt);
    _nst_state_old->curr_path = prev_path;
    _nst_state_old->vt = prev_vt;

    res = _chdir(prev_path->value);
    assert(res == 0);
    
    nst_map_drop_str(var_map, "Type");
    nst_map_drop_str(var_map, "Int");
    nst_map_drop_str(var_map, "Real");
    nst_map_drop_str(var_map, "Bool");
    nst_map_drop_str(var_map, "Null");
    nst_map_drop_str(var_map, "Str");
    nst_map_drop_str(var_map, "Array");
    nst_map_drop_str(var_map, "Vector");
    nst_map_drop_str(var_map, "Map");
    nst_map_drop_str(var_map, "Func");
    nst_map_drop_str(var_map, "Iter");
    nst_map_drop_str(var_map, "Byte");
    nst_map_drop_str(var_map, "IOfile");
    nst_map_drop_str(var_map, "true");
    nst_map_drop_str(var_map, "false");
    nst_map_drop_str(var_map, "null");
    nst_map_drop_str(var_map, "_cwd_");
    nst_map_drop_str(var_map, "_args_");
    nst_map_drop_str(var_map, "_vars_");
    
    return var_map;
}

static inline bool safe_exe(Nst_Node *node)
{
    exe_node(node);
    return !_nst_state_old->error_occurred;
}

static void exe_long_s(Nst_Node *node)
{
    for ( LLNode *cursor = node->nodes->head;
          cursor != NULL;
          cursor = cursor->next )
    {
        if ( !safe_exe(cursor->value) )
        {
            return;
        }

        if ( _nst_state_old->must_return )
            return;
        if ( _nst_state_old->must_break || _nst_state_old->must_continue )
            break;
    }
    SET_NULL;
}

static void exe_while_l(Nst_Node *node)
{
    while ( true )
    {
        if ( !safe_exe(HEAD_NODE(node)) )
            return;

        Nst_Obj *condition = nst_obj_cast(_nst_state_old->value, nst_t_bool, NULL);

        if ( condition == nst_false )
            break;

        if ( !safe_exe(TAIL_NODE(node)) )
            return;

        if ( _nst_state_old->must_break || _nst_state_old->must_return )
        {
            _nst_state_old->must_break = false;
            _nst_state_old->must_continue = false;
            break;
        }
        else if ( _nst_state_old->must_continue )
        {
            _nst_state_old->must_continue = false;
        }
    }
}

static void exe_dowhile_l(Nst_Node *node)
{
    while ( true )
    {
        if ( !safe_exe(TAIL_NODE(node)) )
            return;

        if ( _nst_state_old->must_break || _nst_state_old->must_return )
        {
            _nst_state_old->must_break = false;
            _nst_state_old->must_continue = false;
            break;
        }
        else if ( _nst_state_old->must_continue )
        {
            _nst_state_old->must_continue = false;
        }

        if ( !safe_exe(HEAD_NODE(node)) )
            return;

        Nst_Obj *condition = nst_obj_cast(_nst_state_old->value, nst_t_bool, NULL);

        if ( condition == nst_false )
        {
            dec_ref(condition);
            break;
        }
        dec_ref(condition);
    }
}

static void exe_for_l(Nst_Node *node)
{
    if ( !safe_exe(HEAD_NODE(node)) )
        return;

    Nst_Obj *range = _nst_state_old->value;
    inc_ref(range);

    if ( range->type != nst_t_int )
    {
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            _nst_format_type_error(EXPECTED_TYPE("Int"), range->type_name)
        );

        dec_ref(range);
        return;
    }

    for ( Nst_Int i = 0, n = AS_INT(range); i < n; i++ )
    {
        if ( !safe_exe(TAIL_NODE(node)) )
            break;

        if ( _nst_state_old->must_break || _nst_state_old->must_return )
        {
            _nst_state_old->must_break = false;
            _nst_state_old->must_continue = false;
            break;
        }
        else if ( _nst_state_old->must_continue )
        {
            _nst_state_old->must_continue = false;
        }
    }

    dec_ref(range);
}

static void exe_for_as_l(Nst_Node *node)
{
    if ( !safe_exe(HEAD_NODE(node)) )
        return;

    Nst_IterObj *iterator = AS_ITER(_nst_state_old->value);
    inc_ref((Nst_Obj *)iterator);

    if ( iterator->type != nst_t_iter )
    {
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            _nst_format_type_error(EXPECTED_TYPE("Iter"), iterator->type_name)
        );

        dec_ref((Nst_Obj *)iterator);
        return;
    }

    Nst_Obj *iter_val = iterator->value;
    register Nst_FuncObj *is_done_func = AS_FUNC(iterator->is_done);
    register Nst_FuncObj *get_val_func = AS_FUNC(iterator->get_val);
    register Nst_FuncObj *advance_func = AS_FUNC(iterator->advance);

    call_func_internal(node, AS_FUNC(iterator->start), &iter_val);
    if ( _nst_state_old->error_occurred )
    {
        dec_ref((Nst_Obj *)iterator);
        return;
    }

    Nst_Obj *var_name = HEAD_TOK(node)->value;

    while ( true )
    {
        call_func_internal(node, is_done_func, &iter_val);
        if ( _nst_state_old->error_occurred )
            break;

        if ( _nst_state_old->value->type != nst_t_bool )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                HEAD_NODE(node)->start,
                HEAD_NODE(node)->end,
                _nst_format_type_error(EXPECTED_BOOL_ITER_IS_DONE, _nst_state_old->value->type_name)
            );
            break;
        }

        if ( AS_BOOL(_nst_state_old->value) )
        {
            SET_NULL;
            break;
        }

        call_func_internal(node, get_val_func, &iter_val);
        if ( _nst_state_old->error_occurred )
            break;

        nst_set_val(_nst_state_old->vt, var_name, _nst_state_old->value);

        if ( !safe_exe(TAIL_NODE(node)) )
            break;

        if ( _nst_state_old->must_break || _nst_state_old->must_return )
        {
            _nst_state_old->must_break = false;
            _nst_state_old->must_continue = false;
            break;
        }
        else if ( _nst_state_old->must_continue )
        {
            _nst_state_old->must_continue = false;
        }

        call_func_internal(node, advance_func, &iter_val);
        if ( _nst_state_old->error_occurred )
            return;
    }

    dec_ref(iterator);
}

static void exe_if_e(Nst_Node *node)
{
    if ( !safe_exe(HEAD_NODE(node)) )
        return;

    Nst_Obj *condition = nst_obj_cast(_nst_state_old->value, nst_t_bool, NULL);

    // If there is no else clause
    if ( node->nodes->size == 2 )
    {
        if ( condition == nst_true )
            safe_exe(TAIL_NODE(node));
        else
            SET_NULL;
    }
    else
    {
        if ( condition == nst_true )
            safe_exe(node->nodes->head->next->value);
        else
            safe_exe(TAIL_NODE(node));
    }

    dec_ref(condition);
}

static void exe_func_declr(Nst_Node *node)
{
    Nst_FuncObj *func = AS_FUNC(new_func(node->tokens->size - 1));
    register size_t i = 0;

    for ( LLNode *n = node->tokens->head->next; n != NULL; n = n->next )
        func->args[i++] = TOK(n->value)->value;

    func->body = HEAD_NODE(node);
    Nst_Obj *func_name = HEAD_TOK(node)->value;
    nst_set_val(_nst_state_old->vt, func_name, (Nst_Obj *)func);
    dec_ref(func);
    SET_NULL;
}

static void exe_return_s(Nst_Node *node)
{
    safe_exe(HEAD_NODE(node));
    _nst_state_old->must_return = true;
}

static void exe_stack_op(Nst_Node *node)
{
    int op_tok = HEAD_TOK(node)->type;
    Nst_OpErr err = { "", "" };
    Nst_Obj *res = NULL;
    if ( !safe_exe(HEAD_NODE(node)) )
        return;
    Nst_Obj *ob1 = _nst_state_old->value;
    inc_ref(ob1);

    if ( op_tok == NST_TT_L_AND )
    {
        Nst_Obj *bool_obj = nst_obj_cast(ob1, nst_t_bool, NULL);
        if ( bool_obj == nst_false )
        {
            dec_ref(ob1);
            SET_VALUE(nst_false); // ref incremented by bool_obj
            return;
        }
    }
    else if ( op_tok == NST_TT_L_OR )
    {
        Nst_Obj *bool_obj = nst_obj_cast(ob1, nst_t_bool, NULL);
        if ( bool_obj == nst_true )
        {
            dec_ref(ob1);
            SET_VALUE(nst_true); // ref incremented by bool_obj
            return;
        }
    }

    if ( !safe_exe(TAIL_NODE(node)) )
    {
        dec_ref(ob1);
        return;
    }
    Nst_Obj *ob2 = _nst_state_old->value;
    inc_ref(ob2);

    switch ( op_tok ) {
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
    default: return;
    }

    if ( res == NULL )
    {
        if ( errno == ENOMEM )
            return;
        SET_ERROR(_NST_SET_GENERAL_ERROR, node->start, node->end, err.message);
        _nst_state_old->traceback->error->name = err.name;
        dec_ref(ob1);
        dec_ref(ob2);
        return;
    }

    dec_ref(ob1);
    dec_ref(ob2);

    SET_VALUE(res);
}

static void exe_local_stack_op(Nst_Node *node)
{
    int op_tok = TOK(node->tokens->head->value)->type;
    register size_t arg_count = node->nodes->size;
    Nst_OpErr err = { "", "" };

    switch ( op_tok )
    {
    case NST_TT_CAST:
        if ( arg_count != 2 )
        {
            SET_ERROR(
                _NST_SET_CALL_ERROR,
                node->start,
                node->end,
                arg_count > 2 ? TOO_MANY_ARGS("cast") : TOO_FEW_ARGS("cast")
            );
            return;
        }

        if ( !safe_exe(HEAD_NODE(node)) )
            return;
        Nst_Obj *type = _nst_state_old->value;
        inc_ref(type);
        if ( !safe_exe(TAIL_NODE(node)) )
        {
            dec_ref(type);
            return;
        }
        Nst_Obj *obj = _nst_state_old->value;
        inc_ref(obj);

        Nst_Obj *res = nst_obj_cast(obj, type, &err);
        if ( res == NULL )
        {
            if ( errno == ENOMEM )
                return;
            SET_ERROR(_NST_SET_GENERAL_ERROR, node->start, node->end, err.message);
            _nst_state_old->traceback->error->name = err.name;
            dec_ref(type);
            dec_ref(obj);
            return;
        }

        dec_ref(type);
        dec_ref(obj);

        SET_VALUE(res);
        break;
    case NST_TT_CALL:
        if ( !safe_exe(TAIL_NODE(node)) )
            return;
        Nst_Obj *func_obj = _nst_state_old->value;
        inc_ref(func_obj);

        if ( func_obj->type != nst_t_func )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                _nst_format_type_error(EXPECTED_TYPE("Func"), func_obj->type_name)
            );
            dec_ref(func_obj);
            return;
        }

        Nst_FuncObj *func = AS_FUNC(func_obj);

        if ( arg_count - 1 != func->arg_num )
        {
            SET_ERROR(
                _NST_SET_CALL_ERROR,
                node->start,
                node->end,
                arg_count - 1 > func->arg_num ? TOO_MANY_ARGS_FUNC : TOO_FEW_ARGS_FUNC
            );

            dec_ref(func_obj);
            return;
        }

        Nst_Obj **args_arr = malloc(func->arg_num * sizeof(Nst_Obj *));
        if ( args_arr == NULL )
        {
            errno = ENOMEM;
            return;
        }

        LLNode *n = node->nodes->head;
        for ( size_t i = 0; i < func->arg_num; i++ )
        {
            if ( !safe_exe(n->value) )
            {
                for ( size_t j = 0; j < i; j++ )
                    dec_ref(args_arr[i]);
                dec_ref(func_obj);
                free(args_arr);
                return;
            }
            Nst_Obj *val = _nst_state_old->value;
            inc_ref(val);
            args_arr[i] = val;

            n = n->next;
        }

        call_func_internal(node, func, args_arr);
        dec_ref(func_obj);

        for ( size_t i = 0; i < func->arg_num; i++ )
            dec_ref(args_arr[i]);
        free(args_arr);

        break;
    case NST_TT_RANGE:
        if ( arg_count != 3 && arg_count != 2 )
        {
            SET_ERROR(
                _NST_SET_CALL_ERROR,
                node->start,
                node->end,
                arg_count > 3 ? TOO_MANY_ARGS("range") : TOO_FEW_ARGS("range")
            );
            return;
        }

        if ( !safe_exe(HEAD_NODE(node)) )
            return;
        Nst_Obj *start = _nst_state_old->value;
        inc_ref(start);
        if ( !safe_exe(TAIL_NODE(node)) )
        {
            dec_ref(start);
            return;
        }
        Nst_Obj *stop = _nst_state_old->value;
        inc_ref(stop);

        Nst_Obj *step = NULL;

        if ( start->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                HEAD_NODE(node)->start,
                HEAD_NODE(node)->end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), start->type_name)
            );

            dec_ref(start);
            dec_ref(stop);
            return;
        }

        if ( stop->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), start->type_name)
            );

            dec_ref(start);
            dec_ref(stop);
            return;
        }

        if ( arg_count == 3 )
        {
            Nst_Node *n = node->nodes->head->next->value;
            if ( !safe_exe(n) )
            {
                dec_ref(start);
                dec_ref(stop);
                return;
            }
            step = _nst_state_old->value;
            inc_ref(step);

            if ( step->type != nst_t_int )
            {
                SET_ERROR(
                    _NST_SET_TYPE_ERROR,
                    TAIL_NODE(node)->start,
                    TAIL_NODE(node)->end,
                    _nst_format_type_error(EXPECTED_TYPE("Int"), start->type_name)
                );

                dec_ref(start);
                dec_ref(stop);
                dec_ref(step);
                return;
            }

            if ( AS_INT(step) == 0 )
            {
                SET_ERROR(
                    _NST_SET_VALUE_ERROR,
                    n->start,
                    n->end,
                    ZERO_RANGE_STEP
                );

                dec_ref(start);
                dec_ref(stop);
                dec_ref(step);
                return;
            }
        }
        else
        {
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

        dec_ref(idx);
        dec_ref(start);
        dec_ref(stop);
        dec_ref(step);

        SET_VALUE(nst_new_iter(
            AS_FUNC(new_cfunc(1, nst_num_iter_start)),
            AS_FUNC(new_cfunc(1, nst_num_iter_advance)),
            AS_FUNC(new_cfunc(1, nst_num_iter_is_done)),
            AS_FUNC(new_cfunc(1, nst_num_iter_get_val)),
            data_seq
        ));

        break;
    default:
        return;
    }
}

static void exe_local_op(Nst_Node *node)
{
    int op_tok = TOK(node->tokens->head->value)->type;
    Nst_OpErr err = { "", "" };
    Nst_Obj *res = NULL;
    if ( !safe_exe(HEAD_NODE(node)) )
        return;
    Nst_Obj *ob = _nst_state_old->value;

    switch ( op_tok )
    {
    case NST_TT_LEN:    res = nst_obj_len(ob, &err);    break;
    case NST_TT_L_NOT:  res = nst_obj_lgnot(ob, &err);  break;
    case NST_TT_B_NOT:  res = nst_obj_bwnot(ob, &err);  break;
    case NST_TT_STDIN:  res = nst_obj_stdin(ob, &err);  break;
    case NST_TT_STDOUT: res = nst_obj_stdout(ob, &err); break;
    case NST_TT_TYPEOF: res = nst_obj_typeof(ob, &err); break;
    case NST_TT_IMPORT: res = nst_obj_import(ob, &err); break;
    case NST_TT_NEG:    res = nst_obj_neg(ob, &err);    break;
    case NST_TT_LOC_CALL:
    {
        Nst_FuncObj *func = AS_FUNC(ob);
        if ( func->arg_num != 0 )
        {
            SET_ERROR(
                _NST_SET_CALL_ERROR,
                node->start,
                node->end,
                TOO_FEW_ARGS_FUNC
            );
        }

        call_func_internal(node, func, NULL);
        return;
    }
    default: return;
    }

    if ( res == NULL )
    {
        if ( errno == ENOMEM )
            return;

        if ( _nst_state_old->error_occurred )
        {
            LList_append(_nst_state_old->traceback->positions, &node->start, false);
            LList_append(_nst_state_old->traceback->positions, &node->end, false);
        }
        else
        {
            SET_ERROR(_NST_SET_GENERAL_ERROR, node->start, node->end, err.message);
            _nst_state_old->traceback->error->name = err.name;
        }

        return;
    }

    SET_VALUE(res);
}

static void exe_arr_or_vect_lit(Nst_Node *node)
{
    bool is_arr = node->type == NST_NT_ARR_LIT;

    if ( node->tokens->size != 0 )
    {
        if ( !safe_exe(HEAD_NODE(node)) )
            return;
        Nst_Obj *value = inc_ref(_nst_state_old->value);
        if ( !safe_exe(TAIL_NODE(node)) )
        {
            dec_ref(value);
            return;
        }

        Nst_Obj *quantity = _nst_state_old->value;
        if ( quantity->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_VALUE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), quantity->type_name)
            );
            dec_ref(value);
            return;
        }

        Nst_Int size = AS_INT(quantity);
        if ( size < 0 )
        {
            SET_ERROR(
                _NST_SET_VALUE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                NEGATIVE_SIZE_FOR_SEQUENCE
            );
            dec_ref(value);
            return;
        }

        Nst_SeqObj *seq = is_arr ? AS_SEQ(nst_new_array((size_t)size))
                                 : AS_SEQ(nst_new_vector((size_t)size));
        if ( seq == NULL )
        {
            SET_ERROR(
                _NST_SET_MEMORY_ERROR,
                node->start,
                node->end,
                RAN_OUT_OF_MEMORY
            );
            dec_ref(value);
            return;
        }

        for ( Nst_Int i = 0; i < size; i++ )
            nst_set_value_seq(seq, i, value);

        dec_ref(value);
        SET_VALUE(seq);
        return;
    }

    Nst_SeqObj *seq = is_arr ? AS_SEQ(nst_new_array(node->nodes->size))
                             : AS_SEQ(nst_new_vector(node->nodes->size));
    size_t i = 0;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        if ( !safe_exe(n->value) )
            return;
        nst_set_value_seq(seq, i++, _nst_state_old->value);
    }

    SET_VALUE(seq);
}

static void exe_map_lit(Nst_Node *node)
{
    Nst_MapObj *map = AS_MAP(nst_new_map());
    register Nst_Obj *key = NULL;
    register Nst_Node *key_node = NULL;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        if ( !safe_exe(n->value) )
            return;
        if ( key == NULL )
        {
            key_node = n->value;
            key = _nst_state_old->value;
            inc_ref(_nst_state_old->value);
            continue;
        }

        if ( !nst_map_set(map, key, _nst_state_old->value) )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                key_node->start,
                key_node->end,
                _nst_format_type_error(UNHASHABLE_TYPE, key->type_name);
            );

            return;
        }

        dec_ref(key);
        key = NULL;
    }

    SET_VALUE(map);
}

static void exe_value(Nst_Node *node)
{
    Nst_LexerToken *tok = LList_peek_front(node->tokens);
    inc_ref(tok->value);

    switch ( tok->type )
    {
    case NST_TT_INT:
    case NST_TT_REAL:
    case NST_TT_STRING:
        SET_VALUE(tok->value);
        break;
    }
}

static void exe_access(Nst_Node *node)
{
    Nst_Obj *key = HEAD_TOK(node)->value;
    Nst_Obj *val = nst_get_val(_nst_state_old->vt, key);
    if ( val == NULL )
    {
        SET_NULL;
        return;
    }
    SET_VALUE(val);
}

static void exe_extract_e(Nst_Node *node)
{
    if ( !safe_exe(HEAD_NODE(node)) )
        return;
    Nst_Obj *container = _nst_state_old->value;
    inc_ref(container);
    if ( !safe_exe(TAIL_NODE(node)) )
    {
        dec_ref(container);
        return;
    }
    Nst_Obj *key = _nst_state_old->value;
    inc_ref(key);

    if ( container->type == nst_t_arr || container->type == nst_t_vect )
    {
        if ( key->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), key->type_name)
            );

            dec_ref(container);
            dec_ref(key);
            return;
        }

        Nst_Int idx = AS_INT(key);
        Nst_Obj *val = nst_get_value_seq(container, idx);

        if ( val == NULL )
        {
            SET_ERROR(
                _NST_SET_VALUE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                _nst_format_idx_error(
                    container->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                                 : INDEX_OUT_OF_BOUNDS("Vector"),
                    idx,
                    AS_SEQ(container)->len
                )
            );

            dec_ref(container);
            dec_ref(key);
            return;
        }

        SET_VALUE(val);
    }
    else if ( container->type == nst_t_str )
    {
        if ( key->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), key->type_name)
            );

            dec_ref(container);
            dec_ref(key);
            return;
        }

        Nst_StrObj *str = AS_STR(container);
        Nst_Int idx = AS_INT(key);
        if ( idx < 0 )
            idx += str->len;

        if ( idx < 0 || idx >= (Nst_Int)str->len )
        {
            SET_ERROR(
                _NST_SET_VALUE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                _nst_format_idx_error(INDEX_OUT_OF_BOUNDS("Str"), idx, str->len)
            );

            dec_ref(container);
            dec_ref(key);
            return;
        }

        char *ch = calloc(2, sizeof(char));
        if ( ch == NULL )
        {
            errno = ENOMEM;
            dec_ref(container);
            dec_ref(key);
            return;
        }

        ch[0] = str->value[idx];

        Nst_Obj *val = nst_new_string(ch, 1, true);

        SET_VALUE(val);
    }
    else if ( container->type == nst_t_map )
    {
        Nst_Obj *val = nst_map_get(container, key);

        if ( val == NULL )
        {
            if ( key->hash == -1 )
            {
                SET_ERROR(
                    _NST_SET_VALUE_ERROR,
                    TAIL_NODE(node)->start,
                    TAIL_NODE(node)->end,
                    _nst_format_type_error(UNHASHABLE_TYPE, key->type_name)
                );

                dec_ref(container);
                dec_ref(key);
            }
            else
                SET_NULL;
            return;
        }

        SET_VALUE(val);
    }
    else
    {
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            _nst_format_type_error(EXPECTED_TYPE("Array', 'Vector', 'Map' or 'Str"), container->type_name)
        );
    }

    dec_ref(key);
    dec_ref(container);
}

static void exe_assign_e(Nst_Node *node)
{
    Nst_Node *value_node = HEAD_NODE(node);
    Nst_Node *name_node = TAIL_NODE(node);

    if ( !safe_exe(value_node) )
        return;
    Nst_Obj *value = _nst_state_old->value;

    if ( name_node->type == NST_NT_ACCESS )
    {
        Nst_Obj *name = HEAD_TOK(name_node)->value;

        nst_set_val(_nst_state_old->vt, name, value);
        return; // state->value is still the same
    }

    inc_ref(value);

    // If name_node->type == EXTRACTION
    // can't be anything else because of the parser
    if ( !safe_exe(HEAD_NODE(name_node)) )
    {
        dec_ref(value);
        return;
    }
    Nst_Obj *obj_to_set = _nst_state_old->value;
    inc_ref(obj_to_set);

    if ( !safe_exe(TAIL_NODE(name_node)) )
    {
        dec_ref(value);
        dec_ref(obj_to_set);
        return;
    }
    Nst_Obj *idx = _nst_state_old->value;
    inc_ref(idx);

    SET_VALUE(value);

    if ( obj_to_set->type == nst_t_map )
    {
        bool res = nst_map_set(AS_MAP(obj_to_set), idx, value);
        if ( !res )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                name_node->start,
                name_node->end,
                _nst_format_type_error(UNHASHABLE_TYPE, idx->type_name)
            );
        }

        dec_ref(obj_to_set);
        dec_ref(idx);
        return;
    }
    else if ( obj_to_set->type == nst_t_arr || obj_to_set->type == nst_t_vect )
    {
        if ( idx->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                TAIL_NODE(name_node)->start,
                TAIL_NODE(name_node)->end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), idx->type_name)
            );

            dec_ref(obj_to_set);
            dec_ref(idx);
            return;
        }

        bool res = nst_set_value_seq(obj_to_set, AS_INT(idx), value);

        if ( !res )
        {
            SET_ERROR(
                _NST_SET_VALUE_ERROR,
                name_node->start,
                name_node->end,
                _nst_format_idx_error(
                    obj_to_set->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                                  : INDEX_OUT_OF_BOUNDS("Vector"),
                    AS_INT(idx),
                    AS_SEQ(obj_to_set)->len
                )
            );
        }

        dec_ref(obj_to_set);
        dec_ref(idx);
        return;
    }
    else
    {
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            HEAD_NODE(name_node)->start,
            HEAD_NODE(name_node)->end,
            _nst_format_type_error(EXPECTED_TYPE("Array', 'Vector' or 'Map"), obj_to_set->type_name)
        );

        dec_ref(obj_to_set);
        dec_ref(idx);
        return;
    }
}

static void exe_continue_s(Nst_Node *node)
{
    _nst_state_old->must_continue = true;
    SET_NULL;
}

static void exe_break_s(Nst_Node *node)
{
    _nst_state_old->must_break = true;
    SET_NULL;
}

static void exe_switch_s(Nst_Node *node)
{
    size_t nodes_len = node->nodes->size;

    if ( !safe_exe(HEAD_NODE(node)) )
        return;
    Nst_Obj *main_val = inc_ref(_nst_state_old->value);

    size_t i = 2;
    LLNode *n = node->nodes->head;

    while ( true )
    {
        n = n->next;

        if ( i == nodes_len ) // has reached default case
            exe_node(NODE(n->value));
        if ( i >= nodes_len )
        {
            dec_ref(main_val);
            _nst_state_old->must_continue = false;
            SET_NULL;
            return;
        }

        if ( !safe_exe(n->value) )
        {
            dec_ref(main_val);
            return;
        }

        Nst_Obj *res = nst_obj_eq(main_val, _nst_state_old->value, NULL);
        n = n->next;
        i += 2;

        if ( res == nst_false )
        {
            dec_ref(res);
            continue;
        }
        else
        {
            dec_ref(res);

            if ( !safe_exe(n->value) )
            {
                dec_ref(main_val);
                return;
            }

            if ( i > nodes_len ) // it's the last node
            {
                dec_ref(main_val);
                _nst_state_old->must_continue = false;
                SET_NULL;
                return;
            }

            while ( _nst_state_old->must_continue )
            {
                if ( i == nodes_len )
                    exe_node(NODE(n->next->value));
                if ( i >= nodes_len )
                {
                    dec_ref(main_val);
                    _nst_state_old->must_continue = false;
                    SET_NULL;
                    return;
                }

                n = n->next->next;
                i += 2;
                _nst_state_old->must_continue = false;

                if ( !safe_exe(n->value) )
                {
                    dec_ref(main_val);
                    return;
                }
            }

            SET_NULL;
            return;
        }
    }
}

Nst_Obj *_nst_call_func_old(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err)
{
    if ( func->body != NULL )
    {
        Nst_VarTable *prev_vt = _nst_state_old->vt;

        if ( _nst_state_old->vt->global_table != NULL )
            _nst_state_old->vt = nst_new_var_table(_nst_state_old->vt->global_table, NULL, NULL);
        else
            _nst_state_old->vt = nst_new_var_table(_nst_state_old->vt, NULL, NULL);

        for ( size_t i = 0; i < func->arg_num; i++ )
            nst_set_val(_nst_state_old->vt, func->args[i], args[i]);

        exe_node(func->body);
        if ( _nst_state_old->error_occurred )
        {
            err->name = _nst_state_old->traceback->error->name;
            err->message = _nst_state_old->traceback->error->message;

            nst_destroy_map(_nst_state_old->vt->vars);
            free(_nst_state_old->vt);
            _nst_state_old->vt = prev_vt;
            return NULL;
        }

        _nst_state_old->must_return = false;

        if ( func->body->type != NST_NT_LONG_S && func->body->type != NST_NT_RETURN_S )
            SET_NULL;

        nst_destroy_map(_nst_state_old->vt->vars);
        free(_nst_state_old->vt);

        _nst_state_old->vt = prev_vt;
        return inc_ref(_nst_state_old->value);
    }
    else
    {
        assert(func->cbody != NULL);
        Nst_Obj *res = func->cbody(func->arg_num, args, err);
        return res;
    }
}

static void call_func_internal(Nst_Node *node, Nst_FuncObj *func, Nst_Obj **args)
{
    Nst_OpErr err = { "", "" };
    Nst_Obj *res = _nst_call_func_old(func, args, &err);

    if ( res == NULL )
    {
        if ( _nst_state_old->traceback->error == NULL )
        {
            SET_ERROR(_NST_SET_GENERAL_ERROR, node->start, node->end, err.message);
            _nst_state_old->traceback->error->name = err.name;
            return;
        }
        else
        {
            LList_append(_nst_state_old->traceback->positions, &node->start, false);
            LList_append(_nst_state_old->traceback->positions, &node->end, false);
            return;
        }
    }

    SET_VALUE(res);
}

size_t _nst_get_full_path_old(char *file_path, char **buf, char **file_part)
{
    char *path = malloc(sizeof(char) * MAX_PATH);
    if ( path == NULL )
        return 0;

    DWORD path_len = GetFullPathNameA(file_path, MAX_PATH, path, file_part);
    if ( path_len > MAX_PATH )
    {
        free(path);
        path = malloc(sizeof(char) * path_len);
        if ( path == NULL )
            return 0;
        path_len = GetFullPathNameA(file_path, path_len, path, file_part);
    }

    *buf = path;
    return path_len;
}

static Nst_StrObj *make_cwd(char *file_path)
{
    char *path = NULL;
    char *file_part = NULL;

    _nst_get_full_path_old(file_path, &path, &file_part);

    *(file_part - 1) = 0;

    return AS_STR(nst_new_string(path, file_part - path, true));
}

static bool check_same_file(char *p1, char *p2)
{
    HANDLE f1 = CreateFileA(p1, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if ( f1 == INVALID_HANDLE_VALUE )
        return false;
    HANDLE f2 = CreateFileA(p2, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if ( f2 == INVALID_HANDLE_VALUE )
    {
        CloseHandle(f1);
        return false;
    }

    BY_HANDLE_FILE_INFORMATION info1;
    BY_HANDLE_FILE_INFORMATION info2;

    bool are_equal = GetFileInformationByHandle(f1, &info1) &&
        GetFileInformationByHandle(f2, &info2) &&
        info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber &&
        info1.nFileIndexLow == info2.nFileIndexLow &&
        info1.nFileIndexHigh == info2.nFileIndexHigh;

    CloseHandle(f1);
    CloseHandle(f2);

    return are_equal;
}

static Nst_SeqObj *make_argv(int argc, char **argv)
{
    Nst_SeqObj *args = AS_SEQ(nst_new_array(argc - 1));

    for ( int i = 1; i < argc; i++ )
    {
        Nst_Obj *val = nst_new_string_raw(argv[i], false);
        nst_set_value_seq(args, i - 1, val);
        dec_ref(val);
    }

    return args;
}
