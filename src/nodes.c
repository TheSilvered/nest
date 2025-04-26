#include "nest.h"

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

static bool (*initializers[])(Nst_Node *) = {
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

static void (*destructors[])(Nst_Node *) = {
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

static void (*prints[])(Nst_Node *, Nst_LList *) = {
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

static const char *nt_strings[] = {
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

static const char *snt_strings[] = {
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

static void print_node_list(const char *name, Nst_DynArray nodes,
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
            Nst_NODE(Nst_da_get_p(&nodes, i)),
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

static void print_obj_list(const char *name, Nst_DynArray objs,
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
            NstOBJ(Nst_da_get_p(&objs, i)),
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

bool _Nst_node_cs_init(Nst_Node *node)
{
    return Nst_da_init(&node->v.cs.statements, sizeof(Nst_Node *), 4);
}

void _Nst_node_cs_destroy(Nst_Node *node)
{
    Nst_da_clear_p(&node->v.cs.statements, (Nst_Destructor)Nst_node_destroy);
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
    if (node->v.wl.condition != NULL)
        Nst_node_destroy(node->v.wl.condition);
    if (node->v.wl.body != NULL)
        Nst_node_destroy(node->v.wl.body);
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
    if (node->v.fl.iterator != NULL)
        Nst_node_destroy(node->v.fl.iterator);
    if (node->v.fl.assignment != NULL)
        Nst_node_destroy(node->v.fl.assignment);
    if (node->v.fl.body != NULL)
        Nst_node_destroy(node->v.fl.body);
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
    return Nst_da_init(&node->v.fd.argument_names, sizeof(Nst_Obj *), 2);
}

void _Nst_node_fd_destroy(Nst_Node *node)
{
    Nst_ndec_ref(node->v.fd.name);
    if (node->v.fd.body != NULL)
        Nst_node_destroy(node->v.fd.body);
    Nst_da_clear_p(&node->v.fd.argument_names, (Nst_Destructor)Nst_dec_ref);
}

void print_fd(Nst_Node *node, Nst_LList *levels)
{
    print_obj(EP(fd, name), false);
    print_obj_list(EP(fd, argument_names), false);
    print_node(EP(fd, body), true);
}

bool _Nst_node_rt_init(Nst_Node *node)
{
    node->v.rt.value = NULL;
    return true;
}

void _Nst_node_rt_destroy(Nst_Node *node)
{
    if (node->v.rt.value != NULL)
        Nst_node_destroy(node->v.rt.value);
}

void print_rt(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(rt, value), true);
}

bool _Nst_node_sw_init(Nst_Node *node)
{
    node->v.sw.expr = NULL;
    Nst_da_init(&node->v.sw.values, sizeof(Nst_Node *), 0);
    Nst_da_init(&node->v.sw.bodies, sizeof(Nst_Node *), 0);
    node->v.sw.default_body = NULL;
    return true;
}

void _Nst_node_sw_destroy(Nst_Node *node)
{
    if (node->v.sw.expr != NULL)
        Nst_node_destroy(node->v.sw.expr);
    Nst_da_clear_p(&node->v.sw.values, (Nst_Destructor)Nst_node_destroy);
    Nst_da_clear_p(&node->v.sw.bodies, (Nst_Destructor)Nst_node_destroy);
    if (node->v.sw.default_body != NULL)
        Nst_node_destroy(node->v.sw.default_body);
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
    if (node->v.tc.try_body != NULL)
        Nst_node_destroy(node->v.tc.try_body);
    if (node->v.tc.catch_body != NULL)
        Nst_node_destroy(node->v.tc.catch_body);
    Nst_ndec_ref(node->v.tc.error_name);
}

void print_tc(Nst_Node *node, Nst_LList *levels)
{
    print_obj(EP(tc, error_name), false);
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
    if (node->v.ws.statement != NULL)
        Nst_node_destroy(node->v.ws.statement);
}

void print_ws(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(ws, statement), true);
}

bool _Nst_node_so_init(Nst_Node *node)
{
    node->v.so.op = Nst_TT_INVALID;
    return Nst_da_init(&node->v.so.values, sizeof(Nst_Node *), 2);
}

void _Nst_node_so_destroy(Nst_Node *node)
{
    Nst_da_clear_p(&node->v.so.values, (Nst_Destructor)Nst_node_destroy);
}

void print_so(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(so, op), false);
    print_node_list(EP(so, values), true);
}

bool _Nst_node_ls_init(Nst_Node *node)
{
    node->v.ls.special_value = NULL;
    node->v.ls.op = Nst_TT_INVALID;
    return Nst_da_init(&node->v.ls.values, sizeof(Nst_Node *), 3);
}

void _Nst_node_ls_destroy(Nst_Node *node)
{
    Nst_da_clear_p(&node->v.ls.values, (Nst_Destructor)Nst_node_destroy);
    if (node->v.ls.special_value != NULL)
        Nst_node_destroy(node->v.ls.special_value);
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
    if (node->v.lo.value != NULL)
        Nst_node_destroy(node->v.lo.value);
}

void print_lo(Nst_Node *node, Nst_LList *levels)
{
    print_tok_type(EP(lo, op), false);
    print_node(EP(lo, value), true);
}

bool _Nst_node_sl_init(Nst_Node *node)
{
    Nst_da_init(&node->v.sl.values, sizeof(Nst_Node *), 0);
    node->v.sl.type = Nst_SNT_NOT_SET;
    return true;
}

void _Nst_node_sl_destroy(Nst_Node *node)
{
    Nst_da_clear_p(&node->v.sl.values, (Nst_Destructor)Nst_node_destroy);
}

void print_sl(Nst_Node *node, Nst_LList *levels)
{
    print_seq_node_type(EP(sl, type), false);
    print_node_list(EP(sl, values), true);
}

bool _Nst_node_ml_init(Nst_Node *node)
{
    Nst_da_init(&node->v.ml.keys, sizeof(Nst_Node *), 0);
    Nst_da_init(&node->v.ml.values, sizeof(Nst_Node *), 0);
    return true;
}

void _Nst_node_ml_destroy(Nst_Node *node)
{
    Nst_da_clear_p(&node->v.ml.keys, (Nst_Destructor)Nst_node_destroy);
    Nst_da_clear_p(&node->v.ml.values, (Nst_Destructor)Nst_node_destroy);
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
    Nst_ndec_ref(node->v.vl.value);
}

void print_vl(Nst_Node *node, Nst_LList *levels)
{
    print_obj(EP(vl, value), true);
}

bool _Nst_node_ac_init(Nst_Node *node)
{
    node->v.ac.value = NULL;
    return true;
}

void _Nst_node_ac_destroy(Nst_Node *node)
{
    Nst_ndec_ref(node->v.ac.value);
}

void print_ac(Nst_Node *node, Nst_LList *levels)
{
    print_obj(EP(ac, value), true);
}

bool _Nst_node_ex_init(Nst_Node *node)
{
    node->v.ex.container = NULL;
    node->v.ex.key = NULL;
    return true;
}

void _Nst_node_ex_destroy(Nst_Node *node)
{
    if (node->v.ex.container != NULL)
        Nst_node_destroy(node->v.ex.container);
    if (node->v.ex.key != NULL)
        Nst_node_destroy(node->v.ex.key);
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
    if (node->v.as.value != NULL)
        Nst_node_destroy(node->v.as.value);
    if (node->v.as.name != NULL)
        Nst_node_destroy(node->v.as.name);
}

void print_as(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(as, name), false);
    print_node(EP(as, value), true);
}

bool _Nst_node_ca_init(Nst_Node *node)
{
    node->v.ca.name = NULL;
    node->v.ca.op = Nst_TT_INVALID;
    return Nst_da_init(&node->v.ca.values, sizeof(Nst_Node *), 2);
}

void _Nst_node_ca_destroy(Nst_Node *node)
{
    Nst_da_clear_p(&node->v.ca.values, (Nst_Destructor)Nst_node_destroy);
    if (node->v.ca.name != NULL)
        Nst_node_destroy(node->v.ca.name);
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
    if (node->v.ie.condition != NULL)
        Nst_node_destroy(node->v.ie.condition);
    if (node->v.ie.body_if_true != NULL)
        Nst_node_destroy(node->v.ie.body_if_true);
    if (node->v.ie.body_if_false != NULL)
        Nst_node_destroy(node->v.ie.body_if_false);
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
    if (node->v.we.expr != NULL)
        Nst_node_destroy(node->v.we.expr);
}

void print_we(Nst_Node *node, Nst_LList *levels)
{
    print_node(EP(we, expr), true);
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
