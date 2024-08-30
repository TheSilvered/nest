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
#include "dtoa.h"

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

static i32 get_ch_width(u8 *str, u8 *end)
{
    i32 size = sizeof(u8);
    while (str < end) {
        u8 c = *str;
        if (c >= 0x80) {
            size = sizeof(u16);
            if ((c & 0xf0) == 0xf0)
                return sizeof(u32);
        }
        str++;
    }
    return size;
}

static void fill_indexable_str_utf16(u16 *i_str, u8 *s, u8 *s_end)
{
    while (s < s_end) {
        i32 len = Nst_check_ext_utf8_bytes(s, 4);
        u32 ch = Nst_ext_utf8_to_utf32(s);
        *i_str++ = (u16)ch;
        s += len;
    }
}

static void fill_indexable_str_utf32(u32 *i_str, u8 *s, u8 *s_end)
{
    while (s < s_end) {
        i32 len = Nst_check_ext_utf8_bytes(s, 4);
        u32 ch = Nst_ext_utf8_to_utf32(s);
        *i_str++ = ch;
        s += len;
    }
}

static bool create_indexable_str(Nst_StrObj *str)
{
    u8 *s = (u8 *)str->value;
    u8 *s_end = s + str->len;

    i32 ch_width = get_ch_width(s, s_end);

    if (ch_width == 1) {
        Nst_SET_FLAG(str, Nst_FLAG_STR_IS_ASCII);
        Nst_SET_FLAG(str, Nst_FLAG_STR_CAN_INDEX);
        return true;
    }

    u8 *indexable_str = (u8 *)Nst_malloc(str->true_len, ch_width);
    if (indexable_str == NULL)
        return false;

    if (ch_width == 2) {
        fill_indexable_str_utf16((u16 *)indexable_str, s, s_end);
        Nst_SET_FLAG(str, Nst_FLAG_STR_INDEX_16);
    } else {
        fill_indexable_str_utf32((u32 *)indexable_str, s, s_end);
        Nst_SET_FLAG(str, Nst_FLAG_STR_INDEX_32);
    }

    Nst_SET_FLAG(str, Nst_FLAG_STR_CAN_INDEX);
    str->indexable_str = indexable_str;
    return true;
}

Nst_Obj *Nst_str_new_c_raw(const i8 *val, bool allocated)
{
    return Nst_str_new((i8 *)val, strlen(val), allocated);
}

Nst_Obj *Nst_str_new_c(const i8 *val, usize len, bool allocated)
{
    return Nst_str_new((i8 *)val, len, allocated);
}

Nst_Obj *Nst_str_new(i8 *val, usize len, bool allocated)
{
    return Nst_str_new_len(
        val,
        len,
        Nst_string_utf8_char_len((u8 *)val, len),
        allocated);
}

Nst_Obj *Nst_str_new_len(i8 *val, usize len, usize true_len, bool allocated)
{
    Nst_StrObj *str = Nst_obj_alloc(Nst_StrObj, Nst_t.Str);
    if (str == NULL)
        return NULL;

    if (allocated)
        str->flags |= Nst_FLAG_STR_IS_ALLOC;
    str->len = len;
    str->value = val;
    str->true_len = true_len;
    str->indexable_str = NULL;

    return OBJ(str);
}

Nst_Obj *Nst_str_new_allocated(i8 *val, usize len)
{
    Nst_Obj *str = Nst_str_new(val, len, true);
    if (str == NULL) {
        Nst_free(val);
        return NULL;
    }
    return str;
}

Nst_StrObj Nst_str_temp(i8 *val, usize len)
{
    Nst_StrObj obj;
    obj.type = Nst_t.Str;
    obj.ref_count = 1;
    obj.p_next = NULL;
    obj.hash = -1;
    obj.flags = Nst_FLAG_STR_CAN_INDEX | Nst_FLAG_STR_IS_ASCII;
    obj.value = val;
    obj.len = len;
    obj.true_len = len;
    obj.indexable_str = NULL;
    return obj;
}

Nst_Obj *_Nst_str_copy(Nst_StrObj *src)
{
    i8 *buffer = Nst_malloc_c(src->len + 1, i8);
    if (buffer == NULL)
        return NULL;

    memcpy(buffer, src->value, src->len);

    Nst_Obj *str = Nst_str_new_len(buffer, src->true_len, src->len, true);
    if (str == NULL)
        Nst_free(buffer);
    return str;
}

static i32 get_unicode_escape_len(u8 *str)
{
    u32 ch = Nst_ext_utf8_to_utf32(str);
    if (!Nst_is_valid_cp(ch) || Nst_is_non_character(ch)) {
        return ch <= 0xffff ? 6 : 8;
    }
    return 0;
}

static i32 ch_repr_len(u8 *str, usize i, usize s_len)
{
    if (str[i] < 0x80) {
        switch (str[i]) {
        case '\\':
        case '\a':
        case '\b':
        case '\x1b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v': return 2;
        }

        if (!isprint(str[i]))
            return 4;
        return 1;
    }

    i32 res = Nst_check_ext_utf8_bytes(str + i, s_len - i);

    if (res == -1)
        return 0;

    i32 escape_len = get_unicode_escape_len(str + i);
    if (escape_len)
        return escape_len;
    else
        return res;
}

Nst_Obj *_Nst_str_repr(Nst_StrObj *src)
{
    const i8 *hex_chars = "0123456789abcdef";
    u8 *orig = (u8 *)src->value;
    usize l = src->len;
    usize new_size = 2;
    i32 double_quotes_count = 0;
    i32 single_quotes_count = 0;

    for (usize i = 0; i < l; i++) {
        new_size += ch_repr_len(orig, i, l);
        if (orig[i] == '\'')
            single_quotes_count++;
        else if (orig[i] == '"')
            double_quotes_count++;
    }

    bool using_doub = single_quotes_count > double_quotes_count;

    if (using_doub)
        new_size += double_quotes_count;
    else
        new_size += single_quotes_count;

    i8 *new_str = Nst_malloc_c(new_size + 1, i8);
    if (new_str == NULL)
        return NULL;

    *new_str = using_doub ? '"' : '\'';

    usize i = 1;
    for (usize j = 0; j < l; j++) {
        switch (orig[j]) {
        case '\\': new_str[i++] = '\\'; new_str[i++] = '\\';continue;
        case '\a': new_str[i++] = '\\'; new_str[i++] = 'a'; continue;
        case '\b': new_str[i++] = '\\'; new_str[i++] = 'b'; continue;
        case'\x1b':new_str[i++] = '\\'; new_str[i++] = 'e'; continue;
        case '\f': new_str[i++] = '\\'; new_str[i++] = 'f'; continue;
        case '\n': new_str[i++] = '\\'; new_str[i++] = 'n'; continue;
        case '\r': new_str[i++] = '\\'; new_str[i++] = 'r'; continue;
        case '\t': new_str[i++] = '\\'; new_str[i++] = 't'; continue;
        case '\v': new_str[i++] = '\\'; new_str[i++] = 'v'; continue;
        case '\'':
            if (!using_doub)
                new_str[i++] = '\\';
            new_str[i++] = '\'';
            continue;
        case '"':
            if (using_doub)
                new_str[i++] = '\\';
            new_str[i++] = '"';
            continue;
        }
        if (orig[j] < 0x80) {
            if (isprint(orig[j])) {
                new_str[i++] = orig[j];
                continue;
            }

            new_str[i++] = '\\';
            new_str[i++] = 'x';
            new_str[i++] = hex_chars[(orig[j] & 0xf0) >> 4];
            new_str[i++] = hex_chars[orig[j] & 0x0f];
            continue;
        }

        i32 res = Nst_check_ext_utf8_bytes(orig + j, l - j);
        i32 escape_len = get_unicode_escape_len(orig + j);
        if (escape_len) {
            new_str[i++] = '\\';
            new_str[i++] = escape_len == 8 ? 'U' : 'u';
            i32 cp = Nst_ext_utf8_to_utf32(orig + j);
            for (escape_len -= 3; escape_len >= 0; escape_len--) {
                new_str[i++] = hex_chars[(cp >> (escape_len * 4)) & 0xf];
            }
            j += res - 1;
            continue;
        }

        memcpy(new_str + i, orig + j, res);
        i += res;
        j += res - 1;
    }

    new_str[new_size - 1] = using_doub ? '"' : '\'';
    new_str[new_size] = 0;

    return Nst_str_new_allocated(new_str, new_size);
}

Nst_Obj *_Nst_str_get(Nst_StrObj *str, i64 idx)
{
    if (idx < 0)
        idx += str->true_len;

    if (idx < 0 || idx >= (i64)str->true_len) {
        Nst_set_value_errorf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Str"),
            idx,
            str->true_len);
        return NULL;
    }

    if (!Nst_HAS_FLAG(str, Nst_FLAG_STR_CAN_INDEX)) {
        if (!create_indexable_str(str))
            return NULL;
    }

    if (Nst_HAS_FLAG(str, Nst_FLAG_STR_IS_ASCII)) {
        i8 *c_buf = Nst_malloc_c(2, i8);
        if (c_buf == NULL)
            return NULL;
        c_buf[0] = str->value[idx];
        c_buf[1] = 0;
        return Nst_str_new_allocated(c_buf, 1);
    }

    if (Nst_HAS_FLAG(str, Nst_FLAG_STR_INDEX_16)) {
        i8 *c_buf = Nst_calloc_c(4, i8, NULL);
        if (c_buf == NULL)
            return NULL;
        u32 c = ((u16 *)(str->indexable_str))[idx];
        Nst_ext_utf8_from_utf32(c, (u8 *)c_buf);
        return Nst_str_new_allocated(c_buf, strlen(c_buf));
    }

    if (Nst_HAS_FLAG(str, Nst_FLAG_STR_INDEX_32)) {
        i8 *c_buf = Nst_calloc_c(5, i8, NULL);
        if (c_buf == NULL)
            return NULL;
        u32 c = ((u32 *)(str->indexable_str))[idx];
        Nst_ext_utf8_from_utf32(c, (u8 *)c_buf);
        return Nst_str_new_allocated(c_buf, strlen(c_buf));
    }

    Nst_set_value_error_c(_Nst_EM_STR_INDEX_FAILED);
    return NULL;
}

isize Nst_str_next(Nst_StrObj *str, isize idx)
{
    if (str->len == 0)
        return -1;
    if (idx == -1)
        return 0;
    i32 ch_len = Nst_check_ext_utf8_bytes(
        (u8 *)str->value + idx,
        str->len - idx);
    if (idx + ch_len == (isize)str->len)
        return -1;
    return idx + ch_len;
}

Nst_Obj *Nst_str_next_obj(Nst_StrObj *str, isize *idx)
{
    *idx = Nst_str_next(str, *idx);

    if (*idx == -1)
        return NULL;

    isize idx_val = *idx;
    i32 ch_len = Nst_check_ext_utf8_bytes(
        (u8 *)str->value + idx_val,
        str->len - idx_val);
    i8 *ch_buf = Nst_malloc_c(ch_len + 1, i8);
    if (ch_buf == NULL) {
        *idx = Nst_STR_LOOP_ERROR;
        return NULL;
    }

    memcpy(ch_buf, str->value + idx_val, (usize)ch_len);
    ch_buf[ch_len] = 0;

    Nst_Obj *out_str = Nst_str_new_len(ch_buf, ch_len, 1, true);
    if (out_str == NULL) {
        Nst_free(ch_buf);
        *idx = Nst_STR_LOOP_ERROR;
        return NULL;
    }
    return out_str;
}

i32 Nst_str_next_utf32(Nst_StrObj *str, isize *idx)
{
    *idx = Nst_str_next(str, *idx);

    if (*idx == -1)
        return -1;

    return (i32)Nst_ext_utf8_to_utf32((u8 *)(str->value + *idx));
}

i32 Nst_str_next_utf8(Nst_StrObj *str, isize *idx, i8 *ch_buf)
{
    *idx = Nst_str_next(str, *idx);

    if (*idx == -1)
        return 0;

    i32 ch_len = Nst_check_ext_utf8_bytes(
        (u8 *)str->value + *idx,
        str->len - *idx);
    if (ch_buf == NULL)
        return ch_len;
    memset(ch_buf, 0, 4);
    ch_buf = memcpy(ch_buf, str->value + *idx, ch_len);
    return ch_len;
}

usize _Nst_str_len(Nst_StrObj *str)
{
    return str->true_len;
}

usize _Nst_str_buf_len(Nst_StrObj *str)
{
    return str->len;
}

const i8 *_Nst_str_buf(Nst_StrObj *str)
{
    return (const i8 *)str->value;
}

void _Nst_str_destroy(Nst_StrObj *str)
{
    if (str == NULL)
        return;
    if (Nst_STR_IS_ALLOC(str))
        Nst_free(str->value);
    if (str->indexable_str != NULL)
        Nst_free(str->indexable_str);
}

Nst_Obj *Nst_str_parse_int(Nst_StrObj *str, i32 base)
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
    while (isspace((u8)ch))
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

    while (isspace((u8)ch))
        ch = *++s;

    if (s != end)
        RETURN_INT_ERR;

    return Nst_int_new(num * sign);
}

Nst_Obj *Nst_str_parse_byte(Nst_StrObj *str)
{
    if (str->true_len == 1) {
        u32 utf32_ch = Nst_ext_utf8_to_utf32((u8 *)str->value);
        if (utf32_ch <= 0xff)
            return Nst_byte_new((u8)utf32_ch);
        else
            RETURN_BYTE_ERR;
    }

    i8 *s = str->value;
    i8 *end = s + str->len;
    i8 ch = *s;
    i32 num = 0;
    i32 ch_val = 0;
    i32 sign = 1;
    i32 base = 10;

    ERR_IF_END(s, end, RETURN_BYTE_ERR);

    while (isspace((u8)ch))
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

    while (isspace((u8)ch))
        ch = *++s;
    if (s != end)
        RETURN_BYTE_ERR;
    return Nst_byte_new(num & 0xff);
}

Nst_Obj *Nst_str_parse_real(Nst_StrObj *str)
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

    while (isspace((u8)ch)) {
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
        while (isspace((u8)ch))
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

    while (isspace((u8)ch))
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

    res = Nst_strtod(buf, NULL);
    if (contains_underscores)
        Nst_free(buf);
    return Nst_real_new(res);
}

i32 Nst_str_compare(Nst_StrObj *str1, Nst_StrObj *str2)
{
    u8 *p1 = (u8 *)str1->value;
    u8 *p2 = (u8 *)str2->value;
    u8 *end1 = p1 + str1->len;
    u8 *end2 = p2 + str2->len;

    while (p1 != end1 && p2 != end2) {
        if (*p1 != *p2)
            return *p1 < *p2 ? -1 : 1; // fixed values above 0x7f
        else {
            p1++;
            p2++;
        }
    }

    return (i32)((i64)str1->len - (i64)str2->len);
}

i8 *Nst_str_find(i8 *s1, usize l1, i8 *s2, usize l2)
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
            p1++;
            p2++;
        }

        if (p2 == end2)
            return s1 - 1;
    }

    return NULL;
}

i8 *Nst_str_rfind(i8 *s1, usize l1, i8 *s2, usize l2)
{
    i8 *p = s1 + l1 - l2;

    while (p >= s1) {
        for (usize i = 0; i < l2; i++) {
            if (p[i] != s2[i])
                goto next_cycle;
        }
        return p;

    next_cycle:
        p--;
    }

    return NULL;
}
