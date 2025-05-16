/**
 * @file str.h
 *
 * @brief Nest `Str` object interface
 *
 * @author TheSilvered
 */

#ifndef STR_H
#define STR_H

#include "obj.h"

/* Value of `idx` in case an error occurs when iterating over a string. */
#define Nst_STR_LOOP_ERROR -2

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

Nst_ObjRef *NstC _Nst_str_new_no_err(const char *val);

/**
 * Create a new string object from a C string literal. Uses `strlen` to find
 * the length.
 *
 * @brief Warning: `val` is always assumed to have a lifetime equal or longer
 * than Nest (i.e. it is destroyed with or after `Nst_quit`).
 *
 * @param val: the value of the string in extUTF-8 encoding
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_new_c(const char *val);
/**
 * Create a new string object.
 *
 * @param val: the value of the string to create in extUTF-8 encoding
 * @param len: the length of `val` in bytes
 * @param allocated: whether the value is heap allocated and should be freed
 * with the string
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_new(u8 *val, usize len, bool allocated);
/**
 * Create a new string object from a heap-allocated value.
 *
 * @brief `val` is freed if the string fails to be created.
 *
 * @param val: the value of the string to create
 * @param len: the length of `val` in bytes
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_new_allocated(u8 *val, usize len);

/**
 * Create a new string object with known length.
 *
 * @param val: the value of the string to create
 * @param len: the length in bytes of `val`
 * @param char_len: the length in characters of `val`
 * @param allocated: whether `val` is allocated on the heap
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_new_len(u8 *val, usize len, usize char_len,
                                        bool allocated);

/**
 * Create a new string copying the contents of an already existing one.
 *
 * @brief `src` remains untouched even if the function fails.
 *
 * @param src: the string to copy
 *
 * @return The copied string on success and `NULL` on failure. The error is
 * set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_copy(Nst_Obj *src);
/**
 * Create a new string by making a string representation of an existing one
 * that replaces any special characters such as newlines and tabs with their
 * code representation.
 *
 * @brief `src` remains untouched even if the function fails.
 *
 * @param src: the string to make the representation of
 *
 * @return The copied string on success and `NULL` on failure. The error is
 * set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_repr(Nst_Obj *src);
/**
 * Create a new one-character string with a character of an existing string.
 *
 * @brief If `idx` negative it is subtracted to the length to get the actual
 * index.
 *
 * @param str: the string to get the character from
 * @param idx: the index of the character to get
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 * The function fails if the index falls outside the string.
 */
NstEXP Nst_ObjRef *NstC Nst_str_get_obj(Nst_Obj *str, i64 idx);
/**
 * Get the character at index `idx` of the string.
 *
 * @brief If `idx` negative it is subtracted to the length to get the actual
 * index.
 *
 * @return The character on success and `-1` on failure. The error is set.
 * The function fails if the index falls outside the string.
 */
NstEXP i32 NstC Nst_str_get(Nst_Obj *str, i64 idx);

/**
 * Iterate over the characters of a string.
 *
 * @brief In order to start pass `-1` as `idx`, this will start from the first
 * character.
 *
 * @param str: the string to iterate
 * @param idx: the current index of the iteration
 *
 * @return The index of the first byte of the character currently being
 * iterated. When there are no more characters to iterate over a negative value
 * is returned. No errors can occur.
 */
NstEXP isize NstC Nst_str_next(Nst_Obj *str, isize idx);

/**
 * Iterate over the characters of a string.
 *
 * @brief In order to start set `idx` to `-1`, this will start from the first
 * character.
 *
 * @param str: the string to iterate
 * @param idx: the address to the current index of the iteration
 *
 * @return A `Str` object that contains the character being iterated. It
 * returns `NULL` when there are no more characters to iterate over or when an
 * error occurs. In case an error occurs `idx` is set to `Nst_STR_LOOP_ERROR`.
 */
NstEXP Nst_ObjRef *NstC Nst_str_next_obj(Nst_Obj *str, isize *idx);
/**
 * Iterate over the characters of a string.
 *
 * @brief In order to start set `idx` to `-1`, this will start from the first
 * character.
 *
 * @param str: the string to iterate
 * @param idx: the address to the current index of the iteration
 *
 * @return The Unicode value of the character. It returns `-1` when there are
 * no more characters to iterate over or when an error occurs. In case an error
 * occurs `idx` is set to `Nst_STR_LOOP_ERROR`.
 */
NstEXP i32 NstC Nst_str_next_utf32(Nst_Obj *str, isize *idx);
/**
 * Iterate over the characters of a string.
 *
 * @brief In order to start set `idx` to `-1`, this will start from the first
 * character.
 *
 * @param str: the string to iterate
 * @param idx: the address to the current index of the iteration
 * @param ch_buf: a buffer of length 4 where the bytes of the character are
 * copied, any extra bytes are set to `0`
 *
 * @return The length of the character in bytes. It returns `0` when there are
 * no more characters to iterate over or when an error occurs. In case an error
 * occurs `idx` is set to `Nst_STR_LOOP_ERROR`.
 */
NstEXP i32 NstC Nst_str_next_utf8(Nst_Obj *str, isize *idx, u8 *ch_buf);

/**
 * Parse an `Int` object from a string. Use `Nst_sv_parse_int` for better
 * control.
 *
 * @brief `base` can be any number between `2` and `36`, where above `10`
 * letters start to be used and the function is case-insensitive. If the base
 * is `0` the function uses prefixes such as `0x`, `0b` and `0o` to change the
 * base to `16`, `2` and `8` respectively. If `base` is set to `2`, `8` or
 * `16` when calling the function the prefix is optional and is ignored if
 * found. Any underscores (`_`) inside the number are ignored and any
 * whitespace around the number is ignored.
 *
 * @param str: the string to parse
 * @param base: the base to parse the string in
 *
 * @return The new int object or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_parse_int(Nst_Obj *str, i32 base);
/**
 * Parse a `Byte` object from a string. Use `Nst_sv_parse_byte` for better
 * control.
 *
 * @brief If an invalid literal is found, the function fails rather than
 * returning zero. Valid literals follow Nest's byte syntax, because of this 10
 * is considered invalid but 10b is valid.
 *
 * @param str: the string to parse
 *
 * @return The new byte object or NULL on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_parse_byte(Nst_Obj *str);
/**
 * Parse a `Real` object from a string. Use `Nst_sv_parse_real` for better
 * control.
 *
 * @brief If an invalid literal is found, the function fails rather than
 * returning zero. Valid literals follow Nest's real syntax, because of this
 * `3.` or `.5` are considered invalid and must be written as `3.0` and `0.5`.
 *
 * @param str: the string to parse
 *
 * @return The new real object or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_parse_real(Nst_Obj *str);
/* [docs:link strcmp <https://man7.org/linux/man-pages/man3/strcmp.3.html>] */
/**
 * Compare two Nest strings, similarly to `strcmp`.
 *
 * @param str1: the first string to compare
 * @param str2: the second string to compare
 *
 * @return `0` if the two strings are equal, a value `< 0` if `str2` is greater
 * than `str1` and a value `> 0` if `str1` is greater than `str2`.
 */
NstEXP i32 NstC Nst_str_compare(Nst_Obj *str1, Nst_Obj *str2);

void _Nst_str_destroy(Nst_Obj *str);

/**
 * @return The value of a Nest `Str` object.
 */
NstEXP u8 *NstC Nst_str_value(Nst_Obj *str);
/**
 * @return The length in bytes of the value of a Nest `Str` object.
 */
NstEXP usize NstC Nst_str_len(Nst_Obj *str);
/**
 * @return The number of characters in a Nest `Str` object.
 */
NstEXP usize NstC Nst_str_char_len(Nst_Obj *str);

/* Flags for `Str` objects. */
NstEXP typedef enum _Nst_StrFlags {
    Nst_FLAG_STR_IS_ALLOC  = Nst_FLAG(1),
    Nst_FLAG_STR_IS_ASCII  = Nst_FLAG(2),
    Nst_FLAG_STR_INDEX_16  = Nst_FLAG(3),
    Nst_FLAG_STR_INDEX_32  = Nst_FLAG(4),
    Nst_FLAG_STR_CAN_INDEX = Nst_FLAG(5)
} Nst_StrFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_H
