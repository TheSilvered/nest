#include "nest.h"

#define GET_NODE(pa, i) ((Nst_Node *)Nst_pa_get((pa), (i)))

typedef struct _ParsingState {
    bool in_func;
    bool in_loop;
    bool in_switch;
    bool endl_ends_expr;
    bool break_ends_expr;
    int recursion_lvl;
    usize idx;
    Nst_DynArray *tokens;
} ParsingState;

static ParsingState state;

static inline bool enter_func(ParsingState *initial_state);
static inline void exit_func(ParsingState *initial_state);
static inline bool append_node(Nst_PtrArray *arr, Nst_Node *node);
static inline bool append_tok_val(Nst_PtrArray *arr, Nst_Tok *tok);

static inline Nst_Tok *peek_top(void);
static inline Nst_TokType top_type(void);
static inline Nst_Span top_span(void);
static inline Nst_Pos top_start(void);
static inline Nst_Pos top_end(void);
static inline Nst_Tok *pop_top(void);

static Nst_Node *new_node(Nst_NodeType type, Nst_Span span);

static inline void set_error(const char *msg, Nst_Span span);
static inline void skip_blank(void);
static bool check_local_stack_op_arg_num(usize arg_num, Nst_Span span);

static inline void swap_pa(Nst_PtrArray *arr1, Nst_PtrArray *arr2)
{
    Nst_PtrArray temp = *arr1;
    *arr1 = *arr2;
    *arr2 = temp;
}

static Nst_Node *parse_s_list(void);
static Nst_Node *parse_s_list_with_brackets(void);
static Nst_Node *parse_statement(void);
static Nst_Node *parse_s_while_lp(void);
static Nst_Node *parse_s_for_lp(void);
static Nst_Node *parse_expr(void);
static Nst_Node *parse_s_switch(void);
static Nst_Node *parse_s_fn_decl(void);
static Nst_Node *parse_stack_expr(void);
static Nst_Node *parse_e_stack_op(Nst_PtrArray *values, Nst_Pos start);
static Nst_Node *parse_e_loc_stack_op(Nst_PtrArray *values, Nst_Pos start);
static Nst_Node *parse_e_assignment_name(bool is_compound);
static Nst_Node *parse_e_assignment(Nst_PtrArray *values, Nst_Pos start);
static Nst_Node *parse_e_extraction(void);
static Nst_Node *parse_atom(void);
static Nst_Node *parse_vector_literal(void);
static Nst_Node *parse_arr_or_map_literal(void);
static Nst_Node *parse_seq_body(Nst_Pos start, Nst_Node *first_node, bool arr);
static Nst_Node *parse_map_body(Nst_Pos start, Nst_Node *key);
static Nst_Node *parse_s_try_catch(void);

Nst_Node *Nst_parse(Nst_DynArray *tokens)
{
    state.in_func = false;
    state.in_loop = false;
    state.in_switch = false;
    state.recursion_lvl = 0;
    state.idx = 0;
    state.tokens = tokens;

    Nst_Node *ast = parse_s_list();

    if (ast != NULL && state.idx + 1 < state.tokens->len) {
        Nst_error_setc_syntax("unexpected token");
        Nst_error_add_span(top_span());
        Nst_node_destroy(ast);
        ast = NULL;
    }
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
        Nst_error_setc_memory("too many nested structures, parsing failed");
        Nst_error_add_span(top_span());
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

static inline bool append_node(Nst_PtrArray *arr, Nst_Node *node)
{
    bool result = Nst_pa_append(arr, node);
    if (!result)
        Nst_error_add_span(node->span);
    return result;
}

static inline bool append_tok_val(Nst_PtrArray *arr, Nst_Tok *tok)
{
    Nst_Obj *value = Nst_inc_ref(tok->value);
    bool result = Nst_pa_append(arr, value);
    if (!result) {
        Nst_dec_ref(value);
        Nst_error_add_span(tok->span);
    }
    return result;
}

// Returns the head token in the tokens list
static inline Nst_Tok *peek_top(void)
{
    if (state.idx >= state.tokens->len)
        return NULL;
    return (Nst_Tok *)Nst_da_get(state.tokens, state.idx);
}

// Returns the type of the head token in the tokens list
static inline Nst_TokType top_type(void)
{
    Nst_Tok *head = peek_top();
    if (head == NULL)
        return Nst_TT_INVALID;
    return head->type;
}

// Returns the span of the head token in the tokens list
static inline Nst_Span top_span(void)
{
    Nst_Tok *head = peek_top();
    if (head == NULL)
        return Nst_span_empty();
    return head->span;
}

// Returns the starting position of the head token in the tokens list
static inline Nst_Pos top_start(void)
{
    Nst_Tok *head = peek_top();
    if (head == NULL)
        return Nst_pos_empty();
    return Nst_span_start(head->span);
}

// Returns the ending position of the head token in the tokens list
static inline Nst_Pos top_end(void)
{
    Nst_Tok *head = peek_top();
    if (head == NULL)
        return Nst_pos_empty();
    return Nst_span_end(head->span);
}

// Pops the head token of the tokens list and returns it
static inline Nst_Tok *pop_top(void)
{
    Nst_Tok *top = peek_top();
    state.idx++;
    return top;
}

// Sets a syntax error with the given message and adds the positions
static inline void set_error(const char *msg, Nst_Span span)
{
    Nst_error_setc_syntax(msg);
    Nst_error_add_span(span);
}

// Creates a new node adding the given positions if an error occurs.
// The positions themselves are not added to the node and must be set manually.
static Nst_Node *new_node(Nst_NodeType type, Nst_Span span)
{
    Nst_Node *node = Nst_node_new(type);
    if (node == NULL)
        Nst_error_add_span(span);
    return node;
}

static inline void skip_blank(void)
{
    while (top_type() == Nst_TT_ENDL)
        pop_top();
}

static Nst_Node *parse_s_list(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Node *long_s = Nst_node_new(Nst_NT_S_LIST);
    Nst_Node *statement = NULL;

    if (long_s == NULL)
        goto failure;
    skip_blank();

    Nst_Span span = top_span();

    state.endl_ends_expr = true;
    state.break_ends_expr = false;

    while (top_type() != Nst_TT_R_BRACKET && top_type() != Nst_TT_EOFILE)
    {
        statement = parse_statement();
        if (statement == NULL)
            goto failure;
        if (!append_node(&long_s->v.s_list.statements, statement))
            goto failure;
        skip_blank();
    }

    if (long_s->v.s_list.statements.len != 0) {
        span = GET_NODE(&long_s->v.s_list.statements, 0)->span;
    }
    Nst_node_set_span(long_s, span);

    exit_func(&initial_state);
    return long_s;

failure:
    if (long_s != NULL)
        Nst_node_destroy(long_s);
    if (statement != NULL)
        Nst_node_destroy(statement);
    return NULL;
}

static Nst_Node *parse_s_list_with_brackets(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    skip_blank();

    Nst_Span span = top_span();
    if (top_type() != Nst_TT_L_BRACKET) {
        set_error("expected '['", span);
        return NULL;
    }
    pop_top();
    Nst_Node *long_s = parse_s_list();
    if (long_s == NULL)
        return NULL;

    skip_blank();

    if (top_type() != Nst_TT_R_BRACKET) {
        set_error("unmatched '['", span);
        Nst_node_destroy(long_s);
        return NULL;
    }
    span = Nst_span_expand(span, top_end());
    pop_top();

    Nst_Node *wrapper = new_node(Nst_NT_S_WRAPPER, span);
    if (wrapper == NULL) {
        Nst_node_destroy(long_s);
        return NULL;
    }
    wrapper->v.s_wrapper.statement = long_s;
    Nst_node_set_span(wrapper, span);

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
        pop_top();

        Nst_Node *long_s = parse_s_list();
        if (long_s == NULL)
            return NULL;

        if (top_type() != Nst_TT_R_BRACKET) {
            Nst_node_destroy(long_s);
            set_error("unmatched '['", Nst_span_from_pos(start));
            return NULL;
        }
        pop_top();
        exit_func(&initial_state);
        return long_s;
    } else if (tok_type == Nst_TT_WHILE || tok_type == Nst_TT_DOWHILE) {
        Nst_Node *while_l = parse_s_while_lp();
        exit_func(&initial_state);
        return while_l;
    } else if (tok_type == Nst_TT_FOR) {
        Nst_Node *for_l = parse_s_for_lp();
        exit_func(&initial_state);
        return for_l;
    } else if (tok_type == Nst_TT_FUNC) {
        Nst_Node *func_declr = parse_s_fn_decl();
        exit_func(&initial_state);
        return func_declr;
    } else if (tok_type == Nst_TT_SWITCH) {
        Nst_Node *switch_s = parse_s_switch();
        exit_func(&initial_state);
        return switch_s;
    } else if (tok_type == Nst_TT_RETURN) {
        Nst_Span span = top_span();
        pop_top();

        if (!state.in_func) {
            set_error("'=>' outside of a function", span);
            return NULL;
        }

        Nst_Node *expr;

        tok_type = top_type();
        if (_Nst_TOK_IS_EXPR_END(tok_type)) {
            expr = NULL;
        } else {
            expr = parse_expr();
            if (expr == NULL)
                return NULL;
            span = Nst_span_join(span, expr->span);
        }
        Nst_Node *return_s = new_node(Nst_NT_S_RETURN, span);
        if (return_s == NULL) {
            if (expr != NULL)
                Nst_node_destroy(expr);
            return NULL;
        }
        Nst_node_set_span(return_s, span);
        return_s->v.s_return.value = expr;
        exit_func(&initial_state);
        return return_s;
    } else if (tok_type == Nst_TT_CONTINUE) {
        Nst_Span span = top_span();
        pop_top();

        if (!state.in_loop && !state.in_switch) {
            set_error("'..' outside of a loop", span);
            return NULL;
        }

        Nst_Node *continue_s = new_node(Nst_NT_S_CONTINUE, span);
        if (continue_s == NULL)
            return NULL;
        Nst_node_set_span(continue_s, span);
        exit_func(&initial_state);
        return continue_s;
    } else if (tok_type == Nst_TT_BREAK) {
        Nst_Span span = top_span();
        pop_top();

        if (!state.in_loop) {
            set_error("';' outside of a loop", span);
            return NULL;
        }

        Nst_Node *break_s = new_node(Nst_NT_S_BREAK, span);
        if (break_s == NULL)
            return NULL;
        Nst_node_set_span(break_s, span);
        exit_func(&initial_state);
        return break_s;
    } else if (tok_type == Nst_TT_TRY) {
        Nst_Node *try_catch_s = parse_s_try_catch();
        exit_func(&initial_state);
        return try_catch_s;
    } else {
        Nst_Node *expr = parse_expr();
        exit_func(&initial_state);
        return expr;
    }
}

static Nst_Node *parse_s_while_lp(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Span span = top_span();
    Nst_Node *while_l = new_node(Nst_NT_S_WHILE_LP, span);
    if (while_l == NULL)
        return NULL;
    while_l->v.s_while_lp.is_dowhile = top_type() == Nst_TT_DOWHILE;
    pop_top();
    skip_blank();
    state.endl_ends_expr = false;
    Nst_Node *condition = parse_expr();
    state.endl_ends_expr = initial_state.endl_ends_expr;
    if (condition == NULL) {
        Nst_node_destroy(while_l);
        return NULL;
    }
    while_l->v.s_while_lp.condition = condition;
    state.in_loop = true;
    Nst_Node *body = parse_s_list_with_brackets();
    if (body == NULL) {
        Nst_node_destroy(while_l);
        return NULL;
    }
    while_l->v.s_while_lp.body = body;
    Nst_node_set_span(while_l, Nst_span_join(span, body->span));
    exit_func(&initial_state);
    return while_l;
}

static Nst_Node *parse_s_for_lp(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Span span = top_span();
    Nst_Node *for_l = new_node(Nst_NT_S_FOR_LP, span);
    if (for_l == NULL)
        return NULL;
    pop_top();
    skip_blank();
    state.endl_ends_expr = false;
    Nst_Node *iterator = parse_expr();
    state.endl_ends_expr = initial_state.endl_ends_expr;
    if (iterator == NULL) {
        Nst_node_destroy(for_l);
        return NULL;
    }
    for_l->v.s_for_lp.iterator = iterator;
    skip_blank();
    if (top_type() == Nst_TT_AS) {
        pop_top();
        Nst_Node *assignment = parse_e_assignment_name(false);
        if (assignment == NULL) {
            Nst_node_destroy(for_l);
            return NULL;
        }
        for_l->v.s_for_lp.assignment = assignment;
        skip_blank();
    }
    state.in_loop = true;
    Nst_Node *body = parse_s_list_with_brackets();
    if (body == NULL) {
        Nst_node_destroy(for_l);
        return NULL;
    }
    for_l->v.s_for_lp.body = body;
    Nst_node_set_span(for_l, Nst_span_join(span, body->span));
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
    pop_top();
    Nst_Node *if_e = new_node(Nst_NT_E_IF, condition->span);
    if (if_e == NULL) {
        Nst_node_destroy(condition);
        return NULL;
    }
    if_e->v.e_if.condition = condition;
    skip_blank();
    Nst_Node *body_if_true = parse_statement();
    if (body_if_true == NULL) {
        Nst_node_destroy(if_e);
        return NULL;
    }
    if_e->v.e_if.body_if_true = body_if_true;
    skip_blank();
    if (top_type() != Nst_TT_COLON) {
        Nst_node_set_span(
            if_e,
            Nst_span_join(condition->span, body_if_true->span));
        exit_func(&initial_state);
        return if_e;
    }
    pop_top();
    skip_blank();
    Nst_Node *body_if_false = parse_statement();
    if (body_if_false == NULL) {
        Nst_node_destroy(if_e);
        return NULL;
    }
    if_e->v.e_if.body_if_false = body_if_false;
    Nst_node_set_span(
        if_e,
        Nst_span_join(condition->span, body_if_false->span));
    exit_func(&initial_state);
    return if_e;
}

static Nst_Node *parse_s_switch(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Span span = top_span();
    pop_top();

    Nst_Node *switch_s = new_node(Nst_NT_S_SWITCH, span);
    if (switch_s == NULL)
        return NULL;

    state.endl_ends_expr = false;
    Nst_Node *expr = parse_expr();
    state.endl_ends_expr = initial_state.endl_ends_expr;
    if (expr == NULL) {
        Nst_node_destroy(switch_s);
        return NULL;
    }
    switch_s->v.s_switch.expr = expr;
    skip_blank();
    if (top_type() != Nst_TT_L_BRACKET) {
        Nst_node_destroy(switch_s);
        set_error("expected '['", top_span());
        return NULL;
    }
    pop_top();
    while (true) {
        skip_blank();
        if (top_type() == Nst_TT_R_BRACKET)
            break;
        if (top_type() != Nst_TT_IF) {
            Nst_node_destroy(switch_s);
            set_error("expected '?'", top_span());
            return NULL;
        }
        pop_top();
        skip_blank();

        if (top_type() == Nst_TT_L_BRACKET) {
            state.in_switch = true;
            Nst_Node *default_body = parse_s_list_with_brackets();
            if (default_body == NULL) {
                Nst_node_destroy(switch_s);
                return NULL;
            }
            switch_s->v.s_switch.default_body = default_body;
            break;
        }
        state.endl_ends_expr = false;
        Nst_Node *value = parse_expr();
        state.endl_ends_expr = initial_state.endl_ends_expr;
        if (value == NULL) {
            Nst_node_destroy(switch_s);
            return NULL;
        }
        if (!append_node(&switch_s->v.s_switch.values, value)) {
            Nst_node_destroy(value);
            Nst_node_destroy(switch_s);
            return NULL;
        }
        state.in_switch = true;
        Nst_Node *body = parse_s_list_with_brackets();
        state.in_switch = initial_state.in_switch;
        if (body == NULL) {
            Nst_node_destroy(switch_s);
            return NULL;
        }
        if (!append_node(&switch_s->v.s_switch.bodies, body)) {
            Nst_node_destroy(body);
            Nst_node_destroy(switch_s);
            return NULL;
        }
    }
    skip_blank();
    if (top_type() != Nst_TT_R_BRACKET) {
        Nst_node_destroy(switch_s);
        set_error("expected ']'", top_span());
        return NULL;
    }
    Nst_node_set_span(switch_s, Nst_span_expand(span, top_end()));
    pop_top();
    exit_func(&initial_state);
    return switch_s;
}

static Nst_Node *parse_s_fn_decl(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Span span = top_span();
    Nst_Node *func_declr = new_node(Nst_NT_S_FN_DECL, span);
    if (func_declr == NULL)
        return NULL;

    bool is_lambda = top_type() == Nst_TT_LAMBDA;
    pop_top();
    skip_blank();
    if (!is_lambda && top_type() != Nst_TT_IDENT) {
        Nst_node_destroy(func_declr);
        set_error("expected an identifier", top_span());
        return NULL;
    }
    if (!is_lambda)
        func_declr->v.s_fn_decl.name = Nst_inc_ref(pop_top()->value);

    skip_blank();
    while (top_type() == Nst_TT_IDENT) {
        Nst_Tok *tok = peek_top();
        if (!append_tok_val(&func_declr->v.s_fn_decl.argument_names, tok)) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        pop_top();
        skip_blank();
    }

    if (top_type() == Nst_TT_RETURN) {
        Nst_Span body_span = top_span();
        Nst_Node *return_s = new_node(
            Nst_NT_S_RETURN,
            Nst_span_expand(body_span, top_end()));
        if (return_s == NULL) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        pop_top();
        func_declr->v.s_fn_decl.body = return_s;
        Nst_Node *expr = parse_expr();
        if (expr == NULL) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        return_s->v.s_return.value = expr;
        Nst_node_set_span(return_s, Nst_span_join(body_span, expr->span));
        span = Nst_span_join(span, expr->span);
    } else {
        state.in_func = true;
        Nst_Node *body = parse_s_list_with_brackets();
        if (body == NULL) {
            Nst_node_destroy(func_declr);
            return NULL;
        }
        func_declr->v.s_fn_decl.body = body;
        span = Nst_span_join(span, body->span);
    }
    Nst_node_set_span(func_declr, span);
    exit_func(&initial_state);
    return func_declr;
}

static Nst_Node *parse_stack_expr()
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_PtrArray values;
    if (!Nst_pa_init(&values, 3)) {
        Nst_error_add_span(top_span());
        return NULL;
    }

    Nst_Pos start = top_start();

    while (true) {
        if (!state.endl_ends_expr)
            skip_blank();

        while (_Nst_TOK_IS_ATOM(top_type())) {
            Nst_Node *atom = parse_e_extraction();
            if (atom == NULL)
                goto failure;
            if (!append_node(&values, atom)) {
                Nst_node_destroy(atom);
                goto failure;
            }
            if (!state.endl_ends_expr)
                skip_blank();
        }

        // this can be true only in the first iteration, all subsequent
        // iterations will have at least the values from the previous one
        if (values.len == 0 && !_Nst_TOK_IS_LOCAL_STACK_OP(top_type())) {
            set_error("expected a value", top_span());
            goto failure;
        }

        Nst_Node *operation_node = NULL;
        if (_Nst_TOK_IS_STACK_OP(top_type()))
            operation_node = parse_e_stack_op(&values, start);
        else if (_Nst_TOK_IS_LOCAL_STACK_OP(top_type()))
            operation_node = parse_e_loc_stack_op(&values, start);
        else if (_Nst_TOK_IS_ASSIGNMENT(top_type()))
            operation_node = parse_e_assignment(&values, start);
        else if (_Nst_TOK_IS_EXPR_END(top_type()) && state.endl_ends_expr)
            break;
        else if (_Nst_TOK_IS_EXPR_END(top_type()) && top_type() != Nst_TT_ENDL)
            break;
        else if (state.break_ends_expr && top_type() == Nst_TT_BREAK)
            break;
        else {
            set_error("unexpected token", top_span());
            goto failure;
        }

        if (operation_node == NULL)
            goto failure;

        if (!append_node(&values, operation_node)) {
            Nst_node_destroy(operation_node);
            goto failure;
        }
    }

    // The expression end token must not be consumed

    if (values.len != 1) {
        set_error("expected stack or local stack operator", top_span());
        goto failure;
    }

    Nst_Node *expr = GET_NODE(&values, 0);
    Nst_pa_clear(&values, NULL);
    exit_func(&initial_state);
    return expr;

failure:
    Nst_pa_clear(&values, (Nst_Destructor)Nst_node_destroy);
    return NULL;
}

static Nst_Node *parse_e_stack_op(Nst_PtrArray *values, Nst_Pos start)
{
    Nst_Node *stack_op = new_node(Nst_NT_E_STACK_OP, top_span());
    if (stack_op == NULL)
        return NULL;
    Nst_node_set_span(stack_op, Nst_span_new(start, top_end()));
    swap_pa(&stack_op->v.e_stack_op.values, values);
    stack_op->v.e_stack_op.op = top_type();
    pop_top();
    return stack_op;
}

static Nst_Node *parse_e_loc_stack_op(Nst_PtrArray *values, Nst_Pos start)
{
    Nst_Pos args_end;
    if (values->len == 0)
        args_end = start;
    else
        args_end = Nst_span_end(GET_NODE(values, values->len - 1)->span);

    usize arg_num = values->len;
    if (!check_local_stack_op_arg_num(arg_num, Nst_span_new(start, args_end)))
        return NULL;
    Nst_TokType op = top_type();
    pop_top();
    skip_blank();
    Nst_Node *special_node = parse_e_extraction();
    if (special_node == NULL)
        return NULL;
    Nst_Node *local_stack_op = new_node(
        Nst_NT_E_LOC_STACK_OP,
        Nst_span_expand(special_node->span, start));
    if (local_stack_op == NULL) {
        Nst_node_destroy(special_node);
        return NULL;
    }
    local_stack_op->v.e_loc_stack_op.op = op;
    Nst_node_set_span(
        local_stack_op,
        Nst_span_expand(special_node->span, start));
    swap_pa(&local_stack_op->v.e_loc_stack_op.values, values);
    local_stack_op->v.e_loc_stack_op.special_value = special_node;
    local_stack_op->v.e_loc_stack_op.op = op;
    return local_stack_op;
}

static bool check_local_stack_op_arg_num(usize arg_num, Nst_Span span)
{
    Nst_TokType type = top_type();
    if (type == Nst_TT_CAST && arg_num != 1) {
        set_error("'::' expects only 1 argument on the left", span);
        return false;
    } else if (type == Nst_TT_RANGE && arg_num != 1 && arg_num != 2) {
        set_error("'->' expects only 1 or 2 arguments on the left", span);
        return false;
    } else if (type == Nst_TT_THROW && arg_num != 1) {
        set_error("'!!' expects only 1 argument on the left", span);
        return false;
    } else if (type == Nst_TT_SEQ_CALL && arg_num != 1) {
        set_error("'*@' expects only 1 argument on the left", span);
        return false;
    }
    return true;
}

static Nst_Node *parse_e_assignment_name(bool is_compound)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Span span = top_span();
    Nst_Node *name;

    if (top_type() != Nst_TT_L_BRACE) {
        name = parse_e_extraction();
        if (name == NULL)
            return NULL;
        if (name->type != Nst_NT_E_ACCESS
            && name->type != Nst_NT_E_EXTRACTION)
        {
            set_error("expected an identifier or an extraction", name->span);
            Nst_node_destroy(name);
            return NULL;
        }
        exit_func(&initial_state);
        return name;
    }

    if (is_compound) {
        set_error("cannot unpack values in a compound assignment", top_span());
        return NULL;
    }

    name = new_node(Nst_NT_E_SEQ_LITERAL, top_span());
    if (name == NULL)
        return NULL;
    name->v.e_seq_literal.type = Nst_SNT_ASSIGNMENT_NAMES;
    pop_top();

    state.endl_ends_expr = false;

    while (true) {
        skip_blank();
        Nst_Node *sub_name = parse_e_assignment_name(false);
        if (sub_name == NULL)
            goto failure;
        if (!append_node(&name->v.e_seq_literal.values, sub_name)) {
            Nst_node_destroy(sub_name);
            goto failure;
        }
        skip_blank();
        if (top_type() == Nst_TT_COMMA)
            pop_top();
        else if (top_type() == Nst_TT_R_BRACE) {
            span = Nst_span_expand(span, top_end());
            pop_top();
            break;
        } else {
            set_error("expected ',' or '}'", top_span());
            goto failure;
        }
    }
    Nst_node_set_span(name, span);
    exit_func(&initial_state);
    return name;

failure:
    if (name != NULL)
        Nst_node_destroy(name);
    return NULL;
}

static Nst_Node *parse_e_assignment(Nst_PtrArray *values, Nst_Pos start)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_TokType type = top_type();
    bool is_compound = type != Nst_TT_ASSIGN;

    if (values->len > 1 && !is_compound) {
        set_error(
            "expected stack or local stack operator",
            top_span());
        return NULL;
    }
    pop_top();

    Nst_Node *name = parse_e_assignment_name(is_compound);
    if (name == NULL)
        return NULL;

    Nst_Node *assignment = new_node(
        is_compound ? Nst_NT_E_COMP_ASSIGN : Nst_NT_E_ASSIGNMENT,
        Nst_span_expand(name->span, start));

    if (assignment == NULL) {
        Nst_node_destroy(name);
        return NULL;
    }
    Nst_node_set_span(assignment, Nst_span_expand(name->span, start));

    if (is_compound) {
        swap_pa(&assignment->v.e_comp_assignment.values, values);
        assignment->v.e_comp_assignment.name = name;
        assignment->v.e_comp_assignment.op =
            _Nst_TOK_ASSIGNMENT_TO_STACK_OP(type);
    } else {
        assignment->v.e_assignment.name = name;
        assignment->v.e_assignment.value = GET_NODE(values, 0);
        Nst_pa_clear(values, NULL);
    }

    exit_func(&initial_state);
    return assignment;
}

static Nst_Node *parse_e_extraction(void)
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
        pop_top();
        Nst_Node *key;

        if (!state.endl_ends_expr)
            skip_blank();

        if (top_type() == Nst_TT_IDENT) {
            key = new_node(Nst_NT_E_VALUE, top_span());
            if (key == NULL) {
                Nst_node_destroy(container);
                return NULL;
            }
            Nst_node_set_span(key, top_span());
            key->v.e_value.value = Nst_inc_ref(pop_top()->value);
        } else {
            key = parse_atom();
            if (key == NULL) {
                Nst_node_destroy(container);
                return NULL;
            }
        }

        Nst_Node *extraction = new_node(
            Nst_NT_E_EXTRACTION,
            Nst_span_join(container->span, key->span));
        if (extraction == NULL) {
            Nst_node_destroy(key);
            Nst_node_destroy(container);
            return NULL;
        }
        Nst_node_set_span(
            extraction,
            Nst_span_join(container->span, key->span));
        extraction->v.e_extraction.key = key;
        extraction->v.e_extraction.container = container;
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
        atom = new_node(Nst_NT_E_VALUE, top_span());
        if (atom == NULL)
            return NULL;
        Nst_node_set_span(atom, top_span());
        atom->v.e_value.value = Nst_inc_ref(pop_top()->value);
    } else if (top_type() == Nst_TT_IDENT) {
        atom = new_node(Nst_NT_E_ACCESS, top_span());
        if (atom == NULL)
            return NULL;
        Nst_node_set_span(atom, top_span());
        atom->v.e_access.value = Nst_inc_ref(pop_top()->value);
    } else if (top_type() == Nst_TT_L_PAREN) {
        Nst_Span span = top_span();
        pop_top();
        state.endl_ends_expr = false;
        Nst_Node *expr = parse_expr();
        if (expr == NULL)
            return NULL;
        if (top_type() != Nst_TT_R_PAREN) {
            set_error("unmatched '('", span);
            Nst_node_destroy(expr);
            return NULL;
        }
        span = Nst_span_expand(span, top_end());
        pop_top();

        atom = new_node(Nst_NT_E_WRAPPER, expr->span);
        if (atom == NULL) {
            Nst_node_destroy(expr);
            return NULL;
        }
        atom->v.e_wrapper.expr = expr;
        Nst_node_set_span(atom, span);
    } else if (_Nst_TOK_IS_LOCAL_OP(top_type())) {
        atom = new_node(Nst_NT_E_LOCAL_OP, top_span());
        if (atom == NULL)
            return NULL;
        Nst_Pos start = top_start();
        atom->v.e_local_op.op = top_type();
        pop_top();

        Nst_Node *value = parse_e_extraction();
        if (value == NULL) {
            Nst_node_destroy(atom);
            return NULL;
        }
        atom->v.e_local_op.value = value;
        Nst_node_set_span(atom, Nst_span_expand(value->span, start));
    } else if (top_type() == Nst_TT_CALL) {
        atom = new_node(Nst_NT_E_LOCAL_OP, top_span());
        if (atom == NULL)
            return NULL;
        Nst_Pos start = top_start();
        atom->v.e_local_op.op = Nst_TT_LOC_CALL;
        pop_top();

        Nst_Node *value = parse_e_extraction();
        if (value == NULL) {
            Nst_node_destroy(atom);
            return NULL;
        }
        atom->v.e_local_op.value = value;
        Nst_node_set_span(atom, Nst_span_expand(value->span, start));
    } else if (top_type() == Nst_TT_L_VBRACE) {
        atom = parse_vector_literal();
        if (atom == NULL)
            return NULL;
    } else if (top_type() == Nst_TT_L_BRACE) {
        atom = parse_arr_or_map_literal();
        if (atom == NULL)
            return NULL;
    } else if (top_type() == Nst_TT_LAMBDA) {
        atom = parse_s_fn_decl();
        if (atom == NULL)
            return NULL;
    } else {
        set_error("expected a value", top_span());
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

    pop_top();
    skip_blank();
    if (top_type() == Nst_TT_R_VBRACE) {
        vec_lit = new_node(
            Nst_NT_E_SEQ_LITERAL,
            Nst_span_new(start, top_end()));
        vec_lit->v.e_seq_literal.type = Nst_SNT_VECTOR;
        if (vec_lit == NULL)
            return NULL;
        Nst_node_set_span(vec_lit, Nst_span_new(start, top_end()));
        pop_top();
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
    pop_top();
    skip_blank();

    if (top_type() == Nst_TT_R_BRACE) {
        Nst_Node *map_lit = new_node(
            Nst_NT_E_MAP_LITERAL,
            Nst_span_new(start, top_end()));
        if (map_lit == NULL)
            return NULL;
        Nst_node_set_span(map_lit, Nst_span_new(start, top_end()));
        pop_top();
        exit_func(&initial_state);
        return map_lit;
    } else if (top_type() == Nst_TT_COMMA) {
        pop_top();
        skip_blank();
        if (top_type() != Nst_TT_R_BRACE) {
            set_error("expected '}'", top_span());
            return NULL;
        }
        Nst_Node *arr_lit = new_node(
            Nst_NT_E_SEQ_LITERAL,
            Nst_span_new(start, top_end()));
        if (arr_lit == NULL)
            return NULL;
        Nst_node_set_span(arr_lit, Nst_span_new(start, top_end()));
        arr_lit->v.e_seq_literal.type = Nst_SNT_ARRAY;
        pop_top();
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

    Nst_Node *seq_lit = new_node(
        Nst_NT_E_SEQ_LITERAL,
        Nst_span_from_pos(start));
    if (seq_lit == NULL)
        goto failure;

    state.endl_ends_expr = false;
    state.break_ends_expr = true;

    if (first_node == NULL) {
        first_node = parse_expr();
        if (first_node == NULL)
            goto failure;
    }

    if (!append_node(&seq_lit->v.e_seq_literal.values, first_node)) {
        Nst_node_destroy(first_node);
        Nst_node_destroy(seq_lit);
        return NULL;
    }
    first_node = NULL;
    skip_blank();
    if (top_type() == Nst_TT_BREAK) {
        pop_top();
        Nst_Node *length = parse_expr();
        if (length == NULL)
            goto failure;
        skip_blank();
        if (top_type() != closing_paren) {
            set_error(expected_paren, top_span());
            Nst_node_destroy(length);
            goto failure;
        }
        if (!append_node(&seq_lit->v.e_seq_literal.values, length)) {
            Nst_node_destroy(length);
            goto failure;
        }
        Nst_node_set_span(seq_lit, Nst_span_new(start, top_end()));
        pop_top();
        seq_lit->v.e_seq_literal.type =
            arr ? Nst_SNT_ARRAY_REP : Nst_SNT_VECTOR_REP;
        exit_func(&initial_state);
        return seq_lit;
    }
    seq_lit->v.e_seq_literal.type = arr ? Nst_SNT_ARRAY : Nst_SNT_VECTOR;

    goto from_comma;

    while (true) {
        skip_blank();
        Nst_Node *value = parse_expr();
        if (value == NULL)
            goto failure;
        if (!append_node(&seq_lit->v.e_seq_literal.values, value)) {
            Nst_node_destroy(value);
            goto failure;
        }
    from_comma:
        skip_blank();

        if (top_type() == Nst_TT_COMMA)
            pop_top();
        else if (top_type() == closing_paren) {
            Nst_node_set_span(seq_lit, Nst_span_new(start, top_end()));
            pop_top();
            exit_func(&initial_state);
            return seq_lit;
        } else {
            set_error(expected_comma_or_paren, top_span());
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
    Nst_Node *map_lit = new_node(
        Nst_NT_E_MAP_LITERAL,
        Nst_span_from_pos(start));
    if (map_lit == NULL)
        goto failure;

    if (!append_node(&map_lit->v.e_map_literal.keys, key))
        goto failure;
    key = NULL;
    goto from_colon;

    while (true) {
        key = parse_expr();
        if (key == NULL)
            goto failure;
        if (!append_node(&map_lit->v.e_map_literal.keys, key))
            goto failure;
        key = NULL;

    from_colon:
        skip_blank();
        if (top_type() != Nst_TT_COLON) {
            set_error("expected ':'", top_span());
            goto failure;
        }
        pop_top();
        value = parse_expr();
        if (value == NULL)
            goto failure;
        if (!append_node(&map_lit->v.e_map_literal.values, value))
            goto failure;
        value = NULL;

        if (top_type() == Nst_TT_R_BRACE) {
            Nst_node_set_span(map_lit, Nst_span_new(start, top_end()));
            pop_top();
            exit_func(&initial_state);
            return map_lit;
        } else if (top_type() == Nst_TT_COMMA)
            pop_top();
        else {
            set_error("expected ',' or '}'", top_span());
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

static Nst_Node *parse_s_try_catch(void)
{
    ParsingState initial_state;
    if (!enter_func(&initial_state))
        return NULL;

    Nst_Pos start = top_start();
    pop_top();

    Nst_Node *try_catch_s = new_node(
        Nst_NT_S_TRY_CATCH,
        Nst_span_from_pos(start));
    if (try_catch_s == NULL)
        goto failure;

    state.endl_ends_expr = true;
    state.break_ends_expr = false;

    Nst_Node *try_body = parse_statement();
    if (try_body == NULL)
        goto failure;
    try_catch_s->v.s_try_catch.try_body = try_body;
    skip_blank();

    if (top_type() != Nst_TT_CATCH) {
        set_error("expected '?!'", top_span());
        goto failure;
    }
    pop_top();
    if (top_type() != Nst_TT_IDENT) {
        set_error("expected an identifier", top_span());
        goto failure;
    }

    try_catch_s->v.s_try_catch.error_name = Nst_inc_ref(pop_top()->value);
    skip_blank();
    Nst_Node *catch_body = parse_statement();
    if (catch_body == NULL)
        goto failure;
    try_catch_s->v.s_try_catch.catch_body = catch_body;

    Nst_node_set_span(try_catch_s, Nst_span_expand(catch_body->span, start));
    exit_func(&initial_state);
    return try_catch_s;

failure:
    if (try_catch_s != NULL)
        Nst_node_destroy(try_catch_s);
    return NULL;
}
