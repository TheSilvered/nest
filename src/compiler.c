#include "nest.h"

#define CURR_LEN ((i64)(c_state.ls.instructions.len))
#define CONTINUE_ID(loop_id) (loop_id)
#define BREAK_ID(loop_id) ((loop_id) - 1)

typedef struct _CompilerState {
    i64 loop_id;
    Nst_Obj *unique_values;
    Nst_InstList ls;
} CompileState;

static CompileState c_state;

static i64 inc_loop_id(void);
static void dec_loop_id(void);

static Nst_Inst *add_inst(Nst_InstCode inst, Nst_Span span);
static Nst_Inst *add_inst_ex(Nst_InstCode inst, i64 val, Nst_Span span);
static Nst_Inst *add_inst_obj(Nst_InstCode inst, Nst_Obj *obj, Nst_Span span);
static Nst_Inst *get_inst(usize idx);

static void replace_placeholder_jumps(usize start, usize end, i64 loop_id,
                                      i64 continue_idx, i64 break_idx);

static bool compile_node(Nst_Node *node);
static bool compile_cs(Nst_Node *node);
static bool compile_wl(Nst_Node *node);
static bool compile_dowhile_l(Nst_Node *node);
static bool compile_fl(Nst_Node *node);
static bool compile_for_as_l(Nst_Node *node);
static bool compile_fd(Nst_Node *node);
static bool compile_rt(Nst_Node *node);
static bool compile_continue_s(Nst_Node *node);
static bool compile_break_s(Nst_Node *node);
static bool compile_switch_s(Nst_Node *node);
static bool compile_try_catch_s(Nst_Node *node);
static bool compile_s_wrapper(Nst_Node *node);
static bool compile_stack_op(Nst_Node *node);
static bool compile_comp_op(Nst_Node *node);
static bool compile_lg_op(Nst_Node *node);
static bool compile_local_stack_op(Nst_Node *node);
static bool compile_local_op(Nst_Node *node);
static bool compile_seq_lit(Nst_Node *node);
static bool compile_map_lit(Nst_Node *node);
static bool compile_value(Nst_Node *node);
static bool compile_access(Nst_Node *node);
static bool compile_extract_e(Nst_Node *node);
static bool compile_assign_e(Nst_Node *node);
static bool compile_unpacking_assign_e(Nst_Node *node);
static bool compile_comp_assign_e(Nst_Node *node);
static bool compile_assignment_name(Nst_Node *node, bool local);
static bool compile_if_e(Nst_Node *node);
static bool compile_e_wrapper(Nst_Node *node);

static i64 inc_loop_id(void)
{
    c_state.loop_id -= 2;
    return c_state.loop_id;
}

static void dec_loop_id(void)
{
    c_state.loop_id += 2;
}

static Nst_Inst *add_inst(Nst_InstCode inst, Nst_Span span)
{
    if (!Nst_ilist_add(&c_state.ls, inst, span)) {
        Nst_error_add_span(span);
        return NULL;
    }
    return get_inst(CURR_LEN - 1);
}

static Nst_Inst *add_inst_ex(Nst_InstCode inst, i64 val, Nst_Span span)
{
    if (!Nst_ilist_add_ex(&c_state.ls, inst, val, span)) {
        Nst_error_add_span(span);
        return NULL;
    }
    return get_inst(CURR_LEN - 1);
}

static Nst_Inst *add_inst_obj(Nst_InstCode inst, Nst_Obj *obj, Nst_Span span)
{
    i64 idx = -1;

    if (obj->type == Nst_t.Real) {
        idx = Nst_ilist_add_obj(&c_state.ls, Nst_inc_ref(obj));
        if (idx < 0) {
            Nst_error_add_span(span);
            Nst_dec_ref(obj);
            return NULL;
        }
        goto add_inst;
    }

    Nst_Obj *idx_obj = Nst_map_get(c_state.unique_values, obj);
    if (idx_obj != NULL) {
        idx = Nst_int_i64(idx_obj);
        Nst_dec_ref(idx_obj);
    } else {
        idx = Nst_ilist_add_obj(&c_state.ls, Nst_inc_ref(obj));
        if (idx < 0) {
            Nst_error_add_span(span);
            Nst_dec_ref(obj);
            return NULL;
        }
        idx_obj = Nst_int_new(idx);
        if (idx_obj == NULL) {
            Nst_error_add_span(span);
            return NULL;
        }
        if (!Nst_map_set(c_state.unique_values, obj, idx_obj)) {
            Nst_dec_ref(idx_obj);
            Nst_error_add_span(span);
            return NULL;
        }
        Nst_dec_ref(idx_obj);
    }

add_inst:
    if (!Nst_ilist_add_ex(&c_state.ls, inst, idx, span)) {
        Nst_error_add_span(span);
        return NULL;
    }
    return get_inst(CURR_LEN - 1);
}

static Nst_Inst *get_inst(usize idx)
{
    return (Nst_Inst *)Nst_da_get(&c_state.ls.instructions, idx);
}

static void replace_placeholder_jumps(usize start, usize end, i64 loop_id,
                                      i64 continue_idx, i64 break_idx)
{
    for (usize i = start; i < end; i++) {
        Nst_Inst *inst = get_inst(i);
        if (Nst_ic_is_jump(inst->code)) {
            if (inst->val == CONTINUE_ID(loop_id)) {
                inst->val = continue_idx;
            } else if (inst->val == BREAK_ID(loop_id)) {
                inst->val = break_idx;
            }
        }
    }
}

Nst_InstList Nst_compile(Nst_Node *ast, bool is_module)
{
    c_state.loop_id = 0;
    if (!Nst_ilist_init(&c_state.ls)) {
        Nst_error_add_span(ast->span);
        return c_state.ls;
    }

    c_state.unique_values = Nst_map_new();
    if (c_state.unique_values == NULL) {
        Nst_error_add_span(ast->span);
        Nst_ilist_destroy(&c_state.ls);
        return c_state.ls;
    }

    if (!compile_node(ast))
        goto failure;

    if (is_module) {
        if (!add_inst(Nst_IC_RETURN_VARS, ast->span))
            goto failure;
    } else if (c_state.ls.instructions.len == 0
               || get_inst(CURR_LEN - 1)->code != Nst_IC_RETURN_VAL)
    {
        if (!add_inst_obj(Nst_IC_PUSH_VAL, Nst_c.Null_null, ast->span))
            goto failure;
        if (!add_inst(Nst_IC_RETURN_VAL, ast->span))
            goto failure;
    }

    Nst_dec_ref(c_state.unique_values);
    return c_state.ls;

failure:
    Nst_dec_ref(c_state.unique_values);
    Nst_ilist_destroy(&c_state.ls);
    return c_state.ls;
}

static bool compile_node(Nst_Node *node)
{
    switch (node->type) {
    case Nst_NT_CS: return compile_cs(node);
    case Nst_NT_WL: return compile_wl(node);
    case Nst_NT_FL: return compile_fl(node);
    case Nst_NT_FD: return compile_fd(node);
    case Nst_NT_RT: return compile_rt(node);
    case Nst_NT_CN: return compile_continue_s(node);
    case Nst_NT_BR: return compile_break_s(node);
    case Nst_NT_SW: return compile_switch_s(node);
    case Nst_NT_TC: return compile_try_catch_s(node);
    case Nst_NT_WS: return compile_s_wrapper(node);
    case Nst_NT_NP: return true;
    case Nst_NT_SO: return compile_stack_op(node);
    case Nst_NT_LS: return compile_local_stack_op(node);
    case Nst_NT_LO: return compile_local_op(node);
    case Nst_NT_SL: return compile_seq_lit(node);
    case Nst_NT_ML: return compile_map_lit(node);
    case Nst_NT_VL: return compile_value(node);
    case Nst_NT_AC: return compile_access(node);
    case Nst_NT_EX: return compile_extract_e(node);
    case Nst_NT_AS: return compile_assign_e(node);
    case Nst_NT_CA: return compile_comp_assign_e(node);
    case Nst_NT_IE: return compile_if_e(node);
    case Nst_NT_WE: return compile_e_wrapper(node);
    default:
        Nst_assert_c(false);
        return false;
    }
}

static bool compile_cs(Nst_Node *node)
{
    /*
    Compound statement instructions

    [STATEMENT]
    POP_VAL - added if the statement leaves a value on the evaluation stack
    ^ repeated for every statement
    */

    for (usize i = 0, n = node->v.cs.statements.len; i < n; i++) {
        Nst_Node *statement = Nst_NODE(Nst_da_get_p(&node->v.cs.statements, i));
        if (!compile_node(statement))
            return false;
        if (Nst_NODE_RETUNS_VALUE(statement->type)) {
            if (!add_inst(Nst_IC_POP_VAL, statement->span))
                return false;
        }
    }
    return true;
}

static bool compile_wl(Nst_Node *node)
{
    /*
    While loop instructions

    cond: [CONDITION CODE]
          JUMPIF_F exit
          [BODY CODE]
          JUMP cond
    exit: [CODE CONTINUATION]
    */

    if (node->v.wl.is_dowhile)
        return compile_dowhile_l(node);

    i64 cond_idx = CURR_LEN;
    if (!compile_node(node->v.wl.condition))
        return false;
    Nst_Inst *jumpif_f_exit = add_inst(Nst_IC_JUMPIF_F, node->span);
    if (!jumpif_f_exit)
        return false;

    i64 start = CURR_LEN;
    i64 loop_id = inc_loop_id();
        if (!compile_node(node->v.wl.body))
            return false;
    dec_loop_id();
    i64 end = CURR_LEN;

    if (!add_inst_ex(Nst_IC_JUMP, cond_idx, node->span))
        return false;
    i64 exit_idx = CURR_LEN;
    jumpif_f_exit->val = exit_idx;

    replace_placeholder_jumps(start, end, loop_id, cond_idx, exit_idx);

    return true;
}

static bool compile_dowhile_l(Nst_Node *node)
{
    /*
    Do-while loop instructions

    body: [BODY CODE]
          [CONDITION CODE]
          JUMPIF_T body
          [CODE CONTINUATION]
    */

    i64 start = CURR_LEN;
    i64 loop_id = inc_loop_id();
    if (!compile_node(node->v.wl.body))
        return false;
    dec_loop_id();
    i64 end = CURR_LEN;

    if (!compile_node(node->v.wl.condition))
        return false;

    if (!add_inst_ex(Nst_IC_JUMPIF_T, start, node->span))
        return false;

    i64 break_idx = CURR_LEN;

    replace_placeholder_jumps(start, end, loop_id, end, break_idx);
    return true;
}

static bool compile_fl(Nst_Node *node)
{
    /*
    For loop instructions

          [TIMES TO REPEAT CODE]
          NEW_INT
    cond: JUMPIF_ZERO exit
          [BODY CODE]
          DEC_INT
          JUMP cond
    exit: POP_VAL
          [CODE CONTINUATION]
    */

    if (node->v.fl.assignment != NULL)
        return compile_for_as_l(node);

    if (!compile_node(node->v.fl.iterator))
        return false;
    if (!add_inst(Nst_IC_NEW_INT, node->v.fl.iterator->span))
        return false;

    i64 cond_idx = CURR_LEN;
    Nst_Inst *jumpif_zero_exit = add_inst(Nst_IC_JUMPIF_ZERO, node->span);
    if (!jumpif_zero_exit)
        return false;

    i64 start = CURR_LEN;
    i64 loop_id = inc_loop_id();
    if (!compile_node(node->v.fl.body))
        return false;
    dec_loop_id();
    i64 end = CURR_LEN;

    if (!add_inst(Nst_IC_DEC_INT, node->span))
        return false;
    if (!add_inst_ex(Nst_IC_JUMP, cond_idx, node->span))
        return false;

    i64 exit_idx = CURR_LEN;
    jumpif_zero_exit->val = exit_idx;

    if (!add_inst(Nst_IC_POP_VAL, node->span))
        return false;

    replace_placeholder_jumps(start, end, loop_id, end, exit_idx);

    return true;
}

static bool compile_for_as_l(Nst_Node *node)
{
    /*
    For-as loop instructions

          [ITERATOR CODE]
          FOR_START
          POP_VAL
    cond: FOR_NEXT
          JUMPIF_IEND exit
          [ASSIGN_CODE name]
          [BODY CODE]
          JUMP cond
    exit: POP_VAL
          [CODE CONTINUATION]
    */

    if (!compile_node(node->v.fl.iterator))
        return false;

    if (!add_inst(Nst_IC_FOR_START, node->v.fl.iterator->span))
        return false;
    if (!add_inst(Nst_IC_POP_VAL, node->span))
        return false;

    i64 cond_idx = CURR_LEN;
    if (!add_inst(Nst_IC_FOR_NEXT, node->span))
        return false;
    Nst_Inst *jumpif_iend_exit = add_inst(
        Nst_IC_JUMPIF_IEND,
        node->v.fl.iterator->span);
    if (!jumpif_iend_exit)
        return false;

    if (!compile_unpacking_assign_e(node->v.fl.assignment))
        return false;

    i64 start = CURR_LEN;
    i64 loop_id = inc_loop_id();
    if (!compile_node(node->v.fl.body))
        return false;
    dec_loop_id();
    i64 end = CURR_LEN;

    if (!add_inst_ex(Nst_IC_JUMP, cond_idx, node->span))
        return false;

    i64 exit_idx = CURR_LEN;
    jumpif_iend_exit->val = exit_idx;

    if (!add_inst(Nst_IC_POP_VAL, node->span))
        return false;

    replace_placeholder_jumps(start, end, loop_id, cond_idx, exit_idx);
    dec_loop_id();
    return true;
}

static bool compile_if_e(Nst_Node *node)
{
    /*
    If expression with else body

          [CONDITION CODE]
          JUMPIF_F else
          [CODE IF TRUE]
          JUMP exit
    else: [CODE IF FALSE]
    exit: [CODE CONTINUATION]

    If expression

          [CONDITION CODE]
          JUMPIF_FALSE else
          [CODE IF TRUE]
          JUMP exit
    else: PUSH_VAL null
    exit: [CODE CONTINUATION]

    If expression with else body and both bodies being statements

          [CONDITION CODE]
          JUMPIF_F else
          [CODE IF TRUE]
          JUMP exit
    else: [CODE IF FALSE]
    exit: PUSH_VAL null
          [CODE CONTINUATION]

    If expression with body being a statement

          [CONDITION CODE]
          JUMPIF_FALSE exit
          [CODE IF TRUE]
    exit: PUSH_VAL null
          [CODE CONTINUATION]

    If both the body_if_true and the body_if_false are statements only one
    PUSH_VAL null is added after the exit label otherwise it is added right
    after the body
    */

    if (!compile_node(node->v.ie.condition))
        return false;

    Nst_Inst *jumpif_f_else = add_inst(Nst_IC_JUMPIF_F, node->span);
    if (!jumpif_f_else)
        return false;

    if (!compile_node(node->v.ie.body_if_true))
        return false;

    Nst_Inst *jump_exit = NULL;

    if (node->v.ie.body_if_false == NULL) {
        if (Nst_NODE_RETUNS_VALUE(node->v.ie.body_if_true->type)) {
            jump_exit = add_inst(Nst_IC_JUMP, node->span);
            if (!jump_exit)
                return false;
        }
        jumpif_f_else->val = CURR_LEN;

        if (!add_inst_obj(Nst_IC_PUSH_VAL, Nst_c.Null_null, node->span))
            return false;
        if (jump_exit != NULL)
            jump_exit->val = CURR_LEN;
        return true;
    }

    bool both_statements =
        !Nst_NODE_RETUNS_VALUE(node->v.ie.body_if_true->type) &&
        !Nst_NODE_RETUNS_VALUE(node->v.ie.body_if_false->type);

    if (!both_statements
        && !Nst_NODE_RETUNS_VALUE(node->v.ie.body_if_true->type))
    {
        if (!add_inst_obj(Nst_IC_PUSH_VAL, Nst_c.Null_null, node->span))
            return false;
    }

    jump_exit = add_inst(Nst_IC_JUMP, node->span);
    if (!jump_exit)
        return false;
    jumpif_f_else->val = CURR_LEN;

    if (!compile_node(node->v.ie.body_if_false))
        return false;

    if (!both_statements
        && !Nst_NODE_RETUNS_VALUE(node->v.ie.body_if_false->type))
    {
        if (!add_inst_obj(Nst_IC_PUSH_VAL, Nst_c.Null_null, node->span))
            return false;
    }
    jump_exit->val = CURR_LEN;
    if (both_statements) {
        if (!add_inst_obj(Nst_IC_PUSH_VAL, Nst_c.Null_null, node->span))
            return false;
    }

    return true;
}

static bool compile_fd(Nst_Node *node)
{
    /*
    Func declaration instructions

    MAKE_FUNC
    SET_VAL_LOC name
    */

    i64 prev_loop_id = c_state.loop_id;
    Nst_Obj *prev_values = c_state.unique_values;
    Nst_InstList prev_inst_ls = c_state.ls;
    Nst_InstList inst_list = Nst_compile(node->v.fd.body, false);
    c_state.ls = prev_inst_ls;
    c_state.loop_id = prev_loop_id;
    c_state.unique_values = prev_values;
    if (inst_list.instructions.len == 0)
        return false;

    Nst_FuncPrototype fp;
    if (!Nst_fprototype_init(&fp, inst_list, node->v.fd.argument_names.len)) {
        Nst_ilist_destroy(&inst_list);
        Nst_error_add_span(node->span);
        return false;
    }

    for (usize i = 0; i < fp.arg_num; i++) {
        fp.arg_names[i] = Nst_inc_ref(
            NstOBJ(Nst_da_get_p(&node->v.fd.argument_names, i)));
    }

    isize func_idx = Nst_ilist_add_func(&c_state.ls, &fp);
    if (func_idx < 0) {
        Nst_fprototype_destroy(&fp);
        Nst_error_add_span(node->span);
        return false;
    }

    if (!add_inst_ex(Nst_IC_MAKE_FUNC, func_idx, node->span))
        return false;

    if (node->v.fd.name == NULL)
        return true;

    return add_inst_obj(Nst_IC_SET_VAL_LOC, node->v.fd.name, node->span);
}

static bool compile_rt(Nst_Node *node)
{
    /*
    Return instructions

    [EXPR CODE]
    RETURN_VAL

    Pops all values from the stack until it finds a nullptr
    */
    if (node->v.rt.value == NULL) {
        if (!add_inst_obj(Nst_IC_PUSH_VAL, Nst_c.Null_null, node->span))
            return false;
    } else if (!compile_node(node->v.rt.value))
        return false;

    return add_inst(Nst_IC_RETURN_VAL, node->span);
}

static bool compile_stack_op(Nst_Node *node)
{
    /*
    Stack operation instructions

    [VAL 1 CODE]
    [VAL 2 CODE]
    STACK_OP - op
    [VAL 3 CODE]
    STACK_OP - op
    [VAL 4 CODE]
    STACK_OP - op
    ...
    [VAL N CODE]
    STACK_OP - op
    */

    Nst_Node *lnode = Nst_NODE(Nst_da_get_p(&node->v.so.values, 0));
    if (!compile_node(lnode))
        return false;

    if (node->v.so.values.len == 1)
        return true;

    if (_Nst_TOK_IS_COMP_OP(node->v.so.op) && node->v.so.values.len > 2)
        return compile_comp_op(node);

    if (_Nst_TOK_IS_COND_OP(node->v.so.op))
        return compile_lg_op(node);

    for (usize i = 1, n = node->v.so.values.len; i < n; i++) {
        lnode = Nst_NODE(Nst_da_get_p(&node->v.so.values, i));
        if (!compile_node(lnode))
            return false;
        if (!add_inst_ex(Nst_IC_STACK_OP, node->v.so.op, node->span))
            return false;
    }
    return true;
}

static bool compile_comp_op(Nst_Node *node)
{
    /*
    Comparison operator instructions operating on N values with N > 2

         [VALUE 1]
         [VALUE 2]            -+
         DUP                   |
         ROT 3                 |
         OP_STACK - operator   | Repeated for N - 2 values
         DUP                   | (excludes the first and the last values)
         JUMPIF_F fix          |
         POP_VAL              -+
         [VALUE 3]
         DUP
         ROT 3
         OP_STACK - operator
         DUP
         JUMPIF_F fix
         POP_VAL
         ...
         [VALUE N]
         OP_STACK - operator
         JUMP end
    fix: ROT 2
         POP_VAL
    end: [CODE CONTINUATION]
    */

    Nst_TokType op = node->v.so.op;

    Nst_DynArray jumpif_f_fix_list;
    if (!Nst_da_init(
            &jumpif_f_fix_list,
            sizeof(usize),
            node->v.so.values.len - 2))
    {
        Nst_error_add_span(node->span);
        return false;
    }

    for (usize i = 1, n = node->v.so.values.len; i < n - 1; i++) {
        Nst_Node *lnode = Nst_NODE(Nst_da_get_p(&node->v.so.values, i));
        if (!compile_node(lnode))
            goto failure;
        if (!add_inst(Nst_IC_DUP, node->span))
            goto failure;
        if (!add_inst(Nst_IC_ROT_3, node->span))
            goto failure;
        if (!add_inst_ex(Nst_IC_STACK_OP, op, node->span))
            goto failure;
        if (!add_inst(Nst_IC_DUP, node->span))
            goto failure;
        usize idx = CURR_LEN;
        Nst_da_append(&jumpif_f_fix_list, &idx);
        if (!add_inst(Nst_IC_JUMPIF_F, node->span))
            goto failure;
        if (!add_inst(Nst_IC_POP_VAL, node->span))
            goto failure;
    }

    Nst_Node *last_node = Nst_NODE(Nst_da_get_p(
        &node->v.so.values,
        node->v.so.values.len - 1));
    if (!compile_node(last_node))
        goto failure;

    if (!add_inst_ex(Nst_IC_STACK_OP, op, node->span))
        goto failure;
    if (!add_inst_ex(Nst_IC_JUMP, CURR_LEN + 3, node->span))
        goto failure;

    for (usize i = 0; i < jumpif_f_fix_list.len; i++)
        get_inst(*(usize *)Nst_da_get(&jumpif_f_fix_list, i))->val = CURR_LEN;
    Nst_da_clear(&jumpif_f_fix_list, NULL);

    if (!add_inst(Nst_IC_ROT_2, node->span))
        return false;
    if (!add_inst(Nst_IC_POP_VAL, node->span))
        return false;
    return true;

failure:
    Nst_da_clear(&jumpif_f_fix_list, NULL);
    return false;
}

static bool compile_lg_op(Nst_Node *node)
{
    /*
    L_AND instructions

         [VAL 1 CODE]
         DUP          -+
         JUMPIF_F end  | Repeated for all values
         POP_VAL       |
         [VAL 2 CODE] -+
    end: [CODE CONTINUATION]

    L_OR instructions

         [VAL 1 CODE]
         DUP          -+
         JUMPIF_T end  | Repeated for all values
         POP_VAL       |
         [VAL 2 CODE] -+
    end: [CODE CONTINUATION]
    */

    Nst_DynArray jumpif_end;
    if (!Nst_da_init(&jumpif_end, sizeof(usize), node->v.so.values.len - 1)) {
        Nst_error_add_span(node->span);
        return false;
    }

    Nst_InstCode jump_code =
        node->v.so.op == Nst_TT_L_OR ? Nst_IC_JUMPIF_T : Nst_IC_JUMPIF_F;

    for (usize i = 1, n = node->v.so.values.len; i < n; i++) {
        if (!add_inst(Nst_IC_DUP, node->span))
            goto failure;
        usize idx = CURR_LEN;
        Nst_da_append(&jumpif_end, &idx);
        if (!add_inst(jump_code, node->span))
            goto failure;
        if (!add_inst(Nst_IC_POP_VAL, node->span))
            goto failure;
        if (!compile_node(Nst_NODE(Nst_da_get_p(&node->v.so.values, i))))
            goto failure;
    }

    for (usize i = 0; i < jumpif_end.len; i++)
        get_inst(*(usize *)Nst_da_get(&jumpif_end, i))->val = CURR_LEN;
    Nst_da_clear(&jumpif_end, NULL);
    return true;

failure:
    Nst_da_clear(&jumpif_end, NULL);
    return false;
}

static bool compile_local_stack_op(Nst_Node *node)
{
    /*
    Local stack operation instructions

    [VALUE CODE]
    [VALUE CODE]
    ...
    [SPECIAL VALUE]

    OP_RANGE - arg num

    OP_CAST

    OP_CALL - arg num

    OP_SEQ_CALL for sequence calls

    THROW_ERR
    */

    for (usize i = 0, n = node->v.ls.values.len; i < n; i++) {
        Nst_Node *lnode = Nst_NODE(Nst_da_get_p(&node->v.ls.values, i));
        if (!compile_node(lnode))
            return false;
    }

    if (!compile_node(node->v.ls.special_value))
        return false;

    i64 node_count = (i64)node->v.ls.values.len;

    switch (node->v.ls.op) {
    case Nst_TT_RANGE:
        return add_inst_ex(Nst_IC_OP_RANGE, node_count + 1, node->span);
    case Nst_TT_CAST:
        return add_inst(Nst_IC_OP_CAST, node->span);
    case Nst_TT_CALL:
        return add_inst(Nst_IC_OP_CALL, node->span);
    case Nst_TT_SEQ_CALL:
        return add_inst_ex(Nst_IC_OP_SEQ_CALL, node_count, node->span);
    case Nst_TT_THROW:
        return add_inst(Nst_IC_THROW_ERR, node->span);
    default:
        Nst_assert_c(false);
        return false;
    }
}

static bool compile_local_op(Nst_Node *node)
{
    /*
    Local operator instructions

    [VALUE CODE]
    LOCAL_OP - operator

    OP_IMPORT

    OP_CALL 0
    */

    if (!compile_node(node->v.lo.value))
        return false;

    switch (node->v.lo.op) {
    case Nst_TT_LOC_CALL:
        return add_inst_ex(Nst_IC_OP_CALL, 0, node->span);
    case Nst_TT_IMPORT:
        return add_inst(Nst_IC_OP_IMPORT, node->span);
    default:
        return add_inst_ex(Nst_IC_LOCAL_OP, node->v.lo.op, node->span);
    }
}

static bool compile_seq_lit(Nst_Node *node)
{
    /*
    Vector or array literal instructions

    [VALUE CODE]
    [VALUE CODE]
    ...
    MAKE_ARR | MAKE_VEC - number of elements

    [VALUE CODE]
    [TIMES TO REPEAT CODE]
    MAKE_ARR_REP | MAKE_VEC_REP
    */

    Nst_assert_c(node->v.sl.type != Nst_SNT_ASSIGNMENT_NAMES);

    for (usize i = 0, n = node->v.sl.values.len; i < n; i++) {
        Nst_Node *lnode = Nst_NODE(Nst_da_get_p(&node->v.sl.values, i));
        if (!compile_node(lnode))
            return false;
    }

    switch (node->v.sl.type) {
    case Nst_SNT_ARRAY:
        return add_inst_ex(Nst_IC_MAKE_ARR, node->v.sl.values.len, node->span);
    case Nst_SNT_VECTOR:
        return add_inst_ex(Nst_IC_MAKE_VEC, node->v.sl.values.len, node->span);
    case Nst_SNT_ARRAY_REP:
        return add_inst(Nst_IC_MAKE_ARR_REP, node->span);
    case Nst_SNT_VECTOR_REP:
        return add_inst(Nst_IC_MAKE_VEC_REP, node->span);
    default:
        Nst_assert_c(false);
        return false;
    }
}

static bool compile_map_lit(Nst_Node *node)
{
    /*
    Map literal instructions

    [KEY CODE]   -\ Repeated for each pair
    [VALUE CODE] -/
    MAKE_MAP - number of pairs
    */

    Nst_assert_c(node->v.ml.keys.len == node->v.ml.values.len);

    for (usize i = 0, n = node->v.ml.keys.len; i < n; i++) {
        Nst_Node *key = Nst_NODE(Nst_da_get_p(&node->v.ml.keys, i));
        Nst_Node *val = Nst_NODE(Nst_da_get_p(&node->v.ml.values, i));
        if (!compile_node(key))
            return false;
        if (!compile_node(val))
            return false;
    }

    return add_inst_ex(Nst_IC_MAKE_MAP, node->v.ml.keys.len, node->span);
}

static bool compile_value(Nst_Node *node)
{
    /*
    Value instructions

    PUSH_VAL value
    */
    return add_inst_obj(Nst_IC_PUSH_VAL, node->v.vl.value, node->span);
}

static bool compile_access(Nst_Node *node)
{
    /*
    Access instructions

    GET_VAL name
    */
    return add_inst_obj(Nst_IC_GET_VAL, node->v.ac.value, node->span);
}

static bool compile_extract_e(Nst_Node *node)
{
    /*
    Extraction instructions

    [CONTAINER CODE]
    [INDEX_CODE]
    OP_EXTRACT
    */
    return compile_node(node->v.ex.container)
        && compile_node(node->v.ex.key)
        && add_inst(Nst_IC_OP_EXTRACT, node->span);
}

static bool compile_assign_e(Nst_Node *node)
{
    /*
    Assignment instructions for variable name

    [VALUE CODE]
    SET_VAL name

    Assignment instructions for container

    [VALUE CODE]
    [CONTAINER CODE]
    [KEY CODE]
    SET_CONT_VAL
    */

    if (!compile_node(node->v.as.value))
        return false;

    if (node->v.as.name->type == Nst_NT_SL) {
        return add_inst(Nst_IC_DUP, node->span)
            && compile_unpacking_assign_e(node->v.as.name);
    } else
        return compile_assignment_name(node->v.as.name, false);
}

static bool compile_unpacking_assign_e(Nst_Node *node)
{
    /*
    Unpacking assigment

    [VALUE CODE]
    DUP
    UNPACK_SEQ expected_size

    SET_VAL_LOC name - for variables

    [CONTAINER_CODE]  +
    [INDEX_CODE]      | for containers
    SET_CONTAINER_VAL +
    */

    if (node->type == Nst_NT_SL) {
        Nst_assert_c(node->v.sl.type == Nst_SNT_ASSIGNMENT_NAMES);
        Nst_assert_c(node->v.sl.values.len >= 1);

        if (!add_inst_ex(Nst_IC_UNPACK_SEQ, node->v.sl.values.len, node->span))
            return false;

        for (usize i = 0, n = node->v.sl.values.len; i < n; i++) {
            Nst_Node *lnode = Nst_NODE(Nst_da_get_p(&node->v.sl.values, i));
            if (!compile_unpacking_assign_e(lnode))
                return false;
        }
        return true;
    } else
        return compile_assignment_name(node, true);
}

static bool compile_comp_assign_e(Nst_Node *node)
{
    /*
    Compound assignment instructions

    [NAME NODE RAW]
    [VALUES LIKE STACK OP]
    STACK_OP - op

    Assignment instructions for variable name

    [VALUE CODE]
    SET_VAL name

    Assignment instructions for container

    [VALUE CODE]
    [CONTAINER CODE]
    [KEY CODE]
    SET_CONT_VAL
    */

    if (!compile_node(node->v.ca.name))
        return false;

    for (usize i = 0, n = node->v.ca.values.len; i < n; i++) {
        Nst_Node *lnode = Nst_NODE(Nst_da_get_p(&node->v.ca.values, i));
        if (!compile_node(lnode))
            return false;
        if (!add_inst_ex(Nst_IC_STACK_OP, node->v.ca.op, node->span))
            return false;
    }

    return compile_assignment_name(node->v.ca.name, false);
}

static bool compile_assignment_name(Nst_Node *node, bool local)
{
    if (node->type == Nst_NT_AC) {
        return add_inst_obj(
            local ? Nst_IC_SET_VAL_LOC : Nst_IC_SET_VAL,
            node->v.ac.value,
            node->span);
    } else if (node->type == Nst_NT_EX) {
        return compile_node(node->v.ex.container)
            && compile_node(node->v.ex.key)
            && add_inst(
                local ? Nst_IC_SET_CONT_LOC : Nst_IC_SET_CONT_VAL,
                node->span);
    } else {
        Nst_assert_c(false);
        return false;
    }
}

static bool compile_continue_s(Nst_Node *node)
{
    /*
    Continue instructions

    JUMP loop_id - replaced later with the loop's start
    */
    return add_inst_ex(Nst_IC_JUMP, CONTINUE_ID(c_state.loop_id), node->span);
}

static bool compile_break_s(Nst_Node *node)
{
    /*
    Continue instructions

    JUMP loop_id - replaced later with the loop's end
    */
    return add_inst_ex(Nst_IC_JUMP, BREAK_ID(c_state.loop_id), node->span);
}

static bool compile_switch_s(Nst_Node *node)
{
    /*
    Switch statement instructions

          [EXPRESSION CODE]
          DUP           -+
          [VALUE1 CODE]  |
          OP_STACK ==    | Repeated for every case
          JUMPIF_F end1  |
          [BODY CODE]    |
          JUMP exit     -+
    end1: DUP
          [VALUE2 CODE]
          OP_STACK ==
          JUMPIF_F end2
          [BODY CODE]
          JUMP exit
    end2: [DEFAULT CODE]
    exit: POP_VAL
          [CODE CONTINUATION]
    */

    Nst_assert_c(node->v.sw.values.len == node->v.sw.bodies.len);

    Nst_DynArray jumps_to_exit;
    Nst_da_init(&jumps_to_exit, sizeof(Nst_Inst *), 0);

    if (!compile_node(node->v.sw.expr))
        return false;

    usize prev_start = 0;
    usize prev_end = 0;
    usize loop_id = c_state.loop_id;

    for (usize i = 0, n = node->v.sw.values.len; i < n; i++) {
        Nst_Node *value = Nst_NODE(Nst_da_get_p(&node->v.sw.values, i));
        Nst_Node *body = Nst_NODE(Nst_da_get_p(&node->v.sw.bodies, i));

        if (!add_inst(Nst_IC_DUP, node->span))
            goto failure;
        if (!compile_node(value))
            goto failure;
        if (!add_inst_ex(Nst_IC_STACK_OP, Nst_TT_EQ, node->span))
            goto failure;
        Nst_Inst *jumpif_f_end = add_inst(Nst_IC_JUMPIF_F, value->span);
        if (!jumpif_f_end)
            goto failure;

        if (prev_start != prev_end) {
            // replaces only 'continue' statements, 'break' statements are left
            // untouched for the loop to replace
            replace_placeholder_jumps(
                prev_start, prev_end,
                loop_id,
                CURR_LEN, BREAK_ID(loop_id));
        }

        prev_start = CURR_LEN;
        if (!compile_node(body))
            goto failure;
        prev_end = CURR_LEN;

        Nst_Inst *jump_exit = add_inst(Nst_IC_JUMP, body->span);
        if (!jump_exit)
            goto failure;
        if (!Nst_da_append(&jumps_to_exit, &jump_exit))
            goto failure_add_pos;

        jumpif_f_end->val = CURR_LEN;
    }

    if (node->v.sw.default_body != NULL) {
        if (prev_start != prev_end) {
            replace_placeholder_jumps(
                prev_start, prev_end,
                loop_id,
                CURR_LEN, BREAK_ID(loop_id));
        }
        prev_start = CURR_LEN;
        if (!compile_node(node->v.sw.default_body))
            goto failure;
        prev_end = CURR_LEN;
        // all continue statements in the default body just exit the switch
        replace_placeholder_jumps(
            prev_start, prev_end,
            loop_id,
            CURR_LEN, BREAK_ID(loop_id));
    }

    for (usize i = 0; i < jumps_to_exit.len; i++)
        ((Nst_Inst *)Nst_da_get_p(&jumps_to_exit, i))->val = CURR_LEN;
    Nst_da_clear(&jumps_to_exit, NULL);

    return !add_inst(Nst_IC_POP_VAL, node->span);

failure_add_pos:
    Nst_error_add_span(node->span);
failure:
    Nst_da_clear(&jumps_to_exit, NULL);
    return false;
}

static bool compile_try_catch_s(Nst_Node *node)
{
    /*
    Switch statement instructions

                 PUSH_CATCH catch_start
                 [TRY BODY]
                 POP_CATCH
                 JUMP catch_end
    catch_start: SAVE_ERROR
                 POP_CATCH
                 SET_VAL_LOC err_name
                 [CATCH BODY]
    catch_end:   [CODE CONTINUATION]
    */

    Nst_Inst *push_catch = add_inst(Nst_IC_PUSH_CATCH, node->span);
    if (!push_catch)
        return false;
    if (!compile_node(node->v.tc.try_body))
        return false;

    if (Nst_NODE_RETUNS_VALUE(node->v.tc.try_body->type)) {
        if (!add_inst(Nst_IC_POP_VAL, node->span))
            return false;
    }

    if (!add_inst(Nst_IC_POP_CATCH, node->span))
        return false;

    Nst_Inst *jump_catch_end = add_inst(Nst_IC_JUMP, node->span);
    if (!jump_catch_end)
        return false;
    push_catch->val = CURR_LEN;

    if (!add_inst(Nst_IC_SAVE_ERROR, node->span))
        return false;
    if (!add_inst(Nst_IC_POP_CATCH, node->span))
        return false;
    if (!add_inst_obj(Nst_IC_SET_VAL_LOC, node->v.tc.error_name, node->span))
        return false;
    if (!compile_node(node->v.tc.catch_body))
        return false;

    if (Nst_NODE_RETUNS_VALUE(node->v.tc.catch_body->type)) {
        if (!add_inst(Nst_IC_POP_VAL, node->span))
            return false;
    }
    jump_catch_end->val = CURR_LEN;
    return true;
}

static bool compile_s_wrapper(Nst_Node *node)
{
    return compile_node(node->v.ws.statement);
}

static bool compile_e_wrapper(Nst_Node *node)
{
    return compile_node(node->v.we.expr);
}
