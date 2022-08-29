#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "simple_types.h"
#include "error.h"

#define IS_JUMP(inst) ( inst >= NST_IC_JUMP && inst <= NST_IC_JUMPIF_ZERO )

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    int id;
    Nst_Int int_val;
    Nst_Obj *val;
    Nst_Pos start;
    Nst_Pos end;
}
Nst_RuntimeInstruction;

typedef struct
{
    size_t total_size;
    Nst_RuntimeInstruction *instructions;
}
Nst_InstructionList;

Nst_RuntimeInstruction *new_inst_empty(int id, Nst_Int int_val);
Nst_RuntimeInstruction *new_inst_pos(int id, Nst_Pos start, Nst_Pos end);
Nst_RuntimeInstruction *new_inst_val(int id,
                                     Nst_Obj *val,
                                     Nst_Pos start,
                                     Nst_Pos end);
Nst_RuntimeInstruction *new_inst_int_val(int id,
                                         Nst_Int int_val,
                                         Nst_Pos start,
                                         Nst_Pos end);

void nst_destroy_inst(Nst_RuntimeInstruction *inst);
void nst_destroy_inst_list(Nst_InstructionList *inst_list);

enum Nst_InstructionCodes
{
    NST_IC_NO_OP,
    NST_IC_POP_VAL, // Pop a value from the value stack
    NST_IC_FOR_START,
    NST_IC_RETURN_VAL,
    NST_IC_FOR_ADVANCE,
    NST_IC_SET_VAL_LOC, // Does not push its value onto the stack
    NST_IC_JUMP, // Jump to an index in the instuction array
    NST_IC_JUMPIF_T, // Jump to an index in the instuction array if the top value is nst_true, consumes the value
    NST_IC_JUMPIF_F, // Jump to an index in the instuction array if the top value is nst_false, consumes the value
    NST_IC_JUMPIF_ZERO, // Jumps if the top item is an integer and is zero
    NST_IC_TYPE_CHECK, // Checks the type of the top value on the stack
    NST_IC_HASH_CHECK, // Checks the type on top of the stack is hashable

    // These instruction push a value on the value stack
    NST_IC_SET_VAL, // Sets a value in the current var table
    NST_IC_GET_VAL, // Gets a value in the current var table
    NST_IC_PUSH_VAL, // Push a value onto the value stack
    NST_IC_SET_CONT_VAL, // Sets a value in a map, array or vector
    NST_IC_OP_CALL, // Call the top function on the call stack, consumes the function
    NST_IC_OP_CAST, // Local-stack operator
    NST_IC_OP_RANGE, // Local-stack operator
    NST_IC_STACK_OP, // Stack operator
    NST_IC_LOCAL_OP, // Local operator
    NST_IC_OP_IMPORT, // Module/library import
    NST_IC_OP_EXTRACT, // Index a value of a string, array, vector or map
    NST_IC_DEC_INT, // decrease an integer by one
    NST_IC_NEW_OBJ, // substitutes the top object with a new instance of said object
    NST_IC_DUP, // duplicates the top value on the stack
    NST_IC_MAKE_ARR,
    NST_IC_MAKE_ARR_REP,
    NST_IC_MAKE_VEC,
    NST_IC_MAKE_VEC_REP,
    NST_IC_MAKE_MAP,
    NST_IC_FOR_IS_DONE,
    NST_IC_FOR_GET_VAL
};

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // INSTRUCTIONS_H
