/**
 * @file obj_ops.h
 *
 * @brief Various object operations
 *
 * @author TheSilvered
 */

#ifndef OBJ_OPS_H
#define OBJ_OPS_H

#include "map.h"
#include "sequence.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* Calls `Nst_obj_eq` making the result a C bool. */
NstEXP bool NstC Nst_obj_eq_c(Nst_Obj *ob1, Nst_Obj *ob2);
/* Calls `Nst_obj_ne` making the result a C bool. */
NstEXP bool NstC Nst_obj_ne_c(Nst_Obj *ob1, Nst_Obj *ob2);

// Comparisons

/* Implements the `==` operator. Is guaranteed to not fail. */
NstEXP Nst_Obj *NstC Nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `!=` operator. Is guaranteed to not fail. */
NstEXP Nst_Obj *NstC Nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `>` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `<` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `>=` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `<=` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2);

// Arithmetic operations

/* Implements the `+` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `-` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `*` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `/` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `^` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `%` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2);

// Bitwise operations

/* Implements the `|` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `&` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `^^` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `<<` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `>>` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2);

// Logical operations

/* Implements the `||` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `&&` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `&|` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2);

// Local operations

/* Implements the `-:` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_neg(Nst_Obj *ob);
/* Implements the `$` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_len(Nst_Obj *ob);
/* Implements the `~` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_bwnot(Nst_Obj *ob);
/* Implements the `!` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_lgnot(Nst_Obj *ob);
/* Implements the `>>>` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_stdout(Nst_Obj *ob);
/* Implements the `<<<` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_stdin(Nst_Obj *ob);
/* Implements the `?::` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_typeof(Nst_Obj *ob);
/* Implements the `|#|` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_import(Nst_Obj *ob);

// Other

/**
 * Casts an object to its string representation.
 *
 * @brief All objects can be casted, even custom types, but the function can
 * still fail to allocate memory.
 *
 * @param ob: the object to be casted
 *
 * @return The new string or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC _Nst_repr_str_cast(Nst_Obj *ob);

/**
 * Casts an object from a type to another.
 *
 * @brief Casting an object to `Bool` is guaranteed to not fail. All objects
 * can be casted to `Str` but the function can still fail if a memory
 * allocation is unsuccessful.
 *
 * @param ob: the object to be casted
 * @param type: the type to cast the object to
 *
 * @return The casted object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_obj_cast(Nst_Obj *ob, Nst_Obj *type);
/* Implements the `<.>` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC Nst_obj_contains(Nst_Obj *ob1, Nst_Obj *ob2);
/**
 * Concatenates two objects into a string.
 *
 * @brief Both objects are casted to `Str` before being concatenated.
 *
 * @param ob1: the first object
 * @param ob2: the second object
 *
 * @return The result of the concatenation or `NULL` on failure. The error is
 * set.
 */
NstEXP Nst_Obj *NstC Nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2);
/**
 * Creates a range object given the start, stop and step.
 *
 * @param start: the starting value of the range
 * @param stop: the the excluded stopping index of the range
 * @param step: the step of the range
 *
 * @return The new range object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_obj_range(Nst_Obj *start, Nst_Obj *stop,
                                   Nst_Obj *step);

/**
 * Returns the absolute path for a library to import.
 *
 * @brief If the library is not found on the given path, the standard library
 * directory is checked.
 *
 * @param initial_path: the relative path used to import the library
 * @param path_len: the length in bytes of `initial_path`
 *
 * @return The path on success and `NULL` on failure. The error is set. This
 * function fails if the specified library is not found.
 */
NstEXP Nst_Obj *NstC _Nst_get_import_path(const char *initial_path,
                                          usize path_len);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_OPS_H
