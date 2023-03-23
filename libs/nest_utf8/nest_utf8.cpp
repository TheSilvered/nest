#include <cstring>
#include <cstdlib>
#include "nest_utf8.h"

#define FUNC_COUNT 4

#define SET_INVALID_UTF8 \
    NST_SET_RAW_VALUE_ERROR("the string is not valid UTF-8")

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;
    Nst_OpErr err = { nullptr, nullptr };

    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_valid_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_len_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_at_,   2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(to_iter_,  1);

#if __LINE__ - FUNC_COUNT != 20
#error
#endif

    lib_init_ = err.name == nullptr;
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

NST_FUNC_SIGN(utf8_iter_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(utf8_iter_is_done)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    NST_RETURN_COND((usize)AS_INT(objs[0]) >= STR(objs[1])->len);
}

NST_FUNC_SIGN(utf8_iter_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IntObj *idx = (Nst_IntObj *)objs[0];
    Nst_StrObj *str = (Nst_StrObj *)objs[1];
    usize s_len = str->len;
    Nst_Int val = idx->value;

    if ( (usize)val >= s_len )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            _NST_EM_INDEX_OUT_OF_BOUNDS("Str"),
            "iu",
            val, s_len));
    }
    i32 res = nst_check_utf8_bytes(
        (u8 *)str->value + val,
        s_len - (usize)val);
    if ( res == -1 )
    {
        SET_INVALID_UTF8;
        return nullptr;
    }
    i8 *new_s = (i8 *)nst_malloc(res + 1, sizeof(i8), err);
    if ( new_s == nullptr )
    {
        return nullptr;
    }

    memcpy(new_s, str->value + val, res);
    new_s[res] = '\0';
    idx->value += res;

    return nst_string_new(new_s, res, true, err);
}

NST_FUNC_SIGN(is_valid_)
{
    Nst_StrObj *str;
    NST_DEF_EXTRACT("s", &str);
    u8 *s = (u8 *)str->value;
    for ( usize i = 0, n = str->len; i < n; )
    {
        i32 res = nst_check_utf8_bytes(s + i, n - i);
        if ( res == -1 )
        {
            NST_RETURN_FALSE;
        }
        i += res;
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(get_len_)
{
    Nst_StrObj *str;
    NST_DEF_EXTRACT("s", &str);

    u8 *s = (u8 *)str->value;
    usize len = 0;

    for ( usize i = 0, n = str->len; i < n; len++ )
    {
        i32 res = nst_check_utf8_bytes(s + i, n - i);
        if ( res == -1 )
        {
            SET_INVALID_UTF8;
            return nullptr;
        }
        i += res;
    }

    return nst_int_new(len, err);
}

NST_FUNC_SIGN(get_at_)
{
    Nst_StrObj *str;
    Nst_Int idx;
    NST_DEF_EXTRACT("si", &str, &idx);

    u8 *s = (u8 *)str->value;
    usize u_len = 0;
    usize s_len = str->len;
    Nst_Int curr_idx = 0;
    usize i = 0;
    i32 res;

    for ( ; i < s_len && curr_idx < idx; curr_idx++, u_len++ )
    {
        res = nst_check_utf8_bytes(s + i, s_len - i);
        if ( res == -1 )
        {
            SET_INVALID_UTF8;
            return nullptr;
        }
        i += res;
    }

    if ( curr_idx < idx || i == s_len )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            _NST_EM_INDEX_OUT_OF_BOUNDS("Str (Unicode)"),
            "iu",
            idx, u_len));
        return nullptr;
    }

    res = nst_check_utf8_bytes(s + i, s_len - i);
    if ( res == -1 )
    {
        SET_INVALID_UTF8;
        return nullptr;
    }

    i8 *new_s = (i8 *)nst_malloc(res + 1, sizeof(i8), err);
    if ( new_s == nullptr )
    {
        NST_FAILED_ALLOCATION;
        return nullptr;
    }
    memcpy(new_s, s + i, res);
    new_s[res] = '\0';
    return nst_string_new(new_s, res, true, err);
}

NST_FUNC_SIGN(to_iter_)
{
    Nst_Obj *str;

    NST_DEF_EXTRACT("s", &str);

    // Layout: [idx, str]
    Nst_SeqObj *arr = SEQ(nst_array_new(2, err));
    arr->objs[0] = nst_int_new(0, err);
    arr->objs[1] = nst_inc_ref(str);

    return nst_iter_new(
        FUNC(nst_func_new_c(1, utf8_iter_start, err)),
        FUNC(nst_func_new_c(1, utf8_iter_is_done, err)),
        FUNC(nst_func_new_c(1, utf8_iter_get_val, err)),
        OBJ(arr), err);
}
