/**
 * @file str_view.h
 *
 * @brief String view
 *
 * @author TheSilvered
 */

#ifndef STR_VIEW_H
#define STR_VIEW_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * A structure representing a string view. It does not own the data in `value`.
 *
 * @param value: the string data
 * @param len: the string length
 */
NstEXP typedef struct _Nst_StrView {
    u8 *value;
    usize len;
} Nst_StrView;

/* Create a new `Nst_StrView` given the `value` and the `len`. */
NstEXP Nst_StrView NstC Nst_sv_new(u8 *value, usize len);
/* Create a new `Nst_StrView` given a NUL-terminated string. */
NstEXP Nst_StrView NstC Nst_sv_new_c(const char *value);
/* Create a new `Nst_StrView` from a Nest `Str` object. */
NstEXP Nst_StrView NstC Nst_sv_from_str(Nst_Obj *str);
/**
 * Create a new `Nst_StrView` from a slice of a Nest `Str` object.
 *
 * @param str: the string to slice
 * @param start_idx: the starting character index, included in the slice
 * @param end_idx: the ending character index, excluded from the slice
 *
 * @return The new `Nst_StrView`. It will have a `len` of `0` and a `value` of
 * `NULL` if the indices are the same or are invalid. Any `end_idx` beyond the
 * end of the string is clamped to the length of the string.
 */
NstEXP Nst_StrView NstC Nst_sv_from_str_slice(Nst_Obj *str, usize start_idx,
                                              usize end_idx);

/* Create a new Nest `Str` object from an `Nst_StrView`. */
NstEXP Nst_ObjRef *NstC Nst_str_from_sv(Nst_StrView sv);

/**
 * Iterate through a `Nst_StrView`, assuming it contains `UTF-8`-encoded data.
 *
 * @param sv: the string view to iterate through
 * @param idx: the current iteration index, use `-1` to begin iterating
 * @param out_ch: pointer set to the codepoint of the character, it may be
 * `NULL`
 *
 * @return The next index to pass to this function to continue iterating or a
 * negative number when no more characters are available.
 */
NstEXP isize NstC Nst_sv_next(Nst_StrView sv, isize idx, u32 *out_ch);
/**
 * Iterate through a `Nst_StrView` from the end, assuming it contains
 * `UTF-8`-encoded data.
 *
 * @param sv: the string view to iterate through
 * @param idx: the current iteration index, use `-1` to begin iterating
 * @param out_ch: pointer set to the codepoint of the character, it may be
 * `NULL`
 *
 * @return The next index to pass to this function to continue iterating or a
 * negative number when no more characters are available.
 */
NstEXP isize NstC Nst_sv_prev(Nst_StrView sv, isize idx, u32 *out_ch);

/* [docs:link Nst_SVFLAG_CAN_OVERFLOW Nst_SvNumFlags] */
/* [docs:link Nst_SVFLAG_FULL_MATCH Nst_SvNumFlags] */
/* [docs:link Nst_SVFLAG_CHAR_BYTE Nst_SvNumFlags] */
/* [docs:link Nst_SVFLAG_DISABLE_SEP Nst_SvNumFlags] */
/* [docs:link Nst_SVFLAG_STRICT_REAL Nst_SvNumFlags] */

/**
 * @brief Flags for `Nst_sv_parse_int`, `Nst_sv_parse_byte` and
 * `Nst_sv_parse_real`.
 */
NstEXP typedef enum _Nst_SvNumFlags {
    Nst_SVFLAG_CAN_OVERFLOW = 1,
    Nst_SVFLAG_FULL_MATCH   = 2,
    Nst_SVFLAG_CHAR_BYTE    = 4,
    Nst_SVFLAG_STRICT_REAL  = 8
} Nst_SvNumFlags;

/**
 * Parse an integer from `sv`. Any leading whitespace is ignored. Whitespace is
 * determined using `Nst_unicode_is_whitespace`.
 *
 * @brief The following flags are accepted and modify the behaviour of the
 * function:
 *! `Nst_SVFLAG_CAN_OVERFLOW`: allows the integer to overflow over 2^63-1 or
 * underflow below `-2^63`;
 *! `Nst_SVFLAG_FULL_MATCH`: requires the whole string to be matched as the
 * number leaving no characters behind, whitespace after the number is
 * trimmed;
 *
 * @brief The integer is composed of the following parts:
 *! an optional sign (`+` or `-`)
 *! an optional `0b` or `0B` prefix if `base` is `0` or `2`;
 *! an optional `0o` or `0O` prefix if `base` is `0` or `8`;
 *! an optional `0x` or `0X` prefix if `base` is `0` or `16`;
 *! a run of digits, optionally separated by `sep`, there must be at least one
 * digit between two separators.
 *
 * @brief The function parses as many digits as it can. If the base is `0` it
 * is determined with the prefix: (`0b` or `0B` for binary, `0o` or `0O` for
 * octal, `0x` or `0X` for hexadecimal and no prefix for decimal).
 *
 * @brief For bases 2 to 10 the run of digits consists of characters `[0-9]`
 * from base 11 to 36 the letters `[a-z]` and `[A-Z]` are used, the casing is
 * ignored. Hence, valid bases are `{0, 2, 3, ..., 35, 36}`.
 *
 * @param sv: the string view to parse
 * @param base: the integer base
 * @param flags: parsing flags
 * @param sep: an digit separator, if set to 0 no separator is allowed
 * @param out_num: pointer set to the parsed number, can be NULL
 * @param out_rest: pointer set to the remaining part of `sv`, can be NULL; any
 * whitespace after the number is always trimmed
 *
 * @return `true` on success and `false` on failure. On failure `out_num` is
 * set to `0` if not NULL and `out_rest` is equal to `sv`.
 */
NstEXP bool NstC Nst_sv_parse_int(Nst_StrView sv, u8 base, u32 flags, u32 sep,
                                  i64 *out_num, Nst_StrView *out_rest);
/**
 * Parse an unsigned byte from `sv`. Any leading whitespace is ignored.
 * Whitespace is determined using `Nst_unicode_is_whitespace`.
 *
 * @brief The following flags are accepted and modify the behaviour of the
 * function:
 *! `Nst_SVFLAG_CAN_OVERFLOW`: allows the byte to overflow over 255 or
 * underflow below `0`;
 *! `Nst_SVFLAG_FULL_MATCH`: requires the whole string to be matched as the
 * number leaving no characters behind, whitespace after the number is
 * trimmed;
 *! `Nst_SVFLAG_CHAR_BYTE`: allows single ASCII characters to be parsed as
 * bytes.
 *
 * @brief The byte is composed of the following parts:
 *! an optional sign (`+` or `-`), if there is a minus sign and overflow is not
 * allowed only `'-0'` is a valid byte.
 *! an optional `0b` or `0B` prefix if `base` is `0` or `2`;
 *! an optional `0o` or `0O` prefix if `base` is `0` or `8`
 *! an optional `0x`, `0X`, `0h` or `0H` prefix if `base` is `0` or `16`
 *! a run of digits, optionally separated by underscores (`_`)
 *! an optional suffix `b` or `B`, this is not considered with `base > 10` as
 * `b` becomes a digit. This suffix is required when `Nst_SVFLAG_CHAR_BYTE`
 * is set.
 *
 * @brief The function parses as many digits as it can. If the base is `0` it
 * is determined with the prefix: (`0b` or `0B` for binary, `0o` or `0O` for
 * octal, `0x` or `0X` for hexadecimal and no prefix for decimal).
 *
 * @brief For bases 2 to 10 the run of digits consists of characters `[0-9]`
 * from base 11 to 36 the letters `[a-z]` and `[A-Z]` are used, the casing is
 * ignored. Hence, valid bases are `{0, 2, 3, ..., 35, 36}`.
 *
 * @param sv: the string view to parse
 * @param base: the number base
 * @param flags: parsing flags
 * @param out_num: pointer set to the parsed number, can be NULL
 * @param out_rest: pointer set to the remaining part of `sv`, can be NULL; any
 * whitespace after the number is always trimmed
 *
 * @return `true` on success and `false` on failure. On failure `out_num` is
 * set to `0` if not NULL and `out_rest` is equal to `sv`.
 */
NstEXP bool NstC Nst_sv_parse_byte(Nst_StrView sv, u8 base, u32 flags, u32 sep,
                                   u8 *out_num, Nst_StrView *out_rest);
/**
 * Parse a double precision floating point number from `sv`. Any leading
 * whitespace is ignored. Whitespace is determined using
 * `Nst_unicode_is_whitespace`.
 *
 * @brief The following flags are accepted and modify the behaviour of the
 * function:
 *! `Nst_SVFLAG_FULL_MATCH`: requires the whole string to be matched as the
 * number leaving no characters behind, whitespace after the number is
 * trimmed;
 *! `Nst_SVFLAG_DISABLE_SEP`: if set does not allow `_` to be used as a
 * separator;
 *! `Nst_SVFLAG_STRICT_REAL`: requires the decimal point and that one digit be
 * inserted both before and after it.
 *
 * @brief The number is composed of the following parts:
 *! an optional sign (`+` or `-`)
 *! an optional run of digits, optionally separated by underscores (`_`),
 * required with `Nst_SVFLAG_STRICT_REAL`
 *! an optional dot (`.`), required with `Nst_SVFLAG_STRICT_REAL`
 *! an optional run of digits, optionally separated by underscores (`_`),
 * required with `Nst_SVFLAG_STRICT_REAL`
 *! an optional exponent: `e` or `E` followed by an optional sign (`+` or `-`)
 * followed by a run of digits, optionally separated by underscores (`_`)
 *
 * @brief Note: at least one digit is always required between the sign and the
 * exponent.
 *
 * @param sv: the string view to parse
 * @param flags: parsing flags
 * @param out_num: pointer set to the parsed number, can be NULL
 * @param out_rest: pointer set to the remaining part of `sv`, can be NULL; any
 * whitespace after the number is always trimmed
 *
 * @return `true` on success and `false` on failure. On failure `out_num` is
 * set to `0` if not NULL and `out_rest` is equal to `sv`.
 */
NstEXP bool NstC Nst_sv_parse_real(Nst_StrView sv, u32 flags, u32 sep,
                                   f64 *out_num, Nst_StrView *out_rest);

/**
 * Compare two `Nst_StrView`'s.
 *
 * @return The one of the following values:
 *! `> 0` if `str1 > str2`
 *! `== 0` if `str1 == str2`
 *! `< 0` if `str1 < str2`
 */
NstEXP i32 NstC Nst_sv_compare(Nst_StrView str1, Nst_StrView str2);
/**
 * Search for `substr` inside `str` from the beginning.
 *
 * @return The index where the first occurrence of `substr` appears. If
 * `substr` is not found in `str` then the function returns `-1`.
 */
NstEXP isize NstC Nst_sv_lfind(Nst_StrView str, Nst_StrView substr);
/**
 * Search for `substr` inside `str` from the end.
 *
 * @return The index where the last occurrence of `substr` appears. If `substr`
 * is not found in `str` then the function returns `-1`.
 */
NstEXP isize NstC Nst_sv_rfind(Nst_StrView str, Nst_StrView substr);
/**
 * Search for `substr` inside `str` from the beginning.
 *
 * @return A new string view starting from the end of the first occurrence of
 * `substr`. If `substr` is not found, the view will have length `0` and a
 * `NULL` value.
 */
NstEXP Nst_StrView NstC Nst_sv_ltok(Nst_StrView str, Nst_StrView substr);
/**
 * Search for `substr` inside `str` from the end.
 *
 * @return A new string view starting from the end of the last occurrence of
 * `substr`. If `substr` is not found, the view will have length `0` and a
 * `NULL` value.
 */
NstEXP Nst_StrView NstC Nst_sv_rtok(Nst_StrView str, Nst_StrView substr);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_VIEW_H
