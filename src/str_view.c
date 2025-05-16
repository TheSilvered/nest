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

bool Nst_sv_parse_int(Nst_StrView sv, u8 base, u32 flags, i64 *out_num,
                      Nst_StrView *out_rest)
{
    u32 ch;
    i32 sign = 1;
    i64 num = 0;
    i64 cut_off = 0;
    i64 cut_lim = 0;

    if ((base < 2 || base > 36) && base != 0) {
        Nst_error_setc_value("the base must be between 2 and 36");
        goto error;
    }

    isize offset = skip_whitespace(sv, -1, &ch);
    if (offset < 0)
        RETURN_INT_ERR;

    if (ch == '-' || ch == '+') {
        sign = ch == '-' ? -1 : 1;
        offset = Nst_sv_next(sv, offset, &ch);
    }
    if (offset < 0)
        RETURN_INT_ERR;

    if (ch == '0') {
        isize prev_offset = offset;
        offset = Nst_sv_next(sv, offset, &ch);
        if (offset < 0) {
            offset = prev_offset;
            ch = '0';
        }
        switch (ch) {
        case 'B':
        case 'b':
            if (base == 2 || base == 0) {
                base = 2;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prev_offset;
                ch = '0';
            }
            break;
        case 'O':
        case 'o':
            if (base == 8 || base == 0) {
                base = 8;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prev_offset;
                ch = '0';
            }
            break;
        case 'X':
        case 'x':
            if (base == 16 || base == 0) {
                base = 16;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prev_offset;
                ch = '0';
            }
            break;
        default:
            offset = prev_offset;
            ch = '0';
            break;
        }
        if (offset < 0) {
            offset = prev_offset;
            ch = '0';
            base = 10;
        }
    }
    if (base == 0)
        base = 10;

    bool has_digits = false;
    cut_off = sign == -1 ? -9223372036854775807 - 1 : 9223372036854775807;
    cut_lim = sign * (cut_off % base);
    cut_off /= sign * base;
    while (offset >= 0) {
        i32 ch_val;
        if (ch >= '0' && ch <= '9')
            ch_val = ch - '0';
        else if (ch >= 'a' && ch <= 'z')
            ch_val = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'Z')
            ch_val = ch - 'A' + 10;
        else if (ch == '_' && !(flags & Nst_SVFLAG_DISABLE_SEP)) {
            offset = Nst_sv_next(sv, offset, &ch);
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
        has_digits = true;
        num *= base;
        num += ch_val;
        offset = Nst_sv_next(sv, offset, &ch);
    }
    if (!has_digits)
        RETURN_INT_ERR;

    if (offset >= 0)
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

bool Nst_sv_parse_byte(Nst_StrView sv, u8 base, u32 flags, u8 *out_num,
                       Nst_StrView *out_rest)
{
    if (flags & Nst_SVFLAG_CHAR_BYTE
        && Nst_check_ext_utf8_bytes(sv.value, sv.len) == (isize)sv.len)
    {
        u32 utf32_ch = Nst_ext_utf8_to_utf32(sv.value);
        if (utf32_ch <= 0xff) {
            if (out_num != NULL)
                *out_num = (u8)utf32_ch;
            if (out_rest != NULL)
                *out_rest = Nst_sv_new(NULL, 0);
            return true;
        } else
            RETURN_BYTE_ERR;
    }

    if ((base < 2 || base > 36) && base != 0) {
        Nst_error_setc_value("the base must be between 2 and 36");
        goto error;
    }

    u32 ch = 0;
    i32 num = 0;
    i32 sign = 1;

    isize offset = skip_whitespace(sv, 0, &ch);
    if (offset < 0)
        RETURN_BYTE_ERR;

    if (ch == '-') {
        sign = -1;
        offset = Nst_sv_next(sv, offset, &ch);
    } else if (ch == '+')
        offset = Nst_sv_next(sv, offset, &ch);
    if (offset < 0)
        RETURN_BYTE_ERR;

    if (ch == '0') {
        isize prev_offset = offset;
        offset = Nst_sv_next(sv, offset, &ch);
        if (offset < 0) {
            offset = prev_offset;
            ch = '0';
        }
        switch (ch) {
        case 'B':
        case 'b':
            if (base == 2 || base == 0) {
                base = 2;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prev_offset;
                ch = '0';
            }
            break;
        case 'O':
        case 'o':
            if (base == 8 || base == 0) {
                base = 8;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prev_offset;
                ch = '0';
            }
            break;
        case 'H':
        case 'h':
            if (base == 16 || base == 0) {
                base = 16;
                offset = Nst_sv_next(sv, offset, &ch);
            } else {
                offset = prev_offset;
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
                offset = prev_offset;
                ch = '0';
            }
            break;
        default:
            offset = prev_offset;
            ch = '0';
            break;
        }
        if (offset < 0) {
            offset = prev_offset;
            ch = '0';
            base = 10;
        }
    }
    if (base == 0)
        base = 10;

    bool has_digits = false;
    while (offset >= 0) {
        i32 ch_val;
        if (ch >= '0' && ch <= '9')
            ch_val = ch - '0';
        else if (ch >= 'a' && ch <= 'z')
            ch_val = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'Z')
            ch_val = ch - 'A' + 10;
        else if (ch == '_' && !(flags & Nst_SVFLAG_DISABLE_SEP)) {
            offset = Nst_sv_next(sv, offset, &ch);
            continue;
        } else
            break;

        if (ch_val < 0 || ch_val > base - 1)
            break;

        num *= base;
        num += ch_val;
        has_digits = true;

        if (!(flags & Nst_SVFLAG_CAN_OVERFLOW)) {
            if (num > 255 || num * sign < 0) {
                Nst_error_setc_memory("Byte value overflows");
                goto error;
            }
        } else
            num &= 0xff;

        offset = Nst_sv_next(sv, offset, &ch);
    }

    bool has_suffix = false;
    if (!has_digits && base == 2) // if the literal is 0b without other digits
        has_suffix = true;
    else if (base >= 12)
        has_suffix = true;
    else if (ch == 'b' || ch == 'B') {
        has_suffix = true;
        offset = Nst_sv_next(sv, offset, &ch);
    }

    if (flags & Nst_SVFLAG_CHAR_BYTE && !has_suffix)
        RETURN_BYTE_ERR;

    if (offset >= 0)
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

bool Nst_sv_parse_real(Nst_StrView sv, u32 flags, f64 *out_num,
                       Nst_StrView *out_rest)
{
    u32 ch;
    u8 *buf;
    f64 res;
    bool contains_underscores = false;

    isize offset_without_exp = -1;
    isize offset = skip_whitespace(sv, 0, &ch);
    if (offset < 0)
        RETURN_REAL_ERR;

    if (ch == '+' || ch == '-')
        offset = Nst_sv_next(sv, offset, &ch);
    if (offset < 0)
        RETURN_REAL_ERR;

    bool has_digit = false;
    bool has_digit_before_exp = false;

    while (offset >= 0) {
        if (ch >= '0' && ch <= '9')
            has_digit = true;
        else if (ch == '_' && !(flags & Nst_SVFLAG_DISABLE_SEP))
            contains_underscores = true;
        else
            break;
        offset = Nst_sv_next(sv, offset, &ch);
    }

    if (!has_digit && ch != '.')
        RETURN_REAL_ERR;
    else if (has_digit && ch != '.') {
        if (flags & Nst_SVFLAG_STRICT_REAL)
            RETURN_REAL_ERR;
        else
            goto exp;
    } else if (!has_digit && ch == '.') {
        if (flags & Nst_SVFLAG_STRICT_REAL)
            RETURN_REAL_ERR;
        offset = Nst_sv_next(sv, offset, &ch);
    } else
        offset = Nst_sv_next(sv, offset, &ch);

    has_digit_before_exp = has_digit;
    has_digit = false;

    while (offset >= 0) {
        if (ch >= '0' && ch <= '9')
            has_digit = true;
        else if (ch == '_' && !(flags & Nst_SVFLAG_DISABLE_SEP))
            contains_underscores = true;
        else
            break;
        offset = Nst_sv_next(sv, offset, &ch);
    }

    if (!has_digit && flags & Nst_SVFLAG_STRICT_REAL)
        RETURN_REAL_ERR;
    has_digit_before_exp = has_digit_before_exp || has_digit;

    if (!has_digit_before_exp)
        RETURN_REAL_ERR;

exp:
    // if the exponent is not valid and the syntax is not strict this is where
    // the number actually ends
    offset_without_exp = offset;

    if (ch == 'e' || ch == 'E') {
        offset = Nst_sv_next(sv, offset, &ch);

        if (ch == '+' || ch == '-')
            offset = Nst_sv_next(sv, offset, &ch);

        has_digit = false;
        while (true) {
            if (ch >= '0' && ch <= '9')
                has_digit = true;
            else if (ch == '_' && !(flags & Nst_SVFLAG_DISABLE_SEP))
                contains_underscores = true;
            else
                break;
            offset = Nst_sv_next(sv, offset, &ch);
        }
        if (!has_digit) {
            if (flags & Nst_SVFLAG_STRICT_REAL)
                RETURN_REAL_ERR;
            else
                offset = offset_without_exp;
        }
    }

    if (offset >= 0)
        offset = skip_whitespace(sv, offset, &ch);
    if (flags & Nst_SVFLAG_FULL_MATCH && offset >= 0)
        RETURN_REAL_ERR;

    if (contains_underscores) {
        if (offset < 0)
            offset = sv.len;
        buf = Nst_malloc_c(offset, u8);
        if (buf == NULL)
            return false;
        u8 *p = buf;
        for (usize i = 0; i < (usize)offset; i++) {
            if (sv.value[i] != '_')
                *p++ = sv.value[i];
        }
        *p = '\0';
    } else
        buf = sv.value;

    res = Nst_strtod((char *)buf, NULL);
    if (contains_underscores)
        Nst_free(buf);

    if (out_num != NULL)
        *out_num = res;
    if (out_rest != NULL) {
        if (offset < 0)
            offset = sv.len;
        *out_rest = Nst_sv_new(sv.value + offset, sv.len - offset);
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
