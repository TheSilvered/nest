#include "nodes.h"
#include "mem.h"
#include "format.h"

#define DESTROY_NODE_IF_NOT_NULL(node)                                        \
    if (node != NULL) Nst_node_destroy(node);

#define DESTROY_TOK_IF_NOT_NULL(tok)                                          \
    if (tok != NULL) Nst_tok_destroy(tok);

#define DESTROY_NODE_LLIST(llist)                                             \
    Nst_llist_destroy(llist, (Nst_LListDestructor)Nst_node_destroy)

#define DESTROY_TOK_LLIST(llist)                                              \
    Nst_llist_destroy(llist, (Nst_LListDestructor)Nst_tok_destroy)

// extract property, used to avoid mistakes when printing node data
#define EP(data_name, name) #name, node->data_name.name, levels

static void print_long_s(Nst_Node *node, Nst_LList *levels);
static void print_while_l(Nst_Node *node, Nst_LList *levels);
static void print_for_l(Nst_Node *node, Nst_LList *levels);
static void print_func_declr(Nst_Node *node, Nst_LList *levels);
static void print_return_s(Nst_Node *node, Nst_LList *levels);
static void print_switch_s(Nst_Node *node, Nst_LList *levels);
static void print_try_catch_s(Nst_Node *node, Nst_LList *levels);
static void print_s_wrapper(Nst_Node *node, Nst_LList *levels);
static void print_stack_op(Nst_Node *node, Nst_LList *levels);
static void print_local_stack_op(Nst_Node *node, Nst_LList *levels);
static void print_local_op(Nst_Node *node, Nst_LList *levels);
static void print_seq_lit(Nst_Node *node, Nst_LList *levels);
static void print_map_lit(Nst_Node *node, Nst_LList *levels);
static void print_value(Nst_Node *node, Nst_LList *levels);
static void print_access(Nst_Node *node, Nst_LList *levels);
static void print_extract_e(Nst_Node *node, Nst_LList *levels);
static void print_assign_e(Nst_Node *node, Nst_LList *levels);
static void print_comp_assign_e(Nst_Node *node, Nst_LList *levels);
static void print_if_e(Nst_Node *node, Nst_LList *levels);
static void print_e_wrapper(Nst_Node *node, Nst_LList *levels);

bool (*initializers[])(Nst_Node *) = {
    [Nst_NT_LONG_S]         = _Nst_node_long_s_init,
    [Nst_NT_WHILE_L]        = _Nst_node_while_l_init,
    [Nst_NT_FOR_L]          = _Nst_node_for_l_init,
    [Nst_NT_FUNC_DECLR]     = _Nst_node_func_declr_init,
    [Nst_NT_RETURN_S]       = _Nst_node_return_s_init,
    [Nst_NT_CONTINUE_S]     = NULL,
    [Nst_NT_BREAK_S]        = NULL,
    [Nst_NT_SWITCH_S]       = _Nst_node_switch_s_init,
    [Nst_NT_TRY_CATCH_S]    = _Nst_node_try_catch_s_init,
    [Nst_NT_S_WRAPPER]      = _Nst_node_s_wrapper_init,
    [Nst_NT_STACK_OP]       = _Nst_node_stack_op_init,
    [Nst_NT_LOCAL_STACK_OP] = _Nst_node_local_stack_op_init,
    [Nst_NT_LOCAL_OP]       = _Nst_node_local_op_init,
    [Nst_NT_SEQ_LIT]        = _Nst_node_seq_lit_init,
    [Nst_NT_MAP_LIT]        = _Nst_node_map_lit_init,
    [Nst_NT_VALUE]          = _Nst_node_value_init,
    [Nst_NT_ACCESS]         = _Nst_node_access_init,
    [Nst_NT_EXTRACT_E]      = _Nst_node_extract_e_init,
    [Nst_NT_ASSIGN_E]       = _Nst_node_assign_e_init,
    [Nst_NT_COMP_ASSIGN_E]  = _Nst_node_comp_assign_e_init,
    [Nst_NT_IF_E]           = _Nst_node_if_e_init,
    [Nst_NT_E_WRAPPER]      = _Nst_node_e_wrapper_init,
};

void (*destructors[])(Nst_Node *) = {
    [Nst_NT_LONG_S]         = _Nst_node_long_s_destroy,
    [Nst_NT_WHILE_L]        = _Nst_node_while_l_destroy,
    [Nst_NT_FOR_L]          = _Nst_node_for_l_destroy,
    [Nst_NT_FUNC_DECLR]     = _Nst_node_func_declr_destroy,
    [Nst_NT_RETURN_S]       = _Nst_node_return_s_destroy,
    [Nst_NT_CONTINUE_S]     = NULL,
    [Nst_NT_BREAK_S]        = NULL,
    [Nst_NT_SWITCH_S]       = _Nst_node_switch_s_destroy,
    [Nst_NT_TRY_CATCH_S]    = _Nst_node_try_catch_s_destroy,
    [Nst_NT_S_WRAPPER]      = _Nst_node_s_wrapper_destroy,
    [Nst_NT_STACK_OP]       = _Nst_node_stack_op_destroy,
    [Nst_NT_LOCAL_STACK_OP] = _Nst_node_local_stack_op_destroy,
    [Nst_NT_LOCAL_OP]       = _Nst_node_local_op_destroy,
    [Nst_NT_SEQ_LIT]        = _Nst_node_seq_lit_destroy,
    [Nst_NT_MAP_LIT]        = _Nst_node_map_lit_destroy,
    [Nst_NT_VALUE]          = _Nst_node_value_destroy,
    [Nst_NT_ACCESS]         = _Nst_node_access_destroy,
    [Nst_NT_EXTRACT_E]      = _Nst_node_extract_e_destroy,
    [Nst_NT_ASSIGN_E]       = _Nst_node_assign_e_destroy,
    [Nst_NT_COMP_ASSIGN_E]  = _Nst_node_comp_assign_e_destroy,
    [Nst_NT_IF_E]           = _Nst_node_if_e_destroy,
    [Nst_NT_E_WRAPPER]      = _Nst_node_e_wrapper_destroy
};

void (*prints[])(Nst_Node *, Nst_LList *) = {
    [Nst_NT_LONG_S]         = print_long_s,
    [Nst_NT_WHILE_L]        = print_while_l,
    [Nst_NT_FOR_L]          = print_for_l,
    [Nst_NT_FUNC_DECLR]     = print_func_declr,
    [Nst_NT_RETURN_S]       = print_return_s,
    [Nst_NT_CONTINUE_S]     = NULL,
    [Nst_NT_BREAK_S]        = NULL,
    [Nst_NT_SWITCH_S]       = print_switch_s,
    [Nst_NT_TRY_CATCH_S]    = print_try_catch_s,
    [Nst_NT_S_WRAPPER]      = print_s_wrapper,
    [Nst_NT_STACK_OP]       = print_stack_op,
    [Nst_NT_LOCAL_STACK_OP] = print_local_stack_op,
    [Nst_NT_LOCAL_OP]       = print_local_op,
    [Nst_NT_SEQ_LIT]        = print_seq_lit,
    [Nst_NT_MAP_LIT]        = print_map_lit,
    [Nst_NT_VALUE]          = print_value,
    [Nst_NT_ACCESS]         = print_access,
    [Nst_NT_EXTRACT_E]      = print_extract_e,
    [Nst_NT_ASSIGN_E]       = print_assign_e,
    [Nst_NT_COMP_ASSIGN_E]  = print_comp_assign_e,
    [Nst_NT_IF_E]           = print_if_e,
    [Nst_NT_E_WRAPPER]      = print_e_wrapper
};

const i8 *nt_strings[] = {
    [Nst_NT_LONG_S]         = "LONG_S",
    [Nst_NT_WHILE_L]        = "WHILE_L",
    [Nst_NT_FOR_L]          = "FOR_L",
    [Nst_NT_FUNC_DECLR]     = "FUNC_DECLR",
    [Nst_NT_RETURN_S]       = "RETURN_S",
    [Nst_NT_CONTINUE_S]     = "CONTINUE_S",
    [Nst_NT_BREAK_S]        = "BREAK_S",
    [Nst_NT_SWITCH_S]       = "SWITCH_S",
    [Nst_NT_TRY_CATCH_S]    = "TRY_CATCH_S",
    [Nst_NT_S_WRAPPER]      = "S_WRAPPER",
    [Nst_NT_STACK_OP]       = "STACK_OP",
    [Nst_NT_LOCAL_STACK_OP] = "LOCAL_STACK_OP",
    [Nst_NT_LOCAL_OP]       = "LOCAL_OP",
    [Nst_NT_SEQ_LIT]        = "SEQ_LIT",
    [Nst_NT_MAP_LIT]        = "MAP_LIT",
    [Nst_NT_VALUE]          = "VALUE",
    [Nst_NT_ACCESS]         = "ACCESS",
    [Nst_NT_EXTRACT_E]      = "EXTRACT_E",
    [Nst_NT_ASSIGN_E]       = "ASSIGN_E",
    [Nst_NT_COMP_ASSIGN_E]  = "COMP_ASSIGN_E",
    [Nst_NT_IF_E]           = "IF_E",
    [Nst_NT_E_WRAPPER]      = "E_WRAPPER"
};

const i8 *snt_strings[] = {
    [Nst_SNT_ARRAY]            = "ARRAY",
    [Nst_SNT_ARRAY_REP]        = "ARRAY_REP",
    [Nst_SNT_VECTOR]           = "VECTOR",
    [Nst_SNT_VECTOR_REP]       = "VECTOR_REP",
    [Nst_SNT_ASSIGNMENT_NAMES] = "ASSIGNMENT_NAMES",
    [Nst_SNT_NOT_SET]          = "NOT_SET"
};

static void print_levels(Nst_LList *levels)
{
    for (Nst_LLIST_ITER(level, levels)) {
        if (level->value == NULL)
            Nst_print("   ");
        else
            Nst_print("\xE2\x94\x82  ");
    }
}

static bool print_connection(i8 last, bool add_level, Nst_LList *levels)
{
    if (last == 1) {
        Nst_print("\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80");
        if (add_level && !Nst_llist_append(levels, NULL, false))
            return false;
    } else if (last == 0) {
        Nst_print("\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80");
        if (add_level && !Nst_llist_append(levels, (void *)1, false))
            return false;
    }
    return true;
}

static void print_node(const i8 *name, Nst_Node *node, Nst_LList *levels,
                       i8 last)
{
    print_levels(levels);
    if (!print_connection(last, true, levels))
        return;

    if (name != NULL)
        Nst_printf("%s: ", name);

    if (node != NULL) {
        Nst_printf(
            "%s (%li:%li, %li:%li)\n",
            nt_strings[node->type],
            node->start.line, node->start.col,
            node->end.line, node->end.col);
    } else
        Nst_print("(null)\n");

    if (prints[node->type] != NULL)
        prints[node->type](node, levels);

    Nst_llist_pop(levels);
}

static void print_node_list(const i8 *name, Nst_LList *nodes,
                            Nst_LList *levels, i8 last)
{
    print_levels(levels);
    if (!print_connection(last, true, levels))
        return;
    Nst_printf("%s:\n", name);

    for (Nst_LLIST_ITER(node, nodes)) {
        print_node(NULL, Nst_NODE(node->value), levels, node == nodes->tail);
    }
    Nst_llist_pop(levels);
}

static void print_bool(const i8 *name, bool value, Nst_LList *levels, i8 last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;
    Nst_printf("%s: %s\n", name, value ? "true" : "false");
}

static void print_token(const i8 *name, Nst_Tok* tok, Nst_LList *levels,
                        i8 last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;
    if (name != NULL)
        Nst_printf("%s: ", name);
    if (tok != NULL)
        Nst_print_tok(tok);
    else
        Nst_print("(null)\n");
    Nst_print("\n");
}

static void print_tok_type(const i8 *name, Nst_TokType tok_type,
                           Nst_LList *levels, i8 last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;

    Nst_printf("%s: %s\n", name, Nst_tok_type_to_str(tok_type));
}

static void print_tok_list(const i8 *name, Nst_LList *tokens,
                           Nst_LList *levels, i8 last)
{
    print_levels(levels);
    if (!print_connection(last, true, levels))
        return;
    Nst_printf("%s:\n", name);

    for (Nst_LLIST_ITER(token, tokens)) {
        print_token(
            NULL,
            Nst_NODE(token->value),
            levels,
            token == tokens->tail);
    }
    Nst_llist_pop(levels);
}

static void print_seq_node_type(const i8 *name, Nst_SeqNodeType snt,
                                Nst_LList *levels, i8 last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;

    Nst_printf("%s: %s\n", name, snt_strings[snt]);
}

bool _Nst_node_long_s_init(Nst_Node *node)
{
    node->long_s.statements = Nst_llist_new();
    return node->long_s.statements != NULL;
}

void _Nst_node_long_s_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->long_s.statements);
}

void print_long_s(Nst_Node *node, Nst_LList *levels)
{
    print_node_list(EP(long_s, statements), true);
}

bool _Nst_node_while_l_init(Nst_Node *node)
{
    node->while_l.condition = NULL;
    node->while_l.body = NULL;
    node->while_l.is_dowhile = false;
    return true;
}

void _Nst_node_while_l_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->while_l.condition);
    DESTROY_NODE_IF_NOT_NULL(node->while_l.body);
}

void print_while_l(Nst_Node *node, Nst_LList *levels)
{
    print_bool(EP(while_l, is_dowhile), false);
    print_node(EP(while_l, condition), false);
    print_node(EP(while_l, body), true);
}

bool _Nst_node_for_l_init(Nst_Node *node)
{
    node->for_l.iterator = NULL;
    node->for_l.assignment = NULL;
    node->for_l.body = NULL;
    return true;
}

void _Nst_node_for_l_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->for_l.iterator);
    DESTROY_NODE_IF_NOT_NULL(node->for_l.assignment);
    DESTROY_NODE_IF_NOT_NULL(node->for_l.body);
}

void print_for_l(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(for_l, iterator), false);
    print_node(EP(for_l, assignment), false);
    print_node(EP(for_l, body), true);
}

bool _Nst_node_func_declr_init(Nst_Node *node)
{
    node->func_declr.name = NULL;
    node->func_declr.body = NULL;
    node->func_declr.argument_names = Nst_llist_new();
    return node->func_declr.argument_names != NULL;
}

void _Nst_node_func_declr_destroy(Nst_Node *node)
{
    DESTROY_TOK_IF_NOT_NULL(node->func_declr.name);
    DESTROY_NODE_IF_NOT_NULL(node->func_declr.body);
    DESTROY_TOK_LLIST(node->func_declr.argument_names);
}

void print_func_declr(Nst_Node *node, Nst_LList *levels)
{
    print_token(EP(func_declr, name), false);
    print_tok_list(EP(func_declr, argument_names), false);
    print_node(EP(func_declr, body), true);
}

bool _Nst_node_return_s_init(Nst_Node *node)
{
    node->return_s.value = NULL;
    return true;
}

void _Nst_node_return_s_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->return_s.value);
}

void print_return_s(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(return_s, value), true);
}

bool _Nst_node_switch_s_init(Nst_Node *node)
{
    node->switch_s.expr = NULL;
    node->switch_s.values = Nst_llist_new();
    if (node->switch_s.values == NULL)
        return false;
    node->switch_s.bodies = Nst_llist_new();
    if (node->switch_s.bodies == NULL) {
        DESTROY_NODE_LLIST(node->switch_s.values);
        return false;
    }
    node->switch_s.default_body = NULL;
    return true;
}

void _Nst_node_switch_s_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->switch_s.expr);
    DESTROY_NODE_LLIST(node->switch_s.values);
    DESTROY_NODE_LLIST(node->switch_s.bodies);
    DESTROY_NODE_IF_NOT_NULL(node->switch_s.default_body);
}

void print_switch_s(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(switch_s, expr), false);
    print_node_list(EP(switch_s, values), false);
    print_node_list(EP(switch_s, bodies), false);
    print_node(EP(switch_s, default_body), true);
}

bool _Nst_node_try_catch_s_init(Nst_Node *node)
{
    node->try_catch_s.try_body = NULL;
    node->try_catch_s.catch_body = NULL;
    node->try_catch_s.error_name = NULL;
}

void _Nst_node_try_catch_s_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->try_catch_s.try_body);
    DESTROY_NODE_IF_NOT_NULL(node->try_catch_s.catch_body);
    DESTROY_TOK_IF_NOT_NULL(node->try_catch_s.error_name);
}

void print_try_catch_s(Nst_Node *node, Nst_LList *levels)
{
    print_token(EP(try_catch_s, error_name), false);
    print_node(EP(try_catch_s, try_body), false);
    print_node(EP(try_catch_s, catch_body), true);
}

bool _Nst_node_s_wrapper_init(Nst_Node *node)
{
    node->s_wrapper.statement = NULL;
    return true;
}

void _Nst_node_s_wrapper_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->s_wrapper.statement);
}

void print_s_wrapper(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(s_wrapper, statement), true);
}

bool _Nst_node_stack_op_init(Nst_Node *node)
{
    node->stack_op.values = Nst_llist_new();
    node->stack_op.op = Nst_TT_INVALID;
    return node->stack_op.values != NULL;
}

void _Nst_node_stack_op_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->stack_op.values);
}

void print_stack_op(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(stack_op, op), false);
    print_node_list(EP(stack_op, values), true);
}

bool _Nst_node_local_stack_op_init(Nst_Node *node)
{
    node->local_stack_op.values = Nst_llist_new();
    node->local_stack_op.special_value = NULL;
    node->local_stack_op.op = Nst_TT_INVALID;
    return node->local_stack_op.values != NULL;
}

void _Nst_node_local_stack_op_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->local_stack_op.values);
    DESTROY_NODE_IF_NOT_NULL(node->local_stack_op.special_value);
}

void print_local_stack_op(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(local_stack_op, op), false);
    print_node(EP(local_stack_op, special_value), false);
    print_node_list(EP(local_stack_op, values), true);
}

bool _Nst_node_local_op_init(Nst_Node *node)
{
    node->local_op.value = NULL;
    node->local_op.op = Nst_TT_INVALID;
    return true;
}

void _Nst_node_local_op_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->local_op.value);
}

void print_local_op(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(local_op, op), false);
    print_node(EP(local_op, value), true);
}

bool _Nst_node_seq_lit_init(Nst_Node *node)
{
    node->seq_lit.values = Nst_llist_new();
    node->seq_lit.type = Nst_SNT_NOT_SET;
    return node->seq_lit.values != NULL;
}

void _Nst_node_seq_lit_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->seq_lit.values);
}

void print_seq_lit(Nst_Node *node, Nst_LList *levels)
{
    print_seq_node_type(EP(seq_lit, type), false);
    print_node_list(EP(seq_lit, values), true);
}

bool _Nst_node_map_lit_init(Nst_Node *node)
{
    node->map_lit.keys = Nst_llist_new();
    if (node->map_lit.keys == NULL)
        return false;
    node->map_lit.values = Nst_llist_new();
    if (node->map_lit.values == NULL) {
        DESTROY_NODE_LLIST(node->map_lit.keys);
        return false;
    }
    return true;
}

void _Nst_node_map_lit_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->map_lit.keys);
    DESTROY_NODE_LLIST(node->map_lit.values);
}

void print_map_lit(Nst_Node *node, Nst_LList *levels)
{
    print_node_list(EP(map_lit, keys), false);
    print_node_list(EP(map_lit, values), true);
}

bool _Nst_node_value_init(Nst_Node *node)
{
    node->value.value = NULL;
    return true;
}

void _Nst_node_value_destroy(Nst_Node *node)
{
    DESTROY_TOK_IF_NOT_NULL(node->value.value);
}

void print_value(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(value, value), true);
}

bool _Nst_node_access_init(Nst_Node *node)
{
    node->access.value = NULL;
    return true;
}

void _Nst_node_access_destroy(Nst_Node *node)
{
    DESTROY_TOK_IF_NOT_NULL(node->value.value);
}

void print_access(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(access, value), true);
}

bool _Nst_node_extract_e_init(Nst_Node *node)
{
    node->extract_e.container = NULL;
    node->extract_e.key = NULL;
    return true;
}

void _Nst_node_extract_e_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->extract_e.container);
    DESTROY_NODE_IF_NOT_NULL(node->extract_e.key);
}

void print_extract_e(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(extract_e, container), false);
    print_node(EP(extract_e, key), true);
}

bool _Nst_node_assign_e_init(Nst_Node *node)
{
    node->assign_e.value = NULL;
    node->assign_e.name = NULL;
    return true;
}

void _Nst_node_assign_e_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->assign_e.value);
    DESTROY_NODE_IF_NOT_NULL(node->assign_e.name);
}

void print_assign_e(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(assign_e, name), false);
    print_node(EP(assign_e, value), true);
}

bool _Nst_node_comp_assign_e_init(Nst_Node *node)
{
    node->comp_assign_e.values = Nst_llist_new();
    if (node->comp_assign_e.values == NULL)
        return false;
    node->comp_assign_e.name = NULL;
    node->comp_assign_e.op = Nst_TT_INVALID;
    return true;
}

void _Nst_node_comp_assign_e_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->comp_assign_e.values);
    DESTROY_NODE_IF_NOT_NULL(node->comp_assign_e.name);
}

void print_comp_assign_e(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(comp_assign_e, name), false);
    print_node_list(EP(comp_assign_e, values), true);
}

bool _Nst_node_if_e_init(Nst_Node *node)
{
    node->if_e.condition = NULL;
    node->if_e.body_if_true = NULL;
    node->if_e.body_if_false = NULL;
    return true;
}

void _Nst_node_if_e_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->if_e.condition);
    DESTROY_NODE_IF_NOT_NULL(node->if_e.body_if_true);
    DESTROY_NODE_IF_NOT_NULL(node->if_e.body_if_false);
}

void print_if_e(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(if_e, condition), false);
    print_node(EP(if_e, body_if_true), false);
    print_node(EP(if_e, body_if_false), true);
}

bool _Nst_node_e_wrapper_init(Nst_Node *node)
{
    node->e_wrapper.expr = NULL;
    return true;
}

void _Nst_node_e_wrapper_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->e_wrapper.expr);
}

void print_e_wrapper(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(e_wrapper, expr), true);
}

Nst_Node *Nst_node_new(Nst_NodeType type)
{
   Nst_Node *node = Nst_malloc_c(1, Nst_Node);
   node->start = Nst_no_pos();
   node->end = Nst_no_pos();
   node->type = type;
   if (initializers[type] != NULL && !initializers[type](node)) {
       Nst_free(node);
       return NULL;
   }
   return node;
}

Nst_Node *Nst_node_set_pos(Nst_Node *node, Nst_Pos start, Nst_Pos end)
{
    node->start = start;
    node->end = end;
}

Nst_Node *Nst_node_destroy(Nst_Node *node)
{
    if (destructors[node->type] != NULL)
        destructors[node->type](node);
    Nst_free(node);
}

void Nst_print_node(Nst_Node *node)
{
    // no error handling is done while printing, if an error occurs it is
    // ignored, but if an error has occurred before the function was called
    // we risk clearing a valid error
    if (Nst_error_occurred())
        return;

    Nst_LList *levels = Nst_llist_new();
    if (levels == NULL) {
        Nst_error_clear();
        return;
    }

    print_node(NULL, node, levels, -1);
    Nst_llist_destroy(levels, NULL);
    Nst_error_clear();
}

const i8 *Nst_node_type_to_str(Nst_NodeType nt)
{
    return nt_strings[nt];
}
