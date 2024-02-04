#include "tokens.h"
#include "function.h"
#include "obj_ops.h"
#include "global_consts.h"
#include "mem.h"
#include "format.h"

#define INST(instruction) ((Nst_Inst *)(instruction))
#define CURR_LEN ((i64)(c_state.inst_ls->len))

#define NULL_OR_APPEND_FAILED(inst) ((inst) != NULL || !append_inst(inst))

#define PRINT(str, size) Nst_fwrite(str, size, NULL, Nst_io.out)

typedef enum _CompilationType {
    CT_FILE,
    CT_MODULE,
    CT_FUNCTION
} CompilationType;

typedef struct _CompilerState {
    i32 loop_id;
    Nst_LList *inst_ls;
} CompileState;

static CompileState c_state;

static void inc_loop_id(void)
{
    c_state.loop_id -= 2;
}

static void dec_loop_id(void)
{
    c_state.loop_id += 2;
}

static Nst_InstList *compile_internal(Nst_Node *node, CompilationType ct);
static bool compile_long_s(Nst_Node *node);
static bool compile_while_l(Nst_Node *node);
static bool compile_dowhile_l(Nst_Node *node);
static bool compile_for_l(Nst_Node *node);
static bool compile_for_as_l(Nst_Node *node);
static bool compile_func_declr(Nst_Node *node);
static bool compile_return_s(Nst_Node *node);
static bool compile_continue_s(Nst_Node *node);
static bool compile_break_s(Nst_Node *node);
static bool compile_switch_s(Nst_Node *node);
static bool compile_try_catch_s(Nst_Node *node);
static bool compile_s_wrapper(Nst_Node *node);
static bool compile_stack_op(Nst_Node *node);
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
static bool compile_if_e(Nst_Node *node);
static bool compile_e_wrapper(Nst_Node *node);

Nst_InstList *Nst_compile(Nst_Node *ast, bool is_module)
{
    return compile_internal(ast, is_module ? CT_MODULE : CT_FILE);
}

static Nst_InstList *compile_internal(Nst_Node *node, CompilationType ct)
{
    c_state.loop_id = 0;
    c_state.inst_ls = Nst_llist_new();
    if (c_state.inst_ls == NULL) {
        Nst_error_add_positions(Nst_error_get(), node->start, node->end);
        return NULL;
    }

    compile_node(node);
    if (Nst_error_occurred()) {
        Nst_llist_destroy(
            c_state.inst_ls,
            (Nst_LListDestructor)Nst_inst_destroy);
        return NULL;
    }

    Nst_InstList *inst_list = Nst_malloc_c(1, Nst_InstList);
    if (inst_list == NULL) {
        Nst_error_add_positions(
            Nst_error_get(),
            node->start,
            node->end);
        Nst_llist_destroy(
            c_state.inst_ls,
            (Nst_LListDestructor)Nst_inst_destroy);
        return NULL;
    }

    bool add_return = c_state.inst_ls->tail == NULL
                    || INST(c_state.inst_ls->tail->value)->id
                    != Nst_IC_RETURN_VAL;

    if (ct == CT_MODULE)
        inst_list->total_size = c_state.inst_ls->len + 1;
    else
        inst_list->total_size = c_state.inst_ls->len + (add_return ? 2 : 0);

    inst_list->instructions = Nst_calloc_c(
        inst_list->total_size,
        Nst_Inst, NULL);
    if (inst_list->instructions == NULL) {
        Nst_free(inst_list);
        Nst_llist_destroy(c_state.inst_ls, Nst_free);
        Nst_error_add_positions( Nst_error_get(), node->start, node->end);
        return NULL;
    }

    Nst_LList *funcs = Nst_llist_new();
    if (funcs == NULL) {
        Nst_free(inst_list->instructions);
        Nst_free(inst_list);
        Nst_llist_destroy(c_state.inst_ls, Nst_free);
        Nst_error_add_positions(Nst_error_get(), node->start, node->end);
        return NULL;
    }

    inst_list->functions = funcs;
    usize i = 0;
    Nst_Inst *inst;
    for (Nst_LLIST_ITER(n, c_state.inst_ls)) {
        inst = INST(n->value);

        if (inst->id == Nst_IC_PUSH_VAL
            && inst->val != NULL
            && inst->val->type == Nst_t.Func)
        {
            Nst_inc_ref(inst->val);
            if (!Nst_llist_append(funcs, inst->val, true)) {
                Nst_error_add_positions(Nst_error_get(),
                    node->start,
                    node->end);
                Nst_inst_list_destroy(inst_list);
                return NULL;
            }
        }

        inst_list->instructions[i].id = inst->id;
        inst_list->instructions[i].int_val = inst->int_val;
        inst_list->instructions[i].val = inst->val;
        inst_list->instructions[i].start = inst->start;
        inst_list->instructions[i++].end = inst->end;
    }

    if (ct == CT_MODULE) {
        inst_list->instructions[i].id = Nst_IC_RETURN_VARS;
        inst_list->instructions[i].int_val = 0;
        inst_list->instructions[i].val = NULL;
        inst_list->instructions[i].start = node->start;
        inst_list->instructions[i].end = node->end;
    } else if (add_return) {
        inst_list->instructions[i].id = Nst_IC_PUSH_VAL;
        inst_list->instructions[i].int_val = 0;
        inst_list->instructions[i].val = Nst_inc_ref(Nst_c.Null_null);
        inst_list->instructions[i].start = node->start;
        inst_list->instructions[i++].end = node->end;

        inst_list->instructions[i].id = Nst_IC_RETURN_VAL;
        inst_list->instructions[i].int_val = 0;
        inst_list->instructions[i].val = NULL;
        inst_list->instructions[i].start = node->start;
        inst_list->instructions[i].end = node->end;
    }

    // Using free to not decrease the references of the objects
    Nst_llist_destroy(c_state.inst_ls, Nst_free);
    if (ct != CT_FUNCTION)
        Nst_node_destroy(node);

    return inst_list;
}

static bool append_inst(Nst_Inst *inst)
{
    if (Nst_llist_append(c_state.inst_ls, inst, true))
        return true;
    Nst_error_add_positions(Nst_error_get(), inst->start, inst->end);
    Nst_inst_destroy(inst);
    return false;
}

static bool append_list_savable(Nst_Inst *inst)
{
    if (Nst_llist_append(c_state.inst_ls, inst, true))
        return true;
    Nst_error_add_positions(Nst_error_get(), inst->start, inst->end);
    return false;
}

static Nst_Inst *new_inst(Nst_InstID id, Nst_Pos start, Nst_Pos end)
{
    Nst_Inst *inst = Nst_inst_new(id, start, end);
    if (inst == NULL) {
        Nst_error_add_positions(Nst_error_get(), start, end);
        return NULL;
    }
    return inst;
}

static Nst_Inst *new_inst_v(Nst_InstID id, Nst_Obj *val, Nst_Pos start,
                            Nst_Pos end)
{
    Nst_Inst *inst = Nst_inst_new_val(id, val, start, end);
    if (inst == NULL) {
        Nst_error_add_positions(Nst_error_get(), start, end);
        return NULL;
    }
    return inst;
}

static Nst_Inst *new_inst_i(Nst_InstID id, i64 val, Nst_Pos start,
                            Nst_Pos end)
{
    Nst_Inst *inst = Nst_inst_new_int(id, val, start, end);
    if (inst == NULL) {
        Nst_error_add_positions(Nst_error_get(), start, end);
        return NULL;
    }
    return inst;
}

static void replace_placeholder_jumps(Nst_LLNode *start, Nst_LLNode *end,
                                      i64 continue_idx, i64 break_idx)
{
    if (start == NULL)
        start = c_state.inst_ls->head;
    else
        start = start->next;

    if (end != NULL)
        end = end->next;

    for (Nst_LLNode *cursor = start;
        cursor != NULL && cursor != end;
        cursor = cursor->next)
    {
        Nst_Inst *inst = INST(cursor->value);

        if (Nst_INST_IS_JUMP(inst->id)) {
            // Continue statement
            if (inst->int_val == c_state.loop_id)
                inst->int_val = continue_idx;
            // Break statement
            else if (inst->int_val == c_state.loop_id - 1)
                inst->int_val = break_idx;
        }
    }
}

static bool compile_node(Nst_Node *node)
{
    switch (node->type) {
    case Nst_NT_LONG_S:         return compile_long_s(node);
    case Nst_NT_WHILE_L:        return compile_while_l(node);
    case Nst_NT_FOR_L:          return compile_for_l(node);
    case Nst_NT_FUNC_DECLR:     return compile_func_declr(node);
    case Nst_NT_RETURN_S:       return compile_return_s(node);
    case Nst_NT_CONTINUE_S:     return compile_continue_s(node);
    case Nst_NT_BREAK_S:        return compile_break_s(node);
    case Nst_NT_SWITCH_S:       return compile_switch_s(node);
    case Nst_NT_TRY_CATCH_S:    return compile_try_catch_s(node);
    case Nst_NT_S_WRAPPER:      return compile_s_wrapper(node);
    case Nst_NT_STACK_OP:       return compile_stack_op(node);
    case Nst_NT_LOCAL_STACK_OP: return compile_local_stack_op(node);
    case Nst_NT_LOCAL_OP:       return compile_local_op(node);
    case Nst_NT_SEQ_LIT:        return compile_seq_lit(node);
    case Nst_NT_MAP_LIT:        return compile_map_lit(node);
    case Nst_NT_VALUE:          return compile_value(node);
    case Nst_NT_ACCESS:         return compile_access(node);
    case Nst_NT_EXTRACT_E:      return compile_extract_e(node);
    case Nst_NT_ASSIGN_E:       return compile_assign_e(node);
    case Nst_NT_COMP_ASSIGN_E:  return compile_comp_assign_e(node);
    case Nst_NT_IF_E:           return compile_if_e(node);
    case Nst_NT_E_WRAPPER:      return compile_e_wrapper(node);
    }
}

static bool compile_long_s(Nst_Node *node)
{
    /*
    Long statement bytecode

    [STATEMENT]
    POP_VAL - added if the statement leaves a value on the evaluation stack
    ^ repeated for every statement
    */

    for (Nst_LLIST_ITER(lnode, node->long_s.statements)) {
        Nst_Node *statement = Nst_NODE(lnode->value);
        if (!compile_node(statement))
            return false;
        if (Nst_NODE_RETUNS_VALUE(statement->type)) {
            Nst_Inst *inst = Nst_inst_new(
                Nst_IC_POP_VAL,
                node->start,
                node->end);
            if (NULL_OR_APPEND_FAILED(inst))
                return false;
        }
    }
    return true;
}

static bool compile_while_l(Nst_Node *node)
{
    /*
    While loop bytecode

    cond: [CONDITION CODE]
          JUMPIF_F exit
          [BODY CODE]
          JUMP cond
    exit: [CODE CONTINUATION]
    */

    if (node->while_l.is_dowhile)
        return compile_dowhile_l(node);

    i64 cond_idx = CURR_LEN;
    if (!compile_node(node->while_l.condition))
        return false;
    Nst_Inst *jumpif_f_exit = Nst_inst_new(
        Nst_IC_JUMPIF_F,
        node->start,
        node->end);
    if (NULL_OR_APPEND_FAILED(jumpif_f_exit))
        return false;

    Nst_LLNode *body_start = c_state.inst_ls->tail;
    inc_loop_id();
    if (!compile_node(node->while_l.body))
        return false;
    Nst_LLNode *body_end = c_state.inst_ls->tail;

    Nst_Inst *jump_cond = Nst_inst_new_int(
        Nst_IC_JUMP,
        cond_idx,
        node->start,
        node->end);
    if (NULL_OR_APPEND_FAILED(jump_cond))
        return false;
    i64 exit_idx = CURR_LEN;
    jumpif_f_exit->int_val = exit_idx;

    replace_placeholder_jumps(body_start, body_end, cond_idx, exit_idx);
    dec_loop_id();

    return true;
}

static bool compile_dowhile_l(Nst_Node *node)
{
    /*
    Do-while loop bytecode

    body: [BODY CODE]
          [CONDITION CODE]
          JUMPIF_T body
          [CODE CONTINUATION]
    */
    Nst_LLNode *body_start = c_state.inst_ls->tail;
    i64 body_start_idx = CURR_LEN;
    inc_loop_id();
    if (!compile_node(node->while_l.body))
        return false;
    Nst_LLNode *body_end = c_state.inst_ls->tail;
    i64 continue_idx = CURR_LEN;

    dec_loop_id();
    if (!compile_node(node->while_l.condition))
        return false;
    inc_loop_id();

    Nst_Inst *jumpif_t_body = new_inst_i(
        Nst_IC_JUMPIF_T,
        body_start_idx,
        node->start,
        node->end);
    if (NULL_OR_APPEND_FAILED(jumpif_t_body))
        return false;

    i64 break_idx = CURR_LEN;

    replace_placeholder_jumps(body_start, body_end, continue_idx, break_idx);
    dec_loop_id();
    return true;
}

static bool compile_for_l(Nst_Node *node)
{
    /*
    For loop bytecode

          [TIMES TO REPEAT CODE]
          NEW_INT
    cond: JUMPIF_ZERO exit
          [BODY CODE]
          DEC_INT
          JUMP cond
    exit: POP_VAL
          [CODE CONTINUATION]
    */

    if (node->for_l.assignment != NULL)
        return compile_for_as_l(node);

    if (!compile_node(node->for_l.iterator))
        return false;
    Nst_Inst *new_int = new_inst(
        Nst_IC_NEW_INT,
        node->for_l.iterator->start,
        node->for_l.iterator->end);
    if (NULL_OR_APPEND_FAILED(new_int))
        return false;

    i64 cond_idx = CURR_LEN;
    Nst_Inst *jumpif_zero_exit = new_inst(
        Nst_IC_JUMPIF_ZERO,
        node->start,
        node->end);
    if (NULL_OR_APPEND_FAILED(new_int))
        return false;

    Nst_LLNode *body_start = c_state.inst_ls->tail;
    inc_loop_id();
    if (!compile_node(node->for_l.body))
        return false;
    Nst_LLNode *body_end = c_state.inst_ls->tail;

    i64 continue_idx = CURR_LEN;
    Nst_Inst *dec_int = new_inst(Nst_IC_DEC_INT, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(dec_int))
        return false;
    Nst_Inst *jump_cond = new_inst_i(
        Nst_IC_JUMP,
        cond_idx,
        node->start,
        node->end);
    if (NULL_OR_APPEND_FAILED(jump_cond))
        return false;
    i64 exit_idx = CURR_LEN;
    Nst_Inst *pop_val = new_inst(Nst_IC_POP_VAL, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(pop_val))
        return false;

    jumpif_zero_exit->int_val = exit_idx;

    replace_placeholder_jumps(body_start, body_end, continue_idx, exit_idx);
    dec_loop_id();
    return true;
}

static bool compile_for_as_l(Nst_Node *node)
{
    /*
    For-as loop bytecode

          [ITERATOR CODE]
          FOR_START
          POP_VAL
    cond: FOR_GET_VAL
          JUMPIF_IEND exit
          [ASSIGN_CODE name]
          POP_VAL
          [BODY CODE]
          JUMP cond
    exit: POP_VAL
          [CODE CONTINUATION]
    */

    if (!compile_node(node->for_l.iterator))
        return false;

    Nst_Inst *for_start = new_inst(
        Nst_IC_FOR_START,
        node->for_l.iterator->start,
        node->for_l.iterator->end);
    if (NULL_OR_APPEND_FAILED(for_start))
        return false;
    Nst_Inst *pop_val = new_inst(Nst_IC_POP_VAL, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(pop_val))
        return false;

    i64 cond_idx = CURR_LEN;
    Nst_Inst *for_get_val = new_inst(
        Nst_IC_FOR_GET_VAL,
        node->for_l.iterator->start,
        node->for_l.iterator->end);
    if (NULL_OR_APPEND_FAILED(for_get_val))
        return false;
    Nst_Inst *jumpif_iend_exit = new_inst(
        Nst_IC_JUMPIF_IEND,
        node->for_l.iterator->start,
        node->for_l.iterator->end);
    if (NULL_OR_APPEND_FAILED(jumpif_iend_exit))
        return false;
    if (!compile_unpacking_assign_e(node->for_l.assignment))
        return false;
    pop_val = new_inst(Nst_IC_POP_VAL, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(pop_val))
        return false;

    Nst_LLNode *body_start = c_state.inst_ls->head;
    inc_loop_id();
    if (!compile_node(node->for_l.body))
        return false;
    Nst_LLNode *body_end = c_state.inst_ls->tail;

    Nst_Inst *jump_cond = new_inst_i(
        Nst_IC_JUMP,
        cond_idx,
        node->start,
        node->end);
    if (NULL_OR_APPEND_FAILED(jump_cond))
        return false;

    i64 exit_idx = CURR_LEN;
    pop_val = new_inst(Nst_IC_POP_VAL, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(pop_val))
        return false;

    jumpif_iend_exit->int_val = exit_idx;
    replace_placeholder_jumps(body_start, body_end, cond_idx, exit_idx);
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

    if (!compile_node(node->if_e.condition))
        return false;

    Nst_Inst *jumpif_f_else = new_inst(
        Nst_IC_JUMPIF_F,
        node->start,
        node->end);
    if (NULL_OR_APPEND_FAILED(jumpif_f_else))
        return false;

    if (!compile_node(node->if_e.body_if_true))
        return false;

    Nst_Inst *push_val_null = NULL;
    Nst_Inst *jump_exit = NULL;

    if (node->if_e.body_if_false == NULL) {
        if (Nst_NODE_RETUNS_VALUE(node->if_e.body_if_true)) {
            jump_exit = new_inst(Nst_IC_JUMP, node->start, node->end);
            if (NULL_OR_APPEND_FAILED(jump_exit))
                return false;
        }
        jumpif_f_else->int_val = CURR_LEN;
        push_val_null = new_inst_v(
            Nst_IC_PUSH_VAL,
            Nst_c.Null_null,
            node->start,
            node->end);
        if (NULL_OR_APPEND_FAILED(push_val_null))
            return false;
        if (jump_exit != NULL)
            jump_exit->int_val = CURR_LEN;
        return true;
    }

    bool both_statements = !Nst_NODE_RETUNS_VALUE(node->if_e.body_if_true)
                        && !Nst_NODE_RETUNS_VALUE(node->if_e.body_if_false);

    if (!both_statements) {
        push_val_null = new_inst_v(
            Nst_IC_PUSH_VAL,
            Nst_c.Null_null,
            node->start,
            node->end);
        if (NULL_OR_APPEND_FAILED(push_val_null))
            return false;
    }

    jump_exit = new_inst(Nst_IC_JUMP, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(jump_exit))
        return false;
    jumpif_f_else->int_val = CURR_LEN;

    if (!compile_node(node->if_e.body_if_false))
        return false;

    if (!both_statements) {
        push_val_null = new_inst_v(
            Nst_IC_PUSH_VAL,
            Nst_c.Null_null,
            node->start,
            node->end);
        if (NULL_OR_APPEND_FAILED(push_val_null))
            return false;
    }
    jump_exit->int_val = CURR_LEN;
    if (both_statements) {
        push_val_null = new_inst_v(
            Nst_IC_PUSH_VAL,
            Nst_c.Null_null,
            node->start,
            node->end);
        if (NULL_OR_APPEND_FAILED(push_val_null))
            return false;
    }

    return true;
}

static bool compile_func_declr(Nst_Node *node)
{
    /*
    Func declaration bytecode

    PUSH_VAL function
    SET_VAL_LOC name
    */

    i32 prev_loop_id = c_state.loop_id;
    Nst_LList *prev_inst_ls = c_state.inst_ls;
    Nst_InstList *inst_list = compile_internal(
        node->func_declr.body,
        CT_FUNCTION);
    c_state.inst_ls = prev_inst_ls;
    c_state.loop_id = prev_loop_id;

    Nst_Obj *func = Nst_func_new(
        node->func_declr.argument_names->len,
        inst_list);
    if (func == NULL) {
        Nst_error_add_positions(Nst_error_get(), node->start, node->end);
        return false;
    }
    usize i = 0;
    for (Nst_LLIST_ITER(lnode, node->func_declr.argument_names))
        FUNC(func)->args[i++] = Nst_inc_ref(Nst_TOK(lnode->value)->value);
    Nst_Inst *push_val_func = new_inst_v(
        Nst_IC_PUSH_VAL,
        func,
        node->start,
        node->end);
    Nst_dec_ref(func);
    if (NULL_OR_APPEND_FAILED(push_val_func))
        return false;

    // if the function is a lambda it does not have a name to be set
    if (node->func_declr.name == NULL)
        return true;

    Nst_Inst *set_val_loc = new_inst_v(
        Nst_IC_SET_VAL_LOC,
        node->func_declr.name,
        node->start,
        node->end);
    if (NULL_OR_APPEND_FAILED(push_val_func))
        return false;
    return true;
}

static bool compile_return_s(Nst_Node *node)
{
    /*
    Return bytecode

    [EXPR CODE]
    RETURN_VAL

    Pops all values from the stack until it finds a nullptr
    */
    if (node->return_s.value == NULL) {
        Nst_Inst *push_val_null = new_inst_v(
            Nst_IC_PUSH_VAL,
            Nst_c.Null_null,
            node->start,
            node->end);
        if (NULL_OR_APPEND_FAILED(push_val_null))
            return false;
    } else if (!compile_node(node->return_s.value))
        return false;

    Nst_Inst *return_val = Nst_inst_new(
        Nst_IC_RETURN_VAL,
        node->start,
        node->end);
    return !NULL_OR_APPEND_FAILED(return_val);
}

static bool compile_stack_op(Nst_Node *node)
{
    /*
    Stack operation bytecode

    [VAL 1 CODE]
    [VAL 2 CODE]
    OP_STACK - operator
    */

    if (!compile_node(node->stack_op.values->head))
        return false;

    if (node->stack_op.values->len == 1)
        return true;

    if (Nst_IS_COMP_OP(node->stack_op.op) && node->stack_op.values->len > 2)
        return compile_comp_op(node);

    if (Nst_IS_COND_OP(node->stack_op.op))
        return compile_lg_op(node);

    for (Nst_LLNode *lnode = node->stack_op.values->head->next;
         lnode != NULL;
         lnode = lnode->next)
    {
        if (!compile_node(Nst_NODE(lnode->value)))
            return false;
        Nst_Inst *inst = new_inst_i(
            Nst_IC_STACK_OP,
            node->stack_op.op,
            node->start,
            node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            return false;
    }
    return true;
}

static bool compile_comp_op(Nst_Node *node)
{
    /*
    Comparison operator bytecode operating on N values with N > 2

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

    Nst_Inst *inst;
    Nst_TokType op = node->stack_op.op;

    Nst_LList *jumpif_f_fix_list = Nst_llist_new();
    if (jumpif_f_fix_list == NULL) {
        Nst_error_add_positions(Nst_error_get(), node->start, node->end);
        goto failure;
    }

    for (Nst_LLNode *lnode = node->stack_op.values->head->next;
         lnode->next != NULL; // safe because the list is always >2 long
         lnode = lnode->next)
    {
        if (!compile_node(Nst_NODE(lnode->value)))
            goto failure;
        inst = new_inst(Nst_IC_DUP, node->start, node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            goto failure;
        inst = new_inst_i(Nst_IC_ROT, 3, node->start, node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            goto failure;
        inst = new_inst_i(Nst_IC_STACK_OP, op, node->start, node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            goto failure;
        inst = new_inst(Nst_IC_DUP, node->start, node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            goto failure;
        inst = new_inst(Nst_IC_JUMPIF_F, node->start, node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            goto failure;
        if (!Nst_llist_append(jumpif_f_fix_list, inst, false)) {
            Nst_error_add_positions(Nst_error_get(), node->start, node->end);
            goto failure;
        }
        inst = new_inst(Nst_IC_POP_VAL, node->start, node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            goto failure;
    }

    if (!compile_node(Nst_NODE(node->stack_op.values->tail)))
        goto failure;

    inst = new_inst_i(Nst_IC_STACK_OP, op, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(inst))
        goto failure;
    inst = new_inst_i(Nst_IC_JUMP, CURR_LEN + 3, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(inst))
        goto failure;

    for (Nst_LLIST_ITER(lnode, jumpif_f_fix_list))
        ((Nst_Inst*)lnode->value)->int_val = CURR_LEN;
    Nst_llist_destroy(jumpif_f_fix_list, NULL);
    jumpif_f_fix_list = NULL;

    inst = new_inst_i(Nst_IC_ROT, 2, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(inst))
        return false;
    inst = new_inst(Nst_IC_POP_VAL, node->start, node->end);
    if (NULL_OR_APPEND_FAILED(inst))
        return false;
    return true;

failure:
    if (jumpif_f_fix_list != NULL)
        Nst_llist_destroy(jumpif_f_fix_list, NULL);
    return false;
}

static bool compile_lg_op(Nst_Node *node)
{
    /*
    L_AND bytecode

         [VAL 1 CODE]
         DUP          -+
         JUMPIF_F end  | Repeated for all values
         POP_VAL       |
         [VAL 2 CODE] -+
    end: [CODE CONTINUATION]

    L_OR bytecode

         [VAL 1 CODE]
         DUP          -+
         JUMPIF_T end  | Repeated for all values
         POP_VAL       |
         [VAL 2 CODE] -+
    end: [CODE CONTINUATION]
    */

    Nst_LList *jumpif_end = Nst_llist_new();
    if (jumpif_end == NULL) {
        Nst_error_add_positions(Nst_error_get(), node->start, node->end);
        goto failure;
    }

    Nst_InstID jump_id = node->stack_op.op == Nst_TT_L_OR
        ? Nst_IC_JUMPIF_T
        : Nst_IC_JUMPIF_F;

    Nst_Inst *inst;
    for (Nst_LLNode *lnode = node->stack_op.values->head->next;
         lnode != NULL;
         lnode = lnode->next)
    {
        inst = new_inst(Nst_IC_DUP, node->start, node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            goto failure;

        inst = new_inst(jump_id, node->start, node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            goto failure;
        if (!Nst_llist_append(jumpif_end, inst, false)) {
            Nst_error_add_positions(Nst_error_get(), node->start, node->end);
            goto failure;
        }

        inst = new_inst(Nst_IC_POP_VAL, node->start, node->end);
        if (NULL_OR_APPEND_FAILED(inst))
            goto failure;

        if (!compile_node(Nst_NODE(lnode->value)))
            goto failure;
    }

    for (Nst_LLIST_ITER(lnode, jumpif_end))
        ((Nst_Inst *)lnode->value)->int_val = CURR_LEN;
    Nst_llist_destroy(jumpif_end, NULL);
    return true;

failure:
    if (jumpif_end != NULL)
        Nst_llist_destroy(jumpif_end, NULL);
    return false;
}

static bool compile_local_stack_op(Nst_Node *node)
{
    /*
    Range operation bytecode

    [VALUE CODE]
    TYPE_CHECK Int
    [VALUE CODE]
    TYPE_CHECK Int
    ([VALUE CODE]
     TYPE_CHECK Int)
    OP_RANGE - arg num

    Cast operation bytecode

    [VALUE CODE]
    TYPE_CHECK Type
    [VALUE CODE]
    OP_CAST

    Call operation bytecode

    PUSH_VAL nullptr
    [ARG CODE] *
    [FUNC CODE]
    TYPE_CHECK Func
    OP_CALL - arg num or -1 for SEQ_CALL
    */

    i32 tok_type = HEAD_TOK->type;
    Nst_Inst *inst;

    if (tok_type == Nst_TT_RANGE) {
        compile_node(HEAD_NODE);
        EXCEPT_ERROR();
        inst = Nst_inst_new_val(
            Nst_IC_TYPE_CHECK,
            Nst_t.Int,
            HEAD_NODE->start,
            HEAD_NODE->end);
        ADD_INST(inst);

        if (node->nodes->len == 3) {
            compile_node(SECOND_NODE);
            EXCEPT_ERROR();
            inst = Nst_inst_new_val(
                Nst_IC_TYPE_CHECK,
                Nst_t.Int,
                HEAD_NODE->start,
                HEAD_NODE->end);
            ADD_INST(inst);
        }

        compile_node(TAIL_NODE);
        EXCEPT_ERROR();
        inst = Nst_inst_new_val(
            Nst_IC_TYPE_CHECK,
            Nst_t.Int,
            HEAD_NODE->start,
            HEAD_NODE->end);
        ADD_INST(inst);

        inst = Nst_inst_new_int(
            Nst_IC_OP_RANGE,
            node->nodes->len,
            node->start,
            node->end);
        ADD_INST(inst);
    } else if (tok_type == Nst_TT_CAST) {
        compile_node(HEAD_NODE);
        EXCEPT_ERROR();
        inst = Nst_inst_new_val(
            Nst_IC_TYPE_CHECK,
            Nst_t.Type,
            HEAD_NODE->start,
            HEAD_NODE->end);
        ADD_INST(inst);

        compile_node(TAIL_NODE);
        EXCEPT_ERROR();
        inst = Nst_inst_new(
            Nst_IC_OP_CAST,
            node->start, node->end);
        ADD_INST(inst);
    } else if (tok_type == Nst_TT_CALL || tok_type == Nst_TT_SEQ_CALL) {
        for (Nst_LLIST_ITER(n, node->nodes)) {
            compile_node(Nst_NODE(n->value));
            EXCEPT_ERROR();
        }
        inst = Nst_inst_new_val(
            Nst_IC_TYPE_CHECK,
            Nst_t.Func,
            node->start,
            node->end);
        ADD_INST(inst);
        inst = Nst_inst_new_int(
            Nst_IC_OP_CALL,
            tok_type == Nst_TT_CALL ? (i32)(node->nodes->len - 1) : -1,
            node->start, node->end);
        ADD_INST(inst);
        // The function object is popped by the RETURN_VAL instruction
    } else if (tok_type == Nst_TT_THROW) {
        compile_node(HEAD_NODE);
        EXCEPT_ERROR();
        inst = Nst_inst_new_val(
            Nst_IC_TYPE_CHECK,
            Nst_t.Str,
            HEAD_NODE->start,
            HEAD_NODE->end);
        ADD_INST(inst);

        compile_node(TAIL_NODE);
        EXCEPT_ERROR();
        inst = Nst_inst_new_val(
            Nst_IC_TYPE_CHECK,
            Nst_t.Str,
            TAIL_NODE->start,
            TAIL_NODE->end);
        ADD_INST(inst);

        inst = Nst_inst_new(
            Nst_IC_THROW_ERR,
            node->start, node->end);
        ADD_INST(inst);
    }
}

static bool compile_local_op(Nst_Node *node)
{
    /*
    Import bytecode

    [LIB NAME CODE]
    TYPE_CHECK Str
    OP_IMPORT

    Local call bytecode

    PUSH_VAL nullptr
    [ARG CODE]
    [FUNC CODE]
    TYPE_CHECK Func
    OP_CALL - arg num

    Other operations bytecode

    [VALUE CODE]
    OP_LOCAL - operator
    */

    Nst_Inst *inst;

    if (HEAD_TOK->type == Nst_TT_LOC_CALL) {
        compile_node(HEAD_NODE);
        EXCEPT_ERROR();
        inst = Nst_inst_new_val(
            Nst_IC_TYPE_CHECK,
            Nst_t.Func,
            node->start,
            node->end);
        ADD_INST(inst);
        inst = Nst_inst_new_int(
            Nst_IC_OP_CALL,
            node->nodes->len - 1,
            node->start,
            node->end);
        ADD_INST(inst);
        return;
    }

    compile_node(HEAD_NODE);
    EXCEPT_ERROR();

    if (HEAD_TOK->type == Nst_TT_IMPORT) {
        inst = Nst_inst_new_val(
            Nst_IC_TYPE_CHECK,
            Nst_t.Str,
            node->start,
            node->end);
        ADD_INST(inst);
        inst = Nst_inst_new_int(
            Nst_IC_OP_IMPORT, 0,
            node->start,
            node->end);
        ADD_INST(inst);
        return;
    }

    inst = Nst_inst_new_int(
        Nst_IC_LOCAL_OP,
        HEAD_TOK->type,
        HEAD_NODE->start,
        HEAD_NODE->end);

    ADD_INST(inst);
}

static bool compile_seq_lit(Nst_Node *node)
{
    /*
    Vector or array literal bytecode

    [VALUE CODE] *
    MAKE_ARR | MAKE_VEC - number of elements

    [VALUE CODE]
    [TIMES TO REPEAT CODE]
    TYPE_CHECK Int
    MAKE_ARR_REP | MAKE_VEC_REP
    */

    Nst_Inst *inst;
    for (Nst_LLIST_ITER(n, node->nodes)) {
        compile_node(Nst_NODE(n->value));
        EXCEPT_ERROR();
    }

    if (node->tokens->len != 0) {
        inst = Nst_inst_new_val(
            Nst_IC_TYPE_CHECK,
            Nst_t.Int,
            TAIL_NODE->start,
            TAIL_NODE->end);
        ADD_INST(inst);
        inst = Nst_inst_new_int(
            node->type == Nst_NT_ARR_LIT ? Nst_IC_MAKE_ARR_REP
                                         : Nst_IC_MAKE_VEC_REP,
            node->nodes->len, node->start, node->end);
    } else {
        inst = Nst_inst_new_int(
            node->type == Nst_NT_ARR_LIT ? Nst_IC_MAKE_ARR : Nst_IC_MAKE_VEC,
            node->nodes->len, node->start, node->end);
    }

    ADD_INST(inst);
}

static bool compile_map_lit(Nst_Node *node)
{
    /*
    Map literal bytecode

    ( [VALUE CODE]
      HASH_CHECK )*
    MAKE_MAP - number of elements (key-value pairs * 2)
    */
    Nst_Inst *inst;

    bool is_key = true;
    for (Nst_LLIST_ITER(n, node->nodes)) {
        compile_node(Nst_NODE(n->value));
        EXCEPT_ERROR();

        if (is_key) {
            inst = Nst_inst_new_int(
                Nst_IC_HASH_CHECK,
                0,
                Nst_NODE(n->value)->start,
                Nst_NODE(n->value)->end);
            ADD_INST(inst);
        }
        is_key = !is_key;
    }

    inst = Nst_inst_new_int(
        Nst_IC_MAKE_MAP,
        node->nodes->len,
        node->start, node->end);
    ADD_INST(inst);
}

static bool compile_value(Nst_Node *node)
{
    /*
    Value bytecode

    PUSH_VAL value
    */
    Nst_Inst *push_val_value = new_inst_v(
        Nst_IC_PUSH_VAL,
        node->value.value->value,
        node->start,
        node->end);
    return !NULL_OR_APPEND_FAILED(push_val_value);
}

static bool compile_access(Nst_Node *node)
{
    /*
    Access bytecode

    GET_VAL name
    */
    Nst_Inst *get_val_name = new_inst_v(
        Nst_IC_GET_VAL,
        node->access.value->value,
        node->start,
        node->end);
    return !NULL_OR_APPEND_FAILED(get_val_name);
}

static bool compile_extract_e(Nst_Node *node)
{
    /*
    Extraction bytecode

    [CONTAINER CODE]
    [INDEX_CODE]
    OP_EXTRACT
    */
    if (!compile_node(node->extract_e.container))
        return false;
    if (!compile_node(node->extract_e.key))
        return false;

    Nst_Inst *op_extarct = new_inst(Nst_IC_OP_EXTRACT, node->start, node->end);
    return !NULL_OR_APPEND_FAILED(op_extarct);
}

static bool compile_assign_e(Nst_Node *node)
{
    /*
    Assignment bytecode for variable name

    [VALUE CODE]
    SET_VAL name

    Assignment bytecode for container

    [VALUE CODE]
    [CONTAINER CODE]
    [INDEX_CODE]
    SET_CONTAINER_VAL
    */
    Nst_Inst *inst;
    compile_node(HEAD_NODE);
    EXCEPT_ERROR();

    if (TAIL_NODE->type == Nst_NT_ACCESS) {
        inst = Nst_inst_new_val(
            Nst_IC_SET_VAL,
            Nst_TOK(TAIL_NODE->tokens->head->value)->value,
            node->start,
            node->end);
        ADD_INST(inst);
    } else if (TAIL_NODE->type == Nst_NT_EXTRACT_E) {
        compile_node(Nst_NODE(TAIL_NODE->nodes->head->value)); // Container
        EXCEPT_ERROR();
        compile_node(Nst_NODE(TAIL_NODE->nodes->tail->value)); // Index
        EXCEPT_ERROR();

        inst = Nst_inst_new(
            Nst_IC_SET_CONT_VAL,
            node->start,
            node->end);
        ADD_INST(inst);
    } else {
        inst = Nst_inst_new(Nst_IC_DUP, node->start, node->end);
        ADD_INST(inst);
        compile_unpacking_assign_e(TAIL_NODE, HEAD_NODE->start, HEAD_NODE->end);
    }
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

    Nst_Inst *inst;
    Nst_LList *nodes = Nst_llist_new();
    ADD_POS_AND_QUIT_IF_ERR();
    Nst_llist_push(nodes, node, false);
    ADD_POS_AND_QUIT_IF_ERR();

    while (nodes->len != 0) {
        Nst_Node *curr_node = Nst_NODE(Nst_llist_pop(nodes));

        if (curr_node->type == Nst_NT_ARR_LIT) {
            // insert the array's nodes before any nodes inside the list
            Nst_LLNode *list_head = nodes->head;
            Nst_LLNode *list_tail = nodes->tail;
            nodes->head = NULL;
            nodes->tail = NULL;

            for (Nst_LLIST_ITER(n, curr_node->nodes)) {
                Nst_llist_append(nodes, n->value, false);
                ADD_POS_AND_QUIT_IF_ERR(Nst_llist_destroy(nodes, NULL););
            }

            // the array literal is guaranteed to have at least one item
            nodes->tail->next = list_head;
            nodes->tail = list_tail;

            inst = Nst_inst_new_int(
                Nst_IC_UNPACK_SEQ,
                curr_node->nodes->len,
                v_start,
                v_end);
        } else if (curr_node->type == Nst_NT_ACCESS) {
            inst = Nst_inst_new_val(
                Nst_IC_SET_VAL_LOC,
                Nst_TOK(curr_node->tokens->head->value)->value,
                curr_node->start,
                curr_node->end);
        } else {
            compile_node(Nst_NODE(curr_node->nodes->head->value)); // Container
            EXCEPT_ERROR(Nst_llist_destroy(nodes, NULL););
            compile_node(Nst_NODE(curr_node->nodes->tail->value)); // Index
            EXCEPT_ERROR(Nst_llist_destroy(nodes, NULL););

            inst = Nst_inst_new(
                Nst_IC_SET_CONT_LOC,
                curr_node->start,
                curr_node->end);
        }
        ADD_INST(inst, Nst_llist_destroy(nodes, NULL););
    }
    Nst_llist_destroy(nodes, NULL);
}

static bool compile_continue_s(Nst_Node *node)
{
    /*
    Continue bytecode

    JUMP loop_id - replaced later with the loop's start
    */
    Nst_Inst *jump_loop_id = new_inst_i(
        Nst_IC_JUMP,
        c_state.loop_id,
        node->start,
        node->end);

    return !NULL_OR_APPEND_FAILED(jump_loop_id);
}

static bool compile_break_s(Nst_Node *node)
{
    /*
    Continue bytecode

    JUMP loop_id - replaced later with the loop's end
    */
    Nst_Inst *jump_loop_id = new_inst_i(
        Nst_IC_JUMP,
        c_state.loop_id - 1,
        node->start,
        node->end);
    return !NULL_OR_APPEND_FAILED(jump_loop_id);
}

static bool compile_switch_s(Nst_Node *node)
{
    /*
    Switch statement bytecode

                [EXPRESSION CODE]
                DUP
                [VALUE1 CODE]
                OP_STACK ==
                JUMPIF_F body1_end
                [BODY CODE]
                JUMP switch_end
    body1_end:  DUP
                [VALUE2 CODE]
                OP_STACK ==
                JUMPIF_F body2_end
                [BODY CODE]
                JUMP switch_end
    body2_end:  [DEFAULT CODE]
    switch_end: POP_VAL
                [CODE CONTINUATION]
    */

    Nst_Inst *inst;
    Nst_LLNode *prev_body_start = NULL;
    Nst_LLNode *prev_body_end = NULL;
    Nst_Node *default_body = NULL;
    Nst_LList *jumps_to_switch_end = Nst_llist_new();
    ADD_POS_AND_QUIT_IF_ERR();

    compile_node(HEAD_NODE);
    EXCEPT_ERROR(Nst_llist_destroy(jumps_to_switch_end, NULL););

    for (Nst_LLNode *n = node->nodes->head->next; n != NULL; n = n->next) {
        if (n->next == NULL) {
            default_body = Nst_NODE(n->value);
            break;
        }

        inst = Nst_inst_new(Nst_IC_DUP, node->start, node->end);
        ADD_INST(inst, Nst_llist_destroy(jumps_to_switch_end, NULL););
        compile_node(Nst_NODE(n->value));
        EXCEPT_ERROR(Nst_llist_destroy(jumps_to_switch_end, NULL););
        n = n->next;
        inst = Nst_inst_new_int(
            Nst_IC_STACK_OP,
            Nst_TT_EQ,
            node->start,
            node->end);
        ADD_INST(inst, Nst_llist_destroy(jumps_to_switch_end, NULL););
        Nst_Inst *jump_body_end = Nst_inst_new(
            Nst_IC_JUMPIF_F,
            node->start,
            node->end);
        ADD_INST(jump_body_end, Nst_llist_destroy(jumps_to_switch_end, NULL););
        Nst_LLNode *body_start = c_state.inst_ls->tail;

        inc_loop_id();
        i64 next_body_start = CURR_LEN;
        compile_node(Nst_NODE(n->value));
        EXCEPT_ERROR(Nst_llist_destroy(jumps_to_switch_end, NULL););
        inst = Nst_inst_new(Nst_IC_JUMP, node->start, node->end);
        ADD_INST(inst, Nst_llist_destroy(jumps_to_switch_end, NULL););
        Nst_llist_append(jumps_to_switch_end, inst, false);
        ADD_POS_AND_QUIT_IF_ERR(Nst_llist_destroy(jumps_to_switch_end, NULL););
        jump_body_end->int_val = CURR_LEN;
        Nst_LLNode *body_end = c_state.inst_ls->tail;

        if (prev_body_start != NULL) {
            for (Nst_LLNode *cursor = prev_body_start->next;
                 cursor != NULL && cursor != prev_body_end;
                 cursor = cursor->next)
            {
                inst = INST(cursor->value);

                if (Nst_INST_IS_JUMP(inst->id)
                    && inst->int_val == c_state.loop_id)
                {
                    inst->int_val = next_body_start;
                }
            }
        }

        dec_loop_id();
        prev_body_start = body_start;
        prev_body_end = body_end;
    }

    if (prev_body_start != NULL) {
        inc_loop_id();
        for (Nst_LLNode *cursor = prev_body_start->next;
             cursor != NULL && cursor != prev_body_end;
             cursor = cursor->next)
        {
            inst = INST(cursor->value);

            if (Nst_INST_IS_JUMP(inst->id) && inst->int_val == c_state.loop_id)
                inst->int_val = CURR_LEN;
        }
        dec_loop_id();
    }

    if (default_body != NULL) {
        inc_loop_id();
        Nst_LLNode *default_body_start = c_state.inst_ls->tail;
        compile_node(default_body);
        EXCEPT_ERROR(Nst_llist_destroy(jumps_to_switch_end, NULL););

        for (Nst_LLNode *cursor = default_body_start->next;
             cursor != NULL;
             cursor = cursor->next)
        {
            inst = INST(cursor->value);

            if (Nst_INST_IS_JUMP(inst->id) && inst->int_val == c_state.loop_id)
                inst->int_val = CURR_LEN;
        }

        dec_loop_id();
    }

    for (Nst_LLNode *cursor = jumps_to_switch_end->head;
         cursor != NULL;
         cursor = cursor->next)
    {
        INST(cursor->value)->int_val = CURR_LEN;
    }

    inst = Nst_inst_new(Nst_IC_POP_VAL, node->start, node->end);
    ADD_INST(inst, Nst_llist_destroy(jumps_to_switch_end, NULL););
    Nst_llist_destroy(jumps_to_switch_end, NULL);
}

static bool compile_try_catch_s(Nst_Node* node)
{
    /*
    Switch statement bytecode

                 PUSH_CATCH catch_start
                 [TRY BLOCK CODE]
                 POP_CATCH
                 JUMP catch_end
    catch_start: SAVE_ERROR
                 POP_CATCH
                 SET_VAL_LOC err_name
                 [CATCH BLOCK CODE]
    catch_end:   [CODE_CONTINUATION]
    */

    Nst_Inst *inst;
    Nst_Inst *push_catch = Nst_inst_new(
        Nst_IC_PUSH_CATCH,
        node->start,
        node->end);
    ADD_INST(push_catch);
    compile_node(HEAD_NODE);
    EXCEPT_ERROR();

    if (Nst_NODE_RETUNS_VALUE(HEAD_NODE->type)) {
        inst = Nst_inst_new(Nst_IC_POP_VAL, node->start, node->end);
        ADD_INST(inst);
    }

    inst = Nst_inst_new(Nst_IC_POP_CATCH, node->start, node->end);
    ADD_INST(inst);

    // The code inside this jump will never be optimized since
    // Nst_IC_PUSH_CATCH, that is considered a jump, refers to
    // Nst_IC_SAVE_ERROR that is inside the block
    Nst_Inst *jump_catch_end = Nst_inst_new(
        Nst_IC_JUMP,
        node->start,
        node->end);
    ADD_INST(jump_catch_end);
    push_catch->int_val = CURR_LEN;

    inst = Nst_inst_new(Nst_IC_SAVE_ERROR, node->start, node->end);
    ADD_INST(inst);
    inst = Nst_inst_new(Nst_IC_POP_CATCH, node->start, node->end);
    ADD_INST(inst);
    inst = Nst_inst_new_val(
        Nst_IC_SET_VAL_LOC,
        HEAD_TOK->value,
        HEAD_TOK->start,
        HEAD_TOK->end);
    ADD_INST(inst);
    compile_node(TAIL_NODE);
    EXCEPT_ERROR();
    if (Nst_NODE_RETUNS_VALUE(TAIL_NODE->type)) {
        inst = Nst_inst_new(Nst_IC_POP_VAL, node->start, node->end);
        ADD_INST(inst);
    }
    jump_catch_end->int_val = CURR_LEN;
}

static void print_bytecode(Nst_InstList *ls, i32 indent)
{
    usize tot_size = ls->total_size;
    int idx_width = 1;
    int max_col = 0;
    int max_row = 0;
    i64 max_int = 0;
    int col_width = 1;
    int row_width = 1;
    int int_width = 1;

    for (usize i = 0, n = ls->total_size; i < n; i++) {
        Nst_Inst inst = ls->instructions[i];
        if (inst.int_val > max_int)
            max_int = inst.int_val;
        if (inst.start.col > max_col)
            max_col = inst.start.col;
        if (inst.start.line > max_row)
            max_row = inst.start.line;
    }

    while (tot_size >= 10) {
        tot_size /= 10;
        ++idx_width;
    }
    while (max_col >= 10) {
        max_col /= 10;
        ++col_width;
    }
    while (max_row >= 10) {
        max_row /= 10;
        ++row_width;
    }
    while (max_int >= 10) {
        max_int /= 10;
        ++int_width;
    }

    idx_width = idx_width < 3 ? 3 : idx_width;
    col_width = col_width < 3 ? 3 : col_width;
    row_width = row_width < 3 ? 3 : row_width;
    int_width = int_width < 3 ? 3 : int_width;

    for (i32 i = 0; i < indent; i++)
        PRINT("    ", 4);
    for (i32 i = 3; i < idx_width; i++)
        PRINT(" ", 1);
    PRINT(" Idx |", 6);
    for (i32 i = 3; i < row_width; i++)
        PRINT(" ", 1);
    PRINT("   Pos   ", 9);
    for (i32 i = 3; i < col_width; i++)
        PRINT(" ", 1);
    PRINT("|  Instruction  |", 17);
    for (i32 i = 3; i < int_width; i++)
        PRINT(" ", 1);
    PRINT(" Int | Object\n", 14);

    for (usize i = 0, n = ls->total_size; i < n; i++) {
        Nst_Inst inst = ls->instructions[i];

        for (i32 j = 0; j < indent; j++)
            Nst_print("    ");
        Nst_printf(
            " %*zi | %*li:%-*li | ",
            idx_width, i,
            row_width,
            inst.start.line + 1,
            col_width,
            inst.start.col + 1);

        switch (inst.id) {
        case Nst_IC_NO_OP:         PRINT("NO_OP        ", 13); break;
        case Nst_IC_POP_VAL:       PRINT("POP_VAL      ", 13); break;
        case Nst_IC_FOR_START:     PRINT("FOR_START    ", 13); break;
        case Nst_IC_RETURN_VAL:    PRINT("RETURN_VAL   ", 13); break;
        case Nst_IC_RETURN_VARS:   PRINT("RETURN_VARS  ", 13); break;
        case Nst_IC_SET_VAL_LOC:   PRINT("SET_VAL_LOC  ", 13); break;
        case Nst_IC_SET_CONT_LOC:  PRINT("SET_CONT_LOC ", 13); break;
        case Nst_IC_JUMP:          PRINT("JUMP         ", 13); break;
        case Nst_IC_JUMPIF_T:      PRINT("JUMPIF_T     ", 13); break;
        case Nst_IC_JUMPIF_F:      PRINT("JUMPIF_F     ", 13); break;
        case Nst_IC_JUMPIF_ZERO:   PRINT("JUMPIF_ZERO  ", 13); break;
        case Nst_IC_TYPE_CHECK:    PRINT("TYPE_CHECK   ", 13); break;
        case Nst_IC_HASH_CHECK:    PRINT("HASH_CHECK   ", 13); break;
        case Nst_IC_THROW_ERR:     PRINT("THROW_ERR    ", 13); break;
        case Nst_IC_PUSH_CATCH:    PRINT("PUSH_CATCH   ", 13); break;
        case Nst_IC_POP_CATCH:     PRINT("POP_CATCH    ", 13); break;
        case Nst_IC_SET_VAL:       PRINT("SET_VAL      ", 13); break;
        case Nst_IC_GET_VAL:       PRINT("GET_VAL      ", 13); break;
        case Nst_IC_PUSH_VAL:      PRINT("PUSH_VAL     ", 13); break;
        case Nst_IC_SET_CONT_VAL:  PRINT("SET_CONT_VAL ", 13); break;
        case Nst_IC_OP_CALL:       PRINT("OP_CALL      ", 13); break;
        case Nst_IC_OP_CAST:       PRINT("OP_CAST      ", 13); break;
        case Nst_IC_OP_RANGE:      PRINT("OP_RANGE     ", 13); break;
        case Nst_IC_STACK_OP:      PRINT("STACK_OP     ", 13); break;
        case Nst_IC_LOCAL_OP:      PRINT("LOCAL_OP     ", 13); break;
        case Nst_IC_OP_IMPORT:     PRINT("OP_IMPORT    ", 13); break;
        case Nst_IC_OP_EXTRACT:    PRINT("OP_EXTRACT   ", 13); break;
        case Nst_IC_DEC_INT:       PRINT("DEC_INT      ", 13); break;
        case Nst_IC_NEW_INT:       PRINT("NEW_INT      ", 13); break;
        case Nst_IC_DUP:           PRINT("DUP          ", 13); break;
        case Nst_IC_ROT:           PRINT("ROT          ", 13); break;
        case Nst_IC_MAKE_ARR:      PRINT("MAKE_ARR     ", 13); break;
        case Nst_IC_MAKE_ARR_REP:  PRINT("MAKE_ARR_REP ", 13); break;
        case Nst_IC_MAKE_VEC:      PRINT("MAKE_VEC     ", 13); break;
        case Nst_IC_MAKE_VEC_REP:  PRINT("MAKE_VEC_REP ", 13); break;
        case Nst_IC_MAKE_MAP:      PRINT("MAKE_MAP     ", 13); break;
        case Nst_IC_JUMPIF_IEND:   PRINT("FOR_IS_DONE  ", 13); break;
        case Nst_IC_FOR_GET_VAL:   PRINT("FOR_GET_VAL  ", 13); break;
        case Nst_IC_SAVE_ERROR:    PRINT("SAVE_ERROR   ", 13); break;
        case Nst_IC_UNPACK_SEQ:    PRINT("UNPACK_SEQ   ", 13); break;
        default:                   PRINT("__UNKNOWN__  ", 13); break;
        }

        if (inst.id == Nst_IC_NO_OP) {
            PRINT(" | ", 3);
            for (i32 j = 0; j < idx_width; j++)
                PRINT(" ", 1);
            PRINT(" |", 2);
            PRINT("\n", 1);
            continue;
        }

        if (Nst_INST_IS_JUMP(inst.id) || inst.id == Nst_IC_LOCAL_OP
            || inst.id == Nst_IC_STACK_OP || inst.id == Nst_IC_MAKE_ARR
            || inst.id == Nst_IC_MAKE_VEC || inst.id == Nst_IC_MAKE_MAP
            || inst.int_val != 0)
        {
            Nst_printf(" | %*lli |", idx_width, inst.int_val);
        } else {
            PRINT(" | ", 3);
            for (i32 j = 0; j < idx_width; j++)
                PRINT(" ", 1);
            PRINT(" |", 2);
        }

        if (inst.val != NULL) {
            Nst_printf(" (%s) ", TYPE_NAME(inst.val));
            Nst_StrObj* s = STR(_Nst_repr_str_cast(inst.val));
            if (Nst_error_occurred())
                Nst_error_clear();
            else {
                Nst_fwrite(s->value, s->len, NULL, Nst_io.out);
                Nst_dec_ref(s);
            }

            if (inst.val->type == Nst_t.Func) {
                PRINT("\n\n", 2);
                for (i32 j = 0; j < indent + 1; j++)
                    PRINT("    ", 4);
                PRINT("<Func object> bytecode:\n", 24);
                print_bytecode(FUNC(inst.val)->body.bytecode, indent + 1);
            }
        } else if (inst.id == Nst_IC_STACK_OP || inst.id == Nst_IC_LOCAL_OP) {
            PRINT(" [", 2);

            switch (inst.int_val) {
            case Nst_TT_ADD:    PRINT("+", 1);  break;
            case Nst_TT_SUB:    PRINT("-", 1);  break;
            case Nst_TT_MUL:    PRINT("*", 1);  break;
            case Nst_TT_DIV:    PRINT("/", 1);  break;
            case Nst_TT_POW:    PRINT("^", 1);  break;
            case Nst_TT_MOD:    PRINT("%", 1);  break;
            case Nst_TT_B_AND:  PRINT("&", 1);  break;
            case Nst_TT_B_OR:   PRINT("|", 1);  break;
            case Nst_TT_LEN:    PRINT("$", 1);  break;
            case Nst_TT_L_NOT:  PRINT("!", 1);  break;
            case Nst_TT_B_NOT:  PRINT("~", 1);  break;
            case Nst_TT_GT:     PRINT(">", 1);  break;
            case Nst_TT_LT:     PRINT("<", 1);  break;
            case Nst_TT_B_XOR:  PRINT("^^", 2); break;
            case Nst_TT_LSHIFT: PRINT("<<", 2); break;
            case Nst_TT_RSHIFT: PRINT(">>", 2); break;
            case Nst_TT_CONCAT: PRINT("><", 2); break;
            case Nst_TT_L_AND:  PRINT("&&", 2); break;
            case Nst_TT_L_OR:   PRINT("||", 2); break;
            case Nst_TT_L_XOR:  PRINT("&|", 2); break;
            case Nst_TT_EQ:     PRINT("==", 2); break;
            case Nst_TT_NEQ:    PRINT("!=", 2); break;
            case Nst_TT_GTE:    PRINT(">=", 2); break;
            case Nst_TT_LTE:    PRINT("<=", 2); break;
            case Nst_TT_NEG:    PRINT("-:", 2); break;
            case Nst_TT_STDOUT: PRINT(">>>", 3);break;
            case Nst_TT_STDIN:  PRINT("<<<", 3);break;
            case Nst_TT_TYPEOF: PRINT("?::", 3);break;
            case Nst_TT_CONTAINS:PRINT("<.>", 3);break;
            default: PRINT("__UNKNOWN_OP__", 14);
            }
            PRINT("]", 1);
        }
        PRINT("\n", 1);
    }
}

void Nst_print_bytecode(Nst_InstList *ls)
{
    print_bytecode(ls, 0);
}
