#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "interpreter.h"
#include "error.h"
#include "obj.h"
#include "nst_types.h"
#include "var_table.h"
#include "tokens.h"
#include "obj_ops.h"

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
    Nst_Obj *_val_nst_obj = new_value; \
    if ( state->value != NULL ) \
        dec_ref(state->value); \
    state->value = _val_nst_obj; \
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
}
ExecutionState;

static void exe_node(Node *node, VarTable *vt, ExecutionState *state);
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

void run(Node *node)
{
    if ( node == NULL )
        return;

    init_obj();
    Nst_Traceback tb = { NULL, LList_new() };
    ExecutionState state = {
        &tb,
        nst_null,
        false,
        false,
        false,
        false
    };

    inc_ref(nst_null);

    VarTable *vt = new_var_table(NULL);

    exe_node(node, vt, &state);

    if ( state.error_occurred )
        print_traceback(tb);
}

void exe_node(Node *node, VarTable *vt, ExecutionState *state)
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

        if ( state->must_break     ||
             state->must_continue  ||
             state->must_return       )
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

    SET_NULL;
}

static void exe_dowhile_l(Node *node, VarTable *vt, ExecutionState *state)
{
    assert(false);
}

static void exe_for_l(Node *node, VarTable *vt, ExecutionState *state)
{
    assert(false);
}

static void exe_for_as_l(Node *node, VarTable *vt, ExecutionState *state)
{
    assert(false);
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
    assert(false);
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
    }

    dec_ref(ob1);
    dec_ref(ob2);

    SET_VALUE(res);
}

static void exe_local_stack_op(Node *node, VarTable *vt, ExecutionState *state)
{
    assert(false);
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
    case LEN:    res = obj_len(ob, &err);    break;
    case L_NOT:  res = obj_lgnot(ob, &err);  break;
    case B_NOT:  res = obj_bwnot(ob, &err);  break;
    case OUT:    res = obj_stdout(ob, &err); break;
    case IN:     res = obj_stdin(ob, &err);  break;
    case TYPEOF: res = obj_typeof(ob, &err); break;
    default: return;
    }

    if ( res == NULL )
    {
        if ( errno == ENOMEM )
            return;
        SET_ERROR(GENERAL_ERROR, node->start, node->end, err.message);
        state->traceback->error->name = err.name;
    }

    SET_VALUE(res);
}

static void exe_arr_lit(Node *node, VarTable *vt, ExecutionState *state)
{
    assert(false);
}

static void exe_vect_lit(Node *node, VarTable *vt, ExecutionState *state)
{
    assert(false);
}

static void exe_map_lit(Node *node, VarTable *vt, ExecutionState *state)
{
    assert(false);
}

static void exe_value(Node *node, VarTable *vt, ExecutionState *state)
{
    Token *tok = LList_peek_front(node->tokens);

    switch ( tok->type )
    {
    case INT:
        SET_VALUE(make_obj(
            AS_INT(tok->value),
            nst_t_int, NULL // these should never be freed
        ));
        break;
    case REAL:
        SET_VALUE(make_obj(
            AS_REAL(tok->value),
            nst_t_real, NULL
        ));
        break;
    case STRING:
        SET_VALUE(make_obj(
            AS_STR(tok->value),
            nst_t_str, NULL
        ));
        break;
    }
}

static void exe_access(Node *node, VarTable *vt, ExecutionState *state)
{
    Nst_Obj *key = make_obj(
        TOK(LList_peek_front(node->tokens))->value,
        nst_t_str, NULL
    );

    SET_VALUE(get_val(vt, key));

    if ( state->value == NULL )
        state->value = nst_t_null;

    destroy_obj(key);
}

static void exe_extract_e(Node *node, VarTable *vt, ExecutionState *state)
{
    assert(false);
}

static void exe_assign_e(Node *node, VarTable *vt, ExecutionState *state)
{
    Node *value_node = HEAD_NODE(node);
    Node *name_node = TAIL_NODE(node);

    SAFE_EXE(value_node);
    Nst_Obj *value = state->value;

    if ( name_node->type == ACCESS )
    {
        Nst_Obj *name = make_obj(
            copy_string(HEAD_TOK(name_node)->value),
            nst_t_str,
            destroy_string
        );

        set_val(vt, name, value);
        dec_ref(name);
        return; // state->value is still the same
    }
    
    // If name_node->type == EXTRACTION
    // can't be anything else because of the parser
    SAFE_EXE(HEAD_NODE(name_node));
    Nst_Obj *obj_to_set = state->value;

    SAFE_EXE(TAIL_NODE(name_node));
    Nst_Obj *idx = state->value;
    SET_VALUE(value);

    if ( obj_to_set->type == nst_t_map )
    {
        bool res = map_set(AS_MAP(obj_to_set->value), idx, value);
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
                format_type_error(EXPECTED_TYPE("int"), idx->type_name)
            );
            return;
        }
        
        bool res = set_value_seq(AS_SEQ(obj_to_set->value), *AS_INT(idx->value), value);

        if ( !res )
        {
            SET_ERROR(
                VALUE_ERROR,
                name_node->start,
                name_node->end,
                format_idx_error(
                    INDEX_OUT_OF_BOUNDS,
                    *AS_INT(idx->value),
                    AS_SEQ(obj_to_set->value)->len
                )
            );
            return;
        }

        dec_ref(obj_to_set);
        return;
    }
    else
    {
        SET_ERROR(
            TYPE_ERROR,
            HEAD_NODE(name_node)->start,
            HEAD_NODE(name_node)->end,
            format_type_error(EXPECTED_TYPE("map', 'arr' or 'vect"), obj_to_set->type_name)
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
