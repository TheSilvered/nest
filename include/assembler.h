/**
 * @file assembler.h
 *
 * @file Assemble an instruction sequence into bytecode.
 *
 * @author TheSilvered
 */

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "error.h"

#define Nst_OP_CODE(op) ((Nst_OpCode)(((op) & 0xFF00) >> 8))
#define Nst_OP_ARG(op) ((u8)((op) & 0x00FF))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* Instruction IDs in the Nest virtual machine. */
NstEXP typedef enum _Nst_OpCode {
    Nst_OP_POP_VAL,
    Nst_OP_FOR_START,
    Nst_OP_FOR_NEXT,
    Nst_OP_RETURN_VAL,
    Nst_OP_RETURN_VARS,
    Nst_OP_SET_VAL_LOC,
    Nst_OP_SET_CONT_LOC,
    Nst_OP_THROW_ERR,
    Nst_OP_POP_CATCH,
    Nst_OP_SET_VAL,
    Nst_OP_GET_VAL,
    Nst_OP_PUSH_VAL,
    Nst_OP_SET_CONT_VAL,
    Nst_OP_CALL,
    Nst_OP_SEQ_CALL,
    Nst_OP_CAST,
    Nst_OP_RANGE,
    Nst_OP_STACK,
    Nst_OP_LOCAL,
    Nst_OP_IMPORT,
    Nst_OP_EXTRACT,
    Nst_OP_DEC_INT,
    Nst_OP_NEW_INT,
    Nst_OP_DUP,
    Nst_OP_ROT_2,
    Nst_OP_ROT_3,
    Nst_OP_MAKE_ARR,
    Nst_OP_MAKE_ARR_REP,
    Nst_OP_MAKE_VEC,
    Nst_OP_MAKE_VEC_REP,
    Nst_OP_MAKE_MAP,
    Nst_OP_MAKE_FUNC,
    Nst_OP_SAVE_ERROR,
    Nst_OP_UNPACK_SEQ,
    Nst_OP_EXTEND_ARG,
    Nst_OP_JUMP,
    Nst_OP_JUMPIF_T,
    Nst_OP_JUMPIF_F,
    Nst_OP_JUMPIF_ZERO,
    Nst_OP_JUMPIF_IEND,
    Nst_OP_PUSH_CATCH
} Nst_OpCode;

/* A bytecode instruction. */
NstEXP typedef u16 Nst_Op;

/**
 * The structure representing Nest bytecode.
 *
 * @param copy_count: the number of times it has been copied
 * @param len: the total number of instructions
 * @param bytecode: bytecode instructions
 * @param positions: the positions of the instructions
 * @param obj_count: the number of objects in `objects`
 * @param objects: the array of objects used by the bytecode
 */
NstEXP typedef struct _Nst_Bytecode {
    usize copy_count;
    usize len;
    Nst_Op *bytecode;
    Nst_Span *positions;
    usize obj_len;
    Nst_ObjRef **objects;
} Nst_Bytecode;

/**
 * Assemble an `Nst_InstList` into bytecode. The bytecode is heap allocated and
 * must be freed with `Nst_bc_destroy`.
 *
 * @return The new bytecode or `NULL` on failure. The error is set.
 */
NstEXP Nst_Bytecode *NstC Nst_assemble(Nst_InstList *ilist);
/* Make a copy of an `Nst_Bytecode`. Success is guaranteed. */
NstEXP Nst_Bytecode *NstC Nst_bc_copy(Nst_Bytecode *bc);
/**
 * Destroy and free an `Nst_Bytecode`. This function must be called for every
 * copy made.
 */
NstEXP void NstC Nst_bc_destroy(Nst_Bytecode *bc);
/* Print the bytecode to the standard output. */
NstEXP void NstC Nst_bc_print(Nst_Bytecode *bc);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ASSEMBLER_H
