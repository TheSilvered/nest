/**
 * @file mem.h
 *
 * @brief Heap & dynamic memory management functions
 *
 * @author TheSilvered
 */

#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include "typedefs.h"
#include "error.h"

/**
 * @brief Calls `Nst_malloc` using `sizeof(type)` for the size and casting the
 * result to a pointer of `type`.
 */
#define Nst_malloc_c(count, type) ((type *)Nst_malloc(count, sizeof(type)))

/**
 * @brief Calls `Nst_calloc` using `sizeof(type)` for the size and casting the
 * result to a pointer of `type`.
 */
#define Nst_calloc_c(count, type, init_value)                                 \
    ((type *)Nst_calloc(count, sizeof(type), (void *)(init_value)))

/**
 * @brief Calls `Nst_realloc` using `sizeof(type)` for the size and casting the
 * result to a pointer of `type`.
 */
#define Nst_realloc_c(block, new_count, type, count)                          \
    ((type *)Nst_realloc((void *)(block), new_count, sizeof(type), count))

/**
 * @brief Calls `Nst_crealloc` using `sizeof(type)` for the size and casting
 * the result to a pointer of `type`.
 */
#define Nst_crealloc_c(block, new_count, type, count, init_value)             \
    ((type *)Nst_crealloc(                                                    \
        (void *)(block),                                                      \
        new_count,                                                            \
        sizeof(type),                                                         \
        count,                                                                \
        (void *)(init_value)))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* [docs:link malloc <https://man7.org/linux/man-pages/man3/malloc.3.html>] */
/* [docs:link calloc <https://man7.org/linux/man-pages/man3/malloc.3.html>] */
/* [docs:link realloc <https://man7.org/linux/man-pages/man3/malloc.3.html>] */
/* [docs:link free <https://man7.org/linux/man-pages/man3/malloc.3.html>] */

#ifdef Nst_DBG_COUNT_ALLOC
/* Alias for C `malloc`. */
NstEXP void *NstC Nst_raw_malloc(usize size);
/* Alias for C `calloc`. */
NstEXP void *NstC Nst_raw_calloc(usize count, usize size);
/* Alias for C `realloc`. */
NstEXP void *NstC Nst_raw_realloc(void *block, usize size);
/* Alias for C `free`. */
NstEXP void NstC Nst_raw_free(void *block);
/**
 * @brief Prints the current allocation count to `stdout`. Works only if
 * `Nst_DBG_COUNT_ALLOC` is defined, otherwise does nothing.
 */
NstEXP void NstC Nst_log_alloc_count(void);
/**
 * @brief Prints information about the current allocations to `stdout`. Works
 * only if `Nst_DBG_COUNT_ALLOC` is defined, otherwise does nothing.
 */
NstEXP void NstC Nst_log_alloc_info(void);

#ifdef Nst_MSVC
#pragma deprecated(malloc, calloc, realloc, free)
#else
void *malloc(size_t size) __attribute__((deprecated("use Nst_raw_malloc or Nst_malloc")));
void *calloc(size_t nmemb, size_t size) __attribute__((deprecated("use Nst_raw_calloc or Nst_calloc")));
void *realloc(void *ptr, size_t size) __attribute__((deprecated("use Nst_raw_realloc or Nst_realloc")));
void free(void *ptr) __attribute__((deprecated("use Nst_raw_free or Nst_free")));
#endif

#else
/* [docs:ignore] Alias for C `malloc`. */
#define Nst_raw_malloc malloc
/* [docs:ignore] Alias for C `calloc`. */
#define Nst_raw_calloc calloc
/* [docs:ignore] Alias for C `realloc`. */
#define Nst_raw_realloc realloc
/* [docs:ignore] Alias for C `free`. */
#define Nst_raw_free free

#define Nst_log_alloc_count()
#define Nst_log_alloc_info()
#endif

/* Alias for `Nst_raw_free`. */
#define Nst_free Nst_raw_free

/**
 * Allocates memory on the heap.
 *
 * @brief The elements are contiguous in memory.
 *
 * @param count: the number of elements to allocate
 * @param size: the size in bytes of each element
 *
 * @return A pointer to the allocated memory block or `NULL` on failure. The
 * error is set.
 */
NstEXP void *NstC Nst_malloc(usize count, usize size);
/**
 * Allocates memory on the heap initializing it.
 *
 * @brief The elements are contiguous in memory. If `init_value` is `NULL`, the
 * function has a similar behaviour to `calloc` filling the memory with zeroes.
 * `init_value` is expected to be the same size as the one given for the
 * elements.
 *
 * @param count: the number of elements to allocate
 * @param size: the size in bytes of each element
 * @param init_value: a pointer to the value to initialize each element with
 *
 * @return A pointer to the allocated memory block or `NULL` on failure. The
 * error is set.
 */
NstEXP void *NstC Nst_calloc(usize count, usize size, void *init_value);
/**
 * Changes the size of an allocated memory block.
 *
 * @brief This function never fails when the block is shrunk because if the
 * call to `realloc` fails, the old block is returned.
 *
 * @param block: the block to reallocate
 * @param new_count: the new number of elements of the block
 * @param size: the size in bytes of each element
 * @param count: the current number of elements in the block
 *
 * @return A pointer to the reallocated memory block or `NULL` on failure. The
 * error is set. If either `new_count` or `size` is zero, block is freed and
 * `NULL` is returned with no error.
 */
NstEXP void *NstC Nst_realloc(void *block, usize new_count, usize size,
                              usize count);
/**
 * Changes the size of an allocated memory block initializing new memory.
 *
 * @brief This function never fails when the block is shrunk because if the
 * call to `realloc` fails, the old block is returned. If `init_value` is
 * `NULL`, the function just fills the new memory with zeroes. `init_value` is
 * expected to have a size of `size`.
 *
 * @param block: the block to reallocate
 * @param new_count: the new number of elements of the block
 * @param size: the size in bytes of each element
 * @param count: the current number of elements in the block
 * @param init_value: a pointer to the value to initialize the new elements
 * with
 *
 * @return A pointer to the reallocated memory block or `NULL` on failure. The
 * error is set. If either `new_count` or `size` is zero, block is freed and
 * `NULL` is returned with no error.
 */
NstEXP void *NstC Nst_crealloc(void *block, usize new_count, usize size,
                               usize count, void *init_value);

/* [docs:link memset <https://man7.org/linux/man-pages/man3/memset.3.html>] */

/**
 * Sets the value of an array in memory.
 *
 * @brief Note: unlike `memset` in `string.h` this function does not return a
 * value.
 *
 * @brief Warning: the behaviour of this function is undefined if `block` and
 * `value` overlap.
 *
 * @param block: the pointer to the block of memory to edit
 * @param size: the size in bytes of a unit inside `block`
 * @param count: the number of units inside `block`
 * @param value: a pointer to the value to copy for each unit, if it is NULL
 * the block is filled with zeroes
 */
NstEXP void NstC Nst_memset(void *block, usize size, usize count, void *value);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MEM_H
