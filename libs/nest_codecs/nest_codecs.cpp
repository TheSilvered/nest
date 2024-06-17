#include <cstring>
#include <cstdlib>
#include "nest_codecs.h"

#define SET_INVALID_UTF8                                                      \
    Nst_set_value_error_c("the string is not valid UTF-8")

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(from_cp_,     1),
    Nst_FUNCDECLR(to_cp_,       1),
    Nst_FUNCDECLR(cp_is_valid_, 1),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    return obj_list_;
}

Nst_Obj *NstC from_cp_(usize arg_num, Nst_Obj **args)
{
    i64 cp;
    if (!Nst_extract_args("l", arg_num, args, &cp))
        return nullptr;

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

Nst_Obj *NstC to_cp_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *str;
    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    if (str->true_len != 1) {
        Nst_set_value_error_c("the string must contain only one character");
        return nullptr;
    }

    u32 cp = Nst_ext_utf8_to_utf32((u8 *)str->value);
    return Nst_int_new(cp);
}

Nst_Obj *NstC cp_is_valid_(usize arg_num, Nst_Obj **args)
{
    i64 cp;
    if (!Nst_extract_args("l", arg_num, args, &cp))
        return nullptr;

    Nst_RETURN_BOOL(cp >= 0 && cp <= UINT32_MAX && Nst_is_valid_cp((u32)cp));
}
