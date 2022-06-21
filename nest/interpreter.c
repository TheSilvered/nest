#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <windows.h>
#include "interpreter.h"
#include "error.h"
#include "obj.h"
#include "nst_types.h"
#include "var_table.h"
#include "tokens.h"
#include "obj_ops.h"
#include "hash.h"
#include "llist.h"
#include "lib_import.h"
#include "lexer.h"
#include "parser.h"

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

#define SAFE_EXE(node) do { \
    exe_node(node, vt, state); \
    if ( state->error_occurred ) \
        return; \
    } while ( 0 )

#define OBJ_INIT_FARGS \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *

#define HEAD_NODE(node) (NODE(node->nodes->head->value))
#define TAIL_NODE(node) (NODE(node->nodes->tail->value))
#define HEAD_TOK(node) (TOK(node->tokens->head->value))
#define TAIL_TOK(node) (TOK(node->tokens->tail->value))

typedef struct ExecutionState
{
    Nst_Traceback *traceback;
    Nst_Obj *value;
    bool error_occurred;
    bool must_return;
    bool must_continue;
    bool must_break;
    LList *loaded_libs;
    Nst_string *curr_path;
}
ExecutionState;

static inline void exe_node(Node *node, VarTable *vt, ExecutionState *state);
static void exe_long_s(Node *node, VarTable *vt, ExecutionState *state);
static void exe_while_l(Node *node, VarTable *vt, ExecutionState *state);
static void exe_dowhile_l(Node *node, VarTable *vt, ExecutionState *state);
static void exe_for_l(Node *node, VarTable *vt, ExecutionState *state);
static void exe_for_as_l(Node *node, VarTable *vt, ExecutionState *state);
static void exe_if_e(Node *node, VarTable *vt, ExecutionState *state);
static void exe_func_declr(Node *node, VarTable *vt, ExecutionState *state);
static void exe_return_s(Node *node, VarTable *vt, ExecutionState *state);
static void exe_stack_op(Node *node, VarTable *vt, ExecutionState *state);
static void exe_local_stack_op(Node *node, VarTable *vt, ExecutionState *state);
static void exe_local_op(Node *node, VarTable *vt, ExecutionState *state);
static void exe_arr_lit(Node *node, VarTable *vt, ExecutionState *state);
static void exe_vect_lit(Node *node, VarTable *vt, ExecutionState *state);
static void exe_map_lit(Node *node, VarTable *vt, ExecutionState *state);
static void exe_value(Node *node, VarTable *vt, ExecutionState *state);
static void exe_access(Node *node, VarTable *vt, ExecutionState *state);
static void exe_extract_e(Node *node, VarTable *vt, ExecutionState *state);
static void exe_assign_e(Node *node, VarTable *vt, ExecutionState *state);
static void exe_continue_s(Node *node, VarTable *vt, ExecutionState *state);
static void exe_break_s(Node *node, VarTable *vt, ExecutionState *state);

static void call_func(Node *node,
                      Nst_func *func,
                      VarTable *vt,
                      Nst_Obj **args,
                      ExecutionState *state);
static Nst_Obj *import_lib(Nst_Obj *ob, OpErr *err, ExecutionState *state);

void run(Node *node)
{
    if ( node == NULL )
        return;

    size_t path_len = strlen(node->start.filename);
    char *path = malloc(sizeof(char) * (path_len + 1));
    if ( path == NULL )
        return;

    strcpy(path, node->start.filename);

    for ( char *i = path + path_len - 1; i - path + 1; i-- )
    {
        if ( *i == '/' || *i == '\\' )
            break;
        *i = 0;
        --path_len;
    }

    Nst_string *path_str = new_string(path, path_len, true);

    Nst_Traceback tb = { NULL, LList_new() };
    ExecutionState state = {
        &tb,
        nst_null,
        false,
        false,
        false,
        false,
        LList_new(),
        path_str
    };
    inc_ref(nst_null);
    VarTable *vt = new_var_table(NULL, path_str);

    exe_node(node, vt, &state);

    if ( state.error_occurred && state.traceback->error != NULL )
        print_traceback(tb);

    destroy_map(vt->vars);
    free(vt);
    dec_ref(state.value);
    LList_destroy(state.loaded_libs, FreeLibrary);
}

Nst_map *run_module(char *file_name, ExecutionState *state)
{
    Nst_string *prev_path = state->curr_path;

    size_t new_path_len = strlen(file_name);
    char *new_path = malloc(sizeof(char) * (prev_path->len + new_path_len + 1));
    if ( new_path == NULL )
        return NULL;

    strcpy(new_path, prev_path->value);
    strcat(new_path, file_name);

    for ( char *i = new_path + new_path_len - 1; i - new_path_len + 1; i-- )
    {
        if ( *i == '/' || *i == '\\' )
            break;
        *i = 0;
        --new_path_len;
    }

    Nst_string *path_str = new_string(new_path, new_path_len, true);
    state->curr_path = path_str;

    Node *module_ast = parse(ftokenize(file_name));

    if ( module_ast == NULL )
        return NULL;

    SET_NULL;
    VarTable *vt = new_var_table(NULL, path_str);

    exe_node(module_ast, vt, state);

    if ( state->error_occurred )
        return NULL;

    Nst_map *var_map = vt->vars;
    free(vt);
    state->curr_path = prev_path;
    return var_map;
}

static inline void exe_node(Node *node, VarTable *vt, ExecutionState *state)
{
    switch ( node->type )
    {
    case LONG_S:         exe_long_s        (node, vt, state); break;
    case WHILE_L:        exe_while_l       (node, vt, state); break;
    case DOWHILE_L:      exe_dowhile_l     (node, vt, state); break;
    case FOR_L:          exe_for_l         (node, vt, state); break;
    case FOR_AS_L:       exe_for_as_l      (node, vt, state); break;
    case IF_E:           exe_if_e          (node, vt, state); break;
    case FUNC_DECLR:     exe_func_declr    (node, vt, state); break;
    case RETURN_S:       exe_return_s      (node, vt, state); break;
    case STACK_OP:       exe_stack_op      (node, vt, state); break;
    case LOCAL_STACK_OP: exe_local_stack_op(node, vt, state); break;
    case LOCAL_OP:       exe_local_op      (node, vt, state); break;
    case ARR_LIT:        exe_arr_lit       (node, vt, state); break;
    case VECT_LIT:       exe_vect_lit      (node, vt, state); break;
    case MAP_LIT:        exe_map_lit       (node, vt, state); break;
    case VALUE:          exe_value         (node, vt, state); break;
    case ACCESS:         exe_access        (node, vt, state); break;
    case EXTRACT_E:      exe_extract_e     (node, vt, state); break;
    case ASSIGN_E:       exe_assign_e      (node, vt, state); break;
    case CONTINUE_S:     exe_continue_s    (node, vt, state); break;
    case BREAK_S:        exe_break_s       (node, vt, state); break;
    default: return;
    }
}

static void exe_long_s(Node *node, VarTable *vt, ExecutionState *state)
{
    for ( LLNode *cursor = node->nodes->head;
          cursor != NULL;
          cursor = cursor->next )
    {
        SAFE_EXE(cursor->value);

        if ( state->must_return )
            return;
        if ( state->must_break || state->must_continue )
            break;
    }
    SET_NULL;
}

static void exe_while_l(Node *node, VarTable *vt, ExecutionState *state)
{
    while ( true )
    {
        SAFE_EXE(HEAD_NODE(node));
        Nst_Obj *condition = obj_cast(state->value, nst_t_bool, NULL);

        if ( condition == nst_false )
            break;

        SAFE_EXE(TAIL_NODE(node));

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

static void exe_dowhile_l(Node *node, VarTable *vt, ExecutionState *state)
{
    while ( true )
    {
        SAFE_EXE(TAIL_NODE(node));

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

        SAFE_EXE(HEAD_NODE(node));
        Nst_Obj *condition = obj_cast(state->value, nst_t_bool, NULL);

        if ( condition == nst_false )
            break;
    }
}

static void exe_for_l(Node *node, VarTable *vt, ExecutionState *state)
{
    SAFE_EXE(HEAD_NODE(node));
    Nst_Obj *range = state->value;
    inc_ref(range);

    if ( range->type != nst_t_int )
    {
        SET_ERROR(
            TYPE_ERROR,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            format_type_error(EXPECTED_TYPE("Int"), range->type_name)
        );
        return;
    }


    for ( Nst_int i = 0, n = AS_INT(range); i < n; i++ )
    {
        SAFE_EXE(TAIL_NODE(node));

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

static void exe_for_as_l(Node *node, VarTable *vt, ExecutionState *state)
{
    SAFE_EXE(HEAD_NODE(node));
    Nst_Obj *iter_obj = state->value;
    inc_ref(iter_obj);

    if ( iter_obj->type != nst_t_iter )
    {
        SET_ERROR(
            TYPE_ERROR,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            format_type_error(EXPECTED_TYPE("Iter"), iter_obj->type_name)
        );
        return;
    }

    Nst_iter *iterator = iter_obj->value;
    Nst_Obj *iter_val = iterator->value;

    call_func(node, iterator->start->value, vt, &iter_val, state);
    if ( state->error_occurred )
        return;

    Nst_Obj *var_name = HEAD_TOK(node)->value;

    while ( true )
    {
        call_func(node, iterator->is_done->value, vt, &iter_val, state);
        if ( state->error_occurred )
            return;

        if ( state->value->type != nst_t_bool )
        {
            SET_ERROR(
                TYPE_ERROR,
                HEAD_NODE(node)->start,
                HEAD_NODE(node)->end,
                format_type_error(EXPECTED_BOOL_ITER_IS_DONE, state->value->type_name)
            );
            return;
        }

        if ( AS_BOOL(state->value) )
        {
            SET_NULL;
            break;
        }

        call_func(node, iterator->get_val->value, vt, &iter_val, state);
        if ( state->error_occurred )
            return;

        set_val(vt, var_name, state->value);

        SAFE_EXE(TAIL_NODE(node));

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

        call_func(node, iterator->advance->value, vt, &iter_val, state);
        if ( state->error_occurred )
            return;
    }

    dec_ref(iter_obj);
}

static void exe_if_e(Node *node, VarTable *vt, ExecutionState *state)
{
    SAFE_EXE(HEAD_NODE(node));
    Nst_Obj *condition = obj_cast(state->value, nst_t_bool, NULL);

    // If there is no else clause
    if ( node->nodes->size == 2 )
    {
        if ( condition == nst_true )
            SAFE_EXE(TAIL_NODE(node));
        else
            SET_NULL;
    }
    else
    {
        if ( condition == nst_true )
            SAFE_EXE(node->nodes->head->next->value);
        else
            SAFE_EXE(TAIL_NODE(node));
    }

    dec_ref(condition);
}

static void exe_func_declr(Node *node, VarTable *vt, ExecutionState *state)
{
    Nst_func *func = new_func(node->tokens->size - 1);

    register size_t i = 0;
    for ( LLNode *n = node->tokens->head->next; n != NULL; n = n->next )
        func->args[i++] = TOK(n->value)->value;

    func->body = HEAD_NODE(node);

    Nst_Obj *func_obj = new_func_obj(func);
    Nst_Obj *func_name = HEAD_TOK(node)->value;

    set_val(vt, func_name, func_obj);

    dec_ref(func_obj);

    SET_NULL;
}

static void exe_return_s(Node *node, VarTable *vt, ExecutionState *state)
{
    SAFE_EXE(HEAD_NODE(node));
    state->must_return = true;
}

static void exe_stack_op(Node *node, VarTable *vt, ExecutionState *state)
{
    int op_tok = TOK(node->tokens->head->value)->type;
    OpErr err = { "", "" };
    Nst_Obj *res = NULL;
    SAFE_EXE(HEAD_NODE(node));
    Nst_Obj *ob1 = state->value;
    inc_ref(ob1);
    SAFE_EXE(TAIL_NODE(node));
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
        SET_ERROR(GENERAL_ERROR, node->start, node->end, err.message);
        state->traceback->error->name = err.name;

        return;
    }

    dec_ref(ob1);
    dec_ref(ob2);

    SET_VALUE(res);
}

static void exe_local_stack_op(Node *node, VarTable *vt, ExecutionState *state)
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
                CALL_ERROR,
                node->start,
                node->end,
                arg_count > 2 ? TOO_MANY_ARGS("cast") : TOO_FEW_ARGS("cast")
            );
            return;
        }

        SAFE_EXE(HEAD_NODE(node));
        Nst_Obj *type = state->value;
        inc_ref(type);
        SAFE_EXE(TAIL_NODE(node));
        Nst_Obj *obj = state->value;
        inc_ref(obj);

        Nst_Obj *res = obj_cast(obj, type, &err);
        if ( res == NULL )
        {
            if ( errno == ENOMEM )
                return;
            SET_ERROR(GENERAL_ERROR, node->start, node->end, err.message);
            state->traceback->error->name = err.name;
            return;
        }

        dec_ref(type);
        dec_ref(obj);

        SET_VALUE(res);
        break;
    case CALL:
        SAFE_EXE(TAIL_NODE(node));
        Nst_Obj *func_obj = state->value;
        inc_ref(func_obj);

        if ( func_obj->type != nst_t_func )
        {
            SET_ERROR(
                TYPE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Func"), func_obj->type_name)
            );

            return;
        }

        Nst_func func = *AS_FUNC(func_obj);

        if ( arg_count - 1 != func.arg_num )
        {
            SET_ERROR(
                CALL_ERROR,
                node->start,
                node->end,
                arg_count - 1 > func.arg_num ? TOO_MANY_ARGS_FUNC : TOO_FEW_ARGS_FUNC
            );
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
            SAFE_EXE(n->value);
            Nst_Obj *val = state->value;
            inc_ref(val);
            args_arr[i] = val;

            n = n->next;
        }

        call_func(node, &func, vt, args_arr, state);
        dec_ref(func_obj);

        for ( size_t i = 0; i < func.arg_num; i++ )
            dec_ref(args_arr[i]);
        free(args_arr);

        break;
    case RANGE:
        if ( arg_count != 3 && arg_count != 2 )
        {
            SET_ERROR(
                CALL_ERROR,
                node->start,
                node->end,
                arg_count > 3 ? TOO_MANY_ARGS("range") : TOO_FEW_ARGS("range")
            );
            return;
        }

        SAFE_EXE(HEAD_NODE(node));
        Nst_Obj *start = state->value;
        inc_ref(start);
        SAFE_EXE(TAIL_NODE(node));
        Nst_Obj *stop = state->value;
        inc_ref(stop);

        Nst_Obj *step = NULL;

        if ( start->type != nst_t_int )
        {
            SET_ERROR(
                TYPE_ERROR,
                HEAD_NODE(node)->start,
                HEAD_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Int"), start->type_name)
            );
            return;
        }

        if ( stop->type != nst_t_int )
        {
            SET_ERROR(
                TYPE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Int"), start->type_name)
            );
            return;
        }

        if ( arg_count == 3 )
        {
            Node *n = node->nodes->head->next->value;
            SAFE_EXE(n);
            step = state->value;
            inc_ref(step);

            if ( step->type != nst_t_int )
            {
                SET_ERROR(
                    TYPE_ERROR,
                    TAIL_NODE(node)->start,
                    TAIL_NODE(node)->end,
                    format_type_error(EXPECTED_TYPE("Int"), start->type_name)
                );
                return;
            }

            if ( AS_INT(step) == 0 )
            {
                SET_ERROR(
                    VALUE_ERROR,
                    n->start,
                    n->end,
                    ZERO_RANGE_STEP
                );
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

static void exe_local_op(Node *node, VarTable *vt, ExecutionState *state)
{
    int op_tok = TOK(node->tokens->head->value)->type;
    OpErr err = { "", "" };
    Nst_Obj *res = NULL;
    SAFE_EXE(HEAD_NODE(node));
    Nst_Obj *ob = state->value;

    switch ( op_tok )
    {
    case SUB:    res = obj_neg(ob, &err);    break;
    case LEN:    res = obj_len(ob, &err);    break;
    case L_NOT:  res = obj_lgnot(ob, &err);  break;
    case B_NOT:  res = obj_bwnot(ob, &err);  break;
    case STDIN:  res = obj_stdin(ob, &err);  break;
    case STDOUT: res = obj_stdout(ob, &err); break;
    case TYPEOF: res = obj_typeof(ob, &err); break;
    case IMPORT: res = import_lib(ob, &err, state); break;
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
            SET_ERROR(GENERAL_ERROR, node->start, node->end, err.message);
            state->traceback->error->name = err.name;
        }

        return;
    }

    SET_VALUE(res);
}

static void exe_arr_lit(Node *node, VarTable *vt, ExecutionState *state)
{
    Nst_sequence *arr = new_array_empty(node->nodes->size);
    Nst_Obj **objs = arr->objs;
    size_t i = 0;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        SAFE_EXE(n->value);
        set_value_seq(arr, i++, state->value);
    }

    SET_VALUE(make_obj(arr, nst_t_arr, destroy_seq));
}

static void exe_vect_lit(Node *node, VarTable *vt, ExecutionState *state)
{
    Nst_sequence *arr = new_vector_empty(node->nodes->size);
    Nst_Obj **objs = arr->objs;
    size_t i = 0;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        SAFE_EXE(n->value);
        set_value_seq(arr, i++, state->value);
    }

    SET_VALUE(make_obj(arr, nst_t_vect, destroy_seq));
}

static void exe_map_lit(Node *node, VarTable *vt, ExecutionState *state)
{
    Nst_map *map = new_map();
    register Nst_Obj *key = NULL;
    register Node *key_node = NULL;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        SAFE_EXE(n->value);
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
                TYPE_ERROR,
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

static void exe_value(Node *node, VarTable *vt, ExecutionState *state)
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

static void exe_access(Node *node, VarTable *vt, ExecutionState *state)
{
    Nst_Obj *key = TOK(LList_peek_front(node->tokens))->value;

    Nst_Obj *val = get_val(vt, key);
    if ( val == NULL )
    {
        inc_ref(nst_null);
        val = nst_null;
    }

    SET_VALUE(val);
}

static void exe_extract_e(Node *node, VarTable *vt, ExecutionState *state)
{
    SAFE_EXE(HEAD_NODE(node));
    Nst_Obj *container = state->value;
    inc_ref(container);
    SAFE_EXE(TAIL_NODE(node));
    Nst_Obj *key = state->value;
    inc_ref(key);

    if ( container->type == nst_t_arr || container->type == nst_t_vect )
    {
        if ( key->type != nst_t_int )
        {
            SET_ERROR(
                TYPE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Int"), key->type_name)
            );
            return;
        }

        Nst_int idx = AS_INT(key);
        Nst_Obj *val = get_value_seq(container->value, idx);

        if ( val == NULL )
        {
            SET_ERROR(
                VALUE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_idx_error(
                    container->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                                 : INDEX_OUT_OF_BOUNDS("Vector"),
                    idx,
                    AS_SEQ(container)->len
                )
            );
            return;
        }

        SET_VALUE(val);
    }
    else if ( container->type == nst_t_str )
    {
        if ( key->type != nst_t_int )
        {
            SET_ERROR(
                TYPE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(EXPECTED_TYPE("Int"), key->type_name)
            );
            return;
        }

        Nst_string *str = AS_STR(container);
        Nst_int idx = AS_INT(key);
        if ( idx < 0 )
            idx += str->len;

        if ( idx < 0 || idx >= (Nst_int)str->len )
        {
            SET_ERROR(
                VALUE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_idx_error(INDEX_OUT_OF_BOUNDS("Str"), idx, str->len)
            );
            return;
        }

        char *ch = calloc(2, sizeof(char));
        if ( ch == NULL )
        {
            errno = ENOMEM;
            return;
        }

        ch[0] = str->value[idx];

        Nst_Obj *val = make_obj(new_string(ch, 1, true), nst_t_str, destroy_string);

        SET_VALUE(val);
    }
    else if ( container->type == nst_t_map )
    {
        hash_obj(key);
        if ( key->hash == -1 )
        {
            SET_ERROR(
                VALUE_ERROR,
                TAIL_NODE(node)->start,
                TAIL_NODE(node)->end,
                format_type_error(UNHASHABLE_TYPE, key->type_name)
            );
            return;
        }

        Nst_Obj *val = map_get(container->value, key);

        if ( val == NULL )
        {
            inc_ref(nst_null);
            val = nst_null;
        }

        SET_VALUE(val);
    }
    else
    {
        SET_ERROR(
            TYPE_ERROR,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            format_type_error(EXPECTED_TYPE("Array', 'Vector', 'Map' or 'Str"), container->type_name)
        );
        return;
    }

    dec_ref(key);
    dec_ref(container);
}

static void exe_assign_e(Node *node, VarTable *vt, ExecutionState *state)
{
    Node *value_node = HEAD_NODE(node);
    Node *name_node = TAIL_NODE(node);

    SAFE_EXE(value_node);
    Nst_Obj *value = state->value;

    if ( name_node->type == ACCESS )
    {
        Nst_Obj *name = HEAD_TOK(name_node)->value;

        set_val(vt, name, value);
        return; // state->value is still the same
    }
    
    inc_ref(value);

    // If name_node->type == EXTRACTION
    // can't be anything else because of the parser
    SAFE_EXE(HEAD_NODE(name_node));
    Nst_Obj *obj_to_set = state->value;
    inc_ref(obj_to_set);

    SAFE_EXE(TAIL_NODE(name_node));
    Nst_Obj *idx = state->value;
    inc_ref(idx);

    SET_VALUE(value);

    if ( obj_to_set->type == nst_t_map )
    {
        bool res = map_set(AS_MAP(obj_to_set), idx, value);
        if ( !res )
        {
            SET_ERROR(
                TYPE_ERROR,
                name_node->start,
                name_node->end,
                format_type_error(UNHASHABLE_TYPE, idx->type_name)
            );
            return;
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
                TYPE_ERROR,
                TAIL_NODE(name_node)->start,
                TAIL_NODE(name_node)->end,
                format_type_error(EXPECTED_TYPE("Int"), idx->type_name)
            );
            return;
        }
        
        bool res = set_value_seq(AS_SEQ(obj_to_set), AS_INT(idx), value);

        if ( !res )
        {
            SET_ERROR(
                VALUE_ERROR,
                name_node->start,
                name_node->end,
                format_idx_error(
                    obj_to_set->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                                  : INDEX_OUT_OF_BOUNDS("Vector"),
                    AS_INT(idx),
                    AS_SEQ(obj_to_set)->len
                )
            );
            return;
        }

        dec_ref(obj_to_set);
        dec_ref(idx);
        return;
    }
    else
    {
        SET_ERROR(
            TYPE_ERROR,
            HEAD_NODE(name_node)->start,
            HEAD_NODE(name_node)->end,
            format_type_error(EXPECTED_TYPE("Array', 'Vector' or 'Map"), obj_to_set->type_name)
        );
        return;
    }
}

static void exe_continue_s(Node *node, VarTable *vt, ExecutionState *state)
{
    state->must_continue = true;
    SET_NULL;
}

static void exe_break_s(Node *node, VarTable *vt, ExecutionState *state)
{
    state->must_break = true;
    SET_NULL;
}

static void call_func(Node *node,
                      Nst_func *func,
                      VarTable *vt,
                      Nst_Obj **args,
                      ExecutionState *state)
{
    if ( func->body != NULL )
    {
        VarTable *func_vt = NULL;

        if ( vt->global_table != NULL )
            func_vt = new_var_table(vt->global_table, NULL);
        else
            func_vt = new_var_table(vt, NULL);

        for ( size_t i = 0; i < func->arg_num; i++ )
            set_val(func_vt, func->args[i], args[i]);

        exe_node(func->body, func_vt, state);
        if ( state->error_occurred )
        {
            LList_append(state->traceback->positions, &node->start, false);
            LList_append(state->traceback->positions, &node->end, false);
            return;
        }

        state->must_return = false;

        destroy_map(func_vt->vars);
        free(func_vt);
    }
    else
    {
        assert(func->cbody != NULL);
        OpErr err = { "", "" };
        Nst_Obj *res = func->cbody(func->arg_num, args, &err);
        if ( res == NULL )
        {
            if ( errno == ENOMEM )
                return;
            SET_ERROR(GENERAL_ERROR, node->start, node->end, err.message);
            state->traceback->error->name = err.name;
            return;
        }
        SET_VALUE(res);
    }
}

static Nst_Obj *import_lib(Nst_Obj *ob, OpErr *err, ExecutionState *state)
{

    if ( ob->type != nst_t_str )
    {
        err->name = "Type Error";
        err->message = format_type_error(EXPECTED_TYPE("Str"), ob->type_name);
        return NULL;
    }

    char *file_name = AS_STR(ob)->value;
    bool c_import = false;

    if ( file_name[0] == '_' && file_name[1] == '_' &&
         file_name[2] == 'C' && file_name[3] == '_' &&
         file_name[4] == '_' && file_name[5] == ':' )
    {
        c_import = true;
        file_name += 6; // length of __C__:
    }

    char *file_path = malloc(sizeof(char) * (state->curr_path->len + strlen(file_name) + 1));
    if ( file_path == NULL )
        return NULL;

    strcpy(file_path, state->curr_path->value);
    strcat(file_path, file_name);

    Nst_iofile *file;
    if ( (file = fopen(file_path, "r")) == NULL )
    {
        err->name = "Value Error";
        err->message = format_fnf_error(FILE_NOT_FOUND, file_path);
        return NULL;
    }
    fclose(file);

    if ( !c_import )
    {
        Nst_map *map = run_module(file_path, state);

        if ( map == NULL )
        {
            state->error_occurred = true;
            return NULL;
        }

        return make_obj(map, nst_t_map, destroy_map);
    }

    HMODULE lib = LoadLibraryA(file_path);

    if ( !lib )
    {
        err->name = "Import Error";
        err->message = FILE_NOT_DLL;
        return NULL;
    }

    bool (*lib_init)() = (bool (*)())GetProcAddress(lib, "lib_init");
    if ( lib_init == NULL )
    {
        err->name = "Import Error";
        err->message = NO_LIB_INIT;
        return NULL;
    }

    if ( !lib_init() )
    {
        errno = ENOMEM;
        return NULL;
    }

    FuncDeclr *(*get_func_ptrs)() = (FuncDeclr *(*)())GetProcAddress(lib, "get_func_ptrs");
    if ( get_func_ptrs == NULL )
    {
        err->name = "Import Error";
        err->message = NO_GET_FUNC_PTRS;
        return NULL;
    }

    FuncDeclr *func_ptrs = get_func_ptrs();
    
    if ( func_ptrs == NULL )
    {
        err->name = "Import Error";
        err->message = "module was not initialized correctly";
        return NULL;
    }

    Nst_map *func_map = new_map();

    for ( size_t i = 0;; i++ )
    {
        FuncDeclr func = func_ptrs[i];
        if ( func.func_ptr == NULL )
            break;

        Nst_Obj *func_obj = new_func_obj(new_cfunc(func.arg_num, func.func_ptr));

        map_set(func_map, make_obj(func.name, nst_t_str, NULL), func_obj);
    }

    FARPROC a = GetProcAddress(lib, "init_lib_obj");

    void (*init_lib_obj)(OBJ_INIT_FARGS)
        = (void (*)(OBJ_INIT_FARGS))GetProcAddress(lib, "init_lib_obj");

    if ( init_lib_obj == NULL )
    {
        err->name = "Import Error";
        err->message = "module does not import \"obj.h\"";
        return NULL;
    }

    init_lib_obj(nst_t_type, nst_t_int,  nst_t_real, nst_t_bool,
                      nst_t_null, nst_t_str,  nst_t_arr,  nst_t_vect,
                      nst_t_map,  nst_t_func, nst_t_iter, nst_t_byte,
                      nst_t_file, nst_true,   nst_false,  nst_null);

    LList_append(state->loaded_libs, lib, false);
    return make_obj(func_map, nst_t_map, destroy_map);
}
