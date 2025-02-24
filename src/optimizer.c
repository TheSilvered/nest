#include <errno.h>
#include <string.h>
#include "mem.h"
#include "optimizer.h"
#include "obj_ops.h"
#include "tokens.h"
#include "iter.h"
#include "hash.h"
#include "format.h"
#include "obj_ops.h"

static bool optimize_node(Nst_Node *node);

static bool optimize_cs(Nst_Node *node);
static bool optimize_while_l(Nst_Node *node);
static bool optimize_fl(Nst_Node *node);
static bool optimize_func_declr(Nst_Node *node);
static bool optimize_rt(Nst_Node *node);
static bool optimize_switch_s(Nst_Node *node);
static bool optimize_try_catch_s(Nst_Node *node);
static bool optimize_s_wrapper(Nst_Node *node);
static bool optimize_stack_op(Nst_Node *node);
static bool optimize_local_stack_op(Nst_Node *node);
static bool optimize_local_op(Nst_Node *node);
static bool optimize_seq_lit(Nst_Node *node);
static bool optimize_map_lit(Nst_Node *node);
static bool optimize_extract_e(Nst_Node *node);
static bool optimize_assign_e(Nst_Node *node);
static bool optimize_comp_assign_e(Nst_Node *node);
static bool optimize_if_e(Nst_Node *node);
static bool optimize_e_wrapper(Nst_Node *node);

// Gets the value of the node if it is a Nst_NT_VALUE node and returns NULL
// otherwise
static Nst_Obj *get_value(Nst_Node *node);

bool (*optimizers[])(Nst_Node *) = {
    [Nst_NT_CS] = optimize_cs,
    [Nst_NT_WL] = optimize_while_l,
    [Nst_NT_FL] = optimize_fl,
    [Nst_NT_FD] = optimize_func_declr,
    [Nst_NT_RT] = optimize_rt,
    [Nst_NT_CN] = NULL,
    [Nst_NT_BR] = NULL,
    [Nst_NT_SW] = optimize_switch_s,
    [Nst_NT_TC] = optimize_try_catch_s,
    [Nst_NT_WS] = optimize_s_wrapper,
    [Nst_NT_NP] = NULL,
    [Nst_NT_SO] = optimize_stack_op,
    [Nst_NT_LS] = optimize_local_stack_op,
    [Nst_NT_LO] = optimize_local_op,
    [Nst_NT_SL] = optimize_seq_lit,
    [Nst_NT_ML] = optimize_map_lit,
    [Nst_NT_VL] = NULL,
    [Nst_NT_AC] = NULL,
    [Nst_NT_EX] = optimize_extract_e,
    [Nst_NT_AS] = optimize_assign_e,
    [Nst_NT_CA] = optimize_comp_assign_e,
    [Nst_NT_IE] = optimize_if_e,
    [Nst_NT_WE] = optimize_e_wrapper
};

Nst_Node *NstC Nst_optimize_ast(Nst_Node *ast)
{
    if (!optimize_node(ast)) {
        // should never happen but if it does, it will not crash
        if (Nst_error_get()->positions->len == 0)
            Nst_error_add_pos(ast->start, ast->end);
        Nst_node_destroy(ast);
        return NULL;
    }

    return ast;
}

static Nst_Obj *get_value(Nst_Node *node)
{
    if (node->type != Nst_NT_VL)
        return NULL;
    else
        return node->v.vl.value->value;
}

static void move_into(Nst_Node *new_node, Nst_Node *old_node)
{
    Nst_node_destroy_contents(old_node);
    *old_node = *new_node;
    Nst_free(new_node);
}

static bool optimize_node(Nst_Node *node)
{
    if (node == NULL)
        return true;
    if (optimizers[node->type] != NULL)
        return optimizers[node->type](node);
    return true;
}

static bool optimize_cs(Nst_Node *node)
{
    for (Nst_LLIST_ITER(lnode, node->v.cs.statements)) {
        if (!optimize_node(Nst_NODE(lnode->value)))
            return false;
    }
    return true;
}

static bool optimize_while_l(Nst_Node *node)
{
    if (!optimize_node(node->v.wl.condition))
        return false;
    if (!optimize_node(node->v.wl.body))
        return false;

    Nst_Obj *cond_value = get_value(node->v.wl.condition);

    if (cond_value == NULL || Nst_obj_to_bool(cond_value))
        return true;

    if (node->v.wl.is_dowhile) {
        Nst_Node *body = node->v.wl.body;
        node->v.wl.body = NULL;
        move_into(body, node);
        return true;
    }
    Nst_node_change_type(node, Nst_NT_NP);
    return true;
}

static bool optimize_fl(Nst_Node *node)
{
    if (!optimize_node(node->v.fl.iterator))
        return false;
    if (!optimize_node(node->v.fl.body))
        return false;

    if (node->v.fl.assignment != NULL)
        return true;
    Nst_Obj *repetitions = get_value(node->v.fl.iterator);
    if (repetitions == NULL)
        return true;
    if (repetitions->type == Nst_t.Int && Nst_int_i64(repetitions) == 0)
        Nst_node_change_type(node, Nst_NT_NP);
    return true;
}

static bool optimize_func_declr(Nst_Node *node)
{
    return optimize_node(node->v.fd.body);
}

static bool optimize_rt(Nst_Node *node)
{
    return optimize_node(node->v.rt.value);
}

static bool optimize_switch_s(Nst_Node *node)
{
    if (!optimize_node(node->v.sw.expr))
        return false;

    // it cannot be optimized to jump to the correct value if known because of
    // possible fallthrough
    for (Nst_LLIST_ITER(lnode, node->v.sw.values)) {
        if (!optimize_node(Nst_NODE(lnode->value)))
            return false;
    }
    for (Nst_LLIST_ITER(lnode, node->v.sw.bodies)) {
        if (!optimize_node(Nst_NODE(lnode->value)))
            return false;
    }
    return optimize_node(node->v.sw.default_body);
}

static bool optimize_try_catch_s(Nst_Node *node)
{
    if (!optimize_node(node->v.tc.try_body))
        return false;
    if (!optimize_node(node->v.tc.catch_body))
        return false;
    return true;
}

static bool optimize_s_wrapper(Nst_Node *node)
{
    if (!optimize_node(node->v.ws.statement))
        return false;
    Nst_Node *statement = node->v.ws.statement;
    node->v.ws.statement = NULL;
    move_into(statement, node);
    return true;
}

static bool optimize_stack_values(Nst_LList *values, Nst_TokType op,
                                  Nst_Node *node)
{
    for (Nst_LLIST_ITER(lnode, values)) {
        if (!optimize_node(Nst_NODE(lnode->value)))
            return false;
    }

    if (values->len == 1)
        return true;

    Nst_Obj *(*op_func)(Nst_Obj *, Nst_Obj *) = NULL;

    switch (op) {
    case Nst_TT_ADD:      op_func = Nst_obj_add;      break;
    case Nst_TT_SUB:      op_func = Nst_obj_sub;      break;
    case Nst_TT_MUL:      op_func = Nst_obj_mul;      break;
    case Nst_TT_DIV:      op_func = Nst_obj_div;      break;
    case Nst_TT_POW:      op_func = Nst_obj_pow;      break;
    case Nst_TT_MOD:      op_func = Nst_obj_mod;      break;
    case Nst_TT_B_AND:    op_func = Nst_obj_bwand;    break;
    case Nst_TT_B_OR:     op_func = Nst_obj_bwor;     break;
    case Nst_TT_B_XOR:    op_func = Nst_obj_bwxor;    break;
    case Nst_TT_LSHIFT:   op_func = Nst_obj_bwls;     break;
    case Nst_TT_RSHIFT:   op_func = Nst_obj_bwrs;     break;
    case Nst_TT_CONCAT:   op_func = Nst_obj_concat;   break;
    case Nst_TT_L_AND:    op_func = Nst_obj_lgand;    break;
    case Nst_TT_L_OR:     op_func = Nst_obj_lgor;     break;
    case Nst_TT_L_XOR:    op_func = Nst_obj_lgxor;    break;
    case Nst_TT_CONTAINS: op_func = Nst_obj_contains; break;
    default:
        return true;
    }

    Nst_Obj *curr_value = get_value(Nst_NODE(values->head->value));
    if (curr_value == NULL)
        return true;
    Nst_inc_ref(curr_value);
    Nst_LLNode *prev_lnode = Nst_llist_pop_llnode(values);
    Nst_Node *prev_node = Nst_NODE(prev_lnode->value);

    while (values->len != 0) {
        Nst_Obj *top_value = get_value(Nst_NODE(values->head->value));
        if (top_value == NULL) {
            Nst_dec_ref(prev_node->v.vl.value->value);
            prev_node->v.vl.value->value = curr_value;
            Nst_llist_push_llnode(values, prev_lnode);
            return true;
        }
        Nst_free(prev_lnode);
        Nst_node_destroy(prev_node);

        Nst_Obj *result = op_func(curr_value, top_value);
        Nst_dec_ref(curr_value);
        if (result == NULL) {
            Nst_error_add_pos(node->start, node->end);
            return false;
        }
        curr_value = result;
        prev_lnode = Nst_llist_pop_llnode(values);
        prev_node = Nst_NODE(prev_lnode->value);
    }
    Nst_dec_ref(prev_node->v.vl.value->value);
    prev_node->v.vl.value->value = curr_value;
    Nst_llist_push_llnode(values, prev_lnode);
    return true;
}

static bool optimize_stack_op(Nst_Node *node)
{
    if (!optimize_stack_values(node->v.so.values, node->v.so.op, node))
        return false;

    if (node->v.so.values->len == 1) {
        Nst_Node *expr = Nst_llist_pop(node->v.so.values);
        move_into(expr, node);
    }
    return true;
}

static bool optimize_local_stack_op(Nst_Node *node)
{
    for (Nst_LLIST_ITER(lnode, node->v.ls.values)) {
        if (!optimize_node(lnode->value))
            return false;
    }

    return optimize_node(node->v.ls.special_value);
}

static bool optimize_local_op(Nst_Node *node)
{
    if (!optimize_node(node->v.lo.value))
        return false;

    Nst_Obj *value = get_value(node->v.lo.value);
    if (value == NULL)
        return true;
    Nst_Obj *(*op_func)(Nst_Obj *) = NULL;
    switch (node->v.lo.op) {
    case Nst_TT_LEN:    op_func = Nst_obj_len;    break;
    case Nst_TT_L_NOT:  op_func = Nst_obj_lgnot;  break;
    case Nst_TT_B_NOT:  op_func = Nst_obj_bwnot;  break;
    case Nst_TT_NEG:    op_func = Nst_obj_neg;    break;
    case Nst_TT_TYPEOF: op_func = Nst_obj_typeof; break;
    default:
        return true;
    }

    Nst_Obj *result = op_func(value);
    if (result == NULL) {
        Nst_error_add_pos(node->start, node->end);
        return false;
    }
    Nst_Node *value_node = node->v.lo.value;
    node->v.lo.value = NULL;
    Nst_dec_ref(value_node->v.vl.value->value);
    value_node->v.vl.value->value = result;
    move_into(value_node, node);
    return true;
}

static bool optimize_seq_lit(Nst_Node *node)
{
    for (Nst_LLIST_ITER(lnode, node->v.sl.values)) {
        if (!optimize_node(Nst_NODE(lnode->value)))
            return false;
    }
    return true;
}

static bool optimize_map_lit(Nst_Node *node)
{
    for (Nst_LLIST_ITER(lnode, node->v.ml.keys)) {
        if (!optimize_node(Nst_NODE(lnode->value)))
            return false;
    }

    for (Nst_LLIST_ITER(lnode, node->v.ml.values)) {
        if (!optimize_node(Nst_NODE(lnode->value)))
            return false;
    }
    return true;
}

static bool optimize_extract_e(Nst_Node *node)
{
    if (!optimize_node(node->v.ex.key))
        return false;
    if (!optimize_node(node->v.ex.container))
        return false;
    return true;
}

static bool optimize_assign_e(Nst_Node *node)
{
    if (!optimize_node(node->v.as.value))
        return false;
    if (!optimize_node(node->v.as.name))
        return false;
    return true;
}

static bool optimize_comp_assign_e(Nst_Node *node)
{
    if (!optimize_node(node->v.ca.name))
        return false;
    if (!optimize_stack_values(node->v.ca.values, node->v.ca.op, node))
        return false;
    return true;
}

static bool optimize_if_e(Nst_Node *node)
{
    if (!optimize_node(node->v.ie.condition))
        return false;
    if (!optimize_node(node->v.ie.body_if_true))
        return false;
    if (!optimize_node(node->v.ie.body_if_false))
        return false;
    return true;
}

static bool optimize_e_wrapper(Nst_Node *node)
{
    if (!optimize_node(node->v.we.expr))
        return false;
    Nst_Node *statement = node->v.we.expr;
    node->v.we.expr = NULL;
    move_into(statement, node);
    return true;
}

static bool optimize_bytecode(Nst_InstList *bc, bool optimize_builtins);
static bool can_optimize_consts(Nst_InstList *bc);
static bool is_accessed(Nst_InstList *bc, Nst_Obj *name);
static bool has_assignments(Nst_InstList *bc, Nst_Obj *name);
static bool has_jumps_to(Nst_InstList *bc, i64 idx, i64 avoid_start,
                         i64 avoid_end);
static void replace_access(Nst_InstList *bc, Nst_Obj *name, Nst_Obj *val);
static void optimize_const(Nst_InstList *bc, const i8 *name, Nst_Obj *val);
static void remove_push_pop(Nst_InstList *bc);
static void remove_assign_pop(Nst_InstList *bc);
static void remove_assign_loc_get_val(Nst_InstList *bc);
static bool remove_push_check(Nst_InstList *bc);
static void remove_push_jumpif(Nst_InstList *bc);
static void remove_inst(Nst_InstList *bc, i64 idx);
static bool optimize_funcs(Nst_InstList *bc);
static void remove_dead_code(Nst_InstList *bc);
static bool optimize_chained_jumps(Nst_InstList *bc);

Nst_InstList *Nst_optimize_bytecode(Nst_InstList *bc, bool optimize_builtins)
{
    if (!optimize_bytecode(bc, optimize_builtins)) {
        Nst_inst_list_destroy(bc);
        return NULL;
    }
    return bc;
}

static bool optimize_bytecode(Nst_InstList *bc, bool optimize_builtins)
{
    if (optimize_builtins && can_optimize_consts(bc)) {
        optimize_const(bc, "Type",   Nst_t.Type);
        optimize_const(bc, "Int",    Nst_t.Int);
        optimize_const(bc, "Real",   Nst_t.Real);
        optimize_const(bc, "Bool",   Nst_t.Bool);
        optimize_const(bc, "Null",   Nst_t.Null);
        optimize_const(bc, "Str",    Nst_t.Str);
        optimize_const(bc, "Array",  Nst_t.Array);
        optimize_const(bc, "Vector", Nst_t.Vector);
        optimize_const(bc, "Map",    Nst_t.Map);
        optimize_const(bc, "Func",   Nst_t.Func);
        optimize_const(bc, "Iter",   Nst_t.Iter);
        optimize_const(bc, "Byte",   Nst_t.Byte);
        optimize_const(bc, "IOFile", Nst_t.IOFile);
        optimize_const(bc, "true",   Nst_c.Bool_true);
        optimize_const(bc, "false",  Nst_c.Bool_false);
        optimize_const(bc, "null",   Nst_c.Null_null);
    }

    i64 initial_size;
    do {
        initial_size = bc->total_size;
        if (!remove_push_check(bc))
            return false;
        remove_push_pop(bc);
        remove_assign_pop(bc);
        remove_assign_loc_get_val(bc);
        remove_push_jumpif(bc);
        if (!optimize_chained_jumps(bc))
            return false;
        remove_dead_code(bc);
        if (!optimize_funcs(bc))
            return false;

        // remove NO_OP instructions and compact the bytecode
        i64 size = bc->total_size;
        Nst_Inst *inst_list = bc->instructions;
        for (i64 i = 0; i < size; i++) {
            if (inst_list[i].id == Nst_IC_NO_OP) {
                remove_inst(bc, i);
                i--;
                size--;
            }
        }
    } while (initial_size != (i64)bc->total_size);

    return true;
}

static bool can_optimize_consts(Nst_InstList *bc)
{
    // if _vars_, _globals_, *._vars_, *._globals_ are set to other
    // variables explicitly (_vars_ = x) or implicitly (_vars_ @x)
    // predefined constants cannot be optimized
    if (has_assignments(bc, Nst_s.o__vars_)
        || has_assignments(bc, Nst_s.o__globals_))
    {
        return false;
    }

    if (is_accessed(bc, Nst_s.o__vars_)
        || is_accessed(bc, Nst_s.o__globals_))
    {
        return false;
    }

    return true;
}

static bool is_accessed(Nst_InstList *bc, Nst_Obj *name)
{
    Nst_assert(name->type == Nst_t.Str);

    // if the name is followed by any number of LOCAL_OP, TYPE_CHECK,
    // HASH_CHECK and then POP_VAL, JUMPIF_T or JUMPIF_F
    // or is followed by PUSH_VAL and OP_EXTRACT or SET_CONT_VAL
    // it is not counted as an access

    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    for (i64 i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_PUSH_VAL
            && inst_list[i].val->type == Nst_t.Str
            && Nst_str_compare(inst_list[i].val, name) == 0)
        {
            if (inst_list[++i].id == Nst_IC_OP_EXTRACT)
                return true;
            continue;
        }

        if (inst_list[i].id != Nst_IC_GET_VAL
            || Nst_str_compare(inst_list[i].val, name) != 0)
        {
            continue;
        }

        if (inst_list[i + 1].id == Nst_IC_LOCAL_OP
            || inst_list[i + 1].id == Nst_IC_HASH_CHECK)
        {
            i += 2;

            while (i < size
                   && (inst_list[i].id == Nst_IC_LOCAL_OP
                       || inst_list[i].id == Nst_IC_HASH_CHECK))
            {
                i++;
            }

            if (i == size
                || (inst_list[i].id != Nst_IC_POP_VAL
                    && inst_list[i].id != Nst_IC_JUMPIF_T
                    && inst_list[i].id != Nst_IC_JUMPIF_F))
            {
                return true;
            }
        } else if (inst_list[i + 1].id == Nst_IC_PUSH_VAL) {
            i += 2;

            if (i == size
                || (inst_list[i].id != Nst_IC_OP_EXTRACT
                    && inst_list[i].id != Nst_IC_SET_CONT_VAL))
            {
                return true;
            }
            i -= 2;
        } else
            return true;
    }

    for (Nst_LLIST_ITER(n, bc->functions)) {
        if (is_accessed(Nst_func_nest_body(n->value), name))
            return true;
    }

    return false;
}

static bool has_assignments(Nst_InstList *bc, Nst_Obj *name)
{
    Nst_assert(name->type == Nst_t.Str);

    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    for (i64 i = 0; i < size; i++) {
        if (i > 0 && inst_list[i].id == Nst_IC_SET_CONT_VAL) {
            Nst_Inst prev_inst = inst_list[i - 1];
            if (prev_inst.id == Nst_IC_PUSH_VAL
                && prev_inst.val->type == Nst_t.Str
                && Nst_str_compare(name, prev_inst.val) == 0)
            {
                return true;
            }
        }

        if (inst_list[i].id != Nst_IC_SET_VAL
            && inst_list[i].id != Nst_IC_SET_VAL_LOC)
        {
            continue;
        }

        if (Nst_str_compare(name, inst_list[i].val) == 0)
            return true;
    }

    for (Nst_LLIST_ITER(n, bc->functions)) {
        if (has_assignments(Nst_func_nest_body(n->value), name))
            return true;
    }

    return false;
}

static void replace_access(Nst_InstList *bc, Nst_Obj *name, Nst_Obj *val)
{
    Nst_assert(name->type == Nst_t.Str);
    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    for (i64 i = 0; i < size; i++) {
        if (inst_list[i].id != Nst_IC_GET_VAL)
            continue;

        if (Nst_str_compare(name, inst_list[i].val) == 0) {
            inst_list[i].id = Nst_IC_PUSH_VAL;
            Nst_dec_ref(inst_list[i].val);
            inst_list[i].val = Nst_inc_ref(val);
        }
    }

    for (Nst_LLIST_ITER(n, bc->functions))
        replace_access(Nst_func_nest_body(n->value), name, val);
}

static void optimize_const(Nst_InstList *bc, const i8 *name, Nst_Obj *val)
{
    Nst_Obj *str_obj = Nst_str_new_c_raw(name, false);
    if (str_obj == NULL) {
        Nst_error_clear();
        return;
    }

    if (has_assignments(bc, str_obj))
        goto end;

    replace_access(bc, str_obj, val);

end:
    Nst_dec_ref(str_obj);
}

static bool has_jumps_to(Nst_InstList *bc, i64 idx, i64 avoid_start,
                         i64 avoid_end)
{
    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    Nst_Inst inst;

    for (i64 i = 0; i < size; i++) {
        if (i >= avoid_start && i <= avoid_end)
            continue;

        inst = inst_list[i];
        if (Nst_INST_IS_JUMP(inst.id) && inst.int_val == idx)
            return true;
    }

    return false;
}

static void remove_push_pop(Nst_InstList *bc)
{
    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool expect_pop = false;

    for (i64 i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_PUSH_VAL) {
            expect_pop = true;
            continue;
        } else if (!expect_pop
                   || inst_list[i].id != Nst_IC_POP_VAL
                   || has_jumps_to(bc, i, -1, -1))
        {
            expect_pop = false;
            continue;
        }

        inst_list[i].id = Nst_IC_NO_OP;
        inst_list[i - 1].id = Nst_IC_NO_OP;
        Nst_dec_ref(inst_list[i - 1].val);
        inst_list[i - 1].val = NULL;

        expect_pop = false;
    }
}

static void remove_assign_pop(Nst_InstList *bc)
{
    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool expect_pop = false;

    for (i64 i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_SET_VAL
            || inst_list[i].id == Nst_IC_SET_CONT_VAL)
        {
            expect_pop = true;
            continue;
        } else if (!expect_pop
                   || inst_list[i].id != Nst_IC_POP_VAL
                   || has_jumps_to(bc, i, -1, -1))
        {
            expect_pop = false;
            continue;
        }

        inst_list[i].id = Nst_IC_NO_OP;

        if (inst_list[i - 1].id == Nst_IC_SET_VAL)
            inst_list[i - 1].id = Nst_IC_SET_VAL_LOC;
        else
            inst_list[i - 1].id = Nst_IC_SET_CONT_LOC;

        expect_pop = false;
    }
}

static void remove_assign_loc_get_val(Nst_InstList *bc)
{
    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool expect_get_val = false;
    Nst_Obj *expected_name = NULL;

    for (i64 i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_SET_VAL_LOC) {
            expect_get_val = true;
            expected_name = inst_list[i].val;
            continue;
        } else if (!expect_get_val
                   || inst_list[i].id != Nst_IC_GET_VAL
                   || has_jumps_to(bc, i, -1, -1)
                   || Nst_str_compare(expected_name, inst_list[i].val) != 0)
        {
            expect_get_val = false;
            expected_name = NULL;
            continue;
        }

        if (Nst_str_compare(expected_name, inst_list[i].val) != 0) {
            expect_get_val = false;
            expected_name = NULL;
            continue;
        }

        inst_list[i].id = Nst_IC_NO_OP;
        Nst_dec_ref(inst_list[i].val);
        inst_list[i].val = NULL;
        inst_list[i - 1].id = Nst_IC_SET_VAL;

        expect_get_val = false;
        expected_name = NULL;
    }
}

static bool remove_push_check(Nst_InstList *bc)
{
    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool was_push = false;

    for (i64 i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_PUSH_VAL) {
            was_push = true;
            continue;
        } else if (!was_push
                   || inst_list[i].id != Nst_IC_HASH_CHECK
                   || has_jumps_to(bc, i, -1, -1))
        {
            was_push = false;
            continue;
        }

        Nst_Obj *obj = inst_list[i - 1].val;
        Nst_obj_hash(obj);
        if (obj->hash == -1) {
            Nst_error_setf_type(
                "type '%s' is not hashable",
                Nst_type_name(obj->type).value);
            Nst_error_add_pos(inst_list[i].start, inst_list[i].end);
            return false;
        }

        inst_list[i].id = Nst_IC_NO_OP;
        if (inst_list[i].val != NULL)
            Nst_dec_ref(inst_list[i].val);
        inst_list[i].val = NULL;
        was_push = false;
    }
    return true;
}

static void remove_push_jumpif(Nst_InstList *bc)
{
    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool expect_jumpif = false;

    for (i64 i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_PUSH_VAL) {
            expect_jumpif = true;
            continue;
        }
        else if (!expect_jumpif
                 || !Nst_INST_IS_JUMP(inst_list[i].id)
                 || inst_list[i].id == Nst_IC_JUMP
                 || inst_list[i].id == Nst_IC_JUMPIF_ZERO
                 || has_jumps_to(bc, i, -1, -1))
        {
            expect_jumpif = false;
            continue;
        }

        Nst_Obj *cond = Nst_obj_cast(inst_list[i - 1].val, Nst_t.Bool);
        if ((cond == Nst_c.Bool_false && inst_list[i].id == Nst_IC_JUMPIF_F)
            || (cond == Nst_c.Bool_true && inst_list[i].id == Nst_IC_JUMPIF_T))
        {
            inst_list[i].id = Nst_IC_JUMP;
        } else
            inst_list[i].id = Nst_IC_NO_OP;

        Nst_dec_ref(inst_list[i - 1].val);
        inst_list[i - 1].val = NULL;
        inst_list[i - 1].id = Nst_IC_NO_OP;

        Nst_dec_ref(cond);
        expect_jumpif = false;
    }
}

static void remove_inst(Nst_InstList *bc, i64 idx)
{
    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    if (idx < 0 || idx >= size)
        return;

    for (i64 i = idx; i < size - 1; i++)
        inst_list[i] = inst_list[i + 1];

    bc->total_size--;
    size--;

    for (i64 i = 0; i < size; i++) {
        if (Nst_INST_IS_JUMP(inst_list[i].id) && inst_list[i].int_val > idx)
            inst_list[i].int_val--;
    }
}

static bool optimize_funcs(Nst_InstList *bc)
{
    for (Nst_LLIST_ITER(n, bc->functions)) {
        Nst_InstList *func_bc = Nst_func_nest_body(n->value);
        if (!optimize_bytecode(func_bc, false))
            return false;
    }
    return true;
}

static void remove_dead_code(Nst_InstList *bc)
{
    i64 size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    for (i64 i = 0; i < size; i++) {
        if ((inst_list[i].id != Nst_IC_JUMP
             || i + 1 > inst_list[i].int_val)
            && inst_list[i].id != Nst_IC_RETURN_VAL
            && inst_list[i].id != Nst_IC_THROW_ERR)
        {
            continue;
        }

        bool is_jump_useless = inst_list[i].id == Nst_IC_JUMP;
        bool stop_at_save_error = inst_list[i].id == Nst_IC_THROW_ERR;
        i64 end;
        if (inst_list[i].id != Nst_IC_RETURN_VAL)
            end = inst_list[i].int_val;
        else
            end = size;

        for (i64 j = i + 1; j < end; j++) {
            if (has_jumps_to(bc, j, i + 1, end - 1)) {
                is_jump_useless = false;
                // check the removed instructions again because now they could
                // be referenced
                end = j;
                j = i;
                continue;
            }

            if (stop_at_save_error && inst_list[j].id == Nst_IC_SAVE_ERROR) {
                end = j;
                j = i;
                continue;
            }
        }

        if (is_jump_useless)
            inst_list[i].id = Nst_IC_NO_OP;

        // remove dead instructions
        for (i64 j = i + 1; j < end; j++) {
            if (inst_list[j].val != NULL)
                Nst_dec_ref(inst_list[j].val);

            inst_list[j].id = Nst_IC_NO_OP;
            inst_list[j].val = NULL;
        }
    }
}

typedef u8 *bool_arr_t;

static inline bool_arr_t bool_arr_new(usize size)
{
    usize length = size / 8;
    if (size % 8 != 0)
        length++;
    return Nst_calloc_c(length, u8, NULL);
}

static inline bool bool_arr_get(bool_arr_t array, usize idx)
{
    return (bool)(array[idx / 8] & (1 << (idx % 8)));
}

static inline void bool_arr_set(bool_arr_t array, usize idx, bool value)
{
    u8 byte = array[idx / 8];
    if (value)
        array[idx / 8] = byte | (1 << (idx % 8));
    else
        array[idx / 8] = byte & ~(1 << (idx % 8));
}

static inline void bool_arr_fill(bool_arr_t array, usize size, bool value)
{
    usize length = size / 8;
    if (size % 8 != 0)
        length++;
    u8 set_value = value ? 255 : 0;
    memset(array, set_value, length);
}

static bool optimize_chained_jumps(Nst_InstList *bc)
{
    usize size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    bool_arr_t visited_jumps = bool_arr_new(size);
    if (visited_jumps == NULL)
        return false;

    for (usize i = 0; i < size; i++) {
        if (!Nst_INST_IS_JUMP(inst_list[i].id))
            continue;

        bool_arr_fill(visited_jumps, size, false);
        i64 end_jump = inst_list[i].int_val;
        bool_arr_set(visited_jumps, (usize)end_jump, true);

        while (inst_list[end_jump].id == Nst_IC_JUMP) {
            i64 new_end_jump = inst_list[end_jump].int_val;
            if (bool_arr_get(visited_jumps, (usize)new_end_jump))
                break;
            end_jump = new_end_jump;
            bool_arr_set(visited_jumps, (usize)end_jump, true);
        }

        inst_list[i].int_val = end_jump;
    }
    Nst_free(visited_jumps);
    return true;
}
