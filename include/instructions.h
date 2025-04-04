/**
 * @file instructions.h
 *
 * @brief Bytecode instruction interface
 *
 * @author TheSilvered
 */

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "simple_types.h"
#include "error.h"
#include "llist.h"

/* Checks whether a given instruction ID represents a jump instruction. */
#define Nst_INST_IS_JUMP(inst_id)                                             \
    ((inst_id) >= Nst_IC_JUMP && (inst_id) <= Nst_IC_PUSH_CATCH)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// IMPORTANT when changing the order of the instructions, chance the order of
// the functions in inst_func in interpreter.c

/* Instruction IDs in the Nest virtual machine. */
NstEXP typedef enum _Nst_InstID {
    Nst_IC_NO_OP,
    Nst_IC_POP_VAL,
    Nst_IC_FOR_START,
    Nst_IC_FOR_NEXT,
    Nst_IC_RETURN_VAL,
    Nst_IC_RETURN_VARS,
    Nst_IC_SET_VAL_LOC,
    Nst_IC_SET_CONT_LOC,
    Nst_IC_JUMP,
    Nst_IC_JUMPIF_T,
    Nst_IC_JUMPIF_F,
    Nst_IC_JUMPIF_ZERO,
    Nst_IC_JUMPIF_IEND,
    Nst_IC_PUSH_CATCH,
    Nst_IC_HASH_CHECK,
    Nst_IC_THROW_ERR,
    Nst_IC_POP_CATCH,
    Nst_IC_SET_VAL,
    Nst_IC_GET_VAL,
    Nst_IC_PUSH_VAL,
    Nst_IC_SET_CONT_VAL,
    Nst_IC_OP_CALL,
    Nst_IC_OP_CAST,
    Nst_IC_OP_RANGE,
    Nst_IC_STACK_OP,
    Nst_IC_LOCAL_OP,
    Nst_IC_OP_IMPORT,
    Nst_IC_OP_EXTRACT,
    Nst_IC_DEC_INT,
    Nst_IC_NEW_INT,
    Nst_IC_DUP,
    Nst_IC_ROT,
    Nst_IC_MAKE_ARR,
    Nst_IC_MAKE_ARR_REP,
    Nst_IC_MAKE_VEC,
    Nst_IC_MAKE_VEC_REP,
    Nst_IC_MAKE_MAP,
    Nst_IC_MAKE_FUNC,
    Nst_IC_SAVE_ERROR,
    Nst_IC_UNPACK_SEQ
} Nst_InstID;

/**
 * The structure representing an instruction in Nest.
 *
 * @param id: the ID of the instruction
 * @param int_val: an integer value used by the instruction
 * @param val: an object used by the instruction
 * @param span: the span of the instruction
 */
NstEXP typedef struct _Nst_Inst {
    Nst_InstID id;
    i64 int_val;
    Nst_Obj *val;
    Nst_Span span;
} Nst_Inst;

/**
 * The structure representing a list of instructions in Nest.
 *
 * @param total_size: the total number of instructions in the list
 * @param instructions: the array of instructions
 * @param functions: the list of functions declared withing the bytecode
 */
NstEXP typedef struct _Nst_InstList {
    usize ref_count;
    usize total_size;
    Nst_Inst *instructions;
    Nst_LList *functions;
} Nst_InstList;

/**
 * Creates a new instruction on the heap.
 *
 * @param id: the id of the instruction to create
 * @param start: the start position of the instruction
 * @param end: the end position of the instruction
 *
 * @return The new instruction or `NULL` on failure. The error is set.
 */
NstEXP Nst_Inst *NstC Nst_inst_new(Nst_InstID id, Nst_Span span);
/**
 * Creates a new instruction on the heap with a Nest object value.
 *
 * @brief The reference count of `val` is increased.
 *
 * @param id: the id of the instruction to create
 * @param val: the Nest object value
 * @param start: the start position of the instruction
 * @param end: the end position of the instruction
 *
 * @return The new instruction or `NULL` on failure. The error is set.
 */
NstEXP Nst_Inst *NstC Nst_inst_new_val(Nst_InstID id, Nst_Obj *val,
                                       Nst_Span span);
/**
 * Creates a new instruction on the heap with an integer value.
 *
 * @param id: the id of the instruction to create
 * @param int_val: the integer value
 * @param start: the start position of the instruction
 * @param end: the end position of the instruction
 *
 * @return The new instruction or `NULL` on failure. The error is set.
 */
NstEXP Nst_Inst *NstC Nst_inst_new_int(Nst_InstID id, i64 int_val,
                                       Nst_Span span);

/* Destroys a `Nst_Inst` allocated on the heap. */
NstEXP void NstC Nst_inst_destroy(Nst_Inst *inst);

/* Creates a new `Nst_InstList` from a list of instructions. */
NstEXP Nst_InstList *NstC Nst_inst_list_new(Nst_LList *instructions);
/* Copy a `Nst_InstList`. */
NstEXP Nst_InstList *NstC Nst_inst_list_copy(Nst_InstList *inst_list);

/* Destroys a `Nst_InstList`. */
NstEXP void NstC Nst_inst_list_destroy(Nst_InstList *inst_list);

/**
 * Prints an `Nst_InstList`.
 *
 * @brief This function is called when using the -b option.
 *
 * @param ls: the instruction list to print, it is expected to be valid
 */
NstEXP void NstC Nst_inst_list_print(Nst_InstList *ls);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // INSTRUCTIONS_H
