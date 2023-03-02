#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "llist.h"
#include "parser.h"
#include "error_internal.h"
#include "tokens.h"
#include "global_consts.h"

#define SAFE_LLIST_CREATE(name) \
    Nst_LList *name = nst_llist_new(); \
    if ( name == NULL ) \
    { \
        errno = ENOMEM; \
        return NULL; \
    } \

#define RETURN_ERROR(start, end, message) \
    do { \
    _NST_SET_RAW_SYNTAX_ERROR(p_state.error, start, end, message); \
    return NULL; \
    } while ( 0 )

typedef struct _ParsingState
{
    bool in_func;
    bool in_loop;
    bool in_switch;
    Nst_Error *error;
}
ParsingState;

static ParsingState p_state;
static Nst_LList *tokens;

static inline void skip_blank();
static Nst_Tok *copy_token(Nst_Tok *tok);
static Nst_Node *fix_expr(Nst_Node *expr);

static Nst_Node *parse_long_statement();
static Nst_Node *parse_statement();
static Nst_Node *parse_while_loop();
static Nst_Node *parse_for_loop();
static Nst_Node *parse_if_expr(Nst_Node *condition);
static Nst_Node *parse_switch_statement();
static Nst_Node *parse_func_def_or_lambda();
static Nst_Node *parse_expr(bool break_as_end);
static Nst_Node *parse_stack_op(Nst_Node *value, Nst_Pos start);
static Nst_Node *parse_local_stack_op(Nst_LList *nodes, Nst_Pos start);
static Nst_Node *parse_assignment_name(bool is_compound);
static Nst_Node *parse_assignment(Nst_Node *value);
static Nst_Node *parse_extraction();
static Nst_Node *parse_atom();
static Nst_Node *parse_vector_literal();
static Nst_Node *parse_arr_or_map_literal();
static Nst_Node *parse_try_catch();

Nst_Node *nst_parse(Nst_LList *tokens_list, Nst_Error *error)
{
    if ( tokens_list == NULL )
    {
        return NULL;
    }

    tokens = tokens_list;

    p_state.in_func = false;
    p_state.in_loop = false;
    p_state.in_switch = false;
    p_state.error = error;

    Nst_Node *node = parse_long_statement();

    // i.e. there are tokens other than NST_TT_EOFILE
    if ( !p_state.error->occurred && tokens->size > 1 )
    {
        Nst_Pos start = NST_TOK(nst_llist_peek_front(tokens))->start;
        Nst_Pos end = NST_TOK(nst_llist_peek_front(tokens))->start;

        _NST_SET_RAW_SYNTAX_ERROR(error, start, end, _NST_EM_UNEXPECTED_TOK);
    }

    nst_llist_destroy(tokens, (nst_llist_destructor)nst_token_destroy);
    return node; // NULL if there was an error
}

static inline void skip_blank()
{
    while ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_ENDL )
        nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
}

static Nst_Tok *copy_token(Nst_Tok *tok)
{
    return nst_tok_new_value(
        tok->start,
        tok->end,
        tok->type,
        tok->value != NULL ? nst_inc_ref(tok->value) : NULL);
}

static Nst_Node *parse_long_statement()
{
    SAFE_LLIST_CREATE(nodes);
    Nst_Node *node = NULL;
    skip_blank();

    while ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_R_BRACKET &&
            NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_EOFILE       )
    {
        node = parse_statement();
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            return NULL;
        }
        else if ( node != NULL )
        {
            nst_llist_append(nodes, node, true);
        }
        if ( errno == ENOMEM )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            return NULL;
        }
        skip_blank();
    }

    if ( nodes->size == 0 )
    {
        nst_llist_destroy(nodes, NULL);
        return nst_node_new_empty(
            NST_TOK(nst_llist_peek_front(tokens))->start,
            NST_TOK(nst_llist_peek_front(tokens))->end,
            NST_NT_LONG_S);
    }

    return nst_node_new_nodes(
        NST_NODE(nodes->head->value)->start,
        NST_NODE(nodes->tail->value)->end,
        NST_NT_LONG_S,
        nodes);
}

static Nst_Node *parse_statement()
{
    skip_blank();
    i32 tok_type = NST_TOK(nst_llist_peek_front(tokens))->type;

    if ( tok_type == NST_TT_L_BRACKET )
    {
        Nst_Tok *open_bracket = NST_TOK(nst_llist_pop(tokens));
        Nst_Pos start = open_bracket->start;
        nst_token_destroy(open_bracket);

        Nst_Node *node = parse_long_statement();
        if ( p_state.error->occurred )
        {
            return NULL;
        }

        Nst_Tok *close_bracket = NST_TOK(nst_llist_pop(tokens));
        if ( close_bracket->type != NST_TT_R_BRACKET )
        {
            nst_token_destroy(close_bracket);
            nst_node_destroy(node);
            RETURN_ERROR(start, start, _NST_EM_MISSING_BRACKET);
        }
        nst_token_destroy(close_bracket);
        return node;
    }
    else if ( tok_type == NST_TT_WHILE || tok_type == NST_TT_DOWHILE )
    {
        return parse_while_loop();
    }
    else if ( tok_type == NST_TT_FOR )
    {
        return parse_for_loop();
    }
    else if ( tok_type == NST_TT_FUNC )
    {
        return parse_func_def_or_lambda();
    }
    else if ( tok_type == NST_TT_SWITCH )
    {
        return parse_switch_statement();
    }
    else if ( tok_type == NST_TT_RETURN )
    {
        Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        Nst_Node *expr;
        nst_token_destroy(tok);

        if ( !p_state.in_func )
        {
            RETURN_ERROR(start, end, _NST_EM_BAD_RETURN);
        }

        tok_type = NST_TOK(nst_llist_peek_front(tokens))->type;
        if ( NST_IS_EXPR_END(tok_type) )
        {
            Nst_Tok *null_value = nst_tok_new_value(start, end,
                                                             NST_TT_VALUE,
                                                             nst_c.Null_null);
            nst_inc_ref(nst_c.Null_null);
            SAFE_LLIST_CREATE(value_node_tokens);
            nst_llist_append(value_node_tokens, null_value, true);
            expr = nst_node_new_tokens(
                start,
                end,
                NST_NT_VALUE,
                value_node_tokens);
        }
        else
        {
            expr = parse_expr(false);
            if ( p_state.error->occurred )
            {
                return NULL;
            }
        }

        SAFE_LLIST_CREATE(nodes);
        nst_llist_append(nodes, expr, true);
        return nst_node_new_nodes(start, expr->end, NST_NT_RETURN_S, nodes);
    }
    else if ( tok_type == NST_TT_CONTINUE )
    {
        Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        nst_token_destroy(tok);

        if ( !p_state.in_loop && !p_state.in_switch )
        {
            RETURN_ERROR(start, end, _NST_EM_BAD_CONTINUE);
        }

        return nst_node_new_empty(start, end, NST_NT_CONTINUE_S);
    }
    else if ( tok_type == NST_TT_BREAK )
    {
        Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        nst_token_destroy(tok);

        if ( !p_state.in_loop )
        {
            RETURN_ERROR(start, end, _NST_EM_BAD_BREAK);
        }

        return nst_node_new_empty(start, end, NST_NT_BREAK_S);
    }
    else if ( tok_type == NST_TT_TRY )
    {
        return parse_try_catch();
    }
    else if ( NST_IS_ATOM(tok_type) || NST_IS_LOCAL_STACK_OP(tok_type) )
    {
        return parse_expr(false);
    }
    else
    {
        Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;

        nst_token_destroy(tok);
        RETURN_ERROR(start, end, _NST_EM_UNEXPECTED_TOK);
    }
}

static Nst_Node *parse_while_loop()
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;

    Nst_NodeType node_type =
        tok->type == NST_TT_WHILE ? NST_NT_WHILE_L : NST_NT_DOWHILE_L;

    nst_token_destroy(tok);
    skip_blank();

    Nst_Node *condition = parse_expr(false);
    if ( p_state.error->occurred )
    {
        return NULL;
    }

    skip_blank();

    Nst_Pos err_pos = NST_TOK(nst_llist_peek_front(tokens))->start;

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_L_BRACKET )
    {
        nst_node_destroy(condition);
        RETURN_ERROR(err_pos, err_pos, _NST_EM_EXPECTED_BRACKET);
    }

    nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;

    Nst_Node *body = parse_long_statement();
    if ( p_state.error->occurred )
    {
        nst_node_destroy(condition);
        return NULL;
    }

    p_state.in_loop = prev_state;

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_R_BRACKET )
    {
        nst_node_destroy(condition);
        nst_node_destroy(body);
        RETURN_ERROR(err_pos, err_pos, _NST_EM_MISSING_BRACKET);
    }

    tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos end = tok->end;
    nst_token_destroy(tok);

    SAFE_LLIST_CREATE(nodes);

    nst_llist_append(nodes, condition, true);
    nst_llist_append(nodes, body, true);

    return nst_node_new_nodes(start, end, node_type, nodes);
}

static Nst_Node *parse_for_loop()
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    nst_token_destroy(tok);

    Nst_NodeType node_type = NST_NT_FOR_L;

    skip_blank();
    Nst_Node *range = parse_expr(false);

    if ( p_state.error->occurred )
    {
        return NULL;
    }

    SAFE_LLIST_CREATE(nodes);
    nst_llist_append(nodes, range, true);

    if ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_AS )
    {
        nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
        Nst_Node *name_node = parse_assignment_name(false);

        if ( p_state.error->occurred )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            return NULL;
        }
        nst_llist_append(nodes, name_node, true);
        node_type = NST_NT_FOR_AS_L;
    }

    skip_blank();

    Nst_Pos err_pos = NST_TOK(nst_llist_peek_front(tokens))->start;

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_L_BRACKET )
    {
        nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
        RETURN_ERROR(err_pos, err_pos, _NST_EM_EXPECTED_BRACKET);
    }

    nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;
    Nst_Node *body = parse_long_statement();
    if ( p_state.error->occurred )
    {
        nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
        return NULL;
    }
    p_state.in_loop = prev_state;
    nst_llist_append(nodes, body, true);

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_R_BRACKET )
    {
        nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
        RETURN_ERROR(err_pos, err_pos, _NST_EM_MISSING_BRACKET);
    }

    tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos end = tok->end;
    nst_token_destroy(tok);

    return nst_node_new_nodes(start, end, node_type, nodes);
}

static Nst_Node *parse_if_expr(Nst_Node *condition)
{
    nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
    skip_blank();

    SAFE_LLIST_CREATE(nodes);
    nst_llist_append(nodes, condition, true);

    Nst_Node *body_if_true = parse_statement();
    if ( p_state.error->occurred )
    {
        nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
        return NULL;
    }
    nst_llist_append(nodes, body_if_true, true);

    skip_blank();

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_COLON )
    {
        return nst_node_new_nodes(
            condition->start,
            body_if_true->end,
            NST_NT_IF_E,
            nodes);
    }

    nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
    skip_blank();

    Nst_Node *body_if_false = parse_statement();
    if ( p_state.error->occurred )
    {
        nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
        return NULL;
    }
    nst_llist_append(nodes, body_if_false, true);

    return nst_node_new_nodes(
        condition->start,
        body_if_false->end,
        NST_NT_IF_E,
        nodes);
}

static Nst_Node *parse_switch_statement()
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_Pos err_start;
    Nst_Pos err_end;
    nst_token_destroy(tok);
    skip_blank();

    SAFE_LLIST_CREATE(nodes);

    Nst_Node *main_val = parse_expr(false);

    if ( p_state.error->occurred )
    {
        nst_llist_destroy(nodes, NULL);
        return NULL;
    }
    nst_llist_append(nodes, main_val, true);

    skip_blank();
    tok = NST_TOK(nst_llist_pop(tokens));

    if ( tok->type != NST_TT_L_BRACKET )
    {
        err_start = tok->start;
        err_end = tok->end;
        nst_token_destroy(tok);
        nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
        RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_BRACKET);
    }
    nst_token_destroy(tok);

    bool is_default_case = false;
    bool prev_in_switch = p_state.in_switch;
    p_state.in_switch = true;

    while ( true )
    {
        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));
        if ( tok->type != NST_TT_IF )
        {
            err_start = tok->start;
            err_end = tok->end;
            nst_token_destroy(tok);
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_IF);
        }
        nst_token_destroy(tok);
        skip_blank();
        tok = NST_TOK(nst_llist_peek_front(tokens));

        if ( tok->type == NST_TT_L_BRACKET )
        {
            is_default_case = true;
            err_start = tok->start;
            err_end = tok->end;
            nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
        }
        else
        {
            Nst_Node *val = parse_expr(false);

            if ( p_state.error->occurred )
            {
                nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
                return NULL;
            }

            nst_llist_append(nodes, val, true);
            skip_blank();
            tok = NST_TOK(nst_llist_pop(tokens));
            err_start = tok->start;
            err_end = tok->end;

            if ( tok->type != NST_TT_L_BRACKET )
            {
                nst_token_destroy(tok);
                nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
                RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_BRACKET);
            }
            nst_token_destroy(tok);
        }

        Nst_Node *body = parse_long_statement();

        if ( p_state.error->occurred )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            return NULL;
        }

        nst_llist_append(nodes, body, true);
        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));
        if ( tok->type != NST_TT_R_BRACKET )
        {
            err_start = tok->start;
            err_end = tok->end;
            nst_token_destroy(tok);
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_R_BRACKET);
        }
        nst_token_destroy(tok);
        skip_blank();
        tok = NST_TOK(nst_llist_peek_front(tokens));

        if ( tok->type == NST_TT_R_BRACKET )
        {
            Nst_Pos end = tok->end;
            nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
            p_state.in_switch = prev_in_switch;
            return nst_node_new_nodes(start, end, NST_NT_SWITCH_S, nodes);
        }
        else if ( is_default_case )
        {
            err_start = tok->start;
            err_end = tok->end;
            nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_R_BRACKET);
        }
    }
}

static Nst_Node *parse_func_def_or_lambda()
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_Pos end;
    bool is_lambda = tok->type == NST_TT_LAMBDA;
    nst_token_destroy(tok);

    SAFE_LLIST_CREATE(node_tokens);

    skip_blank();
    while ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_IDENT )
    {
        nst_llist_append(node_tokens, NST_TOK(nst_llist_pop(tokens)), true);
        skip_blank();
    }

    Nst_Pos err_start = NST_TOK(nst_llist_peek_front(tokens))->start;
    Nst_Pos err_end = NST_TOK(nst_llist_peek_front(tokens))->end;

    // if there are no identifiers after #
    if ( node_tokens->size == 0 && !is_lambda )
    {
        nst_llist_destroy(node_tokens, (nst_llist_destructor)nst_token_destroy);
        RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_IDENT);
    }

    bool expects_r_bracket = false;
    Nst_Pos return_start = NST_TOK(nst_llist_peek_front(tokens))->start;

    if ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_L_BRACKET )
    {
        expects_r_bracket = true;
    }
    else if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_RETURN )
    {
        Nst_Pos tok_end = NST_TOK(nst_llist_peek_front(tokens))->start;
        nst_llist_destroy(node_tokens, (nst_llist_destructor)nst_token_destroy);
        RETURN_ERROR(
            return_start,
            tok_end,
            _NST_EM_EXPECTED_RETURN_OR_BRACKET);
    }

    nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));

    bool prev_state = p_state.in_func;
    p_state.in_func = true;

    Nst_Node *body;

    if ( expects_r_bracket )
    {
        body = parse_long_statement();
    }
    else
    {
        body = parse_expr(false);
    }

    if ( p_state.error->occurred )
    {
        nst_llist_destroy(node_tokens, (nst_llist_destructor)nst_token_destroy);
        return NULL;
    }

    p_state.in_func = prev_state;

    if ( expects_r_bracket &&
         NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_R_BRACKET )
    {
        nst_llist_destroy(node_tokens, (nst_llist_destructor)nst_token_destroy);
        nst_node_destroy(body);
        RETURN_ERROR(err_start, err_end, _NST_EM_MISSING_BRACKET);
    }
    else if ( expects_r_bracket )
    {
        tok = NST_TOK(nst_llist_pop(tokens));
        end = tok->end;
        nst_token_destroy(tok);
    }
    else
    {
        SAFE_LLIST_CREATE(return_nodes);
        nst_llist_append(return_nodes, body, true);
        body = nst_node_new_nodes(
            return_start,
            body->end,
            NST_NT_RETURN_S,
            return_nodes);
        end = body->end;
    }

    SAFE_LLIST_CREATE(nodes);
    nst_llist_append(nodes, body, true);

    return nst_node_new_full(
        start, end,
        is_lambda ? NST_NT_LAMBDA : NST_NT_FUNC_DECLR,
        nodes, node_tokens);
}

static Nst_Node *parse_expr(bool break_as_end)
{
    Nst_Node *node = NULL;
    Nst_Pos start = NST_TOK(nst_llist_peek_front(tokens))->start;
    i32 token_type = NST_TOK(nst_llist_peek_front(tokens))->type;

    if ( break_as_end )
    {
        while (!NST_IS_EXPR_END_W_BREAK(token_type))
        {
            node = parse_stack_op(node, start);
            if ( p_state.error->occurred || errno == ENOMEM )
            {
                return NULL;
            }
            token_type = NST_TOK(nst_llist_peek_front(tokens))->type;
        }
    }
    else
    {
        while (!NST_IS_EXPR_END(token_type))
        {
            node = parse_stack_op(node, start);
            if ( p_state.error->occurred || errno == ENOMEM )
            {
                return NULL;
            }
            token_type = NST_TOK(nst_llist_peek_front(tokens))->type;
        }
    }

    if ( node == NULL )
    {
        Nst_Tok *tok = NST_TOK(nst_llist_peek_front(tokens));
        RETURN_ERROR(tok->start, tok->end, _NST_EM_EXPECTED_VALUE);
    }

    node = fix_expr(node);
    if ( errno == ENOMEM )
    {
        nst_node_destroy(node);
        return NULL;
    }

    token_type = NST_TOK(nst_llist_peek_front(tokens))->type;

    if ( token_type == NST_TT_IF )
    {
        // the error propagates automatically
        return parse_if_expr(node);
    }
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
        Nst_Node *new_node = NST_NODE(nst_llist_peek_front(expr->nodes));
        nst_llist_destroy(expr->nodes, NULL);
        nst_llist_destroy(expr->tokens, (nst_llist_destructor)nst_token_destroy);
        free(expr);
        return fix_expr(new_node);
    }

    for ( NST_LLIST_ITER(cursor, expr->nodes) )
    {
        cursor->value = fix_expr(NST_NODE(cursor->value));
    }

    if ( expr->type != NST_NT_STACK_OP )
    {
        return expr;
    }

    Nst_Node *curr_node = expr;
    Nst_Tok *op_tok = NST_TOK(nst_llist_peek_front(expr->tokens));

    // comparisons are handled differently to not copy nodes
    if ( NST_IS_COMP_OP(op_tok->type) )
    {
        return expr;
    }

    // writing 1 2 3 4 + becomes 1 2 + 3 + 4 +
    for ( usize i = 0, n = expr->nodes->size - 2; i < n; i++ )
    {
        // get the positions
        Nst_Pos start = curr_node->start;
        Nst_Pos end = curr_node->end;
        // create the new lists
        SAFE_LLIST_CREATE(new_nodes);
        SAFE_LLIST_CREATE(new_tokens);
        // move the nodes except for the last one
        for ( usize j = 0, m = curr_node->nodes->size - 1; j < m; j++ )
        {
            nst_llist_append(new_nodes, nst_llist_pop(curr_node->nodes), true);
        }

        nst_llist_append(new_tokens, copy_token(op_tok), true);
        Nst_Node *new_node = nst_node_new_full(
            start,
            end,
            NST_NT_STACK_OP,
            new_nodes,
            new_tokens);

        nst_llist_push(curr_node->nodes, new_node, true);
        curr_node = new_node;
    }

    return expr;
}

static Nst_Node *parse_stack_op(Nst_Node *value, Nst_Pos start)
{
    Nst_Node *value_node = NULL;
    SAFE_LLIST_CREATE(new_nodes);
    if ( value != NULL )
    {
        nst_llist_append(new_nodes, value, true);
    }
    else
    {
        value_node = parse_extraction();
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(new_nodes, (nst_llist_destructor)nst_node_destroy);
            return NULL;
        }
        nst_llist_append(new_nodes, value_node, true);
    }

    while ( NST_IS_ATOM(NST_TOK(nst_llist_peek_front(tokens))->type) )
    {
        value_node = parse_extraction();
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(new_nodes, (nst_llist_destructor)nst_node_destroy);
            return NULL;
        }
        nst_llist_append(new_nodes, value_node, true);
    }

    Nst_Node *node = NULL;
    Nst_Tok *op_tok = NST_TOK(nst_llist_peek_front(tokens));
    Nst_Pos end = op_tok->end;
    bool is_local_stack_op = false;

    if ( NST_IS_STACK_OP(op_tok->type) )
    {
        SAFE_LLIST_CREATE(new_tokens);
        nst_llist_append(new_tokens, op_tok, true);
        node = nst_node_new_full(
            start,
            end,
            NST_NT_STACK_OP,
            new_nodes,
            new_tokens);
        nst_llist_pop(tokens);
    }
    else if ( NST_IS_LOCAL_STACK_OP(op_tok->type) )
    {
        node = parse_local_stack_op(new_nodes, start);
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(new_nodes, (nst_llist_destructor)nst_node_destroy);
            return NULL;
        }
        is_local_stack_op = true;
    }
    else if ( NST_IS_ASSIGNMENT(op_tok->type) && op_tok->type != NST_TT_ASSIGN )
    {
        NST_TOK(nst_llist_peek_front(tokens));
        SAFE_LLIST_CREATE(new_tokens);

        Nst_Tok *new_tok = nst_tok_new_noval(
            op_tok->start,
            op_tok->end,
            NST_ASSIGMENT_TO_STACK_OP(op_tok->type));

        nst_llist_append(new_tokens, new_tok, true);
        node = nst_node_new_full(
            start,
            end,
            NST_NT_STACK_OP,
            new_nodes,
            new_tokens);
    }
    else if ( new_nodes->size == 1 && value == NULL )
    {
        node = NST_NODE(nst_llist_pop(new_nodes));
        nst_llist_destroy(new_nodes, NULL);
    }
    else
    {
        nst_llist_destroy(new_nodes, (nst_llist_destructor)nst_node_destroy);
        RETURN_ERROR(op_tok->start, end, _NST_EM_EXPECTED_OP);
    }

    while ( !NST_IS_EXPR_END(NST_TOK(nst_llist_peek_front(tokens))->type) )
    {
        op_tok = NST_TOK(nst_llist_peek_front(tokens));
        if ( NST_IS_STACK_OP(op_tok->type) && !is_local_stack_op )
        {
            SAFE_LLIST_CREATE(new_node_nodes);
            SAFE_LLIST_CREATE(new_node_tokens);
            nst_llist_append(new_node_nodes, node, true);
            op_tok = NST_TOK(nst_llist_pop(tokens));
            nst_llist_append(new_node_tokens, op_tok, true);
            node = nst_node_new_full(
                start,
                op_tok->end,
                NST_NT_STACK_OP,
                new_node_nodes,
                new_node_tokens);
        }
        else if ( NST_IS_LOCAL_STACK_OP(op_tok->type) && is_local_stack_op )
        {
            SAFE_LLIST_CREATE(new_node_nodes);
            nst_llist_append(new_node_nodes, node, true);
            node = parse_local_stack_op(new_node_nodes, start);
            if ( p_state.error->occurred )
            {
                nst_llist_destroy(new_node_nodes, (nst_llist_destructor)nst_node_destroy);
                return NULL;
            }
        }
        else if ( NST_IS_ASSIGNMENT(op_tok->type) )
        {
            node = parse_assignment(node);
            if ( p_state.error->occurred )
            {
                return NULL;
            }
        }
        else
        {
            break;
        }
    }

    return node;
}

static Nst_Node *parse_local_stack_op(Nst_LList *nodes, Nst_Pos start)
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));

    if ( tok->type == NST_TT_CAST && nodes->size != 1 )
    {
        nst_token_destroy(tok);
        RETURN_ERROR(
            NST_NODE(nst_llist_peek_front(nodes))->start,
            NST_NODE(nst_llist_peek_back(nodes))->end,
            _NST_EM_LEFT_ARGS_NUM("::", "1", ""));
    }
    else if ( tok->type == NST_TT_RANGE &&
              nodes->size != 1          &&
              nodes->size != 2 )
    {
        nst_token_destroy(tok);
        RETURN_ERROR(
            NST_NODE(nst_llist_peek_front(nodes))->start,
            NST_NODE(nst_llist_peek_back(nodes))->end,
            _NST_EM_LEFT_ARGS_NUM("->", "1 or 2", "s"));
    }
    else if ( tok->type == NST_TT_THROW && nodes->size != 1 )
    {
        nst_token_destroy(tok);
        RETURN_ERROR(
            NST_NODE(nst_llist_peek_front(nodes))->start,
            NST_NODE(nst_llist_peek_back(nodes))->end,
            _NST_EM_LEFT_ARGS_NUM("!!", "1", ""));
    }
    else if ( tok->type == NST_TT_SEQ_CALL && nodes->size != 1 )
    {
        nst_token_destroy(tok);
        RETURN_ERROR(
            NST_NODE(nst_llist_peek_front(nodes))->start,
            NST_NODE(nst_llist_peek_back(nodes))->end,
            _NST_EM_LEFT_ARGS_NUM("*@", "1", ""));
    }

    SAFE_LLIST_CREATE(node_tokens);
    nst_llist_append(node_tokens, tok, true);

    Nst_Node *special_node = parse_extraction();
    if ( p_state.error->occurred )
    {
        nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
        nst_llist_destroy(node_tokens, (nst_llist_destructor)nst_token_destroy);
        return NULL;
    }

    nst_llist_append(nodes, special_node, true);

    // Nst_Pos start = NODE(LList_peek_front(nodes))->start;
    Nst_Pos end = special_node->end;

    return nst_node_new_full(
        start,
        end,
        NST_NT_LOCAL_STACK_OP,
        nodes,
        node_tokens);
}

static Nst_Node *parse_assignment_name(bool is_compound)
{
    Nst_Tok *tok = NST_TOK(nst_llist_peek_front(tokens));
    Nst_Pos start = tok->start;
    Nst_Node *node;
    Nst_Node *name_node;

    if ( tok->type != NST_TT_L_BRACE )
    {
        node = parse_extraction();

        if ( p_state.error->occurred )
        {
            return NULL;
        }

        if ( node->type != NST_NT_ACCESS && node->type != NST_NT_EXTRACT_E )
        {
            Nst_Pos err_start = node->start;
            Nst_Pos err_end = node->end;
            nst_node_destroy(node);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_IDENT_OR_EXTR);
        }
        return node;
    }

    tok = NST_TOK(nst_llist_pop(tokens));

    if ( is_compound )
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        nst_token_destroy(tok);
        RETURN_ERROR(err_start, err_end, _NST_EM_COMPOUND_ASSIGMENT);
    }

    SAFE_LLIST_CREATE(nodes);

    while ( true )
    {
        nst_token_destroy(tok);
        skip_blank();
        name_node = parse_assignment_name(false);

        if ( p_state.error->occurred )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            return NULL;
        }

        nst_llist_append(nodes, name_node, true);

        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));
        if ( tok->type == NST_TT_COMMA )
        {
            continue;
        }
        else if ( tok->type == NST_TT_R_BRACE )
        {
            Nst_Pos end = tok->end;
            nst_token_destroy(tok);

            return nst_node_new_nodes(start, end, NST_NT_ARR_LIT, nodes);
        }
        else
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_token_destroy(tok);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_COMMA_OR_BRACE);
        }
    }
}

static Nst_Node *parse_assignment(Nst_Node *value)
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    bool is_compound = tok->type != NST_TT_ASSIGN;
    Nst_Node *name = parse_assignment_name(is_compound);

    if ( p_state.error->occurred )
    {
        nst_token_destroy(tok);
        nst_node_destroy(value);
        return NULL;
    }

    // If a compound assignmen operator such as '+=' or '*='
    if ( is_compound )
    {
        SAFE_LLIST_CREATE(new_value_tokens);
        SAFE_LLIST_CREATE(new_value_nodes);

        Nst_Tok *op_tok = nst_tok_new_noval(
            tok->start,
            tok->end,
            NST_ASSIGMENT_TO_STACK_OP(tok->type));

        nst_llist_append(new_value_tokens, op_tok, true);
        // will be freed later if the list is destroyed
        nst_llist_append(new_value_nodes, name, false);
        nst_llist_append(new_value_nodes, value, true);

        value = nst_node_new_full(value->start, name->end, NST_NT_STACK_OP,
                              new_value_nodes, new_value_tokens);
    }
    nst_token_destroy(tok);

    Nst_Pos start = value->start;
    Nst_Pos end = name->end;

    SAFE_LLIST_CREATE(new_nodes);
    nst_llist_append(new_nodes, value, true);
    // here it's freed when the list is destroyed
    nst_llist_append(new_nodes, name, true);
    return nst_node_new_nodes(start, end, NST_NT_ASSIGN_E, new_nodes);
}

static Nst_Node *parse_extraction()
{
    Nst_Node *atom = parse_atom();
    if ( p_state.error->occurred )
    {
        return NULL;
    }

    Nst_Node *final_node = atom;

    while ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_EXTRACT )
    {
        nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
        bool treat_as_string =
            NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_IDENT;

        atom = parse_atom();
        if ( p_state.error->occurred )
        {
            nst_node_destroy(final_node);
            return NULL;
        }

        // makes `a.i` equivalent to `a.'i'`
        if ( treat_as_string )
        {
            atom->type = NST_NT_VALUE;
            NST_TOK(nst_llist_peek_front(atom->tokens))->type = NST_TT_VALUE;
        }

        SAFE_LLIST_CREATE(new_nodes);

        nst_llist_append(new_nodes, final_node, true);
        nst_llist_append(new_nodes, atom, true);

        final_node = nst_node_new_nodes(
            final_node->start,
            atom->end,
            NST_NT_EXTRACT_E,
            new_nodes);
    }
    return final_node;
}

static Nst_Node *parse_atom()
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));

    if ( NST_IS_VALUE(tok->type) )
    {
        SAFE_LLIST_CREATE(new_tokens);
        nst_llist_append(new_tokens, tok, true);
        return nst_node_new_tokens(
            tok->start,
            tok->end,
            tok->type == NST_TT_IDENT ? NST_NT_ACCESS : NST_NT_VALUE,
            new_tokens);
    }
    else if ( tok->type == NST_TT_L_PAREN )
    {
        Nst_Pos start = tok->start;
        Nst_Pos err_end = tok->end;
        nst_token_destroy(tok);
        Nst_Node *expr = parse_expr(false);
        if ( p_state.error->occurred )
        {
            return NULL;
        }

        tok = NST_TOK(nst_llist_pop(tokens));
        if ( tok->type != NST_TT_R_PAREN )
        {
            nst_node_destroy(expr);
            nst_token_destroy(tok);
            RETURN_ERROR(start, err_end, _NST_EM_MISSING_PAREN);
        }

        expr->start = start;
        expr->end = tok->end;

        nst_token_destroy(tok);
        return expr;
    }
    else if ( NST_IS_LOCAL_OP(tok->type) )
    {
        SAFE_LLIST_CREATE(new_tokens);
        nst_llist_append(new_tokens, tok, true);
        SAFE_LLIST_CREATE(new_nodes);
        Nst_Node *value = parse_extraction();
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(new_tokens, (nst_llist_destructor)nst_token_destroy);
            nst_llist_destroy(new_nodes, NULL); // it's empty
            return NULL;
        }

        nst_llist_append(new_nodes, value, true);
        return nst_node_new_full(
            tok->start,
            value->end,
            NST_NT_LOCAL_OP,
            new_nodes,
            new_tokens);
    }
    else if ( tok->type == NST_TT_CALL )
    {
        nst_llist_push(tokens, tok, true);
        SAFE_LLIST_CREATE(nodes);
        Nst_Node *call = parse_local_stack_op(nodes, tok->start);
        if ( call == NULL )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
        }
        return call;
    }
    else if ( tok->type == NST_TT_L_VBRACE )
    {
        nst_llist_push(tokens, tok, true);
        return parse_vector_literal();
    }
    else if ( tok->type == NST_TT_L_BRACE )
    {
        nst_llist_push(tokens, tok, true);
        return parse_arr_or_map_literal();
    }
    else if ( tok->type == NST_TT_LAMBDA )
    {
        nst_llist_push(tokens, tok, true);
        return parse_func_def_or_lambda();
    }
    else
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        nst_token_destroy(tok);
        RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_VALUE);
    }
}

static Nst_Node *parse_vector_literal()
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_Pos err_end = tok->end;
    nst_token_destroy(tok);

    if ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_R_VBRACE )
    {
        tok = NST_TOK(nst_llist_pop(tokens));
        Nst_Pos end = tok->end;
        nst_token_destroy(tok);
        return nst_node_new_empty(start, end, NST_NT_VEC_LIT);
    }

    SAFE_LLIST_CREATE(nodes);

    while ( true )
    {
        Nst_Node *value = parse_expr(true);
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            return NULL;
        }

        nst_llist_append(nodes, value, true);

        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));

        if ( tok->type == NST_TT_BREAK && nodes->size == 1 )
        {
            SAFE_LLIST_CREATE(node_tokens);
            nst_llist_append(node_tokens, tok, true);
            skip_blank();

            value = parse_expr(false);
            if ( p_state.error->occurred )
            {
                nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
                nst_llist_destroy(
                    node_tokens,
                    (nst_llist_destructor)nst_token_destroy);
                return NULL;
            }
            nst_llist_append(nodes, value, true);

            skip_blank();
            tok = NST_TOK(nst_llist_pop(tokens));

            if ( tok->type != NST_TT_R_VBRACE )
            {
                nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
                nst_llist_destroy(
                    node_tokens,
                    (nst_llist_destructor)nst_token_destroy);
                Nst_Pos tok_start = tok->start;
                Nst_Pos tok_end = tok->end;
                nst_token_destroy(tok);
                RETURN_ERROR(tok_start, tok_end, _NST_EM_EXPECTED_VBRACE);
            }
            Nst_Pos end = tok->end;
            nst_token_destroy(tok);
            return nst_node_new_full(
                start,
                end,
                NST_NT_VEC_LIT,
                nodes,
                node_tokens);
        }
        else if ( tok->type == NST_TT_COMMA )
        {
            nst_token_destroy(tok);
            skip_blank();
        }
        else
        {
            break;
        }
    }

    if ( tok->type != NST_TT_R_VBRACE )
    {
        nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
        nst_token_destroy(tok);
        RETURN_ERROR(start, err_end, _NST_EM_MISSING_VBRACE);
    }

    Nst_Pos end = tok->end;
    nst_token_destroy(tok);
    return nst_node_new_nodes(start, end, NST_NT_VEC_LIT, nodes);
}

static Nst_Node *parse_arr_or_map_literal()
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    nst_token_destroy(tok);
    skip_blank();

    bool is_map = false;
    usize count = 0;

    if ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_R_BRACE )
    {
        tok = NST_TOK(nst_llist_pop(tokens));
        Nst_Pos end = tok->end;
        nst_token_destroy(tok);
        return nst_node_new_empty(start, end, NST_NT_MAP_LIT);
    }

    skip_blank();

    if ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_COMMA )
    {
        nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));
        Nst_Pos end = tok->end;
        i32 type = tok->type;
        nst_token_destroy(tok);

        if ( type != NST_TT_R_BRACE )
        {
            RETURN_ERROR(start, end, _NST_EM_EXPECTED_BRACE);
        }
        return nst_node_new_empty(start, end, NST_NT_ARR_LIT);
    }

    SAFE_LLIST_CREATE(nodes);

    while ( true )
    {
        skip_blank();

        Nst_Node *value = parse_expr(true);
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(nodes, NULL); // it's empty
            return NULL;
        }
        nst_llist_append(nodes, value, true);

        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));

        if ( tok->type == NST_TT_COLON && (count == 0 || is_map) )
        {
            is_map = true;

            nst_token_destroy(tok);
            skip_blank();

            value = parse_expr(false);
            if ( p_state.error->occurred )
            {
                nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
                return NULL;
            }
            nst_llist_append(nodes, value, true);

            skip_blank();
            tok = NST_TOK(nst_llist_pop(tokens));
        }
        else if ( tok->type == NST_TT_BREAK && count == 0 )
        {
            SAFE_LLIST_CREATE(node_tokens);
            nst_llist_append(node_tokens, tok, true);
            skip_blank();

            value = parse_expr(false);
            if ( p_state.error->occurred )
            {
                nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
                nst_llist_destroy(
                    node_tokens,
                    (nst_llist_destructor)nst_token_destroy);
                return NULL;
            }
            nst_llist_append(nodes, value, true);

            skip_blank();
            tok = NST_TOK(nst_llist_pop(tokens));

            if ( tok->type != NST_TT_R_BRACE )
            {
                nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
                nst_llist_destroy(
                    node_tokens,
                    (nst_llist_destructor)nst_token_destroy);
                Nst_Pos err_start = tok->start;
                Nst_Pos err_end = tok->end;
                nst_token_destroy(tok);
                RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_BRACE);
            }
            Nst_Pos end = tok->end;
            nst_token_destroy(tok);
            return nst_node_new_full(
                start,
                end,
                NST_NT_ARR_LIT,
                nodes,
                node_tokens);
        }
        // if it's not the first iteration and it's not supposed to be a map
        else if ( tok->type == NST_TT_COLON )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_token_destroy(tok);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_COMMA_OR_BRACE);
        }
        else if ( tok->type != NST_TT_COLON && is_map )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_token_destroy(tok);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_COLON);
        }

        if ( tok->type == NST_TT_R_BRACE )
        {
            Nst_Pos end = tok->end;
            nst_token_destroy(tok);
            return nst_node_new_nodes(
                start, end,
                is_map ? NST_NT_MAP_LIT : NST_NT_ARR_LIT,
                nodes);
        }
        else if ( tok->type != NST_TT_COMMA )
        {
            nst_llist_destroy(nodes, (nst_llist_destructor)nst_node_destroy);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_token_destroy(tok);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_COMMA_OR_BRACE);
        }

        nst_token_destroy(tok);
        count++;
    }
}

static Nst_Node *parse_try_catch()
{
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    nst_token_destroy(tok);

    skip_blank();

    Nst_Node *try_block = parse_statement();
    if ( p_state.error->occurred )
    {
        return NULL;
    }

    skip_blank();

    tok = NST_TOK(nst_llist_pop(tokens));
    if ( tok->type != NST_TT_CATCH )
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        nst_token_destroy(tok);
        nst_node_destroy(try_block);
        RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_CATCH);
    }
    nst_token_destroy(tok);

    skip_blank();

    Nst_Tok *name_tok = NST_TOK(nst_llist_pop(tokens));
    if ( name_tok->type != NST_TT_IDENT )
    {
        Nst_Pos err_start = name_tok->start;
        Nst_Pos err_end = name_tok->end;
        nst_token_destroy(name_tok);
        nst_node_destroy(try_block);
        RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_IDENT);
    }

    skip_blank();

    Nst_Node *catch_block = parse_statement();
    if ( p_state.error->occurred )
    {
        nst_token_destroy(name_tok);
        nst_node_destroy(try_block);
        return NULL;
    }

    SAFE_LLIST_CREATE(nodes);
    SAFE_LLIST_CREATE(node_tokens);

    nst_llist_append(nodes, (void *)try_block, true);
    nst_llist_append(nodes, (void *)catch_block, true);
    nst_llist_append(node_tokens, (void *)name_tok, true);

    return nst_node_new_full(
        start,
        catch_block->end,
        NST_NT_TRY_CATCH_S,
        nodes,
        node_tokens);
}

static void _print_ast(Nst_Node  *node,
                       Nst_Tok   *tok,
                       i32        lvl,
                       Nst_LList *is_last)
{
    Nst_LLNode *cursor = NULL;

    if ( lvl > 0 )
    {
        cursor = is_last->head;
        // Until, but excluding, the last node
        for ( cursor = is_last->head;
              cursor->next != NULL;
              cursor = cursor->next )
        {
            if ( *((bool *)(cursor->value)) )
            {
                printf("   ");
            }
            else
            {
                printf("\xE2\x94\x82  ");
            }
        }
        if ( *((bool *)(cursor->value)) )
        {
            printf("\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80");
        }
        else
        {
            printf("\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80");
        }
    }

    if ( tok != NULL )
    {
        nst_print_tok(tok);
        printf("\n");
        return;
    }

    switch ( node->type )
    {
    case NST_NT_LONG_S:         printf("LONG_S");         break;
    case NST_NT_WHILE_L:        printf("WHILE_L");        break;
    case NST_NT_DOWHILE_L:      printf("DOWHILE_L");      break;
    case NST_NT_FOR_L:          printf("FOR_L");          break;
    case NST_NT_FOR_AS_L:       printf("FOR_AS_L");       break;
    case NST_NT_IF_E:           printf("IF_E");           break;
    case NST_NT_FUNC_DECLR:     printf("FUNC_DECLR");     break;
    case NST_NT_RETURN_S:       printf("RETURN_S");       break;
    case NST_NT_STACK_OP:       printf("STACK_OP");       break;
    case NST_NT_LOCAL_STACK_OP: printf("LOCAL_STACK_OP"); break;
    case NST_NT_LOCAL_OP:       printf("LOCAL_OP");       break;
    case NST_NT_ARR_LIT:        printf("ARR_LIT");        break;
    case NST_NT_VEC_LIT:        printf("VEC_LIT");        break;
    case NST_NT_MAP_LIT:        printf("MAP_LIT");        break;
    case NST_NT_VALUE:          printf("VALUE");          break;
    case NST_NT_ACCESS:         printf("ACCESS");         break;
    case NST_NT_EXTRACT_E:      printf("EXTRACT_E");      break;
    case NST_NT_ASSIGN_E:       printf("ASSIGN_E");       break;
    case NST_NT_CONTINUE_S:     printf("CONTINUE_S");     break;
    case NST_NT_BREAK_S:        printf("BREAK_S");        break;
    case NST_NT_SWITCH_S:       printf("SWITCH_S");       break;
    case NST_NT_LAMBDA:         printf("LAMBDA");         break;
    case NST_NT_TRY_CATCH_S:    printf("TRY_CATCH_S");    break;
    default:                    printf("__UNKNOWN__");    break;
    }

    printf(
        " (%li:%li, %li:%li)\n",
        node->start.line,
        node->start.col,
        node->end.line,
        node->end.col);

    usize tot_len = node->nodes->size + node->tokens->size - 1;
    usize idx = 0;

    Nst_LLNode *prev_tail = is_last->tail;
    bool *last = (bool *)malloc(sizeof(bool));
    if ( last == NULL )
    {
        return;
    }
    nst_llist_append(is_last, last, true);

    for ( cursor = node->tokens->head; cursor != NULL; cursor = cursor->next )
    {
        *last = idx == tot_len;
        _print_ast(NULL, NST_TOK(cursor->value), lvl + 1, is_last);
        idx++;
    }

    idx = 0;

    for ( cursor = node->nodes->head; cursor != NULL; cursor = cursor->next )
    {
        *last = idx == node->nodes->size - 1;
        _print_ast(NST_NODE(cursor->value), NULL, lvl + 1, is_last);
        idx++;
    }

    free(last);
    free(is_last->tail);
    is_last->tail = prev_tail;
    if ( prev_tail == NULL )
    {
        is_last->head = NULL;
    }
    else
    {
        prev_tail->next = NULL;
    }
}

void nst_print_ast(Nst_Node *ast)
{
    Nst_LList *is_last = nst_llist_new();
    if ( is_last == NULL )
    {
        return;
    }

    _print_ast(ast, NULL, 0, is_last);
    nst_llist_destroy(is_last, NULL);
}
