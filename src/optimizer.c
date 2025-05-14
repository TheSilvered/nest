#include <errno.h>
#include <string.h>
#include "nest.h"

#define GET_NODE(pa, i) ((Nst_Node *)Nst_pa_get((pa), (i)))

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

static bool (*optimizers[])(Nst_Node *) = {
    [Nst_NT_S_LIST] = optimize_cs,
    [Nst_NT_S_WHILE_LP] = optimize_while_l,
    [Nst_NT_S_FOR_LP] = optimize_fl,
    [Nst_NT_S_FN_DECL] = optimize_func_declr,
    [Nst_NT_S_RETURN] = optimize_rt,
    [Nst_NT_S_CONTINUE] = NULL,
    [Nst_NT_S_BREAK] = NULL,
    [Nst_NT_S_SWITCH] = optimize_switch_s,
    [Nst_NT_S_TRY_CATCH] = optimize_try_catch_s,
    [Nst_NT_S_WRAPPER] = optimize_s_wrapper,
    [Nst_NT_S_NOP] = NULL,
    [Nst_NT_E_STACK_OP] = optimize_stack_op,
    [Nst_NT_E_LOC_STACK_OP] = optimize_local_stack_op,
    [Nst_NT_E_LOCAL_OP] = optimize_local_op,
    [Nst_NT_E_SEQ_LITERAL] = optimize_seq_lit,
    [Nst_NT_E_MAP_LITERAL] = optimize_map_lit,
    [Nst_NT_E_VALUE] = NULL,
    [Nst_NT_E_ACCESS] = NULL,
    [Nst_NT_E_EXTRACTION] = optimize_extract_e,
    [Nst_NT_E_ASSIGNMENT] = optimize_assign_e,
    [Nst_NT_E_COMP_ASSIGN] = optimize_comp_assign_e,
    [Nst_NT_E_IF] = optimize_if_e,
    [Nst_NT_E_WRAPPER] = optimize_e_wrapper
};

Nst_Node *NstC Nst_optimize_ast(Nst_Node *ast)
{
    if (!optimize_node(ast)) {
        // should never happen but if it does, it will not crash
        if (Nst_error_get()->positions.len == 0)
            Nst_error_add_span(ast->span);
        Nst_node_destroy(ast);
        return NULL;
    }

    return ast;
}

static Nst_Obj *get_value(Nst_Node *node)
{
    if (node->type != Nst_NT_E_VALUE)
        return NULL;
    else
        return node->v.e_value.value;
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
    for (usize i = 0, n = node->v.s_list.statements.len; i < n; i++) {
        if (!optimize_node(GET_NODE(&node->v.s_list.statements, i)))
            return false;
    }
    return true;
}

static bool optimize_while_l(Nst_Node *node)
{
    if (!optimize_node(node->v.s_while_lp.condition))
        return false;
    if (!optimize_node(node->v.s_while_lp.body))
        return false;

    Nst_Obj *cond_value = get_value(node->v.s_while_lp.condition);

    if (cond_value == NULL || Nst_obj_to_bool(cond_value))
        return true;

    if (node->v.s_while_lp.is_dowhile) {
        Nst_Node *body = node->v.s_while_lp.body;
        node->v.s_while_lp.body = NULL;
        move_into(body, node);
        return true;
    }
    Nst_node_change_type(node, Nst_NT_S_NOP);
    return true;
}

static bool optimize_fl(Nst_Node *node)
{
    if (!optimize_node(node->v.s_for_lp.iterator))
        return false;
    if (!optimize_node(node->v.s_for_lp.body))
        return false;

    if (node->v.s_for_lp.assignment != NULL)
        return true;
    Nst_Obj *repetitions = get_value(node->v.s_for_lp.iterator);
    if (repetitions == NULL)
        return true;
    if (repetitions->type == Nst_t.Int && Nst_int_i64(repetitions) == 0)
        Nst_node_change_type(node, Nst_NT_S_NOP);
    return true;
}

static bool optimize_func_declr(Nst_Node *node)
{
    return optimize_node(node->v.s_fn_decl.body);
}

static bool optimize_rt(Nst_Node *node)
{
    return optimize_node(node->v.s_return.value);
}

static bool optimize_switch_s(Nst_Node *node)
{
    if (!optimize_node(node->v.s_switch.expr))
        return false;

    // it cannot be optimized to jump to the correct value if known because of
    // possible fallthrough
    for (usize i = 0, n = node->v.s_switch.values.len; i < n; i++) {
        if (!optimize_node(GET_NODE(&node->v.s_switch.values, i)))
            return false;
    }
    for (usize i = 0, n = node->v.s_switch.bodies.len; i < n; i++) {
        if (!optimize_node(GET_NODE(&node->v.s_switch.bodies, i)))
            return false;
    }
    return optimize_node(node->v.s_switch.default_body);
}

static bool optimize_try_catch_s(Nst_Node *node)
{
    if (!optimize_node(node->v.s_try_catch.try_body))
        return false;
    if (!optimize_node(node->v.s_try_catch.catch_body))
        return false;
    return true;
}

static bool optimize_s_wrapper(Nst_Node *node)
{
    if (!optimize_node(node->v.s_wrapper.statement))
        return false;
    Nst_Node *statement = node->v.s_wrapper.statement;
    node->v.s_wrapper.statement = NULL;
    move_into(statement, node);
    return true;
}

static bool optimize_stack_values(Nst_PtrArray *values, Nst_TokType op,
                                  Nst_Node *node)
{
    for (usize i = 0, n = values->len; i < n; i++) {
        if (!optimize_node(GET_NODE(values, i)))
            return false;
    }

    if (values->len <= 1)
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

    Nst_Node *first_node = GET_NODE(values, 0);
    Nst_Obj *curr_value = get_value(first_node);
    if (curr_value == NULL)
        return true;
    Nst_inc_ref(curr_value);

    while (values->len != 1) {
        Nst_Obj *new_value = get_value(GET_NODE(values, 1));
        if (new_value == NULL) {
            Nst_dec_ref(first_node->v.e_value.value);
            first_node->v.e_value.value = curr_value;
            return true;
        }
        Nst_pa_remove_shift(values, 0, (Nst_Destructor)Nst_node_destroy);

        first_node = GET_NODE(values, 0);
        Nst_Obj *result = op_func(curr_value, new_value);
        Nst_dec_ref(curr_value);
        if (result == NULL) {
            Nst_error_add_span(node->span);
            return false;
        }
        curr_value = result;
    }
    Nst_dec_ref(first_node->v.e_value.value);
    first_node->v.e_value.value = curr_value;
    return true;
}

static bool optimize_stack_op(Nst_Node *node)
{
    if (!optimize_stack_values(
            &node->v.e_stack_op.values,
            node->v.e_stack_op.op,
            node))
    {
        return false;
    }

    if (node->v.e_stack_op.values.len == 1) {
        Nst_Node *expr = Nst_pa_get(&node->v.e_stack_op.values, 0);
        Nst_pa_clear(&node->v.e_stack_op.values, NULL);
        move_into(expr, node);
    }
    return true;
}

static bool optimize_local_stack_op(Nst_Node *node)
{
    for (usize i = 0, n = node->v.e_loc_stack_op.values.len; i < n; i++) {
        if (!optimize_node(GET_NODE(&node->v.e_loc_stack_op.values, i)))
            return false;
    }

    return optimize_node(node->v.e_loc_stack_op.special_value);
}

static bool optimize_local_op(Nst_Node *node)
{
    if (!optimize_node(node->v.e_local_op.value))
        return false;

    Nst_Obj *value = get_value(node->v.e_local_op.value);
    if (value == NULL)
        return true;
    Nst_Obj *(*op_func)(Nst_Obj *) = NULL;
    switch (node->v.e_local_op.op) {
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
        Nst_error_add_span(node->span);
        return false;
    }
    Nst_Node *value_node = node->v.e_local_op.value;
    node->v.e_local_op.value = NULL;
    Nst_dec_ref(value_node->v.e_value.value);
    value_node->v.e_value.value = result;
    move_into(value_node, node);
    return true;
}

static bool optimize_seq_lit(Nst_Node *node)
{
    for (usize i = 0, n = node->v.e_seq_literal.values.len; i < n; i++) {
        if (!optimize_node(GET_NODE(&node->v.e_seq_literal.values, i)))
            return false;
    }
    return true;
}

static bool optimize_map_lit(Nst_Node *node)
{
    for (usize i = 0, n = node->v.e_map_literal.keys.len; i < n; i++) {
        if (!optimize_node(GET_NODE(&node->v.e_map_literal.keys, i)))
            return false;
    }

    for (usize i = 0, n = node->v.e_map_literal.values.len; i < n; i++) {
        if (!optimize_node(GET_NODE(&node->v.e_map_literal.values, i)))
            return false;
    }
    return true;
}

static bool optimize_extract_e(Nst_Node *node)
{
    if (!optimize_node(node->v.e_extraction.key))
        return false;
    if (!optimize_node(node->v.e_extraction.container))
        return false;
    return true;
}

static bool optimize_assign_e(Nst_Node *node)
{
    if (!optimize_node(node->v.e_assignment.value))
        return false;
    if (!optimize_node(node->v.e_assignment.name))
        return false;
    return true;
}

static bool optimize_comp_assign_e(Nst_Node *node)
{
    if (!optimize_node(node->v.e_comp_assignment.name))
        return false;
    if (!optimize_stack_values(
            &node->v.e_comp_assignment.values,
            node->v.e_comp_assignment.op,
            node))
    {
        return false;
    }
    return true;
}

static bool optimize_if_e(Nst_Node *node)
{
    if (!optimize_node(node->v.e_if.condition))
        return false;
    if (!optimize_node(node->v.e_if.body_if_true))
        return false;
    if (!optimize_node(node->v.e_if.body_if_false))
        return false;
    return true;
}

static bool optimize_e_wrapper(Nst_Node *node)
{
    if (!optimize_node(node->v.e_wrapper.expr))
        return false;
    Nst_Node *statement = node->v.e_wrapper.expr;
    node->v.e_wrapper.expr = NULL;
    move_into(statement, node);
    return true;
}

static void optimize_inst_list(Nst_InstList *ls);
static void replace_builtins(Nst_InstList *ls);
static isize get_val_idx(Nst_InstList *ls, Nst_Obj *val);
static void replace_constant(Nst_InstList *ls, Nst_Obj *name, Nst_Obj *val);
static bool replace_access(Nst_InstList *ls, Nst_Obj *name, usize obj_idx);
static void replace_func_access(Nst_FuncPrototype *func, Nst_Obj *name,
                                Nst_Obj *val);
// check if the value of 'name' can be accessed directly at any point in 'ls',
// it recursively checks also the functions in 'ls'
static bool is_accessed(Nst_InstList *ls, Nst_Obj *name);
// check if 'name' is assigned in 'ls'
static bool has_assignments(Nst_InstList *ls, Nst_Obj *name);
// check if any jump points to 'idx'
static bool has_jumps_to(Nst_InstList *ls, i64 idx, i64 avoid_start,
                         i64 avoid_end);
static bool remove_push_pop(Nst_InstList *ls);
static bool remove_assign_pop(Nst_InstList *ls);
static bool remove_assign_loc_get_val(Nst_InstList *ls);
static bool remove_push_jumpif(Nst_InstList *ls);
static bool remove_dead_code(Nst_InstList *ls);
static bool optimize_chained_jumps(Nst_InstList *ls);

static bool objs_eq(Nst_InstList *ls, usize idx1, usize idx2)
{
    Nst_Obj *obj1 = Nst_ilist_get_inst_obj(ls, idx1);
    Nst_Obj *obj2 = Nst_ilist_get_inst_obj(ls, idx2);

    return Nst_obj_eq_c(obj1, obj2);
}

static Nst_InstCode inst_code(Nst_InstList *ls, usize idx)
{
    return Nst_ilist_get_inst(ls, idx)->code;
}

void Nst_optimize_ilist(Nst_InstList *ls, bool optimize_builtins)
{
    optimize_inst_list(ls);
    if (optimize_builtins)
        replace_builtins(ls);
    optimize_inst_list(ls);
}

static void optimize_inst_list(Nst_InstList *ls)
{
    bool changed = false;
    do {
        changed = remove_push_pop(ls);
        changed = remove_assign_pop(ls) || changed;
        changed = remove_assign_loc_get_val(ls) || changed;
        changed = remove_push_jumpif(ls) || changed;
        changed = optimize_chained_jumps(ls) || changed;
        changed = remove_dead_code(ls) || changed;
    } while (changed);

    for (usize i = 0, n = ls->functions.len; i < n; i++) {
        Nst_FuncPrototype *func = Nst_ilist_get_func(ls, i);
        optimize_inst_list(&func->ilist);
    }
}

static void replace_builtins(Nst_InstList *ls)
{
    // if _vars_, _globals_, *._vars_, *._globals_ are set to other
    // variables explicitly (_vars_ = x) or implicitly (_vars_ @x)
    // predefined constants cannot be optimized
    if (has_assignments(ls, Nst_s.o__vars_)
        || has_assignments(ls, Nst_s.o__globals_)
        || is_accessed(ls, Nst_s.o__vars_)
        || is_accessed(ls, Nst_s.o__globals_))
    {
        return;
    }

    replace_constant(ls, Nst_s.t_Type, Nst_t.Type);
    replace_constant(ls, Nst_s.t_Int, Nst_t.Int);
    replace_constant(ls, Nst_s.t_Real, Nst_t.Real);
    replace_constant(ls, Nst_s.t_Bool, Nst_t.Bool);
    replace_constant(ls, Nst_s.t_Null, Nst_t.Null);
    replace_constant(ls, Nst_s.t_Str, Nst_t.Str);
    replace_constant(ls, Nst_s.t_Array, Nst_t.Array);
    replace_constant(ls, Nst_s.t_Vector, Nst_t.Vector);
    replace_constant(ls, Nst_s.t_Map, Nst_t.Map);
    replace_constant(ls, Nst_s.t_Func, Nst_t.Func);
    replace_constant(ls, Nst_s.t_Iter, Nst_t.Iter);
    replace_constant(ls, Nst_s.t_Byte, Nst_t.Byte);
    replace_constant(ls, Nst_s.t_IOFile, Nst_t.IOFile);
    replace_constant(ls, Nst_s.c_true, Nst_c.Bool_true);
    replace_constant(ls, Nst_s.c_false, Nst_c.Bool_false);
    replace_constant(ls, Nst_s.c_null, Nst_c.Null_null);
}

static isize get_val_idx(Nst_InstList *ls, Nst_Obj *val)
{
    isize idx = -1;

    for (usize i = 0, n = ls->objects.len; i < n; i++) {
        Nst_Obj *obj = Nst_ilist_get_obj(ls, i);
        if (Nst_obj_eq_c(obj, val)) {
            idx = i;
            break;
        }
    }
    if (idx == -1)
        idx = Nst_ilist_add_obj(ls, Nst_inc_ref(val));
    return idx;
}

static void replace_constant(Nst_InstList *ls, Nst_Obj *name, Nst_Obj *val)
{
    if (has_assignments(ls, name))
        return;

    usize prev_len = ls->objects.len;
    isize idx = get_val_idx(ls, val);
    if (idx == -1)
        return;

    bool changed = replace_access(ls, name, (usize)idx);

    // do not add the value if it is never used
    if (!changed && (usize)idx == prev_len)
        Nst_pa_pop(&ls->objects, (Nst_Destructor)Nst_dec_ref);

    for (usize i = 0, n = ls->functions.len; i < n; i++) {
        Nst_FuncPrototype *func = Nst_ilist_get_func(ls, i);
        replace_func_access(func, name, val);
    }
}

static void replace_func_access(Nst_FuncPrototype *func, Nst_Obj *name,
                                Nst_Obj *val)
{
    // if the name is in the arguments don't replace it
    for (usize i = 0, n = func->arg_num; i < n; i++) {
        if (Nst_obj_eq_c(name, func->arg_names[i]))
            return;
    }
    usize prev_len = func->ilist.objects.len;
    isize idx = get_val_idx(&func->ilist, val);
    bool changed = replace_access(&func->ilist, name, (usize)idx);
    // do not add the value if it is never used
    if (!changed && (usize)idx == prev_len)
        Nst_pa_pop(&func->ilist.objects, (Nst_Destructor)Nst_dec_ref);

    for (usize i = 0, n = func->ilist.functions.len; i < n; i++) {
        Nst_FuncPrototype *func_ptype = Nst_ilist_get_func(&func->ilist, i);
        replace_func_access(func_ptype, name, val);
    }
}

static bool replace_access(Nst_InstList *ls, Nst_Obj *name, usize obj_idx)
{
    Nst_assert(name->type == Nst_t.Str);
    usize size = Nst_ilist_len(ls);
    bool ret = false;

    for (usize i = 0; i < size; i++) {
        Nst_Inst *inst = Nst_ilist_get_inst(ls, i);
        if (inst->code != Nst_IC_GET_VAL)
            continue;
        Nst_Obj *inst_val = Nst_ilist_get_inst_obj(ls, i);
        if (Nst_obj_eq_c(name, inst_val)) {
            inst->code = Nst_IC_PUSH_VAL;
            inst->val = obj_idx;
            ret = true;
        }
    }
    return ret;
}

static bool is_accessed(Nst_InstList *ls, Nst_Obj *name)
{
    Nst_assert(name->type == Nst_t.Str);

    // Note that this function is only used to check if _vars_ and _globals_
    // can be accessed during runtime through other names and making constant
    // substitution incorrect. The exclusion rules are strict to ensure the
    // correctness of the optimized program

    // Once a name is found all following LOCAL_OPs are skipped and it is
    // exluded in these conditions:
    // 1. It is followed a POP_VAL, JUMPIF_T or JUMPIF_F
    // 2. It is followed by another PUSH_VAL (+ LOCAL_OPs) and an OP_EXTRACT
    // 3. It is followed by two PUSH_VALs (+ LOCAL_OPs) and a SET_CONT_VAL

    usize size = Nst_ilist_len(ls);

    for (usize i = 0; i < size; i++) {
        Nst_InstCode code = inst_code(ls, i);
        Nst_Obj *val = Nst_ilist_get_inst_obj(ls, i);

        if (code != Nst_IC_GET_VAL || !Nst_obj_eq_c(val, name))
            continue;

        do {
            if (++i >= size)
                return true;
            code = inst_code(ls, i);
        } while (code == Nst_IC_LOCAL_OP);

        if (code == Nst_IC_POP_VAL || code == Nst_IC_JUMPIF_F
            || code == Nst_IC_JUMPIF_T)
        {
            continue;
        }

        if (code != Nst_IC_PUSH_VAL)
            return true;
        usize push_val_idx = i;

        do {
            if (++i >= size)
                return true;
            code = inst_code(ls, i);
        } while (code == Nst_IC_LOCAL_OP);

        if (code == Nst_IC_OP_EXTRACT) {
            i = push_val_idx - 1;
            continue;
        }

        if (code != Nst_IC_PUSH_VAL)
            return true;

        do {
            if (++i >= size)
                return true;
            code = inst_code(ls, i);
        } while (code == Nst_IC_LOCAL_OP);

        if (code != Nst_IC_SET_CONT_VAL)
            return true;
        i = push_val_idx - 1;
    }

    for (usize i = 0, n = ls->functions.len; i < n; i++) {
        Nst_FuncPrototype *func = Nst_ilist_get_func(ls, i);
        if (is_accessed(&func->ilist, name))
            return true;
    }
    return false;
}

static bool has_assignments(Nst_InstList *ls, Nst_Obj *name)
{
    Nst_assert(name->type == Nst_t.Str);

    usize size = Nst_ilist_len(ls);

    for (usize i = 0; i < size; i++) {
        Nst_InstCode code = inst_code(ls, i);
        if ((code != Nst_IC_SET_VAL && code != Nst_IC_SET_VAL_LOC) || i == 0)
            continue;

        Nst_Obj *obj = Nst_ilist_get_inst_obj(ls, i);
        if (Nst_obj_eq_c(obj, name))
            return true;
    }

    for (usize i = 0, n = ls->functions.len; i < n; i++) {
        Nst_FuncPrototype *func = Nst_ilist_get_func(ls, i);
        if (has_assignments(&func->ilist, name))
            return true;
    }
    return false;
}

static bool has_jumps_to(Nst_InstList *ls, i64 idx, i64 avoid_start,
                         i64 avoid_end)
{
    usize size = Nst_ilist_len(ls);
    isize no_op_count = 0;
    for (i64 i = idx - 1; i > 0; i--) {
        if (i >= avoid_start && i <= avoid_end) {
            no_op_count++;
            continue;
        }
        Nst_Inst *inst = Nst_ilist_get_inst(ls, i);
        if (inst->code == Nst_IC_NO_OP)
            no_op_count++;
        else
            break;
    }

    for (i64 i = 0; (usize)i < size; i++) {
        if (i >= avoid_start && i <= avoid_end)
            continue;

        Nst_Inst *inst = Nst_ilist_get_inst(ls, i);
        if (!Nst_ic_is_jump(inst->code))
            continue;
        if (inst->val >= idx - no_op_count && inst->val <= idx)
            return true;
    }

    return false;
}

static bool remove_push_pop(Nst_InstList *ls)
{
    usize size = Nst_ilist_len(ls);
    bool expect_pop = false;
    usize push_idx = 0;
    bool ret = false;

    for (usize i = 0; i < size; i++) {
        switch (inst_code(ls, i)) {
        case Nst_IC_PUSH_VAL:
            expect_pop = true;
            push_idx = i;
            break;
        case Nst_IC_NO_OP:
            break;
        case Nst_IC_POP_VAL:
            if (expect_pop && !has_jumps_to(ls, i, -1, -1)) {
                Nst_ilist_set(ls, push_idx, Nst_IC_NO_OP);
                Nst_ilist_set(ls, i, Nst_IC_NO_OP);
                push_idx = 0;
                ret = true;
            }
            // fallthrough
        default:
            expect_pop = false;
            break;
        }
    }
    return ret;
}

static bool remove_assign_pop(Nst_InstList *ls)
{
    usize size = Nst_ilist_len(ls);
    bool expect_pop = false;
    bool is_cont_val = false;
    usize assign_idx = 0;
    bool ret = false;

    for (usize i = 0; i < size; i++) {
        switch (inst_code(ls, i)) {
        case Nst_IC_SET_CONT_VAL:
            is_cont_val = true;
            // fallthrough
        case Nst_IC_SET_VAL:
            expect_pop = true;
            assign_idx = i;
            break;
        case Nst_IC_NO_OP:
            break;
        case Nst_IC_POP_VAL:
            if (expect_pop && !has_jumps_to(ls, i, -1, -1)) {
                if (is_cont_val)
                    Nst_ilist_set(ls, assign_idx, Nst_IC_SET_CONT_LOC);
                else {
                    i64 obj_idx = Nst_ilist_get_inst(ls, assign_idx)->val;
                    Nst_ilist_set_ex(
                        ls, assign_idx,
                        Nst_IC_SET_VAL_LOC,
                        obj_idx);
                }
                Nst_ilist_set(ls, i, Nst_IC_NO_OP);
                is_cont_val = false;
                assign_idx = 0;
                ret = true;
            }
            // fallthrough
        default:
            expect_pop = false;
            break;
        }
    }
    return ret;
}

static bool remove_assign_loc_get_val(Nst_InstList *ls)
{
    usize size = Nst_ilist_len(ls);
    bool expect_get_val = false;
    usize set_val_idx = 0;
    bool ret = false;

    for (usize i = 0; i < size; i++) {
        switch (inst_code(ls, i)) {
        case Nst_IC_SET_VAL_LOC:
            expect_get_val = true;
            set_val_idx = i;
            break;
        case Nst_IC_NO_OP:
            break;
        case Nst_IC_GET_VAL:
            if (expect_get_val && objs_eq(ls, set_val_idx, i)
                && !has_jumps_to(ls, i, -1, -1))
            {
                i64 obj_idx = Nst_ilist_get_inst(ls, set_val_idx)->val;
                Nst_ilist_set_ex(ls, set_val_idx, Nst_IC_SET_VAL, obj_idx);
                Nst_ilist_set(ls, i, Nst_IC_NO_OP);
                set_val_idx = 0;
                ret = true;
            }
            // fallthrough
        default:
            expect_get_val = false;
            break;
        }
    }
    return ret;
}

static bool remove_push_jumpif(Nst_InstList *ls)
{
    usize size = Nst_ilist_len(ls);
    usize push_idx = 0;
    bool expect_jumpif = false;
    bool ret = false;

    for (usize i = 0; i < size; i++) {
        switch (inst_code(ls, i)) {
        case Nst_IC_PUSH_VAL:
            push_idx = i;
            expect_jumpif = true;
            break;
        case Nst_IC_NO_OP:
            break;
        case Nst_IC_JUMPIF_F:
            if (!expect_jumpif)
                break;
            if (Nst_obj_to_bool(Nst_ilist_get_inst_obj(ls, push_idx))) {
                Nst_ilist_set(ls, push_idx, Nst_IC_NO_OP);
                Nst_ilist_set(ls, i, Nst_IC_NO_OP);
            } else {
                i64 jump_val = Nst_ilist_get_inst(ls, i)->val;
                Nst_ilist_set(ls, push_idx, Nst_IC_NO_OP);
                Nst_ilist_set_ex(ls, i, Nst_IC_JUMP, jump_val);
            }
            ret = true;
            expect_jumpif = false;
            push_idx = 0;
            break;
        case Nst_IC_JUMPIF_T:
            if (!expect_jumpif)
                break;
            if (Nst_obj_to_bool(Nst_ilist_get_inst_obj(ls, push_idx))) {
                i64 jump_val = Nst_ilist_get_inst(ls, i)->val;
                Nst_ilist_set(ls, push_idx, Nst_IC_NO_OP);
                Nst_ilist_set_ex(ls, i, Nst_IC_JUMP, jump_val);
            } else {
                Nst_ilist_set(ls, push_idx, Nst_IC_NO_OP);
                Nst_ilist_set(ls, i, Nst_IC_NO_OP);
            }
            ret = true;
            // fallthrough
        default:
            expect_jumpif = false;
            push_idx = 0;
            break;
        }
    }
    return ret;
}

static bool remove_dead_code(Nst_InstList *ls)
{
    usize size = Nst_ilist_len(ls);
    bool ret = false;

    for (usize i = 0; i < size; i++) {
        Nst_Inst *inst = Nst_ilist_get_inst(ls, i);
        Nst_InstCode code = inst->code;
        // Dead code can only be analyzed on defined code paths
        if (code != Nst_IC_JUMP
            && code != Nst_IC_RETURN_VAL
            && code != Nst_IC_THROW_ERR)
        {
            continue;
        }
        // Backwards jumps are ignored
        if (code == Nst_IC_JUMP && (usize)inst->val < i)
            continue;

        // Find the unreachable block of instructions after the current
        // instruciton. The block will have limits [i, end) (i is included,
        // end is exluded)

        // A jump is useless if none of the instructions between it and the
        // point it jumps to are reachable
        bool jump_is_useless = code == Nst_IC_JUMP;
        bool stop_at_save_error = code == Nst_IC_THROW_ERR;
        usize end = code == Nst_IC_RETURN_VAL ? size : (usize)inst->val;

        for (usize j = i + 1; j < end; j++) {
            // when 'end' is changed check the instructions in the block again
            // as the new reachable instructions could contain a new reference
            // to an instruction in the unreachable block

            if (has_jumps_to(ls, j, i + 1, end - 1)) {
                jump_is_useless = false;
                end = j;
                j = i;
                continue;
            }

            if (stop_at_save_error && inst_code(ls, j) == Nst_IC_SAVE_ERROR) {
                end = j;
                j = i;
                continue;
            }
        }

        if (i + 1 >= end && !jump_is_useless)
            continue;

        if (jump_is_useless) {
            ret = inst_code(ls, i) != Nst_IC_NO_OP;
            Nst_ilist_set(ls, i, Nst_IC_NO_OP);
        }

        // remove dead instructions
        for (usize j = i + 1; j < end; j++) {
            ret = inst_code(ls, j) != Nst_IC_NO_OP;
            Nst_ilist_set(ls, j, Nst_IC_NO_OP);
        }
    }
    return ret;
}

typedef u8 *BoolArray;

static inline BoolArray bool_arr_new(usize size)
{
    usize length = size / 8;
    if (size % 8 != 0)
        length++;
    return (BoolArray)Nst_raw_calloc(length, sizeof(u8));
}

static inline bool bool_arr_get(BoolArray array, usize idx)
{
    return (bool)(array[idx / 8] & (1 << (idx % 8)));
}

static inline void bool_arr_set(BoolArray array, usize idx, bool value)
{
    u8 byte = array[idx / 8];
    if (value)
        array[idx / 8] = byte | (1 << (idx % 8));
    else
        array[idx / 8] = byte & ~(1 << (idx % 8));
}

static inline void bool_arr_fill(BoolArray array, usize size, bool value)
{
    usize length = size / 8;
    if (size % 8 != 0)
        length++;
    u8 set_value = value ? 255 : 0;
    memset(array, set_value, length);
}

static bool optimize_chained_jumps(Nst_InstList *ls)
{
    usize size = Nst_ilist_len(ls);
    BoolArray visited_jumps = bool_arr_new(size);
    bool ret = false;
    if (visited_jumps == NULL)
        return false;

    for (usize i = 0; i < size; i++) {
        Nst_Inst *first_jump = Nst_ilist_get_inst(ls, i);
        if (!Nst_ic_is_jump(first_jump->code))
            continue;

        bool_arr_fill(visited_jumps, size, false);
        bool_arr_set(visited_jumps, (usize)first_jump->val, true);
        Nst_Inst *jump = Nst_ilist_get_inst(ls, (usize)first_jump->val);

        while (jump->code == Nst_IC_JUMP) {
            first_jump->val = jump->val;
            usize new_target = (usize)jump->val;
            if (bool_arr_get(visited_jumps, new_target))
                break;
            bool_arr_set(visited_jumps, new_target, true);
            jump = Nst_ilist_get_inst(ls, new_target);
            ret = true;
        }
    }
    Nst_free(visited_jumps);
    return ret;
}
