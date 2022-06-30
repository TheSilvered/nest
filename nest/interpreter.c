#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <windows.h>
#include "interpreter.h"
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
#include "set_error_internal.h"

#define SET_ERROR(err_macro, start, end, message) \
    do { \
        Nst_Error *error = malloc(sizeof(Nst_Error)); \
        if ( error == NULL ) \
        { \
            errno = ENOMEM; \
            return; \
        } \
        err_macro(error, start, end, message); \
        state.traceback->error = error; \
        state.error_occurred = true; \
    } while ( 0 )

#define SET_VALUE(new_value) do { \
    if ( state.value != NULL ) \
        dec_ref(state.value); \
    state.value = new_value; \
    } while ( 0 )

#define SET_NULL do { \
    SET_VALUE(nst_null); \
    inc_ref(nst_null); \
    } while ( 0 )

#define OBJ_INIT_FARGS \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *

#define exe_node(node, vt) do { \
    switch ( node->type ) \
    { \
    case LONG_S:         exe_long_s(node, vt); break; \
    case WHILE_L:        exe_while_l(node, vt); break; \
    case DOWHILE_L:      exe_dowhile_l(node, vt); break; \
    case FOR_L:          exe_for_l(node, vt); break; \
    case FOR_AS_L:       exe_for_as_l(node, vt); break; \
    case IF_E:           exe_if_e(node, vt); break; \
    case FUNC_DECLR:     exe_func_declr(node, vt); break; \
    case RETURN_S:       exe_return_s(node, vt); break; \
    case STACK_OP:       exe_stack_op(node, vt); break; \
    case LOCAL_STACK_OP: exe_local_stack_op(node, vt); break; \
    case LOCAL_OP:       exe_local_op(node, vt); break; \
    case ARR_LIT:        exe_arr_lit(node, vt); break; \
    case VECT_LIT:       exe_vect_lit(node, vt); break; \
    case MAP_LIT:        exe_map_lit(node, vt); break; \
    case VALUE:          exe_value(node, vt); break; \
    case ACCESS:         exe_access(node, vt); break; \
    case EXTRACT_E:      exe_extract_e(node, vt); break; \
    case ASSIGN_E:       exe_assign_e(node, vt); break; \
    case CONTINUE_S:     exe_continue_s(node, vt); break; \
    case BREAK_S:        exe_break_s(node, vt); break; \
    } \
    } while (0)

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
    Nst_string *curr_path;
    LList *loaded_libs;
    LList *lib_paths;
    LList *lib_handles;
}
ExecutionState;

typedef union LibHandleVal
{
    Node *ast;
    HMODULE dll;
}
LibHandleVal;

typedef struct LibHandle
{
    LibHandleVal *val;
    char *path;
}
LibHandle;

static ExecutionState state;

//static inline void exe_node(Node *node, VarTable *vt);
static void exe_long_s(Node *node, VarTable *vt);
static void exe_while_l(Node *node, VarTable *vt);
static void exe_dowhile_l(Node *node, VarTable *vt);
static void exe_for_l(Node *node, VarTable *vt);
static void exe_for_as_l(Node *node, VarTable *vt);
static void exe_if_e(Node *node, VarTable *vt);
static void exe_func_declr(Node *node, VarTable *vt);
static void exe_return_s(Node *node, VarTable *vt);
static void exe_stack_op(Node *node, VarTable *vt);
static void exe_local_stack_op(Node *node, VarTable *vt);
static void exe_local_op(Node *node, VarTable *vt);
static void exe_arr_lit(Node *node, VarTable *vt);
static void exe_vect_lit(Node *node, VarTable *vt);
static void exe_map_lit(Node *node, VarTable *vt);
static void exe_value(Node *node, VarTable *vt);
static void exe_access(Node *node, VarTable *vt);
static void exe_extract_e(Node *node, VarTable *vt);
static void exe_assign_e(Node *node, VarTable *vt);
static void exe_continue_s(Node *node, VarTable *vt);
static void exe_break_s(Node *node, VarTable *vt);

static void call_func(Node *node, Nst_func *func, VarTable *vt, Nst_Obj **args);
static Nst_Obj *import_lib(Nst_Obj *ob, OpErr *err);

static inline bool safe_exe(Node *node, VarTable *vt);
static size_t get_full_path(char *file_path, char **buf, char **file_part)
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

    *buf = path; // shrinking always succeedes
    return path_len;
}

static Nst_string *make_cwd(char *file_path)
{
    char *path = NULL;
    char *file_part = NULL;

    get_full_path(file_path, &path, &file_part);

    *file_part = 0;

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

void run(Node *node, int argc, char **argv)
{
    if ( node == NULL )
        return;

    Nst_string *path_str = make_cwd(node->start.filename);
    VarTable *vt = new_var_table(NULL, path_str);
    Nst_Traceback tb = { NULL, LList_new() };

    state.traceback = &tb;
    state.value = nst_null;
    state.error_occurred = false;
    state.must_return = false;
    state.must_continue = false;
    state.must_break = false;
    state.curr_path = path_str;
    state.loaded_libs = LList_new();
    state.lib_paths = LList_new();
    state.lib_handles = LList_new();
    set_argv(vt, argc, argv);

    inc_ref(nst_null);
    LList_append(state.lib_paths, node->start.filename, false);

    exe_node(node, vt);

    if ( state.error_occurred && state.traceback->error != NULL )
        print_traceback(tb);

    destroy_map(vt->vars);
    free(vt);
    dec_ref(state.value);
    for ( LLNode *n = state.loaded_libs->head; n != NULL; n = n->next )
    {
        void (*free_lib_func)() = (void (*)())GetProcAddress(n->value, "free_lib");
        if ( free_lib_func != NULL )
            free_lib_func();
    }
    LList_destroy(state.loaded_libs, FreeLibrary);
    LList_destroy(state.lib_paths, free);
    LList_destroy(state.lib_handles, free);
}

Nst_map *run_module(char *file_name)
{
    Nst_string *prev_path = state.curr_path;

    Nst_string *path_str = make_cwd(file_name);
    state.curr_path = path_str;

    Node *module_ast = NULL;
    bool found_ast = false;

    for ( LLNode *n = state.lib_handles->head; n != NULL; n = n->next )
    {
        LibHandle *handle = (LibHandle *)(n->value);
        if ( strcmp(handle->path, file_name) == 0 )
        {
            found_ast = true;
            module_ast = (handle->val)->ast;
            break;
        }
    }

    if ( !found_ast )
        module_ast = parse(ftokenize(file_name));

    if ( module_ast == NULL )
        return NULL;

    VarTable *vt = new_var_table(NULL, path_str);

    Nst_sequence *seq = new_array_empty(1);
    seq->objs[0] = new_str_obj(new_string_raw(file_name, true));
    Nst_Obj *arr = new_arr_obj(seq);

    map_set_str(vt->vars, "_args_", arr);
    dec_ref(arr);
    LList_append(state.lib_paths, path_str, false);

    SET_NULL;
    exe_node(module_ast, vt);

    if ( state.error_occurred )
        return NULL;

    LList_pop(state.lib_paths);

    Nst_map *var_map = vt->vars;
    free(vt);
    state.curr_path = prev_path;
    return var_map;
}

static inline bool safe_exe(Node *node, VarTable *vt)
{
    exe_node(node, vt);
    return !state.error_occurred;
}

static void exe_long_s(Node *node, VarTable *vt)
{
    for ( LLNode *cursor = node->nodes->head;
          cursor != NULL;
          cursor = cursor->next )
    {
        if ( !safe_exe(cursor->value, vt) )
        {
            return;
        }

        if ( state.must_return )
            return;
        if ( state.must_break || state.must_continue )
            break;
    }
    SET_NULL;
}

static void exe_while_l(Node *node, VarTable *vt)
{
    while ( true )
    {
        if ( !safe_exe(HEAD_NODE(node), vt) )
            return;

        Nst_Obj *condition = obj_cast(state.value, nst_t_bool, NULL);

        if ( condition == nst_false )
            break;

        if ( !safe_exe(TAIL_NODE(node), vt) )
            return;

        if ( state.must_break || state.must_return )
        {
            state.must_break = false;
            state.must_continue = false;
            break;
        }
        else if ( state.must_continue )
        {
            state.must_continue = false;
        }
    }
}

static void exe_dowhile_l(Node *node, VarTable *vt)
{
    while ( true )
    {
        if ( !safe_exe(TAIL_NODE(node), vt) )
            return;

        if ( state.must_break || state.must_return )
        {
            state.must_break = false;
            state.must_continue = false;
            break;
        }
        else if ( state.must_continue )
        {
            state.must_continue = false;
        }

        if ( !safe_exe(HEAD_NODE(node), vt) )
            return;

        Nst_Obj *condition = obj_cast(state.value, nst_t_bool, NULL);

        if ( condition == nst_false )
        {
            dec_ref(condition);
            break;
        }
        dec_ref(condition);
    }
}

static void exe_for_l(Node *node, VarTable *vt)
{
    if ( !safe_exe(HEAD_NODE(node), vt) )
        return;

    Nst_Obj *range = state.value;
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
        if ( !safe_exe(TAIL_NODE(node), vt) )
            break;

        if ( state.must_break || state.must_return )
        {
            state.must_break = false;
            state.must_continue = false;
            break;
        }
        else if ( state.must_continue )
        {
            state.must_continue = false;
        }
    }

    dec_ref(range);
}

static void exe_for_as_l(Node *node, VarTable *vt)
{
    if ( !safe_exe(HEAD_NODE(node), vt) )
        return;

    Nst_Obj *iter_obj = state.value;
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

    call_func(node, iterator->start->value, vt, &iter_val);
    if ( state.error_occurred )
    {
        dec_ref(iter_obj);
        return;
    }

    Nst_Obj *var_name = HEAD_TOK(node)->value;

    while ( true )
    {
        call_func(node, iterator->is_done->value, vt, &iter_val);
        if ( state.error_occurred )
            break;

        if ( state.value->type != nst_t_bool )
        {
            SET_ERROR(
                SET_TYPE_ERROR_INT,
                HEAD_NODE(node)->start,
                HEAD_NODE(node)->end,
                format_type_error(EXPECTED_BOOL_ITER_IS_DONE, state.value->type_name)
            );
            break;
        }

        if ( AS_BOOL(state.value) )
        {
            SET_NULL;
            break;
        }

        call_func(node, iterator->get_val->value, vt, &iter_val);
        if ( state.error_occurred )
            break;

        set_val(vt, var_name, state.value);

        if ( !safe_exe(TAIL_NODE(node), vt) )
            break;

        if ( state.must_break || state.must_return )
        {
            state.must_break = false;
            state.must_continue = false;
            break;
        }
        else if ( state.must_continue )
        {
            state.must_continue = false;
        }

        call_func(node, iterator->advance->value, vt, &iter_val);
        if ( state.error_occurred )
            return;
    }

    dec_ref(iter_obj);
}

static void exe_if_e(Node *node, VarTable *vt)
{
    if ( !safe_exe(HEAD_NODE(node), vt) )
        return;

    Nst_Obj *condition = obj_cast(state.value, nst_t_bool, NULL);

    // If there is no else clause
    if ( node->nodes->size == 2 )
    {
        if ( condition == nst_true )
            safe_exe(TAIL_NODE(node), vt);
        else
            SET_NULL;
    }
    else
    {
        if ( condition == nst_true )
            safe_exe(node->nodes->head->next->value, vt);
        else
            safe_exe(TAIL_NODE(node), vt);
    }

    dec_ref(condition);
}

static void exe_func_declr(Node *node, VarTable *vt)
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

static void exe_return_s(Node *node, VarTable *vt)
{
    safe_exe(HEAD_NODE(node), vt);
    state.must_return = true;
}

static void exe_stack_op(Node *node, VarTable *vt)
{
    int op_tok = TOK(node->tokens->head->value)->type;
    OpErr err = { "", "" };
    Nst_Obj *res = NULL;
    if ( !safe_exe(HEAD_NODE(node), vt) )
        return;
    Nst_Obj *ob1 = state.value;
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

    if ( !safe_exe(TAIL_NODE(node), vt) )
    {
        dec_ref(ob1);
        return;
    }
    Nst_Obj *ob2 = state.value;
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
        state.traceback->error->name = err.name;
        dec_ref(ob1);
        dec_ref(ob2);
        return;
    }

    dec_ref(ob1);
    dec_ref(ob2);

    SET_VALUE(res);
}

static void exe_local_stack_op(Node *node, VarTable *vt)
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

        if ( !safe_exe(HEAD_NODE(node), vt) )
            return;
        Nst_Obj *type = state.value;
        inc_ref(type);
        if ( !safe_exe(TAIL_NODE(node), vt) )
        {
            dec_ref(type);
            return;
        }
        Nst_Obj *obj = state.value;
        inc_ref(obj);

        Nst_Obj *res = obj_cast(obj, type, &err);
        if ( res == NULL )
        {
            if ( errno == ENOMEM )
                return;
            SET_ERROR(SET_GENERAL_ERROR_INT, node->start, node->end, err.message);
            state.traceback->error->name = err.name;
            dec_ref(type);
            dec_ref(obj);
            return;
        }

        dec_ref(type);
        dec_ref(obj);

        SET_VALUE(res);
        break;
    case CALL:
        if ( !safe_exe(TAIL_NODE(node), vt) )
            return;
        Nst_Obj *func_obj = state.value;
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
            if ( !safe_exe(n->value, vt) )
            {
                for ( size_t j = 0; j < i; j++ )
                    dec_ref(args_arr[i]);
                dec_ref(func_obj);
                free(args_arr);
                return;
            }
            Nst_Obj *val = state.value;
            inc_ref(val);
            args_arr[i] = val;

            n = n->next;
        }

        call_func(node, &func, vt, args_arr);
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

        if ( !safe_exe(HEAD_NODE(node), vt) )
            return;
        Nst_Obj *start = state.value;
        inc_ref(start);
        if ( !safe_exe(TAIL_NODE(node), vt) )
        {
            dec_ref(start);
            return;
        }
        Nst_Obj *stop = state.value;
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
            if ( !safe_exe(n, vt) )
            {
                dec_ref(start);
                dec_ref(stop);
                return;
            }
            step = state.value;
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

static void exe_local_op(Node *node, VarTable *vt)
{
    int op_tok = TOK(node->tokens->head->value)->type;
    OpErr err = { "", "" };
    Nst_Obj *res = NULL;
    if ( !safe_exe(HEAD_NODE(node), vt) )
        return;
    Nst_Obj *ob = state.value;

    switch ( op_tok )
    {
    case LEN:    res = obj_len(ob, &err);    break;
    case L_NOT:  res = obj_lgnot(ob, &err);  break;
    case B_NOT:  res = obj_bwnot(ob, &err);  break;
    case STDIN:  res = obj_stdin(ob, &err);  break;
    case STDOUT: res = obj_stdout(ob, &err); break;
    case TYPEOF: res = obj_typeof(ob, &err); break;
    case IMPORT: res = import_lib(ob, &err); break;
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

        call_func(node, func, vt, NULL);
        return;
    }
    default: return;
    }

    if ( res == NULL )
    {
        if ( errno == ENOMEM )
            return;

        if ( state.error_occurred )
        {
            LList_append(state.traceback->positions, &node->start, false);
            LList_append(state.traceback->positions, &node->end, false);
        }
        else
        {
            SET_ERROR(SET_GENERAL_ERROR_INT, node->start, node->end, err.message);
            state.traceback->error->name = err.name;
        }

        return;
    }

    SET_VALUE(res);
}

static void exe_arr_lit(Node *node, VarTable *vt)
{
    Nst_sequence *arr = new_array_empty(node->nodes->size);
    Nst_Obj **objs = arr->objs;
    size_t i = 0;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        if ( !safe_exe(n->value, vt) )
            return;
        set_value_seq(arr, i++, state.value);
    }

    SET_VALUE(make_obj(arr, nst_t_arr, destroy_seq));
}

static void exe_vect_lit(Node *node, VarTable *vt)
{
    Nst_sequence *arr = new_vector_empty(node->nodes->size);
    Nst_Obj **objs = arr->objs;
    size_t i = 0;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        if ( !safe_exe(n->value, vt) )
            return;
        set_value_seq(arr, i++, state.value);
    }

    SET_VALUE(make_obj(arr, nst_t_vect, destroy_seq));
}

static void exe_map_lit(Node *node, VarTable *vt)
{
    Nst_map *map = new_map();
    register Nst_Obj *key = NULL;
    register Node *key_node = NULL;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        if ( !safe_exe(n->value, vt) )
            return;
        if ( key == NULL )
        {
            key_node = n->value;
            key = state.value;
            inc_ref(state.value);
            continue;
        }

        if ( !map_set(map, key, state.value) )
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

static void exe_value(Node *node, VarTable *vt)
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

static void exe_access(Node *node, VarTable *vt)
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

static void exe_extract_e(Node *node, VarTable *vt)
{
    if ( !safe_exe(HEAD_NODE(node), vt) )
        return;
    Nst_Obj *container = state.value;
    inc_ref(container);
    if ( !safe_exe(TAIL_NODE(node), vt) )
    {
        dec_ref(container);
        return;
    }
    Nst_Obj *key = state.value;
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
        hash_obj(key);
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
            SET_TYPE_ERROR_INT,
            HEAD_NODE(node)->start,
            HEAD_NODE(node)->end,
            format_type_error(EXPECTED_TYPE("Array', 'Vector', 'Map' or 'Str"), container->type_name)
        );
    }

    dec_ref(key);
    dec_ref(container);
}

static void exe_assign_e(Node *node, VarTable *vt)
{
    Node *value_node = HEAD_NODE(node);
    Node *name_node = TAIL_NODE(node);

    if ( !safe_exe(value_node, vt) )
        return;
    Nst_Obj *value = state.value;

    if ( name_node->type == ACCESS )
    {
        Nst_Obj *name = HEAD_TOK(name_node)->value;

        set_val(vt, name, value);
        return; // state.value is still the same
    }

    inc_ref(value);

    // If name_node->type == EXTRACTION
    // can't be anything else because of the parser
    if ( !safe_exe(HEAD_NODE(name_node), vt) )
    {
        dec_ref(value);
        return;
    }
    Nst_Obj *obj_to_set = state.value;
    inc_ref(obj_to_set);

    if ( !safe_exe(TAIL_NODE(name_node), vt) )
    {
        dec_ref(value);
        dec_ref(obj_to_set);
        return;
    }
    Nst_Obj *idx = state.value;
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

static void exe_continue_s(Node *node, VarTable *vt)
{
    state.must_continue = true;
    SET_NULL;
}

static void exe_break_s(Node *node, VarTable *vt)
{
    state.must_break = true;
    SET_NULL;
}

static void call_func(Node *node, Nst_func *func, VarTable *vt, Nst_Obj **args)
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

        exe_node(func->body, func_vt);
        if ( state.error_occurred )
        {
            LList_append(state.traceback->positions, &node->start, false);
            LList_append(state.traceback->positions, &node->end, false);
            return;
        }

        state.must_return = false;

        if ( func->body->type != LONG_S && func->body->type != RETURN_S )
            SET_NULL;

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
            SET_ERROR(SET_GENERAL_ERROR_INT, node->start, node->end, err.message);
            state.traceback->error->name = err.name;
            return;
        }
        SET_VALUE(res);
    }
}

static Nst_Obj *import_lib(Nst_Obj *ob, OpErr *err)
{

    if ( ob->type != nst_t_str )
    {
        err->name = "Type Error";
        err->message = format_type_error(EXPECTED_TYPE("Str"), ob->type_name);
        return NULL;
    }

    char *file_name = AS_STR(ob)->value;
    bool c_import = false;

    if ( AS_STR(ob)->len > 6 &&
         file_name[0] == '_' && file_name[1] == '_' &&
         file_name[2] == 'C' && file_name[3] == '_' &&
         file_name[4] == '_' && file_name[5] == ':' )
    {
        c_import = true;
        file_name += 6; // length of __C__:
    }

    size_t file_name_len = strlen(file_name);
    char *file_path = NULL;
    bool file_path_allocated = false;

    if ( AS_STR(ob)->len > 2 && file_name[1] == ':' )
        file_path = file_name;
    else
    {
        file_path = malloc(sizeof(char) * (state.curr_path->len + file_name_len + 1));
        file_path_allocated = true;
        if ( file_path == NULL )
            return NULL;

        strcpy(file_path, state.curr_path->value);
        strcat(file_path, file_name);
    }

    Nst_iofile *file;
    if ( (file = fopen(file_path, "r")) == NULL )
    {
        char *appdata = getenv("LOCALAPPDATA");
        char *original_path = format_fnf_error(FILE_NOT_FOUND, file_path);
        if ( appdata == NULL || !file_path_allocated )
        {
            err->name = "Value Error";
            err->message = original_path;
            return NULL;
        }

        size_t appdata_len = strlen(appdata);
        char *realloc_file_path = realloc(file_path, appdata_len + file_name_len + 26);
        if ( !realloc_file_path ) return NULL;

        char *lib_dir = "\\Programs\\nest\\nest_libs\\";
        file_path = realloc_file_path;
        memcpy(file_path, appdata, appdata_len);
        memcpy(file_path + appdata_len, lib_dir, 25);
        memcpy(file_path + appdata_len + 25, file_name, file_name_len + 1); // copies also \0

        if ( (file = fopen(file_path, "r")) == NULL )
        {
            err->name = "Value Error";
            err->message = original_path;
            free(file_path);
            return NULL;
        }

        free(original_path);
    }
    fclose(file);

    char *full_path = NULL;
    get_full_path(file_path, &full_path, NULL);
    free(file_path);
    file_path = full_path;

    for ( LLNode *n = state.lib_paths->head; n != NULL; n = n->next )
    {
        if ( strcmp(file_path, (const char *)(n->value)) == 0 )
        {
            err->name = "Import Error";
            err->message = "circular import";
            return NULL;
        }
    }

    if ( !c_import )
    {
        Nst_map *map = run_module(file_path);

        if ( map == NULL )
        {
            state.error_occurred = true;
            return NULL;
        }

        return make_obj(map, nst_t_map, destroy_map);
    }

    bool lib_found = false;
    HMODULE lib = NULL;

    for ( LLNode *n = state.lib_handles->head; n != NULL; n = n->next )
    {
        LibHandle *handle = (LibHandle *)(n->value);
        if ( strcmp(handle->path, file_path) == 0 )
        {
            lib_found = true;
            lib = (handle->val)->dll;
            break;
        }
    }

    if ( !lib_found )
    {
        lib = LoadLibraryA(file_path);

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

    if ( !lib_found )
    {
        LList_append(state.loaded_libs, lib, false);

        LibHandleVal *val = malloc(sizeof(LibHandleVal));
        LibHandle *handle = malloc(sizeof(LibHandle));
        if ( handle == NULL || val == NULL )
        {
            errno = ENOMEM;
            return NULL;
        }

        val->dll = lib;
        handle->val = val;
        handle->path = file_path;

        LList_append(state.lib_handles, handle, true);
    }
    return make_obj(func_map, nst_t_map, destroy_map);
}
