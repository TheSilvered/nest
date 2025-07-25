#include <cstring>
#include <cstdlib>
#include "nest_codecs.h"

#define SET_INVALID_UTF8                                                      \
    Nst_error_setc_value("the string is not valid UTF-8")

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(from_cp_,       1),
    Nst_FUNCDECLR(to_cp_,         1),
    Nst_FUNCDECLR(cp_is_valid_,   1),
    Nst_FUNCDECLR(encoding_info_, 1),
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
        Nst_error_set_value(
            Nst_sprintf("codepoint %#" PRIx64 " ouside the allowed range", cp));
        return nullptr;
    }

    if (!Nst_cp_is_valid((u32)cp)) {
        if (cp <= 0xffff)
            Nst_error_setf_value("invalid code point U+%04" PRIX64, cp);
        else
            Nst_error_setf_value("invalid code point U+%06" PRIX64, cp);

        return nullptr;
    }

    u8 *str = Nst_calloc_c(5, u8, nullptr);
    if (str == nullptr)
        return nullptr;

    i32 len = Nst_utf8_from_utf32((u32)cp, str);
    return Nst_str_new_allocated(str, (usize)len);
}

Nst_Obj *NstC to_cp_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    if (Nst_str_char_len(str) != 1) {
        Nst_error_setc_value("the string must contain only one character");
        return nullptr;
    }

    u32 cp = Nst_ext_utf8_to_utf32((u8 *)Nst_str_value(str));
    return Nst_int_new(cp);
}

Nst_Obj *NstC cp_is_valid_(usize arg_num, Nst_Obj **args)
{
    i64 cp;
    if (!Nst_extract_args("l", arg_num, args, &cp))
        return nullptr;

    Nst_RETURN_BOOL(cp >= 0 && cp <= UINT32_MAX && Nst_cp_is_valid((u32)cp));
}

Nst_Obj *NstC encoding_info_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *name_str;
    if (!Nst_extract_args("s", arg_num, args, &name_str))
        return nullptr;

    Nst_EncodingID cpid = Nst_encoding_from_name(
        (char *)Nst_str_value(name_str));
    if (cpid == Nst_EID_UNKNOWN) {
        Nst_error_setf_value(
            "unknown encoding '%.100s'",
            Nst_str_value(name_str));
        return nullptr;
    }
    Nst_Encoding *cp = Nst_encoding(cpid);
    Nst_Obj *info = Nst_map_new();

    Nst_Obj *mult_max_sz = Nst_int_new(cp->mult_max_sz);
    Nst_Obj *mult_min_sz = Nst_int_new(cp->mult_min_sz);
    Nst_Obj *name = Nst_str_new_c(cp->name);
    Nst_Obj *bom;

    if (cp->bom_size == 0)
        bom = Nst_null_ref();
    else {
        bom = Nst_array_new(cp->bom_size);
        for (usize i = 0, n = cp->bom_size; i < n; i++)
            Nst_seq_setnf(bom, i, Nst_byte_new(cp->bom[i]));
    }

    Nst_map_set_str(info, "name", name);
    Nst_map_set_str(info, "min_len", mult_min_sz);
    Nst_map_set_str(info, "max_len", mult_max_sz);
    Nst_map_set_str(info, "bom", bom);

    Nst_dec_ref(mult_max_sz);
    Nst_dec_ref(mult_min_sz);
    Nst_dec_ref(name);
    Nst_dec_ref(bom);

    return info;
}
