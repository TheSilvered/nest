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
#define EP(data_name, name) #name, node->v.data_name.name, levels

static void print_cs(Nst_Node *node, Nst_LList *levels);
static void print_wl(Nst_Node *node, Nst_LList *levels);
static void print_fl(Nst_Node *node, Nst_LList *levels);
static void print_fd(Nst_Node *node, Nst_LList *levels);
static void print_rt(Nst_Node *node, Nst_LList *levels);
static void print_sw(Nst_Node *node, Nst_LList *levels);
static void print_tc(Nst_Node *node, Nst_LList *levels);
static void print_ws(Nst_Node *node, Nst_LList *levels);
static void print_so(Nst_Node *node, Nst_LList *levels);
static void print_ls(Nst_Node *node, Nst_LList *levels);
static void print_lo(Nst_Node *node, Nst_LList *levels);
static void print_sl(Nst_Node *node, Nst_LList *levels);
static void print_ml(Nst_Node *node, Nst_LList *levels);
static void print_vl(Nst_Node *node, Nst_LList *levels);
static void print_ac(Nst_Node *node, Nst_LList *levels);
static void print_ex(Nst_Node *node, Nst_LList *levels);
static void print_as(Nst_Node *node, Nst_LList *levels);
static void print_ca(Nst_Node *node, Nst_LList *levels);
static void print_ie(Nst_Node *node, Nst_LList *levels);
static void print_we(Nst_Node *node, Nst_LList *levels);

bool (*initializers[])(Nst_Node *) = {
    [Nst_NT_CS] = _Nst_node_cs_init,
    [Nst_NT_WL] = _Nst_node_wl_init,
    [Nst_NT_FL] = _Nst_node_fl_init,
    [Nst_NT_FD] = _Nst_node_fd_init,
    [Nst_NT_RT] = _Nst_node_rt_init,
    [Nst_NT_CN] = NULL,
    [Nst_NT_BR] = NULL,
    [Nst_NT_SW] = _Nst_node_sw_init,
    [Nst_NT_TC] = _Nst_node_tc_init,
    [Nst_NT_WS] = _Nst_node_ws_init,
    [Nst_NT_NP] = NULL,
    [Nst_NT_SO] = _Nst_node_so_init,
    [Nst_NT_LS] = _Nst_node_ls_init,
    [Nst_NT_LO] = _Nst_node_lo_init,
    [Nst_NT_SL] = _Nst_node_sl_init,
    [Nst_NT_ML] = _Nst_node_ml_init,
    [Nst_NT_VL] = _Nst_node_vl_init,
    [Nst_NT_AC] = _Nst_node_ac_init,
    [Nst_NT_EX] = _Nst_node_ex_init,
    [Nst_NT_AS] = _Nst_node_as_init,
    [Nst_NT_CA] = _Nst_node_ca_init,
    [Nst_NT_IE] = _Nst_node_ie_init,
    [Nst_NT_WE] = _Nst_node_we_init,
};

void (*destructors[])(Nst_Node *) = {
    [Nst_NT_CS] = _Nst_node_cs_destroy,
    [Nst_NT_WL] = _Nst_node_wl_destroy,
    [Nst_NT_FL] = _Nst_node_fl_destroy,
    [Nst_NT_FD] = _Nst_node_fd_destroy,
    [Nst_NT_RT] = _Nst_node_rt_destroy,
    [Nst_NT_CN] = NULL,
    [Nst_NT_BR] = NULL,
    [Nst_NT_SW] = _Nst_node_sw_destroy,
    [Nst_NT_TC] = _Nst_node_tc_destroy,
    [Nst_NT_WS] = _Nst_node_ws_destroy,
    [Nst_NT_NP] = NULL,
    [Nst_NT_SO] = _Nst_node_so_destroy,
    [Nst_NT_LS] = _Nst_node_ls_destroy,
    [Nst_NT_LO] = _Nst_node_lo_destroy,
    [Nst_NT_SL] = _Nst_node_sl_destroy,
    [Nst_NT_ML] = _Nst_node_ml_destroy,
    [Nst_NT_VL] = _Nst_node_vl_destroy,
    [Nst_NT_AC] = _Nst_node_ac_destroy,
    [Nst_NT_EX] = _Nst_node_ex_destroy,
    [Nst_NT_AS] = _Nst_node_as_destroy,
    [Nst_NT_CA] = _Nst_node_ca_destroy,
    [Nst_NT_IE] = _Nst_node_ie_destroy,
    [Nst_NT_WE] = _Nst_node_we_destroy
};

void (*prints[])(Nst_Node *, Nst_LList *) = {
    [Nst_NT_CS] = print_cs,
    [Nst_NT_WL] = print_wl,
    [Nst_NT_FL] = print_fl,
    [Nst_NT_FD] = print_fd,
    [Nst_NT_RT] = print_rt,
    [Nst_NT_CN] = NULL,
    [Nst_NT_BR] = NULL,
    [Nst_NT_SW] = print_sw,
    [Nst_NT_TC] = print_tc,
    [Nst_NT_WS] = print_ws,
    [Nst_NT_NP] = NULL,
    [Nst_NT_SO] = print_so,
    [Nst_NT_LS] = print_ls,
    [Nst_NT_LO] = print_lo,
    [Nst_NT_SL] = print_sl,
    [Nst_NT_ML] = print_ml,
    [Nst_NT_VL] = print_vl,
    [Nst_NT_AC] = print_ac,
    [Nst_NT_EX] = print_ex,
    [Nst_NT_AS] = print_as,
    [Nst_NT_CA] = print_ca,
    [Nst_NT_IE] = print_ie,
    [Nst_NT_WE] = print_we
};

const i8 *nt_strings[] = {
    [Nst_NT_CS] = "CS (Compound statement)",
    [Nst_NT_WL] = "WL (While loop)",
    [Nst_NT_FL] = "FL (For loop)",
    [Nst_NT_FD] = "FD (Function declaration)",
    [Nst_NT_RT] = "RT (Return statement)",
    [Nst_NT_CN] = "CN (Continue statement)",
    [Nst_NT_BR] = "BR (Break statement)",
    [Nst_NT_SW] = "SW (Switch statement)",
    [Nst_NT_TC] = "TC (Try-catch statement)",
    [Nst_NT_WS] = "WS (Statement wrapper)",
    [Nst_NT_NP] = "NP (No-op statement)",
    [Nst_NT_SO] = "SO (Stack operation)",
    [Nst_NT_LS] = "LS (Local-stack operation)",
    [Nst_NT_LO] = "LO (Local operation)",
    [Nst_NT_SL] = "SL (Sequence literal)",
    [Nst_NT_ML] = "ML (Map literal)",
    [Nst_NT_VL] = "VL (Value)",
    [Nst_NT_AC] = "AC (Access)",
    [Nst_NT_EX] = "EX (Extract expression)",
    [Nst_NT_AS] = "AS (Assignment expression)",
    [Nst_NT_CA] = "CA (Compound assignment)",
    [Nst_NT_IE] = "IE (If expression)",
    [Nst_NT_WE] = "WE (Expression wrapper)"
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
        if (prints[node->type] != NULL)
            prints[node->type](node, levels);
    } else
        Nst_print("(null)\n");

    remove_level(levels);
}

static void print_node_list(const i8 *name, Nst_LList *nodes,
                            Nst_LList *levels, i8 last)
{
    print_levels(levels);
    if (!print_connection(last, true, levels))
        return;

    if (nodes->len == 0) {
        Nst_printf("%s: (empty)\n", name);
        remove_level(levels);
        return;
    }

    Nst_printf("%s:\n", name);

    for (Nst_LLIST_ITER(node, nodes)) {
        print_node(NULL, Nst_NODE(node->value), levels, node == nodes->tail);
    }
    remove_level(levels);
}

static void print_bool(const i8 *name, bool value, Nst_LList *levels, i8 last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;
    Nst_printf("%s: %s\n", name, value ? "true" : "false");
}

static void print_token(const i8 *name, Nst_Tok *tok, Nst_LList *levels,
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

    if (tokens->len == 0) {
        Nst_printf("%s: (empty)\n", name);
        remove_level(levels);
        return;
    }

    Nst_printf("%s:\n", name);

    for (Nst_LLIST_ITER(token, tokens)) {
        print_token(
            NULL,
            Nst_TOK(token->value),
            levels,
            token == tokens->tail);
    }
    remove_level(levels);
}

static void print_seq_node_type(const i8 *name, Nst_SeqNodeType snt,
                                Nst_LList *levels, i8 last)
{
    print_levels(levels);
    if (!print_connection(last, false, NULL))
        return;

    Nst_printf("%s: %s\n", name, snt_strings[snt]);
}

bool _Nst_node_cs_init(Nst_Node *node)
{
    node->v.cs.statements = Nst_llist_new();
    return node->v.cs.statements != NULL;
}

void _Nst_node_cs_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->v.cs.statements);
}

void print_cs(Nst_Node *node, Nst_LList *levels)
{
    print_node_list(EP(cs, statements), true);
}

bool _Nst_node_wl_init(Nst_Node *node)
{
    node->v.wl.condition = NULL;
    node->v.wl.body = NULL;
    node->v.wl.is_dowhile = false;
    return true;
}

void _Nst_node_wl_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.wl.condition);
    DESTROY_NODE_IF_NOT_NULL(node->v.wl.body);
}

void print_wl(Nst_Node *node, Nst_LList *levels)
{
    print_bool(EP(wl, is_dowhile), false);
    print_node(EP(wl, condition), false);
    print_node(EP(wl, body), true);
}

bool _Nst_node_fl_init(Nst_Node *node)
{
    node->v.fl.iterator = NULL;
    node->v.fl.assignment = NULL;
    node->v.fl.body = NULL;
    return true;
}

void _Nst_node_fl_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.fl.iterator);
    DESTROY_NODE_IF_NOT_NULL(node->v.fl.assignment);
    DESTROY_NODE_IF_NOT_NULL(node->v.fl.body);
}

void print_fl(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(fl, iterator), false);
    print_node(EP(fl, assignment), false);
    print_node(EP(fl, body), true);
}

bool _Nst_node_fd_init(Nst_Node *node)
{
    node->v.fd.name = NULL;
    node->v.fd.body = NULL;
    node->v.fd.argument_names = Nst_llist_new();
    return node->v.fd.argument_names != NULL;
}

void _Nst_node_fd_destroy(Nst_Node *node)
{
    DESTROY_TOK_IF_NOT_NULL(node->v.fd.name);
    DESTROY_NODE_IF_NOT_NULL(node->v.fd.body);
    DESTROY_TOK_LLIST(node->v.fd.argument_names);
}

void print_fd(Nst_Node *node, Nst_LList *levels)
{
    print_token(EP(fd, name), false);
    print_tok_list(EP(fd, argument_names), false);
    print_node(EP(fd, body), true);
}

bool _Nst_node_rt_init(Nst_Node *node)
{
    node->v.rt.value = NULL;
    return true;
}

void _Nst_node_rt_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.rt.value);
}

void print_rt(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(rt, value), true);
}

bool _Nst_node_sw_init(Nst_Node *node)
{
    node->v.sw.expr = NULL;
    node->v.sw.values = Nst_llist_new();
    if (node->v.sw.values == NULL)
        return false;
    node->v.sw.bodies = Nst_llist_new();
    if (node->v.sw.bodies == NULL) {
        DESTROY_NODE_LLIST(node->v.sw.values);
        return false;
    }
    node->v.sw.default_body = NULL;
    return true;
}

void _Nst_node_sw_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.sw.expr);
    DESTROY_NODE_LLIST(node->v.sw.values);
    DESTROY_NODE_LLIST(node->v.sw.bodies);
    DESTROY_NODE_IF_NOT_NULL(node->v.sw.default_body);
}

void print_sw(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(sw, expr), false);
    print_node_list(EP(sw, values), false);
    print_node_list(EP(sw, bodies), false);
    print_node(EP(sw, default_body), true);
}

bool _Nst_node_tc_init(Nst_Node *node)
{
    node->v.tc.try_body = NULL;
    node->v.tc.catch_body = NULL;
    node->v.tc.error_name = NULL;
    return true;
}

void _Nst_node_tc_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.tc.try_body);
    DESTROY_NODE_IF_NOT_NULL(node->v.tc.catch_body);
    DESTROY_TOK_IF_NOT_NULL(node->v.tc.error_name);
}

void print_tc(Nst_Node *node, Nst_LList *levels)
{
    print_token(EP(tc, error_name), false);
    print_node(EP(tc, try_body), false);
    print_node(EP(tc, catch_body), true);
}

bool _Nst_node_ws_init(Nst_Node *node)
{
    node->v.ws.statement = NULL;
    return true;
}

void _Nst_node_ws_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.ws.statement);
}

void print_ws(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(ws, statement), true);
}

bool _Nst_node_so_init(Nst_Node *node)
{
    node->v.so.values = Nst_llist_new();
    node->v.so.op = Nst_TT_INVALID;
    return node->v.so.values != NULL;
}

void _Nst_node_so_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->v.so.values);
}

void print_so(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(so, op), false);
    print_node_list(EP(so, values), true);
}

bool _Nst_node_ls_init(Nst_Node *node)
{
    node->v.ls.values = Nst_llist_new();
    node->v.ls.special_value = NULL;
    node->v.ls.op = Nst_TT_INVALID;
    return node->v.ls.values != NULL;
}

void _Nst_node_ls_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->v.ls.values);
    DESTROY_NODE_IF_NOT_NULL(node->v.ls.special_value);
}

void print_ls(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(ls, op), false);
    print_node(EP(ls, special_value), false);
    print_node_list(EP(ls, values), true);
}

bool _Nst_node_lo_init(Nst_Node *node)
{
    node->v.lo.value = NULL;
    node->v.lo.op = Nst_TT_INVALID;
    return true;
}

void _Nst_node_lo_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.lo.value);
}

void print_lo(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(lo, op), false);
    print_node(EP(lo, value), true);
}

bool _Nst_node_sl_init(Nst_Node *node)
{
    node->v.sl.values = Nst_llist_new();
    node->v.sl.type = Nst_SNT_NOT_SET;
    return node->v.sl.values != NULL;
}

void _Nst_node_sl_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->v.sl.values);
}

void print_sl(Nst_Node *node, Nst_LList *levels)
{
    print_seq_node_type(EP(sl, type), false);
    print_node_list(EP(sl, values), true);
}

bool _Nst_node_ml_init(Nst_Node *node)
{
    node->v.ml.keys = Nst_llist_new();
    if (node->v.ml.keys == NULL)
        return false;
    node->v.ml.values = Nst_llist_new();
    if (node->v.ml.values == NULL) {
        DESTROY_NODE_LLIST(node->v.ml.keys);
        return false;
    }
    return true;
}

void _Nst_node_ml_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->v.ml.keys);
    DESTROY_NODE_LLIST(node->v.ml.values);
}

void print_ml(Nst_Node *node, Nst_LList *levels)
{
    print_node_list(EP(ml, keys), false);
    print_node_list(EP(ml, values), true);
}

bool _Nst_node_vl_init(Nst_Node *node)
{
    node->v.vl.value = NULL;
    return true;
}

void _Nst_node_vl_destroy(Nst_Node *node)
{
    DESTROY_TOK_IF_NOT_NULL(node->v.vl.value);
}

void print_vl(Nst_Node *node, Nst_LList *levels)
{
    print_token(EP(vl, value), true);
}

bool _Nst_node_ac_init(Nst_Node *node)
{
    node->v.ac.value = NULL;
    return true;
}

void _Nst_node_ac_destroy(Nst_Node *node)
{
    DESTROY_TOK_IF_NOT_NULL(node->v.ac.value);
}

void print_ac(Nst_Node *node, Nst_LList *levels)
{
    print_token(EP(ac, value), true);
}

bool _Nst_node_ex_init(Nst_Node *node)
{
    node->v.ex.container = NULL;
    node->v.ex.key = NULL;
    return true;
}

void _Nst_node_ex_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.ex.container);
    DESTROY_NODE_IF_NOT_NULL(node->v.ex.key);
}

void print_ex(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(ex, container), false);
    print_node(EP(ex, key), true);
}

bool _Nst_node_as_init(Nst_Node *node)
{
    node->v.as.value = NULL;
    node->v.as.name = NULL;
    return true;
}

void _Nst_node_as_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.as.value);
    DESTROY_NODE_IF_NOT_NULL(node->v.as.name);
}

void print_as(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(as, name), false);
    print_node(EP(as, value), true);
}

bool _Nst_node_ca_init(Nst_Node *node)
{
    node->v.ca.values = Nst_llist_new();
    if (node->v.ca.values == NULL)
        return false;
    node->v.ca.name = NULL;
    node->v.ca.op = Nst_TT_INVALID;
    return true;
}

void _Nst_node_ca_destroy(Nst_Node *node)
{
    DESTROY_NODE_LLIST(node->v.ca.values);
    DESTROY_NODE_IF_NOT_NULL(node->v.ca.name);
}

void print_ca(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(ca, op), false);
    print_node(EP(ca, name), false);
    print_node_list(EP(ca, values), true);
}

bool _Nst_node_ie_init(Nst_Node *node)
{
    node->v.ie.condition = NULL;
    node->v.ie.body_if_true = NULL;
    node->v.ie.body_if_false = NULL;
    return true;
}

void _Nst_node_ie_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.ie.condition);
    DESTROY_NODE_IF_NOT_NULL(node->v.ie.body_if_true);
    DESTROY_NODE_IF_NOT_NULL(node->v.ie.body_if_false);
}

void print_ie(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(ie, condition), false);
    print_node(EP(ie, body_if_true), false);
    print_node(EP(ie, body_if_false), true);
}

bool _Nst_node_we_init(Nst_Node *node)
{
    node->v.we.expr = NULL;
    return true;
}

void _Nst_node_we_destroy(Nst_Node *node)
{
    DESTROY_NODE_IF_NOT_NULL(node->v.we.expr);
}

void print_we(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(we, expr), true);
}

Nst_Node *Nst_node_new(Nst_NodeType type)
{
   Nst_Node *node = Nst_malloc_c(1, Nst_Node);
   node->start = Nst_pos_empty();
   node->end = Nst_pos_empty();
   node->type = type;
   if (initializers[type] != NULL && !initializers[type](node)) {
       Nst_free(node);
       return NULL;
   }
   return node;
}

void Nst_node_set_pos(Nst_Node *node, Nst_Pos start, Nst_Pos end)
{
    node->start = start;
    node->end = end;
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
