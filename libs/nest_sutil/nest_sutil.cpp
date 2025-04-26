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
                u8 **out_str, u8 **out_str_end)
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

bool clamp_start_end(Nst_Obj *str, Nst_Obj *start_idx, Nst_Obj *end_idx,
                     i64 &start, i64 &end)
{
    usize str_len = Nst_str_char_len(str);
    start = Nst_DEF_VAL(start_idx, Nst_int_i64(start_idx), 0);
    end = Nst_DEF_VAL(end_idx, Nst_int_i64(end_idx), str_len);

    if (start < 0)
        start += str_len;
    if (end < 0)
        end += str_len;

    if (start < 0)
        start = 0;
    if (end > (isize)str_len)
        end = str_len;

    return start < end;
}

Nst_Obj *NstC lfind_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *sub;
    Nst_Obj *start_idx;
    Nst_Obj *end_idx;

    if (!Nst_extract_args(
            "s s ?i ?i",
            arg_num, args,
            &str, &sub, &start_idx, &end_idx))
    {
        return nullptr;
    }

    i64 idx, end;
    usize sub_len = Nst_str_char_len(sub);
    if (!clamp_start_end(str, start_idx, end_idx, idx, end))
        return Nst_inc_ref(Nst_const()->Int_neg1);
    if (end - idx < (isize)sub_len)
        return Nst_inc_ref(Nst_const()->Int_neg1);
    end -= sub_len;

    for (; idx <= end; idx++) {
        bool found = true;
        for (i64 i = 0; i < (isize)sub_len; i++) {
            if (Nst_str_get(str, idx + i) != Nst_str_get(sub, i)) {
                found = false;
                break;
            }
        }
        if (found)
            break;
    }
    if (idx > end)
        return Nst_inc_ref(Nst_const()->Int_neg1);
    else
        return Nst_int_new(idx);
}

Nst_Obj *NstC rfind_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *sub;
    Nst_Obj *start_idx;
    Nst_Obj *end_idx;

    if (!Nst_extract_args(
        "s s ?i ?i",
        arg_num, args,
        &str, &sub, &start_idx, &end_idx))
    {
        return nullptr;
    }

    i64 start, idx;
    usize sub_len = Nst_str_char_len(sub);
    if (!clamp_start_end(str, start_idx, end_idx, start, idx))
        return Nst_inc_ref(Nst_const()->Int_neg1);
    if (idx - start < (isize)sub_len)
        return Nst_inc_ref(Nst_const()->Int_neg1);
    start += sub_len;

    for (; idx >= start; idx--) {
        bool found = true;
        for (i64 i = 1; i <= (isize)sub_len; i++) {
            if (Nst_str_get(str, idx - i) != Nst_str_get(sub, sub_len - i)) {
                found = false;
                break;
            }
        }
        if (found)
            break;
    }
    if (idx < start)
        return Nst_inc_ref(Nst_const()->Int_neg1);
    else
        return Nst_int_new(idx - sub_len);
}

Nst_Obj *NstC starts_with_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str;
    Nst_Obj *substr;

    if (!Nst_extract_args("s s", arg_num, args, &str, &substr))
        return nullptr;

    if (Nst_str_len(str) < Nst_str_len(substr))
        return Nst_false_ref();

    u8 *s = Nst_str_value(str);
    u8 *sub = Nst_str_value(substr);
    u8 *sub_end = sub + Nst_str_len(substr);

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

    u8 *s_end = Nst_str_value(str) + Nst_str_len(str);
    u8 *sub = Nst_str_value(substr);
    u8 *sub_end = sub + Nst_str_len(substr);

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

    u8 *s_start = Nst_str_value(str);
    u8 *s_end = Nst_str_value(str) + Nst_str_len(str) - 1;
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

    u8 *new_str = Nst_malloc_c(len + 1, u8);
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

    u8 *s_start = Nst_str_value(str);
    usize len = Nst_str_len(str);

    while (isspace((u8)*s_start)) {
        s_start++;
        --len;
    }

    u8 *new_str = Nst_malloc_c(len + 1, u8);
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
    u8 *s_start = Nst_str_value(str);
    u8 *s_end = s_start + len - 1;

    if (len == 0)
        return Nst_str_new((u8 *)"", 0, false);

    while (s_end + 1 != s_start && isspace((u8)*s_end)) {
        --s_end;
        --len;
    }

    u8 *new_str = Nst_malloc_c(len + 1, u8);
    if (new_str == nullptr)
        return nullptr;
    memcpy(new_str, s_start, len);
    new_str[len] = '\0';

    return Nst_str_new(new_str, len, true);
}

static bool check_just_args(usize arg_num, Nst_Obj **args, Nst_Obj *&str,
                            usize &just_len, u8 *&fill_ch, usize &fill_ch_len)
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
        fill_ch = (u8 *)" ";
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
    u8 *fill_ch;
    usize fill_ch_len;

    if (!check_just_args(arg_num, args, str, just_len, fill_ch, fill_ch_len))
        return nullptr;

    if (just_len <= Nst_str_char_len(str))
        return Nst_inc_ref(str);

    usize fill_len = usize(just_len - Nst_str_char_len(str));
    usize str_len = Nst_str_len(str);
    usize new_str_len = str_len + fill_ch_len * fill_len;

    u8 *new_str = Nst_malloc_c(new_str_len + 1, u8);
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
    u8 *fill_ch;
    usize fill_ch_len;

    if (!check_just_args(arg_num, args, str, just_len, fill_ch, fill_ch_len))
        return nullptr;

    if (just_len <= Nst_str_char_len(str))
        return Nst_inc_ref(str);

    usize fill_len = usize(just_len - Nst_str_char_len(str));
    usize str_len = Nst_str_len(str);
    usize new_str_len = str_len + fill_ch_len * fill_len;

    u8 *new_str = Nst_malloc_c(new_str_len + 1, u8);
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
    u8 *fill_ch;
    usize fill_ch_len;

    if (!check_just_args(arg_num, args, str, just_len, fill_ch, fill_ch_len))
        return nullptr;

    if (just_len <= Nst_str_char_len(str))
        return Nst_inc_ref(str);

    usize fill_len = usize(just_len - Nst_str_char_len(str));
    usize str_len = Nst_str_len(str);
    usize new_str_len = str_len + fill_ch_len * fill_len;

    u8 *new_str = Nst_malloc_c(new_str_len + 1, u8);
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

    u8 *s = Nst_str_value(str);
    u8 *end = s + Nst_str_len(str);

    while (s != end) {
        if ((u8)*s++ > 0x7f)
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC replace_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str_obj;
    Nst_Obj *from_obj;
    Nst_Obj *to_obj;

    if (!Nst_extract_args("s s ?s", arg_num, args, &str_obj, &from_obj, &to_obj))
        return nullptr;

    if (Nst_str_char_len(from_obj) == 0)
        return Nst_inc_ref(str_obj);

    Nst_StrView str = Nst_sv_from_str(str_obj);
    Nst_StrView from = Nst_sv_from_str(from_obj);
    Nst_StrView to = Nst_DEF_VAL(
        to_obj,
        Nst_sv_from_str(to_obj),
        Nst_sv_new(NULL, 0));

    Nst_StrView str_cpy = str;
    i32 count = 0;

    // Count the occurrences of the substring

    while (true) {
        str_cpy = Nst_sv_ltok(str_cpy, from);
        if (str_cpy.value != nullptr)
            count++;
        if (str_cpy.len == 0)
            break;
    }

    if (count == 0)
        return Nst_inc_ref(str_obj);

    usize new_str_len = 0;
    u8 *new_str = Nst_malloc_c(
        str.len - from.len * count + to.len * count + 1,
        u8);
    if (new_str == nullptr)
        return nullptr;

    // Copy replacing the occurrences

    str_cpy = str;
    while (true) {
        str_cpy = Nst_sv_ltok(str_cpy, from);
        if (str_cpy.value == nullptr) {
            break;
        }
        usize chunk_len = str_cpy.value - str.value - from.len;

        memcpy(new_str + new_str_len, str.value, chunk_len);
        new_str_len += chunk_len;
        memcpy(new_str + new_str_len, to.value, to.len);
        new_str_len += to.len;
        str = str_cpy;
    }

    // Copy the remaining part
    memcpy(new_str + new_str_len, str.value, str.len);
    new_str_len += str.len;

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
        Nst_encoding_from_name((char *)Nst_str_value(encoding_obj)),
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
    u8 *byte_array = Nst_malloc_c(len + 1, u8);
    if (byte_array == nullptr)
        return nullptr;
    Nst_Obj **objs = Nst_seq_objs(seq);

    for (usize i = 0; i < len; i++)
        byte_array[i] = Nst_byte_u8(objs[i]);

    byte_array[len] = 0;

    u8 *str;
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
        Nst_encoding_from_name((char *)Nst_str_value(encoding_obj)),
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
    return Nst_obj_to_repr_str(obj);
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
    u8 *str_val;

    if (opt_str == Nst_null()) {
        str_len = 1;
        str_val = (u8 *)" ";
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

    u8 *new_str = Nst_malloc_c(tot_len + 1, u8);
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
        if (Nst_str_len(str_obj) != 0 && !Nst_vector_append(vector, str_obj)) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        return vector;
    }

    Nst_StrView str = Nst_sv_from_str(str_obj);

    Nst_StrView piece = Nst_sv_new(nullptr, 0);
    do {
        if (quantity != 0) {
            u32 ch;
            isize i = Nst_sv_next(str, -1, &ch);
            bool reached_separator = false;
            for (; i != -1; i = Nst_sv_next(str, i, &ch)) {
                bool is_whitespace = Nst_unicode_is_whitespace(ch);
                if (reached_separator && !is_whitespace) {
                    str.value += i;
                    str.len -= i;
                    break;
                } else if (!reached_separator && is_whitespace) {
                    piece.value = str.value;
                    piece.len = i;
                    reached_separator = true;
                }
            }
            if (i == -1 && reached_separator)
                str.len = 0;
            else if (i == -1)
                piece = str;
        } else
            piece = str;

        if (piece.len == 0)
            continue;
        if (quantity > 0)
            quantity--;

        Nst_Obj *new_str = Nst_str_from_sv(piece);
        if (new_str == nullptr || !Nst_vector_append(vector, new_str)) {
            Nst_ndec_ref(new_str);
            Nst_dec_ref(vector);
            return nullptr;
        }
        Nst_dec_ref(new_str);
    } while (str.value != piece.value);

    return vector;
}

Nst_Obj *NstC lsplit_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str_obj;
    Nst_Obj *sep_obj;
    Nst_Obj *quantity_obj;

    if (!Nst_extract_args(
            "s ?s ?i",
            arg_num, args,
            &str_obj, &sep_obj, &quantity_obj))
    {
        return nullptr;
    }

    i64 quantity = Nst_DEF_VAL(quantity_obj, Nst_int_i64(quantity_obj), -1);

    if (sep_obj == Nst_null())
        return lsplit_whitespace(str_obj, quantity);
    if (Nst_str_len(sep_obj) == 0) {
        Nst_error_setc_value("separator must be at least one character");
        return nullptr;
    }
    Nst_Obj *vector = Nst_vector_new(0);

    // Avoid creating a new string if no splittig can occur

    if (quantity == 0 || Nst_str_len(str_obj) == 0) {
        if (!Nst_vector_append(vector, str_obj)) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        return vector;
    }

    Nst_StrView str = Nst_sv_from_str(str_obj);
    Nst_StrView sep = Nst_sv_from_str(sep_obj);

    Nst_StrView piece;
    do {
        if (quantity != 0) {
            if (quantity > 0)
                quantity--;
            Nst_StrView match = Nst_sv_ltok(str, sep);
            if (match.value == nullptr)
                piece = str;
            else {
                piece.value = str.value;
                piece.len = match.value - str.value - sep.len;
                str = match;
            }
        } else
            piece = str;

        Nst_Obj *new_str = Nst_str_from_sv(piece);
        if (new_str == nullptr || !Nst_vector_append(vector, new_str)) {
            Nst_ndec_ref(new_str);
            Nst_dec_ref(vector);
            return nullptr;
        }
        Nst_dec_ref(new_str);
    } while (str.value != piece.value);

    return vector;
}

Nst_Obj *reverse_vector(Nst_Obj *vector)
{
    usize vec_len = Nst_seq_len(vector);
    Nst_Obj **objs = Nst_seq_objs(vector);
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
        if (Nst_str_len(str_obj) != 0 && !Nst_vector_append(vector, str_obj)) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        return vector;
    }

    Nst_StrView str = Nst_sv_from_str(str_obj);

    Nst_StrView piece = Nst_sv_new(nullptr, 0);
    do {
        if (quantity != 0) {
            u32 ch;
            isize prev_i = -1;
            isize i = Nst_sv_prev(str, -1, &ch);
            bool reached_separator = false;
            for (; i != -1; i = Nst_sv_prev(str, i, &ch)) {
                bool is_whitespace = Nst_unicode_is_whitespace(ch);
                if (reached_separator && !is_whitespace) {
                    str.len = prev_i;
                    break;
                } else if (!reached_separator && is_whitespace) {
                    if (prev_i != -1) {
                        piece.value = str.value + prev_i;
                        piece.len = str.len - prev_i;
                    } else
                        piece = Nst_sv_new(NULL, 0);
                    reached_separator = true;
                }
                prev_i = i;
            }
            if (i == -1 && reached_separator)
                str.len = 0;
            else if (i == -1)
                piece = str;
        } else
            piece = str;

        if (piece.len == 0)
            continue;

        if (quantity > 0)
            quantity--;

        Nst_Obj *new_str = Nst_str_from_sv(piece);
        if (new_str == nullptr || !Nst_vector_append(vector, new_str)) {
            Nst_ndec_ref(new_str);
            Nst_dec_ref(vector);
            return nullptr;
        }
        Nst_dec_ref(new_str);
    } while (str.value != piece.value);

    return reverse_vector(vector);
}

Nst_Obj *NstC rsplit_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *str_obj;
    Nst_Obj *sep_obj;
    Nst_Obj *quantity_obj;

    if (!Nst_extract_args(
        "s ?s ?i",
        arg_num, args,
        &str_obj, &sep_obj, &quantity_obj))
    {
        return nullptr;
    }

    i64 quantity = Nst_DEF_VAL(quantity_obj, Nst_int_i64(quantity_obj), -1);

    if (sep_obj == Nst_null())
        return rsplit_whitespace(str_obj, quantity);
    if (Nst_str_len(sep_obj) == 0) {
        Nst_error_setc_value("separator must be at least one character");
        return nullptr;
    }
    Nst_Obj *vector = Nst_vector_new(0);

    if (quantity == 0 || Nst_str_len(str_obj) == 0) {
        if (!Nst_vector_append(vector, str_obj)) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        return vector;
    }

    Nst_StrView str = Nst_sv_from_str(str_obj);
    Nst_StrView sep = Nst_sv_from_str(sep_obj);

    Nst_StrView piece;
    do {
        if (quantity != 0) {
            if (quantity > 0)
                quantity--;
            Nst_StrView match = Nst_sv_rtok(str, sep);
            if (match.value == nullptr)
                piece = str;
            else {
                piece = match;
                str.len -= match.len + sep.len;
            }
        } else
            piece = str;

        Nst_Obj *new_str = Nst_str_from_sv(piece);
        if (new_str == nullptr || !Nst_vector_append(vector, new_str)) {
            Nst_ndec_ref(new_str);
            Nst_dec_ref(vector);
            return nullptr;
        }
        Nst_dec_ref(new_str);
    } while (str.value != piece.value);

    return reverse_vector(vector);
}

Nst_Obj *NstC bin_(usize arg_num, Nst_Obj **args)
{
    i64 n;
    if (!Nst_extract_args("l", arg_num, args, &n))
        return nullptr;

    usize str_len;
    u8 *str = Nst_fmt("{L:ub}", 6, &str_len, n);
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
    u8 *str = Nst_fmt("{L:uo}", 6, &str_len, n);
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
    const char *fmt = upper ? "{L:uX}" : "{L:ux}";
    u8 *str = Nst_fmt(fmt, 6, &str_len, n);
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

    u8 *str_p = Nst_str_value(str);
    u8 *sub_p = Nst_str_value(substr);
    u8 *sub_p_end = sub_p + Nst_str_len(substr);

    while (sub_p != sub_p_end) {
        if (*str_p++ != *sub_p++)
            return Nst_inc_ref(str);
    }

    usize new_len = Nst_str_len(str) - Nst_str_len(substr);
    u8 *new_str_value = (u8 *)Nst_calloc(1, new_len + 1, str_p);
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

    u8 *str_p = Nst_str_value(str) + Nst_str_len(str) - 1;
    u8 *sub_p = Nst_str_value(substr) + Nst_str_len(substr) - 1;
    u8 *sub_p_end = Nst_str_value(substr) - 1;

    while (sub_p != sub_p_end) {
        if (*str_p-- != *sub_p--)
            return Nst_inc_ref(str);
    }

    usize new_len = Nst_str_len(str) - Nst_str_len(substr);
    u8 *new_str_value = (u8 *)Nst_calloc(1, new_len + 1, Nst_str_value(str));
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

    return Nst_fmt_objs(
        format_str,
        Nst_seq_objs(format_values),
        Nst_seq_len(format_values));
}
