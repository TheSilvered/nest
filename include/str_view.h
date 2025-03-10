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
NstEXP Nst_Obj *NstC Nst_str_from_sv(Nst_StrView sv);

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
NstEXP isize NstC Nst_sv_nextr(Nst_StrView sv, isize idx, u32 *out_ch);

// These will probably change, I will keep them undocumented for now

NstEXP Nst_Obj *NstC Nst_sv_parse_int(Nst_StrView sv, i32 base);
NstEXP Nst_Obj *NstC Nst_sv_parse_byte(Nst_StrView sv);
NstEXP Nst_Obj *NstC Nst_sv_parse_real(Nst_StrView sv);

/**
 * Compare two `Nst_StrView`'s.
 *
 * @return The one of the following values:
 *! `> 0`: `str1 > str2`
 *! `== 0`: `str1 == str2`
 *! `< 0`: `str1 < str2`
 */
NstEXP i32 NstC Nst_sv_compare(Nst_StrView str1, Nst_StrView str2);
/**
 * Searches for `substr` inside `str` from the beginning.
 *
 * @return The index where the first occurrence of `substr` appears. If
 * `substr` is not found in `str` then the function returns `-1`.
 */
NstEXP isize NstC Nst_sv_lfind(Nst_StrView str, Nst_StrView substr);
/**
 * Searches for `substr` inside `str` from the end.
 *
 * @return The index where the last occurrence of `substr` appears. If `substr`
 * is not found in `str` then the function returns `-1`.
 */
NstEXP isize NstC Nst_sv_rfind(Nst_StrView str, Nst_StrView substr);
/**
 * Searches for `substr` inside `str` from the beginning.
 *
 * @return A new string view starting from the end of the first occurrence of
 * `substr`. If `substr` is not found, the view will have length `0` and a
 * `NULL` value.
 */
NstEXP Nst_StrView NstC Nst_sv_ltok(Nst_StrView str, Nst_StrView substr);
/**
 * Searches for `substr` inside `str` from the end.
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
