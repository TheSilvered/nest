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

#if defined(_DEBUG) && !defined(Nst_NO_ALLOC_COUNT)
#define Nst_COUNT_ALLOC
#endif // !Nst_COUNT_ALLOC

/* Alias for C free. */
#define Nst_raw_free Nst_free

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

/**
 * Structure representing a buffer of objects with an arbitrary size.
 *
 * @param len: the number of objects currently in the buffer
 * @param cap: the size in bytes of the allocated block
 * @param unit_size: the size in bytes of one object
 * @param data: the array of objects
 */
NstEXP typedef struct _Nst_SizedBuffer {
    usize len;
    usize cap;
    usize unit_size;
    void *data;
} Nst_SizedBuffer;

/**
 * Structure representing a buffer of chars.
 *
 * @brief Uses the same layout of `Nst_SizedBuffer` to re-use the same
 * functions. Ensures to always contain a valid string if not modified by
 * external functions.
 *
 * @param len: the length of the string in the buffer
 * @param cap: the size in bytes of the allocated block
 * @param unit_size: always `1`
 * @param data: the string
 */
NstEXP typedef struct _Nst_Buffer {
    usize len;
    usize cap;
    usize unit_size;
    i8 *data;
} Nst_Buffer;

/* [docs:link malloc <https://man7.org/linux/man-pages/man3/malloc.3.html>] */
/* [docs:link calloc <https://man7.org/linux/man-pages/man3/malloc.3.html>] */
/* [docs:link realloc <https://man7.org/linux/man-pages/man3/malloc.3.html>] */
/* [docs:link free <https://man7.org/linux/man-pages/man3/malloc.3.html>] */

#ifdef Nst_COUNT_ALLOC
/* Alias for C `malloc`. */
NstEXP void *NstC Nst_raw_malloc(usize size);
/* Alias for C `calloc`. */
NstEXP void *NstC Nst_raw_calloc(usize count, usize size);
/* Alias for C `realloc`. */
NstEXP void *NstC Nst_raw_realloc(void *block, usize size);
/* Alias for C `free`. */
NstEXP void NstC Nst_free(void *block);
#else
/* [docs:ignore] Alias for C `malloc`. */
#define Nst_raw_malloc malloc
/* [docs:ignore] Alias for C `calloc`. */
#define Nst_raw_calloc calloc
/* [docs:ignore] Alias for C `realloc`. */
#define Nst_raw_realloc realloc
/* [docs:ignore] Alias for C `free`. */
#define Nst_free free
#endif

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

/**
 * Initializes a `Nst_SizedBuffer`.
 *
 * @param buf: the buffer to initialize
 * @param unit_size: the size of the elements the buffer will contain
 * @param count: the number of elements to initialize the buffer with
 *
 * @return `true` on succes and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_sbuffer_init(Nst_SizedBuffer *buf, usize unit_size,
                                  usize count);
/**
 * Expands a sized buffer to contain a specified amount new elements.
 *
 * @brief The buffer is expanded only if needed.
 *
 * @param buf: the buffer to expand
 * @param amount: the number of new elements the buffer needs to contain
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_sbuffer_expand_by(Nst_SizedBuffer *buf, usize amount);
/**
 * Expands a sized buffer to contain a total amount of elements.
 *
 * @brief The buffer is expanded only if needed. If the new size is smaller
 * than the current one nothing is done.
 *
 * @param buf: the buffer to expand
 * @param amount: the number of elements the buffer needs to contain
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_sbuffer_expand_to(Nst_SizedBuffer *buf, usize count);
/* Shrinks the capacity of a sized buffer to match its length. */
NstEXP void NstC Nst_sbuffer_fit(Nst_SizedBuffer *buf);
/**
 * Appends an element to the end of the buffer.
 *
 * @brief The buffer is expanded more than needed to reduce the overall number
 * of reallocations.
 *
 * @brief If necessary, the buffer is expanded automatically.
 *
 * @param buf: the buffer to append the element to
 * @param element: a pointer to the element to append
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_sbuffer_append(Nst_SizedBuffer *buf, void *element);
/**
 * Copies the contents of a sized buffer into another.
 *
 * @brief The data of the source buffer is copied into a new block of memory,
 * subsequent changes to the source buffer will not modify the copied one.
 *
 * @param src: the buffer to copy from
 * @param dst: the buffer to copy to
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_sbuffer_copy(Nst_SizedBuffer *src, Nst_SizedBuffer *dst);
/* Destroys the contents of a sized buffer. The buffer itself is not freed. */
NstEXP void NstC Nst_sbuffer_destroy(Nst_SizedBuffer *buf);

/**
 * Initializes a `Nst_Buffer`.
 *
 * @param buf: the buffer to initialize
 * @param initial_size: the initial capacity of the buffer
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_buffer_init(Nst_Buffer *buf, usize initial_size);
/**
 * Expands a buffer to contain a specified amount new characters.
 *
 * @brief The buffer is expanded only if needed. `1` is added to `amount` to
 * take into account the `NUL` character at the end.
 *
 * @param buf: the buffer to expand
 * @param amount: the number of new characters the buffer needs to contain
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_buffer_expand_by(Nst_Buffer *buf, usize amount);
/**
 * Expands a sized buffer to contain a total amount of characters.
 *
 * @brief The buffer is expanded only if needed. If the new size is smaller
 * than the current one nothing is done. `1` is added to the size to take into
 * account the `NUL` character at the end.
 *
 * @param buf: the buffer to expand
 * @param amount: the number of characters the buffer needs to contain
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_buffer_expand_to(Nst_Buffer *buf, usize size);
/* Shrinks the capacity of a buffer to match its length. */
NstEXP void NstC Nst_buffer_fit(Nst_Buffer *buf);
/**
 * Appends a `Nst_StrObj` to the end of the buffer.
 *
 * @brief The buffer is expanded if needed.
 *
 * @param buf: the buffer to append the string to
 * @param str: the string to append
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_buffer_append(Nst_Buffer *buf, Nst_StrObj *str);
/**
 * Appends a C string to the end of the buffer.
 *
 * @brief The buffer is expanded if needed.
 *
 * @param buf: the buffer to append the string to
 * @param str: the string to append
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str);
/**
 * Appends a string of a known length to the end of the buffer.
 *
 * @param buf: the buffer to append the string to
 * @param str: the string to append
 * @param len: the length of the string to append, excluding the NUL character
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_buffer_append_str(Nst_Buffer *buf, i8 *str, usize len);
/**
 * Appends a character to the end of the buffer.
 *
 * @brief The buffer is expanded if needed.
 *
 * @param buf: the buffer to append the string to
 * @param ch: the character to append
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_buffer_append_char(Nst_Buffer *buf, i8 ch);
/**
 * Creates a `Nst_StrObj` from a buffer.
 *
 * @brief The data of the buffer is set to `NULL` and its `len` and `cap` are
 * set to `0`. The function automatically calls `Nst_buffer_fit`.
 *
 * @param buf: the buffer to create the string from
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_StrObj *NstC Nst_buffer_to_string(Nst_Buffer *buf);
/**
 * Copies the contents of a buffer into another.
 *
 * @brief The data of the source buffer is copied into a new block of memory,
 * subsequent changes to the source buffer will not modify the copied one.
 *
 * @param src: the buffer to copy from
 * @param dst: the buffer to copy to
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_buffer_copy(Nst_Buffer *src, Nst_Buffer *dst);
/* Destroys the contents of a buffer. The buffer itself is not freed. */
NstEXP void NstC Nst_buffer_destroy(Nst_Buffer *buf);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MEM_H
