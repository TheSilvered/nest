#ifndef UNICODE_DB_H
#define UNICODE_DB_H

#include "typedefs.h"

/* Mask for the Uppercase property flag. */
#define Nst_UCD_MASK_UPPERCASE    0x001
/* Mask for the Lowercase property flag. */
#define Nst_UCD_MASK_LOWERCASE    0x002
/* Mask for the Cased property flag. */
#define Nst_UCD_MASK_CASED        0x004
/* Mask for the Alphabetic property flag. */
#define Nst_UCD_MASK_ALPHABETIC   0x008
/* Mask for the Numeric_Type=Decimal flag. */
#define Nst_UCD_MASK_DECIMAL      0x010
/* Mask for the Numeric_Type=Digit flag. */
#define Nst_UCD_MASK_DIGIT        0x020
/* Mask for the Numeric_Type=Numeric flag. */
#define Nst_UCD_MASK_NUMERIC      0x040
/* Mask for the XID_Start property flag. */
#define Nst_UCD_MASK_XID_START    0x080
/* Mask for the XID_Continue property flag. */
#define Nst_UCD_MASK_XID_CONTINUE 0x100
/* Mask for characters in categories L, N, P, S, Zs. */
#define Nst_UCD_MASK_PRINTABLE    0x200

/* Maximum number of characters that `Nst_unicode_expand_case` can produce. */
#define Nst_UCD_MAX_CASE_EXPANSION 3

/**
 * Check if a character is titlecase. The character is titlecase if it is
 * cased but not uppercase nor lowercase.
 *
 * @param flags: the character's flags from its `Nst_UnicodeChInfo`
 */
#define Nst_UCD_IS_TITLECASE(flags) \
    (((flags) & Nst_UCD_MASK_CASED) \
        && !((flags) & Nst_UCD_MASK_UPPERCASE) \
        && !((flags) & Nst_UCD_MASK_LOWERCASE))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Unicode character information.
 *
 * @param lower: lowercase mapping, use in `Nst_unicode_expand_case`
 * @param upper: uppercase mapping, use in `Nst_unicode_expand_case`
 * @param title: titlecase mapping, use in `Nst_unicode_expand_case`
 * @param flags: character flags use `Nst_UCD_MASK_*` to query
 */
typedef struct _Nst_UnicodeChInfo {
    i32 lower, upper, title;
    u32 flags;
} Nst_UnicodeChInfo;

/* Get the `Nst_UnicodeChInfo` about a character. */
Nst_UnicodeChInfo Nst_unicode_get_ch_info(u32 codepoint);
/**
 * Changes a codepoint's case.
 *
 * @param codepoint: the codepoint to change the case of
 * @param casing: the casing to change to, pass to this parameter `lower`,
 * `upper` or `title` of `Nst_UnicodeChInfo`
 * @param out_codepoints: an array where the resulting codepoint(s) are placed,
 * it can be `NULL`
 *
 * @return The number of codepoints to which the codepoint expands, which is
 * the number of codepoints written to `out_codepoints` if it is not `NULL`.
 * This number is less than or equal to `Nst_UCD_MAX_CASE_EXPANSION`
 */
usize Nst_unicode_expand_case(u32 codepoint, u32 casing, u32 *out_codepoints);
/* Check if a character is whitespace. */
bool Nst_unicode_is_whitespace(u32 codepoint);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !UNICODE_DB_H