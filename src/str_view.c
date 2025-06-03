#include <string.h>
#include <ctype.h>
#include "nest.h"

#define RETURN_INT_ERR do {                                                   \
    Nst_error_setc_value("invalid Int literal");                              \
    goto error;                                                               \
    } while (0)

#define RETURN_BYTE_ERR do {                                                  \
    Nst_error_setc_value("invalid Byte literal");                             \
    goto error;                                                              \
    } while (0)

#define RETURN_REAL_ERR do {                                                  \
    Nst_error_setc_value("invalid Real literal");                             \
    goto error;                                                               \
    } while (0)

#define REAL_BUF_SIZE 512

Nst_StrView Nst_sv_new(u8 *value, usize len)
{
    Nst_StrView sv = {
        .value = value,
        .len = len
    };
    return sv;
}

Nst_StrView Nst_sv_new_c(const char *value)
{
    Nst_StrView sv = {
        .value = (u8 *)value,
        .len = value == NULL ? 0 : strlen(value)
    };
    return sv;
}

Nst_StrView Nst_sv_from_str(Nst_Obj *str)
{
    Nst_assert(str->type == Nst_t.Str);
    Nst_StrView sv = {
        .value = Nst_str_value(str),
        .len = Nst_str_len(str)
    };
    return sv;
}

Nst_StrView Nst_sv_from_str_slice(Nst_Obj *str, usize start_idx, usize end_idx)
{
    usize char_len = Nst_str_char_len(str);
    if (end_idx > Nst_str_char_len(str))
        end_idx = char_len;

    if (start_idx >= end_idx) {
        return Nst_sv_new(NULL, 0);
    }

    isize i_start = 0;
    for (isize i = Nst_str_next(str, -1); i >= 0; i = Nst_str_next(str, i)) {
        if (start_idx == 0)
            i_start = i;
        if (end_idx == 0)
            return Nst_sv_new(Nst_str_value(str) + i_start, i - i_start);
        start_idx--;
        end_idx--;
    }

    return Nst_sv_new(Nst_str_value(str) + i_start, Nst_str_len(str) - i_start);
}

Nst_ObjRef *Nst_str_from_sv(Nst_StrView sv)
{
    u8 *buf = (u8 *)Nst_calloc(1, sv.len + 1, sv.value);
    if (buf == NULL)
        return NULL;
    buf[sv.len] = '\0';
    return Nst_str_new_allocated(buf, sv.len);
}

isize Nst_sv_next(Nst_StrView sv, isize idx, u32 *ch)
{
    if (sv.len == 0) {
        if (ch != NULL) *ch = 0;
        return -1;
    }

    if (idx >= 0) {
        i32 len = Nst_check_ext_utf8_bytes((u8 *)sv.value + idx, sv.len - idx);
        if (len < 0 || idx + (usize)len >= sv.len) {
            if (ch != NULL) *ch = 0;
            return -1;
        }
        idx += len;
    } else
        idx = 0;
    if (ch != NULL)
        *ch = Nst_ext_utf8_to_utf32((u8 *)sv.value + idx);
    return idx;
}

isize Nst_sv_prev(Nst_StrView sv, isize idx, u32 *ch)
{
    if (sv.len == 0) {
        if (ch != NULL) *ch = 0;
        return -1;
    }

    if (idx < 0)
        idx = (isize)sv.len;

    do {
        idx--;
    } while (idx >= 0 && (sv.value[idx] & 0b11000000) == 0b10000000);

    if (idx == -1) {
        if (ch != NULL) *ch = 0;
        return -1;
    }

    if (ch != NULL)
        *ch = Nst_ext_utf8_to_utf32((u8 *)sv.value + idx);
    return idx;
}

static isize skip_whitespace(Nst_StrView sv, isize offset, u32 *out_final_ch)
{
    u32 ch;
    if (offset == -1)
        offset = Nst_sv_next(sv, -1, &ch);
    else
        ch = Nst_utf8_to_utf32(sv.value + offset);
    for (; offset != -1; offset = Nst_sv_next(sv, offset, &ch)) {
        if (!Nst_unicode_is_whitespace(ch))
            break;
    }
    if (out_final_ch != NULL)
        *out_final_ch = ch;
    return offset;
}

bool Nst_sv_parse_int(Nst_StrView sv, u8 base, u32 flags, u32 sep,
                      i64 *out_num, Nst_StrView *out_rest)
{
    u32 ch;
    i32 sign = 1;

    // Validate base
    if ((base < 2 || base > 36) && base != 0) {
        Nst_error_setc_value("the base must be between 2 and 36");
        goto error;
    }

    // Skip initial whitespace
    isize offset = skip_whitespace(sv, -1, &ch);
    if (offset < 0)
        RETURN_INT_ERR;

    // Parse a possible sign
    if (ch == '-' || ch == '+') {
        sign = ch == '-' ? -1 : 1;
        offset = Nst_sv_next(sv, offset, &ch);
    }
    if (offset < 0)
        RETURN_INT_ERR;

    // Always check for a base prefix, ignore it if the bases don't match
    isize prefix_offset = -1;
    if (ch == '0') {
        prefix_offset = offset;
        offset = Nst_sv_next(sv, offset, &ch);
        if (offset < 0) {
            offset = prefix_offset;
            ch = '0';
        }
        switch (ch) {
        case 'B':
        case 'b':
            if (base == 2 || base == 0) {
                base = 2;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prefix_offset;
                ch = '0';
            }
            break;
        case 'O':
        case 'o':
            if (base == 8 || base == 0) {
                base = 8;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prefix_offset;
                ch = '0';
            }
            break;
        case 'X':
        case 'x':
            if (base == 16 || base == 0) {
                base = 16;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prefix_offset;
                ch = '0';
            }
            break;
        default:
            offset = prefix_offset;
            ch = '0';
            break;
        }
    }
    // If no prefix is found default to base 10
    if (base == 0)
        base = 10;

    bool has_digits = false;
    bool digits_since_sep = false;

    i64 num = 0;
    i64 cut_off = sign == -1 ? -9223372036854775807 - 1 : 9223372036854775807;
    i64 cut_lim = sign * (cut_off % base);
    cut_off /= sign * base;

    while (offset >= 0) {
        i32 ch_val;
        if (ch >= '0' && ch <= '9')
            ch_val = ch - '0';
        else if (ch >= 'a' && ch <= 'z')
            ch_val = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'Z')
            ch_val = ch - 'A' + 10;
        else if (sep != 0 && ch == sep && digits_since_sep) {
            offset = Nst_sv_next(sv, offset, &ch);
            digits_since_sep = false;
            continue;
        } else
            break;

        if (ch_val < 0 || ch_val > base - 1)
            break;

        if (!(flags & Nst_SVFLAG_CAN_OVERFLOW)) {
            if (num > cut_off || (num == cut_off && ch_val > cut_lim)) {
                Nst_error_setc_memory("Int value overflows");
                goto error;
            }
        }
        digits_since_sep = true;
        has_digits = true;
        num *= base;
        num += ch_val;
        offset = Nst_sv_next(sv, offset, &ch);
    }
    if (!has_digits) {
        // If there was a prefix but no digits take the zero from the prefix
        if (prefix_offset == -1)
            RETURN_INT_ERR;
        // Go back to the 0 of the prefix to check for FULL_MATCH
        offset = Nst_sv_next(sv, prefix_offset, NULL);
    } else if (!digits_since_sep && sep != 0)
        // If the number ends with a separator keep it in `rest`
        offset = Nst_sv_prev(sv, offset, &ch);

    // Skip any additional whitespace to allow trailig whitespace in FULL_MATCH
    if (flags & Nst_SVFLAG_FULL_MATCH && offset >= 0)
        offset = skip_whitespace(sv, offset, NULL);

    if (flags & Nst_SVFLAG_FULL_MATCH && offset >= 0)
        RETURN_INT_ERR;

    if (out_num != NULL)
        *out_num = sign * num;
    if (out_rest != NULL) {
        if (offset < 0)
            offset = sv.len;
        *out_rest = Nst_sv_new(sv.value + offset, sv.len - offset);
    }
    return true;

error:
    if (out_num != NULL)
        *out_num = 0;
    if (out_rest != NULL)
        *out_rest = sv;
    return false;
}

bool Nst_sv_parse_byte(Nst_StrView sv, u8 base, u32 flags, u32 sep,
                       u8 *out_num, Nst_StrView *out_rest)
{
    // Validate base
    if ((base < 2 || base > 36) && base != 0) {
        Nst_error_setc_value("the base must be between 2 and 36");
        goto error;
    }

    // Parse single char if requested
    if ((flags & Nst_SVFLAG_CHAR_BYTE)
        && Nst_check_ext_utf8_bytes(sv.value, sv.len) == (isize)sv.len)
    {
        u32 utf32_ch = Nst_ext_utf8_to_utf32(sv.value);
        if (utf32_ch <= 0xff || (flags & Nst_SVFLAG_CAN_OVERFLOW)) {
            if (out_num != NULL)
                *out_num = (u8)(utf32_ch & 0xff);
            if (out_rest != NULL)
                *out_rest = Nst_sv_new(NULL, 0);
            return true;
        } else
            RETURN_BYTE_ERR;
    }

    u32 ch = 0;
    i32 sign = 1;
    bool has_affix = false; // `0h` prefix or `b` suffix

    isize offset = skip_whitespace(sv, -1, &ch);
    if (offset < 0)
        RETURN_BYTE_ERR;

    // Parse a possible sign
    if (ch == '-' || ch == '+') {
        sign = ch == '-' ? -1 : 1;
        offset = Nst_sv_next(sv, offset, &ch);
    }
    if (offset < 0)
        RETURN_BYTE_ERR;

    // Always check for a base prefix, ignore it if the bases don't match
    isize prefix_offset = -1;
    if (ch == '0') {
        prefix_offset = offset;
        offset = Nst_sv_next(sv, offset, &ch);
        if (offset < 0) {
            offset = prefix_offset;
            ch = '0';
        }
        switch (ch) {
        case 'B':
        case 'b':
            if (base == 2 || base == 0) {
                base = 2;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prefix_offset;
                ch = '0';
            }
            break;
        case 'O':
        case 'o':
            if (base == 8 || base == 0) {
                base = 8;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prefix_offset;
                ch = '0';
            }
            break;
        case 'H':
        case 'h':
            if (base == 16 || base == 0) {
                base = 16;
                offset = Nst_sv_next(sv, offset, &ch);
                has_affix = true;
            } else {
                offset = prefix_offset;
                ch = '0';
            }
            break;
        case 'X':
        case 'x':
            if ((base == 16 || base == 0)
                && !(flags & Nst_SVFLAG_CHAR_BYTE))
            {
                base = 16;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prefix_offset;
                ch = '0';
            }
            break;
        default:
            offset = prefix_offset;
            ch = '0';
            break;
        }
    }
    // `0h` is required for CHAR_BYTE
    if (base == 16 && !has_affix && (flags & Nst_SVFLAG_CHAR_BYTE))
        RETURN_BYTE_ERR;
    // If no prefix is found default to base 10
    if (base == 0)
        base = 10;

    bool has_digits = false;
    bool digits_since_sep = false;
    i32 num = 0;

    while (offset >= 0) {
        i32 ch_val;
        if (ch >= '0' && ch <= '9')
            ch_val = ch - '0';
        else if (ch >= 'a' && ch <= 'z')
            ch_val = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'Z')
            ch_val = ch - 'A' + 10;
        else if (sep != 0 && ch == sep && digits_since_sep) {
            offset = Nst_sv_next(sv, offset, &ch);
            digits_since_sep = false;
            continue;
        } else
            break;

        if (ch_val < 0 || ch_val > base - 1)
            break;

        num *= base;
        num += ch_val;

        if (!(flags & Nst_SVFLAG_CAN_OVERFLOW)) {
            if (num > 255 || num * sign < 0) {
                Nst_error_setc_memory("Byte value overflows");
                goto error;
            }
        } else
            num &= 0xff;

        has_digits = true;
        digits_since_sep = true;
        offset = Nst_sv_next(sv, offset, &ch);
    }
    if (!has_digits) {
        // If there was a prefix but no digits take the zero from the prefix
        // When the base is 16 and CHAR_BYTE is set the prefix cannot be taken
        // because it is required
        if (prefix_offset == -1
            || (base == 16 && (flags & Nst_SVFLAG_CHAR_BYTE)))
        {
            RETURN_BYTE_ERR;
        }
        // Go back to the 0 of the prefix to check for FULL_MATCH
        offset = Nst_sv_next(sv, prefix_offset, &ch);
    } else if (!digits_since_sep && sep != 0)
        // If the number ends with a separator keep it in `rest`
        offset = Nst_sv_prev(sv, offset, &ch);

    // Only check for the suffix in bases where `b` is not a digit
    if (base >= 12)
        has_affix = true;
    else if (ch == 'b' || ch == 'B') {
        has_affix = true;
        offset = Nst_sv_next(sv, offset, &ch);
    }

    // Suffix is required for CHAR_BYTE (because of single digit bytes)
    if (flags & Nst_SVFLAG_CHAR_BYTE && !has_affix)
        RETURN_BYTE_ERR;

    // Skip any additional whitespace to allow trailig whitespace in FULL_MATCH
    if (flags & Nst_SVFLAG_FULL_MATCH && offset >= 0)
        offset = skip_whitespace(sv, offset, NULL);

    if (flags & Nst_SVFLAG_FULL_MATCH && offset >= 0)
        RETURN_BYTE_ERR;

    if (out_num != NULL)
        *out_num = (u8)((sign * num) & 0xff);
    if (out_rest != NULL) {
        if (offset < 0)
            offset = sv.len;
        *out_rest = Nst_sv_new(sv.value + offset, sv.len - offset);
    }
    return true;

error:
    if (out_num != NULL)
        *out_num = 0;
    if (out_rest != NULL)
        *out_rest = sv;
    return false;
}

// parse a run of digits with a separator
static isize digit_run(Nst_StrView sv, isize offset, u32 sep, u32 *inout_ch,
                       usize *inout_sep_count, bool *out_has_digits)
{
    u32 ch = *inout_ch;
    bool has_digit_before_sep = false;
    bool has_digits = false;
    usize sep_count = 0;

    while (offset >= 0) {
        if (ch >= '0' && ch <= '9') {
            has_digits = true;
            has_digit_before_sep = true;
        } else if (has_digit_before_sep && ch == sep && sep != 0) {
            sep_count++;
            has_digit_before_sep = false;
        } else
            break;
        offset = Nst_sv_next(sv, offset, &ch);
    }

    if (!has_digit_before_sep && has_digits)
        offset = Nst_sv_prev(sv, offset, &ch);

    if (inout_sep_count != NULL)
        *inout_sep_count += sep_count;
    if (out_has_digits != NULL)
        *out_has_digits = has_digits;
    *inout_ch = ch;

    return offset;
}

static bool parse_real_with_sep(Nst_StrView sv, isize start, isize end,
                                u32 sep, usize sep_count, f64 *out_num)
{
    u8 stack_buf[REAL_BUF_SIZE] = {0};
    u8 *buf = stack_buf;
    usize num_len = (end == -1 ? sv.len : end) - start - sep_count;

    if (num_len > REAL_BUF_SIZE) {
        buf = Nst_calloc(num_len + 1, 1, NULL);
        if (buf == NULL)
            return false;
    }

    u32 ch;
    isize offset = Nst_sv_next(sv, start, &ch);
    memcpy(buf, sv.value + start, offset - start);
    usize buf_idx = offset - start;
    while (offset != end) {
        if (ch == sep && sep != 0) {
            offset = Nst_sv_next(sv, offset, &ch);
            continue;
        }
        isize ch_start = offset;
        offset = Nst_sv_next(sv, offset, &ch);
        usize ch_len = offset < 0 ? sv.len - ch_start : offset - ch_start;
        memcpy(buf + buf_idx, sv.value + ch_start, ch_len);
        buf_idx += ch_len;
    }

    f64 result = Nst_strtod((const char *)buf, NULL);
    if (buf != stack_buf)
        Nst_free(buf);

    *out_num = result;
    return true;
}

bool Nst_sv_parse_real(Nst_StrView sv, u32 flags, u32 sep, f64 *out_num,
                       Nst_StrView *out_rest)
{
    u32 ch;
    isize num_start = skip_whitespace(sv, -1, &ch);
    isize offset = num_start;
    isize num_end = -1;
    f64 val = 0;

    if (offset < 0)
        RETURN_REAL_ERR;

    if (ch == '+' || ch == '-')
        offset = Nst_sv_next(sv, offset, &ch);
    if (offset < 0)
        RETURN_REAL_ERR;

    bool has_digits = false;
    usize sep_count = 0;
    offset = digit_run(sv, offset, sep, &ch, &sep_count, &has_digits);

    if (offset < 0) {
        if (!has_digits || flags & Nst_SVFLAG_STRICT_REAL)
            RETURN_REAL_ERR;
        else
            goto end;
    } else if (!has_digits && flags & Nst_SVFLAG_STRICT_REAL)
        RETURN_REAL_ERR;
    else if (ch != '.') {
        if (!has_digits || flags & Nst_SVFLAG_STRICT_REAL)
            RETURN_REAL_ERR;
        else
            goto exp;
    }

    // Skip the '.'
    offset = Nst_sv_next(sv, offset, &ch);

    bool has_digits_after_dot = false;
    // Sep is disabled
    offset = digit_run(sv, offset, 0, &ch, NULL, &has_digits_after_dot);

    if (!has_digits_after_dot
        && (!has_digits || flags & Nst_SVFLAG_STRICT_REAL))
    {
        RETURN_REAL_ERR;
    }

exp:
    if (ch != 'e' && ch != 'E')
        goto end;

    // No need to save ch, it is not used in `end`
    num_end = offset;
    offset = Nst_sv_next(sv, offset, &ch);
    if (ch == '+' || ch == '-')
        offset = Nst_sv_next(sv, offset, &ch);
    if (offset < 0)
        RETURN_REAL_ERR;

    has_digits = false;
    offset = digit_run(sv, offset, sep, &ch, &sep_count, &has_digits);
    if (has_digits)
        num_end = offset;
end:
    if (num_end < 0)
        num_end = offset;

    // Skip any additional whitespace to allow trailig whitespace in FULL_MATCH
    if (flags & Nst_SVFLAG_FULL_MATCH && offset >= 0)
        offset = skip_whitespace(sv, offset, NULL);
    if (flags & Nst_SVFLAG_FULL_MATCH && offset >= 0)
        RETURN_REAL_ERR;

    // If sep is not set or is a digit sep_count is 0

    if (sep_count == 0)
        val = Nst_strtod((const char *)sv.value + num_start, NULL);
    else if (!parse_real_with_sep(
        sv,
        num_start, num_end,
        sep, sep_count,
        &val))
    {
        goto error;
    }

    if (out_num != NULL)
        *out_num = val;
    if (out_rest != NULL) {
        if (num_end < 0)
            num_end = sv.len;
        *out_rest = Nst_sv_new(sv.value + num_end, sv.len - num_end);
    }
    return true;

error:
    if (out_num != NULL)
        *out_num = 0.0;
    if (out_rest != NULL) {
        *out_rest = sv;
    }
    return false;
}

i32 Nst_sv_compare(Nst_StrView str1, Nst_StrView str2)
{
    u8 *p1 = (u8 *)str1.value;
    u8 *p2 = (u8 *)str2.value;
    u8 *end1 = p1 + str1.len;
    u8 *end2 = p2 + str2.len;

    while (p1 != end1 && p2 != end2) {
        if (*p1 != *p2)
            return *p1 < *p2 ? -1 : 1; // fixed values above 0x7f
        else {
            p1++;
            p2++;
        }
    }

    return (i32)((i64)str1.len - (i64)str2.len);
}

isize Nst_sv_lfind(Nst_StrView str, Nst_StrView substr)
{
    u8 *str_value = str.value;
    u8 *end1 = str.value + str.len;
    u8 *end2 = substr.value + substr.len;
    u8 *p1 = NULL;
    u8 *p2 = NULL;

    if (substr.len > str.len)
        return -1;

    while (str_value != end1) {
        p1 = str_value++;
        p2 = substr.value;

        while (p1 != end1 && p2 != end2 && *p1 == *p2) {
            p1++;
            p2++;
        }

        if (p2 == end2)
            return str_value - 1 - str.value;
    }

    return -1;
}

isize Nst_sv_rfind(Nst_StrView str, Nst_StrView substr)
{
    u8 *p = str.value + str.len - substr.len;

    while (p >= str.value) {
        for (usize i = 0; i < substr.len; i++) {
            if (p[i] != substr.value[i])
                goto next_cycle;
        }
        return p - str.value;

    next_cycle:
        p--;
    }

    return -1;
}

Nst_StrView Nst_sv_ltok(Nst_StrView str, Nst_StrView substr)
{
    isize idx = Nst_sv_lfind(str, substr);
    if (idx == -1)
        return Nst_sv_new(NULL, 0);
    return Nst_sv_new(
        str.value + idx + substr.len,
        str.len - idx - substr.len);
}

Nst_StrView Nst_sv_rtok(Nst_StrView str, Nst_StrView substr)
{
    isize idx = Nst_sv_rfind(str, substr);
    if (idx == -1)
        return Nst_sv_new(NULL, 0);
    return Nst_sv_new(
        str.value + idx + substr.len,
        str.len - idx - substr.len);
}
