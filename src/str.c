#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "mem.h"
#include "str.h"
#include "error.h"
#include "lib_import.h"
#include "lexer.h"
#include "encoding.h"
#include "format.h"

#define IS_WHITESPACE(ch)                                                     \
        (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == '\v'    \
         || ch == '\f')

#define RETURN_INT_ERR do {                                                   \
    Nst_set_value_error_c(_Nst_EM_BAD_INT_LITERAL);                           \
    return NULL;                                                              \
    } while (0)

#define RETURN_BYTE_ERR do {                                                  \
    Nst_set_value_error_c(_Nst_EM_BAD_BYTE_LITERAL);                          \
    return NULL;                                                              \
    } while (0)

#define RETURN_REAL_ERR do {                                                  \
    Nst_set_value_error_c(_Nst_EM_BAD_REAL_LITERAL);                          \
    return NULL;                                                              \
    } while (0)

#define ERR_IF_END(s, end, err_macro) do {                                    \
    if (s == end)                                                             \
        err_macro;                                                            \
    } while (0)

Nst_Obj *Nst_string_new_c_raw(const i8 *val, bool allocated)
{
    return Nst_string_new((i8 *)val, strlen(val), allocated);
}

Nst_Obj *Nst_string_new_c(const i8 *val, usize len, bool allocated)
{
    return Nst_string_new((i8 *)val, len, allocated);
}

Nst_Obj *Nst_string_new(i8 *val, usize len, bool allocated)
{
    Nst_StrObj *str = Nst_obj_alloc(
        Nst_StrObj,
        Nst_t.Str,
        _Nst_string_destroy);
    if (str == NULL)
        return NULL;

    if (allocated)
        str->flags |= Nst_FLAG_STR_IS_ALLOC;
    str->len = len;
    str->value = val;

    return OBJ(str);
}

Nst_Obj *Nst_string_new_allocated(i8 *val, usize len)
{
    Nst_Obj *str = Nst_string_new(val, len, true);
    if (str == NULL) {
        Nst_free(val);
        return NULL;
    }
    return str;
}

Nst_StrObj Nst_string_temp(i8 *val, usize len)
{
    Nst_StrObj obj;
    obj.value = val;
    obj.len = len;
    obj.hash = -1;
    obj.type = Nst_t.Str;
    obj.ref_count = 1;
    return obj;
}

Nst_TypeObj *Nst_type_new(const i8 *val)
{
    Nst_TypeObj *str = Nst_obj_alloc(
        Nst_StrObj,
        Nst_t.Type,
        _Nst_string_destroy);
    if (str == NULL)
        return NULL;

    str->len = strlen(val);
    str->value = (i8 *)val;

    return str;
}

Nst_Obj *_Nst_string_copy(Nst_StrObj *src)
{
    i8 *buffer = Nst_malloc_c(src->len + 1, i8);
    if (buffer == NULL)
        return NULL;

    strcpy(buffer, src->value);

    return Nst_string_new_allocated(buffer, src->len);
}

static bool is_unicode_escape(u8 *str)
{
    u32 ch = Nst_ext_utf8_to_utf32(str);
    if (ch >= 0x80 && ch <= 0x9f)
        return !Nst_is_valid_cp(ch) || Nst_is_non_character(ch);
    return false;
}

Nst_Obj *_Nst_string_repr(Nst_StrObj *src)
{
    const i8 *hex_chars = "0123456789abcdef";
    u8 *orig = (u8 *)src->value;
    usize l = src->len;
    usize new_size = 2;
    i32 double_quotes_count = 0;
    i32 single_quotes_count = 0;
    bool using_doub = false;

    for (usize i = 0; i < l; i++) {
        switch (orig[i]) {
        case '\\':
        case '\a':
        case '\b':
        case '\x1b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v': new_size += 2; break;
        case '\'': single_quotes_count += 1; break;
        case '"': double_quotes_count += 1; break;
        default:
            if (isprint(orig[i])) {
                new_size += 1;
                continue;
            } else if (orig[i] <= 0x7f)
                new_size += 4;
            else {
                i32 res = Nst_check_ext_utf8_bytes(orig + i, l - i);
                if (res == -1)
                    new_size += 4;
                else if (is_unicode_escape(orig + i)) {
                    new_size += 6;
                    i++;
                } else {
                    new_size += res;
                    i += res - 1;
                }
            }
        }
    }

    if (single_quotes_count > double_quotes_count) {
        using_doub = true;
        new_size += double_quotes_count * 2;
        new_size += single_quotes_count;
    } else {
        new_size += single_quotes_count * 2;
        new_size += double_quotes_count;
    }

    i8 *new_str = Nst_malloc_c(new_size + 1, i8);
    if (new_str == NULL)
        return NULL;

    *new_str = using_doub ? '"' : '\'';

    usize i = 1;
    for (usize j = 0; j < l; j++) {
        switch (orig[j]) {
        case '\\': new_str[i++] = '\\'; new_str[i++] = '\\';break;
        case '\a': new_str[i++] = '\\'; new_str[i++] = 'a'; break;
        case '\b': new_str[i++] = '\\'; new_str[i++] = 'b'; break;
        case'\x1b':new_str[i++] = '\\'; new_str[i++] = 'e'; break;
        case '\f': new_str[i++] = '\\'; new_str[i++] = 'f'; break;
        case '\n': new_str[i++] = '\\'; new_str[i++] = 'n'; break;
        case '\r': new_str[i++] = '\\'; new_str[i++] = 'r'; break;
        case '\t': new_str[i++] = '\\'; new_str[i++] = 't'; break;
        case '\v': new_str[i++] = '\\'; new_str[i++] = 'v'; break;
        case '\'':
            if (!using_doub)
                new_str[i++] = '\\';
            new_str[i++] = '\'';
            break;
        case '"':
            if (using_doub)
                new_str[i++] = '\\';
            new_str[i++] = '"';
            break;
        default:
            if (isprint((u8)orig[j]))
                new_str[i++] = orig[j];
            else if (orig[j] <= 0b01111111
                     || Nst_check_ext_utf8_bytes(orig + j, l - j) == -1)
            {
                new_str[i++] = '\\';
                new_str[i++] = 'x';
                new_str[i++] = hex_chars[(u8)orig[j] >> 4];
                new_str[i++] = hex_chars[(u8)orig[j] & 0xf];
            } else {
                i32 res = Nst_check_ext_utf8_bytes(orig + j, l - j);
                if (res == 2 && is_unicode_escape(orig + j)) {
                    res = Nst_check_ext_utf8_bytes(orig + j, l - j);
                    j++;
                    new_str[i++] = '\\';
                    new_str[i++] = 'u';
                    new_str[i++] = '0';
                    new_str[i++] = '0';
                    new_str[i++] = hex_chars[res >> 4];
                    new_str[i++] = hex_chars[res & 0xf];
                    continue;
                }

                for (; res > 0; res--)
                    new_str[i++] = orig[j++];
                j--;
            }
        }
    }

    new_str[new_size - 1] = using_doub ? '"' : '\'';
    new_str[new_size] = 0;

    return Nst_string_new_allocated(new_str, new_size);
}

Nst_Obj *_Nst_string_get(Nst_StrObj *str, i64 idx)
{
    if (idx < 0)
        idx += str->len;

    if (idx < 0 || idx >= (i64)str->len) {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Str"),
            idx,
            str->len));
        return NULL;
    }

    i8 *ch = Nst_malloc_c(3, i8);
    if (ch == NULL)
        return NULL;

    u8 byte = str->value[idx];

    if (byte <= 0x7f) {
        ch[0] = str->value[idx];
        ch[1] = 0;
        return Nst_string_new_allocated(ch, 1);
    }

    ch[0] = 0b11000000 | (byte >> 6);
    ch[1] = 0b10000000 | (byte & 0x3f);
    ch[2] = 0;
    return Nst_string_new_allocated(ch, 2);
}

void _Nst_string_destroy(Nst_StrObj *str)
{
    if (str == NULL)
        return;
    if (Nst_STR_IS_ALLOC(str))
        Nst_free(str->value);
}

Nst_Obj *Nst_string_parse_int(Nst_StrObj *str, i32 base)
{
    i8 *s = str->value;
    i8 *end = s + str->len;
    i8 ch;
    i32 ch_val;
    i32 sign = 1;
    i64 num = 0;
    i64 cut_off = 0;
    i64 cut_lim = 0;

    if ((base < 2 || base > 36) && base != 0) {
        Nst_set_value_error_c(_Nst_EM_BAD_INT_BASE);
        return NULL;
    }
    ERR_IF_END(s, end, RETURN_INT_ERR);

    ch = *s;
    while (IS_WHITESPACE(ch))
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
            Nst_set_memory_error_c(_Nst_EM_INT_TOO_BIG);
            return NULL;
        }
        num *= base;
        num += ch_val;
        ch = *++s;
    }

    while (IS_WHITESPACE(ch))
        ch = *++s;

    if (s != end)
        RETURN_INT_ERR;

    return Nst_int_new(num * sign);
}

Nst_Obj *Nst_string_parse_byte(Nst_StrObj *str)
{
    if (str->len == 1)
        return Nst_byte_new(str->value[0]);

    if (str->len == 2 && (u8)str->value[0] > 0x7f) {
        u32 utf32_ch = Nst_ext_utf8_to_utf32((u8 *)str->value);
        if (utf32_ch <= 0xff)
            return Nst_byte_new((u8)utf32_ch);
        else
            RETURN_BYTE_ERR;
    }

    i8* s = str->value;
    i8* end = s + str->len;
    i8 ch = *s;
    i32 num = 0;
    i32 ch_val = 0;
    i32 sign = 1;
    i32 base = 10;

    ERR_IF_END(s, end, RETURN_BYTE_ERR);

    while (IS_WHITESPACE(ch))
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

    while (IS_WHITESPACE(ch))
        ch = *++s;
    if (s != end)
        RETURN_BYTE_ERR;
    return Nst_byte_new(num & 0xff);
}

Nst_Obj *Nst_string_parse_real(Nst_StrObj *str)
{
    // \s*[+-]?\d+\.\d+(?:[eE][+-]?\d+)

    // strtod accepts also things like .5, 1e2, -1., ecc. that I do not
    // so I need to check if the literal is valid first
    i8 *s = str->value;
    i8 *end = s + str->len;
    i8 *start = s;
    usize len = 0;
    i8 ch = *s;
    i8 *buf;
    f64 res;
    bool contains_underscores = false;

    if (s == end)
        RETURN_REAL_ERR;

    while (IS_WHITESPACE(ch)) {
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
        while (IS_WHITESPACE(ch))
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

    while (IS_WHITESPACE(ch))
        ch = *++s;
    if (s != end)
        RETURN_REAL_ERR;
end:
    if (contains_underscores) {
        buf = Nst_malloc_c(len + 1, i8);
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

    res = strtod(buf, NULL);
    if (contains_underscores)
        Nst_free(buf);
    return Nst_real_new(res);
}

i32 Nst_string_compare(Nst_StrObj *str1, Nst_StrObj *str2)
{
    i8 *p1 = str1->value;
    i8 *p2 = str2->value;
    i8 *end1 = p1 + str1->len;
    i8 *end2 = p2 + str2->len;

    while (p1 != end1 && p2 != end2) {
        if (*p1 != *p2)
            return (i32)(*p1 - *p2);
        else {
            ++p1;
            ++p2;
        }
    }

    return (i32)((i64)str1->len - (i64)str2->len);
}

i8 *Nst_string_find(i8 *s1, usize l1, i8 *s2, usize l2)
{
    i8 *end1 = s1 + l1;
    i8 *end2 = s2 + l2;
    i8 *p1 = NULL;
    i8 *p2 = NULL;

    if (l2 > l1)
        return NULL;

    while (s1 != end1) {
        p1 = s1++;
        p2 = s2;

        while (p1 != end1 && p2 != end2 && *p1 == *p2) {
            ++p1;
            ++p2;
        }

        if (p2 == end2)
            return s1 - 1;
    }

    return NULL;
}
