#include <assert.h>
#include <direct.h>
#include <errno.h>
#include <stdlib.h>
#include "error_internal.h"
#include "hash.h"
#include "interpreter.h"
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
        state->traceback->error = error; \
        state->error_occurred = true; \
    } while ( 0 )

#define SET_VALUE(new_value) do { \
    if ( state->value != NULL ) \
        dec_ref(state->value); \
    state->value = new_value; \
    } while ( 0 )

#define SET_NULL do { \
    SET_VALUE(nst_null); \
    inc_ref(nst_null); \
    } while ( 0 )

#define exe_node(node) do { \
    switch ( node->type ) \
    { \
    case LONG_S:         exe_long_s(node); break; \
    case WHILE_L:        exe_while_l(node); break; \
    case DOWHILE_L:      exe_dowhile_l(node); break; \
    case FOR_L:          exe_for_l(node); break; \
    case FOR_AS_L:       exe_for_as_l(node); break; \
    case IF_E:           exe_if_e(node); break; \
    case FUNC_DECLR:     exe_func_declr(node); break; \
    case RETURN_S:       exe_return_s(node); break; \
    case STACK_OP:       exe_stack_op(node); break; \
    case LOCAL_STACK_OP: exe_local_stack_op(node); break; \
    case LOCAL_OP:       exe_local_op(node); break; \
    case ARR_LIT: \
    case VECT_LIT:       exe_arr_or_vect_lit(node); break; \
    case MAP_LIT:        exe_map_lit(node); break; \
    case VALUE:          exe_value(node); break; \
    case ACCESS:         exe_access(node); break; \
    case EXTRACT_E:      exe_extract_e(node); break; \
    case ASSIGN_E:       exe_assign_e(node); break; \
    case CONTINUE_S:     exe_continue_s(node); break; \
    case BREAK_S:        exe_break_s(node); break; \
    case SWITCH_S:       exe_switch_s(node); break; \
    } \
    } while (0)

#define HEAD_NODE(node) (NODE(node->nodes->head->value))
#define TAIL_NODE(node) (NODE(node->nodes->tail->value))
#define HEAD_TOK(node) (TOK(node->tokens->head->value))
#define TAIL_TOK(node) (TOK(node->tokens->tail->value))

// General execution functions
static void exe_long_s(Node *node);
static void exe_while_l(Node *node);
static void exe_dowhile_l(Node *node);
static void exe_for_l(Node *node);
static void exe_for_as_l(Node *node);
static void exe_if_e(Node *node);
static void exe_func_declr(Node *node);
static void exe_return_s(Node *node);
static void exe_stack_op(Node *node);
static void exe_local_stack_op(Node *node);
static void exe_local_op(Node *node);
static void exe_arr_or_vect_lit(Node *node);
static void exe_map_lit(Node *node);
static void exe_value(Node *node);
static void exe_access(Node *node);
static void exe_extract_e(Node *node);
static void exe_assign_e(Node *node);
static void exe_continue_s(Node *node);
static void exe_break_s(Node * node);
static void exe_switch_s(Node *node);

// Operation functions that need the ExecutionState
static void call_func_internal(Node *node, Nst_func *func, Nst_Obj **args);

// Uitility functions
static inline bool safe_exe(Node *node);
static Nst_string *make_cwd(char *file_path);
static bool check_same_file(char *p1, char *p2);
static Nst_sequence *make_argv(int argc, char **argv);

ExecutionState *state;

void run(Node *node, int argc, char **argv)
{
    if ( node == NULL )
        return;

    state = malloc(sizeof(ExecutionState));
    if ( state == NULL )
        return;

    char *cwd_buf = malloc(sizeof(char) * MAX_PATH);
    if ( cwd_buf == NULL )
        return;

    Nst_string *cwd = new_string_raw(_getcwd(cwd_buf, MAX_PATH), true);
    Nst_Traceback tb = { NULL, LList_new() };

    state->traceback = &tb;
    state->value = nst_null;
    state->vt = new_var_table(NULL, cwd, make_argv(argc, argv));
    state->error_occurred = false;
    state->must_return = false;
    state->must_continue = false;
    state->must_break = false;
    state->curr_path = cwd;
    state->loaded_libs = LList_new();
    state->lib_paths = LList_new();
    state->lib_handles = LList_new();

    inc_ref(nst_null);
    LList_append(state->lib_paths, node->start.filename, false);

    exe_node(node);

    if ( state->error_occurred && state->traceback->error != NULL )
        print_traceback(tb);

    destroy_map(state->vt->vars);
    free(state->vt);
    dec_ref(state->value);
    for ( LLNode *n = state->loaded_libs->head; n != NULL; n = n->next )
    {
        void (*free_lib_func)() = (void (*)())GetProcAddress(n->value, "free_lib");
        if ( free_lib_func != NULL )
            free_lib_func();
    }
    LList_destroy(state->loaded_libs, (void (*)(void *))FreeLibrary);
    LList_destroy(state->lib_paths, free);
    LList_destroy(state->lib_handles, free);
}

Nst_map *run_module(char *file_name)
{
    Node *module_ast = parse(ftokenize(file_name));

    if ( module_ast == NULL )
        return NULL;

    Nst_string *prev_path = state->curr_path;
    VarTable *prev_vt = state->vt;

    Nst_string *path_str = make_cwd(file_name);
    state->curr_path = path_str;

    int res = _chdir(path_str->value);
    assert(res == 0);

    state->vt = new_var_table(NULL, path_str, make_argv(1, &file_name));
    LList_append(state->lib_paths, path_str, false);

    SET_NULL;
    exe_node(module_ast);

    if ( state->error_occurred )
        return NULL;

    LList_pop(state->lib_paths);

    Nst_map *var_map = state->vt->vars;
    free(state->vt);
    state->curr_path = prev_path;
    state->vt = prev_vt;

    res = _chdir(prev_path->value);
    assert(res == 0);
    
    map_drop_str(var_map, "Type");
    map_drop_str(var_map, "Int");
    map_drop_str(var_map, "Real");
    map_drop_str(var_map, "Bool");
    map_drop_str(var_map, "Null");
    map_drop_str(var_map, "Str");
    map_drop_str(var_map, "Array");
    map_drop_str(var_map, "Vector");
    map_drop_str(var_map, "Map");
    map_drop_str(var_map, "Func");
    map_drop_str(var_map, "Iter");
    map_drop_str(var_map, "Byte");
    map_drop_str(var_map, "IOfile");
    map_drop_str(var_map, "true");
    map_drop_str(var_map, "false");
    map_drop_str(var_map, "null");
    map_drop_str(var_map, "_cwd_");
    map_drop_str(var_map, "_args_");
    map_drop_str(var_map, "_vars_");
    
    return var_map;
}

static inline bool safe_exe(Node *node)
{
    exe_node(node);
    return !state->error_occurred;
}

static void exe_long_s(Node *node)
{
    for ( LLNode *cursor = node->nodes->head;
          cursor != NULL;
          cursor = cursor->next )
    {
        if ( !safe_exe(cursor->value) )
        {
            return;
        }

        if ( state->must_return )
            return;
        if ( state->must_break || state->must_continue )
            break;
    }
    SET_NULL;
}

static void exe_while_l(Node *node)
{
    while ( true )
    {
        if ( !safe_exe(HEAD_NODE(node)) )
            return;

        Nst_Obj *condition = obj_cast(state->value, nst_t_bool, NULL);

        if ( condition == nst_false )
            break;

        if ( !safe_exe(TAIL_NODE(node)) )
            return;

        if ( state->must_break || state->must_return )
        {
            state->must_break = false;
            state->must_continue = false;
            break;
        }
        else if ( state->must_continue )
        {
            state->must_continue = false;
        }
    }
}

static void exe_dowhile_l(Node *node)
{
    while ( true )
    {
        if ( !safe_exe(TAIL_NODE(node)) )
            return;

        if ( state->must_break || state->must_return )
        {
            state->must_break = false;
            state->must_continue = false;
            break;
        }
        else if ( state->must_continue )
        {
            state->must_continue = false;
        }

        if ( !safe_exe(HEAD_NODE(node)) )
            return;

        Nst_Obj *condition = obj_cast(state->value, nst_t_bool, NULL);

        if ( condition == nst_false )
        {
            dec_ref(condition);
            break;
        }
        dec_ref(condition);
    }
}

static void exe_for_l(Node *node)
{
    if ( !safe_exe(HEAD_NODE(node)) )
        return;

    Nst_Obj *range = state->value;
    inc_ref(range);

    if ( range->type != nst_t_int )
    {
        SET_ERROR(
            SET_TYPE_ERROR_INT,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            format_type_error(EXPECTED_TYPE("Int"), range->type_name)
        );

        dec_ref(range);
        return;
    }


    for ( Nst_int i = 0, n = AS_INT(range); i < n; i++ )
    {
        if ( !safe_exe(TAIL_NODE(node)) )
            break;

        if ( state->must_break || state->must_return )
        {
            state->must_break = false;
            state->must_continue = false;
            break;
        }
        else if ( state->must_continue )
        {
            state->must_continue = false;
        }
    }

    dec_ref(range);
}

static void exe_for_as_l(Node *node)
{
    if ( !safe_exe(HEAD_NODE(node)) )
        return;

    Nst_Obj *iter_obj = state->value;
    inc_ref(iter_obj);

    if ( iter_obj->type != nst_t_iter )
    {
        SET_ERROR(
            SET_TYPE_ERROR_INT,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            format_type_error(EXPECTED_TYPE("Iter"), iter_obj->type_name)
        );

        dec_ref(iter_obj);
        return;
    }

    Nst_iter *iterator = iter_obj->value;
    Nst_Obj *iter_val = iterator->value;

    call_func_internal(node, iterator->start->value, &iter_val);
    if ( state->error_occurred )
    {
        dec_ref(iter_obj);
        return;
    }

    Nst_Obj *var_name = HEAD_TOK(node)->value;

    while ( true )
    {
        call_func_internal(node, iterator->is_done->value, &iter_val);
        if ( state->error_occurred )
            break;

        if ( state->value->type != nst_t_bool )
        {
            SET_ERROR(
                SET_TYPE_ERROR_INT,
                HEAD_NODE(node)->start,
                HEAD_NODE(node)->end,
                format_type_error(EXPECTED_BOOL_ITER_IS_DONE, state->value->type_name)
            );
            break;
        }

        if ( AS_BOOL(state->value) )
        {
            SET_NULL;
            break;
        }

        call_func_internal(node, iterator->get_val->value, &iter_val);
        if ( state->error_occurred )
            break;

        set_val(state->vt, var_name, state->value);

        if ( !safe_exe(TAIL_NODE(node)) )
            break;

        if ( state->must_break || state->must_return )
        {
            state->must_break = false;
            state->must_continue = false;
            break;
        }
        else if ( state->must_continue )
        {
            state->must_continue = false;
        }

        call_func_internal(node, iterator->advance->value, &iter_val);
        if ( state->error_occurred )
            return;
    }

    dec_ref(iter_obj);
}

static void exe_if_e(Node *node)
{
    if ( !safe_exe(HEAD_NODE(node)) )
        return;

    Nst_Obj *condition = obj_cast(state->value, nst_t_bool, NULL);

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

static void exe_func_declr(Node *node)
{
    Nst_func *func = new_func(node->tokens->size - 1);

    register size_t i = 0;
    for ( LLNode *n = node->tokens->head->next; n != NULL; n = n->next )
        func->args[i++] = TOK(n->value)->value;

    func->body = HEAD_NODE(node);

    Nst_Obj *func_obj = new_func_obj(func);
    Nst_Obj *func_name = HEAD_TOK(node)->value;

    set_val(state->vt, func_name, func_obj);

    dec_ref(func_obj);

    SET_NULL;
}

static void exe_return_s(Node *node)
{
    safe_exe(HEAD_NODE(node));
    state->must_return = true;
}

static void exe_stack_op(Node *node)
{
    int op_tok = TOK(node->tokens->head->value)->type;
    OpErr err = { "", "" };
    Nst_Obj *res = NULL;
    if ( !safe_exe(HEAD_NODE(node)) )
        return;
    Nst_Obj *ob1 = state->value;
    inc_ref(ob1);

    if ( op_tok == L_AND )
    {
        Nst_Obj *bool_obj = obj_cast(ob1, nst_t_bool, NULL);
        if ( bool_obj == nst_false )
        {
            dec_ref(ob1);
            SET_VALUE(nst_false); // ref incremented by bool_obj
            return;
        }
    }
    else if ( op_tok == L_OR )
    {
        Nst_Obj *bool_obj = obj_cast(ob1, nst_t_bool, NULL);
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
    Nst_Obj *ob2 = state->value;
    inc_ref(ob2);

    switch ( op_tok ) {
    case ADD:    res = obj_add(ob1, ob2, &err);   break;
    case SUB:    res = obj_sub(ob1, ob2, &err);   break;
    case MUL:    res = obj_mul(ob1, ob2, &err);   break;
    case DIV:    res = obj_div(ob1, ob2, &err);   break;
    case POW:    res = obj_pow(ob1, ob2, &err);   break;
    case MOD:    res = obj_mod(ob1, ob2, &err);   break;
    case B_AND:  res = obj_bwand(ob1, ob2, &err); break;
    case B_OR:   res = obj_bwor(ob1, ob2, &err);  break;
    case B_XOR:  res = obj_bwxor(ob1, ob2, &err); break;
    case LSHIFT: res = obj_bwls(ob1, ob2, &err);  break;
    case RSHIFT: res = obj_bwrs(ob1, ob2, &err);  break;
    case CONCAT: res = obj_concat(ob1, ob2, &err);break;
    case L_AND:  res = obj_lgand(ob1, ob2, &err); break;
    case L_OR:   res = obj_lgor(ob1, ob2, &err);  break;
    case L_XOR:  res = obj_lgxor(ob1, ob2, &err); break;
    case GT:     res = obj_gt(ob1, ob2, &err);    break;
    case LT:     res = obj_lt(ob1, ob2, &err);    break;
    case EQ:     res = obj_eq(ob1, ob2, &err);    break;
    case NEQ:    res = obj_ne(ob1, ob2, &err);    break;
    case GTE:    res = obj_ge(ob1, ob2, &err);    break;
    case LTE:    res = obj_le(ob1, ob2, &err);    break;
    default: return;
    }

    if ( res == NULL )
    {
        if ( errno == ENOMEM )
            return;
        SET_ERROR(SET_GENERAL_ERROR_INT, node->start, node->end, err.message);
        state->traceback->error->name = err.name;
        dec_ref(ob1);
        dec_ref(ob2);
        return;
    }

    dec_ref(ob1);
    dec_ref(ob2);

    SET_VALUE(res);
}

static void exe_local_stack_op(Node *node)
{
    int op_tok = TOK(node->tokens->head->value)->type;
    register size_t arg_count = node->nodes->size;
    OpErr err = { "", "" };

    switch ( op_tok )
    {
    case CAST:
        if ( arg_count != 2 )
        {
            SET_ERROR(
                SET_CALL_ERROR_INT,
                node->start,
                node->end,
                arg_count > 2 ? TOO_MANY_ARGS("cast") : TOO_FEW_ARGS("cast")
            );
            return;
        }

        if ( !safe_exe(HEAD_NODE(node)) )
            return;
        Nst_Obj *type = state->value;
        inc_ref(type);
        if ( !safe_exe(TAIL_NODE(node)) )
        {
            dec_ref(type);
            return;
        }
        Nst_Obj *obj = state->value;
        inc_ref(obj);

        Nst_Obj *res = obj_cast(obj, type, &err);
        if ( res == NULL )
        {
            if ( errno == ENOMEM )
                return;
            SET_ERROR(SET_GENERAL_ERROR_INT, node->start, node->end, err.message);
            state->traceback->error->name = err.name;
            dec_ref(type);
            dec_ref(obj);
            return;
        }

        dec_ref(type);
        dec_ref(obj);

        SET_VALUE(res);
        break;
    case CALL:
        if ( !safe_exe(TAIL_NODE(node)) )
            return;
        Nst_Obj *func_obj = state->value;
        inc_ref(func_obj);

        if ( func_obj->type != nst_t_func )
        {
            SET_ERROR(
                SET_TYPE_ERROR_INT,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Func"), func_obj->type_name)
            );
            dec_ref(func_obj);
            return;
        }

        Nst_func func = *AS_FUNC(func_obj);

        if ( arg_count - 1 != func.arg_num )
        {
            SET_ERROR(
                SET_CALL_ERROR_INT,
                node->start,
                node->end,
                arg_count - 1 > func.arg_num ? TOO_MANY_ARGS_FUNC : TOO_FEW_ARGS_FUNC
            );

            dec_ref(func_obj);
            return;
        }

        Nst_Obj **args_arr = malloc(func.arg_num * sizeof(Nst_Obj *));
        if ( args_arr == NULL )
        {
            errno = ENOMEM;
            return;
        }

        LLNode *n = node->nodes->head;
        for ( size_t i = 0; i < func.arg_num; i++ )
        {
            if ( !safe_exe(n->value) )
            {
                for ( size_t j = 0; j < i; j++ )
                    dec_ref(args_arr[i]);
                dec_ref(func_obj);
                free(args_arr);
                return;
            }
            Nst_Obj *val = state->value;
            inc_ref(val);
            args_arr[i] = val;

            n = n->next;
        }

        call_func_internal(node, &func, args_arr);
        dec_ref(func_obj);

        for ( size_t i = 0; i < func.arg_num; i++ )
            dec_ref(args_arr[i]);
        free(args_arr);

        break;
    case RANGE:
        if ( arg_count != 3 && arg_count != 2 )
        {
            SET_ERROR(
                SET_CALL_ERROR_INT,
                node->start,
                node->end,
                arg_count > 3 ? TOO_MANY_ARGS("range") : TOO_FEW_ARGS("range")
            );
            return;
        }

        if ( !safe_exe(HEAD_NODE(node)) )
            return;
        Nst_Obj *start = state->value;
        inc_ref(start);
        if ( !safe_exe(TAIL_NODE(node)) )
        {
            dec_ref(start);
            return;
        }
        Nst_Obj *stop = state->value;
        inc_ref(stop);

        Nst_Obj *step = NULL;

        if ( start->type != nst_t_int )
        {
            SET_ERROR(
                SET_TYPE_ERROR_INT,
                HEAD_NODE(node)->start,
                HEAD_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Int"), start->type_name)
            );

            dec_ref(start);
            dec_ref(stop);
            return;
        }

        if ( stop->type != nst_t_int )
        {
            SET_ERROR(
                SET_TYPE_ERROR_INT,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Int"), start->type_name)
            );

            dec_ref(start);
            dec_ref(stop);
            return;
        }

        if ( arg_count == 3 )
        {
            Node *n = node->nodes->head->next->value;
            if ( !safe_exe(n) )
            {
                dec_ref(start);
                dec_ref(stop);
                return;
            }
            step = state->value;
            inc_ref(step);

            if ( step->type != nst_t_int )
            {
                SET_ERROR(
                    SET_TYPE_ERROR_INT,
                    TAIL_NODE(node)->start,
                    TAIL_NODE(node)->end,
                    format_type_error(EXPECTED_TYPE("Int"), start->type_name)
                );

                dec_ref(start);
                dec_ref(stop);
                dec_ref(step);
                return;
            }

            if ( AS_INT(step) == 0 )
            {
                SET_ERROR(
                    SET_VALUE_ERROR_INT,
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
                step = make_obj_free(new_int( 1), nst_t_int);
            else
                step = make_obj_free(new_int(-1), nst_t_int);
        }

        Nst_Obj *idx = make_obj_free(new_int(0), nst_t_int);

        Nst_sequence *data_seq = new_array_empty(4);
        set_value_seq(data_seq, 0, idx);
        set_value_seq(data_seq, 1, start);
        set_value_seq(data_seq, 2, stop);
        set_value_seq(data_seq, 3, step);

        dec_ref(idx);
        dec_ref(start);
        dec_ref(stop);
        dec_ref(step);

        SET_VALUE(new_iter_obj(new_iter(
            new_func_obj(new_cfunc(1, num_iter_start)),
            new_func_obj(new_cfunc(1, num_iter_advance)),
            new_func_obj(new_cfunc(1, num_iter_is_done)),
            new_func_obj(new_cfunc(1, num_iter_get_val)),
            make_obj(data_seq, nst_t_arr, destroy_seq)
        )));

        break;
    default:
        return;
    }
}

static void exe_local_op(Node *node)
{
    int op_tok = TOK(node->tokens->head->value)->type;
    OpErr err = { "", "" };
    Nst_Obj *res = NULL;
    if ( !safe_exe(HEAD_NODE(node)) )
        return;
    Nst_Obj *ob = state->value;

    switch ( op_tok )
    {
    case LEN:    res = obj_len(ob, &err);    break;
    case L_NOT:  res = obj_lgnot(ob, &err);  break;
    case B_NOT:  res = obj_bwnot(ob, &err);  break;
    case STDIN:  res = obj_stdin(ob, &err);  break;
    case STDOUT: res = obj_stdout(ob, &err); break;
    case TYPEOF: res = obj_typeof(ob, &err); break;
    case IMPORT: res = obj_import(ob, &err); break;
    case NEG:    res = obj_neg(ob, &err);    break;
    case LOC_CALL:
    {
        Nst_func *func = AS_FUNC(ob);
        if ( func->arg_num != 0 )
        {
            SET_ERROR(
                SET_CALL_ERROR_INT,
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

        if ( state->error_occurred )
        {
            LList_append(state->traceback->positions, &node->start, false);
            LList_append(state->traceback->positions, &node->end, false);
        }
        else
        {
            SET_ERROR(SET_GENERAL_ERROR_INT, node->start, node->end, err.message);
            state->traceback->error->name = err.name;
        }

        return;
    }

    SET_VALUE(res);
}

static void exe_arr_or_vect_lit(Node *node)
{
    bool is_arr = node->type == ARR_LIT;

    if ( node->tokens->size != 0 )
    {
        if ( !safe_exe(HEAD_NODE(node)) )
            return;
        Nst_Obj *value = inc_ref(state->value);
        if ( !safe_exe(TAIL_NODE(node)) )
        {
            dec_ref(value);
            return;
        }

        Nst_Obj *quantity = state->value;
        if ( quantity->type != nst_t_int )
        {
            SET_ERROR(
                SET_VALUE_ERROR_INT,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Int"), quantity->type_name)
            );
            dec_ref(value);
            return;
        }

        Nst_int size = AS_INT(quantity);
        if ( size < 0 )
        {
            SET_ERROR(
                SET_VALUE_ERROR_INT,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                NEGATIVE_SIZE_FOR_SEQUENCE
            );
            dec_ref(value);
            return;
        }

        Nst_sequence *seq = is_arr ? new_array_empty((size_t)size)
                                   : new_vector_empty((size_t)size);
        if ( seq == NULL )
        {
            SET_ERROR(
                SET_MEMORY_ERROR_INT,
                node->start,
                node->end,
                RAN_OUT_OF_MEMORY
            );
            dec_ref(value);
            return;
        }

        for ( Nst_int i = 0; i < size; i++ )
            set_value_seq(seq, i, value);

        dec_ref(value);
        if ( is_arr )
            SET_VALUE(new_arr_obj(seq));
        else
            SET_VALUE(new_vect_obj(seq));
        return;
    }

    Nst_sequence *seq = is_arr ? new_array_empty(node->nodes->size)
                               : new_vector_empty(node->nodes->size);
    size_t i = 0;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        if ( !safe_exe(n->value) )
            return;
        set_value_seq(seq, i++, state->value);
    }

    if ( is_arr )
        SET_VALUE(new_arr_obj(seq));
    else
        SET_VALUE(new_vect_obj(seq));
}

static void exe_map_lit(Node *node)
{
    Nst_map *map = new_map();
    register Nst_Obj *key = NULL;
    register Node *key_node = NULL;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        if ( !safe_exe(n->value) )
            return;
        if ( key == NULL )
        {
            key_node = n->value;
            key = state->value;
            inc_ref(state->value);
            continue;
        }

        if ( !map_set(map, key, state->value) )
        {
            SET_ERROR(
                SET_TYPE_ERROR_INT,
                key_node->start,
                key_node->end,
                format_type_error(UNHASHABLE_TYPE, key->type_name);
            );

            return;
        }

        dec_ref(key);
        key = NULL;
    }

    SET_VALUE(make_obj(map, nst_t_map, destroy_map));
}

static void exe_value(Node *node)
{
    Token *tok = LList_peek_front(node->tokens);
    inc_ref(tok->value);

    switch ( tok->type )
    {
    case N_INT:
    case N_REAL:
    case STRING:
        SET_VALUE(tok->value);
        break;
    }
}

static void exe_access(Node *node)
{
    Nst_Obj *key = HEAD_TOK(node)->value;
    Nst_Obj *val = get_val(state->vt, key);
    if ( val == NULL )
    {
        SET_NULL;
        return;
    }
    SET_VALUE(val);
}

static void exe_extract_e(Node *node)
{
    if ( !safe_exe(HEAD_NODE(node)) )
        return;
    Nst_Obj *container = state->value;
    inc_ref(container);
    if ( !safe_exe(TAIL_NODE(node)) )
    {
        dec_ref(container);
        return;
    }
    Nst_Obj *key = state->value;
    inc_ref(key);

    if ( container->type == nst_t_arr || container->type == nst_t_vect )
    {
        if ( key->type != nst_t_int )
        {
            SET_ERROR(
                SET_TYPE_ERROR_INT,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Int"), key->type_name)
            );

            dec_ref(container);
            dec_ref(key);
            return;
        }

        Nst_int idx = AS_INT(key);
        Nst_Obj *val = get_value_seq(container->value, idx);

        if ( val == NULL )
        {
            SET_ERROR(
                SET_VALUE_ERROR_INT,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_idx_error(
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
                SET_TYPE_ERROR_INT,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Int"), key->type_name)
            );

            dec_ref(container);
            dec_ref(key);
            return;
        }

        Nst_string *str = AS_STR(container);
        Nst_int idx = AS_INT(key);
        if ( idx < 0 )
            idx += str->len;

        if ( idx < 0 || idx >= (Nst_int)str->len )
        {
            SET_ERROR(
                SET_VALUE_ERROR_INT,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_idx_error(INDEX_OUT_OF_BOUNDS("Str"), idx, str->len)
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

        Nst_Obj *val = make_obj(new_string(ch, 1, true), nst_t_str, destroy_string);

        SET_VALUE(val);
    }
    else if ( container->type == nst_t_map )
    {
        Nst_Obj *val = map_get(container->value, key);

        if ( val == NULL )
        {
            if ( key->hash == -1 )
            {
                SET_ERROR(
                    SET_VALUE_ERROR_INT,
                    TAIL_NODE(node)->start,
                    TAIL_NODE(node)->end,
                    format_type_error(UNHASHABLE_TYPE, key->type_name)
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
            SET_TYPE_ERROR_INT,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            format_type_error(EXPECTED_TYPE("Array', 'Vector', 'Map' or 'Str"), container->type_name)
        );
    }

    dec_ref(key);
    dec_ref(container);
}

static void exe_assign_e(Node *node)
{
    Node *value_node = HEAD_NODE(node);
    Node *name_node = TAIL_NODE(node);

    if ( !safe_exe(value_node) )
        return;
    Nst_Obj *value = state->value;

    if ( name_node->type == ACCESS )
    {
        Nst_Obj *name = HEAD_TOK(name_node)->value;

        set_val(state->vt, name, value);
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
    Nst_Obj *obj_to_set = state->value;
    inc_ref(obj_to_set);

    if ( !safe_exe(TAIL_NODE(name_node)) )
    {
        dec_ref(value);
        dec_ref(obj_to_set);
        return;
    }
    Nst_Obj *idx = state->value;
    inc_ref(idx);

    SET_VALUE(value);

    if ( obj_to_set->type == nst_t_map )
    {
        bool res = map_set(AS_MAP(obj_to_set), idx, value);
        if ( !res )
        {
            SET_ERROR(
                SET_TYPE_ERROR_INT,
                name_node->start,
                name_node->end,
                format_type_error(UNHASHABLE_TYPE, idx->type_name)
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
                SET_TYPE_ERROR_INT,
                TAIL_NODE(name_node)->start,
                TAIL_NODE(name_node)->end,
                format_type_error(EXPECTED_TYPE("Int"), idx->type_name)
            );

            dec_ref(obj_to_set);
            dec_ref(idx);
            return;
        }

        bool res = set_value_seq(AS_SEQ(obj_to_set), AS_INT(idx), value);

        if ( !res )
        {
            SET_ERROR(
                SET_VALUE_ERROR_INT,
                name_node->start,
                name_node->end,
                format_idx_error(
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
            SET_TYPE_ERROR_INT,
            HEAD_NODE(name_node)->start,
            HEAD_NODE(name_node)->end,
            format_type_error(EXPECTED_TYPE("Array', 'Vector' or 'Map"), obj_to_set->type_name)
        );

        dec_ref(obj_to_set);
        dec_ref(idx);
        return;
    }
}

static void exe_continue_s(Node *node)
{
    state->must_continue = true;
    SET_NULL;
}

static void exe_break_s(Node *node)
{
    state->must_break = true;
    SET_NULL;
}

static void exe_switch_s(Node *node)
{
    size_t nodes_len = node->nodes->size;

    if ( !safe_exe(HEAD_NODE(node)) )
        return;
    Nst_Obj *main_val = inc_ref(state->value);

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
            state->must_continue = false;
            SET_NULL;
            return;
        }

        if ( !safe_exe(n->value) )
        {
            dec_ref(main_val);
            return;
        }

        Nst_Obj *res = obj_eq(main_val, state->value, NULL);
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
                state->must_continue = false;
                SET_NULL;
                return;
            }

            while ( state->must_continue )
            {
                if ( i == nodes_len )
                    exe_node(NODE(n->next->value));
                if ( i >= nodes_len )
                {
                    dec_ref(main_val);
                    state->must_continue = false;
                    SET_NULL;
                    return;
                }

                n = n->next->next;
                i += 2;
                state->must_continue = false;

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

Nst_Obj *call_func(Nst_func *func, Nst_Obj **args, OpErr *err)
{
    if ( func->body != NULL )
    {
        VarTable *prev_vt = state->vt;

        if ( state->vt->global_table != NULL )
            state->vt = new_var_table(state->vt->global_table, NULL, NULL);
        else
            state->vt = new_var_table(state->vt, NULL, NULL);

        for ( size_t i = 0; i < func->arg_num; i++ )
            set_val(state->vt, func->args[i], args[i]);

        exe_node(func->body);
        if ( state->error_occurred )
        {
            err->name = state->traceback->error->name;
            err->message = state->traceback->error->message;

            destroy_map(state->vt->vars);
            free(state->vt);
            state->vt = prev_vt;
            return NULL;
        }

        state->must_return = false;

        if ( func->body->type != LONG_S && func->body->type != RETURN_S )
            SET_NULL;

        destroy_map(state->vt->vars);
        free(state->vt);

        state->vt = prev_vt;
        return inc_ref(state->value);
    }
    else
    {
        assert(func->cbody != NULL);
        Nst_Obj *res = func->cbody(func->arg_num, args, err);
        return res;
    }
}

static void call_func_internal(Node *node, Nst_func *func, Nst_Obj **args)
{
    OpErr err = { "", "" };
    Nst_Obj *res = call_func(func, args, &err);

    if ( res == NULL )
    {
        if ( state->traceback->error == NULL )
        {
            SET_ERROR(SET_GENERAL_ERROR_INT, node->start, node->end, err.message);
            state->traceback->error->name = err.name;
            return;
        }
        else
        {
            LList_append(state->traceback->positions, &node->start, false);
            LList_append(state->traceback->positions, &node->end, false);
            return;
        }
    }

    SET_VALUE(res);
}

size_t get_full_path(char *file_path, char **buf, char **file_part)
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

static Nst_string *make_cwd(char *file_path)
{
    char *path = NULL;
    char *file_part = NULL;

    get_full_path(file_path, &path, &file_part);

    *(file_part - 1) = 0;

    return new_string(path, file_part - path, true);
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

static Nst_sequence *make_argv(int argc, char **argv)
{
    Nst_sequence *args = new_array_empty(argc - 1);

    for ( int i = 1; i < argc; i++ )
        set_value_seq(args, i - 1, new_str_obj(new_string_raw(argv[i], false)));

    return args;
}
