/**
 * @file function.h
 *
 * @brief `Func` object interface
 *
 * @author TheSilvered
 */

#ifndef FUNCTION_H
#define FUNCTION_H

#include "compiler.h"
#include "map.h"

#define Nst_FUNC_IS_C(func) ((func)->flags & Nst_FLAG_FUNC_IS_C)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Creates a new function object with an instruction-list body.
 *
 * @brief Note: the `args` array must be set manually after instantiation.
 *
 * @param arg_num: the maximum number of arguments the function accepts
 * @param bytecode: the body of the function
 *
 * @return The new function object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_func_new(usize arg_num, Nst_InstList *bytecode);
/**
 * Creates a new function object with a C function body.
 *
 * @brief Note: the `args` array must NOT be set since it is not used.
 *
 * @param arg_num: the maximum number of arguments the function accepts
 * @param cbody: the body of the function
 *
 * @return The new function object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_func_new_c(usize arg_num, Nst_NestCallable cbody);
/**
 * Sets the `mod_globals` table of a function and all the functions defined
 * inside it.
 *
 * @brief If the field is already set or the function has a C body, it is not
 * modified.
 *
 * @param func: the function to change the `mod_globals` field of
 * @param map: the map to set as the new value
 */
NstEXP void NstC Nst_func_set_vt(Nst_Obj *func, Nst_Obj *map);

/* Gets the number of arguments a function takes. */
NstEXP usize NstC Nst_func_arg_num(Nst_Obj *func);
/* Gets the argument names as a list of objects. */
NstEXP Nst_Obj **NstC Nst_func_args(Nst_Obj *func);
/* Gets the body of a C-function wrapper. */
NstEXP Nst_NestCallable NstC Nst_func_c_body(Nst_Obj *func);
/* Gets the body of a Nest function. */
NstEXP Nst_InstList *NstC Nst_func_nest_body(Nst_Obj *func);
/**
 * @brief Gets the `_globals_` variable map of a function. No reference is
 * added. It may be `NULL`.
 */
NstEXP Nst_Obj *NstC Nst_func_mod_globals(Nst_Obj *func);

/* Traverse function for `Func` objects. */
NstEXP void NstC _Nst_func_traverse(Nst_Obj *func);

void NstC _Nst_func_destroy(Nst_Obj *func);

/* Flags for `Func` objects. */
NstEXP typedef enum _Nst_FuncFlags {
    Nst_FLAG_FUNC_IS_C = Nst_FLAG(1)
} Nst_FuncFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FUNCTION_H
