#include <string.h>
#include "nest.h"

#define RETURN_INT_ERR do {                                                   \
    Nst_error_setc_value("invalid Int literal");                              \
    return NULL;                                                              \
    } while (0)

#define RETURN_BYTE_ERR do {                                                  \
    Nst_error_setc_value("invalid Byte literal");                             \
    return NULL;                                                              \
    } while (0)

#define RETURN_REAL_ERR do {                                                  \
    Nst_error_setc_value("invalid Real literal");                             \
    return NULL;                                                              \
    } while (0)

#define ERR_IF_END(s, end, err_macro) do {                                    \
    if (s == end)                                                             \
        err_macro;                                                            \
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
        .len = strlen(value)
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

Nst_Obj *Nst_str_from_sv(Nst_StrView sv)
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

isize Nst_sv_nextr(Nst_StrView sv, isize idx, u32 *ch)
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

Nst_Obj *Nst_sv_parse_int(Nst_StrView sv, i32 base)
{
    u8 *s = sv.value;
    u8 *end = s + sv.len;
    u8 ch;
    i32 ch_val;
    i32 sign = 1;
    i64 num = 0;
    i64 cut_off = 0;
    i64 cut_lim = 0;

    if ((base < 2 || base > 36) && base != 0) {
        Nst_error_setc_value("the base must be between 2 and 36");
        return NULL;
    }
    ERR_IF_END(s, end, RETURN_INT_ERR);

    ch = *s;
    while (isspace(ch))
        ch = *++s;
    ERR_IF_END(s, end, RETURN_INT_ERR);

    if (ch == '-' || ch == '+') {
        sign = ch == '-' ? -1 : 1;
        ch = *++s;
    }
    ERR_IF_END(s, end, RETURN_INT_ERR);

    if (ch == '0') {
        ch = *++s;
        switch (ch) {
        case 'B':
        case 'b':
            if (base == 2 || base == 0) {
                base = 2;
                ch = *++s;
            } else
                ch = *--s;
            break;
        case 'O':
        case 'o':
            if (base == 8 || base == 0) {
                base = 8;
                ch = *++s;
            } else
                ch = *--s;
            break;
        case 'X':
        case 'x':
            if (base == 16 || base == 0) {
                base = 16;
                ch = *++s;
            } else
                ch = *--s;
            break;
        default:
            ch = *--s;
            break;
        }
    }
    ERR_IF_END(s, end, RETURN_INT_ERR);
    if (base == 0)
        base = 10;

    cut_off = sign == -1 ? -9223372036854775807 - 1 : 9223372036854775807;
    cut_lim = sign * (cut_off % base);
    cut_off /= sign * base;
    while (true) {
        if (ch >= '0' && ch <= '9')
            ch_val = ch - '0';
        else if (ch >= 'a' && ch <= 'z')
            ch_val = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'Z')
            ch_val = ch - 'A' + 10;
        else if (ch == '_') {
            ch = *++s;
            continue;
        } else
            break;

        if (ch_val < 0 || ch_val > base - 1)
            RETURN_INT_ERR;

        if (num > cut_off || (num == cut_off && ch_val > cut_lim)) {
            Nst_error_setc_memory("Int literal's value is too large");
            return NULL;
        }
        num *= base;
        num += ch_val;
        ch = *++s;
    }

    while (isspace((u8)ch))
        ch = *++s;

    if (s != end)
        RETURN_INT_ERR;

    return Nst_int_new(num * sign);
}

Nst_Obj *Nst_sv_parse_byte(Nst_StrView sv)
{
    if (Nst_check_ext_utf8_bytes(sv.value, sv.len) == (isize)sv.len) {
        u32 utf32_ch = Nst_ext_utf8_to_utf32(sv.value);
        if (utf32_ch <= 0xff)
            return Nst_byte_new((u8)utf32_ch);
        else
            RETURN_BYTE_ERR;
    }

    u8 *s = sv.value;
    u8 *end = s + sv.len;
    u8 ch = *s;
    i32 num = 0;
    i32 ch_val = 0;
    i32 sign = 1;
    i32 base = 10;

    ERR_IF_END(s, end, RETURN_BYTE_ERR);

    while (isspace(ch))
        ch = *++s;
    ERR_IF_END(s, end, RETURN_BYTE_ERR);

    if (ch == '-') {
        sign = -1;
        ch = *++s;
    } else if (ch == '+')
        ch = *++s;
    ERR_IF_END(s, end, RETURN_BYTE_ERR);

    if (ch == '0') {
        ch = *++s;
        switch (ch) {
        case 'b':
        case 'B':
            base = 2;
            ch = *++s;
            break;
        case 'o':
        case 'O':
            base = 8;
            ch = *++s;
            break;
        case 'h':
        case 'H':
            base = 16;
            ch = *++s;
            break;
        default:
            ch = *--s;
        }
    }

    bool has_digits = false;
    while (true) {
        if (ch >= '0' && ch <= '9')
            ch_val = ch - '0';
        else if (ch >= 'a' && ch <= 'f' && base == 16)
            ch_val = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F' && base == 16)
            ch_val = ch - 'A' + 10;
        else if (ch == '_') {
            ch = *++s;
            continue;
        } else
            break;

        if (ch_val >= base)
            RETURN_BYTE_ERR;
        has_digits = true;
        num *= base;
        num += ch_val;
        num %= 256;
        ch = *++s;
    }
    num *= sign;

    if (base != 2 && !has_digits)
        RETURN_BYTE_ERR;

    if ((base != 2 || has_digits) && base != 16 && ch != 'b' && ch != 'B')
        RETURN_BYTE_ERR;
    if ((base != 2 || has_digits) && base != 16)
        ch = *++s;

    while (isspace(ch))
        ch = *++s;
    if (s != end)
        RETURN_BYTE_ERR;
    return Nst_byte_new(num & 0xff);
}

Nst_Obj *Nst_sv_parse_real(Nst_StrView sv)
{
    // \s*[+-]?\d+\.\d+(?:[eE][+-]?\d+)

    // strtod accepts also things like .5, 1e2, -1., ecc. that I do not
    // so I need to check if the literal is valid first
    u8 *s = sv.value;
    u8 *end = s + sv.len;
    u8 *start = s;
    usize len = 0;
    u8 ch = *s;
    u8 *buf;
    f64 res;
    bool contains_underscores = false;

    if (s == end)
        RETURN_REAL_ERR;

    while (isspace(ch)) {
        ch = *++s;
        start++;
    }
    ERR_IF_END(s, end, RETURN_REAL_ERR);

    if (ch == '+' || ch == '-') {
        ch = *++s;
        len++;
    }
    ERR_IF_END(s, end, RETURN_REAL_ERR);

    if (ch < '0' || ch > '9')
        RETURN_REAL_ERR;
    while ((ch >= '0' && ch <= '9') || ch == '_') {
        ch = *++s;
        len++;
        if (ch == '_')
            contains_underscores = true;
    }

    if (ch != '.') {
        while (isspace(ch))
            ch = *++s;
        if (s != end)
            RETURN_REAL_ERR;
        goto end;
    }
    ch = *++s;
    len++;

    if (ch < '0' || ch > '9')
        RETURN_REAL_ERR;
    while ((ch >= '0' && ch <= '9') || ch == '_') {
        ch = *++s;
        len++;

        if (ch == '_')
            contains_underscores = true;
    }

    if (ch == 'e' || ch == 'E') {
        ch = *++s;
        len++;

        if (ch == '+' || ch == '-') {
            ch = *++s;
            len++;
        }
        ERR_IF_END(s, end, RETURN_REAL_ERR);

        if (ch < '0' || ch > '9')
            RETURN_REAL_ERR;
        while ((ch >= '0' && ch <= '9') || ch == '_') {
            ch = *++s;
            len++;
            if (ch == '_')
                contains_underscores = true;
        }
    }

    while (isspace(ch))
        ch = *++s;
    if (s != end)
        RETURN_REAL_ERR;
end:
    if (contains_underscores) {
        buf = Nst_malloc_c(len + 1, u8);
        if (buf == NULL)
            return NULL;
        s = buf;
        while (len--) {
            if ((ch = *start++) != '_')
                *s++ = ch;
        }
        *s = '\0';
    } else
        buf = start;

    res = Nst_strtod((char *)buf, NULL);
    if (contains_underscores)
        Nst_free(buf);
    return Nst_real_new(res);
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
