#include <stdlib.h>
#include "compiler.h"
#include "llist.h"
#include "tokens.h"
#include "simple_types.h"
#include "str.h"
#include "obj_ops.h"
#include "function.h"

#define HEAD_NODE (NODE(node->nodes->head->value))
#define TAIL_NODE (NODE(node->nodes->tail->value))
#define HEAD_TOK (TOK(node->tokens->head->value))
#define TAIL_TOK (TOK(node->tokens->tail->value))

#define INST(instruction) ((Nst_RuntimeInstruction *)(instruction))
#define CURR_LEN ((Nst_Int)(c_state.inst_ls->size))

#define ADD_INST(instruction) LList_append(c_state.inst_ls, instruction, true)

typedef struct
{
    int loop_id;
    LList *inst_ls;
}
CompileState;

CompileState c_state;

static inc_loop_id()
{
    c_state.loop_id -= 2;
}

static dec_loop_id()
{
    c_state.loop_id += 2;
}

static Nst_InstructionList *compile_internal(Nst_Node *node, bool is_func);
static void compile_node(Nst_Node *node);
static void compile_long_s(Nst_Node *node);
static void compile_while_l(Nst_Node *node);
static void compile_dowhile_l(Nst_Node *node);
static void compile_for_l(Nst_Node *node);
static void compile_for_as_l(Nst_Node *node);
static void compile_if_e(Nst_Node *node);
static void compile_func_declr(Nst_Node *node);
static void compile_return_s(Nst_Node *node);
static void compile_stack_op(Nst_Node *node);
static void compile_local_stack_op(Nst_Node *node);
static void compile_local_op(Nst_Node *node);
static void compile_arr_or_vect_lit(Nst_Node *node);
static void compile_map_lit(Nst_Node *node);
static void compile_value(Nst_Node *node);
static void compile_access(Nst_Node *node);
static void compile_extract_e(Nst_Node *node);
static void compile_assign_e(Nst_Node *node);
static void compile_continue_s(Nst_Node *node);
static void compile_break_s(Nst_Node *node);
static void compile_switch_s(Nst_Node *node);

Nst_InstructionList *nst_compile(Nst_Node *code)
{
    return compile_internal(code, false);
}

static Nst_InstructionList *compile_internal(Nst_Node *code, bool is_func)
{
    c_state.loop_id = 0;
    c_state.inst_ls = LList_new();
    Nst_InstructionList *inst_list = malloc(sizeof(Nst_InstructionList));
    if ( inst_list == NULL )
        return NULL;

    compile_node(code);

    bool add_return = c_state.inst_ls->tail == NULL ||
        INST(c_state.inst_ls->tail->value)->id != NST_IC_RETURN_VAL;

    inst_list->total_size = c_state.inst_ls->size + (add_return ? 2 : 0);
    inst_list->instructions = malloc(inst_list->total_size * sizeof(Nst_RuntimeInstruction));
    if ( inst_list->instructions == NULL )
    {
        free(inst_list);
        LList_destroy(c_state.inst_ls, free);
        return NULL;
    }

    size_t i = 0;
    for ( LLNode *n = c_state.inst_ls->head;
        n != NULL;
        n = n->next )
    {
        inst_list->instructions[i].id = INST(n->value)->id;
        inst_list->instructions[i].int_val = INST(n->value)->int_val;
        inst_list->instructions[i].val = INST(n->value)->val;
        inst_list->instructions[i].start = INST(n->value)->start;
        inst_list->instructions[i++].end = INST(n->value)->end;
    }

    if ( add_return )
    {
        inst_list->instructions[i].id = NST_IC_PUSH_VAL;
        inst_list->instructions[i].int_val = 0;
        inst_list->instructions[i].val = inc_ref(nst_null);
        inst_list->instructions[i].start = nst_no_pos();
        inst_list->instructions[i++].end = nst_no_pos();

        inst_list->instructions[i].id = NST_IC_RETURN_VAL;
        inst_list->instructions[i].int_val = 0;
        inst_list->instructions[i].val = NULL;
        inst_list->instructions[i].start = nst_no_pos();
        inst_list->instructions[i].end = nst_no_pos();
    }

    LList_destroy(c_state.inst_ls, free); // To not decrease the references of the objects
    if ( !is_func ) nst_destroy_node(code);
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
        compile_node(cursor->value);

        if ( NODE_RETUNS_VALUE(NODE(cursor->value)->type) )
        {
            Nst_RuntimeInstruction *inst = new_inst_empty(NST_IC_POP_VAL, 0);
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

    Nst_RuntimeInstruction *inst;
    Nst_RuntimeInstruction *jump_cond_start = new_inst_empty(NST_IC_JUMP, 0);
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

    inst = new_inst_empty(NST_IC_JUMPIF_T, body_start_idx);
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
                inst->int_val = cond_start_idx;
            // Break statement
            else if ( inst->int_val == c_state.loop_id - 1 )
                inst->int_val = CURR_LEN;
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
    Nst_RuntimeInstruction *inst;
    LLNode *body_start = c_state.inst_ls->tail;

    inc_loop_id();
    Nst_Int body_start_idx = CURR_LEN;
    compile_node(TAIL_NODE); // body
    LLNode *body_end = c_state.inst_ls->tail;

    Nst_Int cond_start_idx = CURR_LEN;
    compile_node(HEAD_NODE); // condition
    if ( body_start == NULL )
        body_start = c_state.inst_ls->head;
    else
        body_start = body_start->next;
    body_end = body_end->next;

    inst = new_inst_empty(NST_IC_JUMPIF_T, body_start_idx);
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
                inst->int_val = cond_start_idx;
            // Break statement
            else if ( inst->int_val == c_state.loop_id - 1 )
                inst->int_val = CURR_LEN;
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
                DEC_INT
                JUMP cond_start
    body_end:   POP_VAL
                [CODE CONTINUATION]
    */

    Nst_RuntimeInstruction *inst;
    compile_node(HEAD_NODE);

    inst = new_inst_val(
        NST_IC_TYPE_CHECK,
        nst_t_int,
        INST(c_state.inst_ls->tail->value)->start,
        INST(c_state.inst_ls->tail->value)->end
    );
    ADD_INST(inst);

    inst = new_inst_empty(NST_IC_NEW_OBJ, 0);
    ADD_INST(inst);

    Nst_Int cond_start_idx = CURR_LEN;
    Nst_RuntimeInstruction *jump_body_end_idx = new_inst_empty(NST_IC_JUMPIF_ZERO, 0);
    ADD_INST(jump_body_end_idx);
    LLNode *body_start = c_state.inst_ls->tail;

    inc_loop_id();
    compile_node(TAIL_NODE); // body
    inst = new_inst_empty(NST_IC_DEC_INT, 0);
    ADD_INST(inst);
    LLNode *body_end = c_state.inst_ls->tail;

    inst = new_inst_empty(NST_IC_JUMP, cond_start_idx);
    ADD_INST(inst);
    Nst_Int body_end_idx = CURR_LEN;
    jump_body_end_idx->int_val = body_end_idx;

    inst = new_inst_empty(NST_IC_POP_VAL, 0);
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
                inst->int_val = cond_start_idx;
            // Break statement
            else if ( inst->int_val == c_state.loop_id - 1 )
                inst->int_val = body_end_idx;
        }
    }

    dec_loop_id();
}

static void compile_for_as_l(Nst_Node *node)
{
    /*
    For-as loop bytecode

                [ITERATOR CODE]
                FOR_START
    cond_start: FOR_IS_DONE
                JUMPIF_T body_end
                FOR_GET_VAL
                SET_VAL_LOC name
                [BODY CODE]
                FOR_ADVANCE
                JUMP cond_start
    body_end:   [CODE CONTINUATION]
    */

    Nst_RuntimeInstruction *inst;
    compile_node(HEAD_NODE);

    // Initialization
    inst = new_inst_int_val(NST_IC_FOR_START, 0, HEAD_NODE->start, HEAD_NODE->end);
    ADD_INST(inst);

    // Condition
    Nst_Int cond_start_idx = CURR_LEN;
    inst = new_inst_int_val(NST_IC_FOR_IS_DONE, 0, HEAD_NODE->start, HEAD_NODE->end);
    ADD_INST(inst);
    Nst_RuntimeInstruction *jump_body_end = new_inst_empty(NST_IC_JUMPIF_T, 0);
    ADD_INST(jump_body_end);

    // Setting variable
    inst = new_inst_int_val(NST_IC_FOR_GET_VAL, 0, HEAD_NODE->start, HEAD_NODE->end);
    ADD_INST(inst);
    inst = new_inst_val(NST_IC_SET_VAL_LOC, HEAD_TOK->value, nst_no_pos(), nst_no_pos());
    ADD_INST(inst);

    // For loop body
    inc_loop_id();
    LLNode *body_start = c_state.inst_ls->tail;
    compile_node(TAIL_NODE);
    inst = new_inst_int_val(NST_IC_FOR_ADVANCE, 0, HEAD_NODE->start, HEAD_NODE->end);
    ADD_INST(inst);
    body_start = body_start->next;
    LLNode *body_end = c_state.inst_ls->tail;
    inst = new_inst_empty(NST_IC_JUMP, cond_start_idx);
    ADD_INST(inst);

    jump_body_end->int_val = CURR_LEN;

    for ( LLNode *cursor = body_start;
        cursor != NULL && cursor != body_end;
        cursor = cursor->next )
    {
        inst = INST(cursor->value);

        if ( IS_JUMP(inst->id) )
        {
            // Continue statement
            if ( inst->int_val == c_state.loop_id )
                inst->int_val = cond_start_idx;
            // Break statement
            else if ( inst->int_val == c_state.loop_id - 1 )
                inst->int_val = CURR_LEN;
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

    Nst_RuntimeInstruction *inst;
    Nst_RuntimeInstruction *jump_if_false = new_inst_empty(NST_IC_JUMPIF_F, 0);
    ADD_INST(jump_if_false);

    compile_node(node->nodes->head->next->value); // Body if true
    if ( NODE(node->nodes->head->next->value)->type == NST_NT_LONG_S )
    {
        inst = new_inst_val(NST_IC_PUSH_VAL, nst_null, nst_no_pos(), nst_no_pos());
        ADD_INST(inst);
    }

    Nst_RuntimeInstruction *jump_at_end = new_inst_empty(NST_IC_JUMP, 0);
    ADD_INST(jump_at_end);
    jump_if_false->int_val = CURR_LEN;

    if ( node->nodes->size == 2 )
    {
        inst = new_inst_val(NST_IC_PUSH_VAL, nst_null, nst_no_pos(), nst_no_pos());
        ADD_INST(inst);
    }
    else
    {
        compile_node(TAIL_NODE); // Body if false
        if ( TAIL_NODE == NST_NT_LONG_S )
        {
            inst = new_inst_val(
                NST_IC_PUSH_VAL,
                nst_null,
                nst_no_pos(),
                nst_no_pos()
            );
            ADD_INST(inst);
        }
    }

    jump_at_end->int_val = CURR_LEN;
}

static void compile_func_declr(Nst_Node *node)
{
    /*
    Func declaration bytecode

    PUSH_VAL function
    SET_VAL_LOC name
    */

    Nst_BcFuncObj *func = AS_BFUNC(new_bfunc(node->tokens->size - 1));
    register size_t i = 0;

    for ( LLNode *n = node->tokens->head->next; n != NULL; n = n->next )
        func->args[i++] = TOK(n->value)->value;

    int prev_loop_id = c_state.loop_id;
    LList *prev_inst_ls = c_state.inst_ls;
    func->body = compile_internal(HEAD_NODE, true);
    c_state.loop_id = prev_loop_id;
    c_state.inst_ls = prev_inst_ls;

    Nst_RuntimeInstruction *inst = new_inst_val(
        NST_IC_PUSH_VAL,
        (Nst_Obj *)func,
        nst_no_pos(),
        nst_no_pos()
    );
    ADD_INST(inst);

    inst = new_inst_val(
        NST_IC_SET_VAL_LOC,
        HEAD_TOK->value,
        nst_no_pos(),
        nst_no_pos()
    );
    ADD_INST(inst);
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
    Nst_RuntimeInstruction *inst = new_inst_empty(NST_IC_RETURN_VAL, 0);
    ADD_INST(inst);
}

static void compile_stack_op(Nst_Node *node)
{
    /*
    General stack operation bytecode

    [VAL 1 CODE]
    [VAL 2 CODE]
    OP_STACK - operator

    L_AND bytecode

           [VAL 1 CODE]
           DUP
           JUMPIF_F e_end
           [VAL 2 CODE]
           OP_STACK &&
    e_end: [CODE CONTINUATION]

    L_OR bytecode

           [VAL 1 CODE]
           DUP
           JUMPIF_T e_end
           [VAL 2 CODE]
           OP_STACK &&
    e_end: [CODE CONTINUATION]
    */

    Nst_RuntimeInstruction *inst;
    Nst_RuntimeInstruction *jump_to_end = NULL;
    compile_node(HEAD_NODE);

    if ( HEAD_TOK->type == NST_TT_L_AND )
    {
        inst = new_inst_empty(NST_IC_DUP, 0);
        ADD_INST(inst);
        jump_to_end = new_inst_empty(NST_IC_JUMPIF_F, 0);
        ADD_INST(jump_to_end);
    }
    else if ( HEAD_TOK->type == NST_TT_L_OR )
    {
        inst = new_inst_empty(NST_IC_DUP, 0);
        ADD_INST(inst);
        jump_to_end = new_inst_empty(NST_IC_JUMPIF_T, 0);
        ADD_INST(jump_to_end);
    }

    compile_node(TAIL_NODE);
    inst = new_inst_int_val(
        NST_IC_STACK_OP,
        HEAD_TOK->type,
        node->start,
        node->end
    );
    ADD_INST(inst);

    if ( jump_to_end != NULL )
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
    PUSH_FUNC
    OP_CALL - arg num
    POP_FUNC
    */

    int tok_type = HEAD_TOK->type;
    Nst_RuntimeInstruction *inst;

    if ( tok_type == NST_TT_RANGE )
    {
        compile_node(HEAD_NODE);
        inst = new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t_int,
            HEAD_NODE->start,
            HEAD_NODE->end
        );
        ADD_INST(inst);

        if ( node->nodes->size == 3 )
        {
            compile_node(node->nodes->head->next->value);
            inst = new_inst_val(
                NST_IC_TYPE_CHECK,
                nst_t_int,
                HEAD_NODE->start,
                HEAD_NODE->end
            );
            ADD_INST(inst);
        }

        compile_node(TAIL_NODE);
        inst = new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t_int,
            HEAD_NODE->start,
            HEAD_NODE->end
        );
        ADD_INST(inst);

        inst = new_inst_empty(NST_IC_OP_RANGE, node->nodes->size);
        ADD_INST(inst);
    }
    else if ( tok_type == NST_TT_CAST )
    {
        compile_node(HEAD_NODE);
        inst = new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t_type,
            HEAD_NODE->start,
            HEAD_NODE->end
        );
        ADD_INST(inst);

        compile_node(TAIL_NODE);
        inst = new_inst_int_val(NST_IC_OP_CAST, 0, node->start, node->end);
        ADD_INST(inst);
    }
    else if ( tok_type == NST_TT_CALL )
    {
        inst = new_inst_empty(NST_IC_PUSH_VAL, 0);
        ADD_INST(inst);

        for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
            compile_node(n->value);

        inst = new_inst_val(NST_IC_TYPE_CHECK, nst_t_func, node->start, node->end);
        ADD_INST(inst);
        inst = new_inst_int_val(NST_IC_PUSH_FUNC, 0, node->start, node->end);
        ADD_INST(inst);
        inst = new_inst_empty(NST_IC_OP_CALL, node->nodes->size - 1);
        ADD_INST(inst);
        inst = new_inst_empty(NST_IC_POP_FUNC, 0);
        ADD_INST(inst);
        // The function object is popped by the RETURN_VAL instruction
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
    PUSH_FUNC
    OP_CALL - arg num
    POP_FUNC

    Other operations bytecode

    [VALUE CODE]
    OP_LOCAL - operator
    */

    Nst_RuntimeInstruction *inst;

    if ( HEAD_TOK->type == NST_TT_LOC_CALL )
    {
        inst = new_inst_empty(NST_IC_PUSH_VAL, 0);
        ADD_INST(inst);
        compile_node(HEAD_NODE);
        inst = new_inst_val(NST_IC_TYPE_CHECK, nst_t_func, node->start, node->end);
        ADD_INST(inst);
        inst = new_inst_int_val(NST_IC_PUSH_FUNC, 0, node->start, node->end);
        ADD_INST(inst);
        inst = new_inst_empty(NST_IC_OP_CALL, node->nodes->size - 1);
        ADD_INST(inst);
        inst = new_inst_empty(NST_IC_POP_FUNC, 0);
        ADD_INST(inst);
        return;
    }

    compile_node(HEAD_NODE);

    if ( HEAD_TOK->type == NST_TT_IMPORT )
    {
        inst = new_inst_val(NST_IC_TYPE_CHECK, nst_t_str, HEAD_NODE->start, HEAD_NODE->end);
        ADD_INST(inst);
        inst = new_inst_int_val(NST_IC_OP_IMPORT, 0, HEAD_NODE->start, HEAD_NODE->end);
        ADD_INST(inst);
        return;
    }

    inst = new_inst_int_val(
        NST_IC_LOCAL_OP,
        HEAD_TOK->type,
        HEAD_NODE->start,
        HEAD_NODE->end
    );

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

    Nst_RuntimeInstruction *inst;
    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
        compile_node(n->value);

    if ( node->tokens->size != 0 )
    {
        inst = new_inst_val(
            NST_IC_TYPE_CHECK,
            nst_t_int,
            TAIL_NODE->start,
            TAIL_NODE->end
        );
        ADD_INST(inst);
        inst = new_inst_empty(
            node->type == NST_NT_ARR_LIT ? NST_IC_MAKE_ARR_REP : NST_IC_MAKE_VEC_REP,
            node->nodes->size
        );
    }
    else
        inst = new_inst_empty(
            node->type == NST_NT_ARR_LIT ? NST_IC_MAKE_ARR : NST_IC_MAKE_VEC,
            node->nodes->size
        );

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
    Nst_RuntimeInstruction *inst;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        compile_node(n->value);
        inst = new_inst_int_val(
            NST_IC_HASH_CHECK,
            0,
            NODE(n->value)->start,
            NODE(n->value)->end
        );
        ADD_INST(inst);
    }

    inst = new_inst_empty(NST_IC_MAKE_MAP, node->nodes->size);
    ADD_INST(inst);
}

static void compile_value(Nst_Node *node)
{
    /*
    Value bytecode

    PUSH_VAL value
    */
    Nst_RuntimeInstruction *inst = new_inst_val(
        NST_IC_PUSH_VAL,
        HEAD_TOK->value,
        node->start,
        node->end
    );
    ADD_INST(inst);
}

static void compile_access(Nst_Node *node)
{
    /*
    Access bytecode

    GET_VAL var_name
    */
    Nst_RuntimeInstruction *inst = new_inst_val(
        NST_IC_GET_VAL,
        HEAD_TOK->value,
        node->start,
        node->end
    );
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

    Nst_RuntimeInstruction *inst = new_inst_pos(
        NST_IC_OP_EXTRACT,
        node->start,
        node->end
    );
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
    Nst_RuntimeInstruction *inst;
    compile_node(HEAD_NODE);

    if ( TAIL_NODE->type == NST_NT_ACCESS )
    {
        inst = new_inst_val(
            NST_IC_SET_VAL,
            TOK(TAIL_NODE->tokens->head->value)->value,
            node->start,
            node->end
        );
    }
    else
    {
        compile_node(NODE(TAIL_NODE->nodes->head->value)); // Container
        compile_node(NODE(TAIL_NODE->nodes->tail->value)); // Index

        inst = new_inst_pos(
            NST_IC_SET_CONT_VAL,
            node->start,
            node->end
        );
    }

    ADD_INST(inst);
}

static void compile_continue_s(Nst_Node *node)
{
    /*
    Continue bytecode

    JUMP loop_id - replaced later with the loop's start
    */
    Nst_RuntimeInstruction *inst = new_inst_int_val(
        NST_IC_JUMP,
        c_state.loop_id,
        node->start,
        node->end
    );

    ADD_INST(inst);
}

static void compile_break_s(Nst_Node *node)
{
    /*
    Continue bytecode

    JUMP loop_id - replaced later with the loop's end
    */
    Nst_RuntimeInstruction *inst = new_inst_int_val(
        NST_IC_JUMP,
        c_state.loop_id - 1,
        node->start,
        node->end
    );

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
    switch_end: [CODE CONTINUATION]
    */

    Nst_RuntimeInstruction *inst;
    LLNode *prev_body_start = NULL;
    LLNode *prev_body_end = NULL;
    Nst_Node *default_body = NULL;
    LList *jumps_to_switch_end = LList_new();

    compile_node(HEAD_NODE);

    size_t i = 1;
    for ( LLNode *n = node->nodes->head->next;
          n != NULL;
          n = n->next )
    {
        if ( n->next == NULL )
        {
            default_body = n->value;
            break;
        }

        inst = new_inst_empty(NST_IC_DUP, 0);
        ADD_INST(inst);
        compile_node(n->value);
        n = n->next;
        inst = new_inst_empty(NST_IC_STACK_OP, NST_TT_EQ);
        ADD_INST(inst);
        Nst_RuntimeInstruction *jump_body_end = new_inst_empty(NST_IC_JUMPIF_F, 0);
        ADD_INST(jump_body_end);
        LLNode *body_start = c_state.inst_ls->tail;

        inc_loop_id();
        Nst_Int next_body_start = CURR_LEN;
        compile_node(n->value);
        inst = new_inst_empty(NST_IC_JUMP, 0);
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
                    inst->int_val = next_body_start;
            }
        }

        dec_loop_id();
        prev_body_start = body_start;
        prev_body_end = body_end;
    }

    if ( prev_body_start != NULL )
    {
        for ( LLNode *cursor = prev_body_start->next;
            cursor != NULL && cursor != prev_body_end;
            cursor = cursor->next )
        {
            inst = INST(cursor->value);

            if ( IS_JUMP(inst->id) && inst->int_val == c_state.loop_id )
                inst->int_val = CURR_LEN;
        }
    }

    if ( default_body != NULL )
        compile_node(default_body);

    for ( LLNode *cursor = jumps_to_switch_end->head;
          cursor != NULL;
          cursor = cursor->next )
        INST(cursor->value)->int_val = CURR_LEN;

    LList_destroy(jumps_to_switch_end, NULL);
}

void nst_print_bytecode(Nst_InstructionList *ls, int indent)
{
    for ( size_t i = 0, n = ls->total_size; i < n; i++ )
    {
        Nst_RuntimeInstruction inst = ls->instructions[i];

        for ( int i = 0; i < indent; i++ ) printf("    ");
        printf("%3zi %02zi:%02zi ", i, inst.start.line + 1, inst.start.col + 1);

        switch ( inst.id )
        {
        case NST_IC_POP_VAL:       printf("POP_VAL      "); break;
        case NST_IC_PUSH_FUNC:     printf("PUSH_FUNC    "); break;
        case NST_IC_POP_FUNC:      printf("POP_FUNC     "); break;
        case NST_IC_JUMP:          printf("JUMP         "); break;
        case NST_IC_JUMPIF_T:      printf("JUMPIF_T     "); break;
        case NST_IC_JUMPIF_F:      printf("JUMPIF_F     "); break;
        case NST_IC_JUMPIF_ZERO:   printf("JUMPIF_ZERO  "); break;
        case NST_IC_SET_VAL:       printf("SET_VAL      "); break;
        case NST_IC_SET_VAL_LOC:   printf("SET_VAL_LOC  "); break;
        case NST_IC_GET_VAL:       printf("GET_VAL      "); break;
        case NST_IC_PUSH_VAL:      printf("PUSH_VAL     "); break;
        case NST_IC_SET_CONT_VAL:  printf("SET_CONT_VAL "); break;
        case NST_IC_OP_CALL:       printf("OP_CALL      "); break;
        case NST_IC_OP_CAST:       printf("OP_CAST      "); break;
        case NST_IC_OP_RANGE:      printf("OP_RANGE     "); break;
        case NST_IC_STACK_OP:      printf("STACK_OP     "); break;
        case NST_IC_LOCAL_OP:      printf("LOCAL_OP     "); break;
        case NST_IC_OP_IMPORT:     printf("OP_IMPORT    "); break;
        case NST_IC_OP_EXTRACT:    printf("OP_EXTRACT   "); break;
        case NST_IC_DEC_INT:       printf("DEC_INT      "); break;
        case NST_IC_NEW_OBJ:       printf("NEW_OBJ      "); break;
        case NST_IC_TYPE_CHECK:    printf("TYPE_CHECK   "); break;
        case NST_IC_HASH_CHECK:    printf("HASH_CHECK   "); break;
        case NST_IC_DUP:           printf("DUP          "); break;
        case NST_IC_MAKE_ARR:      printf("MAKE_ARR     "); break;
        case NST_IC_MAKE_VEC:      printf("MAKE_VEC     "); break;
        case NST_IC_MAKE_MAP:      printf("MAKE_MAP     "); break;
        case NST_IC_FOR_START:     printf("FOR_START    "); break;
        case NST_IC_FOR_ADVANCE:   printf("FOR_ADVANCE  "); break;
        case NST_IC_FOR_IS_DONE:   printf("FOR_IS_DONE  "); break;
        case NST_IC_FOR_GET_VAL:   printf("FOR_GET_VAL  "); break;
        case NST_IC_RETURN_VAL:    printf("RETURN_VAL   "); break;
        //case MAKE_FUNC:     printf("MAKE_FUNC    "); break;
        default:            printf("__UNKNOWN__  "); break;
        }

        printf("%3lli ", inst.int_val);

        if ( inst.val != NULL )
        {
            if ( inst.val->type == nst_t_str )
            {
                Nst_StrObj *s = AS_STR(_nst_repr_string(AS_STR(inst.val)));
                printf("%s", s->value);
                dec_ref(s);
            }
            else
                nst_obj_stdout(inst.val, NULL);

            if ( inst.val->type == nst_t_func )
            {
                printf("\n\n");
                for ( int i = 0; i < indent + 1; i++ ) printf("    ");
                printf("<Func object> bytecode:\n");
                nst_print_bytecode(AS_BFUNC(inst.val)->body, indent + 1);
            }
        }
        else if ( inst.id == NST_IC_PUSH_VAL )
            printf("NULL");

        printf("\n");
    }
}
