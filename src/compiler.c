#include <stdlib.h>
#include "compiler.h"
#include "llist.h"
#include "tokens.h"
#include "simple_types.h"
#include "str.h"
#include "function.h"
#include "obj_ops.h" // _nst_repr_str_cast
#include "global_consts.h"

#define HEAD_NODE (NODE(node->nodes->head->value))
#define TAIL_NODE (NODE(node->nodes->tail->value))
#define SECOND_NODE (NODE(node->nodes->head->next->value))
#define HEAD_TOK (TOK(node->tokens->head->value))
#define TAIL_TOK (TOK(node->tokens->tail->value))

#define INST(instruction) ((Nst_Instruction *)(instruction))
#define CURR_LEN ((Nst_Int)(c_state.inst_ls->size))

#define ADD_INST(instruction) LList_append(c_state.inst_ls, instruction, true)

#define PRINT(str, size) fwrite(str, sizeof(char), size, stdout)

typedef struct
{
    int loop_id;
    LList *inst_ls;
}
CompileState;

CompileState c_state;

static void inc_loop_id()
{
    c_state.loop_id -= 2;
}

static void dec_loop_id()
{
    c_state.loop_id += 2;
}

static Nst_InstructionList *compile_internal(Nst_Node *node,
                                             bool      is_func,
                                             bool      is_module);
static void compile_node(Nst_Node *node);
static void compile_long_s(Nst_Node *node);
static void compile_while_l(Nst_Node *node);
static void compile_dowhile_l(Nst_Node *node);
static void compile_for_l(Nst_Node *node);
static void compile_for_as_l(Nst_Node *node);
static void compile_if_e(Nst_Node *node);
static void compile_func_declr(Nst_Node *node);
static void compile_lambda(Nst_Node *node);
static void compile_return_s(Nst_Node *node);
static void compile_stack_op(Nst_Node *node);
static void compile_comp_op(Nst_Node *node);
static void compile_lg_op(Nst_Node *node);
static void compile_local_stack_op(Nst_Node *node);
static void compile_local_op(Nst_Node *node);
static void compile_arr_or_vect_lit(Nst_Node *node);
static void compile_map_lit(Nst_Node *node);
static void compile_value(Nst_Node *node);
static void compile_access(Nst_Node *node);
static void compile_extract_e(Nst_Node *node);
static void compile_assign_e(Nst_Node *node);
static void compile_unpacking_assign_e(Nst_Node *node,
                                       Nst_Pos   v_start,
                                       Nst_Pos   v_end);
static void compile_continue_s(Nst_Node *node);
static void compile_break_s(Nst_Node *node);
static void compile_switch_s(Nst_Node *node);
static void compile_try_catch_s(Nst_Node *node);

Nst_InstructionList *nst_compile(Nst_Node *code, bool is_module)
{
    return compile_internal(code, false, is_module);
}

static Nst_InstructionList *compile_internal(Nst_Node *code,
                                             bool      is_func,
                                             bool      is_module)
{
    c_state.loop_id = 0;
    c_state.inst_ls = LList_new();
    Nst_InstructionList *inst_list =
        (Nst_InstructionList *)malloc(sizeof(Nst_InstructionList));
    if ( inst_list == NULL )
    {
        return NULL;
    }

    compile_node(code);

    bool add_return = c_state.inst_ls->tail == NULL ||
                      INST(c_state.inst_ls->tail->value)->id != NST_IC_RETURN_VAL;

    if ( is_module )
    {
        inst_list->total_size = c_state.inst_ls->size + 1;
    }
    else
    {
        inst_list->total_size = c_state.inst_ls->size + (add_return ? 2 : 0);
    }
    inst_list->instructions = (Nst_Instruction *)malloc(
        inst_list->total_size * sizeof(Nst_Instruction));
    if ( inst_list->instructions == NULL )
    {
        free(inst_list);
        LList_destroy(c_state.inst_ls, free);
        return NULL;
    }

    LList *funcs = LList_new();
    inst_list->functions = funcs;
    size_t i = 0;
    Nst_Instruction *inst;
    for ( LLNode *n = c_state.inst_ls->head;
          n != NULL;
          n = n->next )
    {
        inst = INST(n->value);

        if ( inst->id == NST_IC_PUSH_VAL &&
             inst->val != NULL &&
             inst->val->type == nst_t.Func )
            LList_append(funcs, inst->val, false);

        inst_list->instructions[i].id = inst->id;
        inst_list->instructions[i].int_val = inst->int_val;
        inst_list->instructions[i].val = inst->val;
        inst_list->instructions[i].start = inst->start;
        inst_list->instructions[i++].end = inst->end;
    }

    if ( is_module )
    {
        inst_list->instructions[i].id = NST_IC_RETURN_VARS;
        inst_list->instructions[i].int_val = 0;
        inst_list->instructions[i].val = NULL;
        inst_list->instructions[i].start = nst_no_pos();
        inst_list->instructions[i].end = nst_no_pos();
    }
    else if ( add_return )
    {
        inst_list->instructions[i].id = NST_IC_PUSH_VAL;
        inst_list->instructions[i].int_val = 0;
        inst_list->instructions[i].val = nst_inc_ref(nst_c.null);
        inst_list->instructions[i].start = nst_no_pos();
        inst_list->instructions[i++].end = nst_no_pos();

        inst_list->instructions[i].id = NST_IC_RETURN_VAL;
        inst_list->instructions[i].int_val = 0;
        inst_list->instructions[i].val = NULL;
        inst_list->instructions[i].start = nst_no_pos();
        inst_list->instructions[i].end = nst_no_pos();
    }

    // Using free to not decrease the references of the objects
    LList_destroy(c_state.inst_ls, free);
    if ( !is_func )
    {
        nst_destroy_node(code);
    }
    return inst_list;
}

static void compile_node(Nst_Node *node)
{
    switch ( node->type )
    {
    case NST_NT_LONG_S:         compile_long_s(node); break;
    case NST_NT_WHILE_L:        compile_while_l(node); break;
    case NST_NT_DOWHILE_L:      compile_dowhile_l(node); break;
    case NST_NT_FOR_L:          compile_for_l(node); break;
    case NST_NT_FOR_AS_L:       compile_for_as_l(node); break;
    case NST_NT_IF_E:           compile_if_e(node); break;
    case NST_NT_FUNC_DECLR:     compile_func_declr(node); break;
    case NST_NT_RETURN_S:       compile_return_s(node); break;
    case NST_NT_STACK_OP:       compile_stack_op(node); break;
    case NST_NT_LOCAL_STACK_OP: compile_local_stack_op(node); break;
    case NST_NT_LOCAL_OP:       compile_local_op(node); break;
    case NST_NT_ARR_LIT:
    case NST_NT_VEC_LIT:        compile_arr_or_vect_lit(node); break;
    case NST_NT_MAP_LIT:        compile_map_lit(node); break;
    case NST_NT_VALUE:          compile_value(node); break;
    case NST_NT_ACCESS:         compile_access(node); break;
    case NST_NT_EXTRACT_E:      compile_extract_e(node); break;
    case NST_NT_ASSIGN_E:       compile_assign_e(node); break;
    case NST_NT_CONTINUE_S:     compile_continue_s(node); break;
    case NST_NT_BREAK_S:        compile_break_s(node); break;
    case NST_NT_SWITCH_S:       compile_switch_s(node); break;
    case NST_NT_LAMBDA:         compile_lambda(node); break;
    case NST_NT_TRY_CATCH_S:    compile_try_catch_s(node); break;
    }
}

static void compile_long_s(Nst_Node *node)
{
    /*
    Long statement bytecode

    [STATEMENT]
    POP_VAL - added if the statement leaves a value on the evaluation stack
    ^ repeated for every statement
    */

    for ( LLNode *cursor = node->nodes->head;
          cursor != NULL;
          cursor = cursor->next )
    {
        compile_node(NODE(cursor->value));

        if ( NODE_RETUNS_VALUE(NODE(cursor->value)->type) )
        {
            Nst_Instruction *inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
            ADD_INST(inst);
        }
    }
}

static void compile_while_l(Nst_Node *node)
{
    /*
    While loop bytecode

                JUMP cond_start
    body_start: [BODY CODE]
    cond_start: [CONDITION CODE]
                JUMPIF body_start
                [CODE CONTINUATION]
    */

    Nst_Instruction *inst;
    Nst_Instruction *jump_cond_start = nst_new_inst_empty(NST_IC_JUMP, 0);
    ADD_INST(jump_cond_start);
    LLNode *body_start = c_state.inst_ls->tail;

    inc_loop_id();
    Nst_Int body_start_idx = CURR_LEN;
    compile_node(TAIL_NODE); // body
    LLNode *body_end = c_state.inst_ls->tail;

    Nst_Int cond_start_idx = CURR_LEN;
    jump_cond_start->int_val = cond_start_idx;
    compile_node(HEAD_NODE); // condition
    body_start = body_start->next;
    body_end = body_end->next;

    inst = nst_new_inst_empty(NST_IC_JUMPIF_T, body_start_idx);
    ADD_INST(inst);

    for ( LLNode *cursor = body_start;
        cursor != NULL && cursor != body_end;
        cursor = cursor->next )
    {
        inst = INST(cursor->value);

        if ( IS_JUMP(inst->id) )
        {
            // Continue statement
            if ( inst->int_val == c_state.loop_id )
            {
                inst->int_val = cond_start_idx;
            }
            // Break statement
            else if ( inst->int_val == c_state.loop_id - 1 )
            {
                inst->int_val = CURR_LEN;
            }
        }
    }

    dec_loop_id();
}

static void compile_dowhile_l(Nst_Node *node)
{
    /*
    Do-while loop bytecode

    body_start: [BODY CODE]
                [CONDITION CODE]
                JUMPIF body_start
                [CODE CONTINUATION]
    */
    Nst_Instruction *inst;
    LLNode *body_start = c_state.inst_ls->tail;

    inc_loop_id();
    Nst_Int body_start_idx = CURR_LEN;
    compile_node(TAIL_NODE); // body
    LLNode *body_end = c_state.inst_ls->tail;

    Nst_Int cond_start_idx = CURR_LEN;
    compile_node(HEAD_NODE); // condition
    if ( body_start == NULL )
    {
        body_start = c_state.inst_ls->head;
    }
    else
    {
        body_start = body_start->next;
    }
    body_end = body_end->next;

    inst = nst_new_inst_empty(NST_IC_JUMPIF_T, body_start_idx);
    ADD_INST(inst);

    for ( LLNode *cursor = body_start;
        cursor != NULL && cursor != body_end;
        cursor = cursor->next )
    {
        inst = INST(cursor->value);

        if ( IS_JUMP(inst->id) )
        {
            // Continue statement
            if ( inst->int_val == c_state.loop_id )
            {
                inst->int_val = cond_start_idx;
            }
            // Break statement
            else if ( inst->int_val == c_state.loop_id - 1 )
            {
                inst->int_val = CURR_LEN;
            }
        }
    }

    dec_loop_id();
}

static void compile_for_l(Nst_Node *node)
{
    /*
    For loop bytecode

                [TIMES TO REPEAT CODE]
                TYPE_CHECK Int
                NEW_OBJ
    cond_start: JUMPIF_ZERO body_end
                [BODY CODE]
    body_end:   DEC_INT
                JUMP cond_start
    loop_end:   POP_VAL
                POP_VAL
                [CODE CONTINUATION]
    */

    Nst_Instruction *inst;
    compile_node(HEAD_NODE);

    inst = nst_new_inst_val(
        NST_IC_TYPE_CHECK,
        nst_t.Int,
        INST(c_state.inst_ls->tail->value)->start,
        INST(c_state.inst_ls->tail->value)->end);
    ADD_INST(inst);

    inst = nst_new_inst_empty(NST_IC_NEW_OBJ, 0);
    ADD_INST(inst);

    Nst_Int cond_start_idx = CURR_LEN;
    Nst_Instruction *jump_body_end_idx =
        nst_new_inst_empty(NST_IC_JUMPIF_ZERO, 0);
    ADD_INST(jump_body_end_idx);
    LLNode *body_start = c_state.inst_ls->tail;

    inc_loop_id();
    compile_node(TAIL_NODE); // body
    Nst_Int body_end_idx = CURR_LEN;
    inst = nst_new_inst_empty(NST_IC_DEC_INT, 0);
    ADD_INST(inst);
    LLNode *body_end = c_state.inst_ls->tail;

    inst = nst_new_inst_empty(NST_IC_JUMP, cond_start_idx);
    ADD_INST(inst);
    Nst_Int loop_end_idx = CURR_LEN;
    jump_body_end_idx->int_val = loop_end_idx;

    inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
    ADD_INST(inst);
    inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
    ADD_INST(inst);

    for ( body_start = body_start->next;
          body_start != NULL && body_start != body_end;
          body_start = body_start->next )
    {
        inst = INST(body_start->value);

        if ( IS_JUMP(inst->id) )
        {
            // Continue statement
            if ( inst->int_val == c_state.loop_id )
            {
                inst->int_val = body_end_idx;
            }
            // Break statement
            else if ( inst->int_val == c_state.loop_id - 1 )
            {
                inst->int_val = loop_end_idx;
            }
        }
    }

    dec_loop_id();
}

static void compile_for_as_l(Nst_Node *node)
{
    /*
    For-as loop bytecode

                [ITERATOR CODE]
                TYPE_CHECK Iter
                FOR_START
                POP_VAL
    cond_start: FOR_IS_DONE
                JUMPIF_T body_end
                FOR_GET_VAL
                SET_VAL_LOC name
                [BODY CODE]
    body_end:   JUMP cond_start
    loop_end:   POP_VAL
                [CODE CONTINUATION]
    */

    Nst_Instruction *inst;
    compile_node(HEAD_NODE);
    inst = nst_new_inst_val(
        NST_IC_TYPE_CHECK,
        nst_t.Iter,
        HEAD_NODE->start,
        HEAD_NODE->end);
    ADD_INST(inst);

    // Initialization
    inst = nst_new_inst_int(
        NST_IC_FOR_START, 1,
        HEAD_NODE->start,
        HEAD_NODE->end);
    ADD_INST(inst);
    inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
    ADD_INST(inst);

    // Condition
    Nst_Int cond_start_idx = CURR_LEN;
    inst = nst_new_inst_int(
        NST_IC_FOR_IS_DONE, 1,
        HEAD_NODE->start,
        HEAD_NODE->end);
    ADD_INST(inst);
    Nst_Instruction *jump_body_end = nst_new_inst_empty(NST_IC_JUMPIF_T, 0);
    ADD_INST(jump_body_end);

    // Setting variable
    inst = nst_new_inst_int(
        NST_IC_FOR_GET_VAL, 1,
        HEAD_NODE->start,
        HEAD_NODE->end);
    ADD_INST(inst);
    compile_unpacking_assign_e(SECOND_NODE, HEAD_NODE->start, HEAD_NODE->end);

    // For loop body
    inc_loop_id();
    LLNode *body_start = c_state.inst_ls->tail;
    compile_node(TAIL_NODE);

    // For loop advance
    Nst_Int loop_advance = CURR_LEN;
    body_start = body_start->next;
    LLNode *body_end = c_state.inst_ls->tail;
    inst = nst_new_inst_empty(NST_IC_JUMP, cond_start_idx);
    ADD_INST(inst);

    Nst_Int body_end_idx = CURR_LEN;
    jump_body_end->int_val = body_end_idx;
    inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
    ADD_INST(inst);

    for ( LLNode *cursor = body_start;
        cursor != NULL && cursor != body_end;
        cursor = cursor->next )
    {
        inst = INST(cursor->value);

        if ( IS_JUMP(inst->id) )
        {
            // Continue statement
            if ( inst->int_val == c_state.loop_id )
            {
                inst->int_val = loop_advance;
            }
            // Break statement
            else if ( inst->int_val == c_state.loop_id - 1 )
            {
                inst->int_val = body_end_idx;
            }
        }
    }

    dec_loop_id();
}

static void compile_if_e(Nst_Node *node)
{
    /*
    If expression with else statement bytecode

            [CONDITION CODE]
            JUMPIF_FALSE elseif
            [CODE IF TRUE]
            JUMP if_end
    elseif: [CODE IF FALSE]
    if_end: [CODE CONTINUATION]

    If expression bytecode

            [CONDITION CODE]
            JUMPIF_FALSE elseif
            [CODE IF TRUE]
            JUMP if_end
    elseif: PUSH_VAL null
    if_end: [CODE CONTINUATION]
    */

    compile_node(HEAD_NODE); // Condition

    Nst_Instruction *inst;
    Nst_Instruction *jump_if_false =
        nst_new_inst_empty(NST_IC_JUMPIF_F, 0);
    ADD_INST(jump_if_false);

    Nst_Node *second_node = SECOND_NODE;
    bool both_are_null = !NODE_RETUNS_VALUE(second_node->type) &&
                         (node->nodes->size == 2 ||
                          !NODE_RETUNS_VALUE(TAIL_NODE->type));

    compile_node(second_node); // Body if true
    if ( !both_are_null && !NODE_RETUNS_VALUE(second_node->type) )
    {
        inst = nst_new_inst_val(
            NST_IC_PUSH_VAL,
            nst_c.null,
            nst_no_pos(),
            nst_no_pos());
        ADD_INST(inst);
    }

    Nst_Instruction *jump_at_end = nst_new_inst_empty(NST_IC_JUMP, 0);
    ADD_INST(jump_at_end);
    jump_if_false->int_val = CURR_LEN;

    if ( node->nodes->size == 2 )
    {
        if ( both_are_null )
        {
            jump_at_end->int_val = CURR_LEN;
        }

        inst = nst_new_inst_val(
            NST_IC_PUSH_VAL,
            nst_c.null,
            nst_no_pos(),
            nst_no_pos());
        ADD_INST(inst);
    }
    else
    {
        compile_node(TAIL_NODE); // Body if false

        if ( both_are_null )
        {
            jump_at_end->int_val = CURR_LEN;
        }

        if ( !NODE_RETUNS_VALUE(TAIL_NODE->type) )
        {
            inst = nst_new_inst_val(
                NST_IC_PUSH_VAL,
                nst_c.null,
                nst_no_pos(),
                nst_no_pos());
            ADD_INST(inst);
        }
    }
    if ( !both_are_null )
    {
        jump_at_end->int_val = CURR_LEN;
    }
}

static void compile_func_declr(Nst_Node *node)
{
    /*
    Func declaration bytecode

    PUSH_VAL function
    SET_VAL_LOC name
    */

    int prev_loop_id = c_state.loop_id;
    LList *prev_inst_ls = c_state.inst_ls;
    Nst_FuncObj *func = FUNC(nst_new_func(
        node->tokens->size - 1,
        compile_internal(HEAD_NODE, true, false)));
    c_state.loop_id = prev_loop_id;
    c_state.inst_ls = prev_inst_ls;

    size_t i = 0;
    for ( LLNode *n = node->tokens->head->next; n != NULL; n = n->next )
    {
        func->args[i++] = nst_inc_ref(TOK(n->value)->value);
    }

    Nst_Instruction *inst = nst_new_inst_val(
        NST_IC_PUSH_VAL,
        OBJ(func),
        nst_no_pos(),
        nst_no_pos());
    ADD_INST(inst);
    nst_dec_ref(func);

    inst = nst_new_inst_val(
        NST_IC_SET_VAL_LOC,
        HEAD_TOK->value,
        nst_no_pos(),
        nst_no_pos());
    ADD_INST(inst);
}

static void compile_lambda(Nst_Node *node)
{
    /*
    Lambda bytecode

    PUSH_VAL lambda function
    */

    int prev_loop_id = c_state.loop_id;
    LList *prev_inst_ls = c_state.inst_ls;
    Nst_FuncObj *func = FUNC(nst_new_func(
        node->tokens->size,
        compile_internal(HEAD_NODE, true, false)));
    c_state.loop_id = prev_loop_id;
    c_state.inst_ls = prev_inst_ls;

    size_t i = 0;
    for ( LLNode *n = node->tokens->head; n != NULL; n = n->next )
    {
        func->args[i++] = nst_inc_ref(TOK(n->value)->value);
    }

    Nst_Instruction *inst = nst_new_inst_val(
        NST_IC_PUSH_VAL,
        OBJ(func),
        node->start,
        node->end);
    ADD_INST(inst);
    nst_dec_ref(func);
}

static void compile_return_s(Nst_Node *node)
{
    /*
    Return bytecode

    [EXPR CODE]
    RETURN_VAL

    Pops all values from the stack until it finds a nullptr
    */

    compile_node(HEAD_NODE);
    Nst_Instruction *inst = nst_new_inst_empty(NST_IC_RETURN_VAL, 0);
    ADD_INST(inst);
}

static void compile_stack_op(Nst_Node *node)
{
    /*
    Stack operation bytecode

    [VAL 1 CODE]
    [VAL 2 CODE]
    OP_STACK - operator
    */

    if ( T_IN_COMP_OP(HEAD_TOK->type) && node->nodes->size > 2 )
    {
        compile_comp_op(node);
        return;
    }

    compile_node(HEAD_NODE);

    if ( HEAD_TOK->type == NST_TT_L_AND || HEAD_TOK->type == NST_TT_L_OR )
    {
        compile_lg_op(node);
        return;
    }

    compile_node(TAIL_NODE);

    Nst_Instruction *inst = nst_new_inst_int(
        NST_IC_STACK_OP,
        HEAD_TOK->type,
        node->start,
        node->end);
    ADD_INST(inst);
}

static void compile_comp_op(Nst_Node *node)
{
    /*
    Comparison operator bytecode

         [VALUE 1]            
         [VALUE 2]            -+
         DUP                   |
         ROT 3                 |
         OP_STACK - operator   | Repeated for each value
         DUP                   |
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

    Nst_Instruction *inst;
    int op_id = HEAD_TOK->type;

    compile_node(HEAD_NODE);
    LList *fix_jumps = LList_new();

    for ( LLNode *n = node->nodes->head->next;
          n->next != NULL;
          n = n->next )
    {
        compile_node(NODE(n->value));
        inst = nst_new_inst_empty(NST_IC_DUP, 0);
        ADD_INST(inst);
        inst = nst_new_inst_empty(NST_IC_ROT, 3);
        ADD_INST(inst);
        inst = nst_new_inst_int(NST_IC_STACK_OP, op_id, node->start, node->end);
        ADD_INST(inst);
        inst = nst_new_inst_empty(NST_IC_DUP, 0);
        ADD_INST(inst);
        inst = nst_new_inst_empty(NST_IC_JUMPIF_F, 0);
        LList_append(fix_jumps, inst, false);
        ADD_INST(inst);
        inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
        ADD_INST(inst);
    }

    compile_node(TAIL_NODE);
    inst = nst_new_inst_int(NST_IC_STACK_OP, op_id, node->start, node->end);
    ADD_INST(inst);
    inst = nst_new_inst_empty(NST_IC_JUMP, CURR_LEN + 3);
    ADD_INST(inst);

    for ( ITER_LLIST(n, fix_jumps) )
    {
        ((Nst_Instruction*)n->value)->int_val = CURR_LEN;
    }

    inst = nst_new_inst_empty(NST_IC_ROT, 2);
    ADD_INST(inst);
    inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
    ADD_INST(inst);
}

static void compile_lg_op(Nst_Node *node)
{
    /*
    L_AND bytecode

           [VAL 1 CODE]
           DUP
           JUMPIF_F e_end
           POP_VAL
           [VAL 2 CODE]
    e_end: [CODE CONTINUATION]

    L_OR bytecode

           [VAL 1 CODE]
           DUP
           JUMPIF_T e_end
           POP_VAL
           [VAL 2 CODE]
    e_end: [CODE CONTINUATION]
    */

    Nst_Instruction *inst;
    Nst_Instruction *jump_to_end;

    inst = nst_new_inst_empty(NST_IC_DUP, 0);
    ADD_INST(inst);

    if ( HEAD_TOK->type == NST_TT_L_AND )
    {
        jump_to_end = nst_new_inst_empty(NST_IC_JUMPIF_F, 0);
        ADD_INST(jump_to_end);
    }
    else
    {
        jump_to_end = nst_new_inst_empty(NST_IC_JUMPIF_T, 0);
        ADD_INST(jump_to_end);
    }

    inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
    ADD_INST(inst);

    compile_node(TAIL_NODE);

    jump_to_end->int_val = CURR_LEN;
}

static void compile_local_stack_op(Nst_Node *node)
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

    int tok_type = HEAD_TOK->type;
    Nst_Instruction *inst;

    if ( tok_type == NST_TT_RANGE )
    {
        compile_node(HEAD_NODE);
        inst = nst_new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t.Int,
            HEAD_NODE->start,
            HEAD_NODE->end);
        ADD_INST(inst);

        if ( node->nodes->size == 3 )
        {
            compile_node(SECOND_NODE);
            inst = nst_new_inst_val(
                NST_IC_TYPE_CHECK,
                nst_t.Int,
                HEAD_NODE->start,
                HEAD_NODE->end);
            ADD_INST(inst);
        }

        compile_node(TAIL_NODE);
        inst = nst_new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t.Int,
            HEAD_NODE->start,
            HEAD_NODE->end);
        ADD_INST(inst);

        inst = nst_new_inst_int(
            NST_IC_OP_RANGE,
            node->nodes->size,
            node->start,
            node->end);
        ADD_INST(inst);
    }
    else if ( tok_type == NST_TT_CAST )
    {
        compile_node(HEAD_NODE);
        inst = nst_new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t.Type,
            HEAD_NODE->start,
            HEAD_NODE->end);
        ADD_INST(inst);

        compile_node(TAIL_NODE);
        inst = nst_new_inst_int(NST_IC_OP_CAST, 0, node->start, node->end);
        ADD_INST(inst);
    }
    else if ( tok_type == NST_TT_CALL || tok_type == NST_TT_SEQ_CALL )
    {
        for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
        {
            compile_node(NODE(n->value));
        }

        inst = nst_new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t.Func,
            node->start,
            node->end);
        ADD_INST(inst);
        inst = nst_new_inst_int(
            NST_IC_OP_CALL,
            tok_type == NST_TT_CALL ? node->nodes->size - 1 : -1,
            node->start, node->end);
        ADD_INST(inst);
        // The function object is popped by the RETURN_VAL instruction
    }
    else if ( tok_type == NST_TT_THROW )
    {
        compile_node(HEAD_NODE);
        inst = nst_new_inst_val(NST_IC_TYPE_CHECK, nst_t.Str, HEAD_NODE->start, HEAD_NODE->end);
        ADD_INST(inst);

        compile_node(TAIL_NODE);
        inst = nst_new_inst_val(NST_IC_TYPE_CHECK, nst_t.Str, TAIL_NODE->start, TAIL_NODE->end);
        ADD_INST(inst);

        inst = nst_new_inst_int(NST_IC_THROW_ERR, 0, node->start, node->end);
        ADD_INST(inst);
    }
}

static void compile_local_op(Nst_Node *node)
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

    Nst_Instruction *inst;

    if ( HEAD_TOK->type == NST_TT_LOC_CALL )
    {
        compile_node(HEAD_NODE);
        inst = nst_new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t.Func,
            node->start,
            node->end);
        ADD_INST(inst);
        inst = nst_new_inst_int(
            NST_IC_OP_CALL,
            node->nodes->size - 1,
            node->start,
            node->end);
        ADD_INST(inst);
        return;
    }

    compile_node(HEAD_NODE);

    if ( HEAD_TOK->type == NST_TT_IMPORT )
    {
        inst = nst_new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t.Str,
            node->start,
            node->end);
        ADD_INST(inst);
        inst = nst_new_inst_int(
            NST_IC_OP_IMPORT, 0,
            node->start,
            node->end);
        ADD_INST(inst);
        return;
    }

    inst = nst_new_inst_int(
        NST_IC_LOCAL_OP,
        HEAD_TOK->type,
        HEAD_NODE->start,
        HEAD_NODE->end);

    ADD_INST(inst);
}

static void compile_arr_or_vect_lit(Nst_Node *node)
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

    Nst_Instruction *inst;
    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        compile_node(NODE(n->value));
    }

    if ( node->tokens->size != 0 )
    {
        inst = nst_new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t.Int,
            TAIL_NODE->start,
            TAIL_NODE->end);
        ADD_INST(inst);
        inst = nst_new_inst_empty(
            node->type == NST_NT_ARR_LIT ? NST_IC_MAKE_ARR_REP
                                         : NST_IC_MAKE_VEC_REP,
            node->nodes->size);
    }
    else
    {
        inst = nst_new_inst_empty(
            node->type == NST_NT_ARR_LIT ? NST_IC_MAKE_ARR : NST_IC_MAKE_VEC,
            node->nodes->size);
    }

    ADD_INST(inst);
}

static void compile_map_lit(Nst_Node *node)
{
    /*
    Map literal bytecode

    ( [VALUE CODE]
      HASH_CHECK )*
    MAKE_MAP - number of elements (key-value pairs * 2)
    */
    Nst_Instruction *inst;

    bool is_key = true;
    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        compile_node(NODE(n->value));

        if ( is_key )
        {
            inst = nst_new_inst_int(
                NST_IC_HASH_CHECK,
                0,
                NODE(n->value)->start,
                NODE(n->value)->end);
            ADD_INST(inst);
        }
        is_key = !is_key;
    }

    inst = nst_new_inst_empty(NST_IC_MAKE_MAP, node->nodes->size);
    ADD_INST(inst);
}

static void compile_value(Nst_Node *node)
{
    /*
    Value bytecode

    PUSH_VAL value
    */
    Nst_Instruction *inst = nst_new_inst_val(
        NST_IC_PUSH_VAL,
        HEAD_TOK->value,
        node->start,
        node->end);
    ADD_INST(inst);
}

static void compile_access(Nst_Node *node)
{
    /*
    Access bytecode

    GET_VAL var_name
    */
    Nst_Instruction *inst = nst_new_inst_val(
        NST_IC_GET_VAL,
        HEAD_TOK->value,
        node->start,
        node->end);
    ADD_INST(inst);
}

static void compile_extract_e(Nst_Node *node)
{
    /*
    Extraction bytecode

    [CONTAINER CODE]
    [INDEX_CODE]
    OP_EXTRACT
    */
    compile_node(HEAD_NODE); // Container
    compile_node(TAIL_NODE); // Value

    Nst_Instruction *inst = nst_new_inst_pos(
        NST_IC_OP_EXTRACT,
        node->start,
        node->end);
    ADD_INST(inst);
}

static void compile_assign_e(Nst_Node *node)
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
    Nst_Instruction *inst;
    compile_node(HEAD_NODE);

    if ( TAIL_NODE->type == NST_NT_ACCESS )
    {
        inst = nst_new_inst_val(
            NST_IC_SET_VAL,
            TOK(TAIL_NODE->tokens->head->value)->value,
            node->start,
            node->end);
        ADD_INST(inst);
    }
    else if ( TAIL_NODE->type == NST_NT_EXTRACT_E )
    {
        compile_node(NODE(TAIL_NODE->nodes->head->value)); // Container
        compile_node(NODE(TAIL_NODE->nodes->tail->value)); // Index

        inst = nst_new_inst_pos(
            NST_IC_SET_CONT_VAL,
            node->start,
            node->end);
        ADD_INST(inst);
    }
    else
    {
        inst = nst_new_inst_empty(NST_IC_DUP, 0);
        ADD_INST(inst);
        compile_unpacking_assign_e(TAIL_NODE, HEAD_NODE->start, HEAD_NODE->end);
    }
}

static void compile_unpacking_assign_e(Nst_Node *node, Nst_Pos v_start, Nst_Pos v_end)
{
    /*
    Unpacking assigment

    [VALUE CODE]
    DUP
    UNPACK_SEQ size_of_expected

    SET_VAL_LOC name - for variables

    [CONTAINER_CODE]  +
    [INDEX_CODE]      | for containers
    SET_CONTAINER_VAL +
    */

    Nst_Instruction *inst;
    LList *nodes = LList_new();
    LList_push(nodes, node, false);

    while ( nodes->size != 0 )
    {
        Nst_Node *curr_node = NODE(LList_pop(nodes));

        if ( curr_node->type == NST_NT_ARR_LIT )
        {
            LLNode *list_head = nodes->head;
            LLNode *list_tail = nodes->tail;
            nodes->head = NULL;
            nodes->tail = NULL;

            for ( ITER_LLIST(n, curr_node->nodes) )
            {
                LList_append(nodes, n->value, false);
            }

            // the array literal is guaranteed to have at least one item
            nodes->tail->next = list_head;
            nodes->tail = list_tail;

            inst = nst_new_inst_int(
                NST_IC_UNPACK_SEQ,
                curr_node->nodes->size,
                v_start,
                v_end);
        }
        else if ( curr_node->type == NST_NT_ACCESS )
        {
            inst = nst_new_inst_val(
                NST_IC_SET_VAL_LOC,
                TOK(curr_node->tokens->head->value)->value,
                curr_node->start,
                curr_node->end);
        }
        else
        {
            compile_node(NODE(curr_node->nodes->head->value)); // Container
            compile_node(NODE(curr_node->nodes->tail->value)); // Index

            inst = nst_new_inst_pos(
                NST_IC_SET_CONT_LOC,
                curr_node->start,
                curr_node->end);
        }

        ADD_INST(inst);
    }

    LList_destroy(nodes, NULL);
}

static void compile_continue_s(Nst_Node *node)
{
    /*
    Continue bytecode

    JUMP loop_id - replaced later with the loop's start
    */
    Nst_Instruction *inst = nst_new_inst_int(
        NST_IC_JUMP,
        c_state.loop_id,
        node->start,
        node->end);

    ADD_INST(inst);
}

static void compile_break_s(Nst_Node *node)
{
    /*
    Continue bytecode

    JUMP loop_id - replaced later with the loop's end
    */
    Nst_Instruction *inst = nst_new_inst_int(
        NST_IC_JUMP,
        c_state.loop_id - 1,
        node->start,
        node->end);

    ADD_INST(inst);
}

static void compile_switch_s(Nst_Node *node)
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

    Nst_Instruction *inst;
    LLNode *prev_body_start = NULL;
    LLNode *prev_body_end = NULL;
    Nst_Node *default_body = NULL;
    LList *jumps_to_switch_end = LList_new();

    compile_node(HEAD_NODE);

    for ( LLNode *n = node->nodes->head->next;
          n != NULL;
          n = n->next )
    {
        if ( n->next == NULL )
        {
            default_body = NODE(n->value);
            break;
        }

        inst = nst_new_inst_empty(NST_IC_DUP, 0);
        ADD_INST(inst);
        compile_node(NODE(n->value));
        n = n->next;
        inst = nst_new_inst_empty(NST_IC_STACK_OP, NST_TT_EQ);
        ADD_INST(inst);
        Nst_Instruction *jump_body_end =
            nst_new_inst_empty(NST_IC_JUMPIF_F, 0);
        ADD_INST(jump_body_end);
        LLNode *body_start = c_state.inst_ls->tail;

        inc_loop_id();
        Nst_Int next_body_start = CURR_LEN;
        compile_node(NODE(n->value));
        inst = nst_new_inst_empty(NST_IC_JUMP, 0);
        ADD_INST(inst);
        LList_append(jumps_to_switch_end, inst, false);
        jump_body_end->int_val = CURR_LEN;
        LLNode *body_end = c_state.inst_ls->tail;

        if ( prev_body_start != NULL )
        {
            for ( LLNode *cursor = prev_body_start->next;
                  cursor != NULL && cursor != prev_body_end;
                  cursor = cursor->next )
            {
                inst = INST(cursor->value);

                if ( IS_JUMP(inst->id) && inst->int_val == c_state.loop_id )
                {
                    inst->int_val = next_body_start;
                }
            }
        }

        dec_loop_id();
        prev_body_start = body_start;
        prev_body_end = body_end;
    }

    if ( prev_body_start != NULL )
    {
        inc_loop_id();
        for ( LLNode *cursor = prev_body_start->next;
            cursor != NULL && cursor != prev_body_end;
            cursor = cursor->next )
        {
            inst = INST(cursor->value);

            if ( IS_JUMP(inst->id) && inst->int_val == c_state.loop_id )
            {
                inst->int_val = CURR_LEN;
            }
        }
        dec_loop_id();
    }

    if ( default_body != NULL )
    {
        inc_loop_id();
        LLNode *default_body_start = c_state.inst_ls->tail;
        compile_node(default_body);

        for ( LLNode *cursor = default_body_start->next;
            cursor != NULL;
            cursor = cursor->next )
        {
            inst = INST(cursor->value);

            if ( IS_JUMP(inst->id) && inst->int_val == c_state.loop_id )
            {
                inst->int_val = CURR_LEN;
            }
        }

        dec_loop_id();
    }

    for ( LLNode *cursor = jumps_to_switch_end->head;
          cursor != NULL;
          cursor = cursor->next )
        INST(cursor->value)->int_val = CURR_LEN;

    inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
    ADD_INST(inst);
    LList_destroy(jumps_to_switch_end, NULL);
}

static void compile_try_catch_s(Nst_Node* node)
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

    Nst_Instruction *inst;
    Nst_Instruction *push_catch =
        nst_new_inst_empty(NST_IC_PUSH_CATCH, 0);
    ADD_INST(push_catch);
    compile_node(HEAD_NODE);

    if ( NODE_RETUNS_VALUE(HEAD_NODE->type) )
    {
        inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
        ADD_INST(inst);
    }

    inst = nst_new_inst_empty(NST_IC_POP_CATCH, 0);
    ADD_INST(inst);

    // The code inside this jump will never be optimized since NST_IC_PUSH_CATCH,
    // that is considered a jump, refers to NST_IC_SAVE_ERROR that is inside the
    // block
    Nst_Instruction *jump_catch_end = nst_new_inst_empty(NST_IC_JUMP, 0);
    ADD_INST(jump_catch_end);
    push_catch->int_val = CURR_LEN;

    inst = nst_new_inst_empty(NST_IC_SAVE_ERROR, 0);
    ADD_INST(inst);
    inst = nst_new_inst_empty(NST_IC_POP_CATCH, 0);
    ADD_INST(inst);
    inst = nst_new_inst_val(
        NST_IC_SET_VAL_LOC,
        HEAD_TOK->value,
        HEAD_TOK->start,
        HEAD_TOK->end);
    ADD_INST(inst);
    compile_node(TAIL_NODE);
    if ( NODE_RETUNS_VALUE(TAIL_NODE->type) )
    {
        inst = nst_new_inst_empty(NST_IC_POP_VAL, 0);
        ADD_INST(inst);
    }
    jump_catch_end->int_val = CURR_LEN;
}

void nst_print_bytecode(Nst_InstructionList *ls, int indent)
{
    size_t tot_size = ls->total_size;
    int i_len = 1; // maximum length of the index in a string

    while ( tot_size >= 10 )
    {
        tot_size /= 10;
        ++i_len;
    }

    i_len = i_len < 3 ? 3 : i_len;

    for ( int i = 0; i < indent; i++ )
    {
        PRINT("    ", 4);
    }
    for ( int i = 3; i < i_len;  i++ )
    {
        putchar(' ');
    }

    PRINT(" Idx |   Pos   |  Instruction  | ", 33);

    for ( int i = 3; i < i_len; i++ )
    {
        putchar(' ');
    }

    PRINT("Int | Object\n", 13);

    for ( size_t i = 0, n = ls->total_size; i < n; i++ )
    {
        Nst_Instruction inst = ls->instructions[i];

        for ( int j = 0; j < indent; j++ )
        {
            printf("    ");
        }
        if ( inst.start.text == NULL )
        {
            printf(" %*zi |         | ", i_len, i);
        }
        else
        {
            printf(
                " %*zi | %3li:%-3li | ",
                i_len, i,
                inst.start.line + 1,
                inst.start.col + 1);
        }

        switch ( inst.id )
        {
        case NST_IC_NO_OP:         PRINT("NO_OP        ", 13); break;
        case NST_IC_POP_VAL:       PRINT("POP_VAL      ", 13); break;
        case NST_IC_FOR_START:     PRINT("FOR_START    ", 13); break;
        case NST_IC_RETURN_VAL:    PRINT("RETURN_VAL   ", 13); break;
        case NST_IC_RETURN_VARS:   PRINT("RETURN_VARS  ", 13); break;
        case NST_IC_SET_VAL_LOC:   PRINT("SET_VAL_LOC  ", 13); break;
        case NST_IC_SET_CONT_LOC:  PRINT("SET_CONT_LOC ", 13); break;
        case NST_IC_JUMP:          PRINT("JUMP         ", 13); break;
        case NST_IC_JUMPIF_T:      PRINT("JUMPIF_T     ", 13); break;
        case NST_IC_JUMPIF_F:      PRINT("JUMPIF_F     ", 13); break;
        case NST_IC_JUMPIF_ZERO:   PRINT("JUMPIF_ZERO  ", 13); break;
        case NST_IC_TYPE_CHECK:    PRINT("TYPE_CHECK   ", 13); break;
        case NST_IC_HASH_CHECK:    PRINT("HASH_CHECK   ", 13); break;
        case NST_IC_THROW_ERR:     PRINT("THROW_ERR    ", 13); break;
        case NST_IC_PUSH_CATCH:    PRINT("PUSH_CATCH   ", 13); break;
        case NST_IC_POP_CATCH:     PRINT("POP_CATCH    ", 13); break;
        case NST_IC_SET_VAL:       PRINT("SET_VAL      ", 13); break;
        case NST_IC_GET_VAL:       PRINT("GET_VAL      ", 13); break;
        case NST_IC_PUSH_VAL:      PRINT("PUSH_VAL     ", 13); break;
        case NST_IC_SET_CONT_VAL:  PRINT("SET_CONT_VAL ", 13); break;
        case NST_IC_OP_CALL:       PRINT("OP_CALL      ", 13); break;
        case NST_IC_OP_CAST:       PRINT("OP_CAST      ", 13); break;
        case NST_IC_OP_RANGE:      PRINT("OP_RANGE     ", 13); break;
        case NST_IC_STACK_OP:      PRINT("STACK_OP     ", 13); break;
        case NST_IC_LOCAL_OP:      PRINT("LOCAL_OP     ", 13); break;
        case NST_IC_OP_IMPORT:     PRINT("OP_IMPORT    ", 13); break;
        case NST_IC_OP_EXTRACT:    PRINT("OP_EXTRACT   ", 13); break;
        case NST_IC_DEC_INT:       PRINT("DEC_INT      ", 13); break;
        case NST_IC_NEW_OBJ:       PRINT("NEW_OBJ      ", 13); break;
        case NST_IC_DUP:           PRINT("DUP          ", 13); break;
        case NST_IC_ROT:           PRINT("ROT          ", 13); break;
        case NST_IC_MAKE_ARR:      PRINT("MAKE_ARR     ", 13); break;
        case NST_IC_MAKE_ARR_REP:  PRINT("MAKE_ARR_REP ", 13); break;
        case NST_IC_MAKE_VEC:      PRINT("MAKE_VEC     ", 13); break;
        case NST_IC_MAKE_VEC_REP:  PRINT("MAKE_VEC_REP ", 13); break;
        case NST_IC_MAKE_MAP:      PRINT("MAKE_MAP     ", 13); break;
        case NST_IC_FOR_IS_DONE:   PRINT("FOR_IS_DONE  ", 13); break;
        case NST_IC_FOR_GET_VAL:   PRINT("FOR_GET_VAL  ", 13); break;
        case NST_IC_SAVE_ERROR:    PRINT("SAVE_ERROR   ", 13); break;
        case NST_IC_UNPACK_SEQ:    PRINT("UNPACK_SEQ   ", 13); break;
        default:                   PRINT("__UNKNOWN__  ", 13); break;
        }

        if ( inst.id == NST_IC_NO_OP )
        {
            PRINT(" | ", 3);
            for ( int j = 0; j < i_len; j++ )
            {
                putchar(' ');
            }
            PRINT(" |", 2);
            putchar('\n');
            continue;
        }

        if ( IS_JUMP(inst.id)           ||
             inst.id == NST_IC_LOCAL_OP ||
             inst.id == NST_IC_STACK_OP ||
             inst.id == NST_IC_MAKE_ARR ||
             inst.id == NST_IC_MAKE_VEC ||
             inst.id == NST_IC_MAKE_MAP ||
             inst.int_val != 0 )
        {
            printf(" | %*lli |", i_len > 3 ? i_len : 3, inst.int_val);
        }
        else
        {
            PRINT(" | ", 3);
            for ( int j = 0; j < i_len; j++ )
            {
                putchar(' ');
            }
            PRINT(" |", 2);
        }

        if ( inst.val != NULL )
        {
            printf(" (%s) ", TYPE_NAME(inst.val));
            Nst_StrObj* s = STR(_nst_repr_str_cast(inst.val));
            fwrite(s->value, sizeof(char), s->len, stdout);
            nst_dec_ref(s);

            if ( inst.val->type == nst_t.Func )
            {
                PRINT("\n\n", 2);
                for ( int j = 0; j < indent + 1; j++ )
                {
                    PRINT("    ", 4);
                }
                PRINT("<Func object> bytecode:\n", 24);
                nst_print_bytecode(FUNC(inst.val)->body.bytecode, indent + 1);
            }
        }
        else if ( inst.id == NST_IC_STACK_OP || inst.id == NST_IC_LOCAL_OP )
        {
            PRINT(" [", 2);

            switch ( inst.int_val )
            {
            case NST_TT_ADD:    PRINT("+", 1);  break;
            case NST_TT_SUB:    PRINT("-", 1);  break;
            case NST_TT_MUL:    PRINT("*", 1);  break;
            case NST_TT_DIV:    PRINT("/", 1);  break;
            case NST_TT_POW:    PRINT("^", 1);  break;
            case NST_TT_MOD:    PRINT("%", 1);  break;
            case NST_TT_B_AND:  PRINT("&", 1);  break;
            case NST_TT_B_OR:   PRINT("|", 1);  break;
            case NST_TT_LEN:    PRINT("$", 1);  break;
            case NST_TT_L_NOT:  PRINT("!", 1);  break;
            case NST_TT_B_NOT:  PRINT("~", 1);  break;
            case NST_TT_GT:     PRINT(">", 1);  break;
            case NST_TT_LT:     PRINT("<", 1);  break;
            case NST_TT_B_XOR:  PRINT("^^", 2); break;
            case NST_TT_LSHIFT: PRINT("<<", 2); break;
            case NST_TT_RSHIFT: PRINT(">>", 2); break;
            case NST_TT_CONCAT: PRINT("><", 2); break;
            case NST_TT_L_AND:  PRINT("&&", 2); break;
            case NST_TT_L_OR:   PRINT("||", 2); break;
            case NST_TT_L_XOR:  PRINT("&|", 2); break;
            case NST_TT_EQ:     PRINT("==", 2); break;
            case NST_TT_NEQ:    PRINT("!=", 2); break;
            case NST_TT_GTE:    PRINT(">=", 2); break;
            case NST_TT_LTE:    PRINT("<=", 2); break;
            case NST_TT_NEG:    PRINT("-:", 2); break;
            case NST_TT_STDOUT: PRINT(">>>", 3);break;
            case NST_TT_STDIN:  PRINT("<<<", 3);break;
            case NST_TT_TYPEOF: PRINT("?::", 3);break;
            default: PRINT("__UNKNOWN_OP__", 14);
            }

            putchar(']');
        }

        putchar('\n');
    }
}
