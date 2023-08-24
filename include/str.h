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
/* Casts `ptr` to `Nst_TypeObj *`. */
#define TYPE(ptr) ((Nst_TypeObj *)(ptr))
/* Gets the name of the type of an object as a C string. */
#define TYPE_NAME(obj) (STR(obj->type)->value)
/* Checks if the value of a string is allocated. */
#define Nst_STR_IS_ALLOC(str) ((str)->flags & Nst_FLAG_STR_IS_ALLOC)

/* Alias of `_Nst_string_copy` that casts `src` to `Nst_StrObj *`. */
#define Nst_string_copy(src) _Nst_string_copy(STR(src))
/* Alias of `_Nst_string_repr` that casts `src` to `Nst_StrObj *`. */
#define Nst_string_repr(src) _Nst_string_repr(STR(src))
/* Alias of `_Nst_string_get` that casts `str` to `Nst_StrObj *`. */
#define Nst_string_get(str, idx) _Nst_string_get(STR(str), idx)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Structure representing a Nest string.
 *
 * @param len: the length of the string
 * @param value: the value of the string
 */
NstEXP typedef struct _Nst_StrObj {
    Nst_OBJ_HEAD;
    usize len;
    i8 *value;
} Nst_StrObj;

/* Defined for completeness, Nest `Type` objects are just strings. */
NstEXP typedef Nst_StrObj Nst_TypeObj;

/**
 * Creates a new string object with a value taken from a C string of unknown
 * length.
 *
 * @param val: the value of the string
 * @param allocated: whether the value is heap allocated and should be freed
 * with the string
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_string_new_c_raw(const i8 *val, bool allocated);
/**
 * Creates a new string object from a string literal of known length.
 *
 * @param val: the value of the string
 * @param len: the length of the string literal
 * @param allocated: whether the value is heap allocated and should be freed
 * with the string
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_string_new_c(const i8 *val, usize len, bool allocated);
/**
 * Creates a new string object.
 *
 * @param val: the value of the string to create
 * @param len: the length of the string
 * @param allocated: whether the value is heap allocated and should be freed
 * with the string
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_string_new(i8 *val, usize len, bool allocated);
/**
 * Creates a new string object from a heap-allocated value.
 *
 * @brief val is freed if the string fails to be created.
 *
 * @param val: the value of the string to create
 * @param len: the length of the string
 *
 * @return The new string on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_string_new_allocated(i8 *val, usize len);

/**
 * Creates a new temporary read-only string object.
 *
 * @brief This object is not allocated on the heap and cannot be returned by
 * a function, its intended use is only on functions where a string object is
 * needed but you have the string in another form. Nothing is allocated and
 * it must not be destroyed in any way.
 *
 * @param val: the value of the string
 * @param len: the length of the string
 *
 * @return A `Nst_StrObj` struct, **NOT POINTER**. This function never fails.
 */
NstEXP Nst_StrObj NstC Nst_string_temp(i8 *val, usize len);

/**
 * Creates a new `Nst_TypeObj`.
 *
 * @param val: the name of the type
 *
 * @return The new object on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_TypeObj *NstC Nst_type_new(const i8 *val);

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
NstEXP Nst_Obj *NstC _Nst_string_copy(Nst_StrObj *src);
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
NstEXP Nst_Obj *NstC _Nst_string_repr(Nst_StrObj *src);
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
NstEXP Nst_Obj *NstC _Nst_string_get(Nst_StrObj *str, i64 idx);

/**
 * Parses a `Nst_IntObj` from a string.
 *
 * @brief If an invalid literal is found, the function fails rather than
 * returning zero. `base` can be any number between `2` and `36`, where above
 * `10` letters start to be used. If the base is `0` the function uses prefixes
 * such as `0x`, `0b` and `0o` to change the base. If the base is `2`, `8` or
 * `16` the prefix is optional and is ignored if found.
 *
 * @param str: the string to parse
 * @param base: the base to parse the string in
 *
 * @return The new int object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_string_parse_int(Nst_StrObj *str, i32 base);
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
NstEXP Nst_Obj *NstC Nst_string_parse_byte(Nst_StrObj* str);
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
NstEXP Nst_Obj *NstC Nst_string_parse_real(Nst_StrObj *str);
/* [docs:link strcmp https://man7.org/linux/man-pages/man3/strcmp.3.html] */
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
NstEXP i32 NstC Nst_string_compare(Nst_StrObj *str1, Nst_StrObj *str2);

/* String object destructor. */
NstEXP void NstC _Nst_string_destroy(Nst_StrObj *str);
/**
 * Finds the first occurrence of a substring inside a string.
 *
 * @brief If the pointer is not `NULL` it is guaranteed to be between
 * `s1 <= p < s1 + l1`, where `p` is the pointer returned.
 *
 * @param s1: the main string
 * @param l1: the length of `s1`
 * @param s2: the substring to find inside the main string
 * @param l2: the length of `s2`
 *
 * @return The pointer to the start of `s1` or `NULL` if the string could not
 * be found. No error is set.
 */
NstEXP i8 *NstC Nst_string_find(i8 *s1, usize l1, i8 *s2, usize l2);

/* `Nst_StrObj`-specific flags. */
NstEXP typedef enum _Nst_StrFlags {
    Nst_FLAG_STR_IS_ALLOC = 0b1
} Nst_StrFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_H
