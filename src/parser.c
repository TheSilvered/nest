#include "nest.h"

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

static Nst_Node *parse_cs(void);
static Nst_Node *parse_cs_with_brackets(void);
static Nst_Node *parse_statement(void);
static Nst_Node *parse_wl(void);
static Nst_Node *parse_fl(void);
static Nst_Node *parse_expr(void);
static Nst_Node *parse_sw(void);
static Nst_Node *parse_fd(void);
static Nst_Node *parse_stack_expr(void);
static Nst_Node *parse_so(Nst_LList **values, Nst_Pos start);
static Nst_Node *parse_ls(Nst_LList **values, Nst_Pos start);
static Nst_Node *parse_as_name(bool is_compound);
static Nst_Node *parse_as(Nst_LList **values, Nst_Pos start);
static Nst_Node *parse_ex(void);
static Nst_Node *parse_atom(void);
static Nst_Node *parse_vector_literal(void);
static Nst_Node *parse_arr_or_map_literal(void);
static Nst_Node *parse_seq_body(Nst_Pos start, Nst_Node *first_node, bool arr);
static Nst_Node *parse_map_body(Nst_Pos start, Nst_Node *key);
static Nst_Node *parse_tc(void);

Nst_Node *Nst_parse(Nst_LList *tokens)
{
    if (tokens == NULL)
        return NULL;

    state.in_func = false;
    state.in_loop = false;
    state.in_switch = false;
    state.recursion_lvl = 0;
    state.tokens = tokens;

    Nst_Node *ast = parse_cs();

    if (ast != NULL && state.tokens->len > 1) {
        Nst_Pos start = Nst_TOK(Nst_llist_peek_front(state.tokens))->start;
        Nst_Pos end = Nst_TOK(Nst_llist_peek_front(state.tokens))->end;

        Nst_error_setc_syntax("unexpected token");
        Nst_error_add_pos(start, end);
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
        Nst_error_setc_memory("too many nested structures, parsing failed");
        Nst_error_add_pos(start, end);
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
        Nst_error_add_pos(node->start, node->end);
    return result;
}

static inline bool append_tok(Nst_LList *llist, Nst_Tok *tok)
{
    bool result = Nst_llist_append(llist, tok, true);
    if (!result)
        Nst_error_add_pos(tok->start, tok->end);
    return result;
}

// Returns the head token on the tokens list
static inline Nst_Tok *peek_top(void)
{
    return Nst_TOK(state.tokens->head->value);
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
        return Nst_pos_empty();
    return head->start;
}

// Returns the ending position of the head token on the tokens list
static inline Nst_Pos top_end(void)
{
    Nst_Tok *head = peek_top();
    if (head == NULL)
        return Nst_pos_empty();
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
    Nst_error_setc_syntax(msg);
    Nst_error_add_pos(start, end);
}

// Creates a new node adding the given positions if an error occurs.
// The positions themselves are not added to the node and must be set manually.
static Nst_Node *new_node(Nst_NodeType type, Nst_Pos start, Nst_Pos end)
{
    Nst_Node *node = Nst_node_new(type);
    if (node == NULL)
        Nst_error_add_pos(start, end);
    return node;
}

static inline void skip_blank(void)
{
    while (top_type() == Nst_TT_ENDL)
        destroy_top();
}

static Nst_Node *parse_cs(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Node *long_s = Nst_node_new(Nst_NT_CS);
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
        if (!append_node(long_s->v.cs.statements, statement))
            goto failure;
        skip_blank();
    }

    if (long_s->v.cs.statements->len != 0) {
        start = Nst_NODE(long_s->v.cs.statements->head->value)->start;
        end = Nst_NODE(long_s->v.cs.statements->tail->value)->start;
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

static Nst_Node *parse_cs_with_brackets(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    skip_blank();

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    if (top_type() != Nst_TT_L_BRACKET) {
        set_error("expected '['", start, end);
        return NULL;
    }
    destroy_top();
    Nst_Node *long_s = parse_cs();
    if (long_s == NULL)
        return NULL;

    skip_blank();

    if (top_type() != Nst_TT_R_BRACKET) {
        set_error("unmatched '['", start, end);
        Nst_node_destroy(long_s);
        return NULL;
    }
    end = top_end();
    destroy_top();

    Nst_Node *wrapper = new_node(Nst_NT_WS, start, end);
    if (wrapper == NULL) {
        Nst_node_destroy(long_s);
        return NULL;
    }
    wrapper->v.ws.statement = long_s;
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

        Nst_Node *long_s = parse_cs();
        if (long_s == NULL)
            return NULL;

        if (top_type() != Nst_TT_R_BRACKET) {
            Nst_node_destroy(long_s);
            set_error("unmatched '['", start, start);
            return NULL;
        }
        destroy_top();
        exit_func(&initial_state);
        return long_s;
    } else if (tok_type == Nst_TT_WHILE || tok_type == Nst_TT_DOWHILE) {
        Nst_Node *while_l = parse_wl();
        exit_func(&initial_state);
        return while_l;
    } else if (tok_type == Nst_TT_FOR) {
        Nst_Node *for_l = parse_fl();
        exit_func(&initial_state);
        return for_l;
    } else if (tok_type == Nst_TT_FUNC) {
        Nst_Node *func_declr = parse_fd();
        exit_func(&initial_state);
        return func_declr;
    } else if (tok_type == Nst_TT_SWITCH) {
        Nst_Node *switch_s = parse_sw();
        exit_func(&initial_state);
        return switch_s;
    } else if (tok_type == Nst_TT_RETURN) {
        Nst_Pos start = top_start();
        Nst_Pos end = top_end();
        destroy_top();

        if (!state.in_func) {
            set_error("'=>' outside of a function", start, end);
            return NULL;
        }

        Nst_Node *expr;
        Nst_Pos expr_end;

        tok_type = top_type();
        if (_Nst_TOK_IS_EXPR_END(tok_type)) {
            expr = NULL;
            expr_end = end;
        } else {
            expr = parse_expr();
            if (expr == NULL)
                return NULL;
            expr_end = expr->end;
        }

        Nst_Node *return_s = new_node(Nst_NT_RT, start, expr_end);
        if (return_s == NULL) {
            if (expr != NULL)
                Nst_node_destroy(expr);
            return NULL;
        }
        Nst_node_set_pos(return_s, start, expr_end);
        return_s->v.rt.value = expr;
        exit_func(&initial_state);
        return return_s;
    } else if (tok_type == Nst_TT_CONTINUE) {
        Nst_Pos start = top_start();
        Nst_Pos end = top_end();
        destroy_top();

        if (!state.in_loop && !state.in_switch) {
            set_error("'..' outside of a loop", start, end);
            return NULL;
        }

        Nst_Node *continue_s = new_node(Nst_NT_CN, start, end);
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
            set_error("';' outside of a loop", start, end);
            return NULL;
        }

        Nst_Node *break_s = new_node(Nst_NT_BR, start, end);
        if (break_s == NULL)
            return NULL;
        Nst_node_set_pos(break_s, start, end);
        exit_func(&initial_state);
        return break_s;
    } else if (tok_type == Nst_TT_TRY) {
        Nst_Node *try_catch_s = parse_tc();
        exit_func(&initial_state);
        return try_catch_s;
    } else {
        Nst_Node *expr = parse_expr();
        exit_func(&initial_state);
        return expr;
    }
}

static Nst_Node *parse_wl(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    Nst_Node *while_l = new_node(Nst_NT_WL, start, end);
    if (while_l == NULL)
        return NULL;
    while_l->v.wl.is_dowhile = top_type() == Nst_TT_DOWHILE;
    destroy_top();
    skip_blank();
    state.endl_ends_expr = false;
    Nst_Node *condition = parse_expr();
    state.endl_ends_expr = initial_state.endl_ends_expr;
    if (condition == NULL) {
        Nst_node_destroy(while_l);
        return NULL;
    }
    while_l->v.wl.condition = condition;
    state.in_loop = true;
    Nst_Node *body = parse_cs_with_brackets();
    if (body == NULL) {
        Nst_node_destroy(while_l);
        return NULL;
    }
    while_l->v.wl.body = body;
    Nst_node_set_pos(while_l, start, body->end);
    exit_func(&initial_state);
    return while_l;
}

static Nst_Node *parse_fl(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    Nst_Node *for_l = new_node(Nst_NT_FL, start, end);
    if (for_l == NULL)
        return NULL;
    destroy_top();
    skip_blank();
    state.endl_ends_expr = false;
    Nst_Node *iterator = parse_expr();
    state.endl_ends_expr = initial_state.endl_ends_expr;
    if (iterator == NULL) {
        Nst_node_destroy(for_l);
        return NULL;
    }
    for_l->v.fl.iterator = iterator;
    skip_blank();
    if (top_type() == Nst_TT_AS) {
        destroy_top();
        Nst_Node *assignment = parse_as_name(false);
        if (assignment == NULL) {
            Nst_node_destroy(for_l);
            return NULL;
        }
        for_l->v.fl.assignment = assignment;
        skip_blank();
    }
    state.in_loop = true;
    Nst_Node *body = parse_cs_with_brackets();
    if (body == NULL) {
        Nst_node_destroy(for_l);
        return NULL;
    }
    for_l->v.fl.body = body;
    Nst_node_set_pos(for_l, start, body->end);
    exit_func(&initial_state);
    return for_l;
}

static Nst_Node *parse_expr(void)
{
    ParsingState initial_state;
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
    Nst_Node *if_e = new_node(Nst_NT_IE, condition->start, condition->end);
    if (if_e == NULL) {
        Nst_node_destroy(condition);
        return NULL;
    }
    if_e->v.ie.condition = condition;
    skip_blank();
    Nst_Node *body_if_true = parse_statement();
    if (body_if_true == NULL) {
        Nst_node_destroy(if_e);
        return NULL;
    }
    if_e->v.ie.body_if_true = body_if_true;
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
    if_e->v.ie.body_if_false = body_if_false;
    Nst_node_set_pos(if_e, condition->start, body_if_false->end);
    exit_func(&initial_state);
    return if_e;
}

static Nst_Node *parse_sw(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    destroy_top();

    Nst_Node *switch_s = new_node(Nst_NT_SW, start, end);
    if (switch_s == NULL)
        return NULL;

    state.endl_ends_expr = false;
    Nst_Node *expr = parse_expr();
    state.endl_ends_expr = initial_state.endl_ends_expr;
    if (expr == NULL) {
        Nst_node_destroy(switch_s);
        return NULL;
    }
    switch_s->v.sw.expr = expr;
    skip_blank();
    if (top_type() != Nst_TT_L_BRACKET) {
        Nst_node_destroy(switch_s);
        set_error("expected '['", top_start(), top_end());
        return NULL;
    }
    destroy_top();
    while (true) {
        skip_blank();
        if (top_type() == Nst_TT_R_BRACKET)
            break;
        if (top_type() != Nst_TT_IF) {
            Nst_node_destroy(switch_s);
            set_error("expected '?'", top_start(), top_end());
            return NULL;
        }
        destroy_top();
        skip_blank();

        if (top_type() == Nst_TT_L_BRACKET) {
            state.in_switch = true;
            Nst_Node *default_body = parse_cs_with_brackets();
            if (default_body == NULL) {
                Nst_node_destroy(switch_s);
                return NULL;
            }
            switch_s->v.sw.default_body = default_body;
            break;
        }
        state.endl_ends_expr = false;
        Nst_Node *value = parse_expr();
        state.endl_ends_expr = initial_state.endl_ends_expr;
        if (value == NULL) {
            Nst_node_destroy(switch_s);
            return NULL;
        }
        if (!append_node(switch_s->v.sw.values, value)) {
            Nst_node_destroy(value);
            Nst_node_destroy(switch_s);
            return NULL;
        }
        state.in_switch = true;
        Nst_Node *body = parse_cs_with_brackets();
        state.in_switch = initial_state.in_switch;
        if (body == NULL) {
            Nst_node_destroy(switch_s);
            return NULL;
        }
        if (!append_node(switch_s->v.sw.bodies, body)) {
            Nst_node_destroy(body);
            Nst_node_destroy(switch_s);
            return NULL;
        }
    }
    skip_blank();
    if (top_type() != Nst_TT_R_BRACKET) {
        Nst_node_destroy(switch_s);
        set_error("expected ']'", top_start(), top_end());
        return NULL;
    }
    Nst_node_set_pos(switch_s, start, top_end());
    destroy_top();
    exit_func(&initial_state);
    return switch_s;
}

static Nst_Node *parse_fd(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = top_end();
    Nst_Node *func_declr = new_node(Nst_NT_FD, start, end);
    if (func_declr == NULL)
        return NULL;

    bool is_lambda = top_type() == Nst_TT_LAMBDA;
    destroy_top();
    skip_blank();
    if (!is_lambda && top_type() != Nst_TT_IDENT) {
        Nst_node_destroy(func_declr);
        set_error("expected an identifier", top_start(), top_end());
        return NULL;
    }
    if (!is_lambda)
        func_declr->v.fd.name = pop_top();

    skip_blank();
    while (top_type() == Nst_TT_IDENT) {
        Nst_Tok *tok = peek_top();
        if (!append_tok(func_declr->v.fd.argument_names, tok)) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        pop_top();
        skip_blank();
    }

    if (top_type() == Nst_TT_RETURN) {
        Nst_Pos body_start = top_start();
        Nst_Node *return_s = new_node(Nst_NT_RT, body_start, top_end());
        if (return_s == NULL) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        destroy_top();
        func_declr->v.fd.body = return_s;
        Nst_Node *expr = parse_expr();
        if (expr == NULL) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        return_s->v.rt.value = expr;
        Nst_node_set_pos(return_s, body_start, expr->end);
        end = expr->end;
    } else {
        state.in_func = true;
        Nst_Node *body = parse_cs_with_brackets();
        if (body == NULL) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        func_declr->v.fd.body = body;
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
        Nst_error_add_pos(top_start(), top_end());
        return NULL;
    }

    Nst_Pos start = top_start();

    while (true) {
        if (!state.endl_ends_expr)
            skip_blank();

        while (_Nst_TOK_IS_ATOM(top_type())) {
            Nst_Node *atom = parse_ex();
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
        if (values->len == 0 && !_Nst_TOK_IS_LOCAL_STACK_OP(top_type())) {
            set_error("expected a value", top_start(), top_end());
            goto failure;
        }

        Nst_Node *operation_node = NULL;
        if (_Nst_TOK_IS_STACK_OP(top_type()))
            operation_node = parse_so(&values, start);
        else if (_Nst_TOK_IS_LOCAL_STACK_OP(top_type()))
            operation_node = parse_ls(&values, start);
        else if (_Nst_TOK_IS_ASSIGNMENT(top_type()))
            operation_node = parse_as(&values, start);
        else if (_Nst_TOK_IS_EXPR_END(top_type()) && state.endl_ends_expr)
            break;
        else if (_Nst_TOK_IS_EXPR_END(top_type()) && top_type() != Nst_TT_ENDL)
            break;
        else if (state.break_ends_expr && top_type() == Nst_TT_BREAK)
            break;
        else {
            set_error("unexpected token", top_start(), top_end());
            goto failure;
        }

        if (operation_node == NULL)
            goto failure;

        if (!append_node(values, operation_node)) {
            Nst_node_destroy(operation_node);
            goto failure;
        }
    }

    // The expression end token must not be consumed

    if (values->len != 1) {
        set_error(
            "expected stack or local stack operator",
            top_start(),
            top_end());
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

static Nst_Node *parse_so(Nst_LList **values, Nst_Pos start)
{
    Nst_Node *stack_op = new_node(Nst_NT_SO, top_start(), top_end());
    if (stack_op == NULL)
        return NULL;
    Nst_node_set_pos(stack_op, start, top_end());
    Nst_LList *temp = *values;
    *values = stack_op->v.so.values;
    stack_op->v.so.values = temp;
    stack_op->v.so.op = top_type();
    destroy_top();
    return stack_op;
}

static Nst_Node *parse_ls(Nst_LList **values, Nst_Pos start)
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
    Nst_Node *special_node = parse_ex();
    if (special_node == NULL)
        return NULL;
    Nst_Node *local_stack_op = new_node(
        Nst_NT_LS,
        start,
        special_node->end);
    if (local_stack_op == NULL) {
        Nst_node_destroy(special_node);
        return NULL;
    }
    local_stack_op->v.ls.op = op;
    Nst_node_set_pos(local_stack_op, start, special_node->end);
    Nst_LList *temp = *values;
    *values = local_stack_op->v.ls.values;
    local_stack_op->v.ls.values = temp;
    local_stack_op->v.ls.special_value = special_node;
    local_stack_op->v.ls.op = op;
    return local_stack_op;
}

static bool check_local_stack_op_arg_num(usize arg_num, Nst_Pos start,
                                         Nst_Pos end)
{
    Nst_TokType type = top_type();
    if (type == Nst_TT_CAST && arg_num != 1) {
        set_error("'::' expects only 1 argument on the left", start, end);
        return false;
    } else if (type == Nst_TT_RANGE && arg_num != 1 && arg_num != 2) {
        set_error("'->' expects only 1 or 2 arguments on the left", start, end);
        return false;
    } else if (type == Nst_TT_THROW && arg_num != 1) {
        set_error("'!!' expects only 1 argument on the left", start, end);
        return false;
    } else if (type == Nst_TT_SEQ_CALL && arg_num != 1) {
        set_error("'*@' expects only 1 argument on the left", start, end);
        return false;
    }
    return true;
}

static Nst_Node *parse_as_name(bool is_compound)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    Nst_Pos end = start;
    Nst_Node *name;

    if (top_type() != Nst_TT_L_BRACE) {
        name = parse_ex();
        if (name == NULL)
            return NULL;
        if (name->type != Nst_NT_AC && name->type != Nst_NT_EX) {
            set_error(
                "expected an identifier or an extraction",
                name->start, name->end);
            Nst_node_destroy(name);
            return NULL;
        }
        exit_func(&initial_state);
        return name;
    }

    if (is_compound) {
        set_error(
            "cannot unpack values in a compound assignment",
            top_start(), top_end());
        return NULL;
    }

    name = new_node(Nst_NT_SL, top_start(), top_end());
    if (name == NULL)
        return NULL;
    name->v.sl.type = Nst_SNT_ASSIGNMENT_NAMES;
    destroy_top();

    state.endl_ends_expr = false;

    while (true) {
        skip_blank();
        Nst_Node *sub_name = parse_as_name(false);
        if (sub_name == NULL)
            goto failure;
        if (!append_node(name->v.sl.values, sub_name)) {
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
            set_error("expected ',' or '}'", top_start(), top_end());
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

static Nst_Node *parse_as(Nst_LList **values, Nst_Pos start)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_TokType type = top_type();
    bool is_compound = type != Nst_TT_ASSIGN;

    if ((*values)->len > 1 && !is_compound) {
        set_error(
            "expected stack or local stack operator",
            top_start(),
            top_end());
        return NULL;
    }
    destroy_top();

    Nst_Node *name = parse_as_name(is_compound);
    if (name == NULL)
        return NULL;

    Nst_Node *assignment = new_node(
        is_compound ? Nst_NT_CA : Nst_NT_AS,
        start,
        name->end);

    if (assignment == NULL) {
        Nst_node_destroy(name);
        return NULL;
    }
    Nst_node_set_pos(assignment, start, name->end);

    if (is_compound) {
        Nst_LList *temp = *values;
        *values = assignment->v.ca.values;
        assignment->v.ca.values = temp;
        assignment->v.ca.name = name;
        assignment->v.ca.op = _Nst_TOK_ASSIGNMENT_TO_STACK_OP(type);
    } else {
        assignment->v.as.name = name;
        assignment->v.as.value = Nst_llist_pop(*values);
    }

    exit_func(&initial_state);
    return assignment;
}

static Nst_Node *parse_ex(void)
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
        Nst_Node *key;

        if (!state.endl_ends_expr)
            skip_blank();

        if (top_type() == Nst_TT_IDENT) {
            key = new_node(Nst_NT_VL, top_start(), top_end());
            if (key == NULL) {
                Nst_node_destroy(container);
                return NULL;
            }
            Nst_node_set_pos(key, top_start(), top_end());
            key->v.vl.value = pop_top();
            key->v.vl.value->type = Nst_TT_VALUE;
        } else {
            key = parse_atom();
            if (key == NULL) {
                Nst_node_destroy(container);
                return NULL;
            }
        }

        Nst_Node *extraction = new_node(
            Nst_NT_EX,
            container->start,
            key->end);
        if (extraction == NULL) {
            Nst_node_destroy(key);
            Nst_node_destroy(container);
            return NULL;
        }
        Nst_node_set_pos(extraction, container->start, key->end);
        extraction->v.ex.key = key;
        extraction->v.ex.container = container;
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
        atom = new_node(Nst_NT_VL, top_start(), top_end());
        if (atom == NULL)
            return NULL;
        Nst_node_set_pos(atom, top_start(), top_end());
        atom->v.vl.value = pop_top();
    } else if (top_type() == Nst_TT_IDENT) {
        atom = new_node(Nst_NT_AC, top_start(), top_end());
        if (atom == NULL)
            return NULL;
        Nst_node_set_pos(atom, top_start(), top_end());
        atom->v.ac.value = pop_top();
    } else if (top_type() == Nst_TT_L_PAREN) {
        Nst_Pos start = top_start();
        Nst_Pos end = top_end();
        destroy_top();
        state.endl_ends_expr = false;
        Nst_Node *expr = parse_expr();
        if (expr == NULL)
            return NULL;
        if (top_type() != Nst_TT_R_PAREN) {
            set_error("unmatched '('", start, end);
            Nst_node_destroy(expr);
            return NULL;
        }
        end = top_end();
        destroy_top();

        atom = new_node(Nst_NT_WE, expr->start, expr->end);
        if (atom == NULL) {
            Nst_node_destroy(expr);
            return NULL;
        }
        atom->v.we.expr = expr;
        Nst_node_set_pos(atom, start, end);
    } else if (_Nst_TOK_IS_LOCAL_OP(top_type())) {
        atom = new_node(Nst_NT_LO, top_start(), top_end());
        if (atom == NULL)
            return NULL;
        Nst_Pos start = top_start();
        atom->v.lo.op = top_type();
        destroy_top();

        Nst_Node *value = parse_ex();
        if (value == NULL) {
            Nst_node_destroy(atom);
            return NULL;
        }
        atom->v.lo.value = value;
        Nst_node_set_pos(atom, start, value->end);
    } else if (top_type() == Nst_TT_CALL) {
        atom = new_node(Nst_NT_LO, top_start(), top_end());
        if (atom == NULL)
            return NULL;
        Nst_Pos start = top_start();
        atom->v.lo.op = Nst_TT_LOC_CALL;
        destroy_top();

        Nst_Node *value = parse_ex();
        if (value == NULL) {
            Nst_node_destroy(atom);
            return NULL;
        }
        atom->v.lo.value = value;
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
        atom = parse_fd();
        if (atom == NULL)
            return NULL;
    } else {
        set_error("expected a value", top_start(), top_end());
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
        vec_lit = new_node(Nst_NT_SL, start, top_end());
        vec_lit->v.sl.type = Nst_SNT_VECTOR;
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
        Nst_Node *map_lit = new_node(Nst_NT_ML, start, top_end());
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
            set_error("expected '}'", top_start(), top_end());
            return NULL;
        }
        Nst_Node *arr_lit = new_node(Nst_NT_SL, start, top_end());
        if (arr_lit == NULL)
            return NULL;
        Nst_node_set_pos(arr_lit, start, top_end());
        arr_lit->v.sl.type = Nst_SNT_ARRAY;
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
    const char *expected_paren = arr
        ? "expected '}'"
        : "expected '}>'";

    const char *expected_comma_or_paren = arr
        ? "expected ',' or '}'"
        : "expected ',' or '}>'";

    Nst_Node *seq_lit = new_node(Nst_NT_SL, start, start);
    if (seq_lit == NULL)
        goto failure;

    state.endl_ends_expr = false;
    state.break_ends_expr = true;

    if (first_node == NULL) {
        first_node = parse_expr();
        if (first_node == NULL)
            goto failure;
    }

    if (!append_node(seq_lit->v.sl.values, first_node)) {
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
            Nst_node_destroy(length);
            goto failure;
        }
        if (!append_node(seq_lit->v.sl.values, length)) {
            Nst_node_destroy(length);
            goto failure;
        }
        Nst_node_set_pos(seq_lit, start, top_end());
        destroy_top();
        seq_lit->v.sl.type = arr ? Nst_SNT_ARRAY_REP : Nst_SNT_VECTOR_REP;
        exit_func(&initial_state);
        return seq_lit;
    }
    seq_lit->v.sl.type = arr ? Nst_SNT_ARRAY : Nst_SNT_VECTOR;

    goto from_comma;

    while (true) {
        skip_blank();
        Nst_Node *value = parse_expr();
        if (value == NULL)
            goto failure;
        if (!append_node(seq_lit->v.sl.values, value)) {
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
    Nst_Node *map_lit = new_node(Nst_NT_ML, start, start);
    if (map_lit == NULL)
        goto failure;

    if (!append_node(map_lit->v.ml.keys, key))
        goto failure;
    key = NULL;
    goto from_colon;

    while (true) {
        key = parse_expr();
        if (key == NULL)
            goto failure;
        if (!append_node(map_lit->v.ml.keys, key))
            goto failure;
        key = NULL;

    from_colon:
        skip_blank();
        if (top_type() != Nst_TT_COLON) {
            set_error("expected ':'", top_start(), top_end());
            goto failure;
        }
        destroy_top();
        value = parse_expr();
        if (value == NULL)
            goto failure;
        if (!append_node(map_lit->v.ml.values, value))
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
            set_error("expected ',' or '}'", top_start(), top_end());
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

static Nst_Node *parse_tc(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    destroy_top();

    Nst_Node *try_catch_s = new_node(Nst_NT_TC, start, start);
    if (try_catch_s == NULL)
        goto failure;

    state.endl_ends_expr = true;
    state.break_ends_expr = false;

    Nst_Node *try_body = parse_statement();
    if (try_body == NULL)
        goto failure;
    try_catch_s->v.tc.try_body = try_body;
    skip_blank();

    if (top_type() != Nst_TT_CATCH) {
        set_error("expected '?!'", top_start(), top_end());
        goto failure;
    }
    destroy_top();
    if (top_type() != Nst_TT_IDENT) {
        set_error("expected an identifier", top_start(), top_end());
        goto failure;
    }

    try_catch_s->v.tc.error_name = pop_top();
    skip_blank();
    Nst_Node *catch_body = parse_statement();
    if (catch_body == NULL)
        goto failure;
    try_catch_s->v.tc.catch_body = catch_body;

    Nst_node_set_pos(try_catch_s, start, catch_body->end);
    exit_func(&initial_state);
    return try_catch_s;

failure:
    if (try_catch_s != NULL)
        Nst_node_destroy(try_catch_s);
    return NULL;
}
