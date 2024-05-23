/**
 * @file iter.h
 *
 * @brief Nst_IterObj interface
 *
 * @author TheSilvered
 */

#ifndef ITER_H
#define ITER_H

#include "lib_import.h"

/* Casts `ptr` to `Nst_IterObj *`. */
#define ITER(ptr) ((Nst_IterObj *)(ptr))
/* Alias for `_Nst_iter_start` that casts `iter` to `Nst_IterObj *`. */
#define Nst_iter_start(iter) _Nst_iter_start(ITER(iter))
/* Alias for `_Nst_iter_get_val` that casts `iter` to `Nst_IterObj *`. */
#define Nst_iter_get_val(iter) _Nst_iter_get_val(ITER(iter))
/**
 * @brief Alias for `_Nst_iter_new` that casts `start` and `end` to
 * `Nst_FuncObj *` and casts `value` to `Nst_Obj *`.
 */
#define Nst_iter_new(start, end, value)                                       \
    _Nst_iter_new(FUNC(start), FUNC(end), OBJ(value))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * The structure defining a Nest iterator object.
 *
 * @param start: the function that initializes the iterator
 * @param get_val: the function of the iterator that gets the current value
 * @param value: the value passed to the functions of the iterator
 */
NstEXP typedef struct _Nst_IterObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_FuncObj *start;
    Nst_FuncObj *get_val;
    Nst_Obj *value;
} Nst_IterObj;

/**
 * Creates a new Nest iterator object.
 *
 * @brief Note: this function takes one reference of `start`, `is_done`,
 * `get_val` and value both on success and on failure.
 *
 * @param start: the `start` function for the new iterator
 * @param get_val: the `get_val` function for the new iterator
 * @param value: the `value` for the new iterator
 *
 * @return The new object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC _Nst_iter_new(Nst_FuncObj *start, Nst_FuncObj *get_val,
                                  Nst_Obj *value);
/* Destructor for Nest iter objects. */
NstEXP void NstC _Nst_iter_destroy(Nst_IterObj *iter);
/* Traverse function for Nest iter objects. */
NstEXP void NstC _Nst_iter_traverse(Nst_IterObj *iter);

/**
 * Calls the `start` function of a `Nst_IterObj`.
 *
 * @param iter: the iterator to start
 *
 * @return `true` on success and `false` on success. The error is set.
 */
NstEXP bool NstC _Nst_iter_start(Nst_IterObj *iter);

/**
 * Calls the `get_val` function of a `Nst_IterObj`.
 *
 * @param iter: the iterator to get the value from
 *
 * @return The resulting object on success and `NULL` on failure. The error is
 * set.
 */
NstEXP Nst_Obj *NstC _Nst_iter_get_val(Nst_IterObj *iter);

// Functions for range iterators

/* The `start` function of the range iterator. */
NstEXP Nst_FUNC_SIGN(Nst_iter_range_start);
/* The `get_val` function of the range iterator. */
NstEXP Nst_FUNC_SIGN(Nst_iter_range_get_val);

// Functions for sequence iterators

/* The `start` function of the sequence iterator. */
NstEXP Nst_FUNC_SIGN(Nst_iter_seq_start);
/* The `get_val` function of the sequence iterator. */
NstEXP Nst_FUNC_SIGN(Nst_iter_seq_get_val);

// Functions for string iterators

/* The `start` function of the string iterator. */
NstEXP Nst_FUNC_SIGN(Nst_iter_str_start);
/* The `get_val` function of the string iterator. */
NstEXP Nst_FUNC_SIGN(Nst_iter_str_get_val);

// Functions for map iterators

/* The `start` function of the map iterator. */
NstEXP Nst_FUNC_SIGN(Nst_iter_map_start);
/* The `get_val` function of the map iterator. */
NstEXP Nst_FUNC_SIGN(Nst_iter_map_get_val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // ITER_H
