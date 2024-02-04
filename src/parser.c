#include "parser.h"
#include "global_consts.h"

typedef struct _ParsingState {
    bool in_func;
    bool in_loop;
    bool in_switch;
    bool endl_ends_expr;
    bool break_ends_expr;
    int recursion_lvl;
    Nst_LList *tokens;
} ParsingState;

ParsingState state;

static inline bool enter_func(ParsingState *initial_state);
static inline void exit_func(ParsingState *initial_state);
static inline bool append_node(Nst_LList *llist, Nst_Node *node);
static inline bool append_tok(Nst_LList *llist, Nst_Tok *node);

static inline Nst_Tok *peek_top(void);
static inline Nst_TokType top_type(void);
static inline Nst_Pos top_start(void);
static inline Nst_Pos top_end(void);
static inline Nst_Tok *pop_top(void);
static inline void destroy_top(void);

static Nst_Node *new_node(Nst_NodeType type, Nst_Pos start, Nst_Pos end);

static inline void set_error(const char *msg, Nst_Pos start, Nst_Pos end);
static inline void skip_blank(void);
static bool check_local_stack_op_arg_num(usize arg_num, Nst_Pos start,
                                         Nst_Pos end);

static Nst_Node *parse_long_s(void);
static Nst_Node *parse_long_s_with_brackets(void);
static Nst_Node *parse_statement(void);
static Nst_Node *parse_while_l(void);
static Nst_Node *parse_for_l(void);
static Nst_Node *parse_expr(void);
static Nst_Node *parse_switch_s(void);
static Nst_Node *parse_func_declr(void);
static Nst_Node *parse_stack_expr(void);
static Nst_Node *parse_stack_op(Nst_LList **values, Nst_Pos start);
static Nst_Node *parse_local_stack_op(Nst_LList **values, Nst_Pos start);
static Nst_Node *parse_assignment_name(bool is_compound);
static Nst_Node *parse_assignment(Nst_LList **values, Nst_Pos start);
static Nst_Node *parse_extraction(void);
static Nst_Node *parse_atom(void);
static Nst_Node *parse_vector_literal(void);
static Nst_Node *parse_arr_or_map_literal(void);
static Nst_Node *parse_seq_body(Nst_Pos start, Nst_Node *first_node, bool arr);
static Nst_Node *parse_map_body(Nst_Pos start, Nst_Node *key);
static Nst_Node *parse_try_catch(void);

Nst_Node *Nst_parse(Nst_LList *tokens)
{
    if (tokens == NULL)
        return NULL;

    state.in_func = false;
    state.in_loop = false;
    state.in_switch = false;
    state.recursion_lvl = 0;
    state.tokens = tokens;

    Nst_Node *ast = parse_long_s();

    if (ast != NULL && state.tokens->len > 1) {
        Nst_Pos start = Nst_TOK(Nst_llist_peek_front(state.tokens))->start;
        Nst_Pos end = Nst_TOK(Nst_llist_peek_front(state.tokens))->end;

        Nst_set_internal_syntax_error_c(
            Nst_error_get(),
            start, end,
            _Nst_EM_UNEXPECTED_TOK);
        Nst_node_destroy(ast);
        ast = NULL;
    }
    Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
    return ast;
}

static inline bool enter_func(ParsingState *initial_state)
{
    initial_state->in_func = state.in_func;
    initial_state->in_loop = state.in_loop;
    initial_state->in_switch = state.in_switch;
    initial_state->endl_ends_expr = state.endl_ends_expr;
    initial_state->recursion_lvl = state.recursion_lvl;
    initial_state->break_ends_expr = state.break_ends_expr;

    state.recursion_lvl++;
    if (state.recursion_lvl > 1500) {
        Nst_Pos start = top_start();
        Nst_Pos end = top_end();
        Nst_set_memory_error_c(_Nst_EM_RECURSIVE_CALLS("1500"));
        Nst_error_add_positions(Nst_error_get(), start, end);
        return false;
    }

    return true;
}

static inline void exit_func(ParsingState *initial_state)
{
    state.in_func = initial_state->in_func;
    state.in_loop = initial_state->in_loop;
    state.in_switch = initial_state->in_switch;
    state.endl_ends_expr = initial_state->endl_ends_expr;
    state.break_ends_expr = initial_state->break_ends_expr;
    state.recursion_lvl = initial_state->recursion_lvl;
}

static inline bool append_node(Nst_LList *llist, Nst_Node *node)
{
    bool result = Nst_llist_append(llist, node, true);
    if (!result)
        Nst_error_add_positions(Nst_error_get(), node->start, node->end);
    return result;
}

static inline bool append_tok(Nst_LList *llist, Nst_Tok *tok)
{
    bool result = Nst_llist_append(llist, tok, true);
    if (!result)
        Nst_error_add_positions(Nst_error_get(), tok->start, tok->end);
    return result;
}

// Returns the head token on the tokens list
static inline Nst_Tok *peek_top(void)
{
    return Nst_TOK(state.tokens->head);
}

// Returns the type of the head token on the tokens list
static inline Nst_TokType top_type(void)
{
    Nst_Tok *head = peek_top();
    if (head == NULL)
        return Nst_TT_INVALID;
    return head->type;
}

// Returns the starting position of the head token on the tokens list
static inline Nst_Pos top_start(void)
{
    Nst_Tok *head = peek_top();
    if (head == NULL)
        return Nst_no_pos();
    return head->start;
}

// Returns the ending position of the head token on the tokens list
static inline Nst_Pos top_end(void)
{
    Nst_Tok *head = peek_top();
    if (head == NULL)
        return Nst_no_pos();
    return head->end;
}

// Pops the head token of the tokens list and returns it
static inline Nst_Tok *pop_top(void)
{
    return Nst_TOK(Nst_llist_pop(state.tokens));
}

// Pops the head token of the tokens list and destroys it
static inline void destroy_top(void)
{
    Nst_Tok *head = pop_top();
    if (head != NULL)
        Nst_tok_destroy(head);
}

// Sets a syntax error with the given message and adds the positions
static inline void set_error(const char *msg, Nst_Pos start, Nst_Pos end)
{
    Nst_set_syntax_error_c(msg);
    Nst_error_add_positions(Nst_error_get(), start, end);
}

// Creates a new node adding the given positions if an error occurs.
// The positions themselves are not added to the node and must be set manually.
static Nst_Node *new_node(Nst_NodeType type, Nst_Pos start, Nst_Pos end)
{
    Nst_Node *node = Nst_node_new(type);
    if (node == NULL)
        Nst_error_add_positions(Nst_error_get(), start, end);
    return node;
}

static inline void skip_blank(void)
{
    while (top_type() == Nst_TT_ENDL)
        destroy_top();
}

static Nst_Node *parse_long_s(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Node *long_s = Nst_node_new(Nst_NT_LONG_S);
    Nst_Node *statement = NULL;

    if (long_s == NULL)
        goto failure;
    skip_blank();

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();

    state.endl_ends_expr = true;
    state.break_ends_expr = false;

    while (top_type() != Nst_TT_R_BRACKET && top_type() != Nst_TT_EOFILE)
    {
        statement = parse_statement();
        if (statement == NULL)
            goto failure;
        if (!append_node(long_s->long_s.statements, statement))
            goto failure;
        skip_blank();
    }

    if (long_s->long_s.statements->len != 0) {
        start = Nst_NODE(long_s->long_s.statements->head->value)->start;
        end = Nst_NODE(long_s->long_s.statements->tail->value)->start;
    }
    Nst_node_set_pos(long_s, start, end);

    exit_func(&initial_state);
    return long_s;

failure:
    if (long_s != NULL)
        Nst_node_destroy(long_s);
    if (statement != NULL)
        Nst_node_destroy(statement);
    return NULL;
}

static Nst_Node *parse_long_s_with_brackets(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    skip_blank();

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    if (top_type() != Nst_TT_L_BRACKET) {
        set_error(_Nst_EM_EXPECTED_BRACKET, start, end);
        return NULL;
    }
    destroy_top();
    Nst_Node *long_s = parse_long_s();
    if (long_s == NULL)
        return NULL;

    skip_blank();

    if (top_type() != Nst_TT_R_BRACKET) {
        set_error(_Nst_EM_MISSING_BRACKET, start, end);
        return NULL;
    }
    end = top_end();
    destroy_top();

    Nst_Node *wrapper = new_node(Nst_NT_S_WRAPPER, start, end);
    if (wrapper == NULL) {
        Nst_node_destroy(long_s);
        return NULL;
    }
    wrapper->s_wrapper.statement = long_s;
    Nst_node_set_pos(wrapper, start, end);

    exit_func(&initial_state);
    return wrapper;
}

static Nst_Node *parse_statement(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    skip_blank();
    Nst_TokType tok_type = top_type();
    if (tok_type == Nst_TT_L_BRACKET) {
        Nst_Pos start = top_start();
        destroy_top();

        Nst_Node *long_s = parse_long_s();
        if (long_s == NULL)
            return NULL;

        if (top_type() != Nst_TT_R_BRACKET) {
            Nst_node_destroy(long_s);
            set_error(_Nst_EM_MISSING_BRACKET, start, start);
            return NULL;
        }
        destroy_top();
        exit_func(&initial_state);
        return long_s;
    } else if (tok_type == Nst_TT_WHILE || tok_type == Nst_TT_DOWHILE) {
        Nst_Node *while_l = parse_while_l(state);
        exit_func(&initial_state);
        return while_l;
    } else if (tok_type == Nst_TT_FOR) {
        Nst_Node *for_l = parse_for_l(state);
        exit_func(&initial_state);
        return for_l;
    } else if (tok_type == Nst_TT_FUNC) {
        Nst_Node *func_declr = parse_func_declr(state);
        exit_func(&initial_state);
        return func_declr;
    } else if (tok_type == Nst_TT_SWITCH) {
        Nst_Node *switch_s = parse_switch_s(state);
        exit_func(&initial_state);
        return switch_s;
    } else if (tok_type == Nst_TT_RETURN) {
        Nst_Pos start = top_start();
        Nst_Pos end = top_end();
        destroy_top();

        if (!state.in_func) {
            set_error(_Nst_EM_BAD_RETURN, start, end);
            return NULL;
        }

        Nst_Node *expr;
        Nst_Pos expr_end;

        tok_type = top_type();
        if (Nst_IS_EXPR_END(tok_type)) {
            expr = NULL;
            expr_end = end;
        } else {
            expr = parse_expr();
            if (expr == NULL)
                return NULL;
            expr_end = expr->end;
        }

        Nst_Node *return_s = new_node(Nst_NT_RETURN_S, start, expr_end);
        if (return_s == NULL) {
            if (expr != NULL)
                Nst_node_destroy(expr);
            return NULL;
        }
        Nst_node_set_pos(return_s, start, expr_end);
        return_s->return_s.value = expr;
        exit_func(&initial_state);
        return return_s;
    } else if (tok_type == Nst_TT_CONTINUE) {
        Nst_Pos start = top_start();
        Nst_Pos end = top_end();
        destroy_top();

        if (!state.in_loop && !state.in_switch) {
            set_error(_Nst_EM_BAD_CONTINUE, start, end);
            return NULL;
        }

        Nst_Node *continue_s = new_node(Nst_NT_CONTINUE_S, start, end);
        if (continue_s == NULL)
            return NULL;
        Nst_node_set_pos(continue_s, start, end);
        exit_func(&initial_state);
        return continue_s;
    } else if (tok_type == Nst_TT_BREAK) {
        Nst_Pos start = top_start();
        Nst_Pos end = top_end();
        destroy_top();

        if (!state.in_loop) {
            set_error(_Nst_EM_BAD_BREAK, start, end);
            return NULL;
        }

        Nst_Node *break_s = new_node(Nst_NT_BREAK_S, start, end);
        if (break_s == NULL)
            return NULL;
        Nst_node_set_pos(break_s, start, end);
        exit_func(&initial_state);
        return break_s;
    } else if (tok_type == Nst_TT_TRY) {
        Nst_Node *try_catch_s = parse_try_catch();
        exit_func(&initial_state);
        return try_catch_s;
    } else if (Nst_IS_ATOM(tok_type)) {
        Nst_Node *expr = parse_expr();
        exit_func(&initial_state);
        return expr;
    } else {
        Nst_Pos start = top_start();
        Nst_Pos end = top_end();
        destroy_top();
        set_error(_Nst_EM_UNEXPECTED_TOK, start, end);
    }
}

static Nst_Node *parse_while_l(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    Nst_Node *while_l = new_node(Nst_NT_WHILE_L, start, end);
    if (while_l == NULL)
        return NULL;
    while_l->while_l.is_dowhile = top_type() == Nst_TT_DOWHILE;
    destroy_top();
    skip_blank();
    state.endl_ends_expr = false;
    Nst_Node *condition = parse_expr();
    state.endl_ends_expr = initial_state.endl_ends_expr;
    if (condition == NULL) {
        Nst_node_destroy(while_l);
        return NULL;
    }
    while_l->while_l.condition = condition;

    Nst_Node *body = parse_long_s_with_brackets();
    if (body == NULL) {
        Nst_node_destroy(while_l);
        return NULL;
    }
    while_l->while_l.body = body;
    Nst_node_set_pos(while_l, start, body->end);
    exit_func(&initial_state);
    return while_l;
}

static Nst_Node *parse_for_l(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    Nst_Node *for_l = new_node(Nst_NT_FOR_L, start, end);
    if (for_l == NULL)
        return NULL;
    skip_blank();
    state.endl_ends_expr = false;
    Nst_Node *iterator = parse_expr();
    state.endl_ends_expr = initial_state.endl_ends_expr;
    if (iterator == NULL) {
        Nst_node_destroy(for_l);
        return NULL;
    }
    for_l->for_l.iterator = iterator;
    skip_blank();
    if (top_type() == Nst_TT_AS) {
        destroy_top();
        Nst_Node *assignment = parse_assignment_name(false);
        if (assignment == NULL) {
            Nst_node_destroy(for_l);
            return NULL;
        }
        for_l->for_l.assignment = assignment;
        skip_blank();
    }

    Nst_Node *body = parse_long_s_with_brackets();
    if (body == NULL) {
        Nst_node_destroy(for_l);
        return NULL;
    }
    for_l->for_l.body = body;
    Nst_node_set_pos(for_l, start, body->end);
    exit_func(&initial_state);
    return for_l;
}

static Nst_Node *parse_expr(void)
{
    ParsingState *initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Node *condition = parse_stack_expr();
    if (condition == NULL)
        return NULL;

    if (!state.endl_ends_expr)
        skip_blank();

    if (top_type() != Nst_TT_IF) {
        exit_func(&initial_state);
        return condition;
    }
    destroy_top();
    Nst_Node *if_e = new_node(Nst_NT_IF_E, condition->start, condition->end);
    if (if_e == NULL) {
        Nst_node_destroy(condition);
        return NULL;
    }
    if_e->if_e.condition = condition;
    skip_blank();
    Nst_Node *body_if_true = parse_statement();
    if (body_if_true == NULL) {
        Nst_node_destroy(if_e);
        return NULL;
    }
    if_e->if_e.body_if_true = body_if_true;
    skip_blank();
    if (top_type() != Nst_TT_COLON) {
        Nst_node_set_pos(if_e, condition->start, body_if_true->end);
        exit_func(&initial_state);
        return if_e;
    }
    destroy_top();
    skip_blank();
    Nst_Node *body_if_false = parse_statement();
    if (body_if_false == NULL) {
        Nst_node_destroy(if_e);
        return NULL;
    }
    if_e->if_e.body_if_false = body_if_false;
    Nst_node_set_pos(if_e, condition->start, body_if_false->end);
    exit_func(&initial_state);
    return if_e;
}

static Nst_Node *parse_switch_s(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    destroy_top();

    Nst_Node *switch_s = new_node(Nst_NT_SWITCH_S, start, end);
    if (switch_s == NULL)
        return NULL;

    state.endl_ends_expr = false;
    Nst_Node *expr = parse_expr();
    state.endl_ends_expr = initial_state.endl_ends_expr;
    if (expr == NULL) {
        Nst_node_destroy(switch_s);
        return NULL;
    }
    switch_s->switch_s.expr = expr;
    skip_blank();
    if (top_type() != Nst_TT_L_BRACKET) {
        Nst_node_destroy(switch_s);
        set_error(_Nst_EM_EXPECTED_BRACKET, top_start(), top_end());
        return NULL;
    }
    destroy_top();
    while (true) {
        skip_blank();
        if (top_type() == Nst_TT_R_BRACKET)
            break;
        if (top_type() != Nst_TT_IF) {
            Nst_node_destroy(switch_s);
            set_error(_Nst_EM_EXPECTED_IF, top_start(), top_end());
            return NULL;
        }
        destroy_top();
        skip_blank();

        if (top_type() == Nst_TT_L_BRACKET) {
            Nst_Node *default_body = parse_long_s_with_brackets();
            if (default_body == NULL) {
                Nst_node_destroy(switch_s);
                return NULL;
            }
            switch_s->switch_s.default_body = default_body;
            break;
        }
        state.endl_ends_expr = false;
        Nst_Node *value = parse_expr();
        state.endl_ends_expr = initial_state.endl_ends_expr;
        if (value == NULL) {
            Nst_node_destroy(switch_s);
            return NULL;
        }
        if (!append_node(switch_s->switch_s.values, value)) {
            Nst_node_destroy(value);
            Nst_node_destroy(switch_s);
            return NULL;
        }
        Nst_Node *body = parse_long_s_with_brackets();
        if (body == NULL) {
            Nst_node_destroy(switch_s);
            return NULL;
        }
        if (!append_node(switch_s->switch_s.bodies, body)) {
            Nst_node_destroy(body);
            Nst_node_destroy(switch_s);
            return NULL;
        }
    }

    if (top_type() != Nst_TT_R_BRACKET) {
        Nst_node_destroy(switch_s);
        set_error(_Nst_EM_EXPECTED_R_BRACKET, top_start(), top_end());
        return NULL;
    }
    Nst_node_set_pos(switch_s, start, top_end());
    destroy_top();
    exit_func(&initial_state);
    return switch_s;
}

static Nst_Node *parse_func_declr(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    Nst_Node *func_declr = new_node(Nst_NT_FUNC_DECLR, start, end);
    if (func_declr == NULL)
        return NULL;

    bool is_lambda = top_type() == Nst_TT_LAMBDA;
    destroy_top();
    skip_blank();
    if (!is_lambda && top_type() != Nst_TT_IDENT) {
        Nst_node_destroy(func_declr);
        set_error(_Nst_EM_EXPECTED_IDENT, top_start(), top_end());
        return NULL;
    }
    if (!is_lambda)
        func_declr->func_declr.name = pop_top();

    skip_blank();
    while (top_type() == Nst_TT_IDENT) {
        Nst_Tok *tok = peek_top();
        if (!append_tok(func_declr->func_declr.argument_names, tok)) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        pop_top();
        skip_blank();
    }

    if (top_type() == Nst_TT_RETURN) {
        Nst_Pos body_start = top_start();
        Nst_Node *return_s = new_node(Nst_NT_RETURN_S, body_start, top_end());
        if (return_s == NULL) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        func_declr->func_declr.body = return_s;
        state.endl_ends_expr = false;
        Nst_Node *expr = parse_expr();
        state.endl_ends_expr = initial_state.endl_ends_expr;
        if (expr == NULL) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        return_s->return_s.value = expr;
        Nst_node_set_pos(return_s, body_start, expr->end);
        end = expr->end;
    } else {
        Nst_Node *body = parse_long_s_with_brackets();
        if (body == NULL) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        func_declr->func_declr.body = body;
        end = body->end;
    }
    Nst_node_set_pos(func_declr, start, end);
    exit_func(&initial_state);
    return func_declr;
}

static Nst_Node *parse_stack_expr()
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_LList *values = Nst_llist_new();
    if (values == NULL) {
        Nst_error_add_positions(Nst_error_get(), top_start(), top_end());
        return NULL;
    }

    Nst_Pos start = top_start();

    while (true) {
        if (!state.endl_ends_expr)
            skip_blank();

        while (Nst_IS_ATOM(top_type())) {
            Nst_Node *atom = parse_extraction();
            if (atom == NULL)
                goto failure;
            if (!append_node(values, atom)) {
                Nst_node_destroy(atom);
                goto failure;
            }
            if (!state.endl_ends_expr)
                skip_blank();
        }

        // this can be true only in the first iteration, all other subsequent
        // iterations will have at least the values from the previous one
        if (values->len == 0 && !Nst_IS_LOCAL_STACK_OP(top_type())) {
            set_error(_Nst_EM_EXPECTED_VALUE, top_start(), top_end());
            goto failure;
        }

        Nst_Node *operation_node;
        if (Nst_IS_STACK_OP(top_type()))
            operation_node = parse_stack_op(&values, start);
        else if (Nst_IS_LOCAL_STACK_OP(top_type()))
            operation_node = parse_local_stack_op(&values, start);
        else if (Nst_IS_ASSIGNMENT(top_type()))
            operation_node = parse_assignment(&values, start);
        else if (Nst_IS_EXPR_END(top_type()) && state.endl_ends_expr)
            break;
        else if (Nst_IS_EXPR_END(top_type()) && top_type() != Nst_TT_ENDL)
            break;
        else if (state.break_ends_expr && top_type() != Nst_TT_BREAK)
            break;

        if (operation_node == NULL)
            goto failure;

        if (!append_node(values, operation_node)) {
            Nst_node_destroy(operation_node);
            goto failure;
        }
    }

    // The expression end token must not be consumed

    if (values->len != 1) {
        set_error(_Nst_EM_EXPECTED_OP, top_start(), top_end());
        goto failure;
    }

    Nst_Node *expr = Nst_llist_pop(values);
    Nst_llist_destroy(values, NULL);
    exit_func(&initial_state);
    return expr;

failure:
    Nst_llist_destroy(values, (Nst_LListDestructor)Nst_node_destroy);
    return NULL;
}

static Nst_Node *parse_stack_op(Nst_LList **values, Nst_Pos start)
{
    Nst_Node *stack_op = new_node(Nst_NT_STACK_OP, top_start(), top_end());
    if (stack_op == NULL)
        return NULL;
    Nst_node_set_pos(stack_op, start, top_end());
    Nst_LList *temp = *values;
    *values = stack_op->stack_op.values;
    stack_op->stack_op.values = temp;
    stack_op->stack_op.op = top_type();
    destroy_top();
    return stack_op;
}

static Nst_Node *parse_local_stack_op(Nst_LList **values, Nst_Pos start)
{
    Nst_Pos args_end;
    if ((*values)->len == 0)
        args_end = start;
    else
        args_end = Nst_NODE((*values)->tail->value)->end;

    usize arg_num = (*values)->len;
    if (!check_local_stack_op_arg_num(arg_num, start, args_end))
        return NULL;
    Nst_TokType op = top_type();
    destroy_top();
    skip_blank();
    Nst_Node *special_node = parse_extraction();
    if (special_node == NULL)
        return NULL;
    Nst_Node *local_stack_op = new_node(
        Nst_NT_LOCAL_STACK_OP,
        start,
        special_node->end);
    if (local_stack_op == NULL) {
        Nst_node_destroy(special_node);
        return NULL;
    }
    Nst_node_set_pos(local_stack_op, start, special_node->end);
    Nst_LList *temp = *values;
    *values = local_stack_op->local_stack_op.values;
    local_stack_op->local_stack_op.values = temp;
    local_stack_op->local_stack_op.special_value = special_node;
    local_stack_op->local_stack_op.op = op;
    return local_stack_op;
}

static bool check_local_stack_op_arg_num(usize arg_num, Nst_Pos start,
                                         Nst_Pos end)
{
    Nst_NodeType type = top_type();
    if (type == Nst_TT_CAST && arg_num != 1) {
        set_error(_Nst_EM_LEFT_ARGS_NUM("::", "1", ""), start, end);
        return false;
    } else if (type == Nst_TT_RANGE && arg_num != 1 && arg_num != 2) {
        set_error(_Nst_EM_LEFT_ARGS_NUM("->", "1 or 2", "s"), start, end);
        return false;
    } else if (type == Nst_TT_THROW && arg_num != 1) {
        set_error(_Nst_EM_LEFT_ARGS_NUM("!!", "1", ""), start, end);
        return false;
    } else if (type == Nst_TT_SEQ_CALL && arg_num != 1) {
        set_error(_Nst_EM_LEFT_ARGS_NUM("*@", "1", ""), start, end);
        return false;
    }
    return true;
}

static Nst_Node *parse_assignment_name(bool is_compound)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = start;
    Nst_Node *name;

    if (top_type() != Nst_TT_L_BRACE) {
        name = parse_extraction();
        if (name == NULL)
            return NULL;
        if (name->type != Nst_NT_ACCESS && name->type != Nst_NT_EXTRACT_E) {
            set_error(_Nst_EM_EXPECTED_IDENT_OR_EXTR, name->start, name->end);
            Nst_node_destroy(name);
            return NULL;
        }
        exit_func(&initial_state);
        return name;
    }

    if (is_compound) {
        set_error(_Nst_EM_COMPOUND_ASSIGMENT, top_start(), top_end());
        return NULL;
    }

    name = new_node(Nst_NT_SEQ_LIT, top_start(), top_end());
    if (name == NULL)
        return NULL;
    name->seq_lit.type = Nst_SNT_ASSIGNMENT_NAMES;
    destroy_top();

    state.endl_ends_expr = false;

    while (true) {
        skip_blank();
        Nst_Node *sub_name = parse_assignment_name(false);
        if (sub_name == NULL)
            goto failure;
        if (!append_node(name->seq_lit.values, sub_name)) {
            Nst_node_destroy(sub_name);
            goto failure;
        }
        skip_blank();
        if (top_type() == Nst_TT_COMMA)
            destroy_top();
        else if (top_type() == Nst_TT_R_BRACE) {
            end = top_end();
            destroy_top();
            break;
        } else {
            set_error(_Nst_EM_EXPECTED_COMMA_OR_BRACE, top_start(), top_end());
            goto failure;
        }
    }
    Nst_node_set_pos(name, start, end);
    exit_func(&initial_state);
    return name;

failure:
    if (name != NULL)
        Nst_node_destroy(name);
    return NULL;
}

static Nst_Node *parse_assignment(Nst_LList **values, Nst_Pos start)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_TokType type = top_type();
    bool is_compound = type != Nst_TT_ASSIGN;

    if ((*values)->len > 1 && is_compound) {
        set_error(_Nst_EM_EXPECTED_OP, top_start(), top_end());
        return NULL;
    }

    Nst_Node *name = parse_assignment_name(is_compound);
    if (name == NULL)
        return NULL;

    Nst_Node *assignment = new_node(
        is_compound ? Nst_NT_COMP_ASSIGN_E : Nst_NT_ASSIGN_E,
        start,
        name->end);

    if (assignment == NULL) {
        Nst_node_destroy(name);
        return NULL;
    }
    Nst_node_set_pos(assignment, start, name->end);

    if (is_compound) {
        Nst_LList *temp = *values;
        *values = assignment->comp_assign_e.values;
        assignment->comp_assign_e.values = temp;
        assignment->comp_assign_e.name = name;
        assignment->comp_assign_e.op = Nst_ASSIGNMENT_TO_STACK_OP(type);
    } else {
        assignment->assign_e.name = name;
        assignment->assign_e.value = Nst_llist_pop(*values);
    }

    exit_func(&initial_state);
    return assignment;
}

static Nst_Node *parse_extraction(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Node *container = parse_atom();
    if (container == NULL)
        return NULL;

    if (!state.endl_ends_expr)
        skip_blank();

    while (top_type() == Nst_TT_EXTRACT) {
        destroy_top();
        Nst_Node *idx;

        if (!state.endl_ends_expr)
            skip_blank();

        if (top_type() == Nst_TT_IDENT) {
            idx = new_node(Nst_NT_VALUE, top_start(), top_end());
            if (idx == NULL) {
                Nst_node_destroy(container);
                return NULL;
            }
            Nst_node_set_pos(idx, top_start(), top_end());
            idx->value.value = pop_top();
            idx->value.value->type = Nst_TT_VALUE;
        } else {
            idx = parse_atom();
            if (idx == NULL) {
                Nst_node_destroy(container);
                return NULL;
            }
        }

        Nst_Node *extraction = new_node(
            Nst_NT_EXTRACT_E,
            container->start,
            idx->end);
        if (extraction == NULL) {
            Nst_node_destroy(idx);
            Nst_node_destroy(container);
            return NULL;
        }
        Nst_node_set_pos(extraction, container->start, idx->end);
        container = extraction;

        if (!state.endl_ends_expr)
            skip_blank();
    }
    exit_func(&initial_state);
    return container;
}

static Nst_Node *parse_atom(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Node *atom = NULL;

    if (top_type() == Nst_TT_VALUE) {
        atom = new_node(Nst_NT_VALUE, top_start(), top_end());
        if (atom == NULL)
            return NULL;
        Nst_node_set_pos(atom, top_start(), top_end());
        atom->value.value = pop_top();
    } else if (top_type() == Nst_TT_IDENT) {
        atom = new_node(Nst_NT_ACCESS, top_start(), top_end());
        if (atom == NULL)
            return NULL;
        Nst_node_set_pos(atom, top_start(), top_end());
        atom->access.value = pop_top();
    } else if (top_type() == Nst_TT_L_PAREN) {
        Nst_Pos start = top_start();
        Nst_Pos end = top_end();
        destroy_top();
        state.endl_ends_expr = false;
        Nst_Node *expr = parse_expr();
        if (expr == NULL)
            return NULL;
        if (top_type() != Nst_TT_R_PAREN) {
            set_error(_Nst_EM_MISSING_PAREN, start, end);
            Nst_node_destroy(expr);
            return NULL;
        }
        end = top_end();
        destroy_top();

        atom = new_node(Nst_NT_E_WRAPPER, expr->start, expr->end);
        if (atom == NULL) {
            Nst_node_destroy(expr);
            return NULL;
        }
        atom->e_wrapper.expr = expr;
        Nst_node_set_pos(atom, start, end);
    } else if (Nst_IS_LOCAL_OP(top_type())) {
        atom = new_node(Nst_NT_LOCAL_OP, top_start(), top_end());
        if (atom == NULL)
            return NULL;
        Nst_Pos start = top_start();
        atom->local_op.op = top_type();

        Nst_Node *value = parse_extraction();
        if (value == NULL) {
            Nst_node_destroy(atom);
            return NULL;
        }
        atom->local_op.value = value;
        Nst_node_set_pos(atom, start, value->end);
    } else if (top_type() == Nst_TT_L_VBRACE) {
        atom = parse_vector_literal();
        if (atom == NULL)
            return NULL;
    } else if (top_type() == Nst_TT_L_BRACE) {
        atom = parse_arr_or_map_literal();
        if (atom == NULL)
            return NULL;
    } else if (top_type() == Nst_TT_LAMBDA) {
        atom = parse_func_declr();
        if (atom == NULL)
            return NULL;
    } else {
        set_error(_Nst_EM_EXPECTED_VALUE, top_start(), top_end());
        return NULL;
    }

    exit_func(&initial_state);
    return atom;
}

static Nst_Node *parse_vector_literal(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Node *vec_lit;

    destroy_top();
    skip_blank();
    if (top_type() == Nst_TT_R_VBRACE) {
        destroy_top();
        vec_lit = new_node(Nst_NT_SEQ_LIT, start, top_end());
        if (vec_lit == NULL)
            return NULL;
        Nst_node_set_pos(vec_lit, start, top_end());
        destroy_top();
        exit_func(&initial_state);
        return vec_lit;
    }

    vec_lit = parse_seq_body(start, NULL, false);
    if (vec_lit == NULL)
        return NULL;

    exit_func(&initial_state);
    return vec_lit;
}

static Nst_Node *parse_arr_or_map_literal(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;
    Nst_Pos start = top_start();
    destroy_top();
    skip_blank();

    if (top_type() == Nst_TT_R_BRACE) {
        Nst_Node *map_lit = new_node(Nst_NT_MAP_LIT, start, top_end());
        if (map_lit == NULL)
            return NULL;
        Nst_node_set_pos(map_lit, start, top_end());
        destroy_top();
        exit_func(&initial_state);
        return map_lit;
    } else if (top_type() == Nst_TT_COMMA) {
        destroy_top();
        skip_blank();
        if (top_type() != Nst_TT_R_BRACE) {
            set_error(_Nst_EM_EXPECTED_BRACE, top_start(), top_end());
            return NULL;
        }
        Nst_Node *arr_lit = new_node(Nst_NT_SEQ_LIT, start, top_end());
        if (arr_lit == NULL)
            return NULL;
        Nst_node_set_pos(arr_lit, start, top_end());
        arr_lit->seq_lit.type = Nst_SNT_ARRAY;
        destroy_top();
        exit_func(&initial_state);
        return arr_lit;
    }

    state.endl_ends_expr = false;
    state.break_ends_expr = true;

    Nst_Node *first_node = parse_expr();
    if (first_node == NULL)
        return NULL;
    skip_blank();

    Nst_Node *node;

    if (top_type() == Nst_TT_COLON)
        node = parse_map_body(start, first_node);
    else
        node = parse_seq_body(start, first_node, true);

    if (node == NULL)
        return NULL;
    exit_func(&initial_state);
    return node;
}

static Nst_Node *parse_seq_body(Nst_Pos start, Nst_Node *first_node, bool arr)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_TokType closing_paren = arr ? Nst_TT_R_BRACE : Nst_TT_R_VBRACE;
    const i8 *expected_paren = arr
        ? _Nst_EM_EXPECTED_BRACE
        : _Nst_EM_EXPECTED_VBRACE;

    const i8 *expected_comma_or_paren = arr
        ? _Nst_EM_EXPECTED_COMMA_OR_BRACE
        : _Nst_EM_EXPECTED_COMMA_OR_VBRACE;

    Nst_Node *seq_lit = new_node(Nst_NT_SEQ_LIT, start, start);
    if (seq_lit == NULL)
        goto failure;

    state.endl_ends_expr = false;
    state.break_ends_expr = true;

    if (first_node == NULL) {
        first_node = parse_expr();
        if (first_node == NULL)
            goto failure;
    }

    if (!append_node(seq_lit->seq_lit.values, first_node)) {
        Nst_node_destroy(first_node);
        Nst_node_destroy(seq_lit);
        return NULL;
    }
    first_node = NULL;
    skip_blank();
    if (top_type() == Nst_TT_BREAK) {
        destroy_top();
        Nst_Node *length = parse_expr();
        if (length == NULL)
            goto failure;
        skip_blank();
        if (top_type() != closing_paren) {
            set_error(expected_paren, top_start(), top_end());
            goto failure;
        }
        if (!append_node(seq_lit->seq_lit.values, length)) {
            Nst_node_destroy(length);
            goto failure;
        }
        Nst_node_set_pos(seq_lit, start, top_end());
        destroy_top();
        seq_lit->seq_lit.type = arr ? Nst_SNT_ARRAY_REP : Nst_SNT_VECTOR_REP;
        exit_func(&initial_state);
        return seq_lit;
    }
    seq_lit->seq_lit.type = arr ? Nst_SNT_ARRAY : Nst_SNT_VECTOR;

    goto from_comma;

    while (true) {
        skip_blank();
        Nst_Node *value = parse_expr();
        if (value == NULL)
            goto failure;
        if (!append_node(seq_lit->seq_lit.values, value)) {
            Nst_node_destroy(value);
            goto failure;
        }
    from_comma:
        skip_blank();

        if (top_type() == Nst_TT_COMMA)
            destroy_top();
        else if (top_type() == closing_paren) {
            Nst_node_set_pos(seq_lit, start, top_end());
            destroy_top();
            exit_func(&initial_state);
            return seq_lit;
        } else {
            set_error(expected_comma_or_paren, top_start(), top_end());
            goto failure;
        }
    }

failure:
    if (seq_lit != NULL)
        Nst_node_destroy(seq_lit);
    if (first_node != NULL)
        Nst_node_destroy(first_node);
    return NULL;
}

static Nst_Node *parse_map_body(Nst_Pos start, Nst_Node *key)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Node *value = NULL;
    Nst_Node *map_lit = new_node(Nst_NT_MAP_LIT, start, start);
    if (map_lit == NULL)
        goto failure;

    if (!append_node(map_lit->map_lit.keys, key))
        goto failure;
    key = NULL;
    goto from_colon;

    while (true) {
        key = parse_expr();
        if (key == NULL)
            goto failure;
        if (!append_node(map_lit->map_lit.keys, key))
            goto failure;
        key = NULL;

    from_colon:
        skip_blank();
        if (top_type() != Nst_TT_COLON) {
            set_error(_Nst_EM_EXPECTED_COLON, top_start(), top_end());
            goto failure;
        }
        value = parse_expr();
        if (value == NULL)
            goto failure;
        if (!append_node(map_lit->map_lit.values, value))
            goto failure;
        value = NULL;

        if (top_type() == Nst_TT_R_BRACE) {
            Nst_node_set_pos(map_lit, start, top_end());
            destroy_top();
            exit_func(&initial_state);
            return map_lit;
        } else if (top_type() == Nst_TT_COMMA)
            destroy_top();
        else {
            set_error(_Nst_EM_EXPECTED_COMMA_OR_BRACE, top_start(), top_end());
            goto failure;
        }
    }

failure:
    if (key != NULL)
        Nst_node_destroy(key);
    if (value != NULL)
        Nst_node_destroy(value);
    if (map_lit != NULL)
        Nst_node_destroy(map_lit);
    return NULL;
}

static Nst_Node *parse_try_catch(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    destroy_top();

    Nst_Node *try_catch_s = new_node(Nst_NT_TRY_CATCH_S, start, start);
    if (try_catch_s == NULL)
        goto failure;

    state.endl_ends_expr = true;
    state.break_ends_expr = false;

    Nst_Node *try_body = parse_statement();
    if (try_body == NULL)
        goto failure;
    try_catch_s->try_catch_s.try_body = try_body;
    skip_blank();

    if (top_type() != Nst_TT_CATCH)
        goto failure;
    destroy_top();
    if (top_type() != Nst_TT_IDENT)
        goto failure;

    try_catch_s->try_catch_s.error_name = pop_top();
    skip_blank();
    Nst_Node *catch_body = parse_statement();
    if (catch_body == NULL)
        goto failure;
    try_catch_s->try_catch_s.catch_body = catch_body;

    Nst_node_set_pos(try_catch_s, start, catch_body->end);
    exit_func(&initial_state);
    return try_catch_s;

failure:
    if (try_catch_s != NULL)
        Nst_node_destroy(try_catch_s);
    return NULL;
}
