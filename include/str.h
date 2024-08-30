/**
 * @file str.h
 *
 * @brief Nst_StrObj interface
 *
 * @author TheSilvered
 */

#ifndef STR_H
#define STR_H

#include "obj.h"

/* Casts `ptr` to `Nst_StrObj *`. */
#define STR(ptr) ((Nst_StrObj *)(ptr))
/* Gets the name of the type of an object as a C string. */
#define TYPE_NAME(obj) (obj->type->name.value)
/* Gets the name of the type as a Nest string. */
#define Nst_TYPE_STR(type) (&((struct _Nst_TypeObj *)(type))->name)
/* Checks if the value of a string is allocated. */
#define Nst_STR_IS_ALLOC(str) ((str)->flags & Nst_FLAG_STR_IS_ALLOC)

/* Alias of `_Nst_str_copy` that casts `src` to `Nst_StrObj *`. */
#define Nst_str_copy(src) _Nst_str_copy(STR(src))
/* Alias of `_Nst_str_repr` that casts `src` to `Nst_StrObj *`. */
#define Nst_str_repr(src) _Nst_str_repr(STR(src))
/* Alias of `_Nst_str_get` that casts `str` to `Nst_StrObj *`. */
#define Nst_str_get(str, idx) _Nst_str_get(STR(str), idx)

/* Value of `idx` in case an error occurs when iterating over a string. */
#define Nst_STR_LOOP_ERROR -2

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Structure representing a Nest string.
 *
 * @param len: the length in bytes of `value`
 * @param true_len: the length in characters of `value`
 * @param value: the value of the string
 * @param indexable_str: the string in UTF-16 or UTF-32 depending on the
 * characters it contains
 */
NstEXP typedef struct _Nst_StrObj {
    Nst_OBJ_HEAD;
    usize len;
    usize true_len;
    i8 *value;
    u8 *indexable_str;
} Nst_StrObj;

/**
 * Creates a new string object with a value taken from a C string of unknown
 * length.
 *
 * @param val: the value of the string in extUTF-8 encoding
 * @param allocated: whether the value is heap allocated and should be freed
 * with the string
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_str_new_c_raw(const i8 *val, bool allocated);
/**
 * Creates a new string object from a string literal of known length.
 *
 * @param val: the value of the string in extUTF-8 encoding
 * @param len: the length of `val` in bytes
 * @param allocated: whether the value is heap allocated and should be freed
 * with the string
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_str_new_c(const i8 *val, usize len, bool allocated);
/**
 * Creates a new string object.
 *
 * @param val: the value of the string to create in extUTF-8 encoding
 * @param len: the length of `val` in bytes
 * @param allocated: whether the value is heap allocated and should be freed
 * with the string
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_str_new(i8 *val, usize len, bool allocated);
/**
 * Creates a new string object from a heap-allocated value.
 *
 * @brief `val` is freed if the string fails to be created.
 *
 * @param val: the value of the string to create
 * @param len: the length of `val` in bytes
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_str_new_allocated(i8 *val, usize len);

/**
 * Creates a new string object with known length.
 *
 * @param val: the value of the string to create
 * @param len: the length in characters of `val`
 * @param true_len: the length in bytes  of `val`
 * @param allocated: whether `val` is allocated on the heap
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_str_new_len(i8 *val, usize len, usize true_len,
                                     bool allocated);

/**
 * Creates a new temporary read-only string object.
 *
 * @brief This object is not allocated on the heap and cannot be returned by
 * a function, its intended use is only on functions where a string object is
 * needed but you have the string in another form. Nothing is allocated and
 * it must not be destroyed in any way. If the string is indexed, `val` is
 * assumed to contain only 7-bit ASCII characters. If it may not contain only
 * those characters, create a string with `Nst_str_new` or other similar
 * functions.
 *
 * @param val: the value of the string
 * @param len: the length of the string
 *
 * @return A `Nst_StrObj` struct, **NOT POINTER**. This function never fails.
 */
NstEXP Nst_StrObj NstC Nst_str_temp(i8 *val, usize len);

/**
 * Creates a new string copying the contents of an already existing one.
 *
 * @brief `src` remains untouched even if the function fails.
 *
 * @param src: the string to copy
 *
 * @return The copied string on success and `NULL` on failure. The error is
 * set.
 */
NstEXP Nst_Obj *NstC _Nst_str_copy(Nst_StrObj *src);
/**
 * Creates a new string by making a string representation of an existing one
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
NstEXP Nst_Obj *NstC _Nst_str_repr(Nst_StrObj *src);
/**
 * Creates a new one-character string with a character of an existing string.
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
NstEXP Nst_Obj *NstC _Nst_str_get(Nst_StrObj *str, i64 idx);

/**
 * Iterates over the characters of a string.
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
NstEXP isize NstC Nst_str_next(Nst_StrObj *str, isize idx);

/**
 * Iterates over the characters of a string.
 *
 * @brief In order to start set `idx` to `-1`, this will start from the first
 * character.
 *
 * @param str: the string to iterate
 * @param idx: the address to the current index of the iteration
 *
 * @return A `Nst_StrObj` that contains the character being iterated. It
 * returns `NULL` when there are no more characters to iterate over or when an
 * error occurs. In case an error occurs `idx` is set to `Nst_STR_LOOP_ERROR`.
 */
NstEXP Nst_Obj *NstC Nst_str_next_obj(Nst_StrObj *str, isize *idx);
/**
 * Iterates over the characters of a string.
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
NstEXP i32 NstC Nst_str_next_utf32(Nst_StrObj *str, isize *idx);
/**
 * Iterates over the characters of a string.
 *
 * @brief In order to start set `idx` to `-1`, this will start from the first
 * character.
 *
 * @param str: the string to iterate
 * @param idx: the address to the current index of the iteration
 * @param ch_buf: a buffer of length 4 where the bytes of the charcter are
 * copied, any extra bytes are set to `0`
 *
 * @return The length of the character in bytes. It returns `0` when there are
 * no more characters to iterate over or when an error occurs. In case an error
 * occurs `idx` is set to `Nst_STR_LOOP_ERROR`.
 */
NstEXP i32 NstC Nst_str_next_utf8(Nst_StrObj *str, isize *idx, i8 *ch_buf);

/**
 * Parses a `Nst_IntObj` from a string.
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
NstEXP Nst_Obj *NstC Nst_str_parse_int(Nst_StrObj *str, i32 base);
/**
 * Parses a Nst_ByteObj from a string.
 *
 * @brief If an invalid literal is found, the function fails rather than
 * returning zero. Valid literals follow Nest's byte syntax, because of this 10
 * is considered invalid but 10b is valid.
 *
 * @param str: the string to parse
 *
 * @return The new byte object or NULL on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_str_parse_byte(Nst_StrObj *str);
/**
 * Parses a `Nst_RealObj` from a string.
 *
 * @brief If an invalid literal is found, the function fails rather than
 * returning zero. Valid literals follow Nest's real syntax, because of this
 * `3.` or `.5` are considered invalid and must be written as `3.0` and `0.5`.
 *
 * @param str: the string to parse
 *
 * @return The new real object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_str_parse_real(Nst_StrObj *str);
/* [docs:link strcmp <https://man7.org/linux/man-pages/man3/strcmp.3.html>] */
/**
 * Compares two Nest strings, similarly to `strcmp` but takes into account
 * possible `NUL` bytes inside the compared string.
 *
 * @param str1: the first string to compare
 * @param str2: the second string to compare
 *
 * @return `0` if the two strings are equal, a value `< 0` if `str2` is greater
 * than `str1` and a value `> 0` if `str1` is greater than `str2`.
 */
NstEXP i32 NstC Nst_str_compare(Nst_StrObj *str1, Nst_StrObj *str2);

/* String object destructor. */
NstEXP void NstC _Nst_str_destroy(Nst_StrObj *str);
/**
 * Finds the first occurrence of a substring inside a string.
 *
 * @brief If the pointer returned is not `NULL` it is guaranteed to be between
 * `s1 <= p < s1 + l1`, where `p` is the pointer.
 *
 * @param s1: the main string
 * @param l1: the length of `s1` in bytes
 * @param s2: the substring to find inside the main string
 * @param l2: the length of `s2` in bytes
 *
 * @return The pointer to the start of `s1` or `NULL` if the string could not
 * be found. No error is set.
 */
NstEXP i8 *NstC Nst_str_find(i8 *s1, usize l1, i8 *s2, usize l2);
/**
 * Finds the first occurrence of a substring inside a string starting from the
 * right.
 *
 * @brief If the pointer returned is not `NULL` it is guaranteed to be between
 * `s1 <= p < s1 + l1`, where `p` is the pointer.
 *
 * @param s1: the main string
 * @param l1: the length of `s1`
 * @param s2: the substring to find inside the main string
 * @param l2: the length of `s2`
 *
 * @return The pointer to the start of `s1` or `NULL` if the string could not
 * be found. No error is set.
 */
NstEXP i8 *NstC Nst_str_rfind(i8 *s1, usize l1, i8 *s2, usize l2);

/* `Nst_StrObj`-specific flags. */
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
