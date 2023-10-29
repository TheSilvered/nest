#include <cstring>
#include <cstdlib>
#include "nest_codecs.h"

#define FUNC_COUNT 3

#define SET_INVALID_UTF8                                                      \
    Nst_set_value_error_c("the string is not valid UTF-8")

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;

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

Nst_FUNC_SIGN(from_cp_)
{
    i64 cp;
    Nst_DEF_EXTRACT("l", &cp);

    if (cp < 0 || cp > UINT32_MAX) {
        Nst_set_value_error(
            Nst_sprintf("codepoint %#llx ouside the allowed range", cp));
        return nullptr;
    }

    if (!Nst_is_valid_cp((u32)cp)) {
        if (cp <= 0xffff)
            Nst_set_value_error(Nst_sprintf("invalid code point U+%04llX", cp));
        else
            Nst_set_value_error(Nst_sprintf("invalid code point U+%06llX", cp));

        return nullptr;
    }

    u8 *str = Nst_calloc_c(5, u8, nullptr);
    if (str == nullptr)
        return nullptr;

    i32 len = Nst_utf8_from_utf32((u32)cp, str);
    return Nst_string_new_allocated((i8 *)str, (usize)len);
}

Nst_FUNC_SIGN(to_cp_)
{
    Nst_StrObj *str;
    Nst_DEF_EXTRACT("s", &str);

    if (str->true_len != 1) {
        Nst_set_value_error_c("the string must contain only one character");
        return nullptr;
    }

    u32 cp = Nst_ext_utf8_to_utf32((u8 *)str->value);
    return Nst_int_new(cp);
}

Nst_FUNC_SIGN(cp_is_valid_)
{
    i64 cp;
    Nst_DEF_EXTRACT("l", &cp);

    Nst_RETURN_COND(cp >= 0 && cp <= UINT32_MAX && Nst_is_valid_cp((u32)cp));
}
