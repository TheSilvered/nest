#include <errno.h>
#include <stdio.h>
#include "mem.h"
#include "llist.h"
#include "parser.h"
#include "error_internal.h"
#include "tokens.h"
#include "global_consts.h"

#define RETURN_ERROR(start, end, message) \
    do { \
    _NST_SET_RAW_SYNTAX_ERROR(p_state.error, start, end, message); \
    return NULL; \
    } while ( 0 )

// Sets an error if cond is true or the error's name is not NULL
// SET_ERROR_IF_OP_ERR(0) will set the error if it occurred
#define SET_ERROR_IF_OP_ERR(...) do { \
    if ( nst_error_occurred() ) \
    { \
        Nst_Tok *_t_ = NST_TOK(nst_llist_peek_front(tokens)); \
        _NST_SET_ERROR_FROM_OP_ERR(p_state.error, _t_->start, _t_->end); \
        __VA_ARGS__ \
        return NULL; \
    } } while ( 0 )

#define SAFE_NODE_APPEND(node, node_to_append) \
    nst_llist_append(node->nodes, node_to_append, true); \
    SET_ERROR_IF_OP_ERR(nst_node_destroy(node);nst_node_destroy(node_to_append);)

#define SAFE_TOK_APPEND(node, tok_to_append) \
    nst_llist_append(node->tokens, tok_to_append, true); \
    SET_ERROR_IF_OP_ERR(nst_node_destroy(node);nst_token_destroy(tok_to_append);)

#define RETURN_IF_ERROR(node) \
    if ( p_state.error->occurred ) { \
        nst_node_destroy(node); \
        return NULL; \
    }

#define INC_RECURSION_LVL do { \
    recursion_lvl++; \
    if ( recursion_lvl > 1500 ) \
    { \
        Nst_Tok *_t_ = NST_TOK(nst_llist_peek_front(tokens)); \
        _NST_SET_RAW_MEMORY_ERROR( \
            p_state.error, \
            _t_->start, \
            _t_->end, \
            "over 1500 recursive calls, parsing failed"); \
        return NULL; \
    } \
    } while ( 0 )

#define DEC_RECURSION_LVL recursion_lvl--

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
static int recursion_lvl = 0;

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
    recursion_lvl = 0;
    Nst_Node *node = parse_long_statement();

    // i.e. there are tokens other than NST_TT_EOFILE
    if ( !p_state.error->occurred && tokens->size > 1 )
    {
        Nst_Pos start = NST_TOK(nst_llist_peek_front(tokens))->start;
        Nst_Pos end = NST_TOK(nst_llist_peek_front(tokens))->start;

        _NST_SET_RAW_SYNTAX_ERROR(error, start, end, _NST_EM_UNEXPECTED_TOK);
    }

    nst_llist_destroy(tokens, (Nst_LListDestructor)nst_token_destroy);
    return node; // NULL if there was an error
}

static inline void skip_blank()
{
    while ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_ENDL )
    {
        nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
    }
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
    INC_RECURSION_LVL;
    Nst_Node *long_statement_node = nst_node_new(NST_NT_LONG_S);
    SET_ERROR_IF_OP_ERR();

    Nst_Node *node = NULL;
    Nst_LList *nodes = long_statement_node->nodes;
    skip_blank();

    while ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_R_BRACKET &&
            NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_EOFILE       )
    {
        node = parse_statement();
        RETURN_IF_ERROR(long_statement_node);
        SAFE_NODE_APPEND(long_statement_node, node);
        skip_blank();
    }

    if ( nodes->size == 0 )
        nst_node_set_pos(
            long_statement_node,
            NST_TOK(nst_llist_peek_front(tokens))->start,
            NST_TOK(nst_llist_peek_front(tokens))->end);
    else
        nst_node_set_pos(
            long_statement_node,
            NST_NODE(nodes->head->value)->start,
            NST_NODE(nodes->tail->value)->end);

    DEC_RECURSION_LVL;
    return long_statement_node;
}

static Nst_Node *parse_statement()
{
    INC_RECURSION_LVL;
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
        DEC_RECURSION_LVL;
        return node;
    }
    else if ( tok_type == NST_TT_WHILE || tok_type == NST_TT_DOWHILE )
    {
        DEC_RECURSION_LVL;
        return parse_while_loop();
    }
    else if ( tok_type == NST_TT_FOR )
    {
        DEC_RECURSION_LVL;
        return parse_for_loop();
    }
    else if ( tok_type == NST_TT_FUNC )
    {
        DEC_RECURSION_LVL;
        return parse_func_def_or_lambda();
    }
    else if ( tok_type == NST_TT_SWITCH )
    {
        DEC_RECURSION_LVL;
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
            Nst_Tok *null_value = nst_tok_new_value(
                start, end,
                NST_TT_VALUE,
                nst_inc_ref(nst_c.Null_null));
            SET_ERROR_IF_OP_ERR();
            expr = nst_node_new_pos(
                NST_NT_VALUE,
                start,
                end);
            SET_ERROR_IF_OP_ERR(nst_token_destroy(null_value););
            SAFE_TOK_APPEND(expr, null_value);
        }
        else
        {
            expr = parse_expr(false);
            if ( p_state.error->occurred )
            {
                return NULL;
            }
        }

        Nst_Node *return_s_node = nst_node_new_pos(
            NST_NT_RETURN_S,
            start, expr->end);
        SET_ERROR_IF_OP_ERR(nst_node_destroy(expr););
        SAFE_NODE_APPEND(return_s_node, expr);
        DEC_RECURSION_LVL;
        return return_s_node;
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

        Nst_Node *continue_node = nst_node_new_pos(
            NST_NT_CONTINUE_S,
            start, end);
        SET_ERROR_IF_OP_ERR();
        DEC_RECURSION_LVL;
        return continue_node;
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
        Nst_Node *break_node = nst_node_new_pos(
            NST_NT_BREAK_S,
            start, end);
        SET_ERROR_IF_OP_ERR();
        DEC_RECURSION_LVL;
        return break_node;
    }
    else if ( tok_type == NST_TT_TRY )
    {
        DEC_RECURSION_LVL;
        return parse_try_catch();
    }
    else if ( NST_IS_ATOM(tok_type) || NST_IS_LOCAL_STACK_OP(tok_type) )
    {
        DEC_RECURSION_LVL;
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
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;

    Nst_Node *while_l_node = nst_node_new(
        tok->type == NST_TT_WHILE ? NST_NT_WHILE_L : NST_NT_DOWHILE_L);
    SET_ERROR_IF_OP_ERR();

    nst_token_destroy(tok);
    skip_blank();

    Nst_Node *condition = parse_expr(false);
    RETURN_IF_ERROR(while_l_node);
    SAFE_NODE_APPEND(while_l_node, condition);
    skip_blank();

    Nst_Pos err_pos = NST_TOK(nst_llist_peek_front(tokens))->start;

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_L_BRACKET )
    {
        nst_node_destroy(while_l_node);
        RETURN_ERROR(err_pos, err_pos, _NST_EM_EXPECTED_BRACKET);
    }

    nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;

    Nst_Node *body = parse_long_statement();
    RETURN_IF_ERROR(while_l_node);
    SAFE_NODE_APPEND(while_l_node, body);
    p_state.in_loop = prev_state;

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_R_BRACKET )
    {
        nst_node_destroy(while_l_node);
        RETURN_ERROR(err_pos, err_pos, _NST_EM_MISSING_BRACKET);
    }

    tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos end = tok->end;
    nst_token_destroy(tok);

    nst_node_set_pos(while_l_node, start, end);
    DEC_RECURSION_LVL;
    return while_l_node;
}

static Nst_Node *parse_for_loop()
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    nst_token_destroy(tok);

    Nst_Node *for_l_node = nst_node_new(NST_NT_FOR_L);
    SET_ERROR_IF_OP_ERR();

    skip_blank();
    Nst_Node *range = parse_expr(false);
    RETURN_IF_ERROR(for_l_node);

    nst_llist_append(for_l_node->nodes, range, true);
    SET_ERROR_IF_OP_ERR(nst_node_destroy(for_l_node); nst_node_destroy(range););

    if ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_AS )
    {
        nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
        Nst_Node *name_node = parse_assignment_name(false);
        RETURN_IF_ERROR(for_l_node);
        SAFE_NODE_APPEND(for_l_node, name_node);
        for_l_node->type = NST_NT_FOR_AS_L;
    }

    skip_blank();

    Nst_Pos err_pos = NST_TOK(nst_llist_peek_front(tokens))->start;

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_L_BRACKET )
    {
        nst_node_destroy(for_l_node);
        RETURN_ERROR(err_pos, err_pos, _NST_EM_EXPECTED_BRACKET);
    }

    nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;
    Nst_Node *body = parse_long_statement();
    RETURN_IF_ERROR(for_l_node);
    SAFE_NODE_APPEND(for_l_node, body);
    p_state.in_loop = prev_state;

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_R_BRACKET )
    {
        nst_node_destroy(for_l_node);
        RETURN_ERROR(err_pos, err_pos, _NST_EM_MISSING_BRACKET);
    }

    tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos end = tok->end;
    nst_token_destroy(tok);

    nst_node_set_pos(for_l_node, start, end);
    DEC_RECURSION_LVL;
    return for_l_node;
}

static Nst_Node *parse_if_expr(Nst_Node *condition)
{
    INC_RECURSION_LVL;
    nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
    skip_blank();

    Nst_Node *if_expr_node = nst_node_new(NST_NT_IF_E);
    SET_ERROR_IF_OP_ERR();
    SAFE_NODE_APPEND(if_expr_node, condition);

    Nst_Node *body_if_true = parse_statement();
    RETURN_IF_ERROR(if_expr_node);
    SAFE_NODE_APPEND(if_expr_node, body_if_true);
    skip_blank();

    if ( NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_COLON )
    {
        nst_node_set_pos(if_expr_node, condition->start, body_if_true->end);
        DEC_RECURSION_LVL;
        return if_expr_node;
    }

    nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
    skip_blank();

    Nst_Node *body_if_false = parse_statement();
    RETURN_IF_ERROR(if_expr_node);
    SAFE_NODE_APPEND(if_expr_node, body_if_false);

    nst_node_set_pos(if_expr_node, condition->start, body_if_false->end);
    DEC_RECURSION_LVL;
    return if_expr_node;
}

static Nst_Node *parse_switch_statement()
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_Pos err_start;
    Nst_Pos err_end;
    nst_token_destroy(tok);
    skip_blank();

    Nst_Node *switch_s_node = nst_node_new(NST_NT_SWITCH_S);
    SET_ERROR_IF_OP_ERR();

    Nst_Node *main_val = parse_expr(false);
    RETURN_IF_ERROR(switch_s_node);
    SAFE_NODE_APPEND(switch_s_node, main_val);

    skip_blank();
    tok = NST_TOK(nst_llist_pop(tokens));

    if ( tok->type != NST_TT_L_BRACKET )
    {
        err_start = tok->start;
        err_end = tok->end;
        nst_token_destroy(tok);
        nst_node_destroy(switch_s_node);
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
            nst_node_destroy(switch_s_node);
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
            RETURN_IF_ERROR(switch_s_node);
            SAFE_NODE_APPEND(switch_s_node, val);
            skip_blank();
            tok = NST_TOK(nst_llist_pop(tokens));
            err_start = tok->start;
            err_end = tok->end;

            if ( tok->type != NST_TT_L_BRACKET )
            {
                nst_token_destroy(tok);
                nst_node_destroy(switch_s_node);
                RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_BRACKET);
            }
            nst_token_destroy(tok);
        }

        Nst_Node *body = parse_long_statement();
        RETURN_IF_ERROR(switch_s_node);
        SAFE_NODE_APPEND(switch_s_node, body);
        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));
        if ( tok->type != NST_TT_R_BRACKET )
        {
            err_start = tok->start;
            err_end = tok->end;
            nst_token_destroy(tok);
            nst_node_destroy(switch_s_node);
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
            nst_node_set_pos(switch_s_node, start, end);
            DEC_RECURSION_LVL;
            return switch_s_node;
        }
        else if ( is_default_case )
        {
            err_start = tok->start;
            err_end = tok->end;
            nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
            nst_node_destroy(switch_s_node);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_R_BRACKET);
        }
    }
}

static Nst_Node *parse_func_def_or_lambda()
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_Pos end;
    bool is_lambda = tok->type == NST_TT_LAMBDA;
    nst_token_destroy(tok);

    Nst_Node *func_node = nst_node_new(
        is_lambda ? NST_NT_LAMBDA : NST_NT_FUNC_DECLR);

    skip_blank();
    while ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_IDENT )
    {
        tok = NST_TOK(nst_llist_pop(tokens));
        SAFE_TOK_APPEND(func_node, tok);
        skip_blank();
    }

    Nst_Pos err_start = NST_TOK(nst_llist_peek_front(tokens))->start;
    Nst_Pos err_end = NST_TOK(nst_llist_peek_front(tokens))->end;

    // if there are no identifiers after #
    if ( func_node->tokens->size == 0 && !is_lambda )
    {
        nst_node_destroy(func_node);
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
        nst_node_destroy(func_node);
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
    RETURN_IF_ERROR(func_node);

    p_state.in_func = prev_state;

    if ( expects_r_bracket &&
         NST_TOK(nst_llist_peek_front(tokens))->type != NST_TT_R_BRACKET )
    {
        nst_node_destroy(func_node);
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
        Nst_Node *return_node = nst_node_new_pos(
            NST_NT_RETURN_S,
            return_start,
            body->end);

        nst_llist_append(return_node->nodes, body, true);
        SET_ERROR_IF_OP_ERR(
            nst_node_destroy(func_node);
        nst_node_destroy(body);
        nst_node_destroy(return_node););
        body = return_node;
        end = return_node->end;
    }

    SAFE_NODE_APPEND(func_node, body);
    nst_node_set_pos(func_node, start, end);
    DEC_RECURSION_LVL;
    return func_node;
}

static Nst_Node *parse_expr(bool break_as_end)
{
    INC_RECURSION_LVL;
    Nst_Node *node = NULL;
    Nst_Pos start = NST_TOK(nst_llist_peek_front(tokens))->start;
    i32 token_type = NST_TOK(nst_llist_peek_front(tokens))->type;

    if ( break_as_end )
    {
        while (!NST_IS_EXPR_END_W_BREAK(token_type))
        {
            node = parse_stack_op(node, start);
            if ( p_state.error->occurred )
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
            if ( p_state.error->occurred )
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
    if ( node == NULL )
    {
        return NULL;
    }

    token_type = NST_TOK(nst_llist_peek_front(tokens))->type;

    if ( token_type == NST_TT_IF )
    {
        DEC_RECURSION_LVL;
        // the error propagates automatically
        return parse_if_expr(node);
    }
    DEC_RECURSION_LVL;
    return node;
}

static Nst_Node *fix_expr(Nst_Node *expr)
{
    INC_RECURSION_LVL;
    if ( expr->type != NST_NT_STACK_OP &&
         expr->type != NST_NT_LOCAL_STACK_OP &&
         expr->type != NST_NT_ASSIGN_E )
    {
        DEC_RECURSION_LVL;
        return expr;
    }

    else if ( expr->type == NST_NT_STACK_OP && expr->nodes->size == 1 )
    {
        Nst_Node *new_node = NST_NODE(nst_llist_peek_front(expr->nodes));
        nst_llist_empty(expr->tokens, (Nst_LListDestructor)nst_token_destroy);
        nst_llist_empty(expr->nodes, NULL);
        nst_free(expr->tokens);
        nst_free(expr);
        DEC_RECURSION_LVL;
        return fix_expr(new_node);
    }

    for ( NST_LLIST_ITER(cursor, expr->nodes) )
    {
        cursor->value = fix_expr(NST_NODE(cursor->value));
    }

    if ( expr->type != NST_NT_STACK_OP )
    {
        DEC_RECURSION_LVL;
        return expr;
    }

    Nst_Node *curr_node = expr;
    Nst_Tok *op_tok = NST_TOK(nst_llist_peek_front(expr->tokens));

    // comparisons are handled differently to not copy nodes
    if ( NST_IS_COMP_OP(op_tok->type) )
    {
        DEC_RECURSION_LVL;
        return expr;
    }

    // writing 1 2 3 4 + becomes 1 2 + 3 + 4 +
    for ( usize i = 0, n = expr->nodes->size - 2; i < n; i++ )
    {
        // get the positions
        Nst_Pos start = curr_node->start;
        Nst_Pos end = curr_node->end;
        Nst_Node *new_node = nst_node_new_pos(
            NST_NT_STACK_OP,
            start, end);

        // move the nodes except for the last one
        for ( usize j = 0, m = curr_node->nodes->size - 1; j < m; j++ )
        {
            nst_llist_append_llnode(
                new_node->nodes,
                nst_llist_pop_llnode(curr_node->nodes));
        }

        Nst_Tok *new_tok = copy_token(op_tok);
        SET_ERROR_IF_OP_ERR(
                nst_node_destroy(expr);
                nst_node_destroy(new_node););
        nst_llist_append(new_node->tokens, new_tok, true);
        SET_ERROR_IF_OP_ERR(
                nst_node_destroy(expr);
                nst_node_destroy(new_node);
                nst_token_destroy(new_tok););

        nst_llist_push(curr_node->nodes, new_node, true);
        SET_ERROR_IF_OP_ERR(nst_node_destroy(expr););
        curr_node = new_node;
    }

    DEC_RECURSION_LVL;
    return expr;
}

static Nst_Node *parse_stack_op(Nst_Node *value, Nst_Pos start)
{
    INC_RECURSION_LVL;
    Nst_Node *value_node = NULL;
    Nst_LList *new_nodes = nst_llist_new();
    SET_ERROR_IF_OP_ERR();

    if ( value != NULL )
    {
        nst_llist_append(new_nodes, value, true);
        SET_ERROR_IF_OP_ERR(nst_node_destroy(value););
    }
    else
    {
        value_node = parse_extraction();
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(new_nodes, NULL);
            return NULL;
        }
        nst_llist_append(new_nodes, value_node, true);
        SET_ERROR_IF_OP_ERR(nst_node_destroy(value_node););
    }

    while ( NST_IS_ATOM(NST_TOK(nst_llist_peek_front(tokens))->type) )
    {
        value_node = parse_extraction();
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(new_nodes, (Nst_LListDestructor)nst_node_destroy);
            return NULL;
        }
        nst_llist_append(new_nodes, value_node, true);
        SET_ERROR_IF_OP_ERR(nst_node_destroy(value_node););
    }

    Nst_Node *node = NULL;
    Nst_Tok *op_tok = NST_TOK(nst_llist_peek_front(tokens));
    Nst_Pos end = op_tok->end;
    bool is_local_stack_op = false;

    if ( NST_IS_STACK_OP(op_tok->type) )
    {
        node = nst_node_new_pos(NST_NT_STACK_OP, start, end);
        SET_ERROR_IF_OP_ERR(
            nst_llist_destroy(new_nodes, (Nst_LListDestructor)nst_node_destroy););
        nst_llist_move_nodes(new_nodes, node->nodes);
        nst_free(new_nodes);
        nst_llist_pop(tokens);
        SAFE_TOK_APPEND(node, op_tok);
    }
    else if ( NST_IS_LOCAL_STACK_OP(op_tok->type) )
    {
        node = parse_local_stack_op(new_nodes, start);
        if ( p_state.error->occurred )
        {
            nst_llist_destroy(new_nodes, (Nst_LListDestructor)nst_node_destroy);
            return NULL;
        }
        is_local_stack_op = true;
    }
    else if ( NST_IS_ASSIGNMENT(op_tok->type) && op_tok->type != NST_TT_ASSIGN )
    {
        Nst_Tok *new_tok = nst_tok_new_noval(
            op_tok->start,
            op_tok->end,
            NST_ASSIGMENT_TO_STACK_OP(op_tok->type));
        SET_ERROR_IF_OP_ERR(
            nst_llist_destroy(new_nodes, (Nst_LListDestructor)nst_node_destroy););
        node = nst_node_new_pos(
            NST_NT_STACK_OP,
            start, end);
        SET_ERROR_IF_OP_ERR(
            nst_llist_destroy(new_nodes, (Nst_LListDestructor)nst_node_destroy);
            nst_token_destroy(new_tok););
        nst_llist_move_nodes(new_nodes, node->nodes);
        nst_free(new_nodes);
        SAFE_TOK_APPEND(node, new_tok);
    }
    else if ( new_nodes->size == 1 && value == NULL )
    {
        node = NST_NODE(nst_llist_pop(new_nodes));
        nst_llist_destroy(new_nodes, NULL);
    }
    else
    {
        nst_llist_destroy(new_nodes, (Nst_LListDestructor)nst_node_destroy);
        RETURN_ERROR(op_tok->start, end, _NST_EM_EXPECTED_OP);
    }

    while ( !NST_IS_EXPR_END(NST_TOK(nst_llist_peek_front(tokens))->type) )
    {
        op_tok = NST_TOK(nst_llist_peek_front(tokens));
        if ( NST_IS_STACK_OP(op_tok->type) && !is_local_stack_op )
        {
            Nst_Node *new_node = nst_node_new_pos(
                NST_NT_STACK_OP,
                start, op_tok->end);
            SET_ERROR_IF_OP_ERR(nst_node_destroy(node););
            SAFE_NODE_APPEND(new_node, node);
            op_tok = NST_TOK(nst_llist_pop(tokens));
            SAFE_TOK_APPEND(new_node, op_tok);
        }
        else if ( NST_IS_LOCAL_STACK_OP(op_tok->type) && is_local_stack_op )
        {
            new_nodes = nst_llist_new();
            SET_ERROR_IF_OP_ERR(nst_node_destroy(node););
            nst_llist_append(new_nodes, node, true);
            SET_ERROR_IF_OP_ERR(nst_node_destroy(node););
            node = parse_local_stack_op(new_nodes, start);
            if ( p_state.error->occurred )
            {
                nst_llist_destroy(new_nodes, (Nst_LListDestructor)nst_node_destroy);
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

    DEC_RECURSION_LVL;
    return node;
}

static Nst_Node *parse_local_stack_op(Nst_LList *nodes, Nst_Pos start)
{
    INC_RECURSION_LVL;
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

    Nst_Node *local_stack_op_node = nst_node_new(NST_NT_LOCAL_STACK_OP);
    SAFE_TOK_APPEND(local_stack_op_node, tok);
    nst_llist_move_nodes(nodes, local_stack_op_node->nodes);

    Nst_Node *special_node = parse_extraction();
    RETURN_IF_ERROR(local_stack_op_node);
    SAFE_NODE_APPEND(local_stack_op_node, special_node);

    nst_free(nodes);
    nst_node_set_pos(local_stack_op_node, start, special_node->end);
    DEC_RECURSION_LVL;
    return local_stack_op_node;
}

static Nst_Node *parse_assignment_name(bool is_compound)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_peek_front(tokens));
    Nst_Pos start = tok->start;
    Nst_Node *node;

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
        DEC_RECURSION_LVL;
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

    node = nst_node_new(NST_NT_ARR_LIT);
    SET_ERROR_IF_OP_ERR();

    while ( true )
    {
        nst_token_destroy(tok);
        skip_blank();
        Nst_Node *name_node = parse_assignment_name(false);
        RETURN_IF_ERROR(node);
        SAFE_NODE_APPEND(node, name_node);

        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));
        if ( tok->type == NST_TT_COMMA )
        {
            continue;
        }
        else if ( tok->type == NST_TT_R_BRACE )
        {
            nst_node_set_pos(node, start, tok->end);
            nst_token_destroy(tok);
            DEC_RECURSION_LVL;
            return node;
        }
        else
        {
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_token_destroy(tok);
            nst_node_destroy(node);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_COMMA_OR_BRACE);
        }
    }
}

static Nst_Node *parse_assignment(Nst_Node *value)
{
    INC_RECURSION_LVL;
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
        Nst_Tok *op_tok = nst_tok_new_noval(
            tok->start,
            tok->end,
            NST_ASSIGMENT_TO_STACK_OP(tok->type));
        nst_token_destroy(tok);
        SET_ERROR_IF_OP_ERR(
            nst_node_destroy(value);
            nst_node_destroy(name););
        Nst_Node *op_node = nst_node_new_pos(
            NST_NT_STACK_OP,
            value->start,
            name->end);
        SET_ERROR_IF_OP_ERR(
            nst_node_destroy(value);
            nst_token_destroy(op_tok);
            nst_node_destroy(name););

        nst_llist_append(op_node->tokens, op_tok, true);
        SET_ERROR_IF_OP_ERR(
            nst_node_destroy(value);
            nst_token_destroy(op_tok);
            nst_node_destroy(op_node);
            nst_node_destroy(name););
        nst_llist_append(op_node->nodes, name, false);
        SET_ERROR_IF_OP_ERR(
            nst_node_destroy(value);
            nst_node_destroy(op_node);
            nst_node_destroy(name););
        nst_llist_append(op_node->nodes, value, true);
        SET_ERROR_IF_OP_ERR(
            nst_node_destroy(value);
            nst_node_destroy(op_node);
            nst_node_destroy(name););
        value = op_node;
    }
    else
    {
        nst_token_destroy(tok);
    }

    Nst_Pos start = value->start;
    Nst_Pos end = name->end;
    Nst_Node *assignment_node = nst_node_new_pos(
        NST_NT_ASSIGN_E,
        start, end);
    SET_ERROR_IF_OP_ERR(
        nst_node_destroy(value);
        nst_node_destroy(name););
    nst_llist_append(assignment_node->nodes, value, true);
    SET_ERROR_IF_OP_ERR(
        nst_node_destroy(value);
        nst_node_destroy(name);
        nst_node_destroy(assignment_node););
    SAFE_NODE_APPEND(assignment_node, name);
    DEC_RECURSION_LVL;
    return assignment_node;
}

static Nst_Node *parse_extraction()
{
    INC_RECURSION_LVL;
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
        RETURN_IF_ERROR(final_node);

        // makes `a.i` equivalent to `a.'i'`
        if ( treat_as_string )
        {
            atom->type = NST_NT_VALUE;
            NST_TOK(nst_llist_peek_front(atom->tokens))->type = NST_TT_VALUE;
        }

        Nst_Node *extraction_node = nst_node_new_pos(
            NST_NT_EXTRACT_E,
            final_node->start,
            atom->end);
        SET_ERROR_IF_OP_ERR(
            nst_node_destroy(final_node);
            nst_node_destroy(atom););
        nst_llist_append(extraction_node->nodes, final_node, true);
        SET_ERROR_IF_OP_ERR(
            nst_node_destroy(final_node);
            nst_node_destroy(atom);
            nst_node_destroy(extraction_node););
        SAFE_NODE_APPEND(extraction_node, atom);

        final_node = extraction_node;
    }
    DEC_RECURSION_LVL;
    return final_node;
}

static Nst_Node *parse_atom()
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_peek_front(tokens));

    if ( NST_IS_VALUE(tok->type) )
    {
        Nst_Node *value_node = nst_node_new_pos(
            tok->type == NST_TT_IDENT ? NST_NT_ACCESS : NST_NT_VALUE,
            tok->start, tok->end);
        SET_ERROR_IF_OP_ERR();
        nst_llist_pop(tokens);
        SAFE_TOK_APPEND(value_node, tok);
        DEC_RECURSION_LVL;
        return value_node;
    }
    else if ( tok->type == NST_TT_L_PAREN )
    {
        Nst_Pos start = tok->start;
        Nst_Pos err_end = tok->end;
        nst_token_destroy(tok);
        nst_llist_pop(tokens);
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
        DEC_RECURSION_LVL;
        return expr;
    }
    else if ( NST_IS_LOCAL_OP(tok->type) )
    {
        Nst_Node *local_op_node = nst_node_new(NST_NT_LOCAL_OP);
        SET_ERROR_IF_OP_ERR();
        nst_llist_pop(tokens);
        SAFE_TOK_APPEND(local_op_node, tok);

        Nst_Node *value = parse_extraction();
        RETURN_IF_ERROR(local_op_node);
        SAFE_NODE_APPEND(local_op_node, value);

        nst_node_set_pos(local_op_node, tok->start, value->end);
        DEC_RECURSION_LVL;
        return local_op_node;
    }
    else if ( tok->type == NST_TT_CALL )
    {
        Nst_LList *nodes = nst_llist_new();
        SET_ERROR_IF_OP_ERR();
        Nst_Node *call = parse_local_stack_op(nodes, tok->start);
        if ( call == NULL )
        {
            nst_llist_destroy(nodes, NULL);
        }
        DEC_RECURSION_LVL;
        return call;
    }
    else if ( tok->type == NST_TT_L_VBRACE )
    {
        DEC_RECURSION_LVL;
        return parse_vector_literal();
    }
    else if ( tok->type == NST_TT_L_BRACE )
    {
        DEC_RECURSION_LVL;
        return parse_arr_or_map_literal();
    }
    else if ( tok->type == NST_TT_LAMBDA )
    {
        DEC_RECURSION_LVL;
        return parse_func_def_or_lambda();
    }
    else
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        nst_token_destroy(tok);
        nst_llist_pop(tokens);
        RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_VALUE);
    }
}

static Nst_Node *parse_vector_literal()
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_Pos err_end = tok->end;
    nst_token_destroy(tok);
    Nst_Node *vect_node = nst_node_new(NST_NT_VEC_LIT);
    SET_ERROR_IF_OP_ERR();

    if ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_R_VBRACE )
    {
        tok = NST_TOK(nst_llist_pop(tokens));
        nst_token_destroy(tok);
        DEC_RECURSION_LVL;
        return vect_node;
    }

    while ( true )
    {
        Nst_Node *value = parse_expr(true);
        RETURN_IF_ERROR(vect_node);
        SAFE_NODE_APPEND(vect_node, value);

        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));

        if ( tok->type == NST_TT_BREAK && vect_node->nodes->size == 1 )
        {
            SAFE_TOK_APPEND(vect_node, tok);
            skip_blank();

            value = parse_expr(false);
            RETURN_IF_ERROR(vect_node);
            SAFE_NODE_APPEND(vect_node, value);

            skip_blank();
            tok = NST_TOK(nst_llist_pop(tokens));

            if ( tok->type != NST_TT_R_VBRACE )
            {
                nst_node_destroy(vect_node);
                Nst_Pos tok_start = tok->start;
                Nst_Pos tok_end = tok->end;
                nst_token_destroy(tok);
                RETURN_ERROR(tok_start, tok_end, _NST_EM_EXPECTED_VBRACE);
            }
            nst_token_destroy(tok);
            nst_node_set_pos(vect_node, start, tok->end);
            DEC_RECURSION_LVL;
            return vect_node;
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
        nst_node_destroy(vect_node);
        nst_token_destroy(tok);
        RETURN_ERROR(start, err_end, _NST_EM_MISSING_VBRACE);
    }

    nst_node_set_pos(vect_node, start, tok->end);
    nst_token_destroy(tok);
    DEC_RECURSION_LVL;
    return vect_node;
}

static Nst_Node *parse_arr_or_map_literal()
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    bool is_map = false;
    usize count = 0;

    nst_token_destroy(tok);
    skip_blank();

    Nst_Node *node = nst_node_new(NST_NT_MAP_LIT);
    SET_ERROR_IF_OP_ERR();

    if ( NST_TOK(nst_llist_peek_front(tokens))->type == NST_TT_R_BRACE )
    {
        tok = NST_TOK(nst_llist_pop(tokens));
        nst_node_set_pos(node, start, tok->end);
        nst_token_destroy(tok);
        DEC_RECURSION_LVL;
        return node;
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
            nst_node_destroy(node);
            RETURN_ERROR(start, end, _NST_EM_EXPECTED_BRACE);
        }
        node->type = NST_NT_ARR_LIT;
        nst_node_set_pos(node, start, tok->end);
        DEC_RECURSION_LVL;
        return node;
    }

    while ( true )
    {
        skip_blank();

        Nst_Node *value = parse_expr(true);
        RETURN_IF_ERROR(node);
        SAFE_NODE_APPEND(node, value);

        skip_blank();
        tok = NST_TOK(nst_llist_pop(tokens));

        if ( tok->type == NST_TT_COLON && (count == 0 || is_map) )
        {
            is_map = true;

            nst_token_destroy(tok);
            skip_blank();

            value = parse_expr(false);
            RETURN_IF_ERROR(node);
            SAFE_NODE_APPEND(node, value);

            skip_blank();
            tok = NST_TOK(nst_llist_pop(tokens));
        }
        else if ( tok->type == NST_TT_BREAK && count == 0 )
        {
            SAFE_TOK_APPEND(node, tok);
            skip_blank();

            value = parse_expr(false);
            RETURN_IF_ERROR(node);
            SAFE_NODE_APPEND(node, value);

            skip_blank();
            tok = NST_TOK(nst_llist_pop(tokens));

            if ( tok->type != NST_TT_R_BRACE )
            {
                nst_node_destroy(node);
                Nst_Pos err_start = tok->start;
                Nst_Pos err_end = tok->end;
                nst_token_destroy(tok);
                RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_BRACE);
            }
            nst_node_set_pos(node, start, tok->end);
            nst_token_destroy(tok);
            node->type = NST_NT_ARR_LIT;
            DEC_RECURSION_LVL;
            return node;
        }
        // if it's not the first iteration and it's not supposed to be a map
        else if ( tok->type == NST_TT_COLON )
        {
            nst_node_destroy(node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_token_destroy(tok);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_COMMA_OR_BRACE);
        }
        else if ( tok->type != NST_TT_COLON && is_map )
        {
            nst_node_destroy(node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            nst_token_destroy(tok);
            RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_COLON);
        }

        if ( tok->type == NST_TT_R_BRACE )
        {
            if ( !is_map )
            {
                node->type = NST_NT_ARR_LIT;
            }
            nst_node_set_pos(node, start, tok->end);
            nst_token_destroy(tok);
            DEC_RECURSION_LVL;
            return node;
        }
        else if ( tok->type != NST_TT_COMMA )
        {
            nst_node_destroy(node);
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
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    nst_token_destroy(tok);
    skip_blank();

    Nst_Node *try_catch_node = nst_node_new(NST_NT_TRY_CATCH_S);
    SET_ERROR_IF_OP_ERR();

    Nst_Node *try_block = parse_statement();
    RETURN_IF_ERROR(try_catch_node);
    SAFE_NODE_APPEND(try_catch_node, try_block);
    skip_blank();

    tok = NST_TOK(nst_llist_pop(tokens));
    if ( tok->type != NST_TT_CATCH )
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        nst_token_destroy(tok);
        nst_node_destroy(try_catch_node);
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
        nst_node_destroy(try_catch_node);
        RETURN_ERROR(err_start, err_end, _NST_EM_EXPECTED_IDENT);
    }
    SAFE_TOK_APPEND(try_catch_node, name_tok);
    skip_blank();

    Nst_Node *catch_block = parse_statement();
    RETURN_IF_ERROR(try_catch_node);
    SAFE_NODE_APPEND(try_catch_node, catch_block);

    nst_node_set_pos(try_catch_node, start, catch_block->end);
    DEC_RECURSION_LVL;
    return try_catch_node;
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
    bool *last = (bool *)nst_raw_malloc(sizeof(bool));
    if ( last == NULL )
    {
        return;
    }
    nst_llist_append(is_last, last, true);
    if ( prev_tail == is_last->tail )
    {
        nst_error_clear();
        nst_free(last);
        return;
    }

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

    nst_free(last);
    nst_free(is_last->tail);
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
    Nst_LList *is_last = nst_llist_new(NULL);
    if ( is_last == NULL )
    {
        return;
    }

    _print_ast(ast, NULL, 0, is_last);
    nst_llist_destroy(is_last, NULL);
}
