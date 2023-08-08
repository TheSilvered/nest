#include <cctype>
#include <cstring>
#include <cstdlib>
#include "nest_sutil.h"

#define FUNC_COUNT 31

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(lfind_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(rfind_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(starts_with_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(ends_with_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(trim_,  1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(ltrim_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(rtrim_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(ljust_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(rjust_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(center_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(to_title_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(to_upper_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(to_lower_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_title_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_upper_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_lower_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_alpha_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_digit_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_alnum_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_charset_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_printable_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(replace_substr_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(bytearray_to_str_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(str_to_bytearray_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(repr_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(join_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(split_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(bin_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(oct_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(hex_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(parse_int_, 2);

#if __LINE__ - FUNC_COUNT != 17
#error
#endif

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

Nst_FUNC_SIGN(lfind_)
{
    Nst_StrObj *str1 = nullptr;
    Nst_StrObj *str2 = nullptr;

    Nst_DEF_EXTRACT("s s", &str1, &str2);

    if (str1 == str2)
        Nst_RETURN_ZERO;

    i8 *sub = Nst_string_find(str1->value, str1->len, str2->value, str2->len);

    if (sub == nullptr)
        return Nst_inc_ref(Nst_const()->Int_neg1);
    else
        return Nst_int_new(sub - str1->value);
}

Nst_FUNC_SIGN(rfind_)
{
    Nst_StrObj *str1;
    Nst_StrObj *str2;

    Nst_DEF_EXTRACT("s s", &str1, &str2);

    if (str1 == str2)
        Nst_RETURN_ZERO;

    i8 *s1 = str1->value + str1->len - 1;
    i8 *s2 = str2->value + str2->len - 1;
    i8 *p1;
    i8 *p2;
    i8 *s1_start = str1->value;
    i8 *s2_start = str2->value;

    while (s1 - s1_start + 1) {
        p1 = s1--;
        p2 = s2;

        while (p1 - s1_start + 1 && p2 - s2_start + 1 && *p1 == *p2) {
            p1--;
            p2--;
        }

        if (p2 - s2_start + 1 == 0)
            return Nst_int_new(p1 - s1_start + 1);
    }

    return Nst_inc_ref(Nst_const()->Int_neg1);
}

Nst_FUNC_SIGN(starts_with_)
{
    Nst_StrObj *str;
    Nst_StrObj *substr;

    Nst_DEF_EXTRACT("s s", &str, &substr);

    if (str->len < substr->len)
        Nst_RETURN_FALSE;

    i8 *s = str->value;
    i8 *sub = substr->value;
    i8 *sub_end = sub + substr->len;

    while (sub != sub_end) {
        if (*sub++ != *s++)
            Nst_RETURN_FALSE;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(ends_with_)
{
    Nst_StrObj *str;
    Nst_StrObj *substr;

    Nst_DEF_EXTRACT("s s", &str, &substr);

    if (str->len < substr->len)
        Nst_RETURN_FALSE;

    i8 *s_end = str->value + str->len;
    i8 *sub = substr->value;
    i8 *sub_end = sub + substr->len;

    while (sub_end != sub) {
        if (*--sub_end != *--s_end)
            Nst_RETURN_FALSE;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(trim_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    i8 *s_start = str->value;
    i8 *s_end = str->value + str->len - 1;
    usize len = str->len;

    while (isspace((u8)*s_start)) {
        ++s_start;
        --len;
    }

    if (s_start == s_end + 1)
        return Nst_string_new_c("", 0, false);

    while (isspace((u8)*s_end)) {
        --s_end;
        --len;
    }

    i8 *new_str = Nst_malloc_c(len + 1, i8);
    if (new_str == nullptr)
        return nullptr;

    strncpy(new_str, s_start, len);

    return Nst_string_new(new_str, len, true);
}

Nst_FUNC_SIGN(ltrim_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    i8 *s_start = str->value;
    usize len = str->len;

    while (isspace((u8)*s_start)) {
        ++s_start;
        --len;
    }

    i8 *new_str = Nst_malloc_c(len + 1, i8);
    if (new_str == nullptr)
        return nullptr;
    strcpy(new_str, s_start);

    return Nst_string_new(new_str, len, true);
}

Nst_FUNC_SIGN(rtrim_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    usize len = str->len;
    i8 *s_start = str->value;
    i8 *s_end = s_start + len - 1;

    if (len == 0)
        return Nst_string_new((i8 *)"", 0, false);

    while (s_end + 1 != s_start && isspace((u8)*s_end)) {
        --s_end;
        --len;
    }

    i8 *new_str = Nst_malloc_c(len + 1, i8);
    if (new_str == nullptr) {
        Nst_failed_allocation();
        return nullptr;
    }
    strncpy(new_str, s_start, len);

    return Nst_string_new(new_str, len, true);
}

Nst_FUNC_SIGN(ljust_)
{
    Nst_StrObj *str;
    i64 just_len;
    Nst_Obj *just_char;

    Nst_DEF_EXTRACT("s i ?s", &str, &just_len, &just_char);

    usize len = str->len;

    if (just_len <= (i64)len)
        return Nst_inc_ref(args[0]);

    i8 just_ch;

    if (just_char == Nst_null())
        just_ch = ' ';
    else {
        if (STR(just_char)->len != 1) {
            Nst_set_value_error_c(
                "filling string must be one character long");
            return nullptr;
        }
        just_ch = *STR(just_char)->value;
    }

    i8 *new_str = Nst_malloc_c((usize)just_len + 1, i8);
    if (new_str == nullptr)
        return nullptr;
    memcpy(new_str, str->value, len);
    memset(new_str + len, just_ch, (usize)(just_len - len));
    new_str[just_len] = 0;

    return Nst_string_new(new_str, (usize)just_len, true);
}

Nst_FUNC_SIGN(rjust_)
{
    Nst_StrObj *str;
    i64 just_len;
    Nst_Obj *just_char;

    Nst_DEF_EXTRACT("s i ?s", &str, &just_len, &just_char);

    usize len = str->len;

    if (just_len <= (i64)len)
        return Nst_inc_ref(args[0]);

    i8 just_ch;

    if (just_char == Nst_null())
        just_ch = ' ';
    else {
        if (STR(just_char)->len != 1) {
            Nst_set_value_error_c(
                "filling string must be one character long");
            return nullptr;
        }
        just_ch = *STR(just_char)->value;
    }

    i8 *new_str = Nst_malloc_c((usize)just_len + 1, i8);
    if (new_str == nullptr)
        return nullptr;
    memset(new_str, just_ch, (usize)(just_len - len));
    memcpy(new_str + (just_len - len), str->value, len);
    new_str[just_len] = 0;

    return Nst_string_new(new_str, (usize)just_len, true);
}

Nst_FUNC_SIGN(center_)
{
    Nst_StrObj *str;
    i64 just_len;
    Nst_Obj *just_char;

    Nst_DEF_EXTRACT("s i ?s", &str, &just_len, &just_char);

    usize len = str->len;

    if (just_len <= (i64)len)
        return Nst_inc_ref(args[0]);

    i8 just_ch;

    if (just_char == Nst_null())
        just_ch = ' ';
    else {
        if (STR(just_char)->len != 1) {
            Nst_set_value_error_c(
                "filling string must be one character long");
            return nullptr;
        }
        just_ch = *STR(just_char)->value;
    }

    i8 *new_str = Nst_malloc_c((usize)just_len + 1, i8);
    if (new_str == nullptr)
        return nullptr;
    usize half = (usize)(just_len - len) / 2;
    memset(new_str, just_ch, usize(just_len));
    memcpy(new_str + half, str->value, len);
    new_str[just_len] = 0;

    return Nst_string_new(new_str, (usize)just_len, true);
}

Nst_FUNC_SIGN(to_title_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    Nst_StrObj *new_str = STR(Nst_string_copy(str));
    i8 *s = new_str->value;
    i8 *end = s + new_str->len;
    bool new_word = true;

    while (s != end) {
        if (isspace((u8)*s)) {
            new_word = true;
            ++s;
            continue;
        }
        if (new_word) {
            *s = toupper((u8)*s);
            new_word = false;
        } else
            *s = tolower((u8)*s);
        ++s;
    }

    return OBJ(new_str);
}

Nst_FUNC_SIGN(to_upper_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    Nst_StrObj *new_str = STR(Nst_string_copy(str));
    i8 *s = new_str->value;
    i8 *end = s + new_str->len;

    while (s != end) {
        *s = toupper((u8)*s);
        ++s;
    }

    return OBJ(new_str);
}

Nst_FUNC_SIGN(to_lower_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    Nst_StrObj *new_str = STR(Nst_string_copy(str));
    i8 *s = new_str->value;
    i8 *end = s + new_str->len;

    while (s != end) {
        *s = tolower((u8)*s);
        ++s;
    }

    return OBJ(new_str);
}

Nst_FUNC_SIGN(is_title_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    i8 *s = str->value;
    i8 *end = s + str->len;
    bool new_word = true;

    while (s != end) {
        if (isspace((u8)*s)) {
            new_word = true;
            ++s;
            continue;
        } else if (!isalpha((u8)*s)) {
            ++s;
            continue;
        }

        if ((new_word && isupper((u8)*s)) || (!new_word && islower((u8)*s))) {
            new_word = false;
            ++s;
            continue;
        }
        Nst_RETURN_FALSE;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(is_upper_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    i8 *s = str->value;
    i8 *end = s + str->len;

    while (s != end) {
        if (isalpha((u8)*s) && !isupper((u8)*s))
            Nst_RETURN_FALSE;
        ++s;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(is_lower_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    i8 *s = str->value;
    i8 *end = s + str->len;

    while (s != end) {
        if (isalpha((u8)*s) && !islower((u8)*s))
            Nst_RETURN_FALSE;
        ++s;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(is_alpha_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    i8 *s = str->value;
    i8 *end = s + str->len;

    while (s != end) {
        if (!isalpha((u8)*s++))
            Nst_RETURN_FALSE;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(is_digit_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    i8 *s = str->value;
    i8 *end = s + str->len;

    while (s != end) {
        if (!isdigit((u8)*s++))
            Nst_RETURN_FALSE;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(is_alnum_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    i8 *s = str->value;
    i8 *end = s + str->len;

    while (s != end) {
        if (!isalnum((u8)*s++))
            Nst_RETURN_FALSE;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(is_charset_)
{
    Nst_StrObj *str1;
    Nst_StrObj *str2;

    Nst_DEF_EXTRACT("s s", &str1, &str2);

    i8 *s1 = str1->value;
    i8 *s2 = str2->value;
    i8 *end1 = s1 + str1->len;
    i8 *end2 = s2 + str2->len;
    i8 *p2 = s2;

    while (s1 != end1) {
        p2 = s2;
        while (p2 != end2) {
            if (*s1 == *p2)
                break;
            ++p2;
        }

        if (p2 == end2)
            Nst_RETURN_FALSE;
        ++s1;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(is_printable_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    i8 *s = str->value;
    i8 *end = s + str->len;

    while (s != end) {
        if (!isprint((u8)*s++))
            Nst_RETURN_FALSE;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(replace_substr_)
{
    Nst_StrObj *str;
    Nst_StrObj *str_from;
    Nst_StrObj *str_to;

    Nst_DEF_EXTRACT("s s s", &str, &str_from, &str_to);

    usize s_len = str->len;
    usize s_from_len = str_from->len;
    usize s_to_len = str_to->len;
    i8 *s = str->value;
    i8 *s_from = str_from->value;
    i8 *s_to = str_to->value;

    i8 *sub = nullptr;
    usize new_str_len = 0;
    i32 count = 0;

    while (true) {
        sub = Nst_string_find(s, s_len, s_from, s_from_len);
        if (sub == nullptr)
            break;

        s_len -= sub + s_from_len - s;
        s = sub + s_from_len;
        count++;
    }

    if (count == 0)
        return Nst_inc_ref(str);

    s_len = str->len;
    s = str->value;
    i8 *new_str = Nst_malloc_c(
        s_len - s_from_len * count + s_to_len * count + 1,
        i8);
    if (new_str == nullptr)
        return nullptr;

    // Copy replacing the occurrence
    while (true) {
        sub = Nst_string_find(s, s_len, s_from, s_from_len);
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
    return Nst_string_new(new_str, new_str_len, true);
}

Nst_FUNC_SIGN(bytearray_to_str_)
{
    Nst_SeqObj *seq;

    Nst_DEF_EXTRACT("A.B", &seq);

    usize len = seq->len;
    i8 *new_str = Nst_malloc_c(len + 1, i8);
    if (new_str == nullptr) {
        Nst_failed_allocation();
        return nullptr;
    }
    Nst_Obj **objs = seq->objs;

    for (usize i = 0; i < len; i++)
        new_str[i] = AS_BYTE(objs[i]);

    new_str[len] = 0;
    return Nst_string_new(new_str, len, true);
}

Nst_FUNC_SIGN(str_to_bytearray_)
{
    Nst_StrObj *str;

    Nst_DEF_EXTRACT("s", &str);

    usize len = str->len;
    i8 *s = str->value;
    Nst_SeqObj *new_arr = SEQ(Nst_array_new(len));
    Nst_Obj **objs = new_arr->objs;

    for (usize i = 0; i < len; i++)
        objs[i] = Nst_byte_new(s[i]);

    return OBJ(new_arr);
}

Nst_FUNC_SIGN(repr_)
{
    Nst_Obj *obj;
    Nst_DEF_EXTRACT("o", &obj);
    return _Nst_repr_str_cast(obj);
}

Nst_FUNC_SIGN(join_)
{
    Nst_SeqObj *seq;
    Nst_Obj *opt_str;

    Nst_DEF_EXTRACT("A ?s", &seq, &opt_str);

    usize str_len;
    i8 *str_val;

    if (opt_str == Nst_null()) {
        str_len = 1;
        str_val = (i8 *)" ";
    } else {
        str_len = STR(opt_str)->len;
        str_val = STR(opt_str)->value;
    }

    usize len = seq->len;
    usize tot_len = str_len * (len - 1);
    Nst_Obj **objs = Nst_malloc_c(len, Nst_Obj *);
    if (objs == nullptr) {
        Nst_failed_allocation();
        return nullptr;
    }

    for (usize i = 0; i < len; i++) {
        objs[i] = Nst_obj_cast(seq->objs[i], Nst_type()->Str);
        tot_len += STR(objs[i])->len;
    }

    i8 *new_str = Nst_malloc_c(tot_len + 1, i8);
    if (new_str == nullptr) {
        Nst_free(objs);
        Nst_failed_allocation();
        return nullptr;
    }
    usize str_idx = 0;

    for (usize i = 0; i < len; i++) {
        Nst_StrObj *curr_str = STR(objs[i]);
        memcpy(new_str + str_idx, curr_str->value, curr_str->len);
        str_idx += curr_str->len;
        Nst_dec_ref(objs[i]);
        if (i + 1 == len)
            break;
        memcpy(new_str + str_idx, str_val, str_len);
        str_idx += str_len;
    }
    Nst_free(objs);
    new_str[tot_len] = '\0';
    return Nst_string_new(new_str, tot_len, true);
}

Nst_FUNC_SIGN(split_)
{
    Nst_StrObj *str;
    Nst_Obj *opt_substr;

    Nst_DEF_EXTRACT("s ?s", &str, &opt_substr);

    i8 *sub;
    usize sub_len;
    bool rm_spaces = false;

    if (opt_substr == Nst_null()) {
        sub = (i8 *)" ";
        sub_len = 1;
        rm_spaces = true;
    } else {
        if (STR(opt_substr)->len == 0) {
            Nst_set_value_error_c(
                "separator must be at least one character long");
            return nullptr;
        }
        sub = STR(opt_substr)->value;
        sub_len = STR(opt_substr)->len;
    }

    Nst_SeqObj *vector = SEQ(Nst_vector_new(0));

    i8 *s = str->value;
    i8 *sub_idx = s;
    i8 *str_split;
    usize s_len = str->len;
    Nst_Obj *str_obj;

    if (rm_spaces) {
        while (*s == ' ') {
            s++;
            s_len--;
        }
    }

    if (s_len == 0)
        return OBJ(vector);

    while ((sub_idx = Nst_string_find(s, s_len, sub, sub_len)) != nullptr) {
        str_split = Nst_malloc_c(sub_idx - s + 1, i8);
        if (str_split == nullptr) {
            Nst_dec_ref(vector);
            Nst_failed_allocation();
            return nullptr;
        }

        memcpy(str_split, s, sub_idx - s);
        str_split[sub_idx - s] = '\0';
        str_obj = Nst_string_new(str_split, sub_idx - s, true);
        Nst_vector_append(vector, str_obj);
        Nst_dec_ref(str_obj);
        s_len -= sub_idx - s + sub_len;
        s = sub_idx + sub_len;

        if (rm_spaces) {
            while (*s == ' ') {
                s++;
                s_len--;
            }
            if (s_len == 0)
                break;
        }
    }

    if (s_len != 0) {
        str_split = Nst_malloc_c(s_len + 1, i8);
        if (str_split == nullptr) {
            Nst_dec_ref(vector);
            return nullptr;
        }
        memcpy(str_split, s, s_len);
        str_split[s_len] = '\0';
        str_obj = Nst_string_new(str_split, s_len, true);
        Nst_vector_append(vector, str_obj);
        Nst_dec_ref(str_obj);
    }

    return OBJ(vector);
}

static i64 highest_bit(u64 n)
{
    u64 str_len = 63;
    u64 part_size = 32;
    // binary search to find the highest true bit
    for (i32 i = 0; i < 6; i++) {
        if (n < (1ull << str_len >> part_size))
            str_len -= part_size;

        part_size >>= 1;
    }

    // if the highest bit is a 1
    if (1ull << 63ull & n)
        return str_len + 1;
    else
        return str_len;
}

Nst_FUNC_SIGN(bin_)
{
    i64 n;
    Nst_DEF_EXTRACT("i", &n);

    i64 str_len = highest_bit(n) + 1;

    i8 *buf = Nst_malloc_c((usize)str_len, i8);
    if (buf == nullptr) {
        Nst_failed_allocation();
        return nullptr;
    }

    for (i64 i = 0; i < i64(str_len - 1); i++) {
        if (1ll << i & n)
            buf[str_len - i - 2] = '1';
        else
            buf[str_len - i - 2] = '0';
    }
    buf[str_len - 1] = '\0';

    return Nst_string_new(buf, usize(str_len) - 1, true);
}

Nst_FUNC_SIGN(oct_)
{
    i64 n;
    Nst_DEF_EXTRACT("i", &n);

    i64 h_bit = highest_bit(n);
    i64 str_len = h_bit / 3;
    if (h_bit % 3)
        str_len += 2;
    else
        str_len += 1;

    i8 *buf = Nst_malloc_c((usize)str_len, i8);
    if (buf == nullptr) {
        Nst_failed_allocation();
        return nullptr;
    }

    for (i64 i = 0; i < i64(str_len - 1); i++) {
        i8 ch = i8((07ull << (i * 3) & u64(n)) >> (i * 3));
        buf[str_len - i - 2] = '0' + ch;
    }
    buf[str_len - 1] = '\0';

    return Nst_string_new(buf, usize(str_len) - 1, true);
}

Nst_FUNC_SIGN(hex_)
{
    i64 n;
    bool upper;
    Nst_DEF_EXTRACT("i y", &n, &upper);

    const i8 *digits;
    if (upper)
        digits = "0123456789abcdef";
    else
        digits = "0123456789ABCDEF";

    i64 h_bit = highest_bit(n);
    i64 str_len = h_bit / 4;
    if (h_bit % 4)
        str_len += 2;
    else
        str_len += 1;

    i8 *buf = Nst_malloc_c((usize)str_len, i8);
    if (buf == nullptr) {
        Nst_failed_allocation();
        return nullptr;
    }

    for (i64 i = 0; i < i64(str_len - 1); i++) {
        u64 ch_idx = (0xfull << (i * 4) & u64(n)) >> (i * 4);
        buf[str_len - i - 2] = digits[ch_idx];
    }
    buf[str_len - 1] = '\0';

    return Nst_string_new(buf, usize(str_len) - 1, true);
}

Nst_FUNC_SIGN(parse_int_)
{
    Nst_StrObj *str;
    Nst_Obj *base_obj;
    Nst_DEF_EXTRACT("s ?i", &str, &base_obj);

    i64 base = Nst_DEF_VAL(base_obj, AS_INT(base_obj), 0);

    return Nst_string_parse_int(str, i32(base));
}
