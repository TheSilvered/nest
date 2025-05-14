/**
 * @file str_builder.h
 *
 * @brief String builder
 *
 * @author TheSilvered
 */

#ifndef STR_BUILDER_H
#define STR_BUILDER_H

#include "typedefs.h"
#include "str_view.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Structure representing a string builder. This structure owns `value`.
 *
 * @param value: the string
 * @param len: the length of the string
 * @param cap: the size in bytes of the allocated block
 */
NstEXP typedef struct _Nst_StrBuilder {
    u8 *value;
    usize len;
    usize cap;
} Nst_StrBuilder;

/**
 * Initialize `sb`. Pre-allocate `reserve` slots.
 *
 * @return `true` on success and `false` if a memory allocation fails. The
 * error is set.
 */
NstEXP bool NstC Nst_sb_init(Nst_StrBuilder *sb, usize reserve);
/**
 * Destroy an `Nst_StrBuilder`. Calling this function on a builder that failed
 * to initialize, that was already destroyed or that was passed to
 * `Nst_str_from_sb` will do nothing.
 */
NstEXP void NstC Nst_sb_destroy(Nst_StrBuilder *sb);
/**
 * Ensure that the string can be expanded by `amount` bytes without
 * reallocating.
 *
 * @return `true` on success and `false` if a memory allocation fails. The
 * error is set.
 */
NstEXP bool NstC Nst_sb_reserve(Nst_StrBuilder *sb, usize amount);
/**
 * Add `chars` to the end of a `Nst_StrBuilder`.
 *
 * @param sb: the string builder to append the value to
 * @param chars: the characters to append
 * @param count: the length of `chars`
 *
 * @return `true` on success and `false` if a memory allocation fails. The
 * error is set.
 */
NstEXP bool NstC Nst_sb_push(Nst_StrBuilder *sb, u8 *chars, usize count);
/**
 * Add the value of a `Nst_StrView` to the end of a `Nst_StrBuilder`.
 *
 * @return `true` on success and `false` if a memory allocation fails. The
 * error is set.
 */
NstEXP bool NstC Nst_sb_push_sv(Nst_StrBuilder *sb, Nst_StrView sv);
/**
 * Add the value of a Nest `Str` object to the end of a `Nst_StrBuilder`.
 *
 * @return `true` on success and `false` if a memory allocation fails. The
 * error is set.
 */
NstEXP bool NstC Nst_sb_push_str(Nst_StrBuilder *sb, Nst_Obj *str);
/**
 * Add a NUL-terminated string (a C-string) to the end of a `Nst_StrBuilder`.
 *
 * @return `true` on success and `false` if a memory allocation fails. The
 * error is set.
 */
NstEXP bool NstC Nst_sb_push_c(Nst_StrBuilder *sb, const char *s);
/**
 * Add an array of codepoints to the end of a `Nst_StrBuilder`. The codepoints
 * are encoded in extUTF-8.
 *
 * @param sb: the string builder to append the value to
 * @param cps: the array of codepoints
 * @param count: the number of codepoints in the array
 *
 * @return `true` on success and `false` if a memory allocation fails. The
 * error is set.
 */
NstEXP bool NstC Nst_sb_push_cps(Nst_StrBuilder *sb, u32 *cps, usize count);
NstEXP bool NstC Nst_sb_push_char(Nst_StrBuilder *sb, char ch);

/**
 * Create a new Nest `Str` object from a `Nst_StrBuilder`. The string builder
 * is emptied and its value is moved to the new string.
 *
 * @return The new object on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_str_from_sb(Nst_StrBuilder *sb);
/* Create a new `Nst_StrView` from a `Nst_StrBuilder`. */
NstEXP Nst_StrView NstC Nst_sv_from_sb(Nst_StrBuilder *sb);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_BUILDER_H