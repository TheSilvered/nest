#include "nest.h"

// extract property, used to avoid mistakes when printing node data
#define EP(data_name, name) #name, node->v.data_name.name, levels

static void print_s_list(Nst_Node *node, Nst_LList *levels);
static void print_s_while_lp(Nst_Node *node, Nst_LList *levels);
static void print_s_for_lp(Nst_Node *node, Nst_LList *levels);
static void print_s_fn_decl(Nst_Node *node, Nst_LList *levels);
static void print_s_return(Nst_Node *node, Nst_LList *levels);
static void print_s_switch(Nst_Node *node, Nst_LList *levels);
static void print_s_try_catch(Nst_Node *node, Nst_LList *levels);
static void print_s_wrapper(Nst_Node *node, Nst_LList *levels);
static void print_e_stack_op(Nst_Node *node, Nst_LList *levels);
static void print_e_loc_stack_op(Nst_Node *node, Nst_LList *levels);
static void print_e_local_op(Nst_Node *node, Nst_LList *levels);
static void print_e_seq_literal(Nst_Node *node, Nst_LList *levels);
static void print_e_map_literal(Nst_Node *node, Nst_LList *levels);
static void print_e_value(Nst_Node *node, Nst_LList *levels);
static void print_e_access(Nst_Node *node, Nst_LList *levels);
static void print_e_extraction(Nst_Node *node, Nst_LList *levels);
static void print_e_assignment(Nst_Node *node, Nst_LList *levels);
static void print_e_comp_assign(Nst_Node *node, Nst_LList *levels);
static void print_e_if(Nst_Node *node, Nst_LList *levels);
static void print_e_wrapper(Nst_Node *node, Nst_LList *levels);

static bool (*initializers[])(Nst_Node *) = {
    [Nst_NT_S_LIST] = _Nst_node_s_list_init,
    [Nst_NT_S_WHILE_LP] = _Nst_node_s_while_lp_init,
    [Nst_NT_S_FOR_LP] = _Nst_node_s_for_lp_init,
    [Nst_NT_S_FN_DECL] = _Nst_node_s_fn_decl_init,
    [Nst_NT_S_RETURN] = _Nst_node_s_return_init,
    [Nst_NT_S_CONTINUE] = NULL,
    [Nst_NT_S_BREAK] = NULL,
    [Nst_NT_S_SWITCH] = _Nst_node_s_switch_init,
    [Nst_NT_S_TRY_CATCH] = _Nst_node_s_try_catch_init,
    [Nst_NT_S_WRAPPER] = _Nst_node_s_wrapper_init,
    [Nst_NT_S_NOP] = NULL,
    [Nst_NT_E_STACK_OP] = _Nst_node_e_stack_op_init,
    [Nst_NT_E_LOC_STACK_OP] = _Nst_node_e_loc_stack_op_init,
    [Nst_NT_E_LOCAL_OP] = _Nst_node_e_local_op_init,
    [Nst_NT_E_SEQ_LITERAL] = _Nst_node_e_seq_literal_init,
    [Nst_NT_E_MAP_LITERAL] = _Nst_node_e_map_literal_init,
    [Nst_NT_E_VALUE] = _Nst_node_e_value_init,
    [Nst_NT_E_ACCESS] = _Nst_node_e_access_init,
    [Nst_NT_E_EXTRACTION] = _Nst_node_e_extraction_init,
    [Nst_NT_E_ASSIGNMENT] = _Nst_node_e_assignment_init,
    [Nst_NT_E_COMP_ASSIGN] = _Nst_node_e_comp_assign_init,
    [Nst_NT_E_IF] = _Nst_node_e_if_init,
    [Nst_NT_E_WRAPPER] = _Nst_node_e_wrapper_init,
};

static void (*destructors[])(Nst_Node *) = {
    [Nst_NT_S_LIST] = _Nst_node_s_list_destroy,
    [Nst_NT_S_WHILE_LP] = _Nst_node_s_while_lp_destroy,
    [Nst_NT_S_FOR_LP] = _Nst_node_s_for_lp_destroy,
    [Nst_NT_S_FN_DECL] = _Nst_node_s_fn_decl_destroy,
    [Nst_NT_S_RETURN] = _Nst_node_s_return_destroy,
    [Nst_NT_S_CONTINUE] = NULL,
    [Nst_NT_S_BREAK] = NULL,
    [Nst_NT_S_SWITCH] = _Nst_node_s_switch_destroy,
    [Nst_NT_S_TRY_CATCH] = _Nst_node_s_try_catch_destroy,
    [Nst_NT_S_WRAPPER] = _Nst_node_s_wrapper_destroy,
    [Nst_NT_S_NOP] = NULL,
    [Nst_NT_E_STACK_OP] = _Nst_node_e_stack_op_destroy,
    [Nst_NT_E_LOC_STACK_OP] = _Nst_node_e_loc_stack_op_destroy,
    [Nst_NT_E_LOCAL_OP] = _Nst_node_e_local_op_destroy,
    [Nst_NT_E_SEQ_LITERAL] = _Nst_node_e_seq_literal_destroy,
    [Nst_NT_E_MAP_LITERAL] = _Nst_node_e_map_literal_destroy,
    [Nst_NT_E_VALUE] = _Nst_node_e_value_destroy,
    [Nst_NT_E_ACCESS] = _Nst_node_e_access_destroy,
    [Nst_NT_E_EXTRACTION] = _Nst_node_e_extraction_destroy,
    [Nst_NT_E_ASSIGNMENT] = _Nst_node_e_assignment_destroy,
    [Nst_NT_E_COMP_ASSIGN] = _Nst_node_e_comp_assign_destroy,
    [Nst_NT_E_IF] = _Nst_node_e_if_destroy,
    [Nst_NT_E_WRAPPER] = _Nst_node_e_wrapper_destroy
};

static void (*prints[])(Nst_Node *, Nst_LList *) = {
    [Nst_NT_S_LIST] = print_s_list,
    [Nst_NT_S_WHILE_LP] = print_s_while_lp,
    [Nst_NT_S_FOR_LP] = print_s_for_lp,
    [Nst_NT_S_FN_DECL] = print_s_fn_decl,
    [Nst_NT_S_RETURN] = print_s_return,
    [Nst_NT_S_CONTINUE] = NULL,
    [Nst_NT_S_BREAK] = NULL,
    [Nst_NT_S_SWITCH] = print_s_switch,
    [Nst_NT_S_TRY_CATCH] = print_s_try_catch,
    [Nst_NT_S_WRAPPER] = print_s_wrapper,
    [Nst_NT_S_NOP] = NULL,
    [Nst_NT_E_STACK_OP] = print_e_stack_op,
    [Nst_NT_E_LOC_STACK_OP] = print_e_loc_stack_op,
    [Nst_NT_E_LOCAL_OP] = print_e_local_op,
    [Nst_NT_E_SEQ_LITERAL] = print_e_seq_literal,
    [Nst_NT_E_MAP_LITERAL] = print_e_map_literal,
    [Nst_NT_E_VALUE] = print_e_value,
    [Nst_NT_E_ACCESS] = print_e_access,
    [Nst_NT_E_EXTRACTION] = print_e_extraction,
    [Nst_NT_E_ASSIGNMENT] = print_e_assignment,
    [Nst_NT_E_COMP_ASSIGN] = print_e_comp_assign,
    [Nst_NT_E_IF] = print_e_if,
    [Nst_NT_E_WRAPPER] = print_e_wrapper
};

static const char *nt_strings[] = {
    [Nst_NT_S_LIST] = "Statement list",
    [Nst_NT_S_WHILE_LP] = "While loop",
    [Nst_NT_S_FOR_LP] = "For loop",
    [Nst_NT_S_FN_DECL] = "Function declaration",
    [Nst_NT_S_RETURN] = "Return statement",
    [Nst_NT_S_CONTINUE] = "Continue statement",
    [Nst_NT_S_BREAK] = "Break statement",
    [Nst_NT_S_SWITCH] = "Switch statement",
    [Nst_NT_S_TRY_CATCH] = "Try-catch statement",
    [Nst_NT_S_WRAPPER] = "Statement wrapper",
    [Nst_NT_S_NOP] = "No-op statement",
    [Nst_NT_E_STACK_OP] = "Stack operation",
    [Nst_NT_E_LOC_STACK_OP] = "Local-stack operation",
    [Nst_NT_E_LOCAL_OP] = "Local operation",
    [Nst_NT_E_SEQ_LITERAL] = "Sequence literal",
    [Nst_NT_E_MAP_LITERAL] = "Map literal",
    [Nst_NT_E_VALUE] = "Value",
    [Nst_NT_E_ACCESS] = "Access",
    [Nst_NT_E_EXTRACTION] = "Extract expression",
    [Nst_NT_E_ASSIGNMENT] = "Assignment expression",
    [Nst_NT_E_COMP_ASSIGN] = "Compound assignment",
    [Nst_NT_E_IF] = "If expression",
    [Nst_NT_E_WRAPPER] = "Expression wrapper"
};

static const char *snt_strings[] = {
    [Nst_SNT_ARRAY] = "ARRAY",
    [Nst_SNT_ARRAY_REP] = "ARRAY_REP",
    [Nst_SNT_VECTOR] = "VECTOR",
    [Nst_SNT_VECTOR_REP] = "VECTOR_REP",
    [Nst_SNT_ASSIGNMENT_NAMES] = "ASSIGNMENT_NAMES",
    [Nst_SNT_NOT_SET] = "NOT_SET"
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

static bool print_connection(bool last, bool add_level, Nst_LList *levels)
{
    if (last) {
        Nst_print("\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80");
        if (add_level && !Nst_llist_append(levels, NULL, false))
            return false;
    } else {
        Nst_print("\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80");
        if (add_level && !Nst_llist_append(levels, (void *)1, false))
            return false;
    }
    return true;
}

static void remove_level(Nst_LList *levels)
{
    if (levels->len == 0)
        return;
    if (levels->len == 1) {
        Nst_llist_pop(levels);
        return;
    }
    Nst_LLNode *prev = levels->head;
    for (Nst_LLNode *lnode = prev->next; lnode->next != NULL;) {
        prev = lnode;
        lnode = lnode->next;
    }
    Nst_free(levels->tail);
    prev->next = NULL;
    levels->tail = prev;
    levels->len--;
}

static void print_node(const char *name, Nst_Node *node, Nst_LList *levels,
                       bool last)
{
    if (levels->len != 0) {
        print_levels(levels);
        if (!print_connection(last, true, levels))
            return;
    }

    if (name != NULL)
        Nst_printf("%s: ", name);

    if (node != NULL) {
        Nst_printf(
            "%s (%" PRIi32 ":%" PRIi32 ", %" PRIi32 ":%" PRIi32 ")\n",
            nt_strings[node->type],
            node->span.start_line, node->span.start_col,
            node->span.end_line, node->span.end_col);
        if (prints[node->type] != NULL)
            prints[node->type](node, levels);
    } else
        Nst_print("(null)\n");

    remove_level(levels);
}

static void print_node_list(const char *name, Nst_PtrArray nodes,
                            Nst_LList *levels, bool last)
{
    print_levels(levels);
    if (!print_connection(last, true, levels))
        return;

    if (nodes.len == 0) {
        Nst_printf("%s: (empty)\n", name);
        remove_level(levels);
        return;
    }

    Nst_printf("%s:\n", name);

    for (usize i = 0; i < nodes.len; i++) {
        print_node(
            NULL,
            (Nst_Node *)Nst_pa_get(&nodes, i),
            levels,
            i == nodes.len - 1);
    }
    remove_level(levels);
}

static void print_bool(const char *name, bool value, Nst_LList *levels,
                       bool last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;
    Nst_printf("%s: %s\n", name, value ? "true" : "false");
}

static void print_obj(const char *name, Nst_Obj *obj, Nst_LList *levels,
                      bool last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;
    if (name != NULL)
        Nst_printf("%s: ", name);
    if (obj != NULL) {
        Nst_Obj *repr_obj = Nst_obj_to_repr_str(obj);
        if (repr_obj != NULL) {
            Nst_fwrite(Nst_str_value(repr_obj), Nst_str_len(repr_obj), NULL,
                       Nst_io.out);
            Nst_dec_ref(repr_obj);
        }
    } else
        Nst_print("(null)");
    Nst_print("\n");
}

static void print_tok_type(const char *name, Nst_TokType tok_type,
                           Nst_LList *levels, bool last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;

    Nst_printf("%s: %s\n", name, Nst_tok_type_to_str(tok_type));
}

static void print_obj_list(const char *name, Nst_PtrArray objs,
                           Nst_LList *levels, bool last)
{
    print_levels(levels);
    if (!print_connection(last, true, levels))
        return;

    if (objs.len == 0) {
        Nst_printf("%s: (empty)\n", name);
        remove_level(levels);
        return;
    }

    Nst_printf("%s:\n", name);

    for (usize i = 0; i < objs.len; i++) {
        print_obj(
            NULL,
            NstOBJ(Nst_pa_get(&objs, i)),
            levels,
            i == objs.len - 1);
    }
    remove_level(levels);
}

static void print_seq_node_type(const char *name, Nst_SeqNodeType snt,
                                Nst_LList *levels, bool last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;

    Nst_printf("%s: %s\n", name, snt_strings[snt]);
}

bool _Nst_node_s_list_init(Nst_Node *node)
{
    return Nst_pa_init(&node->v.s_list.statements, 4);
}

void _Nst_node_s_list_destroy(Nst_Node *node)
{
    Nst_pa_clear(&node->v.s_list.statements, (Nst_Destructor)Nst_node_destroy);
}

void print_s_list(Nst_Node *node, Nst_LList *levels)
{
    print_node_list(EP(s_list, statements), true);
}

bool _Nst_node_s_while_lp_init(Nst_Node *node)
{
    node->v.s_while_lp.condition = NULL;
    node->v.s_while_lp.body = NULL;
    node->v.s_while_lp.is_dowhile = false;
    return true;
}

void _Nst_node_s_while_lp_destroy(Nst_Node *node)
{
    if (node->v.s_while_lp.condition != NULL)
        Nst_node_destroy(node->v.s_while_lp.condition);
    if (node->v.s_while_lp.body != NULL)
        Nst_node_destroy(node->v.s_while_lp.body);
}

void print_s_while_lp(Nst_Node *node, Nst_LList *levels)
{
    print_bool(EP(s_while_lp, is_dowhile), false);
    print_node(EP(s_while_lp, condition), false);
    print_node(EP(s_while_lp, body), true);
}

bool _Nst_node_s_for_lp_init(Nst_Node *node)
{
    node->v.s_for_lp.iterator = NULL;
    node->v.s_for_lp.assignment = NULL;
    node->v.s_for_lp.body = NULL;
    return true;
}

void _Nst_node_s_for_lp_destroy(Nst_Node *node)
{
    if (node->v.s_for_lp.iterator != NULL)
        Nst_node_destroy(node->v.s_for_lp.iterator);
    if (node->v.s_for_lp.assignment != NULL)
        Nst_node_destroy(node->v.s_for_lp.assignment);
    if (node->v.s_for_lp.body != NULL)
        Nst_node_destroy(node->v.s_for_lp.body);
}

void print_s_for_lp(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(s_for_lp, iterator), false);
    print_node(EP(s_for_lp, assignment), false);
    print_node(EP(s_for_lp, body), true);
}

bool _Nst_node_s_fn_decl_init(Nst_Node *node)
{
    node->v.s_fn_decl.name = NULL;
    node->v.s_fn_decl.body = NULL;
    return Nst_pa_init(&node->v.s_fn_decl.argument_names, 2);
}

void _Nst_node_s_fn_decl_destroy(Nst_Node *node)
{
    Nst_ndec_ref(node->v.s_fn_decl.name);
    if (node->v.s_fn_decl.body != NULL)
        Nst_node_destroy(node->v.s_fn_decl.body);
    Nst_pa_clear(&node->v.s_fn_decl.argument_names, (Nst_Destructor)Nst_dec_ref);
}

void print_s_fn_decl(Nst_Node *node, Nst_LList *levels)
{
    print_obj(EP(s_fn_decl, name), false);
    print_obj_list(EP(s_fn_decl, argument_names), false);
    print_node(EP(s_fn_decl, body), true);
}

bool _Nst_node_s_return_init(Nst_Node *node)
{
    node->v.s_return.value = NULL;
    return true;
}

void _Nst_node_s_return_destroy(Nst_Node *node)
{
    if (node->v.s_return.value != NULL)
        Nst_node_destroy(node->v.s_return.value);
}

void print_s_return(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(s_return, value), true);
}

bool _Nst_node_s_switch_init(Nst_Node *node)
{
    node->v.s_switch.expr = NULL;
    Nst_pa_init(&node->v.s_switch.values, 0);
    Nst_pa_init(&node->v.s_switch.bodies, 0);
    node->v.s_switch.default_body = NULL;
    return true;
}

void _Nst_node_s_switch_destroy(Nst_Node *node)
{
    if (node->v.s_switch.expr != NULL)
        Nst_node_destroy(node->v.s_switch.expr);
    Nst_pa_clear(&node->v.s_switch.values, (Nst_Destructor)Nst_node_destroy);
    Nst_pa_clear(&node->v.s_switch.bodies, (Nst_Destructor)Nst_node_destroy);
    if (node->v.s_switch.default_body != NULL)
        Nst_node_destroy(node->v.s_switch.default_body);
}

void print_s_switch(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(s_switch, expr), false);
    print_node_list(EP(s_switch, values), false);
    print_node_list(EP(s_switch, bodies), false);
    print_node(EP(s_switch, default_body), true);
}

bool _Nst_node_s_try_catch_init(Nst_Node *node)
{
    node->v.s_try_catch.try_body = NULL;
    node->v.s_try_catch.catch_body = NULL;
    node->v.s_try_catch.error_name = NULL;
    return true;
}

void _Nst_node_s_try_catch_destroy(Nst_Node *node)
{
    if (node->v.s_try_catch.try_body != NULL)
        Nst_node_destroy(node->v.s_try_catch.try_body);
    if (node->v.s_try_catch.catch_body != NULL)
        Nst_node_destroy(node->v.s_try_catch.catch_body);
    Nst_ndec_ref(node->v.s_try_catch.error_name);
}

void print_s_try_catch(Nst_Node *node, Nst_LList *levels)
{
    print_obj(EP(s_try_catch, error_name), false);
    print_node(EP(s_try_catch, try_body), false);
    print_node(EP(s_try_catch, catch_body), true);
}

bool _Nst_node_s_wrapper_init(Nst_Node *node)
{
    node->v.s_wrapper.statement = NULL;
    return true;
}

void _Nst_node_s_wrapper_destroy(Nst_Node *node)
{
    if (node->v.s_wrapper.statement != NULL)
        Nst_node_destroy(node->v.s_wrapper.statement);
}

void print_s_wrapper(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(s_wrapper, statement), true);
}

bool _Nst_node_e_stack_op_init(Nst_Node *node)
{
    node->v.e_stack_op.op = Nst_TT_INVALID;
    return Nst_pa_init(&node->v.e_stack_op.values, 2);
}

void _Nst_node_e_stack_op_destroy(Nst_Node *node)
{
    Nst_pa_clear(&node->v.e_stack_op.values, (Nst_Destructor)Nst_node_destroy);
}

void print_e_stack_op(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(e_stack_op, op), false);
    print_node_list(EP(e_stack_op, values), true);
}

bool _Nst_node_e_loc_stack_op_init(Nst_Node *node)
{
    node->v.e_loc_stack_op.special_value = NULL;
    node->v.e_loc_stack_op.op = Nst_TT_INVALID;
    return Nst_pa_init(&node->v.e_loc_stack_op.values, 3);
}

void _Nst_node_e_loc_stack_op_destroy(Nst_Node *node)
{
    Nst_pa_clear(&node->v.e_loc_stack_op.values, (Nst_Destructor)Nst_node_destroy);
    if (node->v.e_loc_stack_op.special_value != NULL)
        Nst_node_destroy(node->v.e_loc_stack_op.special_value);
}

void print_e_loc_stack_op(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(e_loc_stack_op, op), false);
    print_node(EP(e_loc_stack_op, special_value), false);
    print_node_list(EP(e_loc_stack_op, values), true);
}

bool _Nst_node_e_local_op_init(Nst_Node *node)
{
    node->v.e_local_op.value = NULL;
    node->v.e_local_op.op = Nst_TT_INVALID;
    return true;
}

void _Nst_node_e_local_op_destroy(Nst_Node *node)
{
    if (node->v.e_local_op.value != NULL)
        Nst_node_destroy(node->v.e_local_op.value);
}

void print_e_local_op(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(e_local_op, op), false);
    print_node(EP(e_local_op, value), true);
}

bool _Nst_node_e_seq_literal_init(Nst_Node *node)
{
    Nst_pa_init(&node->v.e_seq_literal.values, 0);
    node->v.e_seq_literal.type = Nst_SNT_NOT_SET;
    return true;
}

void _Nst_node_e_seq_literal_destroy(Nst_Node *node)
{
    Nst_pa_clear(&node->v.e_seq_literal.values, (Nst_Destructor)Nst_node_destroy);
}

void print_e_seq_literal(Nst_Node *node, Nst_LList *levels)
{
    print_seq_node_type(EP(e_seq_literal, type), false);
    print_node_list(EP(e_seq_literal, values), true);
}

bool _Nst_node_e_map_literal_init(Nst_Node *node)
{
    Nst_pa_init(&node->v.e_map_literal.keys, 0);
    Nst_pa_init(&node->v.e_map_literal.values, 0);
    return true;
}

void _Nst_node_e_map_literal_destroy(Nst_Node *node)
{
    Nst_pa_clear(&node->v.e_map_literal.keys, (Nst_Destructor)Nst_node_destroy);
    Nst_pa_clear(&node->v.e_map_literal.values, (Nst_Destructor)Nst_node_destroy);
}

void print_e_map_literal(Nst_Node *node, Nst_LList *levels)
{
    print_node_list(EP(e_map_literal, keys), false);
    print_node_list(EP(e_map_literal, values), true);
}

bool _Nst_node_e_value_init(Nst_Node *node)
{
    node->v.e_value.value = NULL;
    return true;
}

void _Nst_node_e_value_destroy(Nst_Node *node)
{
    Nst_ndec_ref(node->v.e_value.value);
}

void print_e_value(Nst_Node *node, Nst_LList *levels)
{
    print_obj(EP(e_value, value), true);
}

bool _Nst_node_e_access_init(Nst_Node *node)
{
    node->v.e_access.value = NULL;
    return true;
}

void _Nst_node_e_access_destroy(Nst_Node *node)
{
    Nst_ndec_ref(node->v.e_access.value);
}

void print_e_access(Nst_Node *node, Nst_LList *levels)
{
    print_obj(EP(e_access, value), true);
}

bool _Nst_node_e_extraction_init(Nst_Node *node)
{
    node->v.e_extraction.container = NULL;
    node->v.e_extraction.key = NULL;
    return true;
}

void _Nst_node_e_extraction_destroy(Nst_Node *node)
{
    if (node->v.e_extraction.container != NULL)
        Nst_node_destroy(node->v.e_extraction.container);
    if (node->v.e_extraction.key != NULL)
        Nst_node_destroy(node->v.e_extraction.key);
}

void print_e_extraction(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(e_extraction, container), false);
    print_node(EP(e_extraction, key), true);
}

bool _Nst_node_e_assignment_init(Nst_Node *node)
{
    node->v.e_assignment.value = NULL;
    node->v.e_assignment.name = NULL;
    return true;
}

void _Nst_node_e_assignment_destroy(Nst_Node *node)
{
    if (node->v.e_assignment.value != NULL)
        Nst_node_destroy(node->v.e_assignment.value);
    if (node->v.e_assignment.name != NULL)
        Nst_node_destroy(node->v.e_assignment.name);
}

void print_e_assignment(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(e_assignment, name), false);
    print_node(EP(e_assignment, value), true);
}

bool _Nst_node_e_comp_assign_init(Nst_Node *node)
{
    node->v.e_comp_assignment.name = NULL;
    node->v.e_comp_assignment.op = Nst_TT_INVALID;
    return Nst_pa_init(&node->v.e_comp_assignment.values, 2);
}

void _Nst_node_e_comp_assign_destroy(Nst_Node *node)
{
    Nst_pa_clear(&node->v.e_comp_assignment.values, (Nst_Destructor)Nst_node_destroy);
    if (node->v.e_comp_assignment.name != NULL)
        Nst_node_destroy(node->v.e_comp_assignment.name);
}

void print_e_comp_assign(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(e_comp_assignment, op), false);
    print_node(EP(e_comp_assignment, name), false);
    print_node_list(EP(e_comp_assignment, values), true);
}

bool _Nst_node_e_if_init(Nst_Node *node)
{
    node->v.e_if.condition = NULL;
    node->v.e_if.body_if_true = NULL;
    node->v.e_if.body_if_false = NULL;
    return true;
}

void _Nst_node_e_if_destroy(Nst_Node *node)
{
    if (node->v.e_if.condition != NULL)
        Nst_node_destroy(node->v.e_if.condition);
    if (node->v.e_if.body_if_true != NULL)
        Nst_node_destroy(node->v.e_if.body_if_true);
    if (node->v.e_if.body_if_false != NULL)
        Nst_node_destroy(node->v.e_if.body_if_false);
}

void print_e_if(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(e_if, condition), false);
    print_node(EP(e_if, body_if_true), false);
    print_node(EP(e_if, body_if_false), true);
}

bool _Nst_node_e_wrapper_init(Nst_Node *node)
{
    node->v.e_wrapper.expr = NULL;
    return true;
}

void _Nst_node_e_wrapper_destroy(Nst_Node *node)
{
    if (node->v.e_wrapper.expr != NULL)
        Nst_node_destroy(node->v.e_wrapper.expr);
}

void print_e_wrapper(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(e_wrapper, expr), true);
}

Nst_Node *Nst_node_new(Nst_NodeType type)
{
   Nst_Node *node = Nst_malloc_c(1, Nst_Node);
   node->span = Nst_span_empty();
   node->type = type;
   if (initializers[type] != NULL && !initializers[type](node)) {
       Nst_free(node);
       return NULL;
   }
   return node;
}

void Nst_node_set_span(Nst_Node *node, Nst_Span span)
{
    node->span = span;
}

void Nst_node_destroy(Nst_Node *node)
{
    if (destructors[node->type] != NULL)
        destructors[node->type](node);
    Nst_free(node);
}

void Nst_node_destroy_contents(Nst_Node *node)
{
    if (destructors[node->type] != NULL)
        destructors[node->type](node);
}

void Nst_print_node(Nst_Node *node)
{
    // no error handling is done while printing, if an error occurs it is
    // ignored, but if an error has occurred before the function was called
    // we risk clearing a valid error
    if (Nst_error_occurred())
        return;

    Nst_LList levels;
    Nst_llist_init(&levels);

    print_node(NULL, node, &levels, true);
    Nst_llist_empty(&levels, NULL);
    Nst_error_clear();
}

const char *Nst_node_type_to_str(Nst_NodeType nt)
{
    return nt_strings[nt];
}

bool Nst_node_change_type(Nst_Node *node, Nst_NodeType new_type)
{
    Nst_Node backup = *node;

    node->type = new_type;
    if (initializers[new_type] != NULL && !initializers[new_type](node)) {
        *node = backup;
        return false;
    }

    if (destructors[backup.type] != NULL)
        destructors[backup.type](&backup);
    return true;
}
