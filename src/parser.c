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
    _Nst_SET_RAW_SYNTAX_ERROR(p_state.error, start, end, message); \
    return NULL; \
    } while ( 0 )

// Sets an error if cond is true or the error's name is not NULL
// SET_ERROR_IF_OP_ERR(0) will set the error if it occurred
#define SET_ERROR_IF_OP_ERR(...) do { \
    if ( Nst_error_occurred() ) \
    { \
        Nst_Tok *_t_ = Nst_TOK(Nst_llist_peek_front(tokens)); \
        _Nst_SET_ERROR_FROM_OP_ERR(p_state.error, _t_->start, _t_->end); \
        __VA_ARGS__ \
        return NULL; \
    } } while ( 0 )

#define SAFE_NODE_APPEND(node, node_to_append) \
    Nst_llist_append(node->nodes, node_to_append, true); \
    SET_ERROR_IF_OP_ERR(Nst_node_destroy(node);Nst_node_destroy(node_to_append);)

#define SAFE_TOK_APPEND(node, tok_to_append) \
    Nst_llist_append(node->tokens, tok_to_append, true); \
    SET_ERROR_IF_OP_ERR(Nst_node_destroy(node);Nst_token_destroy(tok_to_append);)

#define RETURN_IF_ERROR(node) \
    if ( p_state.error->occurred ) { \
        Nst_node_destroy(node); \
        return NULL; \
    }

#define INC_RECURSION_LVL do { \
    recursion_lvl++; \
    if ( recursion_lvl > 1500 ) \
    { \
        Nst_Tok *_t_ = Nst_TOK(Nst_llist_peek_front(tokens)); \
        _Nst_SET_RAW_MEMORY_ERROR( \
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

static inline void skip_blank(void);
static Nst_Tok *copy_token(Nst_Tok *tok);
static Nst_Node *fix_expr(Nst_Node *expr);

static Nst_Node *parse_long_statement(void);
static Nst_Node *parse_statement(void);
static Nst_Node *parse_while_loop(void);
static Nst_Node *parse_for_loop(void);
static Nst_Node *parse_if_expr(Nst_Node *condition);
static Nst_Node *parse_switch_statement(void);
static Nst_Node *parse_func_def_or_lambda(void);
static Nst_Node *parse_expr(bool break_as_end);
static Nst_Node *parse_stack_op(Nst_Node *value, Nst_Pos start);
static Nst_Node *parse_local_stack_op(Nst_LList *nodes, Nst_Pos start);
static Nst_Node *parse_assignment_name(bool is_compound);
static Nst_Node *parse_assignment(Nst_Node *value);
static Nst_Node *parse_extraction(void);
static Nst_Node *parse_atom(void);
static Nst_Node *parse_vector_literal(void);
static Nst_Node *parse_arr_or_map_literal(void);
static Nst_Node *parse_try_catch(void);

Nst_Node *Nst_parse(Nst_LList *tokens_list, Nst_Error *error)
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

    // i.e. there are tokens other than Nst_TT_EOFILE
    if ( !p_state.error->occurred && tokens->size > 1 )
    {
        Nst_Pos start = Nst_TOK(Nst_llist_peek_front(tokens))->start;
        Nst_Pos end = Nst_TOK(Nst_llist_peek_front(tokens))->start;

        _Nst_SET_RAW_SYNTAX_ERROR(error, start, end, _Nst_EM_UNEXPECTED_TOK);
    }

    Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_token_destroy);
    return node; // NULL if there was an error
}

static inline void skip_blank(void)
{
    while ( Nst_TOK(Nst_llist_peek_front(tokens))->type == Nst_TT_ENDL )
    {
        Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
    }
}

static Nst_Tok *copy_token(Nst_Tok *tok)
{
    return Nst_tok_new_value(
        tok->start,
        tok->end,
        tok->type,
        tok->value != NULL ? Nst_inc_ref(tok->value) : NULL);
}

static Nst_Node *parse_long_statement(void)
{
    INC_RECURSION_LVL;
    Nst_Node *long_statement_node = Nst_node_new(Nst_NT_LONG_S);
    SET_ERROR_IF_OP_ERR();

    Nst_Node *node = NULL;
    Nst_LList *nodes = long_statement_node->nodes;
    skip_blank();

    while ( Nst_TOK(Nst_llist_peek_front(tokens))->type != Nst_TT_R_BRACKET &&
            Nst_TOK(Nst_llist_peek_front(tokens))->type != Nst_TT_EOFILE       )
    {
        node = parse_statement();
        RETURN_IF_ERROR(long_statement_node);
        SAFE_NODE_APPEND(long_statement_node, node);
        skip_blank();
    }

    if ( nodes->size == 0 )
        Nst_node_set_pos(
            long_statement_node,
            Nst_TOK(Nst_llist_peek_front(tokens))->start,
            Nst_TOK(Nst_llist_peek_front(tokens))->end);
    else
        Nst_node_set_pos(
            long_statement_node,
            Nst_NODE(nodes->head->value)->start,
            Nst_NODE(nodes->tail->value)->end);

    DEC_RECURSION_LVL;
    return long_statement_node;
}

static Nst_Node *parse_statement(void)
{
    INC_RECURSION_LVL;
    skip_blank();
    i32 tok_type = Nst_TOK(Nst_llist_peek_front(tokens))->type;

    if ( tok_type == Nst_TT_L_BRACKET )
    {
        Nst_Tok *open_bracket = Nst_TOK(Nst_llist_pop(tokens));
        Nst_Pos start = open_bracket->start;
        Nst_token_destroy(open_bracket);

        Nst_Node *node = parse_long_statement();
        if ( p_state.error->occurred )
        {
            return NULL;
        }

        Nst_Tok *close_bracket = Nst_TOK(Nst_llist_pop(tokens));
        if ( close_bracket->type != Nst_TT_R_BRACKET )
        {
            Nst_token_destroy(close_bracket);
            Nst_node_destroy(node);
            RETURN_ERROR(start, start, _Nst_EM_MISSING_BRACKET);
        }
        Nst_token_destroy(close_bracket);
        DEC_RECURSION_LVL;
        return node;
    }
    else if ( tok_type == Nst_TT_WHILE || tok_type == Nst_TT_DOWHILE )
    {
        DEC_RECURSION_LVL;
        return parse_while_loop();
    }
    else if ( tok_type == Nst_TT_FOR )
    {
        DEC_RECURSION_LVL;
        return parse_for_loop();
    }
    else if ( tok_type == Nst_TT_FUNC )
    {
        DEC_RECURSION_LVL;
        return parse_func_def_or_lambda();
    }
    else if ( tok_type == Nst_TT_SWITCH )
    {
        DEC_RECURSION_LVL;
        return parse_switch_statement();
    }
    else if ( tok_type == Nst_TT_RETURN )
    {
        Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        Nst_Node *expr;
        Nst_token_destroy(tok);

        if ( !p_state.in_func )
        {
            RETURN_ERROR(start, end, _Nst_EM_BAD_RETURN);
        }

        tok_type = Nst_TOK(Nst_llist_peek_front(tokens))->type;
        if ( Nst_IS_EXPR_END(tok_type) )
        {
            Nst_Tok *null_value = Nst_tok_new_value(
                start, end,
                Nst_TT_VALUE,
                Nst_inc_ref(Nst_c.Null_null));
            SET_ERROR_IF_OP_ERR();
            expr = Nst_node_new_pos(
                Nst_NT_VALUE,
                start,
                end);
            SET_ERROR_IF_OP_ERR(Nst_token_destroy(null_value););
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

        Nst_Node *return_s_node = Nst_node_new_pos(
            Nst_NT_RETURN_S,
            start, expr->end);
        SET_ERROR_IF_OP_ERR(Nst_node_destroy(expr););
        SAFE_NODE_APPEND(return_s_node, expr);
        DEC_RECURSION_LVL;
        return return_s_node;
    }
    else if ( tok_type == Nst_TT_CONTINUE )
    {
        Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        Nst_token_destroy(tok);

        if ( !p_state.in_loop && !p_state.in_switch )
        {
            RETURN_ERROR(start, end, _Nst_EM_BAD_CONTINUE);
        }

        Nst_Node *continue_node = Nst_node_new_pos(
            Nst_NT_CONTINUE_S,
            start, end);
        SET_ERROR_IF_OP_ERR();
        DEC_RECURSION_LVL;
        return continue_node;
    }
    else if ( tok_type == Nst_TT_BREAK )
    {
        Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        Nst_token_destroy(tok);

        if ( !p_state.in_loop )
        {
            RETURN_ERROR(start, end, _Nst_EM_BAD_BREAK);
        }
        Nst_Node *break_node = Nst_node_new_pos(
            Nst_NT_BREAK_S,
            start, end);
        SET_ERROR_IF_OP_ERR();
        DEC_RECURSION_LVL;
        return break_node;
    }
    else if ( tok_type == Nst_TT_TRY )
    {
        DEC_RECURSION_LVL;
        return parse_try_catch();
    }
    else if ( Nst_IS_ATOM(tok_type) || Nst_IS_LOCAL_STACK_OP(tok_type) )
    {
        DEC_RECURSION_LVL;
        return parse_expr(false);
    }
    else
    {
        Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;

        Nst_token_destroy(tok);
        RETURN_ERROR(start, end, _Nst_EM_UNEXPECTED_TOK);
    }
}

static Nst_Node *parse_while_loop(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
    Nst_Pos start = tok->start;

    Nst_Node *while_l_node = Nst_node_new(
        tok->type == Nst_TT_WHILE ? Nst_NT_WHILE_L : Nst_NT_DOWHILE_L);
    SET_ERROR_IF_OP_ERR();

    Nst_token_destroy(tok);
    skip_blank();

    Nst_Node *condition = parse_expr(false);
    RETURN_IF_ERROR(while_l_node);
    SAFE_NODE_APPEND(while_l_node, condition);
    skip_blank();

    Nst_Pos err_pos = Nst_TOK(Nst_llist_peek_front(tokens))->start;

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type != Nst_TT_L_BRACKET )
    {
        Nst_node_destroy(while_l_node);
        RETURN_ERROR(err_pos, err_pos, _Nst_EM_EXPECTED_BRACKET);
    }

    Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;

    Nst_Node *body = parse_long_statement();
    RETURN_IF_ERROR(while_l_node);
    SAFE_NODE_APPEND(while_l_node, body);
    p_state.in_loop = prev_state;

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type != Nst_TT_R_BRACKET )
    {
        Nst_node_destroy(while_l_node);
        RETURN_ERROR(err_pos, err_pos, _Nst_EM_MISSING_BRACKET);
    }

    tok = Nst_TOK(Nst_llist_pop(tokens));
    Nst_Pos end = tok->end;
    Nst_token_destroy(tok);

    Nst_node_set_pos(while_l_node, start, end);
    DEC_RECURSION_LVL;
    return while_l_node;
}

static Nst_Node *parse_for_loop(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_token_destroy(tok);

    Nst_Node *for_l_node = Nst_node_new(Nst_NT_FOR_L);
    SET_ERROR_IF_OP_ERR();

    skip_blank();
    Nst_Node *range = parse_expr(false);
    RETURN_IF_ERROR(for_l_node);

    Nst_llist_append(for_l_node->nodes, range, true);
    SET_ERROR_IF_OP_ERR(Nst_node_destroy(for_l_node); Nst_node_destroy(range););

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type == Nst_TT_AS )
    {
        Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
        Nst_Node *name_node = parse_assignment_name(false);
        RETURN_IF_ERROR(for_l_node);
        SAFE_NODE_APPEND(for_l_node, name_node);
        for_l_node->type = Nst_NT_FOR_AS_L;
    }

    skip_blank();

    Nst_Pos err_pos = Nst_TOK(Nst_llist_peek_front(tokens))->start;

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type != Nst_TT_L_BRACKET )
    {
        Nst_node_destroy(for_l_node);
        RETURN_ERROR(err_pos, err_pos, _Nst_EM_EXPECTED_BRACKET);
    }

    Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;
    Nst_Node *body = parse_long_statement();
    RETURN_IF_ERROR(for_l_node);
    SAFE_NODE_APPEND(for_l_node, body);
    p_state.in_loop = prev_state;

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type != Nst_TT_R_BRACKET )
    {
        Nst_node_destroy(for_l_node);
        RETURN_ERROR(err_pos, err_pos, _Nst_EM_MISSING_BRACKET);
    }

    tok = Nst_TOK(Nst_llist_pop(tokens));
    Nst_Pos end = tok->end;
    Nst_token_destroy(tok);

    Nst_node_set_pos(for_l_node, start, end);
    DEC_RECURSION_LVL;
    return for_l_node;
}

static Nst_Node *parse_if_expr(Nst_Node *condition)
{
    INC_RECURSION_LVL;
    Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
    skip_blank();

    Nst_Node *if_expr_node = Nst_node_new(Nst_NT_IF_E);
    SET_ERROR_IF_OP_ERR();
    SAFE_NODE_APPEND(if_expr_node, condition);

    Nst_Node *body_if_true = parse_statement();
    RETURN_IF_ERROR(if_expr_node);
    SAFE_NODE_APPEND(if_expr_node, body_if_true);
    skip_blank();

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type != Nst_TT_COLON )
    {
        Nst_node_set_pos(if_expr_node, condition->start, body_if_true->end);
        DEC_RECURSION_LVL;
        return if_expr_node;
    }

    Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
    skip_blank();

    Nst_Node *body_if_false = parse_statement();
    RETURN_IF_ERROR(if_expr_node);
    SAFE_NODE_APPEND(if_expr_node, body_if_false);

    Nst_node_set_pos(if_expr_node, condition->start, body_if_false->end);
    DEC_RECURSION_LVL;
    return if_expr_node;
}

static Nst_Node *parse_switch_statement(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_Pos err_start;
    Nst_Pos err_end;
    Nst_token_destroy(tok);
    skip_blank();

    Nst_Node *switch_s_node = Nst_node_new(Nst_NT_SWITCH_S);
    SET_ERROR_IF_OP_ERR();

    Nst_Node *main_val = parse_expr(false);
    RETURN_IF_ERROR(switch_s_node);
    SAFE_NODE_APPEND(switch_s_node, main_val);

    skip_blank();
    tok = Nst_TOK(Nst_llist_pop(tokens));

    if ( tok->type != Nst_TT_L_BRACKET )
    {
        err_start = tok->start;
        err_end = tok->end;
        Nst_token_destroy(tok);
        Nst_node_destroy(switch_s_node);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_BRACKET);
    }
    Nst_token_destroy(tok);

    bool is_default_case = false;
    bool prev_in_switch = p_state.in_switch;
    p_state.in_switch = true;

    while ( true )
    {
        skip_blank();
        tok = Nst_TOK(Nst_llist_pop(tokens));
        if ( tok->type != Nst_TT_IF )
        {
            err_start = tok->start;
            err_end = tok->end;
            Nst_token_destroy(tok);
            Nst_node_destroy(switch_s_node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_IF);
        }
        Nst_token_destroy(tok);
        skip_blank();
        tok = Nst_TOK(Nst_llist_peek_front(tokens));

        if ( tok->type == Nst_TT_L_BRACKET )
        {
            is_default_case = true;
            err_start = tok->start;
            err_end = tok->end;
            Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
        }
        else
        {
            Nst_Node *val = parse_expr(false);
            RETURN_IF_ERROR(switch_s_node);
            SAFE_NODE_APPEND(switch_s_node, val);
            skip_blank();
            tok = Nst_TOK(Nst_llist_pop(tokens));
            err_start = tok->start;
            err_end = tok->end;

            if ( tok->type != Nst_TT_L_BRACKET )
            {
                Nst_token_destroy(tok);
                Nst_node_destroy(switch_s_node);
                RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_BRACKET);
            }
            Nst_token_destroy(tok);
        }

        Nst_Node *body = parse_long_statement();
        RETURN_IF_ERROR(switch_s_node);
        SAFE_NODE_APPEND(switch_s_node, body);
        skip_blank();
        tok = Nst_TOK(Nst_llist_pop(tokens));
        if ( tok->type != Nst_TT_R_BRACKET )
        {
            err_start = tok->start;
            err_end = tok->end;
            Nst_token_destroy(tok);
            Nst_node_destroy(switch_s_node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_R_BRACKET);
        }
        Nst_token_destroy(tok);
        skip_blank();
        tok = Nst_TOK(Nst_llist_peek_front(tokens));

        if ( tok->type == Nst_TT_R_BRACKET )
        {
            Nst_Pos end = tok->end;
            Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
            p_state.in_switch = prev_in_switch;
            Nst_node_set_pos(switch_s_node, start, end);
            DEC_RECURSION_LVL;
            return switch_s_node;
        }
        else if ( is_default_case )
        {
            err_start = tok->start;
            err_end = tok->end;
            Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
            Nst_node_destroy(switch_s_node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_R_BRACKET);
        }
    }
}

static Nst_Node *parse_func_def_or_lambda(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_Pos end;
    bool is_lambda = tok->type == Nst_TT_LAMBDA;
    Nst_token_destroy(tok);

    Nst_Node *func_node = Nst_node_new(
        is_lambda ? Nst_NT_LAMBDA : Nst_NT_FUNC_DECLR);

    skip_blank();
    while ( Nst_TOK(Nst_llist_peek_front(tokens))->type == Nst_TT_IDENT )
    {
        tok = Nst_TOK(Nst_llist_pop(tokens));
        SAFE_TOK_APPEND(func_node, tok);
        skip_blank();
    }

    Nst_Pos err_start = Nst_TOK(Nst_llist_peek_front(tokens))->start;
    Nst_Pos err_end = Nst_TOK(Nst_llist_peek_front(tokens))->end;

    // if there are no identifiers after #
    if ( func_node->tokens->size == 0 && !is_lambda )
    {
        Nst_node_destroy(func_node);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_IDENT);
    }

    bool expects_r_bracket = false;
    Nst_Pos return_start = Nst_TOK(Nst_llist_peek_front(tokens))->start;

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type == Nst_TT_L_BRACKET )
    {
        expects_r_bracket = true;
    }
    else if ( Nst_TOK(Nst_llist_peek_front(tokens))->type != Nst_TT_RETURN )
    {
        Nst_Pos tok_end = Nst_TOK(Nst_llist_peek_front(tokens))->start;
        Nst_node_destroy(func_node);
        RETURN_ERROR(
            return_start,
            tok_end,
            _Nst_EM_EXPECTED_RETURN_OR_BRACKET);
    }

    Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));

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
         Nst_TOK(Nst_llist_peek_front(tokens))->type != Nst_TT_R_BRACKET )
    {
        Nst_node_destroy(func_node);
        Nst_node_destroy(body);
        RETURN_ERROR(err_start, err_end, _Nst_EM_MISSING_BRACKET);
    }
    else if ( expects_r_bracket )
    {
        tok = Nst_TOK(Nst_llist_pop(tokens));
        end = tok->end;
        Nst_token_destroy(tok);
    }
    else
    {
        Nst_Node *return_node = Nst_node_new_pos(
            Nst_NT_RETURN_S,
            return_start,
            body->end);

        Nst_llist_append(return_node->nodes, body, true);
        SET_ERROR_IF_OP_ERR(
            Nst_node_destroy(func_node);
        Nst_node_destroy(body);
        Nst_node_destroy(return_node););
        body = return_node;
        end = return_node->end;
    }

    SAFE_NODE_APPEND(func_node, body);
    Nst_node_set_pos(func_node, start, end);
    DEC_RECURSION_LVL;
    return func_node;
}

static Nst_Node *parse_expr(bool break_as_end)
{
    INC_RECURSION_LVL;
    Nst_Node *node = NULL;
    Nst_Pos start = Nst_TOK(Nst_llist_peek_front(tokens))->start;
    i32 token_type = Nst_TOK(Nst_llist_peek_front(tokens))->type;

    if ( break_as_end )
    {
        while (!Nst_IS_EXPR_END_W_BREAK(token_type))
        {
            node = parse_stack_op(node, start);
            if ( p_state.error->occurred )
            {
                return NULL;
            }
            token_type = Nst_TOK(Nst_llist_peek_front(tokens))->type;
        }
    }
    else
    {
        while (!Nst_IS_EXPR_END(token_type))
        {
            node = parse_stack_op(node, start);
            if ( p_state.error->occurred )
            {
                return NULL;
            }
            token_type = Nst_TOK(Nst_llist_peek_front(tokens))->type;
        }
    }

    if ( node == NULL )
    {
        Nst_Tok *tok = Nst_TOK(Nst_llist_peek_front(tokens));
        RETURN_ERROR(tok->start, tok->end, _Nst_EM_EXPECTED_VALUE);
    }

    node = fix_expr(node);
    if ( node == NULL )
    {
        return NULL;
    }

    token_type = Nst_TOK(Nst_llist_peek_front(tokens))->type;

    if ( token_type == Nst_TT_IF )
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
    if ( expr->type != Nst_NT_STACK_OP &&
         expr->type != Nst_NT_LOCAL_STACK_OP &&
         expr->type != Nst_NT_ASSIGN_E )
    {
        DEC_RECURSION_LVL;
        return expr;
    }

    else if ( expr->type == Nst_NT_STACK_OP && expr->nodes->size == 1 )
    {
        Nst_Node *new_node = Nst_NODE(Nst_llist_peek_front(expr->nodes));
        Nst_llist_empty(expr->tokens, (Nst_LListDestructor)Nst_token_destroy);
        Nst_llist_empty(expr->nodes, NULL);
        Nst_free(expr->tokens);
        Nst_free(expr);
        DEC_RECURSION_LVL;
        return fix_expr(new_node);
    }

    for ( Nst_LLIST_ITER(cursor, expr->nodes) )
    {
        cursor->value = fix_expr(Nst_NODE(cursor->value));
    }

    if ( expr->type != Nst_NT_STACK_OP )
    {
        DEC_RECURSION_LVL;
        return expr;
    }

    Nst_Node *curr_node = expr;
    Nst_Tok *op_tok = Nst_TOK(Nst_llist_peek_front(expr->tokens));

    // comparisons are handled differently to not copy nodes
    if ( Nst_IS_COMP_OP(op_tok->type) )
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
        Nst_Node *new_node = Nst_node_new_pos(
            Nst_NT_STACK_OP,
            start, end);

        // move the nodes except for the last one
        for ( usize j = 0, m = curr_node->nodes->size - 1; j < m; j++ )
        {
            Nst_llist_append_llnode(
                new_node->nodes,
                Nst_llist_pop_llnode(curr_node->nodes));
        }

        Nst_Tok *new_tok = copy_token(op_tok);
        SET_ERROR_IF_OP_ERR(
                Nst_node_destroy(expr);
                Nst_node_destroy(new_node););
        Nst_llist_append(new_node->tokens, new_tok, true);
        SET_ERROR_IF_OP_ERR(
                Nst_node_destroy(expr);
                Nst_node_destroy(new_node);
                Nst_token_destroy(new_tok););

        Nst_llist_push(curr_node->nodes, new_node, true);
        SET_ERROR_IF_OP_ERR(Nst_node_destroy(expr););
        curr_node = new_node;
    }

    DEC_RECURSION_LVL;
    return expr;
}

static Nst_Node *parse_stack_op(Nst_Node *value, Nst_Pos start)
{
    INC_RECURSION_LVL;
    Nst_Node *value_node = NULL;
    Nst_LList *new_nodes = Nst_llist_new();
    SET_ERROR_IF_OP_ERR();

    if ( value != NULL )
    {
        Nst_llist_append(new_nodes, value, true);
        SET_ERROR_IF_OP_ERR(Nst_node_destroy(value););
    }
    else
    {
        value_node = parse_extraction();
        if ( p_state.error->occurred )
        {
            Nst_llist_destroy(new_nodes, NULL);
            return NULL;
        }
        Nst_llist_append(new_nodes, value_node, true);
        SET_ERROR_IF_OP_ERR(Nst_node_destroy(value_node););
    }

    while ( Nst_IS_ATOM(Nst_TOK(Nst_llist_peek_front(tokens))->type) )
    {
        value_node = parse_extraction();
        if ( p_state.error->occurred )
        {
            Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy);
            return NULL;
        }
        Nst_llist_append(new_nodes, value_node, true);
        SET_ERROR_IF_OP_ERR(Nst_node_destroy(value_node););
    }

    Nst_Node *node = NULL;
    Nst_Tok *op_tok = Nst_TOK(Nst_llist_peek_front(tokens));
    Nst_Pos end = op_tok->end;
    bool is_local_stack_op = false;

    if ( Nst_IS_STACK_OP(op_tok->type) )
    {
        node = Nst_node_new_pos(Nst_NT_STACK_OP, start, end);
        SET_ERROR_IF_OP_ERR(
            Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy););
        Nst_llist_move_nodes(new_nodes, node->nodes);
        Nst_free(new_nodes);
        Nst_llist_pop(tokens);
        SAFE_TOK_APPEND(node, op_tok);
    }
    else if ( Nst_IS_LOCAL_STACK_OP(op_tok->type) )
    {
        node = parse_local_stack_op(new_nodes, start);
        if ( p_state.error->occurred )
        {
            Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy);
            return NULL;
        }
        is_local_stack_op = true;
    }
    else if ( Nst_IS_ASSIGNMENT(op_tok->type) && op_tok->type != Nst_TT_ASSIGN )
    {
        Nst_Tok *new_tok = Nst_tok_new_noval(
            op_tok->start,
            op_tok->end,
            Nst_ASSIGMENT_TO_STACK_OP(op_tok->type));
        SET_ERROR_IF_OP_ERR(
            Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy););
        node = Nst_node_new_pos(
            Nst_NT_STACK_OP,
            start, end);
        SET_ERROR_IF_OP_ERR(
            Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy);
            Nst_token_destroy(new_tok););
        Nst_llist_move_nodes(new_nodes, node->nodes);
        Nst_free(new_nodes);
        SAFE_TOK_APPEND(node, new_tok);
    }
    else if ( new_nodes->size == 1 && value == NULL )
    {
        node = Nst_NODE(Nst_llist_pop(new_nodes));
        Nst_llist_destroy(new_nodes, NULL);
    }
    else
    {
        Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy);
        RETURN_ERROR(op_tok->start, end, _Nst_EM_EXPECTED_OP);
    }

    while ( !Nst_IS_EXPR_END(Nst_TOK(Nst_llist_peek_front(tokens))->type) )
    {
        op_tok = Nst_TOK(Nst_llist_peek_front(tokens));
        if ( Nst_IS_STACK_OP(op_tok->type) && !is_local_stack_op )
        {
            Nst_Node *new_node = Nst_node_new_pos(
                Nst_NT_STACK_OP,
                start, op_tok->end);
            SET_ERROR_IF_OP_ERR(Nst_node_destroy(node););
            SAFE_NODE_APPEND(new_node, node);
            op_tok = Nst_TOK(Nst_llist_pop(tokens));
            SAFE_TOK_APPEND(new_node, op_tok);
        }
        else if ( Nst_IS_LOCAL_STACK_OP(op_tok->type) && is_local_stack_op )
        {
            new_nodes = Nst_llist_new();
            SET_ERROR_IF_OP_ERR(Nst_node_destroy(node););
            Nst_llist_append(new_nodes, node, true);
            SET_ERROR_IF_OP_ERR(Nst_node_destroy(node););
            node = parse_local_stack_op(new_nodes, start);
            if ( p_state.error->occurred )
            {
                Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy);
                return NULL;
            }
        }
        else if ( Nst_IS_ASSIGNMENT(op_tok->type) )
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
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));

    if ( tok->type == Nst_TT_CAST && nodes->size != 1 )
    {
        Nst_token_destroy(tok);
        RETURN_ERROR(
            Nst_NODE(Nst_llist_peek_front(nodes))->start,
            Nst_NODE(Nst_llist_peek_back(nodes))->end,
            _Nst_EM_LEFT_ARGS_NUM("::", "1", ""));
    }
    else if ( tok->type == Nst_TT_RANGE &&
              nodes->size != 1          &&
              nodes->size != 2 )
    {
        Nst_token_destroy(tok);
        RETURN_ERROR(
            Nst_NODE(Nst_llist_peek_front(nodes))->start,
            Nst_NODE(Nst_llist_peek_back(nodes))->end,
            _Nst_EM_LEFT_ARGS_NUM("->", "1 or 2", "s"));
    }
    else if ( tok->type == Nst_TT_THROW && nodes->size != 1 )
    {
        Nst_token_destroy(tok);
        RETURN_ERROR(
            Nst_NODE(Nst_llist_peek_front(nodes))->start,
            Nst_NODE(Nst_llist_peek_back(nodes))->end,
            _Nst_EM_LEFT_ARGS_NUM("!!", "1", ""));
    }
    else if ( tok->type == Nst_TT_SEQ_CALL && nodes->size != 1 )
    {
        Nst_token_destroy(tok);
        RETURN_ERROR(
            Nst_NODE(Nst_llist_peek_front(nodes))->start,
            Nst_NODE(Nst_llist_peek_back(nodes))->end,
            _Nst_EM_LEFT_ARGS_NUM("*@", "1", ""));
    }

    Nst_Node *local_stack_op_node = Nst_node_new(Nst_NT_LOCAL_STACK_OP);
    SAFE_TOK_APPEND(local_stack_op_node, tok);
    Nst_llist_move_nodes(nodes, local_stack_op_node->nodes);

    Nst_Node *special_node = parse_extraction();
    RETURN_IF_ERROR(local_stack_op_node);
    SAFE_NODE_APPEND(local_stack_op_node, special_node);

    Nst_free(nodes);
    Nst_node_set_pos(local_stack_op_node, start, special_node->end);
    DEC_RECURSION_LVL;
    return local_stack_op_node;
}

static Nst_Node *parse_assignment_name(bool is_compound)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_peek_front(tokens));
    Nst_Pos start = tok->start;
    Nst_Node *node;

    if ( tok->type != Nst_TT_L_BRACE )
    {
        node = parse_extraction();

        if ( p_state.error->occurred )
        {
            return NULL;
        }

        if ( node->type != Nst_NT_ACCESS && node->type != Nst_NT_EXTRACT_E )
        {
            Nst_Pos err_start = node->start;
            Nst_Pos err_end = node->end;
            Nst_node_destroy(node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_IDENT_OR_EXTR);
        }
        DEC_RECURSION_LVL;
        return node;
    }

    tok = Nst_TOK(Nst_llist_pop(tokens));

    if ( is_compound )
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        Nst_token_destroy(tok);
        RETURN_ERROR(err_start, err_end, _Nst_EM_COMPOUND_ASSIGMENT);
    }

    node = Nst_node_new(Nst_NT_ARR_LIT);
    SET_ERROR_IF_OP_ERR();

    while ( true )
    {
        Nst_token_destroy(tok);
        skip_blank();
        Nst_Node *name_node = parse_assignment_name(false);
        RETURN_IF_ERROR(node);
        SAFE_NODE_APPEND(node, name_node);

        skip_blank();
        tok = Nst_TOK(Nst_llist_pop(tokens));
        if ( tok->type == Nst_TT_COMMA )
        {
            continue;
        }
        else if ( tok->type == Nst_TT_R_BRACE )
        {
            Nst_node_set_pos(node, start, tok->end);
            Nst_token_destroy(tok);
            DEC_RECURSION_LVL;
            return node;
        }
        else
        {
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            Nst_token_destroy(tok);
            Nst_node_destroy(node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_COMMA_OR_BRACE);
        }
    }
}

static Nst_Node *parse_assignment(Nst_Node *value)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
    bool is_compound = tok->type != Nst_TT_ASSIGN;
    Nst_Node *name = parse_assignment_name(is_compound);
    if ( p_state.error->occurred )
    {
        Nst_token_destroy(tok);
        Nst_node_destroy(value);
        return NULL;
    }

    // If a compound assignmen operator such as '+=' or '*='
    if ( is_compound )
    {
        Nst_Tok *op_tok = Nst_tok_new_noval(
            tok->start,
            tok->end,
            Nst_ASSIGMENT_TO_STACK_OP(tok->type));
        Nst_token_destroy(tok);
        SET_ERROR_IF_OP_ERR(
            Nst_node_destroy(value);
            Nst_node_destroy(name););
        Nst_Node *op_node = Nst_node_new_pos(
            Nst_NT_STACK_OP,
            value->start,
            name->end);
        SET_ERROR_IF_OP_ERR(
            Nst_node_destroy(value);
            Nst_token_destroy(op_tok);
            Nst_node_destroy(name););

        Nst_llist_append(op_node->tokens, op_tok, true);
        SET_ERROR_IF_OP_ERR(
            Nst_node_destroy(value);
            Nst_token_destroy(op_tok);
            Nst_node_destroy(op_node);
            Nst_node_destroy(name););
        Nst_llist_append(op_node->nodes, name, false);
        SET_ERROR_IF_OP_ERR(
            Nst_node_destroy(value);
            Nst_node_destroy(op_node);
            Nst_node_destroy(name););
        Nst_llist_append(op_node->nodes, value, true);
        SET_ERROR_IF_OP_ERR(
            Nst_node_destroy(value);
            Nst_node_destroy(op_node);
            Nst_node_destroy(name););
        value = op_node;
    }
    else
    {
        Nst_token_destroy(tok);
    }

    Nst_Pos start = value->start;
    Nst_Pos end = name->end;
    Nst_Node *assignment_node = Nst_node_new_pos(
        Nst_NT_ASSIGN_E,
        start, end);
    SET_ERROR_IF_OP_ERR(
        Nst_node_destroy(value);
        Nst_node_destroy(name););
    Nst_llist_append(assignment_node->nodes, value, true);
    SET_ERROR_IF_OP_ERR(
        Nst_node_destroy(value);
        Nst_node_destroy(name);
        Nst_node_destroy(assignment_node););
    SAFE_NODE_APPEND(assignment_node, name);
    DEC_RECURSION_LVL;
    return assignment_node;
}

static Nst_Node *parse_extraction(void)
{
    INC_RECURSION_LVL;
    Nst_Node *atom = parse_atom();
    if ( p_state.error->occurred )
    {
        return NULL;
    }

    Nst_Node *final_node = atom;

    while ( Nst_TOK(Nst_llist_peek_front(tokens))->type == Nst_TT_EXTRACT )
    {
        Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
        bool treat_as_string =
            Nst_TOK(Nst_llist_peek_front(tokens))->type == Nst_TT_IDENT;

        atom = parse_atom();
        RETURN_IF_ERROR(final_node);

        // makes `a.i` equivalent to `a.'i'`
        if ( treat_as_string )
        {
            atom->type = Nst_NT_VALUE;
            Nst_TOK(Nst_llist_peek_front(atom->tokens))->type = Nst_TT_VALUE;
        }

        Nst_Node *extraction_node = Nst_node_new_pos(
            Nst_NT_EXTRACT_E,
            final_node->start,
            atom->end);
        SET_ERROR_IF_OP_ERR(
            Nst_node_destroy(final_node);
            Nst_node_destroy(atom););
        Nst_llist_append(extraction_node->nodes, final_node, true);
        SET_ERROR_IF_OP_ERR(
            Nst_node_destroy(final_node);
            Nst_node_destroy(atom);
            Nst_node_destroy(extraction_node););
        SAFE_NODE_APPEND(extraction_node, atom);

        final_node = extraction_node;
    }
    DEC_RECURSION_LVL;
    return final_node;
}

static Nst_Node *parse_atom(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_peek_front(tokens));

    if ( Nst_IS_VALUE(tok->type) )
    {
        Nst_Node *value_node = Nst_node_new_pos(
            tok->type == Nst_TT_IDENT ? Nst_NT_ACCESS : Nst_NT_VALUE,
            tok->start, tok->end);
        SET_ERROR_IF_OP_ERR();
        Nst_llist_pop(tokens);
        SAFE_TOK_APPEND(value_node, tok);
        DEC_RECURSION_LVL;
        return value_node;
    }
    else if ( tok->type == Nst_TT_L_PAREN )
    {
        Nst_Pos start = tok->start;
        Nst_Pos err_end = tok->end;
        Nst_token_destroy(tok);
        Nst_llist_pop(tokens);
        Nst_Node *expr = parse_expr(false);
        if ( p_state.error->occurred )
        {
            return NULL;
        }

        tok = Nst_TOK(Nst_llist_pop(tokens));
        if ( tok->type != Nst_TT_R_PAREN )
        {
            Nst_node_destroy(expr);
            Nst_token_destroy(tok);
            RETURN_ERROR(start, err_end, _Nst_EM_MISSING_PAREN);
        }

        expr->start = start;
        expr->end = tok->end;

        Nst_token_destroy(tok);
        DEC_RECURSION_LVL;
        return expr;
    }
    else if ( Nst_IS_LOCAL_OP(tok->type) )
    {
        Nst_Node *local_op_node = Nst_node_new(Nst_NT_LOCAL_OP);
        SET_ERROR_IF_OP_ERR();
        Nst_llist_pop(tokens);
        SAFE_TOK_APPEND(local_op_node, tok);

        Nst_Node *value = parse_extraction();
        RETURN_IF_ERROR(local_op_node);
        SAFE_NODE_APPEND(local_op_node, value);

        Nst_node_set_pos(local_op_node, tok->start, value->end);
        DEC_RECURSION_LVL;
        return local_op_node;
    }
    else if ( tok->type == Nst_TT_CALL )
    {
        Nst_LList *nodes = Nst_llist_new();
        SET_ERROR_IF_OP_ERR();
        Nst_Node *call = parse_local_stack_op(nodes, tok->start);
        if ( call == NULL )
        {
            Nst_llist_destroy(nodes, NULL);
        }
        DEC_RECURSION_LVL;
        return call;
    }
    else if ( tok->type == Nst_TT_L_VBRACE )
    {
        DEC_RECURSION_LVL;
        return parse_vector_literal();
    }
    else if ( tok->type == Nst_TT_L_BRACE )
    {
        DEC_RECURSION_LVL;
        return parse_arr_or_map_literal();
    }
    else if ( tok->type == Nst_TT_LAMBDA )
    {
        DEC_RECURSION_LVL;
        return parse_func_def_or_lambda();
    }
    else
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        Nst_token_destroy(tok);
        Nst_llist_pop(tokens);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_VALUE);
    }
}

static Nst_Node *parse_vector_literal(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_Pos err_end = tok->end;
    Nst_token_destroy(tok);
    Nst_Node *vect_node = Nst_node_new(Nst_NT_VEC_LIT);
    SET_ERROR_IF_OP_ERR();

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type == Nst_TT_R_VBRACE )
    {
        tok = Nst_TOK(Nst_llist_pop(tokens));
        Nst_token_destroy(tok);
        DEC_RECURSION_LVL;
        return vect_node;
    }

    while ( true )
    {
        Nst_Node *value = parse_expr(true);
        RETURN_IF_ERROR(vect_node);
        SAFE_NODE_APPEND(vect_node, value);

        skip_blank();
        tok = Nst_TOK(Nst_llist_pop(tokens));

        if ( tok->type == Nst_TT_BREAK && vect_node->nodes->size == 1 )
        {
            SAFE_TOK_APPEND(vect_node, tok);
            skip_blank();

            value = parse_expr(false);
            RETURN_IF_ERROR(vect_node);
            SAFE_NODE_APPEND(vect_node, value);

            skip_blank();
            tok = Nst_TOK(Nst_llist_pop(tokens));

            if ( tok->type != Nst_TT_R_VBRACE )
            {
                Nst_node_destroy(vect_node);
                Nst_Pos tok_start = tok->start;
                Nst_Pos tok_end = tok->end;
                Nst_token_destroy(tok);
                RETURN_ERROR(tok_start, tok_end, _Nst_EM_EXPECTED_VBRACE);
            }
            Nst_token_destroy(tok);
            Nst_node_set_pos(vect_node, start, tok->end);
            DEC_RECURSION_LVL;
            return vect_node;
        }
        else if ( tok->type == Nst_TT_COMMA )
        {
            Nst_token_destroy(tok);
            skip_blank();
        }
        else
        {
            break;
        }
    }

    if ( tok->type != Nst_TT_R_VBRACE )
    {
        Nst_node_destroy(vect_node);
        Nst_token_destroy(tok);
        RETURN_ERROR(start, err_end, _Nst_EM_MISSING_VBRACE);
    }

    Nst_node_set_pos(vect_node, start, tok->end);
    Nst_token_destroy(tok);
    DEC_RECURSION_LVL;
    return vect_node;
}

static Nst_Node *parse_arr_or_map_literal(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    bool is_map = false;
    usize count = 0;

    Nst_token_destroy(tok);
    skip_blank();

    Nst_Node *node = Nst_node_new(Nst_NT_MAP_LIT);
    SET_ERROR_IF_OP_ERR();

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type == Nst_TT_R_BRACE )
    {
        tok = Nst_TOK(Nst_llist_pop(tokens));
        Nst_node_set_pos(node, start, tok->end);
        Nst_token_destroy(tok);
        DEC_RECURSION_LVL;
        return node;
    }

    skip_blank();

    if ( Nst_TOK(Nst_llist_peek_front(tokens))->type == Nst_TT_COMMA )
    {
        Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
        skip_blank();
        tok = Nst_TOK(Nst_llist_pop(tokens));
        Nst_Pos end = tok->end;
        i32 type = tok->type;
        Nst_token_destroy(tok);

        if ( type != Nst_TT_R_BRACE )
        {
            Nst_node_destroy(node);
            RETURN_ERROR(start, end, _Nst_EM_EXPECTED_BRACE);
        }
        node->type = Nst_NT_ARR_LIT;
        Nst_node_set_pos(node, start, tok->end);
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
        tok = Nst_TOK(Nst_llist_pop(tokens));

        if ( tok->type == Nst_TT_COLON && (count == 0 || is_map) )
        {
            is_map = true;

            Nst_token_destroy(tok);
            skip_blank();

            value = parse_expr(false);
            RETURN_IF_ERROR(node);
            SAFE_NODE_APPEND(node, value);

            skip_blank();
            tok = Nst_TOK(Nst_llist_pop(tokens));
        }
        else if ( tok->type == Nst_TT_BREAK && count == 0 )
        {
            SAFE_TOK_APPEND(node, tok);
            skip_blank();

            value = parse_expr(false);
            RETURN_IF_ERROR(node);
            SAFE_NODE_APPEND(node, value);

            skip_blank();
            tok = Nst_TOK(Nst_llist_pop(tokens));

            if ( tok->type != Nst_TT_R_BRACE )
            {
                Nst_node_destroy(node);
                Nst_Pos err_start = tok->start;
                Nst_Pos err_end = tok->end;
                Nst_token_destroy(tok);
                RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_BRACE);
            }
            Nst_node_set_pos(node, start, tok->end);
            Nst_token_destroy(tok);
            node->type = Nst_NT_ARR_LIT;
            DEC_RECURSION_LVL;
            return node;
        }
        // if it's not the first iteration and it's not supposed to be a map
        else if ( tok->type == Nst_TT_COLON )
        {
            Nst_node_destroy(node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            Nst_token_destroy(tok);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_COMMA_OR_BRACE);
        }
        else if ( tok->type != Nst_TT_COLON && is_map )
        {
            Nst_node_destroy(node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            Nst_token_destroy(tok);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_COLON);
        }

        if ( tok->type == Nst_TT_R_BRACE )
        {
            if ( !is_map )
            {
                node->type = Nst_NT_ARR_LIT;
            }
            Nst_node_set_pos(node, start, tok->end);
            Nst_token_destroy(tok);
            DEC_RECURSION_LVL;
            return node;
        }
        else if ( tok->type != Nst_TT_COMMA )
        {
            Nst_node_destroy(node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            Nst_token_destroy(tok);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_COMMA_OR_BRACE);
        }

        Nst_token_destroy(tok);
        count++;
    }
}

static Nst_Node *parse_try_catch(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));
    Nst_Pos start = tok->start;
    Nst_token_destroy(tok);
    skip_blank();

    Nst_Node *try_catch_node = Nst_node_new(Nst_NT_TRY_CATCH_S);
    SET_ERROR_IF_OP_ERR();

    Nst_Node *try_block = parse_statement();
    RETURN_IF_ERROR(try_catch_node);
    SAFE_NODE_APPEND(try_catch_node, try_block);
    skip_blank();

    tok = Nst_TOK(Nst_llist_pop(tokens));
    if ( tok->type != Nst_TT_CATCH )
    {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        Nst_token_destroy(tok);
        Nst_node_destroy(try_catch_node);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_CATCH);
    }
    Nst_token_destroy(tok);
    skip_blank();

    Nst_Tok *name_tok = Nst_TOK(Nst_llist_pop(tokens));
    if ( name_tok->type != Nst_TT_IDENT )
    {
        Nst_Pos err_start = name_tok->start;
        Nst_Pos err_end = name_tok->end;
        Nst_token_destroy(name_tok);
        Nst_node_destroy(try_catch_node);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_IDENT);
    }
    SAFE_TOK_APPEND(try_catch_node, name_tok);
    skip_blank();

    Nst_Node *catch_block = parse_statement();
    RETURN_IF_ERROR(try_catch_node);
    SAFE_NODE_APPEND(try_catch_node, catch_block);

    Nst_node_set_pos(try_catch_node, start, catch_block->end);
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
        Nst_print_tok(tok);
        printf("\n");
        return;
    }

    switch ( node->type )
    {
    case Nst_NT_LONG_S:         printf("LONG_S");         break;
    case Nst_NT_WHILE_L:        printf("WHILE_L");        break;
    case Nst_NT_DOWHILE_L:      printf("DOWHILE_L");      break;
    case Nst_NT_FOR_L:          printf("FOR_L");          break;
    case Nst_NT_FOR_AS_L:       printf("FOR_AS_L");       break;
    case Nst_NT_IF_E:           printf("IF_E");           break;
    case Nst_NT_FUNC_DECLR:     printf("FUNC_DECLR");     break;
    case Nst_NT_RETURN_S:       printf("RETURN_S");       break;
    case Nst_NT_STACK_OP:       printf("STACK_OP");       break;
    case Nst_NT_LOCAL_STACK_OP: printf("LOCAL_STACK_OP"); break;
    case Nst_NT_LOCAL_OP:       printf("LOCAL_OP");       break;
    case Nst_NT_ARR_LIT:        printf("ARR_LIT");        break;
    case Nst_NT_VEC_LIT:        printf("VEC_LIT");        break;
    case Nst_NT_MAP_LIT:        printf("MAP_LIT");        break;
    case Nst_NT_VALUE:          printf("VALUE");          break;
    case Nst_NT_ACCESS:         printf("ACCESS");         break;
    case Nst_NT_EXTRACT_E:      printf("EXTRACT_E");      break;
    case Nst_NT_ASSIGN_E:       printf("ASSIGN_E");       break;
    case Nst_NT_CONTINUE_S:     printf("CONTINUE_S");     break;
    case Nst_NT_BREAK_S:        printf("BREAK_S");        break;
    case Nst_NT_SWITCH_S:       printf("SWITCH_S");       break;
    case Nst_NT_LAMBDA:         printf("LAMBDA");         break;
    case Nst_NT_TRY_CATCH_S:    printf("TRY_CATCH_S");    break;
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
    bool *last = (bool *)Nst_raw_malloc(sizeof(bool));
    if ( last == NULL )
    {
        return;
    }
    Nst_llist_append(is_last, last, true);
    if ( prev_tail == is_last->tail )
    {
        Nst_error_clear();
        Nst_free(last);
        return;
    }

    for ( cursor = node->tokens->head; cursor != NULL; cursor = cursor->next )
    {
        *last = idx == tot_len;
        _print_ast(NULL, Nst_TOK(cursor->value), lvl + 1, is_last);
        idx++;
    }

    idx = 0;

    for ( cursor = node->nodes->head; cursor != NULL; cursor = cursor->next )
    {
        *last = idx == node->nodes->size - 1;
        _print_ast(Nst_NODE(cursor->value), NULL, lvl + 1, is_last);
        idx++;
    }

    Nst_free(last);
    Nst_free(is_last->tail);
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

void Nst_print_ast(Nst_Node *ast)
{
    Nst_LList *is_last = Nst_llist_new();
    if ( is_last == NULL )
    {
        return;
    }

    _print_ast(ast, NULL, 0, is_last);
    Nst_llist_destroy(is_last, NULL);
}
