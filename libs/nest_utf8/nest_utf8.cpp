#include "nest_utf8.h"

#define FUNC_COUNT 4

#define SET_INVALID_UTF8 \
    NST_SET_RAW_VALUE_ERROR("the string is not valid UTF-8")

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
    {
        return false;
    }

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_valid_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_len_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_at_,   2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(to_iter_,  1);

#if __LINE__ - FUNC_COUNT != 21
#error
#endif

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
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
    NST_RETURN_COND((size_t)AS_INT(objs[0]) >= STR(objs[1])->len);
}

NST_FUNC_SIGN(utf8_iter_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IntObj *idx = (Nst_IntObj *)objs[0];
    Nst_StrObj *str = (Nst_StrObj *)objs[1];
    size_t s_len = str->len;
    Nst_Int val = idx->value;

    if ( (size_t)val >= s_len )
    {
        NST_SET_VALUE_ERROR(_nst_format_error(
            _NST_EM_INDEX_OUT_OF_BOUNDS("Str"),
            "iu",
            val, s_len));
    }
    int res = nst_check_utf8_bytes(
        (unsigned char *)str->value + val,
        s_len - (size_t)val);
    if ( res == -1 )
    {
        SET_INVALID_UTF8;
        return nullptr;
    }
    char *new_s = new char[res + 1];
    memcpy(new_s, str->value + val, res);
    new_s[res] = '\0';
    idx->value += res;

    return nst_new_string(new_s, res, true);
}

NST_FUNC_SIGN(is_valid_)
{
    Nst_StrObj *str;
    NST_DEF_EXTRACT("s", &str);
    unsigned char *s = (unsigned char *)str->value;
    for ( size_t i = 0, n = str->len; i < n; )
    {
        int res = nst_check_utf8_bytes(s + i, n - i);
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

    unsigned char *s = (unsigned char *)str->value;
    size_t len = 0;

    for ( size_t i = 0, n = str->len; i < n; len++ )
    {
        int res = nst_check_utf8_bytes(s + i, n - i);
        if ( res == -1 )
        {
            SET_INVALID_UTF8;
            return nullptr;
        }
        i += res;
    }

    return nst_new_int(len);
}

NST_FUNC_SIGN(get_at_)
{
    Nst_StrObj *str;
    Nst_Int idx;
    NST_DEF_EXTRACT("si", &str, &idx);

    unsigned char *s = (unsigned char *)str->value;
    size_t u_len = 0;
    size_t s_len = str->len;
    Nst_Int curr_idx = 0;
    size_t i = 0;
    int res;

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
        NST_SET_VALUE_ERROR(_nst_format_error(
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

    char *new_s = new char[res + 1];
    memcpy(new_s, s + i, res);
    new_s[res] = '\0';
    return nst_new_string(new_s, res, true);
}

NST_FUNC_SIGN(to_iter_)
{
    Nst_Obj *str;

    NST_DEF_EXTRACT("s", &str);

    // Layout: [idx, str]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = nst_inc_ref(str);

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, utf8_iter_start)),
        FUNC(nst_new_cfunc(1, utf8_iter_is_done)),
        FUNC(nst_new_cfunc(1, utf8_iter_get_val)),
        OBJ(arr));
}
