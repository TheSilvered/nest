/**
 * @file dyn_array.h
 *
 * @brief Dynamic heap-allocated array.
 *
 * @author TheSilvered
 */

#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Structure representing a dynamic array of objects with an arbitrary size.
 *
 * @param len: the number of objects currently in the buffer
 * @param cap: the size in bytes of the allocated block
 * @param unit_size: the size in bytes of one object
 * @param data: the array of objects
 */
NstEXP typedef struct _Nst_DynArray {
    usize len;
    usize cap;
    usize unit_size;
    void *data;
} Nst_DynArray;

/**
 * Initializes a `Nst_DynArray`.
 *
 * @param buf: the buffer to initialize
 * @param unit_size: the size of the elements the array will contain
 * @param reserve: the number of elements to initialize the array with
 *
 * @return `true` on succes and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_da_init(Nst_DynArray *arr, usize unit_size, usize reserve);
/**
 * Copy the contents of an arry into another.
 *
 * @brief The data of the source buffer is copied into a new block of memory,
 * subsequent changes to the source will not modify the copy.
 *
 * @param src: the array to copy from
 * @param dst: the array to copy to
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_da_init_copy(Nst_DynArray *arr, Nst_DynArray *dst);

/**
 * Reserve `amount` free slots to avoid reallocations.
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_da_reserve(Nst_DynArray *arr, usize amount);
/**
 * Append an element to the end of the array.
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_da_append(Nst_DynArray *arr, void *element);
/**
 * Pops the last element of the array.
 *
 * @return `true` if the element was popped successfully and `false` if there
 * was no item to pop. No error is set.
 */
NstEXP bool NstC Nst_da_pop(Nst_DynArray *arr);
/**
 * Remove the element of an array at `index` and puts the last element of
 * the array in its place.
 *
 * @brief Note: this function operates in constant time.
 *
 * @return `true` if the element was removed successfully and `false` if there
 * was no element to remove. No error is set.
 */
NstEXP bool NstC Nst_da_remove_swap(Nst_DynArray *arr, usize index);
/**
 * Remove the element of an array at `index` and puts the last element of
 * the array in its place
 *
 * @brief Note: this function operates in linear time.
 *
 * @return `true` if the element was removed successfully and `false` if there
 * was no element to remove. No error is set.
 */
NstEXP bool NstC Nst_da_remove_shift(Nst_DynArray *arr, usize index);
/**
 * Get the element of an array at `index`.
 *
 * @return A pointer to the start of the element in the array or `NULL` if the
 * index is out of bounds. No error is set.
 */
NstEXP void *NstC Nst_da_at(Nst_DynArray *arr, usize index);
/* Clear the contents of an array freeing any allocated memory. */
NstEXP void NstC Nst_da_clear(Nst_DynArray *arr);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !DYN_ARRAY_H
