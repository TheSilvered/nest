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

/* Alias for `_Nst_obj_eq` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_eq(ob1, ob2) _Nst_obj_eq(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_ne` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_ne(ob1, ob2) _Nst_obj_ne(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_gt` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_gt(ob1, ob2) _Nst_obj_gt(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_lt` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_lt(ob1, ob2) _Nst_obj_lt(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_ge` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_ge(ob1, ob2) _Nst_obj_ge(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_le` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_le(ob1, ob2) _Nst_obj_le(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_add` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_add(ob1, ob2) _Nst_obj_add(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_sub` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_sub(ob1, ob2) _Nst_obj_sub(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_mul` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_mul(ob1, ob2) _Nst_obj_mul(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_div` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_div(ob1, ob2) _Nst_obj_div(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_pow` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_pow(ob1, ob2) _Nst_obj_pow(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_mod` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_mod(ob1, ob2) _Nst_obj_mod(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_bwor` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_bwor(ob1, ob2) _Nst_obj_bwor(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_bwand` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_bwand(ob1, ob2) _Nst_obj_bwand(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_bwxor` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_bwxor(ob1, ob2) _Nst_obj_bwxor(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_bwls` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_bwls(ob1, ob2) _Nst_obj_bwls(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_bwrs` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_bwrs(ob1, ob2) _Nst_obj_bwrs(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_lgor` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_lgor(ob1, ob2) _Nst_obj_lgor(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_lgand` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_lgand(ob1, ob2) _Nst_obj_lgand(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_lgxor` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_lgxor(ob1, ob2) _Nst_obj_lgxor(OBJ(ob1), OBJ(ob2))
/**
 * @brief Alias for `_Nst_obj_cast` that casts `ob` to `Nst_Obj *` and `type`
 * to `Nst_TypeObj *`.
 */
#define Nst_obj_cast(ob, type) _Nst_obj_cast(OBJ(ob), TYPE(type))
/* Alias for `_Nst_obj_contains` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_contains(ob1, ob2) _Nst_obj_contains(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_concat` that casts both objects to `Nst_Obj *`. */
#define Nst_obj_concat(ob1, ob2) _Nst_obj_concat(OBJ(ob1), OBJ(ob2))
/* Alias for `_Nst_obj_range` that casts all objects to `Nst_Obj *`. */
#define Nst_obj_range(ob1, ob2, ob3)                                          \
    _Nst_obj_range(OBJ(ob1), OBJ(ob2), OBJ(ob3))
/* Alias for `_Nst_obj_neg` that casts `ob` to `Nst_Obj *`. */
#define Nst_obj_neg(ob) _Nst_obj_neg(OBJ(ob))
/* Alias for `_Nst_obj_len` that casts `ob` to `Nst_Obj *`. */
#define Nst_obj_len(ob) _Nst_obj_len(OBJ(ob))
/* Alias for `_Nst_obj_bwnot` that casts `ob` to `Nst_Obj *`. */
#define Nst_obj_bwnot(ob) _Nst_obj_bwnot(OBJ(ob))
/* Alias for `_Nst_obj_lgnot` that casts `ob` to `Nst_Obj *`. */
#define Nst_obj_lgnot(ob) _Nst_obj_lgnot(OBJ(ob))
/* Alias for `_Nst_obj_stdout` that casts `ob` to `Nst_Obj *`. */
#define Nst_obj_stdout(ob) _Nst_obj_stdout(OBJ(ob))
/* Alias for `_Nst_obj_stdin` that casts `ob` to `Nst_Obj *`. */
#define Nst_obj_stdin(ob) _Nst_obj_stdin(OBJ(ob))
/* Alias for `_Nst_obj_typeof` that casts `ob` to `Nst_Obj *`. */
#define Nst_obj_typeof(ob) _Nst_obj_typeof(OBJ(ob))
/* Alias for `_Nst_obj_import` that casts `ob` to `Nst_Obj *`. */
#define Nst_obj_import(ob) _Nst_obj_import(OBJ(ob))

/* Calls `Nst_obj_eq` making the result a C bool. */
#define Nst_obj_eq_c(ob1, ob2) _Nst_obj_to_bool(Nst_obj_eq(ob1, ob2))
/* Calls `Nst_obj_ne` making the result a C bool. */
#define Nst_obj_ne_c(ob1, ob2) _Nst_obj_to_bool(Nst_obj_ne(ob1, ob2))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Comparisons

/* Implements the `==` operator. Is guaranteed to not fail. */
NstEXP Nst_Obj *NstC _Nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `!=` operator. Is guaranteed to not fail. */
NstEXP Nst_Obj *NstC _Nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `>` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `<` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `>=` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `<=` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2);

// Arithmetic operations

/* Implements the `+` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `-` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `*` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `/` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `^` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `%` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2);

// Bitwise operations

/* Implements the `|` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `&` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `^^` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `<<` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `>>` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2);

// Logical operations

/* Implements the `||` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `&&` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2);
/* Implements the `&|` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2);

// Local operations

/* Implements the `-:` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_neg(Nst_Obj *ob);
/* Implements the `$` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_len(Nst_Obj *ob);
/* Implements the `~` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_bwnot(Nst_Obj *ob);
/* Implements the `!` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_lgnot(Nst_Obj *ob);
/* Implements the `>>>` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_stdout(Nst_Obj *ob);
/* Implements the `<<<` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_stdin(Nst_Obj *ob);
/* Implements the `?::` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_typeof(Nst_Obj *ob);
/* Implements the `|#|` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_import(Nst_Obj *ob);

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

/* [docs:link Nst_obj_cast(seq_obj,\Nst_type()->Str) _Nst_obj_cast] */
/* [docs:link Nst_obj_cast(map_obj,\Nst_type()->Str) _Nst_obj_cast] */

/**
 * Internal cast from sequence to string.
 *
 * @brief It is recommended to use `Nst_obj_cast(seq_obj, Nst_type()->Str)`,
 * this is used internally.
 *
 * @param seq_obj: the object to cast
 * @param all_objs: a list containing all collections encountered up to that
 * point, on the first call it is empty and is populated automatically
 *
 * @return The new string object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC _Nst_obj_str_cast_seq(Nst_SeqObj *seq_obj,
                                           Nst_LList *all_objs);
/**
 * Internal cast from map to string.
 *
 * @brief It is recommended to use `Nst_obj_cast(map_obj, Nst_type()->Str)`,
 * this is used internally.
 *
 * @param map_obj: the object to cast
 * @param all_objs: a list containing all collections encountered up to that
 * point, on the first call it is empty and is populated automatically
 *
 * @return The new string object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC _Nst_obj_str_cast_map(Nst_MapObj *map_obj,
                                           Nst_LList *all_objs);
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
NstEXP Nst_Obj *NstC _Nst_obj_cast(Nst_Obj *ob, Nst_TypeObj *type);
/* Implements the `<.>` operator. On failure the error is set. */
NstEXP Nst_Obj *NstC _Nst_obj_contains(Nst_Obj *ob1, Nst_Obj *ob2);
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
NstEXP Nst_Obj *NstC _Nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2);
/**
 * Creates a range object given the start, stop and step.
 *
 * @param start: the starting value of the range
 * @param stop: the the excluded stopping index of the range
 * @param step: the step of the range
 *
 * @return The new range object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC _Nst_obj_range(Nst_Obj *start, Nst_Obj *stop,
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
NstEXP Nst_StrObj *NstC _Nst_get_import_path(i8 *initial_path, usize path_len);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_OPS_H
