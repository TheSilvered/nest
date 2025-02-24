#include <cctype>
#include <cstring>
#include <cstdlib>
#include "nest_sutil.h"

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(lfind_, 4),
    Nst_FUNCDECLR(rfind_, 4),
    Nst_FUNCDECLR(starts_with_, 2),
    Nst_FUNCDECLR(ends_with_, 2),
    Nst_FUNCDECLR(trim_,  1),
    Nst_FUNCDECLR(ltrim_, 1),
    Nst_FUNCDECLR(rtrim_, 1),
    Nst_FUNCDECLR(ljust_, 3),
    Nst_FUNCDECLR(rjust_, 3),
    Nst_FUNCDECLR(cjust_, 3),
    Nst_FUNCDECLR(to_title_, 1),
    Nst_FUNCDECLR(to_upper_, 1),
    Nst_FUNCDECLR(to_lower_, 1),
    Nst_FUNCDECLR(is_title_, 1),
    Nst_FUNCDECLR(is_upper_, 1),
    Nst_FUNCDECLR(is_lower_, 1),
    Nst_FUNCDECLR(is_alpha_, 1),
    Nst_FUNCDECLR(is_digit_, 1),
    Nst_FUNCDECLR(is_decimal_, 1),
    Nst_FUNCDECLR(is_numeric_, 1),
    Nst_FUNCDECLR(is_alnum_, 1),
    Nst_FUNCDECLR(is_space_, 1),
    Nst_FUNCDECLR(is_charset_, 2),
    Nst_FUNCDECLR(is_printable_, 1),
    Nst_FUNCDECLR(is_ascii_, 1),
    Nst_FUNCDECLR(replace_, 3),
    Nst_FUNCDECLR(decode_, 2),
    Nst_FUNCDECLR(encode_, 2),
    Nst_FUNCDECLR(repr_, 1),
    Nst_FUNCDECLR(join_, 2),
    Nst_FUNCDECLR(lsplit_, 3),
    Nst_FUNCDECLR(rsplit_, 3),
    Nst_FUNCDECLR(bin_, 1),
    Nst_FUNCDECLR(oct_, 1),
    Nst_FUNCDECLR(hex_, 2),
    Nst_FUNCDECLR(parse_int_, 2),
    Nst_FUNCDECLR(lremove_, 2),
    Nst_FUNCDECLR(rremove_, 2),
    Nst_FUNCDECLR(fmt_, 2),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    return obj_list_;
}

void get_in_str(Nst_Obj *str, Nst_Obj *start_idx, Nst_Obj *end_idx,
                i8 **out_str, i8 **out_str_end)
{
    usize str1_true_len = Nst_str_char_len(str);
    i64 start = Nst_DEF_VAL(start_idx, Nst_int_i64(start_idx), 0);
    i64 end = Nst_DEF_VAL(end_idx, Nst_int_i64(end_idx), str1_true_len);

    if (start < 0)
        start += str1_true_len;
    if (end < 0)
        end += str1_true_len;

    if (start < 0)
        start = 0;
    if (end > (isize)str1_true_len)
        end = str1_true_len;

    if (start >= end) {
        *out_str = nullptr;
        *out_str_end = 0;
        return;
    }

    isize i;
    for (i = Nst_str_next(str, -1); i >= 0; i = Nst_str_next(str, i)) {
        if (start == 0)
            *out_str = Nst_str_value(str) + i;
        if (end == 0) {
            *out_str_end = Nst_str_value(str) + i;
            return;
        }
        start--;
        end--;
    }

    *out_str_end = Nst_str_value(str) + Nst_str_len(str);
}

Nst_Obj *NstC lfind_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str1;
    Nst_Obj *str2;
    Nst_Obj *start_idx;
    Nst_Obj *end_idx;

    if (!Nst_extract_args(
            "s s ?i ?i",
            arg_num, args,
            &str1, &str2, &start_idx, &end_idx))
    {
        return nullptr;
    }

    i8 *str1_value;
    usize str1_len;

    if (start_idx != Nst_null() || end_idx != Nst_null()) {
        i8 *str1_end;
        get_in_str(str1, start_idx, end_idx, &str1_value, &str1_end);
        if (str1_value == nullptr)
            return Nst_inc_ref(Nst_const()->Int_neg1);
        str1_len = str1_end - str1_value;
    } else {
        str1_value = Nst_str_value(str1);
        str1_len = Nst_str_len(str1);
    }

    if (str1_value == Nst_str_value(str2))
        return Nst_inc_ref(Nst_const()->Int_0);

    i8 *sub = Nst_str_lfind(
        str1_value, str1_len,
        Nst_str_value(str2), Nst_str_len(str2));

    if (sub == nullptr)
        return Nst_inc_ref(Nst_const()->Int_neg1);

    return Nst_int_new(
        i64(Nst_encoding_utf8_char_len(
            (u8 *)Nst_str_value(str1),
            sub - Nst_str_value(str1))));
}

Nst_Obj *NstC rfind_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str1;
    Nst_Obj *str2;
    Nst_Obj *start_idx;
    Nst_Obj *end_idx;

    if (!Nst_extract_args(
            "s s ?i ?i",
            arg_num, args,
            &str1, &str2, &start_idx, &end_idx))
    {
        return nullptr;
    }

    i8 *str1_value;
    usize str1_len;

    if (start_idx != Nst_null() || end_idx != Nst_null()) {
        i8 *str1_end;
        get_in_str(str1, start_idx, end_idx, &str1_value, &str1_end);
        if (str1_value == nullptr)
            return Nst_inc_ref(Nst_const()->Int_neg1);
        str1_len = str1_end - str1_value;
    } else {
        str1_value = Nst_str_value(str1);
        str1_len = Nst_str_len(str1);
    }

    if (str1_value == Nst_str_value(str2))
        return Nst_inc_ref(Nst_const()->Int_0);

    i8 *sub = Nst_str_rfind(
        str1_value, str1_len,
        Nst_str_value(str2), Nst_str_len(str2));

    if (sub == nullptr)
        return Nst_inc_ref(Nst_const()->Int_neg1);

    return Nst_int_new(
        i64(Nst_encoding_utf8_char_len(
            (u8 *)Nst_str_value(str1),
            sub - Nst_str_value(str1))));
}

Nst_Obj *NstC starts_with_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *substr;

    if (!Nst_extract_args("s s", arg_num, args, &str, &substr))
        return nullptr;

    if (Nst_str_len(str) < Nst_str_len(substr))
        return Nst_false_ref();

    i8 *s = Nst_str_value(str);
    i8 *sub = Nst_str_value(substr);
    i8 *sub_end = sub + Nst_str_len(substr);

    while (sub != sub_end) {
        if (*sub++ != *s++)
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC ends_with_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *substr;

    if (!Nst_extract_args("s s", arg_num, args, &str, &substr))
        return nullptr;

    if (Nst_str_len(str) < Nst_str_len(substr))
        return Nst_false_ref();

    i8 *s_end = Nst_str_value(str) + Nst_str_len(str);
    i8 *sub = Nst_str_value(substr);
    i8 *sub_end = sub + Nst_str_len(substr);

    while (sub_end != sub) {
        if (*--sub_end != *--s_end)
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC trim_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    i8 *s_start = Nst_str_value(str);
    i8 *s_end = Nst_str_value(str) + Nst_str_len(str) - 1;
    usize len = Nst_str_len(str);

    while (isspace((u8)*s_start)) {
        s_start++;
        --len;
    }

    if (s_start == s_end + 1)
        return Nst_str_new_c("", 0, false);

    while (isspace((u8)*s_end)) {
        --s_end;
        --len;
    }

    i8 *new_str = Nst_malloc_c(len + 1, i8);
    if (new_str == nullptr)
        return nullptr;

    memcpy(new_str, s_start, len);
    new_str[len] = '\0';

    return Nst_str_new(new_str, len, true);
}

Nst_Obj *NstC ltrim_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    i8 *s_start = Nst_str_value(str);
    usize len = Nst_str_len(str);

    while (isspace((u8)*s_start)) {
        s_start++;
        --len;
    }

    i8 *new_str = Nst_malloc_c(len + 1, i8);
    if (new_str == nullptr)
        return nullptr;
    memcpy(new_str, s_start, len);
    new_str[len] = '\0';

    return Nst_str_new(new_str, len, true);
}

Nst_Obj *NstC rtrim_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    usize len = Nst_str_len(str);
    i8 *s_start = Nst_str_value(str);
    i8 *s_end = s_start + len - 1;

    if (len == 0)
        return Nst_str_new((i8 *)"", 0, false);

    while (s_end + 1 != s_start && isspace((u8)*s_end)) {
        --s_end;
        --len;
    }

    i8 *new_str = Nst_malloc_c(len + 1, i8);
    if (new_str == nullptr)
        return nullptr;
    memcpy(new_str, s_start, len);
    new_str[len] = '\0';

    return Nst_str_new(new_str, len, true);
}

static bool check_just_args(usize arg_num, Nst_Obj **args, Nst_Obj *&str,
                            usize &just_len, i8 *&fill_ch, usize &fill_ch_len)
{
    Nst_Obj *just_ch_obj;
    i64 just_len_signed;
    if (!Nst_extract_args(
            "s i ?s",
            arg_num, args,
            &str, &just_len_signed, &just_ch_obj))
    {
        return false;
    }

    usize fill_ch_true_len;

    if (just_ch_obj == Nst_null()) {
        fill_ch = (i8 *)" ";
        fill_ch_len = 1;
        fill_ch_true_len = 1;
    } else {
        fill_ch = Nst_str_value(just_ch_obj);
        fill_ch_len = Nst_str_len(just_ch_obj);
        fill_ch_true_len = Nst_str_char_len(just_ch_obj);
    }

    if (fill_ch_true_len != 1) {
        Nst_error_setc_value("filling string must be one character long");
        return false;
    }

    just_len = just_len_signed < 0 ? 0 : (usize)just_len_signed;
    return true;
}

Nst_Obj *ljust_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    usize just_len;
    i8 *fill_ch;
    usize fill_ch_len;

    if (!check_just_args(arg_num, args, str, just_len, fill_ch, fill_ch_len))
        return nullptr;

    if (just_len <= Nst_str_char_len(str))
        return Nst_inc_ref(str);

    usize fill_len = usize(just_len - Nst_str_char_len(str));
    usize str_len = Nst_str_len(str);
    usize new_str_len = str_len + fill_ch_len * fill_len;

    i8 *new_str = Nst_malloc_c(new_str_len + 1, i8);
    if (new_str == nullptr)
        return nullptr;

    memcpy(new_str, Nst_str_value(str), str_len);
    Nst_memset(new_str + str_len, fill_ch_len, fill_len, (void *)fill_ch);

    new_str[str_len + fill_ch_len * fill_len] = 0;
    Nst_Obj *new_str_obj = Nst_str_new_len(
        new_str,
        new_str_len,
        just_len,
        true);
    if (new_str_obj == nullptr)
        Nst_free(new_str);
    return new_str_obj;
}

Nst_Obj *rjust_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    usize just_len;
    i8 *fill_ch;
    usize fill_ch_len;

    if (!check_just_args(arg_num, args, str, just_len, fill_ch, fill_ch_len))
        return nullptr;

    if (just_len <= Nst_str_char_len(str))
        return Nst_inc_ref(str);

    usize fill_len = usize(just_len - Nst_str_char_len(str));
    usize str_len = Nst_str_len(str);
    usize new_str_len = str_len + fill_ch_len * fill_len;

    i8 *new_str = Nst_malloc_c(new_str_len + 1, i8);
    if (new_str == nullptr)
        return nullptr;

    Nst_memset(new_str, fill_ch_len, fill_len, (void *)fill_ch);
    memcpy(new_str + fill_len * fill_ch_len, Nst_str_value(str), str_len);
    new_str[str_len + fill_ch_len * fill_len] = 0;
    Nst_Obj *new_str_obj = Nst_str_new_len(
        new_str,
        new_str_len,
        just_len,
        true);
    if (new_str_obj == nullptr)
        Nst_free(new_str);
    return new_str_obj;
}

Nst_Obj *NstC cjust_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    usize just_len;
    i8 *fill_ch;
    usize fill_ch_len;

    if (!check_just_args(arg_num, args, str, just_len, fill_ch, fill_ch_len))
        return nullptr;

    if (just_len <= Nst_str_char_len(str))
        return Nst_inc_ref(str);

    usize fill_len = usize(just_len - Nst_str_char_len(str));
    usize str_len = Nst_str_len(str);
    usize new_str_len = str_len + fill_ch_len * fill_len;

    i8 *new_str = Nst_malloc_c(new_str_len + 1, i8);
    if (new_str == nullptr)
        return nullptr;

    usize left_fill = fill_len / 2;

    Nst_memset(new_str, fill_ch_len, left_fill, (void *)fill_ch);
    memcpy(new_str + left_fill * fill_ch_len, Nst_str_value(str), str_len);
    Nst_memset(
        new_str + left_fill * fill_ch_len + str_len,
        fill_ch_len,
        fill_len - left_fill,
        (void *)fill_ch);

    new_str[str_len + fill_ch_len * fill_len] = 0;
    Nst_Obj *new_str_obj = Nst_str_new_len(
        new_str,
        new_str_len,
        just_len,
        true);
    if (new_str_obj == nullptr)
        Nst_free(new_str);
    return new_str_obj;
}

Nst_Obj *NstC to_title_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    Nst_StrBuilder sb;
    u32 out_cps[Nst_UCD_MAX_CASE_EXPANSION] = { 0 };
    bool new_word = true;

    if (!Nst_sb_init(&sb, Nst_str_len(str)))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        bool is_ws = Nst_unicode_is_whitespace(cp);
        if (is_ws)
            new_word = true;

        Nst_UnicodeChInfo ch_info = Nst_unicode_get_ch_info(cp);
        usize cp_count = Nst_unicode_expand_case(
            cp,
            new_word ? ch_info.title : ch_info.lower,
            out_cps);
        if (!Nst_sb_push_cps(&sb, out_cps, cp_count)) {
            Nst_sb_destroy(&sb);
            return nullptr;
        }
        if (!is_ws)
            new_word = false;
    }

    return Nst_str_from_sb(&sb);
}

Nst_Obj *NstC to_upper_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    Nst_StrBuilder sb;
    u32 out_cps[Nst_UCD_MAX_CASE_EXPANSION] = { 0 };

    if (!Nst_sb_init(&sb, Nst_str_len(str)))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo ch_info = Nst_unicode_get_ch_info(cp);
        usize cp_count = Nst_unicode_expand_case(cp, ch_info.upper, out_cps);
        if (!Nst_sb_push_cps(&sb, out_cps, cp_count)) {
            Nst_sb_destroy(&sb);
            return nullptr;
        }
    }

    return Nst_str_from_sb(&sb);
}

Nst_Obj *NstC to_lower_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    Nst_StrBuilder sb;
    u32 out_cps[Nst_UCD_MAX_CASE_EXPANSION] = { 0 };

    if (!Nst_sb_init(&sb, Nst_str_len(str)))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo ch_info = Nst_unicode_get_ch_info(cp);
        usize cp_count = Nst_unicode_expand_case(cp, ch_info.lower, out_cps);
        if (!Nst_sb_push_cps(&sb, out_cps, cp_count)) {
            Nst_sb_destroy(&sb);
            return nullptr;
        }
    }

    return Nst_str_from_sb(&sb);
}

Nst_Obj *NstC is_title_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    bool new_word = true;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        if (Nst_unicode_is_whitespace(cp)) {
            new_word = true;
            continue;
        }

        Nst_UnicodeChInfo info = Nst_unicode_get_ch_info(cp);
        if (!(info.flags & Nst_UCD_MASK_ALPHABETIC)
            || !(info.flags & Nst_UCD_MASK_CASED)) {
            new_word = false;
            continue;
        }
        if (new_word && !Nst_unicode_is_titlecase(info))
            return Nst_false_ref();
        else if (!new_word && !(info.flags & Nst_UCD_MASK_LOWERCASE))
            return Nst_false_ref();
        new_word = false;
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_upper_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo info = Nst_unicode_get_ch_info(cp);
        if (info.flags & Nst_UCD_MASK_CASED
            && !(info.flags & Nst_UCD_MASK_UPPERCASE))
        {
            return Nst_false_ref();
        }
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_lower_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo info = Nst_unicode_get_ch_info(cp);
        if (info.flags & Nst_UCD_MASK_CASED
            && !(info.flags & Nst_UCD_MASK_LOWERCASE))
        {
            return Nst_false_ref();
        }
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_alpha_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo info = Nst_unicode_get_ch_info(cp);
        if (!(info.flags & Nst_UCD_MASK_ALPHABETIC))
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_digit_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo info = Nst_unicode_get_ch_info(cp);
        if (!(info.flags & Nst_UCD_MASK_DIGIT))
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_decimal_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo info = Nst_unicode_get_ch_info(cp);
        if (!(info.flags & Nst_UCD_MASK_DECIMAL))
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_numeric_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo info = Nst_unicode_get_ch_info(cp);
        if (!(info.flags & Nst_UCD_MASK_NUMERIC))
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_alnum_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo info = Nst_unicode_get_ch_info(cp);
        if (!(info.flags & Nst_UCD_MASK_ALPHABETIC)
            && !(info.flags & Nst_UCD_MASK_NUMERIC))
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_space_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        if (!Nst_unicode_is_whitespace(cp))
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_charset_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str1;
    Nst_Obj *str2;

    if (!Nst_extract_args("s s", arg_num, args, &str1, &str2))
        return nullptr;

    if (Nst_str_len(str1) == 0)
        return Nst_true_ref();
    else if (Nst_str_len(str2) == 0)
        return Nst_false_ref();

    isize i = -1;
    for (i32 ch1 = Nst_str_next_utf32(str1, &i);
         ch1 != -1;
         ch1 = Nst_str_next_utf32(str1, &i))
    {
        isize j = -1;
        bool found_ch = false;
        for (i32 ch2 = Nst_str_next_utf32(str2, &j);
             ch2 != -1;
             ch2 = Nst_str_next_utf32(str2, &j))
        {
            if (ch1 == ch2) {
                found_ch = true;
                break;
            }
        }
        if (!found_ch)
            return Nst_false_ref();
    }
    return Nst_true_ref();
}

Nst_Obj *NstC is_printable_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    isize idx = -1;
    for (i32 cp = Nst_str_next_utf32(str, &idx);
         idx != -1;
         cp = Nst_str_next_utf32(str, &idx))
    {
        Nst_UnicodeChInfo info = Nst_unicode_get_ch_info(cp);
        if (!(info.flags & Nst_UCD_MASK_PRINTABLE))
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC is_ascii_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;

    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    i8 *s = Nst_str_value(str);
    i8 *end = s + Nst_str_len(str);

    while (s != end) {
        if ((u8)*s++ > 0x7f)
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC replace_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *str_from;
    Nst_Obj *str_to;

    if (!Nst_extract_args("s s ?s", arg_num, args, &str, &str_from, &str_to))
        return nullptr;

    usize s_len = Nst_str_len(str);
    usize s_from_len = Nst_str_len(str_from);
    i8 *s = Nst_str_value(str);
    i8 *s_from = Nst_str_value(str_from);
    usize s_to_len = Nst_DEF_VAL(str_to, Nst_str_len(str_to), 0);
    i8 *s_to = Nst_DEF_VAL(str_to, Nst_str_value(str_to), (i8 *)"");

    i8 *sub = nullptr;
    usize new_str_len = 0;
    i32 count = 0;

    while (true) {
        sub = Nst_str_lfind(s, s_len, s_from, s_from_len);
        if (sub == nullptr)
            break;

        s_len -= sub + s_from_len - s;
        s = sub + s_from_len;
        count++;
    }

    if (count == 0)
        return Nst_inc_ref(str);

    s_len = Nst_str_len(str);
    s = Nst_str_value(str);
    i8 *new_str = Nst_malloc_c(
        s_len - s_from_len * count + s_to_len * count + 1,
        i8);
    if (new_str == nullptr)
        return nullptr;

    // Copy replacing the occurrences
    while (true) {
        sub = Nst_str_lfind(s, s_len, s_from, s_from_len);
        if (sub == nullptr)
            break;

        memcpy(new_str + new_str_len, s, sub - s);
        new_str_len += sub - s;
        memcpy(new_str + new_str_len, s_to, s_to_len);
        new_str_len += s_to_len;
        s_len -= sub - s + s_from_len;
        s = sub + s_from_len;
    }

    // Copy the remaining part
    memcpy(new_str + new_str_len, s, s_len);
    new_str_len += s_len;

    new_str[new_str_len] = 0;
    return Nst_str_new_allocated(new_str, new_str_len);
}

Nst_Obj *NstC decode_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *encoding_obj;

    if (!Nst_extract_args("A.B ?s", arg_num, args, &seq, &encoding_obj))
        return nullptr;

    Nst_EncodingID cpid = Nst_DEF_VAL(
        encoding_obj,
        Nst_encoding_from_name(Nst_str_value(encoding_obj)),
        Nst_EID_EXT_UTF8);
    if (cpid == Nst_EID_UNKNOWN) {
        Nst_error_set_value(
            Nst_sprintf(
                "invalid encoding '%.100s'",
                Nst_str_value(encoding_obj)));
        return nullptr;
    }
    cpid = Nst_encoding_to_single_byte(cpid);

    Nst_Encoding *encoding = Nst_encoding(cpid);

    usize len = Nst_seq_len(seq);
    i8 *byte_array = Nst_malloc_c(len + 1, i8);
    if (byte_array == nullptr)
        return nullptr;
    Nst_Obj **objs = _Nst_seq_objs(seq);

    for (usize i = 0; i < len; i++)
        byte_array[i] = Nst_byte_u8(objs[i]);

    byte_array[len] = 0;

    i8 *str;
    usize str_len;
    bool result = Nst_encoding_translate(
        encoding, Nst_encoding(Nst_EID_EXT_UTF8),
        byte_array, len,
        (void **)&str, &str_len);

    Nst_free(byte_array);
    if (!result)
        return nullptr;

    return Nst_str_new(str, str_len, true);
}

Nst_Obj *NstC encode_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *encoding_obj;

    if (!Nst_extract_args("s ?s", arg_num, args, &str, &encoding_obj))
        return nullptr;

    Nst_EncodingID cpid = Nst_DEF_VAL(
        encoding_obj,
        Nst_encoding_from_name(Nst_str_value(encoding_obj)),
        Nst_EID_UTF8);
    if (cpid == Nst_EID_UNKNOWN) {
        Nst_error_set_value(
            Nst_sprintf(
                "invalid encoding '%.100s'",
                Nst_str_value(encoding_obj)));
        return nullptr;
    }
    cpid = Nst_encoding_to_single_byte(cpid);

    Nst_Encoding *encoding = Nst_encoding(cpid);

    u8 *byte_array;
    usize array_len;
    bool result = Nst_encoding_translate(
        Nst_encoding(Nst_EID_EXT_UTF8), encoding,
        Nst_str_value(str), Nst_str_len(str),
        (void **)&byte_array, &array_len);

    if (!result)
        return nullptr;

    Nst_Obj *new_arr = Nst_array_new(array_len);

    for (usize i = 0; i < array_len; i++)
        Nst_seq_setnf(new_arr, i, Nst_byte_new(byte_array[i]));
    Nst_free(byte_array);

    return new_arr;
}

Nst_Obj *NstC repr_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *obj;
    if (!Nst_extract_args("o", arg_num, args, &obj))
        return nullptr;
    return _Nst_repr_str_cast(obj);
}

Nst_Obj *NstC join_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *opt_str;

    if (!Nst_extract_args("A ?s", arg_num, args, &seq, &opt_str))
        return nullptr;

    if (Nst_seq_len(seq) == 0)
        return Nst_str_new_c("", 0, false);

    usize str_len;
    i8 *str_val;

    if (opt_str == Nst_null()) {
        str_len = 1;
        str_val = (i8 *)" ";
    } else {
        str_len = Nst_str_len(opt_str);
        str_val = Nst_str_value(opt_str);
    }

    usize len = Nst_seq_len(seq);
    usize tot_len = str_len * (len - 1);
    Nst_Obj **objs = Nst_malloc_c(len, Nst_Obj *);
    if (objs == nullptr)
        return nullptr;

    for (usize i = 0; i < len; i++) {
        objs[i] = Nst_obj_cast(Nst_seq_getnf(seq, i), Nst_type()->Str);
        tot_len += Nst_str_len(objs[i]);
    }

    i8 *new_str = Nst_malloc_c(tot_len + 1, i8);
    if (new_str == nullptr) {
        Nst_free(objs);
        return nullptr;
    }
    usize str_idx = 0;

    for (usize i = 0; i < len; i++) {
        Nst_Obj *curr_str = objs[i];
        memcpy(
            new_str + str_idx,
            Nst_str_value(curr_str),
            Nst_str_len(curr_str));
        str_idx += Nst_str_len(curr_str);
        Nst_dec_ref(objs[i]);
        if (i + 1 == len)
            break;
        memcpy(new_str + str_idx, str_val, str_len);
        str_idx += str_len;
    }
    Nst_free(objs);
    new_str[tot_len] = '\0';
    return Nst_str_new(new_str, tot_len, true);
}

Nst_Obj *NstC lsplit_whitespace(Nst_Obj *str_obj, i64 quantity)
{
    Nst_Obj *vector = Nst_vector_new(0);

    if (quantity == 0) {
        if (!Nst_vector_append(vector, str_obj)) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        return vector;
    }

    i8 *str = Nst_str_value(str_obj);
    usize str_len = Nst_str_len(str_obj);

    while (str_len > 0 && isspace((u8)*str)) {
        str++;
        str_len--;
    }

    if (str_len == 0)
        return vector;

    while (str_len > 0) {
        i8 *sub_idx = str;
        while (sub_idx - str < (isize)str_len) {
            if (!isspace((u8)*sub_idx))
                sub_idx++;
            else
                break;
        }
        if (sub_idx - str == (isize)str_len)
            break;

        usize new_str_len = sub_idx - str;
        i8 *new_str_val = Nst_malloc_c(new_str_len + 1, i8);
        if (new_str_val == nullptr) {
            Nst_dec_ref(vector);
            return nullptr;
        }

        memcpy(new_str_val, str, new_str_len);
        new_str_val[new_str_len] = '\0';
        Nst_Obj *new_str = Nst_str_new(new_str_val, new_str_len, true);
        Nst_vector_append(vector, new_str);
        Nst_dec_ref(new_str);
        str_len -= new_str_len;
        str = sub_idx;

        while (str_len > 0 && isspace((u8)*str)) {
            str++;
            str_len--;
        }

        if (str_len == 0)
            return vector;

        if (quantity > 0)
            quantity--;
        if (quantity == 0)
            break;
    }

    if (str_len != 0) {
        i8 *new_str_val = Nst_malloc_c(str_len + 1, i8);
        if (new_str_val == nullptr) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        memcpy(new_str_val, str, str_len);
        new_str_val[str_len] = '\0';
        Nst_Obj *new_str = Nst_str_new(new_str_val, str_len, true);
        Nst_vector_append(vector, new_str);
        Nst_dec_ref(new_str);
    }

    return vector;
}

Nst_Obj *NstC lsplit_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str_obj;
    Nst_Obj *opt_substr;
    Nst_Obj *quantity_obj;

    if (!Nst_extract_args(
            "s ?s ?i",
            arg_num, args,
            &str_obj, &opt_substr, &quantity_obj))
    {
        return nullptr;
    }

    i64 quantity = Nst_DEF_VAL(quantity_obj, Nst_int_i64(quantity_obj), -1);

    if (opt_substr == Nst_null())
        return lsplit_whitespace(str_obj, quantity);
    if (Nst_str_len(opt_substr) == 0) {
        Nst_error_setc_value("separator must be at least one character");
        return nullptr;
    }
    Nst_Obj *vector = Nst_vector_new(0);

    if (quantity == 0) {
        if (!Nst_vector_append(vector, str_obj)) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        return vector;
    } else if (Nst_str_len(str_obj) == 0)
        return vector;

    i8 *sub = Nst_str_value(opt_substr);
    usize sub_len = Nst_str_len(opt_substr);
    i8 *str = Nst_str_value(str_obj);
    usize str_len = Nst_str_len(str_obj);

    while (str_len > 0) {
        i8 *sub_idx = Nst_str_lfind(str, str_len, sub, sub_len);
        if (sub_idx == nullptr)
            break;

        usize new_str_len = sub_idx - str;
        i8 *new_str_val = Nst_malloc_c(new_str_len + 1, i8);
        if (new_str_val == nullptr) {
            Nst_dec_ref(vector);
            return nullptr;
        }

        memcpy(new_str_val, str, new_str_len);
        new_str_val[new_str_len] = '\0';
        Nst_Obj *new_str = Nst_str_new(new_str_val, new_str_len, true);
        Nst_vector_append(vector, new_str);
        Nst_dec_ref(new_str);
        str_len -= new_str_len + sub_len;
        str = sub_idx + sub_len;

        if (quantity > 0)
            quantity--;
        if (quantity == 0)
            break;
    }

    {
        i8 *new_str_val = Nst_malloc_c(str_len + 1, i8);
        if (new_str_val == nullptr) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        memcpy(new_str_val, str, str_len);
        new_str_val[str_len] = '\0';
        Nst_Obj *new_str = Nst_str_new(new_str_val, str_len, true);
        Nst_vector_append(vector, new_str);
        Nst_dec_ref(new_str);
    }

    return vector;
}

Nst_Obj *reverse_vector(Nst_Obj *vector)
{
    usize vec_len = Nst_seq_len(vector);
    Nst_Obj **objs = _Nst_seq_objs(vector);
    for (usize i = 0, n = vec_len / 2; i < n; i++) {
        Nst_Obj *temp = objs[i];
        objs[i] = objs[vec_len - i - 1];
        objs[vec_len - i - 1] = temp;
    }
    return vector;
}

Nst_Obj *NstC rsplit_whitespace(Nst_Obj *str_obj, i64 quantity)
{
    Nst_Obj *vector = Nst_vector_new(0);

    if (quantity == 0) {
        if (!Nst_vector_append(vector, str_obj)) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        return vector;
    }

    i8 *str = Nst_str_value(str_obj);
    usize str_len = Nst_str_len(str_obj);

    while (str_len > 0 && isspace((u8)*(str + str_len - 1))) {
        str_len--;
    }

    if (str_len == 0)
        return vector;

    while (str_len > 0) {
        i8 *sub_idx = str + str_len - 1;
        while (sub_idx - str >= 0) {
            if (!isspace((u8)*sub_idx))
                sub_idx--;
            else
                break;
        }
        if (sub_idx - str < 0)
            break;
        sub_idx++;

        usize new_str_len = str_len - (sub_idx - str);
        i8 *new_str_val = Nst_malloc_c(new_str_len + 1, i8);
        if (new_str_val == nullptr) {
            Nst_dec_ref(vector);
            return nullptr;
        }

        memcpy(new_str_val, sub_idx, new_str_len);
        new_str_val[new_str_len] = '\0';
        Nst_Obj *new_str = Nst_str_new(new_str_val, new_str_len, true);
        Nst_vector_append(vector, new_str);
        Nst_dec_ref(new_str);
        str_len -= new_str_len;

        sub_idx--;
        while (str_len > 0 && isspace((u8)*sub_idx)) {
            sub_idx--;
            str_len--;
        }

        if (str_len == 0)
            return reverse_vector(vector);

        if (quantity > 0)
            quantity--;
        if (quantity == 0)
            break;
    }

    if (str_len != 0) {
        i8 *new_str_val = Nst_malloc_c(str_len + 1, i8);
        if (new_str_val == nullptr) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        memcpy(new_str_val, str, str_len);
        new_str_val[str_len] = '\0';
        Nst_Obj *new_str = Nst_str_new(new_str_val, str_len, true);
        Nst_vector_append(vector, new_str);
        Nst_dec_ref(new_str);
    }

    return reverse_vector(vector);
}

Nst_Obj *NstC rsplit_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str_obj;
    Nst_Obj *opt_substr;
    Nst_Obj *quantity_obj;

    if (!Nst_extract_args(
        "s ?s ?i",
        arg_num, args,
        &str_obj, &opt_substr, &quantity_obj))
    {
        return nullptr;
    }

    i64 quantity = Nst_DEF_VAL(quantity_obj, Nst_int_i64(quantity_obj), -1);

    if (opt_substr == Nst_null())
        return rsplit_whitespace(str_obj, quantity);
    if (Nst_str_len(opt_substr) == 0) {
        Nst_error_setc_value("separator must be at least one character");
        return nullptr;
    }
    Nst_Obj *vector = Nst_vector_new(0);

    if (quantity == 0) {
        if (!Nst_vector_append(vector, str_obj)) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        return vector;
    } else if (Nst_str_len(str_obj) == 0)
        return vector;

    i8 *sub = Nst_str_value(opt_substr);
    usize sub_len = Nst_str_len(opt_substr);
    i8 *str = Nst_str_value(str_obj);
    usize str_len = Nst_str_len(str_obj);

    while (str_len > 0) {
        i8 *sub_idx = Nst_str_rfind(str, str_len, sub, sub_len);
        if (sub_idx == nullptr)
            break;

        usize new_str_len = str_len - (sub_idx - str) - sub_len;
        i8 *new_str_val = Nst_malloc_c(new_str_len + 1, i8);
        if (new_str_val == nullptr) {
            Nst_dec_ref(vector);
            return nullptr;
        }

        memcpy(new_str_val, sub_idx + sub_len, new_str_len);
        new_str_val[new_str_len] = '\0';
        Nst_Obj *new_str = Nst_str_new(new_str_val, new_str_len, true);
        Nst_vector_append(vector, new_str);
        Nst_dec_ref(new_str);
        str_len -= new_str_len + sub_len;

        if (quantity > 0)
            quantity--;
        if (quantity == 0)
            break;
    }

    {
        i8 *new_str_val = Nst_malloc_c(str_len + 1, i8);
        if (new_str_val == nullptr) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        memcpy(new_str_val, str, str_len);
        new_str_val[str_len] = '\0';
        Nst_Obj *new_str = Nst_str_new(new_str_val, str_len, true);
        Nst_vector_append(vector, new_str);
        Nst_dec_ref(new_str);
    }

    return reverse_vector(vector);
}

Nst_Obj *NstC bin_(usize arg_num, Nst_Obj **args)
{
    i64 n;
    if (!Nst_extract_args("l", arg_num, args, &n))
        return nullptr;

    usize str_len;
    i8 *str = Nst_fmt("{L:ub}", 6, &str_len, n);
    if (str == nullptr)
        return nullptr;

    return Nst_str_new_allocated(str, str_len);
}

Nst_Obj *NstC oct_(usize arg_num, Nst_Obj **args)
{
    i64 n;
    if (!Nst_extract_args("l", arg_num, args, &n))
        return nullptr;

    usize str_len;
    i8 *str = Nst_fmt("{L:uo}", 6, &str_len, n);
    if (str == nullptr)
        return nullptr;

    return Nst_str_new_allocated(str, str_len);
}

Nst_Obj *NstC hex_(usize arg_num, Nst_Obj **args)
{
    i64 n;
    bool upper;
    if (!Nst_extract_args("l y", arg_num, args, &n, &upper))
        return nullptr;

    usize str_len;
    const i8 *fmt = upper ? "{L:uX}" : "{L:ux}";
    i8 *str = Nst_fmt(fmt, 6, &str_len, n);
    if (str == nullptr)
        return nullptr;

    return Nst_str_new_allocated(str, str_len);
}

Nst_Obj *NstC parse_int_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *base_obj;
    if (!Nst_extract_args("s ?i", arg_num, args, &str, &base_obj))
        return nullptr;

    i64 base = Nst_DEF_VAL(base_obj, Nst_int_i64(base_obj), 0);

    return Nst_str_parse_int(str, i32(base));
}

Nst_Obj *NstC lremove_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *substr;
    if (!Nst_extract_args("s s", arg_num, args, &str, &substr))
        return nullptr;

    if (Nst_str_len(str) < Nst_str_len(substr))
        return Nst_inc_ref(str);

    i8 *str_p = Nst_str_value(str);
    i8 *sub_p = Nst_str_value(substr);
    i8 *sub_p_end = sub_p + Nst_str_len(substr);

    while (sub_p != sub_p_end) {
        if (*str_p++ != *sub_p++)
            return Nst_inc_ref(str);
    }

    usize new_len = Nst_str_len(str) - Nst_str_len(substr);
    i8 *new_str_value = (i8 *)Nst_calloc(1, new_len + 1, str_p);
    if (new_str_value == NULL)
        return NULL;
    return Nst_str_new_allocated(new_str_value, new_len);
}

Nst_Obj *NstC rremove_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *substr;
    if (!Nst_extract_args("s s", arg_num, args, &str, &substr))
        return nullptr;

    if (Nst_str_len(str) < Nst_str_len(substr))
        return Nst_inc_ref(str);

    i8 *str_p = Nst_str_value(str) + Nst_str_len(str) - 1;
    i8 *sub_p = Nst_str_value(substr) + Nst_str_len(substr) - 1;
    i8 *sub_p_end = Nst_str_value(substr) - 1;

    while (sub_p != sub_p_end) {
        if (*str_p-- != *sub_p--)
            return Nst_inc_ref(str);
    }

    usize new_len = Nst_str_len(str) - Nst_str_len(substr);
    i8 *new_str_value = (i8 *)Nst_calloc(1, new_len + 1, Nst_str_value(str));
    if (new_str_value == NULL)
        return NULL;
    new_str_value[new_len] = '\0';
    return Nst_str_new_allocated(new_str_value, new_len);
}

Nst_Obj *NstC fmt_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *format_str;
    Nst_Obj *format_values;
    if (!Nst_extract_args("s A", arg_num, args, &format_str, &format_values))
        return nullptr;

    return Nst_fmt_objs(format_str, format_values);
}
