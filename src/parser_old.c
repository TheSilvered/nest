#if 0
#include <errno.h>
#include <stdio.h>
#include "mem.h"
#include "llist.h"
#include "parser_old.h"
#include "tokens.h"
#include "global_consts.h"
#include "format.h"

#define HEAD_TOK Nst_TOK(Nst_llist_peek_front(tokens))
#define POP_HEAD_TOK Nst_TOK(Nst_llist_pop(tokens))
#define DESTROY_HEAD_TOK Nst_tok_destroy(POP_HEAD_TOK)

#define RETURN_ERROR(start, end, message) do {                                \
    Nst_set_internal_syntax_error_c(Nst_error_get(), start, end, message);    \
    return NULL;                                                              \
    } while (0)

// Sets an error if cond is true or the error's name is not NULL
// ADD_POS_AND_QUIT_IF_ERR(0) will set the error if it occurred
#define ADD_POS_AND_QUIT_IF_ERR(...) do {                                     \
    if (Nst_error_occurred()) {                                               \
        Nst_error_add_positions(                                              \
            Nst_error_get(),                                                  \
            HEAD_TOK->start,                                            \
            HEAD_TOK->end);                                             \
        __VA_ARGS__                                                           \
        return NULL;                                                          \
    }                                                                         \
    } while (0)

#define SAFE_NODE_APPEND(node, node_to_append) do {                           \
    Nst_llist_append(node->nodes, node_to_append, true);                      \
    ADD_POS_AND_QUIT_IF_ERR(                                                  \
        Nst_node_destroy__old(node);                                               \
        Nst_node_destroy__old(node_to_append););                                   \
    } while (0)

#define SAFE_TOK_APPEND(node, tok_to_append) do {                             \
    Nst_llist_append(node->tokens, tok_to_append, true);                      \
    ADD_POS_AND_QUIT_IF_ERR(                                                  \
        Nst_node_destroy__old(node);                                               \
        Nst_tok_destroy(tok_to_append););                                     \
    } while (0)

#define RETURN_IF_ERROR(node) do {                                            \
    if (Nst_error_occurred()) {                                               \
        Nst_node_destroy__old(node);                                               \
        return NULL;                                                          \
    }                                                                         \
    } while (0)

#define INC_RECURSION_LVL do {                                                \
    recursion_lvl++;                                                          \
    if (recursion_lvl > 1500) {                                               \
        Nst_set_internal_memory_error_c(                                      \
            Nst_error_get(),                                                  \
            HEAD_TOK->start,                                            \
            HEAD_TOK->end,                                              \
            "over 1500 recursive calls, parsing failed");                     \
        return NULL;                                                          \
    }                                                                         \
    } while (0)

#define DEC_RECURSION_LVL recursion_lvl--

typedef struct _ParsingState {
    bool in_func;
    bool in_loop;
    bool in_switch;
} ParsingState__old;

static ParsingState__old p_state;
static Nst_LList *tokens;
static int recursion_lvl = 0;

static inline void skip_blank(void);
static Nst_Tok *copy_token(Nst_Tok *tok);
static Nst_Node__old *fix_expr(Nst_Node__old *expr);

static Nst_Node__old *parse_long_s(void);
static Nst_Node__old *parse_statement(void);
static Nst_Node__old *parse_while_l(void);
static Nst_Node__old *parse_for_l(void);
static Nst_Node__old *parse_if_e(Nst_Node__old *condition);
static Nst_Node__old *parse_switch_s(void);
static Nst_Node__old *parse_func_declr(void);
static Nst_Node__old *parse_expr(bool break_as_end);
static Nst_Node__old *parse_stack_op(Nst_Node__old *value, Nst_Pos start);
static Nst_Node__old *parse_local_stack_op(Nst_LList *nodes, Nst_Pos start);
static Nst_Node__old *parse_assignment_name(bool is_compound);
static Nst_Node__old *parse_assignment(Nst_Node__old *value);
static Nst_Node__old *parse_extraction(void);
static Nst_Node__old *parse_atom(void);
static Nst_Node__old *parse_vector_literal(void);
static Nst_Node__old *parse_arr_or_map_literal(void);
static Nst_Node__old *parse_try_catch(void);

Nst_Node__old *Nst_parse(Nst_LList *tokens_list)
{
    if (tokens_list == NULL)
        return NULL;

    tokens = tokens_list;

    p_state.in_func = false;
    p_state.in_loop = false;
    p_state.in_switch = false;
    recursion_lvl = 0;
    Nst_Node__old *node = parse_long_s();

    // i.e. there are tokens other than Nst_TT_EOFILE
    if (!Nst_error_occurred() && tokens->len > 1) {
        Nst_Pos start = HEAD_TOK->start;
        Nst_Pos end = HEAD_TOK->start;

        Nst_set_internal_syntax_error_c(
            Nst_error_get(),
            start, end,
            _Nst_EM_UNEXPECTED_TOK);
    }

    Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
    return node; // NULL if there was an error
}

static inline void skip_blank(void)
{
    while (HEAD_TOK->type == Nst_TT_ENDL)
        DESTROY_HEAD_TOK;
}

static Nst_Tok *copy_token(Nst_Tok *tok)
{
    return Nst_tok_new_value(
        tok->start,
        tok->end,
        tok->type,
        Nst_ninc_ref(tok->value));
}

static Nst_Node__old *parse_long_s(void)
{
    INC_RECURSION_LVL;
    Nst_Node__old *long_statement_node = Nst_node_new__old(Nst_NT_LONG_S);
    ADD_POS_AND_QUIT_IF_ERR();

    Nst_Node__old *node = NULL;
    Nst_LList *nodes = long_statement_node->nodes;
    skip_blank();

    while (HEAD_TOK->type != Nst_TT_R_BRACKET
           && HEAD_TOK->type != Nst_TT_EOFILE)
    {
        node = parse_statement();
        RETURN_IF_ERROR(long_statement_node);
        SAFE_NODE_APPEND(long_statement_node, node);
        skip_blank();
    }

    if (nodes->len == 0) {
        Nst_node_set_pos__old(
            long_statement_node,
            HEAD_TOK->start,
            HEAD_TOK->end);
    } else {
        Nst_node_set_pos__old(
            long_statement_node,
            Nst_NODE(nodes->head->value)->start,
            Nst_NODE(nodes->tail->value)->end);
    }

    DEC_RECURSION_LVL;
    return long_statement_node;
}

static Nst_Node__old *parse_statement(void)
{
    INC_RECURSION_LVL;
    skip_blank();
    i32 tok_type = HEAD_TOK->type;

    if (tok_type == Nst_TT_L_BRACKET) {
        Nst_Tok *open_bracket = POP_HEAD_TOK;
        Nst_Pos start = open_bracket->start;
        Nst_tok_destroy(open_bracket);

        Nst_Node__old *node = parse_long_s();
        if (Nst_error_occurred())
            return NULL;

        Nst_Tok *close_bracket = POP_HEAD_TOK;
        if (close_bracket->type != Nst_TT_R_BRACKET) {
            Nst_tok_destroy(close_bracket);
            Nst_node_destroy__old(node);
            RETURN_ERROR(start, start, _Nst_EM_MISSING_BRACKET);
        }
        Nst_tok_destroy(close_bracket);
        DEC_RECURSION_LVL;
        return node;
    } else if (tok_type == Nst_TT_WHILE || tok_type == Nst_TT_DOWHILE) {
        DEC_RECURSION_LVL;
        return parse_while_l();
    } else if (tok_type == Nst_TT_FOR) {
        DEC_RECURSION_LVL;
        return parse_for_l();
    } else if (tok_type == Nst_TT_FUNC) {
        DEC_RECURSION_LVL;
        return parse_func_declr();
    } else if (tok_type == Nst_TT_SWITCH) {
        DEC_RECURSION_LVL;
        return parse_switch_s();
    } else if (tok_type == Nst_TT_RETURN) {
        Nst_Tok *tok = POP_HEAD_TOK;
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        Nst_Node__old *expr;
        Nst_tok_destroy(tok);

        if (!p_state.in_func)
            RETURN_ERROR(start, end, _Nst_EM_BAD_RETURN);

        tok_type = HEAD_TOK->type;
        if (Nst_IS_EXPR_END(tok_type)) {
            Nst_Tok *null_value = Nst_tok_new_value(
                start, end,
                Nst_TT_VALUE,
                Nst_inc_ref(Nst_c.Null_null));
            ADD_POS_AND_QUIT_IF_ERR();
            expr = Nst_node_new_pos__old(
                Nst_NT_VALUE,
                start,
                end);
            ADD_POS_AND_QUIT_IF_ERR(Nst_tok_destroy(null_value););
            SAFE_TOK_APPEND(expr, null_value);
        } else {
            expr = parse_expr(false);
            if (Nst_error_occurred())
                return NULL;
        }

        Nst_Node__old *return_s_node = Nst_node_new_pos__old(
            Nst_NT_RETURN_S,
            start, expr->end);
        ADD_POS_AND_QUIT_IF_ERR(Nst_node_destroy__old(expr););
        SAFE_NODE_APPEND(return_s_node, expr);
        DEC_RECURSION_LVL;
        return return_s_node;
    } else if (tok_type == Nst_TT_CONTINUE) {
        Nst_Tok *tok = POP_HEAD_TOK;
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        Nst_tok_destroy(tok);

        if (!p_state.in_loop && !p_state.in_switch)
            RETURN_ERROR(start, end, _Nst_EM_BAD_CONTINUE);

        Nst_Node__old *continue_node = Nst_node_new_pos__old(
            Nst_NT_CONTINUE_S,
            start, end);
        ADD_POS_AND_QUIT_IF_ERR();
        DEC_RECURSION_LVL;
        return continue_node;
    } else if (tok_type == Nst_TT_BREAK) {
        Nst_Tok *tok = POP_HEAD_TOK;
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;
        Nst_tok_destroy(tok);

        if (!p_state.in_loop)
            RETURN_ERROR(start, end, _Nst_EM_BAD_BREAK);
        Nst_Node__old *break_node = Nst_node_new_pos__old(
            Nst_NT_BREAK_S,
            start, end);
        ADD_POS_AND_QUIT_IF_ERR();
        DEC_RECURSION_LVL;
        return break_node;
    } else if (tok_type == Nst_TT_TRY) {
        DEC_RECURSION_LVL;
        return parse_try_catch();
    } else if (Nst_IS_ATOM(tok_type) || Nst_IS_LOCAL_STACK_OP(tok_type)) {
        DEC_RECURSION_LVL;
        return parse_expr(false);
    } else {
        Nst_Tok *tok = POP_HEAD_TOK;
        Nst_Pos start = tok->start;
        Nst_Pos end = tok->end;

        Nst_tok_destroy(tok);
        RETURN_ERROR(start, end, _Nst_EM_UNEXPECTED_TOK);
    }
}

static Nst_Node__old *parse_while_l(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = POP_HEAD_TOK;
    Nst_Pos start = tok->start;

    Nst_Node__old *while_l_node = Nst_node_new__old(
        tok->type == Nst_TT_WHILE ? Nst_NT_WHILE_L : Nst_NT_DOWHILE_L);
    ADD_POS_AND_QUIT_IF_ERR();

    Nst_tok_destroy(tok);
    skip_blank();

    Nst_Node__old *condition = parse_expr(false);
    RETURN_IF_ERROR(while_l_node);
    SAFE_NODE_APPEND(while_l_node, condition);
    skip_blank();

    Nst_Pos err_pos = HEAD_TOK->start;

    if (HEAD_TOK->type != Nst_TT_L_BRACKET) {
        Nst_node_destroy__old(while_l_node);
        RETURN_ERROR(err_pos, err_pos, _Nst_EM_EXPECTED_BRACKET);
    }

    DESTROY_HEAD_TOK;

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;

    Nst_Node__old *body = parse_long_s();
    RETURN_IF_ERROR(while_l_node);
    SAFE_NODE_APPEND(while_l_node, body);
    p_state.in_loop = prev_state;

    if (HEAD_TOK->type != Nst_TT_R_BRACKET) {
        Nst_node_destroy__old(while_l_node);
        RETURN_ERROR(err_pos, err_pos, _Nst_EM_MISSING_BRACKET);
    }

    tok = POP_HEAD_TOK;
    Nst_Pos end = tok->end;
    Nst_tok_destroy(tok);

    Nst_node_set_pos__old(while_l_node, start, end);
    DEC_RECURSION_LVL;
    return while_l_node;
}

static Nst_Node__old *parse_for_l(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = POP_HEAD_TOK;
    Nst_Pos start = tok->start;
    Nst_tok_destroy(tok);

    Nst_Node__old *for_l_node = Nst_node_new__old(Nst_NT_FOR_L);
    ADD_POS_AND_QUIT_IF_ERR();

    skip_blank();
    Nst_Node__old *range = parse_expr(false);
    RETURN_IF_ERROR(for_l_node);

    Nst_llist_append(for_l_node->nodes, range, true);
    ADD_POS_AND_QUIT_IF_ERR(Nst_node_destroy__old(for_l_node); Nst_node_destroy__old(range););

    if (HEAD_TOK->type == Nst_TT_AS) {
        DESTROY_HEAD_TOK;
        Nst_Node__old *name_node = parse_assignment_name(false);
        RETURN_IF_ERROR(for_l_node);
        SAFE_NODE_APPEND(for_l_node, name_node);
        for_l_node->type = Nst_NT_FOR_AS_L;
    }

    skip_blank();

    Nst_Pos err_pos = HEAD_TOK->start;

    if (HEAD_TOK->type != Nst_TT_L_BRACKET) {
        Nst_node_destroy__old(for_l_node);
        RETURN_ERROR(err_pos, err_pos, _Nst_EM_EXPECTED_BRACKET);
    }

    DESTROY_HEAD_TOK;

    bool prev_state = p_state.in_loop;
    p_state.in_loop = true;
    Nst_Node__old *body = parse_long_s();
    RETURN_IF_ERROR(for_l_node);
    SAFE_NODE_APPEND(for_l_node, body);
    p_state.in_loop = prev_state;

    if (HEAD_TOK->type != Nst_TT_R_BRACKET) {
        Nst_node_destroy__old(for_l_node);
        RETURN_ERROR(err_pos, err_pos, _Nst_EM_MISSING_BRACKET);
    }

    tok = POP_HEAD_TOK;
    Nst_Pos end = tok->end;
    Nst_tok_destroy(tok);

    Nst_node_set_pos__old(for_l_node, start, end);
    DEC_RECURSION_LVL;
    return for_l_node;
}

static Nst_Node__old *parse_if_e(Nst_Node__old *condition)
{
    INC_RECURSION_LVL;
    DESTROY_HEAD_TOK;
    skip_blank();

    Nst_Node__old *if_expr_node = Nst_node_new__old(Nst_NT_IF_E);
    ADD_POS_AND_QUIT_IF_ERR();
    SAFE_NODE_APPEND(if_expr_node, condition);

    Nst_Node__old *body_if_true = parse_statement();
    RETURN_IF_ERROR(if_expr_node);
    SAFE_NODE_APPEND(if_expr_node, body_if_true);
    skip_blank();

    if (HEAD_TOK->type != Nst_TT_COLON) {
        Nst_node_set_pos__old(if_expr_node, condition->start, body_if_true->end);
        DEC_RECURSION_LVL;
        return if_expr_node;
    }

    DESTROY_HEAD_TOK;
    skip_blank();

    Nst_Node__old *body_if_false = parse_statement();
    RETURN_IF_ERROR(if_expr_node);
    SAFE_NODE_APPEND(if_expr_node, body_if_false);

    Nst_node_set_pos__old(if_expr_node, condition->start, body_if_false->end);
    DEC_RECURSION_LVL;
    return if_expr_node;
}

static Nst_Node__old *parse_switch_s(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = POP_HEAD_TOK;
    Nst_Pos start = tok->start;
    Nst_Pos err_start;
    Nst_Pos err_end;
    Nst_tok_destroy(tok);
    skip_blank();

    Nst_Node__old *switch_s_node = Nst_node_new__old(Nst_NT_SWITCH_S);
    ADD_POS_AND_QUIT_IF_ERR();

    Nst_Node__old *main_val = parse_expr(false);
    RETURN_IF_ERROR(switch_s_node);
    SAFE_NODE_APPEND(switch_s_node, main_val);

    skip_blank();
    tok = POP_HEAD_TOK;

    if (tok->type != Nst_TT_L_BRACKET) {
        err_start = tok->start;
        err_end = tok->end;
        Nst_tok_destroy(tok);
        Nst_node_destroy__old(switch_s_node);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_BRACKET);
    }
    Nst_tok_destroy(tok);

    bool is_default_case = false;
    bool prev_in_switch = p_state.in_switch;
    p_state.in_switch = true;

    while (true) {
        skip_blank();
        tok = POP_HEAD_TOK;
        if (tok->type != Nst_TT_IF) {
            err_start = tok->start;
            err_end = tok->end;
            Nst_tok_destroy(tok);
            Nst_node_destroy__old(switch_s_node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_IF);
        }
        Nst_tok_destroy(tok);
        skip_blank();
        tok = HEAD_TOK;

        if (tok->type == Nst_TT_L_BRACKET) {
            is_default_case = true;
            err_start = tok->start;
            err_end = tok->end;
            DESTROY_HEAD_TOK;
        } else {
            Nst_Node__old *val = parse_expr(false);
            RETURN_IF_ERROR(switch_s_node);
            SAFE_NODE_APPEND(switch_s_node, val);
            skip_blank();
            tok = POP_HEAD_TOK;
            err_start = tok->start;
            err_end = tok->end;

            if (tok->type != Nst_TT_L_BRACKET) {
                Nst_tok_destroy(tok);
                Nst_node_destroy__old(switch_s_node);
                RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_BRACKET);
            }
            Nst_tok_destroy(tok);
        }

        Nst_Node__old *body = parse_long_s();
        RETURN_IF_ERROR(switch_s_node);
        SAFE_NODE_APPEND(switch_s_node, body);
        skip_blank();
        tok = POP_HEAD_TOK;
        if (tok->type != Nst_TT_R_BRACKET) {
            err_start = tok->start;
            err_end = tok->end;
            Nst_tok_destroy(tok);
            Nst_node_destroy__old(switch_s_node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_R_BRACKET);
        }
        Nst_tok_destroy(tok);
        skip_blank();
        tok = HEAD_TOK;

        if (tok->type == Nst_TT_R_BRACKET) {
            Nst_Pos end = tok->end;
            DESTROY_HEAD_TOK;
            p_state.in_switch = prev_in_switch;
            Nst_node_set_pos__old(switch_s_node, start, end);
            DEC_RECURSION_LVL;
            return switch_s_node;
        } else if (is_default_case) {
            err_start = tok->start;
            err_end = tok->end;
            DESTROY_HEAD_TOK;
            Nst_node_destroy__old(switch_s_node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_R_BRACKET);
        }
    }
}

static Nst_Node__old *parse_func_declr(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = POP_HEAD_TOK;
    Nst_Pos start = tok->start;
    Nst_Pos end;
    bool is_lambda = tok->type == Nst_TT_LAMBDA;
    Nst_tok_destroy(tok);

    Nst_Node__old *func_node = Nst_node_new__old(
        is_lambda ? Nst_NT_LAMBDA : Nst_NT_FUNC_DECLR);

    skip_blank();
    while (HEAD_TOK->type == Nst_TT_IDENT) {
        tok = POP_HEAD_TOK;
        SAFE_TOK_APPEND(func_node, tok);
        skip_blank();
    }

    Nst_Pos err_start = HEAD_TOK->start;
    Nst_Pos err_end = HEAD_TOK->end;

    // if there are no identifiers after #
    if (func_node->tokens->len == 0 && !is_lambda) {
        Nst_node_destroy__old(func_node);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_IDENT);
    }

    bool expects_r_bracket = false;
    Nst_Pos return_start = HEAD_TOK->start;

    if (HEAD_TOK->type == Nst_TT_L_BRACKET)
        expects_r_bracket = true;
    else if (HEAD_TOK->type != Nst_TT_RETURN) {
        Nst_Pos tok_end = HEAD_TOK->start;
        Nst_node_destroy__old(func_node);
        RETURN_ERROR(
            return_start,
            tok_end,
            _Nst_EM_EXPECTED_RETURN_OR_BRACKET);
    }

    DESTROY_HEAD_TOK;

    bool prev_state = p_state.in_func;
    p_state.in_func = true;

    Nst_Node__old *body;

    if (expects_r_bracket)
        body = parse_long_s();
    else
        body = parse_expr(false);
    RETURN_IF_ERROR(func_node);

    p_state.in_func = prev_state;

    if (expects_r_bracket && HEAD_TOK->type != Nst_TT_R_BRACKET) {
        Nst_node_destroy__old(func_node);
        Nst_node_destroy__old(body);
        RETURN_ERROR(err_start, err_end, _Nst_EM_MISSING_BRACKET);
    } else if (expects_r_bracket) {
        tok = POP_HEAD_TOK;
        end = tok->end;
        Nst_tok_destroy(tok);
    } else {
        Nst_Node__old *return_node = Nst_node_new_pos__old(
            Nst_NT_RETURN_S,
            return_start,
            body->end);

        Nst_llist_append(return_node->nodes, body, true);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_node_destroy__old(func_node);
        Nst_node_destroy__old(body);
        Nst_node_destroy__old(return_node););
        body = return_node;
        end = return_node->end;
    }

    SAFE_NODE_APPEND(func_node, body);
    Nst_node_set_pos__old(func_node, start, end);
    DEC_RECURSION_LVL;
    return func_node;
}

static Nst_Node__old *parse_expr(bool break_as_end)
{
    INC_RECURSION_LVL;
    Nst_Node__old *node = NULL;
    Nst_Pos start = HEAD_TOK->start;
    i32 token_type = HEAD_TOK->type;

    while (!Nst_IS_EXPR_END(token_type)
           && (!break_as_end || token_type != Nst_TT_BREAK)) {
        node = parse_stack_op(node, start);
        if (Nst_error_occurred())
            return NULL;
        token_type = HEAD_TOK->type;
    }

    if (node == NULL) {
        Nst_Tok *tok = HEAD_TOK;
        RETURN_ERROR(tok->start, tok->end, _Nst_EM_EXPECTED_VALUE);
    }

    node = fix_expr(node);
    if (node == NULL)
        return NULL;

    token_type = HEAD_TOK->type;

    if (token_type == Nst_TT_IF) {
        DEC_RECURSION_LVL;
        // the error propagates automatically
        return parse_if_e(node);
    }
    DEC_RECURSION_LVL;
    return node;
}

static Nst_Node__old *fix_expr(Nst_Node__old *expr)
{
    INC_RECURSION_LVL;
    if (expr->type != Nst_NT_STACK_OP
        && expr->type != Nst_NT_LOCAL_STACK_OP
        && expr->type != Nst_NT_ASSIGN_E)
    {
        DEC_RECURSION_LVL;
        return expr;
    }

    if (expr->type == Nst_NT_STACK_OP && expr->nodes->len == 1) {
        Nst_Node__old *new_node = Nst_NODE(Nst_llist_peek_front(expr->nodes));
        Nst_llist_empty(expr->tokens, (Nst_LListDestructor)Nst_tok_destroy);
        Nst_llist_empty(expr->nodes, NULL);
        Nst_free(expr->tokens);
        Nst_free(expr);
        DEC_RECURSION_LVL;
        return fix_expr(new_node);
    }

    for (Nst_LLIST_ITER(cursor, expr->nodes))
        cursor->value = fix_expr(Nst_NODE(cursor->value));

    if (expr->type != Nst_NT_STACK_OP) {
        DEC_RECURSION_LVL;
        return expr;
    }

    Nst_Node__old *curr_node = expr;
    Nst_Tok *op_tok = Nst_TOK(Nst_llist_peek_front(expr->tokens));

    // comparisons are handled differently to not copy nodes
    if (Nst_IS_COMP_OP(op_tok->type)) {
        DEC_RECURSION_LVL;
        return expr;
    }

    // writing 1 2 3 4 + becomes 1 2 + 3 + 4 +
    for (usize i = 0, n = expr->nodes->len - 2; i < n; i++) {
        // get the positions
        Nst_Pos start = curr_node->start;
        Nst_Pos end = curr_node->end;
        Nst_Node__old *new_node = Nst_node_new_pos__old(
            Nst_NT_STACK_OP,
            start, end);

        // move the nodes except for the last one
        for (usize j = 0, m = curr_node->nodes->len - 1; j < m; j++) {
            Nst_llist_append_llnode(
                new_node->nodes,
                Nst_llist_pop_llnode(curr_node->nodes));
        }

        Nst_Tok *new_tok = copy_token(op_tok);
        ADD_POS_AND_QUIT_IF_ERR(
                Nst_node_destroy__old(expr);
                Nst_node_destroy__old(new_node););
        Nst_llist_append(new_node->tokens, new_tok, true);
        ADD_POS_AND_QUIT_IF_ERR(
                Nst_node_destroy__old(expr);
                Nst_node_destroy__old(new_node);
                Nst_tok_destroy(new_tok););

        Nst_llist_push(curr_node->nodes, new_node, true);
        ADD_POS_AND_QUIT_IF_ERR(Nst_node_destroy__old(expr););
        curr_node = new_node;
    }

    DEC_RECURSION_LVL;
    return expr;
}

static Nst_Node__old *parse_stack_op(Nst_Node__old *value, Nst_Pos start)
{
    INC_RECURSION_LVL;
    Nst_Node__old *value_node = NULL;
    Nst_LList *new_nodes = Nst_llist_new();
    ADD_POS_AND_QUIT_IF_ERR();

    if (value != NULL) {
        Nst_llist_append(new_nodes, value, true);
        ADD_POS_AND_QUIT_IF_ERR(Nst_node_destroy__old(value););
    } else {
        value_node = parse_extraction();
        if (Nst_error_occurred()) {
            Nst_llist_destroy(new_nodes, NULL);
            return NULL;
        }
        Nst_llist_append(new_nodes, value_node, true);
        ADD_POS_AND_QUIT_IF_ERR(Nst_node_destroy__old(value_node););
    }

    while (Nst_IS_ATOM(HEAD_TOK->type)) {
        value_node = parse_extraction();
        if (Nst_error_occurred()) {
            Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy__old);
            return NULL;
        }
        Nst_llist_append(new_nodes, value_node, true);
        ADD_POS_AND_QUIT_IF_ERR(Nst_node_destroy__old(value_node););
    }

    Nst_Node__old *node = NULL;
    Nst_Tok *op_tok = HEAD_TOK;
    Nst_Pos end = op_tok->end;
    bool is_local_stack_op = false;

    if (Nst_IS_STACK_OP(op_tok->type)) {
        node = Nst_node_new_pos__old(Nst_NT_STACK_OP, start, end);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_llist_destroy(
                new_nodes,
                (Nst_LListDestructor)Nst_node_destroy__old););
        Nst_llist_move_nodes(new_nodes, node->nodes);
        Nst_free(new_nodes);
        Nst_llist_pop(tokens);
        SAFE_TOK_APPEND(node, op_tok);
    } else if (Nst_IS_LOCAL_STACK_OP(op_tok->type)) {
        node = parse_local_stack_op(new_nodes, start);
        if (Nst_error_occurred()) {
            Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy__old);
            return NULL;
        }
        is_local_stack_op = true;
    } else if (Nst_IS_ASSIGNMENT(op_tok->type)
               && op_tok->type != Nst_TT_ASSIGN)
    {
        Nst_Tok *new_tok = Nst_tok_new_noval(
            op_tok->start,
            op_tok->end,
            Nst_ASSIGNMENT_TO_STACK_OP(op_tok->type));
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_llist_destroy(
                new_nodes,
                (Nst_LListDestructor)Nst_node_destroy__old););
        node = Nst_node_new_pos__old(
            Nst_NT_STACK_OP,
            start, end);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_llist_destroy(
                new_nodes,
                (Nst_LListDestructor)Nst_node_destroy__old);
            Nst_tok_destroy(new_tok););
        Nst_llist_move_nodes(new_nodes, node->nodes);
        Nst_free(new_nodes);
        SAFE_TOK_APPEND(node, new_tok);
    } else if (new_nodes->len == 1 && value == NULL) {
        node = Nst_NODE(Nst_llist_pop(new_nodes));
        Nst_llist_destroy(new_nodes, NULL);
    } else {
        Nst_llist_destroy(new_nodes, (Nst_LListDestructor)Nst_node_destroy__old);
        RETURN_ERROR(op_tok->start, end, _Nst_EM_EXPECTED_OP);
    }

    while (!Nst_IS_EXPR_END(HEAD_TOK->type)) {
        op_tok = HEAD_TOK;
        if (Nst_IS_STACK_OP(op_tok->type) && !is_local_stack_op) {
            Nst_Node__old *new_node = Nst_node_new_pos__old(
                Nst_NT_STACK_OP,
                start, op_tok->end);
            ADD_POS_AND_QUIT_IF_ERR(Nst_node_destroy__old(node););
            SAFE_NODE_APPEND(new_node, node);
            op_tok = POP_HEAD_TOK;
            SAFE_TOK_APPEND(new_node, op_tok);
        } else if (Nst_IS_LOCAL_STACK_OP(op_tok->type) && is_local_stack_op) {
            new_nodes = Nst_llist_new();
            ADD_POS_AND_QUIT_IF_ERR(Nst_node_destroy__old(node););
            Nst_llist_append(new_nodes, node, true);
            ADD_POS_AND_QUIT_IF_ERR(Nst_node_destroy__old(node););
            node = parse_local_stack_op(new_nodes, start);
            if (Nst_error_occurred()) {
                Nst_llist_destroy(
                    new_nodes,
                    (Nst_LListDestructor)Nst_node_destroy__old);
                return NULL;
            }
        } else if (Nst_IS_ASSIGNMENT(op_tok->type)) {
            node = parse_assignment(node);
            if (Nst_error_occurred())
                return NULL;
        } else
            break;
    }

    DEC_RECURSION_LVL;
    return node;
}

static Nst_Node__old *parse_local_stack_op(Nst_LList *nodes, Nst_Pos start)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = POP_HEAD_TOK;

    if (tok->type == Nst_TT_CAST && nodes->len != 1) {
        Nst_tok_destroy(tok);
        RETURN_ERROR(
            Nst_NODE(Nst_llist_peek_front(nodes))->start,
            Nst_NODE(Nst_llist_peek_back(nodes))->end,
            _Nst_EM_LEFT_ARGS_NUM("::", "1", ""));
    } else if (tok->type == Nst_TT_RANGE
               && nodes->len != 1
               && nodes->len != 2)
    {
        Nst_tok_destroy(tok);
        RETURN_ERROR(
            Nst_NODE(Nst_llist_peek_front(nodes))->start,
            Nst_NODE(Nst_llist_peek_back(nodes))->end,
            _Nst_EM_LEFT_ARGS_NUM("->", "1 or 2", "s"));
    } else if (tok->type == Nst_TT_THROW && nodes->len != 1) {
        Nst_tok_destroy(tok);
        RETURN_ERROR(
            Nst_NODE(Nst_llist_peek_front(nodes))->start,
            Nst_NODE(Nst_llist_peek_back(nodes))->end,
            _Nst_EM_LEFT_ARGS_NUM("!!", "1", ""));
    }
    else if (tok->type == Nst_TT_SEQ_CALL && nodes->len != 1) {
        Nst_tok_destroy(tok);
        RETURN_ERROR(
            Nst_NODE(Nst_llist_peek_front(nodes))->start,
            Nst_NODE(Nst_llist_peek_back(nodes))->end,
            _Nst_EM_LEFT_ARGS_NUM("*@", "1", ""));
    }

    Nst_Node__old *local_stack_op_node = Nst_node_new__old(Nst_NT_LOCAL_STACK_OP);
    SAFE_TOK_APPEND(local_stack_op_node, tok);
    Nst_llist_move_nodes(nodes, local_stack_op_node->nodes);

    Nst_Node__old *special_node = parse_extraction();
    RETURN_IF_ERROR(local_stack_op_node);
    SAFE_NODE_APPEND(local_stack_op_node, special_node);

    Nst_free(nodes);
    Nst_node_set_pos__old(local_stack_op_node, start, special_node->end);
    DEC_RECURSION_LVL;
    return local_stack_op_node;
}

static Nst_Node__old *parse_assignment_name(bool is_compound)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = HEAD_TOK;
    Nst_Pos start = tok->start;
    Nst_Node__old *node;

    if (tok->type != Nst_TT_L_BRACE) {
        node = parse_extraction();

        if (Nst_error_occurred())
            return NULL;

        if (node->type != Nst_NT_ACCESS && node->type != Nst_NT_EXTRACT_E) {
            Nst_Pos err_start = node->start;
            Nst_Pos err_end = node->end;
            Nst_node_destroy__old(node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_IDENT_OR_EXTR);
        }
        DEC_RECURSION_LVL;
        return node;
    }

    tok = POP_HEAD_TOK;

    if (is_compound) {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        Nst_tok_destroy(tok);
        RETURN_ERROR(err_start, err_end, _Nst_EM_COMPOUND_ASSIGMENT);
    }

    node = Nst_node_new__old(Nst_NT_ARR_LIT);
    ADD_POS_AND_QUIT_IF_ERR();

    while (true) {
        Nst_tok_destroy(tok);
        skip_blank();
        Nst_Node__old *name_node = parse_assignment_name(false);
        RETURN_IF_ERROR(node);
        SAFE_NODE_APPEND(node, name_node);

        skip_blank();
        tok = POP_HEAD_TOK;
        if (tok->type == Nst_TT_COMMA)
            continue;
        else if (tok->type == Nst_TT_R_BRACE) {
            Nst_node_set_pos__old(node, start, tok->end);
            Nst_tok_destroy(tok);
            DEC_RECURSION_LVL;
            return node;
        } else {
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            Nst_tok_destroy(tok);
            Nst_node_destroy__old(node);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_COMMA_OR_BRACE);
        }
    }
}

static Nst_Node__old *parse_assignment(Nst_Node__old *value)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = POP_HEAD_TOK;
    bool is_compound = tok->type != Nst_TT_ASSIGN;
    Nst_Node__old *name = parse_assignment_name(is_compound);
    if (Nst_error_occurred()) {
        Nst_tok_destroy(tok);
        Nst_node_destroy__old(value);
        return NULL;
    }

    // If a compound assignmen operator such as '+=' or '*='
    if (is_compound) {
        Nst_Tok *op_tok = Nst_tok_new_noval(
            tok->start,
            tok->end,
            Nst_ASSIGNMENT_TO_STACK_OP(tok->type));
        Nst_tok_destroy(tok);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_node_destroy__old(value);
            Nst_node_destroy__old(name););
        Nst_Node__old *op_node = Nst_node_new_pos__old(
            Nst_NT_STACK_OP,
            value->start,
            name->end);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_node_destroy__old(value);
            Nst_tok_destroy(op_tok);
            Nst_node_destroy__old(name););

        Nst_llist_append(op_node->tokens, op_tok, true);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_node_destroy__old(value);
            Nst_tok_destroy(op_tok);
            Nst_node_destroy__old(op_node);
            Nst_node_destroy__old(name););
        Nst_llist_append(op_node->nodes, name, false);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_node_destroy__old(value);
            Nst_node_destroy__old(op_node);
            Nst_node_destroy__old(name););
        Nst_llist_append(op_node->nodes, value, true);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_node_destroy__old(value);
            Nst_node_destroy__old(op_node);
            Nst_node_destroy__old(name););
        value = op_node;
    } else
        Nst_tok_destroy(tok);

    Nst_Pos start = value->start;
    Nst_Pos end = name->end;
    Nst_Node__old *assignment_node = Nst_node_new_pos__old(
        Nst_NT_ASSIGN_E,
        start, end);
    ADD_POS_AND_QUIT_IF_ERR(
        Nst_node_destroy__old(value);
        Nst_node_destroy__old(name););
    Nst_llist_append(assignment_node->nodes, value, true);
    ADD_POS_AND_QUIT_IF_ERR(
        Nst_node_destroy__old(value);
        Nst_node_destroy__old(name);
        Nst_node_destroy__old(assignment_node););
    SAFE_NODE_APPEND(assignment_node, name);
    DEC_RECURSION_LVL;
    return assignment_node;
}

static Nst_Node__old *parse_extraction(void)
{
    INC_RECURSION_LVL;
    Nst_Node__old *atom = parse_atom();
    if (Nst_error_occurred())
        return NULL;

    Nst_Node__old *final_node = atom;

    while (HEAD_TOK->type == Nst_TT_EXTRACT) {
        DESTROY_HEAD_TOK;
        bool treat_as_string = HEAD_TOK->type == Nst_TT_IDENT;

        atom = parse_atom();
        RETURN_IF_ERROR(final_node);

        // makes `a.i` equivalent to `a.'i'`
        if (treat_as_string) {
            atom->type = Nst_NT_VALUE;
            Nst_TOK(Nst_llist_peek_front(atom->tokens))->type = Nst_TT_VALUE;
        }

        Nst_Node__old *extraction_node = Nst_node_new_pos__old(
            Nst_NT_EXTRACT_E,
            final_node->start,
            atom->end);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_node_destroy__old(final_node);
            Nst_node_destroy__old(atom););
        Nst_llist_append(extraction_node->nodes, final_node, true);
        ADD_POS_AND_QUIT_IF_ERR(
            Nst_node_destroy__old(final_node);
            Nst_node_destroy__old(atom);
            Nst_node_destroy__old(extraction_node););
        SAFE_NODE_APPEND(extraction_node, atom);

        final_node = extraction_node;
    }
    DEC_RECURSION_LVL;
    return final_node;
}

static Nst_Node__old *parse_atom(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = HEAD_TOK;

    if (Nst_IS_VALUE(tok->type)) {
        Nst_Node__old *value_node = Nst_node_new_pos__old(
            tok->type == Nst_TT_IDENT ? Nst_NT_ACCESS : Nst_NT_VALUE,
            tok->start, tok->end);
        ADD_POS_AND_QUIT_IF_ERR();
        Nst_llist_pop(tokens);
        SAFE_TOK_APPEND(value_node, tok);
        DEC_RECURSION_LVL;
        return value_node;
    } else if (tok->type == Nst_TT_L_PAREN) {
        Nst_Pos start = tok->start;
        Nst_Pos err_end = tok->end;
        Nst_tok_destroy(tok);
        Nst_llist_pop(tokens);
        Nst_Node__old *expr = parse_expr(false);
        if (Nst_error_occurred())
            return NULL;

        tok = POP_HEAD_TOK;
        if (tok->type != Nst_TT_R_PAREN) {
            Nst_node_destroy__old(expr);
            Nst_tok_destroy(tok);
            RETURN_ERROR(start, err_end, _Nst_EM_MISSING_PAREN);
        }

        expr->start = start;
        expr->end = tok->end;

        Nst_tok_destroy(tok);
        DEC_RECURSION_LVL;
        return expr;
    } else if (Nst_IS_LOCAL_OP(tok->type)) {
        Nst_Node__old *local_op_node = Nst_node_new__old(Nst_NT_LOCAL_OP);
        ADD_POS_AND_QUIT_IF_ERR();
        Nst_llist_pop(tokens);
        SAFE_TOK_APPEND(local_op_node, tok);

        Nst_Node__old *value = parse_extraction();
        RETURN_IF_ERROR(local_op_node);
        SAFE_NODE_APPEND(local_op_node, value);

        Nst_node_set_pos__old(local_op_node, tok->start, value->end);
        DEC_RECURSION_LVL;
        return local_op_node;
    } else if (tok->type == Nst_TT_CALL) {
        Nst_LList *nodes = Nst_llist_new();
        ADD_POS_AND_QUIT_IF_ERR();
        Nst_Node__old *call = parse_local_stack_op(nodes, tok->start);
        if (call == NULL)
            Nst_llist_destroy(nodes, NULL);
        DEC_RECURSION_LVL;
        return call;
    } else if (tok->type == Nst_TT_L_VBRACE) {
        DEC_RECURSION_LVL;
        return parse_vector_literal();
    } else if (tok->type == Nst_TT_L_BRACE) {
        DEC_RECURSION_LVL;
        return parse_arr_or_map_literal();
    } else if (tok->type == Nst_TT_LAMBDA) {
        DEC_RECURSION_LVL;
        return parse_func_declr();
    } else {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        Nst_tok_destroy(tok);
        Nst_llist_pop(tokens);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_VALUE);
    }
}

static Nst_Node__old *parse_vector_literal(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = POP_HEAD_TOK;
    Nst_Pos start = tok->start;
    Nst_Pos err_end = tok->end;
    Nst_tok_destroy(tok);
    Nst_Node__old *vect_node = Nst_node_new__old(Nst_NT_VEC_LIT);
    ADD_POS_AND_QUIT_IF_ERR();

    if (HEAD_TOK->type == Nst_TT_R_VBRACE) {
        tok = POP_HEAD_TOK;
        Nst_node_set_pos__old(vect_node, start, tok->end);
        Nst_tok_destroy(tok);
        DEC_RECURSION_LVL;
        return vect_node;
    }

    while (true) {
        Nst_Node__old *value = parse_expr(true);
        RETURN_IF_ERROR(vect_node);
        SAFE_NODE_APPEND(vect_node, value);

        skip_blank();
        tok = POP_HEAD_TOK;

        if (tok->type == Nst_TT_BREAK && vect_node->nodes->len == 1) {
            SAFE_TOK_APPEND(vect_node, tok);
            skip_blank();

            value = parse_expr(false);
            RETURN_IF_ERROR(vect_node);
            SAFE_NODE_APPEND(vect_node, value);

            skip_blank();
            tok = POP_HEAD_TOK;

            if (tok->type != Nst_TT_R_VBRACE) {
                Nst_node_destroy__old(vect_node);
                Nst_Pos tok_start = tok->start;
                Nst_Pos tok_end = tok->end;
                Nst_tok_destroy(tok);
                RETURN_ERROR(tok_start, tok_end, _Nst_EM_EXPECTED_VBRACE);
            }
            Nst_tok_destroy(tok);
            Nst_node_set_pos__old(vect_node, start, tok->end);
            DEC_RECURSION_LVL;
            return vect_node;
        } else if (tok->type == Nst_TT_COMMA) {
            Nst_tok_destroy(tok);
            skip_blank();
        } else
            break;
    }

    if (tok->type != Nst_TT_R_VBRACE) {
        Nst_node_destroy__old(vect_node);
        Nst_tok_destroy(tok);
        RETURN_ERROR(start, err_end, _Nst_EM_MISSING_VBRACE);
    }

    Nst_node_set_pos__old(vect_node, start, tok->end);
    Nst_tok_destroy(tok);
    DEC_RECURSION_LVL;
    return vect_node;
}

static Nst_Node__old *parse_arr_or_map_literal(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = POP_HEAD_TOK;
    Nst_Pos start = tok->start;
    bool is_map = false;
    usize count = 0;

    Nst_tok_destroy(tok);
    skip_blank();

    Nst_Node__old *node = Nst_node_new__old(Nst_NT_MAP_LIT);
    ADD_POS_AND_QUIT_IF_ERR();

    if (HEAD_TOK->type == Nst_TT_R_BRACE) {
        tok = POP_HEAD_TOK;
        Nst_node_set_pos__old(node, start, tok->end);
        Nst_tok_destroy(tok);
        DEC_RECURSION_LVL;
        return node;
    }

    skip_blank();

    if (HEAD_TOK->type == Nst_TT_COMMA) {
        DESTROY_HEAD_TOK;
        skip_blank();
        tok = POP_HEAD_TOK;
        Nst_Pos end = tok->end;
        i32 type = tok->type;
        Nst_tok_destroy(tok);

        if (type != Nst_TT_R_BRACE) {
            Nst_node_destroy__old(node);
            RETURN_ERROR(start, end, _Nst_EM_EXPECTED_BRACE);
        }
        node->type = Nst_NT_ARR_LIT;
        Nst_node_set_pos__old(node, start, tok->end);
        DEC_RECURSION_LVL;
        return node;
    }

    while (true) {
        skip_blank();

        Nst_Node__old *value = parse_expr(true);
        RETURN_IF_ERROR(node);
        SAFE_NODE_APPEND(node, value);

        skip_blank();
        tok = POP_HEAD_TOK;

        if (tok->type == Nst_TT_COLON && (count == 0 || is_map)) {
            is_map = true;

            Nst_tok_destroy(tok);
            skip_blank();

            value = parse_expr(false);
            RETURN_IF_ERROR(node);
            SAFE_NODE_APPEND(node, value);

            skip_blank();
            tok = POP_HEAD_TOK;
        } else if (tok->type == Nst_TT_BREAK && count == 0) {
            SAFE_TOK_APPEND(node, tok);
            skip_blank();

            value = parse_expr(false);
            RETURN_IF_ERROR(node);
            SAFE_NODE_APPEND(node, value);

            skip_blank();
            tok = POP_HEAD_TOK;

            if (tok->type != Nst_TT_R_BRACE) {
                Nst_node_destroy__old(node);
                Nst_Pos err_start = tok->start;
                Nst_Pos err_end = tok->end;
                Nst_tok_destroy(tok);
                RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_BRACE);
            }
            Nst_node_set_pos__old(node, start, tok->end);
            Nst_tok_destroy(tok);
            node->type = Nst_NT_ARR_LIT;
            DEC_RECURSION_LVL;
            return node;
        // if it's not the first iteration and it's not supposed to be a map
        } else if (tok->type == Nst_TT_COLON) {
            Nst_node_destroy__old(node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            Nst_tok_destroy(tok);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_COMMA_OR_BRACE);
        } else if (tok->type != Nst_TT_COLON && is_map) {
            Nst_node_destroy__old(node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            Nst_tok_destroy(tok);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_COLON);
        }

        if (tok->type == Nst_TT_R_BRACE) {
            if (!is_map)
                node->type = Nst_NT_ARR_LIT;
            Nst_node_set_pos__old(node, start, tok->end);
            Nst_tok_destroy(tok);
            DEC_RECURSION_LVL;
            return node;
        } else if (tok->type != Nst_TT_COMMA) {
            Nst_node_destroy__old(node);
            Nst_Pos err_start = tok->start;
            Nst_Pos err_end = tok->end;
            Nst_tok_destroy(tok);
            RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_COMMA_OR_BRACE);
        }

        Nst_tok_destroy(tok);
        count++;
    }
}

static Nst_Node__old *parse_try_catch(void)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = POP_HEAD_TOK;
    Nst_Pos start = tok->start;
    Nst_tok_destroy(tok);
    skip_blank();

    Nst_Node__old *try_catch_node = Nst_node_new__old(Nst_NT_TRY_CATCH_S);
    ADD_POS_AND_QUIT_IF_ERR();

    Nst_Node__old *try_block = parse_statement();
    RETURN_IF_ERROR(try_catch_node);
    SAFE_NODE_APPEND(try_catch_node, try_block);
    skip_blank();

    tok = POP_HEAD_TOK;
    if (tok->type != Nst_TT_CATCH) {
        Nst_Pos err_start = tok->start;
        Nst_Pos err_end = tok->end;
        Nst_tok_destroy(tok);
        Nst_node_destroy__old(try_catch_node);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_CATCH);
    }
    Nst_tok_destroy(tok);
    skip_blank();

    Nst_Tok *name_tok = POP_HEAD_TOK;
    if (name_tok->type != Nst_TT_IDENT) {
        Nst_Pos err_start = name_tok->start;
        Nst_Pos err_end = name_tok->end;
        Nst_tok_destroy(name_tok);
        Nst_node_destroy__old(try_catch_node);
        RETURN_ERROR(err_start, err_end, _Nst_EM_EXPECTED_IDENT);
    }
    SAFE_TOK_APPEND(try_catch_node, name_tok);
    skip_blank();

    Nst_Node__old *catch_block = parse_statement();
    RETURN_IF_ERROR(try_catch_node);
    SAFE_NODE_APPEND(try_catch_node, catch_block);

    Nst_node_set_pos__old(try_catch_node, start, catch_block->end);
    DEC_RECURSION_LVL;
    return try_catch_node;
}

static void _print_ast(Nst_Node__old *node, Nst_Tok *tok, i32 lvl,
                       Nst_LList *is_last)
{
    Nst_LLNode *cursor = NULL;

    if (lvl > 0) {
        cursor = is_last->head;
        // Until, but excluding, the last node
        for (cursor = is_last->head;
             cursor->next != NULL;
             cursor = cursor->next)
        {
            if (cursor->value)
                Nst_print("   ");
            else
                Nst_print("\xE2\x94\x82  ");
        }
        if (cursor->value)
            Nst_print("\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80");
        else
            Nst_print("\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80");
    }

    if (tok != NULL) {
        Nst_print_tok(tok);
        Nst_print("\n");
        return;
    }

    switch (node->type) {
    case Nst_NT_LONG_S:         Nst_print("LONG_S");         break;
    case Nst_NT_WHILE_L:        Nst_print("WHILE_L");        break;
    case Nst_NT_DOWHILE_L:      Nst_print("DOWHILE_L");      break;
    case Nst_NT_FOR_L:          Nst_print("FOR_L");          break;
    case Nst_NT_FOR_AS_L:       Nst_print("FOR_AS_L");       break;
    case Nst_NT_IF_E:           Nst_print("IF_E");           break;
    case Nst_NT_FUNC_DECLR:     Nst_print("FUNC_DECLR");     break;
    case Nst_NT_RETURN_S:       Nst_print("RETURN_S");       break;
    case Nst_NT_STACK_OP:       Nst_print("STACK_OP");       break;
    case Nst_NT_LOCAL_STACK_OP: Nst_print("LOCAL_STACK_OP"); break;
    case Nst_NT_LOCAL_OP:       Nst_print("LOCAL_OP");       break;
    case Nst_NT_ARR_LIT:        Nst_print("ARR_LIT");        break;
    case Nst_NT_VEC_LIT:        Nst_print("VEC_LIT");        break;
    case Nst_NT_MAP_LIT:        Nst_print("MAP_LIT");        break;
    case Nst_NT_VALUE:          Nst_print("VALUE");          break;
    case Nst_NT_ACCESS:         Nst_print("ACCESS");         break;
    case Nst_NT_EXTRACT_E:      Nst_print("EXTRACT_E");      break;
    case Nst_NT_ASSIGN_E:       Nst_print("ASSIGN_E");       break;
    case Nst_NT_CONTINUE_S:     Nst_print("CONTINUE_S");     break;
    case Nst_NT_BREAK_S:        Nst_print("BREAK_S");        break;
    case Nst_NT_SWITCH_S:       Nst_print("SWITCH_S");       break;
    case Nst_NT_LAMBDA:         Nst_print("LAMBDA");         break;
    case Nst_NT_TRY_CATCH_S:    Nst_print("TRY_CATCH_S");    break;
    default:                    Nst_print("__UNKNOWN__");    break;
    }

    Nst_printf(
        " (%li:%li, %li:%li)\n",
        node->start.line,
        node->start.col,
        node->end.line,
        node->end.col);

    usize tot_len = node->nodes->len + node->tokens->len - 1;
    usize idx = 0;

    Nst_LLNode *prev_tail = is_last->tail;
    if (!Nst_llist_append(is_last, NULL, false)) {
        Nst_error_clear();
        return;
    }

    for (cursor = node->tokens->head; cursor != NULL; cursor = cursor->next) {
#ifdef _Nst_ARCH_x64
        is_last->tail->value = (void *)(i64)(idx == tot_len);
#else
        is_last->tail->value = (void *)(idx == tot_len);
#endif
        _print_ast(NULL, Nst_TOK(cursor->value), lvl + 1, is_last);
        idx++;
    }

    idx = 0;

    for (cursor = node->nodes->head; cursor != NULL; cursor = cursor->next) {
#ifdef _Nst_ARCH_x64
        is_last->tail->value = (void *)(i64)(idx == node->nodes->len - 1);
#else
        is_last->tail->value = (void *)(idx == node->nodes->len - 1);
#endif
        _print_ast(Nst_NODE(cursor->value), NULL, lvl + 1, is_last);
        idx++;
    }

    Nst_free(is_last->tail);
    is_last->tail = prev_tail;
    if (prev_tail == NULL)
        is_last->head = NULL;
    else
        prev_tail->next = NULL;
}

void Nst_print_ast(Nst_Node__old *ast)
{
    Nst_LList *is_last = Nst_llist_new();
    if (is_last == NULL)
        return;

    _print_ast(ast, NULL, 0, is_last);
    Nst_llist_destroy(is_last, NULL);
}
#endif
;