#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "llist.h"
#include "parser.h"
#include "error_internal.h"
#include "tokens.h"

#define SAFE_LLIST_CREATE(name) \
    LList *name = LList_new(); \
    if ( name == NULL ) \
    { \
        errno = ENOMEM; \
        return NULL; \
    } \

#define RETURN_ERROR(start, end, message) \
    Nst_Error *error = malloc(sizeof(Nst_Error)); \
    if ( error == NULL ) \
    { \
        errno = ENOMEM; \
        return NULL; \
    } \
    _NST_SET_SYNTAX_ERROR(error, start, end, message); \
    p_state.error = error; \
    return NULL

typedef struct ParsingState
{
    bool in_func;
    bool in_loop;
    bool in_switch;
    Nst_Error *error;
}
ParsingState;

static ParsingState p_state;
static LList *tokens;

static inline void skip_blank();
static Nst_Node *fix_expr(Nst_Node *expr);

static Nst_Node *parse_long_statement();
static Nst_Node *parse_statement();
static Nst_Node *parse_while_loop();
static Nst_Node *parse_for_loop();
static Nst_Node *parse_if_expr(Nst_Node *condition);
static Nst_Node *parse_switch_statement();
static Nst_Node *parse_func_def();
static Nst_Node *parse_expr(bool break_as_end);
static Nst_Node *parse_stack_op(Nst_Node *value);
static Nst_Node *parse_local_stack_op(LList *nodes);
static Nst_Node *parse_assignment(Nst_Node *value);
static Nst_Node *parse_extraction();
static Nst_Node *parse_atom();
static Nst_Node *parse_vector_literal();
static Nst_Node *parse_arr_or_map_literal();

Nst_Node *nst_parse(LList *tokens_list)
{
    if ( tokens_list == NULL )
        return NULL;

    tokens = tokens_list;

    p_state.in_func = false;
    p_state.in_loop = false;
    p_state.error = NULL;

    Nst_Node *node = parse_long_statement();
    if ( p_state.error != NULL )
    {
        nst_print_error(*(p_state.error));
    }
    // i.e. the only token left is EOFILE
    else if ( tokens->size > 1 )
    {
        Nst_Pos start = TOK(LList_peek_front(tokens))->start;
        Nst_Pos end = TOK(LList_peek_front(tokens))->start;

        Nst_Error *error = malloc(sizeof(Nst_Error));
        if ( error == NULL )
        {
            errno = ENOMEM;
            return NULL;
        }

        _NST_SET_SYNTAX_ERROR(error, start, end, UNEXPECTED_TOK);
        nst_print_error(*error);
        printf("\n");
        LList_destroy(tokens, nst_destroy_token);
        return NULL;
    }

    LList_destroy(tokens, nst_destroy_token);
    return node; // NULL if there was an error
}

static inline void skip_blank()
{
    while ( TOK(LList_peek_front(tokens))->type == NST_TT_ENDL )
        nst_destroy_token(LList_pop(tokens));
}

static Nst_Node *parse_long_statement()
{
    SAFE_LLIST_CREATE(nodes);
    Nst_Node *node = NULL;
    skip_blank();

    while ( TOK(LList_peek_front(tokens))->type != NST_TT_R_BRACKET &&
            TOK(LList_peek_front(tokens))->type != NST_TT_EOFILE       )
    {
        node = parse_statement();
        if ( p_state.error != NULL )
        {
            return NULL;
        }
        else if ( node != NULL )
            LList_append(nodes, node, true);
        if ( errno == ENOMEM )
        {
            return NULL;
        }
        skip_blank();
    }

    if ( nodes->size == 0 )
    {
        LList_destroy(nodes, NULL);
        return nst_new_node_empty(
            TOK(LList_peek_front(tokens))->start,
            TOK(LList_peek_front(tokens))->end,
            NST_NT_LONG_S
        );
    }

    return nst_new_node_nodes(
        NODE(nodes->head->value)->start,
        NODE(nodes->tail->value)->end,
        NST_NT_LONG_S,
        nodes
    );
}

static Nst_Node *parse_statement()
{
    skip_blank();
    int tok_type = TOK(LList_peek_front(tokens))->type;

    if ( tok_type == NST_TT_L_BRACKET )
    {
        Nst_LexerToken *open_bracket = LList_pop(tokens);
        Nst_Pos start = open_bracket->start;
        nst_destroy_token(open_bracket);

        Nst_Node *node = parse_long_statement();
        if ( p_state.error != NULL ) return NULL;

        Nst_LexerToken *close_bracket = LList_pop(tokens);
        if ( close_bracket->type != NST_TT_R_BRACKET )
        {
            RETURN_ERROR(start, start, MISSING_BRACKET);
        }
        return node;
    }
    else if ( tok_type == NST_TT_WHILE || tok_type == NST_TT_DOWHILE )
        return parse_while_loop();
    else if ( tok_type == NST_TT_FOR )
        return parse_for_loop();
    else if ( tok_type == NST_TT_FUNC )
        return parse_func_def();
    else if ( tok_type == NST_TT_SWITCH )
        return parse_switch_statement();
    else if ( tok_type == NST_TT_RETURN )
    {
        Nst_LexerToken *tok = LList_pop(tokens);
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        nst_destroy_token(tok);

        if ( !p_state.in_func )
        {
            RETURN_ERROR(start, end, BAD_RETURN);
        }

        Nst_Node *expr = parse_expr(false);
        if ( p_state.error != NULL ) return NULL;

        SAFE_LLIST_CREATE(nodes);
        LList_append(nodes, expr, true);
        return nst_new_node_nodes(start, expr->end, NST_NT_RETURN_S, nodes);
    }
    else if ( tok_type == NST_TT_CONTINUE )
    {
        Nst_LexerToken *tok = LList_pop(tokens);
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        nst_destroy_token(tok);

        if ( !p_state.in_loop && !p_state.in_switch )
        {
            RETURN_ERROR(start, end, BAD_CONTINUE);
        }

        return nst_new_node_empty(start, end, NST_NT_CONTINUE_S);
    }
    else if ( tok_type == NST_TT_BREAK )
    {
        Nst_LexerToken *tok = LList_pop(tokens);
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        nst_destroy_token(tok);

        if ( !p_state.in_loop )
        {
            RETURN_ERROR(start, end, BAD_BREAK);
        }

        return nst_new_node_empty(start, end, NST_NT_BREAK_S);
    }
    else if ( T_IN_ATOM(tok_type) || T_IN_LOCAL_STACK_OP(tok_type) )
        return parse_expr(false);
    else
    {
        Nst_LexerToken *tok = LList_pop(tokens);
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;

        nst_destroy_token(tok);
        RETURN_ERROR(start, end, UNEXPECTED_TOK);
    }
}

static Nst_Node *parse_while_loop()
{
    Nst_LexerToken *tok = LList_pop(tokens);
    Nst_Pos start = tok->start;

    int node_type = tok->type == NST_TT_WHILE ? NST_NT_WHILE_L : NST_NT_DOWHILE_L;

    nst_destroy_token(tok);

    Nst_Node *condition = parse_expr(false);
    if ( p_state.error != NULL ) return NULL;

    Nst_Pos err_pos = TOK(LList_peek_front(tokens))->start;

    if ( TOK(LList_peek_front(tokens))->type != NST_TT_L_BRACKET )
    {
        nst_destroy_node(condition);
        RETURN_ERROR(err_pos, err_pos, EXPECTED_BRACKET);
    }

    nst_destroy_token(LList_pop(tokens));

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;

    Nst_Node *body = parse_long_statement();
    if ( p_state.error != NULL )
    {
        nst_destroy_node(condition);
        return NULL;
    }

    p_state.in_loop = prev_state;

    if ( TOK(LList_peek_front(tokens))->type != NST_TT_R_BRACKET )
    {
        nst_destroy_node(condition);
        nst_destroy_node(body);
        RETURN_ERROR(err_pos, err_pos, MISSING_BRACKET);
    }

    tok = LList_pop(tokens);
    Nst_Pos end = tok->end;
    nst_destroy_token(tok);

    SAFE_LLIST_CREATE(nodes);

    LList_append(nodes, condition, true);
    LList_append(nodes, body, true);

    return nst_new_node_nodes(start, end, node_type, nodes);
}

static Nst_Node *parse_for_loop()
{
    Nst_LexerToken *tok = LList_pop(tokens);
    Nst_Pos start = tok->start;
    nst_destroy_token(tok);

    int node_type = NST_NT_FOR_L;

    Nst_Node *range = parse_expr(false);

    if ( p_state.error != NULL ) return NULL;

    SAFE_LLIST_CREATE(node_tokens);

    if ( TOK(LList_peek_front(tokens))->type == NST_TT_AS )
    {
        nst_destroy_token(LList_pop(tokens));
        if ( TOK(LList_peek_front(tokens))->type != NST_TT_IDENT )
        {
            Nst_Pos err_start = TOK(LList_peek_front(tokens))->start;
            Nst_Pos err_end = TOK(LList_peek_front(tokens))->end;
            RETURN_ERROR(err_start, err_end, EXPECTED_IDENT);
        }
        LList_append(node_tokens, LList_pop(tokens), true);
        node_type = NST_NT_FOR_AS_L;
    }

    Nst_Pos err_pos = TOK(LList_peek_front(tokens))->start;

    if ( TOK(LList_peek_front(tokens))->type != NST_TT_L_BRACKET )
    {
        nst_destroy_node(range);
        RETURN_ERROR(err_pos, err_pos, EXPECTED_BRACKET);
    }

    nst_destroy_token(LList_pop(tokens));

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;
    Nst_Node *body = parse_long_statement();
    if ( p_state.error != NULL )
    {
        nst_destroy_node(range);
        LList_destroy(node_tokens, nst_destroy_token);
        return NULL;
    }
    p_state.in_loop = prev_state;

    if ( TOK(LList_peek_front(tokens))->type != NST_TT_R_BRACKET )
    {
        nst_destroy_node(range);
        nst_destroy_node(body);
        RETURN_ERROR(err_pos, err_pos, UNEXPECTED_TOK);
    }

    tok = LList_pop(tokens);
    Nst_Pos end = tok->end;
    nst_destroy_token(tok);

    SAFE_LLIST_CREATE(nodes);

    LList_append(nodes, range, true);
    LList_append(nodes, body, true);

    return nst_new_node_full(start, end, node_type, nodes, node_tokens);
}

static Nst_Node *parse_if_expr(Nst_Node *condition)
{
    nst_destroy_token(LList_pop(tokens));
    skip_blank();

    SAFE_LLIST_CREATE(nodes);
    LList_append(nodes, condition, true);

    Nst_Node *body_if_true = parse_statement();
    if ( p_state.error != NULL )
    {
        LList_destroy(nodes, nst_destroy_node);
        return NULL;
    }
    LList_append(nodes, body_if_true, true);

    skip_blank();

    if ( TOK(LList_peek_front(tokens))->type != NST_TT_COLON )
        return nst_new_node_nodes(
            condition->start,
            body_if_true->end,
            NST_NT_IF_E,
            nodes
        );

    nst_destroy_token(LList_pop(tokens));
    skip_blank();

    Nst_Node *body_if_false = parse_statement();
    if ( p_state.error != NULL )
    {
        LList_destroy(nodes, nst_destroy_node);
        return NULL;
    }
    LList_append(nodes, body_if_false, true);

    return nst_new_node_nodes(
        condition->start,
        body_if_false->end,
        NST_NT_IF_E,
        nodes
    );
}

static Nst_Node *parse_switch_statement()
{
    Nst_LexerToken *tok = LList_pop(tokens);
    Nst_Pos start = tok->start;
    Nst_Pos err_start;
    Nst_Pos err_end;
    nst_destroy_token(tok);
    skip_blank();

    SAFE_LLIST_CREATE(nodes);

    Nst_Node *main_val = parse_expr(false);

    if ( p_state.error != NULL )
    {
        LList_destroy(nodes, NULL);
        return NULL;
    }
    LList_append(nodes, main_val, true);

    skip_blank();
    tok = LList_pop(tokens);

    if ( tok->type != NST_TT_L_BRACKET )
    {
        err_start = tok->start;
        err_end = tok->end;
        nst_destroy_token(tok);
        RETURN_ERROR(err_start, err_end, EXPECTED_BRACKET);
    }
    nst_destroy_token(tok);

    bool is_default_case = false;
    bool prev_in_switch = p_state.in_switch;
    p_state.in_switch = true;

    while ( true )
    {
        skip_blank();
        tok = LList_pop(tokens);
        if ( tok->type != NST_TT_IF )
        {
            err_start = tok->start;
            err_end = tok->end;
            nst_destroy_token(tok);
            RETURN_ERROR(err_start, err_end, EXPECTED_QUESTION_MARK);
        }
        nst_destroy_token(tok);
        skip_blank();
        tok = LList_peek_front(tokens);

        if ( tok->type == NST_TT_L_BRACKET )
        {
            is_default_case = true;
            err_start = tok->start;
            err_end = tok->end;
            nst_destroy_token(LList_pop(tokens));
        }
        else
        {
            Nst_Node *val = parse_expr(false);

            if ( p_state.error != NULL )
            {
                LList_destroy(nodes, nst_destroy_node);
                return NULL;
            }

            LList_append(nodes, val, true);
            skip_blank();
            tok = LList_pop(tokens);
            err_start = tok->start;
            err_end = tok->end;

            if ( tok->type != NST_TT_L_BRACKET )
            {
                nst_destroy_token(tok);
                LList_destroy(nodes, nst_destroy_node);
                RETURN_ERROR(err_start, err_end, EXPECTED_BRACKET);
            }
            nst_destroy_token(tok);
        }

        Nst_Node *body = parse_long_statement();

        if ( p_state.error != NULL )
        {
            LList_destroy(nodes, nst_destroy_node);
            return NULL;
        }

        LList_append(nodes, body, true);
        skip_blank();
        tok = LList_pop(tokens);
        if ( tok->type != NST_TT_R_BRACKET )
        {
            err_start = tok->start;
            err_end = tok->end;
            nst_destroy_token(tok);
            LList_destroy(nodes, nst_destroy_node);
            RETURN_ERROR(err_start, err_end, EXPECTED_R_BRACKET);
        }
        nst_destroy_token(tok);
        skip_blank();
        tok = LList_peek_front(tokens);

        if ( tok->type == NST_TT_R_BRACKET )
        {
            Nst_Pos end = tok->end;
            nst_destroy_token(LList_pop(tokens));
            p_state.in_switch = prev_in_switch;
            return nst_new_node_nodes(start, end, NST_NT_SWITCH_S, nodes);
        }
        else if ( is_default_case )
        {
            err_start = tok->start;
            err_end = tok->end;
            nst_destroy_token(LList_pop(tokens));
            LList_destroy(nodes, nst_destroy_node);
            RETURN_ERROR(err_start, err_end, EXPECTED_R_BRACKET);
        }
    }
}

static Nst_Node *parse_func_def()
{
    Nst_LexerToken *tok = LList_pop(tokens);
    Nst_Pos start = tok->start;
    nst_destroy_token(tok);

    SAFE_LLIST_CREATE(node_tokens);

    while ( TOK(LList_peek_front(tokens))->type == NST_TT_IDENT )
        LList_append(node_tokens, LList_pop(tokens), true);

    skip_blank();

    Nst_Pos err_start = TOK(LList_peek_front(tokens))->start;
    Nst_Pos err_end = TOK(LList_peek_front(tokens))->end;

    if ( TOK(LList_peek_front(tokens))->type != NST_TT_L_BRACKET )
    {
        LList_destroy(node_tokens, nst_destroy_token);
        RETURN_ERROR(err_start, err_end, EXPECTED_BRACKET);
    }

    nst_destroy_token(LList_pop(tokens));

    bool prev_state = p_state.in_func;
    p_state.in_func = true;

    Nst_Node *body = parse_long_statement();
    if ( p_state.error != NULL )
    {
        LList_destroy(node_tokens, nst_destroy_token);
        return NULL;
    }

    p_state.in_func = prev_state;

    SAFE_LLIST_CREATE(nodes);
    LList_append(nodes, body, true);

    if ( TOK(LList_peek_front(tokens))->type != NST_TT_R_BRACKET )
    {
        LList_destroy(node_tokens, nst_destroy_token);
        RETURN_ERROR(err_start, err_end, MISSING_BRACKET);
    }

    tok = LList_pop(tokens);
    Nst_Pos end = tok->end;
    nst_destroy_token(tok);

    return nst_new_node_full(start, end, NST_NT_FUNC_DECLR, nodes, node_tokens);
}

static Nst_Node *parse_expr(bool break_as_end)
{
    Nst_Node *node = NULL;
    int token_type = TOK(LList_peek_front(tokens))->type;

    if ( break_as_end )
        while ( !T_IN_EXPR_END_W_BREAK(token_type) )
        {
            node = parse_stack_op(node);
            if ( p_state.error != NULL || errno == ENOMEM )
                return NULL;
            token_type = TOK(LList_peek_front(tokens))->type;
        }
    else
        while ( !T_IN_EXPR_END(token_type) )
        {
            node = parse_stack_op(node);
            if ( p_state.error != NULL || errno == ENOMEM )
                return NULL;
            token_type = TOK(LList_peek_front(tokens))->type;
        }

    if ( node == NULL )
    {
        Nst_LexerToken *tok = TOK(LList_peek_front(tokens));
        RETURN_ERROR(tok->start, tok->end, EXPECTED_VALUE);
    }

    node = fix_expr(node);
    if ( errno == ENOMEM )
    {
        nst_destroy_node(node);
        return NULL;
    }

    int tok_type = TOK(LList_peek_front(tokens))->type;

    if ( tok_type == NST_TT_IF )
        // the error propagates automatically
        return parse_if_expr(node);
    return node;
}

static Nst_Node *fix_expr(Nst_Node *expr)
{
    if ( expr->type != NST_NT_STACK_OP &&
         expr->type != NST_NT_LOCAL_STACK_OP &&
         expr->type != NST_NT_ASSIGN_E )
        return expr;

    else if ( expr->type == NST_NT_STACK_OP && expr->nodes->size == 1 )
    {
        Nst_Node *new_node = LList_peek_front(expr->nodes);
        LList_destroy(expr->nodes, NULL);
        LList_destroy(expr->tokens, nst_destroy_token);
        free(expr);
        return new_node;
    }

    LLNode *cursor = expr->nodes->head;

    for ( LLNode *cursor = expr->nodes->head;
          cursor != NULL;
          cursor = cursor->next )
        cursor->value = fix_expr(cursor->value);

    if ( expr->type != NST_NT_STACK_OP )
    {
        return expr;
    }

    Nst_Node *curr_node = expr;
    Nst_LexerToken *op_tok = LList_peek_front(expr->tokens);

    // writing 1 2 3 < becomes (1 2 <) (2 3 <) &&
    if ( T_IN_COMP_OP(op_tok->type) && expr->nodes->size > 2)
    {
        SAFE_LLIST_CREATE(pairs);
        for ( size_t i = 0, n = expr->nodes->size - 1; i < n; i++ )
        {
            SAFE_LLIST_CREATE(new_nodes);
            SAFE_LLIST_CREATE(new_tokens);

            LList_append(new_nodes, LList_pop(expr->nodes), true);
            if ( i == n - 1 )
                LList_append(new_nodes, LList_pop(expr->nodes), true);
            else
                LList_append(new_nodes, LList_peek_front(expr->nodes), false);

            LList_append(new_tokens, op_tok, i == n - 1);

            Nst_Pos start = NODE(LList_peek_front(new_nodes))->start;
            Nst_Pos end = NODE(LList_peek_back(new_nodes))->end;

            LList_append(pairs,
                nst_new_node_full(
                    start,
                    end,
                    NST_NT_STACK_OP,
                    new_nodes, new_tokens
                ),
                true
            );
        }
        LList_destroy(expr->nodes, NULL);
        expr->nodes = pairs;
        op_tok = nst_new_token_noval(op_tok->start, op_tok->end, NST_TT_L_AND);
        LList_pop(expr->tokens);
        LList_append(expr->tokens, op_tok, true);
    }

    // writing 1 2 3 4 + becomes 1 2 + 3 + 4 +
    for ( size_t i = 0, n = expr->nodes->size - 2; i < n; i++ )
    {
        // get the positions
        Nst_Pos start = curr_node->start;
        Nst_Pos end = curr_node->end;
        // create the new lists
        SAFE_LLIST_CREATE(new_nodes);
        SAFE_LLIST_CREATE(new_tokens);
        // move the nodes except for the last one
        for ( size_t j = 0, m = curr_node->nodes->size - 1; j < m; j++ )
            LList_append(new_nodes, LList_pop(curr_node->nodes), true);

        LList_append(new_tokens, op_tok, false);
        Nst_Node *new_node = nst_new_node_full(
            start,
            end,
            NST_NT_STACK_OP,
            new_nodes,
            new_tokens
        );

        LList_push(curr_node->nodes, new_node, true);
        curr_node = new_node;
    }

    return expr;
}

static Nst_Node *parse_stack_op(Nst_Node *value)
{
    Nst_Pos start = TOK(LList_peek_front(tokens))->start;

    Nst_Node *value_node = NULL;
    SAFE_LLIST_CREATE(new_nodes);
    if ( value != NULL )
        LList_append(new_nodes, value, true);
    else
    {
        value_node = parse_extraction();
        if ( p_state.error != NULL )
        {
            LList_destroy(new_nodes, nst_destroy_node);
            return NULL;
        }
        LList_append(new_nodes, value_node, true);
    }

    while ( T_IN_ATOM(TOK(LList_peek_front(tokens))->type) )
    {
        value_node = parse_extraction();
        if ( p_state.error != NULL )
        {
            LList_destroy(new_nodes, nst_destroy_node);
            return NULL;
        }
        LList_append(new_nodes, value_node, true);
    }

    Nst_Node *node = NULL;
    Nst_LexerToken *op_tok = LList_peek_front(tokens);
    Nst_Pos end = op_tok->end;
    bool is_local_stack_op = false;

    if ( T_IN_STACK_OP(op_tok->type) )
    {
        LList_pop(tokens);
        SAFE_LLIST_CREATE(new_tokens);
        LList_append(new_tokens, op_tok, true);
        node = nst_new_node_full(
            start,
            end,
            NST_NT_STACK_OP,
            new_nodes,
            new_tokens
        );
    }
    else if ( T_IN_LOCAL_STACK_OP(op_tok->type) )
    {
        node = parse_local_stack_op(new_nodes);
        if ( p_state.error != NULL ) return NULL;
        is_local_stack_op = true;
    }
    else if ( new_nodes->size == 1 && value == NULL )
    {
        node = LList_pop(new_nodes);
    }
    else
    {
        LList_destroy(new_nodes, nst_destroy_node);
        RETURN_ERROR(end, end, EXPECTED_OP);
    }

    while ( !T_IN_EXPR_END(TOK(LList_peek_front(tokens))->type) )
    {
        op_tok = LList_peek_front(tokens);
        if ( T_IN_STACK_OP(op_tok->type) && !is_local_stack_op )
        {
            SAFE_LLIST_CREATE(new_node_nodes);
            SAFE_LLIST_CREATE(new_node_tokens);
            LList_append(new_node_nodes, node, true);
            op_tok = LList_pop(tokens);
            LList_append(new_node_tokens, op_tok, true);
            node = nst_new_node_full(
                start,
                op_tok->end,
                NST_NT_STACK_OP,
                new_node_nodes,
                new_node_tokens
            );
        }
        else if ( T_IN_LOCAL_STACK_OP(op_tok->type) && is_local_stack_op )
        {
            SAFE_LLIST_CREATE(new_node_nodes);
            LList_append(new_node_nodes, node, true);
            node = parse_local_stack_op(new_node_nodes);
            if ( p_state.error != NULL ) return NULL;
        }
        else if ( T_IN_ASSIGNMENT(op_tok->type) )
        {
            node = parse_assignment(node);
            if ( p_state.error != NULL ) return NULL;
        }
        else break;
    }

    return node;
}

static Nst_Node *parse_local_stack_op(LList *nodes)
{
    Nst_LexerToken *tok = LList_pop(tokens);

    if ( tok->type == NST_TT_CAST && nodes->size != 1 )
    {
        RETURN_ERROR(
            NODE(LList_peek_front(nodes))->start,
            NODE(LList_peek_back(nodes))->end,
            nodes->size > 2 ? TOO_MANY_ARGS("::") : TOO_FEW_ARGS("::")
        );
    }
    else if ( tok->type == NST_TT_RANGE && (nodes->size != 1 && nodes->size != 2) )
    {
        RETURN_ERROR(
            NODE(LList_peek_front(nodes))->start,
            NODE(LList_peek_back(nodes))->end,
            nodes->size > 2 ? TOO_MANY_ARGS("->") : TOO_FEW_ARGS("->")
        );
    }

    SAFE_LLIST_CREATE(node_tokens);
    LList_append(node_tokens, tok, true);

    Nst_Node *special_node = parse_extraction();
    if ( p_state.error != NULL )
    {
        LList_destroy(nodes, nst_destroy_node);
        LList_destroy(node_tokens, nst_destroy_token);
        return NULL;
    }

    LList_append(nodes, special_node, true);

    Nst_Pos start = NODE(LList_peek_front(nodes))->start;
    Nst_Pos end = special_node->end;

    return nst_new_node_full(
        start,
        end,
        NST_NT_LOCAL_STACK_OP,
        nodes,
        node_tokens
    );
}

static Nst_Node *parse_assignment(Nst_Node *value)
{

    Nst_LexerToken *tok = LList_pop(tokens);
    bool has_shared_node = false;

    Nst_Node *name = parse_extraction();
    if ( p_state.error != NULL )
    {
        nst_destroy_token(tok);
        nst_destroy_node(value);
        return NULL;
    }

    Nst_Pos start = value->start;
    Nst_Pos end = value->end;

    if ( name->type != NST_NT_ACCESS && name->type != NST_NT_EXTRACT_E )
    {
        nst_destroy_token(tok);
        nst_destroy_node(value);
        Nst_Pos err_start = name->start;
        Nst_Pos err_end = name->end;
        nst_destroy_node(name);
        RETURN_ERROR(err_start, err_end, EXPECTED_IDENT_OR_EXTR);
    }

    // If a compound assignmen operator such as '+=' or '*='
    if ( tok->type != NST_TT_ASSIGN )
    {
        SAFE_LLIST_CREATE(new_value_tokens);
        SAFE_LLIST_CREATE(new_value_nodes);

        Nst_LexerToken *op_tok = nst_new_token_noval(
            tok->start,
            tok->end,
            ASSIGMENT_TO_STACK_OP(tok->type)
        );

        LList_append(new_value_tokens, op_tok, true);
        // will be freed later if the list is destroyed
        LList_append(new_value_nodes, name, false);
        LList_append(new_value_nodes, value, true);

        value = nst_new_node_full(value->start, name->end, NST_NT_STACK_OP,
                              new_value_nodes, new_value_tokens);
        has_shared_node = true;
    }
    nst_destroy_token(tok);

    SAFE_LLIST_CREATE(new_nodes);
    LList_append(new_nodes, value, true);
    LList_append(new_nodes, name, true); // here it's freed when the list is destroyed
    return nst_new_node_nodes(start, end, NST_NT_ASSIGN_E, new_nodes);
}

static Nst_Node *parse_extraction()
{
    Nst_Node *atom = parse_atom();
    if ( p_state.error != NULL ) return NULL;

    Nst_Node *final_node = atom;

    while ( TOK(LList_peek_front(tokens))->type == NST_TT_EXTRACT )
    {
        nst_destroy_token(LList_pop(tokens));
        bool treat_as_string = TOK(LList_peek_front(tokens))->type == NST_TT_IDENT;

        atom = parse_atom();
        if ( p_state.error != NULL )
        {
            nst_destroy_node(final_node);
            return NULL;
        }

        // makes `a.i` equivalent to `a.'i'`
        if ( treat_as_string )
        {
            atom->type = NST_NT_VALUE;
            TOK(LList_peek_front(atom->tokens))->type = NST_TT_STRING;
        }

        SAFE_LLIST_CREATE(new_nodes);

        LList_append(new_nodes, final_node, true);
        LList_append(new_nodes, atom, true);

        final_node = nst_new_node_nodes(
            final_node->start,
            atom->end,
            NST_NT_EXTRACT_E,
            new_nodes
        );
    }
    return final_node;
}

static Nst_Node *parse_atom()
{
    Nst_LexerToken *tok = LList_pop(tokens);

    if ( T_IN_VALUE(tok->type) )
    {
        SAFE_LLIST_CREATE(new_tokens);
        LList_append(new_tokens, tok, true);
        return nst_new_node_tokens(
            tok->start,
            tok->end,
            tok->type == NST_TT_IDENT ? NST_NT_ACCESS : NST_NT_VALUE,
            new_tokens
        );
    }
    else if ( tok->type == NST_TT_L_PAREN )
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        nst_destroy_token(tok);
        Nst_Node *expr = parse_expr(false);
        if ( p_state.error != NULL ) return NULL;

        tok = LList_pop(tokens);
        if ( tok->type != NST_TT_R_PAREN )
        {
            nst_destroy_node(expr);
            RETURN_ERROR(err_start, err_end, MISSING_PAREN);
        }
        nst_destroy_token(tok);
        return expr;
    }
    else if ( T_IN_LOCAL_OP(tok->type) )
    {
        SAFE_LLIST_CREATE(new_tokens);
        LList_append(new_tokens, tok, true);
        SAFE_LLIST_CREATE(new_nodes);
        Nst_Node *value = parse_extraction();
        if ( p_state.error != NULL )
        {
            LList_destroy(new_tokens, nst_destroy_token);
            LList_destroy(new_nodes, NULL); // it's empty
            return NULL;
        }

        LList_append(new_nodes, value, true);
        return nst_new_node_full(
            tok->start,
            value->end,
            NST_NT_LOCAL_OP,
            new_nodes,
            new_tokens
        );
    }
    else if ( tok->type == NST_TT_CALL )
    {
        LList_push(tokens, tok, true);
        SAFE_LLIST_CREATE(nodes);
        return parse_local_stack_op(nodes);
    }
    else if ( tok->type == NST_TT_L_VBRACE )
    {
        LList_push(tokens, tok, true);
        return parse_vector_literal();
    }
    else if ( tok->type == NST_TT_L_BRACE )
    {
        LList_push(tokens, tok, true);
        return parse_arr_or_map_literal();
    }
    else
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        RETURN_ERROR(err_start, err_end, EXPECTED_VALUE);
    }
}

static Nst_Node *parse_vector_literal()
{
    Nst_LexerToken *tok = LList_pop(tokens);
    Nst_Pos start = tok->start;
    Nst_Pos err_end = tok->end;
    nst_destroy_token(tok);

    if ( TOK(LList_peek_front(tokens))->type == NST_TT_R_VBRACE )
    {
        tok = LList_pop(tokens);
        Nst_Pos end = tok->end;
        nst_destroy_token(tok);
        return nst_new_node_empty(start, end, NST_NT_VEC_LIT);
    }

    SAFE_LLIST_CREATE(nodes);

    while ( true )
    {
        Nst_Node *value = parse_expr(true);
        if ( p_state.error != NULL )
        {
            LList_destroy(nodes, nst_destroy_node);
            return NULL;
        }

        LList_append(nodes, value, true);

        tok = LList_pop(tokens);

        if ( tok->type == NST_TT_BREAK && nodes->size == 1 )
        {
            SAFE_LLIST_CREATE(node_tokens);
            LList_append(node_tokens, tok, true);
            skip_blank();

            value = parse_expr(false);
            if ( p_state.error != NULL )
            {
                LList_destroy(nodes, nst_destroy_node);
                return NULL;
            }
            LList_append(nodes, value, true);

            skip_blank();
            tok = LList_pop(tokens);

            if ( tok->type != NST_TT_R_VBRACE )
            {
                LList_destroy(nodes, nst_destroy_node);
                LList_destroy(nodes, nst_destroy_token);
                Nst_Pos err_start = tok->start;
                Nst_Pos err_end = tok->end;
                nst_destroy_token(tok);
                RETURN_ERROR(err_start, err_end, EXPECTED_BRACE);
            }
            Nst_Pos end = tok->end;
            nst_destroy_token(tok);
            return nst_new_node_full(
                start,
                end,
                NST_NT_VEC_LIT,
                nodes,
                node_tokens
            );
        }
        else if ( tok->type == NST_TT_COMMA )
        {
            nst_destroy_token(tok);
        }
        else break;
    }

    if ( tok->type != NST_TT_R_VBRACE )
    {
        LList_destroy(nodes, nst_destroy_node);
        RETURN_ERROR(start, err_end, MISSING_VECTOR_BRACE);
    }

    Nst_Pos end = tok->end;
    return nst_new_node_nodes(start, end, NST_NT_VEC_LIT, nodes);
}

static Nst_Node *parse_arr_or_map_literal()
{
    Nst_LexerToken *tok = LList_pop(tokens);
    Nst_Pos start = tok->start;
    nst_destroy_token(tok);

    bool is_map = false;
    size_t count = 0;

    if ( TOK(LList_peek_front(tokens))->type == NST_TT_R_BRACE )
    {
        tok = LList_pop(tokens);
        Nst_Pos end = tok->end;
        nst_destroy_token(tok);
        return nst_new_node_empty(start, end, NST_NT_MAP_LIT);
    }

    SAFE_LLIST_CREATE(nodes);

    while ( true )
    {
        skip_blank();

        Nst_Node *value = parse_expr(true);
        if ( p_state.error != NULL )
        {
            LList_destroy(nodes, NULL); // it's empty
            return NULL;
        }
        LList_append(nodes, value, true);

        skip_blank();
        tok = LList_pop(tokens);

        if ( tok->type == NST_TT_COLON && (count == 0 || is_map) )
        {
            is_map = true;

            nst_destroy_token(tok);
            skip_blank();

            value = parse_expr(false);
            if ( p_state.error != NULL )
            {
                LList_destroy(nodes, nst_destroy_node);
                return NULL;
            }
            LList_append(nodes, value, true);

            skip_blank();
            tok = LList_pop(tokens);
        }
        else if ( tok->type == NST_TT_BREAK && count == 0 )
        {
            SAFE_LLIST_CREATE(node_tokens);
            LList_append(node_tokens, tok, true);
            skip_blank();

            value = parse_expr(false);
            if ( p_state.error != NULL )
            {
                LList_destroy(nodes, nst_destroy_node);
                return NULL;
            }
            LList_append(nodes, value, true);

            skip_blank();
            tok = LList_pop(tokens);

            if ( tok->type != NST_TT_R_BRACE )
            {
                LList_destroy(nodes, nst_destroy_node);
                LList_destroy(nodes, nst_destroy_token);
                Nst_Pos err_start = tok->start;
                Nst_Pos err_end = tok->end;
                nst_destroy_token(tok);
                RETURN_ERROR(err_start, err_end, EXPECTED_BRACE);
            }
            Nst_Pos end = tok->end;
            nst_destroy_token(tok);
            return nst_new_node_full(
                start,
                end,
                NST_NT_ARR_LIT,
                nodes,
                node_tokens
            );
        }
        // if it's not the first iteration and it's not supposed to be a map
        else if ( tok->type == NST_TT_COLON )
        {
            LList_destroy(nodes, nst_destroy_node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_destroy_token(tok);
            RETURN_ERROR(err_start, err_end, EXPECTED_COMMA_OR_BRACE);
        }
        else if ( tok->type != NST_TT_COLON && is_map )
        {
            LList_destroy(nodes, nst_destroy_node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_destroy_token(tok);
            RETURN_ERROR(err_start, err_end, EXPECTED_COLON);
        }

        if ( tok->type == NST_TT_R_BRACE )
        {
            Nst_Pos end = tok->end;
            nst_destroy_token(tok);
            return nst_new_node_nodes(
                start, end,
                is_map ? NST_NT_MAP_LIT : NST_NT_ARR_LIT,
                nodes
            );
        }
        else if ( tok->type != NST_TT_COMMA )
        {
            LList_destroy(nodes, nst_destroy_node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_destroy_token(tok);
            RETURN_ERROR(err_start, err_end, EXPECTED_COMMA_OR_BRACE);
        }
        count++;
    }
}

static void _print_ast(Nst_Node *node, Nst_LexerToken *tok, int lvl, LList *is_last)
{
    LLNode *cursor = NULL;

    if ( lvl > 0 )
    {
        cursor = is_last->head;
        // Until, but excluding, the last node
        for ( cursor = is_last->head; cursor->next != NULL; cursor = cursor->next )
        {
            if ( cursor == NULL )
            {
                printf("cursor is null\n");
                return;
            }
            if ( *((bool *)(cursor->value)) )
                printf("   ");
            else
                printf("|  ");
        }
        if ( *((bool *)(cursor->value)) )
            printf("\\--");
        else
            printf("+--");
    }

    if ( tok != NULL )
    {
        nst_print_token(tok);
        printf("\n");
        return;
    }

    if      ( node->type == NST_NT_LONG_S ) printf("LONG_S");
    else if ( node->type == NST_NT_WHILE_L ) printf("WHILE_L");
    else if ( node->type == NST_NT_DOWHILE_L ) printf("DOWHILE_L");
    else if ( node->type == NST_NT_FOR_L ) printf("FOR_L");
    else if ( node->type == NST_NT_FOR_AS_L ) printf("FOR_AS_L");
    else if ( node->type == NST_NT_IF_E ) printf("IF_E");
    else if ( node->type == NST_NT_FUNC_DECLR ) printf("FUNC_DECLR");
    else if ( node->type == NST_NT_RETURN_S ) printf("RETURN_S");
    else if ( node->type == NST_NT_STACK_OP ) printf("STACK_OP");
    else if ( node->type == NST_NT_LOCAL_STACK_OP ) printf("LOCAL_STACK_OP");
    else if ( node->type == NST_NT_LOCAL_OP ) printf("LOCAL_OP");
    else if ( node->type == NST_NT_ARR_LIT ) printf("ARR_LIT");
    else if ( node->type == NST_NT_VEC_LIT ) printf("VECT_LIT");
    else if ( node->type == NST_NT_MAP_LIT ) printf("MAP_LIT");
    else if ( node->type == NST_NT_VALUE ) printf("VALUE");
    else if ( node->type == NST_NT_ACCESS ) printf("ACCESS");
    else if ( node->type == NST_NT_EXTRACT_E ) printf("EXTRACT_E");
    else if ( node->type == NST_NT_ASSIGN_E ) printf("ASSIGN_E");
    else if ( node->type == NST_NT_CONTINUE_S ) printf("CONTINUE_S");
    else if ( node->type == NST_NT_BREAK_S ) printf("BREAK_S");
    else printf("__UNKNOWN__");
    printf("\n");

    size_t tot_len = node->nodes->size + node->tokens->size - 1;
    size_t idx = 0;

    LLNode *prev_tail = is_last->tail;
    bool *last = malloc(sizeof(bool));
    if ( last == NULL )
        return;
    LList_append(is_last, last, true);

    for ( cursor = node->nodes->head; cursor != NULL; cursor = cursor->next )
    {
        *last = idx == tot_len;
        _print_ast(cursor->value, NULL, lvl + 1, is_last);
        idx++;
    }

    idx = 0;

    for ( cursor = node->tokens->head; cursor != NULL; cursor = cursor->next )
    {
        *last = idx == node->tokens->size - 1;
        _print_ast(NULL, cursor->value, lvl + 1, is_last);
        idx++;
    }

    free(last);
    free(is_last->tail);
    is_last->tail = prev_tail;
    if ( prev_tail == NULL )
        is_last->head = NULL;
    else
        prev_tail->next = NULL;
}

void nst_print_ast(Nst_Node *ast)
{
    LList *is_last = LList_new();
    if ( is_last == NULL )
        return;

    _print_ast(ast, NULL, 0, is_last);
}
