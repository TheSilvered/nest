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
 * @param len: the number of objects currently in `data`
 * @param cap: the capacity of `data`
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
 * Structure representing a dynamic array of pointers.
 *
 * @param len: the number of pointers currently in in `data`
 * @param cap: the capacity of `data`
 * @param data: the array of pointers
 */
NstEXP typedef struct _Nst_PtrArray {
    usize len;
    usize cap;
    void **data;
} Nst_PtrArray;

/**
 * Initialize a `Nst_DynArray`.
 *
 * @brief If `reserve` is `0` no memory is allocated.
 *
 * @param buf: the buffer to initialize
 * @param unit_size: the size of the elements the array will contain
 * @param reserve: the capacity to initialize the array with
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
NstEXP bool NstC Nst_da_init_copy(Nst_DynArray *src, Nst_DynArray *dst);

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
 * Pop the last element of the array.
 *
 * @return `true` if the element was popped successfully and `false` if there
 * was no item to pop. No error is set.
 */
NstEXP bool NstC Nst_da_pop(Nst_DynArray *arr, Nst_Destructor dstr);
/**
 * Remove the element of an array at `index` and put the last element of the
 * array in its place.
 *
 * @brief Note: this function operates in constant time.
 *
 * @return `true` if the element was removed successfully and `false` if there
 * was no element to remove. No error is set.
 */
NstEXP bool NstC Nst_da_remove_swap(Nst_DynArray *arr, usize index,
                                    Nst_Destructor dstr);
/**
 * Remove the element of an array at `index` and puts the last element of
 * the array in its place.
 *
 * @brief Note: this function operates in linear time.
 *
 * @return `true` if the element was removed successfully and `false` if there
 * was no element to remove. No error is set.
 */
NstEXP bool NstC Nst_da_remove_shift(Nst_DynArray *arr, usize index,
                                     Nst_Destructor dstr);
/**
 * Get the pointer to the element of an array at `index`.
 *
 * @return A pointer to the start of the element in the array or `NULL` if the
 * index is out of bounds. No error is set.
 */
NstEXP void *NstC Nst_da_get(Nst_DynArray *arr, usize index);
/* Clear the contents of an array freeing any allocated memory. */
NstEXP void NstC Nst_da_clear(Nst_DynArray *arr, Nst_Destructor dstr);
/**
 * Set an element in an `Nst_DynArray`.
 *
 * @param arr: the array to modify
 * @param index: the index to set
 * @param element: a pointer to the value to copy
 * @param dstr: the destructor used to destroy the previous element at `index`
 */
NstEXP void NstC Nst_da_set(Nst_DynArray *arr, usize index, void *element,
                            Nst_Destructor dstr);

/**
 * Initialize a `Nst_PtrArray`.
 *
 * @brief If `reserve` is `0` no memory is allocated.
 *
 * @param buf: the buffer to initialize
 * @param reserve: the capacity to initialize the array with
 *
 * @return `true` on succes and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_pa_init(Nst_PtrArray *arr, usize reserve);
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
NstEXP bool NstC Nst_pa_init_copy(Nst_PtrArray *src, Nst_PtrArray *dst);
/**
 * Reserve `amount` free slots to avoid reallocations.
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_pa_reserve(Nst_PtrArray *arr, usize amount);
/**
 * Append an element to the end of the array.
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_pa_append(Nst_PtrArray *arr, void *element);
/**
 * Pop the last element of the array.
 *
 * @return `true` if the element was popped successfully and `false` if there
 * was no item to pop. No error is set.
 */
NstEXP bool NstC Nst_pa_pop(Nst_PtrArray *arr, Nst_Destructor dstr);
/**
 * Remove the element of an array at `index` and put the last element of the
 * array in its place.
 *
 * @brief Note: this function operates in constant time.
 *
 * @return `true` if the element was removed successfully and `false` if there
 * was no element to remove. No error is set.
 */
NstEXP bool NstC Nst_pa_remove_swap(Nst_PtrArray *arr, usize index,
                                    Nst_Destructor dstr);
/**
 * Remove the element of an array at `index` and puts the last element of
 * the array in its place.
 *
 * @brief Note: this function operates in linear time.
 *
 * @return `true` if the element was removed successfully and `false` if there
 * was no element to remove. No error is set.
 */
NstEXP bool NstC Nst_pa_remove_shift(Nst_PtrArray *arr, usize index,
                                     Nst_Destructor dstr);
/**
 * Get the pointer to the element of an array at `index`.
 *
 * @return A pointer to the start of the element in the array or `NULL` if the
 * index is out of bounds. No error is set.
 */
NstEXP void *NstC Nst_pa_get(Nst_PtrArray *arr, usize index);
/* Clear the contents of an array freeing any allocated memory. */
NstEXP void NstC Nst_pa_clear(Nst_PtrArray *arr, Nst_Destructor dstr);
/**
 * Set an element in an `Nst_PtrArray`.
 *
 * @param arr: the array to modify
 * @param index: the index to set
 * @param element: a pointer to the value to copy
 * @param dstr: the destructor used to destroy the previous element at `index`
 */
NstEXP void NstC Nst_pa_set(Nst_PtrArray *arr, usize index, void *element,
                            Nst_Destructor dstr);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !DYN_ARRAY_H
