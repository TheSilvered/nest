#include <errno.h>
#include "optimizer.h"
#include "obj_ops.h"
#include "tokens.h"
#include "error_internal.h"
#include "iter.h"

#define HEAD_NODE NODE(node->nodes->head->value)
#define TAIL_NODE NODE(node->nodes->tail->value)

#define HEAD_TOK TOK(node->tokens->head->value)
#define TAIL_TOK TOK(node->tokens->tail->value)

#define SET_ERROR(err_macro, start, end, message) \
    do { \
        Nst_Error *error = malloc(sizeof(Nst_Error)); \
        if ( error == NULL ) \
        { \
            errno = ENOMEM; \
            return; \
        } \
        err_macro(error, start, end, message); \
        o_state.error = error; \
    } while ( 0 )

typedef struct
{
    Nst_Error *error;
}
OptimizerState;

static OptimizerState o_state;

static void ast_optimize_node(Nst_Node *node);
static void ast_optimize_node_nodes(Nst_Node *node);
static void ast_optimize_stack_op(Nst_Node *node);
static void ast_optimize_local_op(Nst_Node *node);
static void ast_optimize_long_s(Nst_Node *node);

inline void destroy_node_if_allocated(LList *nodes)
{
    if ( nodes->head->allocated )
        nst_destroy_node(LList_pop(nodes));
    else
        LList_pop(nodes);
}

inline void destroy_tok_if_allocated(LList *tokens)
{
    if ( tokens->head->allocated )
        nst_destroy_token(LList_pop(tokens));
    else
        LList_pop(tokens);
}

Nst_Node *nst_optimize_ast(Nst_Node *ast)
{
    o_state.error = NULL;
    ast_optimize_node(ast);

    if ( o_state.error != NULL )
    {
        nst_print_error(*o_state.error);
        nst_destroy_node(ast);
        return NULL;
    }

    return ast;
}

static void ast_optimize_node(Nst_Node *node)
{
    switch ( node->type )
    {
    case NST_NT_STACK_OP:
        ast_optimize_stack_op(node);
        break;
    case NST_NT_LOCAL_OP:
        ast_optimize_local_op(node);
        break;
    case NST_NT_LONG_S:
        ast_optimize_long_s(node);
    case NST_NT_VALUE:
        break;
    default:
        ast_optimize_node_nodes(node);
    }
}

static void ast_optimize_node_nodes(Nst_Node *node)
{
    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        ast_optimize_node(n->value);
        if ( o_state.error != NULL )
            return;
    }
}

static void ast_optimize_stack_op(Nst_Node *node)
{
    ast_optimize_node(HEAD_NODE);
    ast_optimize_node(TAIL_NODE);

    if ( o_state.error != NULL )
        return;

    if ( HEAD_NODE->type != NST_NT_VALUE || TAIL_NODE->type != NST_NT_VALUE )
        return;

    int op_tok = HEAD_TOK->type;
    Nst_OpErr err = { "", "" };
    Nst_Obj *ob1 = TOK(HEAD_NODE->tokens->head->value)->value;
    Nst_Obj *ob2 = TOK(TAIL_NODE->tokens->head->value)->value;
    Nst_Obj *res = NULL;

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
        o_state.error->name = err.name;
        return;
    }

    destroy_node_if_allocated(node->nodes);
    destroy_node_if_allocated(node->nodes);
    destroy_tok_if_allocated(node->tokens);

    node->type = NST_NT_VALUE;
    int new_tok_type;

    if ( res->type == nst_t_int )
        new_tok_type = NST_TT_INT;
    else if ( res->type == nst_t_real )
        new_tok_type = NST_TT_REAL;
    else
        new_tok_type = NST_TT_STRING;

    Nst_LexerToken *new_tok = nst_new_token_value(
        node->start,
        node->end,
        new_tok_type,
        res
    );

    LList_append(node->tokens, new_tok, true);
}

static void ast_optimize_local_op(Nst_Node *node)
{
    ast_optimize_node(HEAD_NODE);

    if ( o_state.error != NULL )
        return;

    if ( HEAD_NODE->type != NST_NT_VALUE )
        return;

    int op_tok = HEAD_TOK->type;
    Nst_OpErr err = { "", "" };
    Nst_Obj *ob = TOK(HEAD_NODE->tokens->head->value)->value;
    Nst_Obj *res = NULL;

    switch ( op_tok )
    {
    case NST_TT_LEN:    res = nst_obj_len(ob, &err);    break;
    case NST_TT_L_NOT:  res = nst_obj_lgnot(ob, &err);  break;
    case NST_TT_B_NOT:  res = nst_obj_bwnot(ob, &err);  break;
    case NST_TT_TYPEOF: res = nst_obj_typeof(ob, &err); break;
    case NST_TT_NEG:    res = nst_obj_neg(ob, &err);    break;
    default: return;
    }

    if ( res == NULL )
    {
        if ( errno == ENOMEM )
            return;
        SET_ERROR(_NST_SET_GENERAL_ERROR, node->start, node->end, err.message);
        o_state.error->name = err.name;
        return;
    }

    destroy_node_if_allocated(node->nodes);
    destroy_tok_if_allocated(node->tokens);

    node->type = NST_NT_VALUE;
    int new_tok_type;

    if ( res->type == nst_t_int )
        new_tok_type = NST_TT_INT;
    else if ( res->type == nst_t_real )
        new_tok_type = NST_TT_REAL;
    else
        new_tok_type = NST_TT_STRING;

    LList_append(
        node->tokens,
        nst_new_token_value(
            node->start,
            node->end,
            new_tok_type,
            res
        ),
        true
    );
}

static void ast_optimize_long_s(Nst_Node *node)
{
    LList *nodes = node->nodes;
    LLNode *prev_valid_node = NULL;
    Nst_Node *curr_node;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        ast_optimize_node(n->value);
        if ( o_state.error != NULL )
            return;

        curr_node = n->value;

        if ( curr_node->type != NST_NT_VALUE && curr_node->type != NST_NT_ACCESS )
        {
            prev_valid_node = n;
            continue;
        }

        if ( prev_valid_node == NULL )
        {
            nodes->head = n->next;
            if ( n->next == NULL )
                nodes->tail = NULL;
        }
        else
        {
            prev_valid_node->next = n->next;
            if ( n->next == NULL )
                nodes->tail = prev_valid_node;
        }

        nst_destroy_node(n->value);
        free(n);

        if ( prev_valid_node == NULL )
            n = nodes->head;
        else
            n = prev_valid_node;

        if ( n == NULL )
            break;
    }
}
