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

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* The code of a `Nst_Inst`. */
NstEXP typedef enum _Nst_InstCode {
    Nst_IC_NO_OP,
    Nst_IC_POP_VAL,
    Nst_IC_FOR_START,
    Nst_IC_FOR_NEXT,
    Nst_IC_RETURN_VAL,
    Nst_IC_RETURN_VARS,
    Nst_IC_SET_VAL_LOC,
    Nst_IC_SET_CONT_LOC,
    Nst_IC_THROW_ERR,
    Nst_IC_POP_CATCH,
    Nst_IC_SET_VAL,
    Nst_IC_GET_VAL,
    Nst_IC_PUSH_VAL,
    Nst_IC_SET_CONT_VAL,
    Nst_IC_OP_CALL,
    Nst_IC_OP_SEQ_CALL,
    Nst_IC_OP_CAST,
    Nst_IC_OP_RANGE,
    Nst_IC_STACK_OP,
    Nst_IC_LOCAL_OP,
    Nst_IC_OP_IMPORT,
    Nst_IC_OP_EXTRACT,
    Nst_IC_DEC_INT,
    Nst_IC_NEW_INT,
    Nst_IC_DUP,
    Nst_IC_ROT_2,
    Nst_IC_ROT_3,
    Nst_IC_MAKE_ARR,
    Nst_IC_MAKE_ARR_REP,
    Nst_IC_MAKE_VEC,
    Nst_IC_MAKE_VEC_REP,
    Nst_IC_MAKE_MAP,
    Nst_IC_MAKE_FUNC,
    Nst_IC_SAVE_ERROR,
    Nst_IC_UNPACK_SEQ,
    Nst_IC_JUMP,
    Nst_IC_JUMPIF_T,
    Nst_IC_JUMPIF_F,
    Nst_IC_JUMPIF_ZERO,
    Nst_IC_JUMPIF_IEND,
    Nst_IC_PUSH_CATCH
} Nst_InstCode;

/**
 * A structure representing a Nest instruction.
 *
 * @param code: the code of the instruction
 * @param span: the position of the instruction in the code
 * @param val: the integer value of the instruction
 */
NstEXP typedef struct _Nst_Inst {
    Nst_InstCode code;
    Nst_Span span;
    i64 val;
} Nst_Inst;

/**
 * A structure representing a list of instructions.
 *
 * @param instructions: the list of instructions
 * @param objects: the list of objects that the instructions reference
 * @param functions: the list of `Nst_FuncPrototype` of the functions defined
 * in the file
 */
NstEXP typedef struct _Nst_InstList {
    Nst_DynArray instructions;
    Nst_DynArray objects;
    Nst_DynArray functions;
} Nst_InstList;

/**
 * A structure representing a function before it is assembled.
 *
 * @param ilist: the body of the function
 * @param arg_names: the names of the arguments as an array of `Str` objects
 * @param arg_num: the number of arguments the function can accept
 */
NstEXP typedef struct _Nst_FuncPrototype {
    Nst_InstList ilist;
    Nst_ObjRef **arg_names;
    usize arg_num;
} Nst_FuncPrototype;

/* Check if an instruction code is a jump instruction. */
NstEXP bool NstC Nst_ic_is_jump(Nst_InstCode code);

/* Initialize an `Nst_InstList`. */
NstEXP bool NstC Nst_ilist_init(Nst_InstList *list);
/* Destroy the contents of an `Nst_InstList`. */
NstEXP void NstC Nst_ilist_destroy(Nst_InstList *list);

/**
 * Add an instruction to an `Nst_InstList`. The `val` field of the instruction
 * is set to `0`.
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_ilist_add(Nst_InstList *list, Nst_InstCode code,
                               Nst_Span span);
/**
 * Add an instruction to an `Nst_InstList` specifying its `val` field.
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_ilist_add_ex(Nst_InstList *list, Nst_InstCode code,
                                  i64 val, Nst_Span span);

/**
 * Append an object to an `Nst_InstList`. A reference is taken from `obj`.
 *
 * @return The index of the added object or `-1` on failure. The error is set.
 */
NstEXP isize NstC Nst_ilist_add_obj(Nst_InstList *list, Nst_ObjRef *obj);
/**
 * Append a function prototype to an `Nst_InstList`.
 *
 * @return The index of the added prototype or `-1` on failure. The error is
 * set.
 */
NstEXP isize NstC Nst_ilist_add_func(Nst_InstList *list,
                                     Nst_FuncPrototype *fp);

/**
 * @return The instruction at index `idx` of an `Nst_InstList`. If the index is
 * out of bounds `NULL` is returned. No error is set.
 */
NstEXP Nst_Inst *NstC Nst_ilist_get_inst(Nst_InstList *list, usize idx);
/**
 * @return The object at index `idx` of an `Nst_InstList`. If the index is
 * out of bounds `NULL` is returned. No error is set.
 */
NstEXP Nst_Obj *NstC Nst_ilist_get_obj(Nst_InstList *list, usize idx);
/**
 * @return The function prototype at index `idx` of an `Nst_InstList`. If the
 * index is out of bounds `NULL` is returned. No error is set.
 */
NstEXP Nst_FuncPrototype *NstC Nst_ilist_get_func(Nst_InstList *list,
                                                  usize idx);
/**
 * @return The object associated with the instruction at `idx`. The equivalent
 * of getting an instruction with `Nst_ilist_get_inst` and then getting the
 * object with `Nst_ilist_get_obj` using the value of the instruction as the
 * index.
 */
NstEXP Nst_Obj *NstC Nst_ilist_get_inst_obj(Nst_InstList *list, usize idx);
/**
 * @return The function prototype associated with the instruction at `idx`. The
 * equivalent of getting an instruction with `Nst_ilist_get_inst` and then
 * getting the prototype with `Nst_ilist_get_func` using the value of the
 * instruction as the index.
 */
NstEXP Nst_FuncPrototype *NstC Nst_ilist_get_inst_func(Nst_InstList *list,
                                                       usize idx);

/* Set the code of the instruction at index `idx`. Its value is set to `0`. */
NstEXP void NstC Nst_ilist_set(Nst_InstList *list, usize idx,
                               Nst_InstCode code);
/* Set the code and the value of the instruction at index `idx`. */
NstEXP void NstC Nst_ilist_set_ex(Nst_InstList *list, usize idx,
                                  Nst_InstCode code, i64 val);
/* @return The number of instructions in an `Nst_InstList`. */
NstEXP usize NstC Nst_ilist_len(Nst_InstList *list);

/* Print an `Nst_InstList` to the standard output. */
NstEXP void NstC Nst_ilist_print(Nst_InstList *list);

/* Initialize a `Nst_FuncPrototype`. */
NstEXP bool NstC Nst_fprototype_init(Nst_FuncPrototype *fp, Nst_InstList ls,
                                     usize arg_num);
/* Destroy the contents of a `Nst_FuncPrototype`. */
NstEXP void NstC Nst_fprototype_destroy(Nst_FuncPrototype *fp);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // INSTRUCTIONS_H
