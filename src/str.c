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
#include "str_view.h"

/**
 * @param len: the length in bytes of `value`
 * @param char_len: the length in characters of `value`
 * @param value: the value of the string
 * @param indexable_str: the string in UTF-16 or UTF-32 depending on the
 * characters it contains
 */
NstEXP typedef struct _Nst_StrObj {
    Nst_OBJ_HEAD;
    usize len;
    usize char_len;
    i8 *value;
    u8 *indexable_str;
} Nst_StrObj;

#define STR(ptr) ((Nst_StrObj *)(ptr))

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

    u8 *indexable_str = (u8 *)Nst_malloc(str->char_len, ch_width);
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

Nst_Obj *_Nst_str_new_no_err(const i8 *value)
{
    Nst_StrObj *str = STR(Nst_raw_malloc(sizeof(Nst_StrObj)));
    if (str == NULL)
        return NULL;

#ifdef Nst_DBG_TRACK_OBJ_INIT_POS
    str->init_line = -1;
    str->init_col = -1;
    str->init_path = NULL;
#endif

    str->ref_count = 1;
    str->p_next = NULL;
    str->hash = -1;
    str->flags = 0;
    str->len = strlen(value);
    str->value = (i8 *)value;
    str->indexable_str = NULL;

    str->type = Nst_t.Str;
    Nst_inc_ref(Nst_t.Str);
    return OBJ(str);
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
        Nst_encoding_utf8_char_len((u8 *)val, len),
        allocated);
}

Nst_Obj *Nst_str_new_len(i8 *val, usize len, usize char_len, bool allocated)
{
    Nst_StrObj *str = Nst_obj_alloc(Nst_StrObj, Nst_t.Str);
    if (str == NULL)
        return NULL;

    if (allocated)
        str->flags |= Nst_FLAG_STR_IS_ALLOC;
    str->len = len;
    str->value = val;
    str->char_len = char_len;
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

Nst_Obj *Nst_str_copy(Nst_Obj *src)
{
    Nst_assert(src->type == Nst_t.Str);
    i8 *buffer = Nst_malloc_c(STR(src)->len + 1, i8);
    if (buffer == NULL)
        return NULL;

    memcpy(buffer, STR(src)->value, STR(src)->len);

    Nst_Obj *str = Nst_str_new_len(
        buffer,
        STR(src)->len, STR(src)->char_len,
        true);
    if (str == NULL)
        Nst_free(buffer);
    return str;
}

Nst_Obj *Nst_str_repr(Nst_Obj *src)
{
    Nst_assert(src->type == Nst_t.Str);
    usize repr_len;
    i8 *repr_str = Nst_repr(
        STR(src)->value,
        STR(src)->len,
        &repr_len,
        false, false);
    if (repr_str == NULL)
        return NULL;
    return Nst_str_new_allocated(repr_str, repr_len);
}

Nst_Obj *Nst_str_get(Nst_Obj *str, i64 idx)
{
    Nst_assert(str->type == Nst_t.Str);
    if (idx < 0)
        idx += STR(str)->char_len;

    if (idx < 0 || idx >= (i64)STR(str)->char_len) {
        Nst_set_value_errorf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Str"),
            idx,
            STR(str)->char_len);
        return NULL;
    }

    if (!Nst_HAS_FLAG(str, Nst_FLAG_STR_CAN_INDEX)) {
        if (!create_indexable_str(STR(str)))
            return NULL;
    }

    if (Nst_HAS_FLAG(str, Nst_FLAG_STR_IS_ASCII)) {
        i8 *c_buf = Nst_malloc_c(2, i8);
        if (c_buf == NULL)
            return NULL;
        c_buf[0] = STR(str)->value[idx];
        c_buf[1] = 0;
        return Nst_str_new_allocated(c_buf, 1);
    }

    if (Nst_HAS_FLAG(str, Nst_FLAG_STR_INDEX_16)) {
        i8 *c_buf = Nst_calloc_c(4, i8, NULL);
        if (c_buf == NULL)
            return NULL;
        u32 c = ((u16 *)(STR(str)->indexable_str))[idx];
        Nst_ext_utf8_from_utf32(c, (u8 *)c_buf);
        return Nst_str_new_allocated(c_buf, strlen(c_buf));
    }

    if (Nst_HAS_FLAG(str, Nst_FLAG_STR_INDEX_32)) {
        i8 *c_buf = Nst_calloc_c(5, i8, NULL);
        if (c_buf == NULL)
            return NULL;
        u32 c = ((u32 *)(STR(str)->indexable_str))[idx];
        Nst_ext_utf8_from_utf32(c, (u8 *)c_buf);
        return Nst_str_new_allocated(c_buf, strlen(c_buf));
    }

    Nst_set_value_error_c(_Nst_EM_STR_INDEX_FAILED);
    return NULL;
}

isize Nst_str_next(Nst_Obj *str, isize idx)
{
    Nst_assert(str->type == Nst_t.Str);
    if (STR(str)->len == 0)
        return -1;
    if (idx == -1)
        return 0;
    i32 ch_len = Nst_check_ext_utf8_bytes(
        (u8 *)STR(str)->value + idx,
        STR(str)->len - idx);
    if (idx + ch_len == (isize)STR(str)->len)
        return -1;
    return idx + ch_len;
}

Nst_Obj *Nst_str_next_obj(Nst_Obj *str, isize *idx)
{
    Nst_assert(str->type == Nst_t.Str);
    *idx = Nst_str_next(str, *idx);

    if (*idx == -1)
        return NULL;

    isize idx_val = *idx;
    i32 ch_len = Nst_check_ext_utf8_bytes(
        (u8 *)STR(str)->value + idx_val,
        STR(str)->len - idx_val);
    i8 *ch_buf = Nst_malloc_c(ch_len + 1, i8);
    if (ch_buf == NULL) {
        *idx = Nst_STR_LOOP_ERROR;
        return NULL;
    }

    memcpy(ch_buf, STR(str)->value + idx_val, (usize)ch_len);
    ch_buf[ch_len] = 0;

    Nst_Obj *out_str = Nst_str_new_len(ch_buf, ch_len, 1, true);
    if (out_str == NULL) {
        Nst_free(ch_buf);
        *idx = Nst_STR_LOOP_ERROR;
        return NULL;
    }
    return out_str;
}

i32 Nst_str_next_utf32(Nst_Obj *str, isize *idx)
{
    Nst_assert(str->type == Nst_t.Str);
    *idx = Nst_str_next(str, *idx);

    if (*idx == -1)
        return -1;

    return (i32)Nst_ext_utf8_to_utf32((u8 *)(STR(str)->value + *idx));
}

i32 Nst_str_next_utf8(Nst_Obj *str, isize *idx, i8 *ch_buf)
{
    Nst_assert(str->type == Nst_t.Str);
    *idx = Nst_str_next(str, *idx);

    if (*idx == -1)
        return 0;

    i32 ch_len = Nst_check_ext_utf8_bytes(
        (u8 *)STR(str)->value + *idx,
        STR(str)->len - *idx);
    if (ch_buf == NULL)
        return ch_len;
    memset(ch_buf, 0, 4);
    ch_buf = memcpy(ch_buf, STR(str)->value + *idx, ch_len);
    return ch_len;
}

void _Nst_str_destroy(Nst_Obj *str)
{
    Nst_assert(str->type == Nst_t.Str);
    if (str == NULL)
        return;
    if (str->flags & Nst_FLAG_STR_IS_ALLOC)
        Nst_free(STR(str)->value);
    if (STR(str)->indexable_str != NULL)
        Nst_free(STR(str)->indexable_str);
}

Nst_Obj *Nst_str_parse_int(Nst_Obj *str, i32 base)
{
    Nst_assert(str->type == Nst_t.Str);
    return Nst_sv_parse_int(Nst_sv_from_str(str), base);
}

Nst_Obj *Nst_str_parse_byte(Nst_Obj *str)
{
    Nst_assert(str->type == Nst_t.Str);
    return Nst_sv_parse_byte(Nst_sv_from_str(str));
}

Nst_Obj *Nst_str_parse_real(Nst_Obj *str)
{
    Nst_assert(str->type == Nst_t.Str);
    return Nst_sv_parse_real(Nst_sv_from_str(str));
}

i32 Nst_str_compare(Nst_Obj *str1, Nst_Obj *str2)
{
    Nst_assert(str1->type == Nst_t.Str);
    Nst_assert(str2->type == Nst_t.Str);

    return Nst_sv_compare(Nst_sv_from_str(str1), Nst_sv_from_str(str2));
}

i8 *Nst_str_lfind(i8 *s1, usize l1, i8 *s2, usize l2)
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

i8 *Nst_str_value(Nst_Obj *str)
{
    Nst_assert(str->type == Nst_t.Str);
    return STR(str)->value;
}

usize Nst_str_len(Nst_Obj *str)
{
    Nst_assert(str->type == Nst_t.Str);
    return STR(str)->len;
}

usize Nst_str_char_len(Nst_Obj *str)
{
    Nst_assert(str->type == Nst_t.Str);
    return STR(str)->char_len;
}
