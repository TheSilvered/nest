#include <cstring>
#include <cstdlib>
#include "nest_codecs.h"

#define FUNC_COUNT 6

#define SET_INVALID_UTF8                                                      \
    Nst_set_value_error_c("the string is not valid UTF-8")

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_len_,     1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_at_,      2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(to_iter_,     1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(from_cp_,     1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(to_cp_,       1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(cp_is_valid_, 1);

#if __LINE__ - FUNC_COUNT != 19
#error
#endif

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

Nst_FUNC_SIGN(utf8_iter_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(utf8_iter_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_RETURN_COND((usize)AS_INT(objs[0]) >= STR(objs[1])->len);
}

Nst_FUNC_SIGN(utf8_iter_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IntObj *idx = (Nst_IntObj *)objs[0];
    Nst_StrObj *str = (Nst_StrObj *)objs[1];
    usize s_len = str->len;
    i64 val = idx->value;

    if ((usize)val >= s_len) {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Str (Unicode)"),
            val, s_len));
    }
    i32 res = Nst_check_utf8_bytes(
        (u8 *)str->value + val,
        s_len - (usize)val);
    if (res == -1) {
        SET_INVALID_UTF8;
        return nullptr;
    }
    i8 *new_s = Nst_malloc_c(res + 1, i8);
    if (new_s == nullptr)
        return nullptr;

    memcpy(new_s, str->value + val, res);
    new_s[res] = '\0';
    idx->value += res;

    return Nst_string_new(new_s, res, true);
}

Nst_FUNC_SIGN(get_len_)
{
    Nst_StrObj *str;
    Nst_DEF_EXTRACT("s", &str);

    u8 *s = (u8 *)str->value;
    usize len = 0;

    for (usize i = 0, n = str->len; i < n; len++) {
        i32 res = Nst_check_utf8_bytes(s + i, n - i);
        if (res == -1) {
            SET_INVALID_UTF8;
            return nullptr;
        }
        i += res;
    }

    return Nst_int_new(len);
}

Nst_FUNC_SIGN(get_at_)
{
    Nst_StrObj *str;
    i64 idx;
    Nst_DEF_EXTRACT("si", &str, &idx);

    u8 *s = (u8 *)str->value;
    usize u_len = 0;
    usize s_len = str->len;
    i64 curr_idx = 0;
    usize i = 0;
    i32 res;

    for (; i < s_len && curr_idx < idx; curr_idx++, u_len++) {
        res = Nst_check_utf8_bytes(s + i, s_len - i);
        if (res == -1) {
            SET_INVALID_UTF8;
            return nullptr;
        }
        i += res;
    }

    if (curr_idx < idx || i == s_len) {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Str (Unicode)"),
            idx, u_len));
        return nullptr;
    }

    res = Nst_check_utf8_bytes(s + i, s_len - i);
    if (res == -1) {
        SET_INVALID_UTF8;
        return nullptr;
    }

    i8 *new_s = Nst_malloc_c(res + 1, i8);
    if (new_s == nullptr)
        return nullptr;

    memcpy(new_s, s + i, res);
    new_s[res] = '\0';
    return Nst_string_new(new_s, res, true);
}

Nst_FUNC_SIGN(to_iter_)
{
    Nst_Obj *str;

    Nst_DEF_EXTRACT("s", &str);

    // Layout: [idx, str]
    Nst_Obj *arr = Nst_array_create_c("iO", 0, str);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, utf8_iter_start)),
        FUNC(Nst_func_new_c(1, utf8_iter_is_done)),
        FUNC(Nst_func_new_c(1, utf8_iter_get_val)),
        arr);
}

Nst_FUNC_SIGN(from_cp_)
{
    i64 cp;
    Nst_DEF_EXTRACT("l", &cp);

    if (cp < 0 || cp > UINT32_MAX) {
        Nst_set_value_error(
            Nst_sprintf("codepoint %lli ouside the allowed range", cp));
        return nullptr;
    }

    if (!Nst_is_valid_cp((u32)cp)) {
        if (cp <= 0xffff)
            Nst_set_value_error(Nst_sprintf("invalid code point U+%04llX", cp));
        else
            Nst_set_value_error(Nst_sprintf("invalid code point U+%06llX", cp));

        return nullptr;
    }

    u8 *str = Nst_malloc_c(5, u8);
    if (str == NULL)
        return nullptr;

    i32 len = Nst_utf8_from_utf32((u32)cp, str);
    return Nst_string_new_allocated((i8 *)str, (usize)len);
}

Nst_FUNC_SIGN(to_cp_)
{
    Nst_StrObj *str;
    Nst_DEF_EXTRACT("s", &str);

    usize str_len = str->len;
    if (str_len > 4) {
        Nst_set_value_error_c("the string must contain only one character");
        return nullptr;
    }

    if (Nst_check_utf8_bytes((u8 *)str->value, str_len) != (i32)str_len) {
        Nst_set_value_error_c("the string must contain only one character");
        return nullptr;
    }

    u32 cp = Nst_utf8_to_utf32((u8 *)str->value);
    return Nst_int_new(cp);
}

Nst_FUNC_SIGN(cp_is_valid_)
{
    i64 cp;
    Nst_DEF_EXTRACT("l", &cp);

    Nst_RETURN_COND(cp < 0 || cp > UINT32_MAX || !Nst_is_valid_cp((u32)cp));
}
