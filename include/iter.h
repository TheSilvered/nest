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

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Creates a new Nest iterator object.
 *
 * @brief Note: this function takes one reference of `start`, `next` and
 * `value` both on success and on failure.
 *
 * @param start: the `start` function for the new iterator
 * @param next: the `next` function for the new iterator
 * @param value: the `value` for the new iterator
 *
 * @return The new object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_iter_new(Nst_Obj *start, Nst_Obj *next,
                                  Nst_Obj *value);
/* Destructor for Nest iter objects. */
NstEXP void NstC _Nst_iter_destroy(Nst_Obj *iter);
/* Traverse function for Nest iter objects. */
NstEXP void NstC _Nst_iter_traverse(Nst_Obj *iter);

/**
 * Calls the `start` function of an `Iter` object.
 *
 * @param iter: the iterator to start
 *
 * @return `true` on success and `false` on success. The error is set.
 */
NstEXP bool NstC Nst_iter_start(Nst_Obj *iter);

/**
 * Calls the `next` function of an `Iter` object.
 *
 * @param iter: the iterator to get the value from
 *
 * @return The resulting object on success and `NULL` on failure. The error is
 * set.
 */
NstEXP Nst_Obj *NstC Nst_iter_next(Nst_Obj *iter);

/* Get the `start` function of an iterator. No reference is added. */
NstEXP Nst_Obj *NstC Nst_iter_start_func(Nst_Obj *iter);
/* Get the `next` function of an iterator. No reference is added. */
NstEXP Nst_Obj *NstC Nst_iter_next_func(Nst_Obj *iter);
/**
 * @brief Get the `value` passed to `start` and `next` of an iterator. No
 * reference is added.
 */
NstEXP Nst_Obj *NstC Nst_iter_value(Nst_Obj *iter);

// Functions for range iterators

/* The `start` function of the range iterator. */
NstEXP Nst_Obj *NstC Nst_iter_range_start(usize arg_num, Nst_Obj **args);
/* The `next` function of the range iterator. */
NstEXP Nst_Obj *NstC Nst_iter_range_next(usize arg_num, Nst_Obj **args);

// Functions for sequence iterators

/* The `start` function of the sequence iterator. */
NstEXP Nst_Obj *NstC Nst_iter_seq_start(usize arg_num, Nst_Obj **args);
/* The `next` function of the sequence iterator. */
NstEXP Nst_Obj *NstC Nst_iter_seq_next(usize arg_num, Nst_Obj **args);

// Functions for string iterators

/* The `start` function of the string iterator. */
NstEXP Nst_Obj *NstC Nst_iter_str_start(usize arg_num, Nst_Obj **args);
/* The `next` function of the string iterator. */
NstEXP Nst_Obj *NstC Nst_iter_str_next(usize arg_num, Nst_Obj **args);

// Functions for map iterators

/* The `start` function of the map iterator. */
NstEXP Nst_Obj *NstC Nst_iter_map_start(usize arg_num, Nst_Obj **args);
/* The `next` function of the map iterator. */
NstEXP Nst_Obj *NstC Nst_iter_map_next(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // ITER_H
