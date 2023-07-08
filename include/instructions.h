/* Bytecode instruction interface */

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "simple_types.h"
#include "error.h"

#define NST_INST_IS_JUMP(inst) ( (inst >= NST_IC_JUMP && inst <= NST_IC_JUMPIF_ZERO) \
                                 || inst == NST_IC_PUSH_CATCH )
// Creates a new instruction on the heap with positions and a value
#define nst_inst_new_val(id, val, start, end) \
    _nst_inst_new_val(id, OBJ(val), start, end)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef enum _Nst_InstID
{
    NST_IC_NO_OP,
    NST_IC_POP_VAL, // Pop a value from the value stack
    NST_IC_FOR_START,
    NST_IC_FOR_IS_DONE,
    NST_IC_FOR_GET_VAL,
    NST_IC_RETURN_VAL,
    NST_IC_RETURN_VARS, // Returns the var table
    NST_IC_SET_VAL_LOC, // Assigns the value without leaving on on the stack
    NST_IC_SET_CONT_LOC, // Assigns the value of a sequence without leaving it
                         // on the stack
    NST_IC_JUMP, // Jump to an index in the instruction array
    NST_IC_JUMPIF_T, // Jump to an index in the instruction array if the top
                     // value is truthy, consumes the value
    NST_IC_JUMPIF_F, // Jump to an index in the instruction array if the top
                     // value is falsy, consumes the value
    NST_IC_JUMPIF_ZERO, // Jumps if the top item is an integer and is zero
    NST_IC_TYPE_CHECK, // Checks the type of the top value on the stack
    NST_IC_HASH_CHECK, // Checks the type on top of the stack is hashable
    NST_IC_THROW_ERR, // throws an error
    NST_IC_PUSH_CATCH, // pushes the start index of a catch block
    NST_IC_POP_CATCH, // pops the top index in the catch stack

    // These instruction push a value on the stack
    NST_IC_SET_VAL, // Sets a value in the current var table
    NST_IC_GET_VAL, // Gets a value in the current var table
    NST_IC_PUSH_VAL, // Push a value onto the value stack
    NST_IC_SET_CONT_VAL, // Sets a value in a map, array or vector
    NST_IC_OP_CALL, // Call the top function on the call stack
    NST_IC_OP_CAST, // Local-stack operator
    NST_IC_OP_RANGE, // Local-stack operator
    NST_IC_STACK_OP, // Stack operator
    NST_IC_LOCAL_OP, // Local operator
    NST_IC_OP_IMPORT, // Module/library import
    NST_IC_OP_EXTRACT, // Index a value of a string, array, vector or map
    NST_IC_DEC_INT, // decrease an integer by one
    NST_IC_NEW_OBJ, // duplicates the integer at the top creating a new object
    NST_IC_DUP, // duplicates the top value on the stack
    NST_IC_ROT, // rotates N items on the stack
    NST_IC_MAKE_ARR, // makes an array of length N consuming N values
    NST_IC_MAKE_ARR_REP, // makes an array repeating the same object consuming
                         // 2 values from the stack
    NST_IC_MAKE_VEC, // makes a vector instead of an array
    NST_IC_MAKE_VEC_REP,
    NST_IC_MAKE_MAP, // makes a map of size N/2 consuming N values from the stack
    NST_IC_SAVE_ERROR, // creates a map with the current error's info and pushes
                       // it on the stack
    NST_IC_UNPACK_SEQ // pushes the values of the sequence on top of the stack
                      // from the last to the first
}
Nst_InstID;

NstEXP typedef struct _Nst_Instruction
{
    Nst_InstID id;
    Nst_Int int_val;
    Nst_Obj *val;
    Nst_Pos start;
    Nst_Pos end;
}
Nst_Inst;

NstEXP typedef struct _Nst_InstList
{
    usize total_size;
    Nst_Inst *instructions;
    Nst_LList *functions; // all the functions declared inside the bytecode
}
Nst_InstList;

// Creates a new instruction on the heap with positions
NstEXP Nst_Inst *nst_inst_new(Nst_InstID id, Nst_Pos start, Nst_Pos end);
NstEXP Nst_Inst *_nst_inst_new_val(Nst_InstID id,
                                   Nst_Obj   *val,
                                   Nst_Pos    start,
                                   Nst_Pos    end);
// Creates a new instruction on the heap with positions and an integer value
NstEXP Nst_Inst *nst_inst_new_int(Nst_InstID id,
                                  Nst_Int    int_val,
                                  Nst_Pos    start,
                                  Nst_Pos    end);

// Destroys an instruction allocated on the heap
NstEXP void nst_inst_destroy(Nst_Inst *inst);
// Destroys an instruction list
NstEXP void nst_inst_list_destroy(Nst_InstList *inst_list);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // INSTRUCTIONS_H
