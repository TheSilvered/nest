#include <cctype>
#include <cstring>
#include <cstdlib>
#include "nest_sutil.h"

#define FUNC_COUNT 28

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
    {
        return false;
    }

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(lfind_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rfind_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(starts_with_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ends_with_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(trim_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ltrim_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rtrim_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ljust_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rjust_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(center_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(to_upper_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(to_lower_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_upper_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_lower_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_alpha_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_digit_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_alnum_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_charset_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_printable_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(replace_substr_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(bytearray_to_str_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(str_to_bytearray_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(repr_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(join_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(split_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(bin_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(oct_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(hex_, 2);

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

static char *find_substring(char *s1, size_t l1, char *s2, size_t l2)
{
    char* end1 = s1 + l1;
    char* end2 = s2 + l2;
    char* p1 = nullptr;
    char* p2 = nullptr;

    while (s1 != end1)
    {
        p1 = s1++;
        p2 = s2;

        while (p1 != end1 && p2 != end2 && *p1 == *p2)
        {
            ++p1;
            ++p2;
        }

        if (p2 == end2)
        {
            return s1 - 1;
        }
    }

    return nullptr;
}

NST_FUNC_SIGN(lfind_)
{
    Nst_StrObj *str1 = nullptr;
    Nst_StrObj *str2 = nullptr;

    NST_DEF_EXTRACT("ss", &str1, &str2);

    if ( str1 == str2 )
    {
        NST_RETURN_ZERO;
    }

    char *sub = find_substring(str1->value, str1->len, str2->value, str2->len);

    if ( sub == nullptr )
    {
        return nst_inc_ref(nst_c.Int_neg1);
    }
    else
    {
        return nst_new_int(sub - str1->value);
    }
}

NST_FUNC_SIGN(rfind_)
{
    Nst_StrObj *str1;
    Nst_StrObj *str2;

    NST_DEF_EXTRACT("ss", &str1, &str2);

    if ( str1 == str2 )
    {
        NST_RETURN_ZERO;
    }

    char *s1 = str1->value + str1->len - 1;
    char *s2 = str2->value + str2->len - 1;
    char *p1;
    char *p2;
    char *s1_start = str1->value;
    char *s2_start = str2->value;

    while ( s1 - s1_start + 1 )
    {
        p1 = s1--;
        p2 = s2;

        while ( p1 - s1_start + 1 &&
                p2 - s2_start + 1 &&
                *p1 == *p2 )
        {
            p1--;
            p2--;
        }

        if ( p2 - s2_start + 1 == 0 )
        {
            return nst_new_int(p1 - s1_start + 1);
        }
    }

    return nst_inc_ref(nst_c.Int_neg1);
}

NST_FUNC_SIGN(starts_with_)
{
    Nst_StrObj *str;
    Nst_StrObj *substr;

    NST_DEF_EXTRACT("ss", &str, &substr);

    if ( str->len < substr->len )
    {
        NST_RETURN_FALSE;
    }

    char *s = str->value;
    char *sub = substr->value;
    char *sub_end = sub + substr->len;

    while ( sub != sub_end )
    {
        if ( *sub++ != *s++)
            NST_RETURN_FALSE;
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(ends_with_)
{
    Nst_StrObj *str;
    Nst_StrObj *substr;

    NST_DEF_EXTRACT("ss", &str, &substr);

    if ( str->len < substr->len )
    {
        NST_RETURN_FALSE;
    }

    char *s_end = str->value + str->len;
    char *sub = substr->value;
    char *sub_end = sub + substr->len;

    while ( sub_end != sub )
    {
        if ( *--sub_end != *--s_end)
            NST_RETURN_FALSE;
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(trim_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    char *s_start = str->value;
    char *s_end = str->value + str->len - 1;
    size_t len = str->len;

    while ( isspace((unsigned char)*s_start) )
    {
        ++s_start;
        --len;
    }

    if ( s_start == s_end + 1 )
    {
        return nst_new_cstring("", 0, false);
    }

    while ( isspace((unsigned char)*s_end) )
    {
        --s_end;
        --len;
    }

    char *new_str = new char[len + 1];
    strncpy(new_str, s_start, len);

    return nst_new_string(new_str, len, true);
}

NST_FUNC_SIGN(ltrim_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    char *s_start = str->value;
    size_t len = str->len;

    while ( isspace((unsigned char)*s_start) )
    {
        ++s_start;
        --len;
    }

    char *new_str = new char[len + 1];
    strcpy(new_str, s_start);

    return nst_new_string(new_str, len, true);
}

NST_FUNC_SIGN(rtrim_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    size_t len = str->len;
    char *s_start = str->value;
    char *s_end = s_start + len - 1;

    if ( len == 0 )
    {
        return nst_new_string((char*)"", 0, false);
    }

    while ( s_end + 1 != s_start && isspace((unsigned char)*s_end) )
    {
        --s_end;
        --len;
    }

    char *new_str = new char[len + 1];
    strncpy(new_str, s_start, len);

    return nst_new_string(new_str, len, true);
}

NST_FUNC_SIGN(ljust_)
{
    Nst_StrObj *str;
    Nst_Int just_len;
    Nst_Obj *just_char;

    NST_DEF_EXTRACT("si?s", &str, &just_len, &just_char);

    size_t len = str->len;

    if ( just_len <= (Nst_Int)len )
    {
        return nst_inc_ref(args[0]);
    }

    char just_ch;

    if ( just_char == nst_c.null )
    {
        just_ch = ' ';
    }
    else
    {
        if ( STR(just_char)->len != 1 )
        {
            NST_SET_RAW_VALUE_ERROR(
                "filling string must be one character long");
            return nullptr;
        }
        just_ch = *STR(just_char)->value;
    }

    char *new_str = new char[(unsigned int)(just_len + 1)];
    memcpy(new_str, str->value, len);
    memset(new_str + len, just_ch, (size_t)(just_len - len));
    new_str[just_len] = 0;

    return nst_new_string(new_str, (size_t)just_len, true);
}

NST_FUNC_SIGN(rjust_)
{
    Nst_StrObj *str;
    Nst_Int just_len;
    Nst_Obj *just_char;

    NST_DEF_EXTRACT("si?s", &str, &just_len, &just_char);

    size_t len = str->len;

    if ( just_len <= (Nst_Int)len )
    {
        return nst_inc_ref(args[0]);
    }

    char just_ch;

    if ( just_char == nst_c.null )
    {
        just_ch = ' ';
    }
    else
    {
        if ( STR(just_char)->len != 1 )
        {
            NST_SET_RAW_VALUE_ERROR(
                "filling string must be one character long");
            return nullptr;
        }
        just_ch = *STR(just_char)->value;
    }

    char *new_str = new char[(unsigned int)(just_len + 1)];
    memset(new_str, just_ch, (size_t)(just_len - len));
    memcpy(new_str + (just_len - len), str->value, len);
    new_str[just_len] = 0;

    return nst_new_string(new_str, (size_t)just_len, true);
}

NST_FUNC_SIGN(center_)
{
    Nst_StrObj *str;
    Nst_Int just_len;
    Nst_Obj *just_char;

    NST_DEF_EXTRACT("si?s", &str, &just_len, &just_char);

    size_t len = str->len;

    if ( just_len <= (Nst_Int)len )
    {
        return nst_inc_ref(args[0]);
    }

    char just_ch;

    if ( just_char == nst_c.null )
    {
        just_ch = ' ';
    }
    else
    {
        if ( STR(just_char)->len != 1 )
        {
            NST_SET_RAW_VALUE_ERROR(
                "filling string must be one character long");
            return nullptr;
        }
        just_ch = *STR(just_char)->value;
    }

    char *new_str = new char[(unsigned int)(just_len + 1)];
    size_t half = (size_t)(just_len - len) / 2;
    memset(new_str, just_ch, just_len);
    memcpy(new_str + half, str->value, len);
    new_str[just_len] = 0;

    return nst_new_string(new_str, (size_t)just_len, true);
}

NST_FUNC_SIGN(to_upper_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    Nst_StrObj *new_str = STR(nst_copy_string(str));
    char *s = new_str->value;
    char* end = s + new_str->len;

    while (s != end)
    {
        *s = toupper((unsigned char)*s);
        ++s;
    }

    return OBJ(new_str);
}

NST_FUNC_SIGN(to_lower_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    Nst_StrObj *new_str = STR(nst_copy_string(str));
    char *s = new_str->value;
    char *end = s + new_str->len;

    while ( s != end )
    {
        *s = tolower((unsigned char)*s);
        ++s;
    }

    return OBJ(new_str);
}

NST_FUNC_SIGN(is_upper_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    char *s = str->value;
    char *end = s + str->len;

    while ( s != end )
    {
        if ( isalpha((unsigned char)*s) && !isupper((unsigned char)*s) )
        {
            NST_RETURN_FALSE;
        }
        ++s;
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(is_lower_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    char *s = str->value;
    char *end = s + str->len;

    while ( s != end )
    {
        if ( isalpha((unsigned char)*s) && !islower((unsigned char)*s) )
        {
            NST_RETURN_FALSE;
        }
        ++s;
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(is_alpha_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    char *s = str->value;
    char *end = s + str->len;

    while ( s != end )
    {
        if ( !isalpha((unsigned char)*s++) )
        {
            NST_RETURN_FALSE;
        }
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(is_digit_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    char *s = str->value;
    char *end = s + str->len;

    while ( s != end )
    {
        if ( !isdigit((unsigned char)*s++) )
        {
            NST_RETURN_FALSE;
        }
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(is_alnum_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    char *s = str->value;
    char *end = s + str->len;

    while ( s != end )
    {
        if ( !isalnum((unsigned char)*s++) )
        {
            NST_RETURN_FALSE;
        }
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(is_charset_)
{
    Nst_StrObj *str1;
    Nst_StrObj *str2;

    NST_DEF_EXTRACT("ss", &str1, &str2);

    char *s1 = str1->value;
    char *s2 = str2->value;
    char *end1 = s1 + str1->len;
    char *end2 = s2 + str2->len;
    char *p2 = s2;

    while ( s1 != end1 )
    {
        p2 = s2;
        while ( p2 != end2 )
        {
            if ( *s1 == *p2 )
            {
                break;
            }
            ++p2;
        }

        if ( p2 == end2 )
        {
            NST_RETURN_FALSE;
        }
        ++s1;
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(is_printable_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    char *s = str->value;
    char *end = s + str->len;

    while ( s != end )
    {
        if ( !isprint((unsigned char)*s++) )
        {
            NST_RETURN_FALSE;
        }
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(replace_substr_)
{
    Nst_StrObj *str;
    Nst_StrObj *str_from;
    Nst_StrObj *str_to;

    NST_DEF_EXTRACT("sss", &str, &str_from, &str_to);

    size_t s_len = str->len;
    size_t s_from_len = str_from->len;
    size_t s_to_len = str_to->len;
    char *s = str->value;
    char *s_from = str_from->value;
    char *s_to = str_to->value;

    char *sub = nullptr;
    size_t new_str_len = 0;
    int count = 0;

    while ( true )
    {
        sub = find_substring(s, s_len, s_from, s_from_len);
        if ( sub == nullptr )
        {
            break;
        }

        s_len -= sub + s_from_len - s;
        s = sub + s_from_len;
        count++;
    }

    if ( count == 0 )
    {
        return nst_inc_ref(str);
    }

    s_len = str->len;
    s = str->value;
    char *new_str = new char[s_len - s_from_len * count + s_to_len * count + 1];

    // Copy replacing the occurrence
    while ( true )
    {
        sub = find_substring(s, s_len, s_from, s_from_len);
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
    return nst_new_string(new_str, new_str_len, true);
}

NST_FUNC_SIGN(bytearray_to_str_)
{
    Nst_SeqObj *seq;

    NST_DEF_EXTRACT("A", &seq);

    size_t len = seq->len;
    char *new_str = new char[len + 1];
    Nst_Obj **objs = seq->objs;

    for ( size_t i = 0; i < len; i++ )
    {
        if ( objs[i]->type != nst_t.Byte )
        {
            NST_SET_TYPE_ERROR(_nst_format_error(
                "expected only type 'Byte', got type '%s' instead",
                "s",
                TYPE_NAME(objs[i])));
            delete[] new_str;
            return nullptr;
        }

        new_str[i] = AS_BYTE(objs[i]);
    }

    new_str[len] = 0;
    return nst_new_string(new_str, len, true);
}

NST_FUNC_SIGN(str_to_bytearray_)
{
    Nst_StrObj *str;

    NST_DEF_EXTRACT("s", &str);

    size_t len = str->len;
    char *s = str->value;
    Nst_SeqObj *new_arr = SEQ(nst_new_array(len));
    Nst_Obj **objs = new_arr->objs;

    for ( size_t i = 0; i < len; i++ )
    {
        objs[i] = nst_new_byte(s[i]);
    }

    return OBJ(new_arr);
}

NST_FUNC_SIGN(repr_)
{
    Nst_Obj *obj;
    NST_DEF_EXTRACT("o", &obj);
    return _nst_repr_str_cast(obj);
}

NST_FUNC_SIGN(join_)
{
    Nst_SeqObj *seq;
    Nst_Obj *opt_str;

    NST_DEF_EXTRACT("A?s", &seq, &opt_str);

    size_t str_len;
    char *str_val;

    if ( opt_str == nst_c.null )
    {
        str_len = 1;
        str_val = (char *)" ";
    }
    else
    {
        str_len = STR(opt_str)->len;
        str_val = STR(opt_str)->value;
    }

    size_t len = seq->len;
    size_t tot_len = str_len * (len - 1);
    Nst_Obj **objs = new Nst_Obj *[len];

    for ( size_t i = 0; i < len; i++ )
    {
        objs[i] = nst_obj_cast(seq->objs[i], nst_t.Str, nullptr);
        tot_len += STR(objs[i])->len;
    }

    char *new_str = new char[tot_len + 1];
    size_t str_idx = 0;

    for ( size_t i = 0; i < len; i++ )
    {
        Nst_StrObj *curr_str = STR(objs[i]);
        memcpy(new_str + str_idx, curr_str->value, curr_str->len);
        str_idx += curr_str->len;
        nst_dec_ref(objs[i]);
        if ( i + 1 == len )
        {
            break;
        }
        memcpy(new_str + str_idx, str_val, str_len);
        str_idx += str_len;
    }

    new_str[tot_len] = '\0';
    return nst_new_string(new_str, tot_len, true);
}

NST_FUNC_SIGN(split_)
{
    Nst_StrObj *str;
    Nst_Obj *opt_substr;

    NST_DEF_EXTRACT("s?s", &str, &opt_substr);

    char *sub;
    size_t sub_len;
    bool rm_spaces = false;

    if ( opt_substr == nst_c.null )
    {
        sub = (char *)" ";
        sub_len = 1;
        rm_spaces = true;
    }
    else
    {
        if ( STR(opt_substr)->len == 0 )
        {
            NST_SET_RAW_VALUE_ERROR(
                "separator must be at least one character long");
            return nullptr;
        }
        sub = STR(opt_substr)->value;
        sub_len = STR(opt_substr)->len;
    }

    Nst_SeqObj *vector = SEQ(nst_new_vector(0));

    char *s = str->value;
    char *sub_idx = s;
    char *str_split;
    size_t s_len = str->len;
    Nst_Obj *str_obj;

    if ( rm_spaces )
    {
        while ( *s == ' ' )
        {
            s++;
            s_len--;
        }
    }

    if ( s_len == 0 )
    {
        return OBJ(vector);
    }

    while ( (sub_idx = find_substring(s, s_len, sub, sub_len)) != nullptr )
    {
        str_split = new char[sub_idx - s + 1];
        memcpy(str_split, s, sub_idx - s);
        str_split[sub_idx - s] = '\0';
        str_obj = nst_new_string(str_split, sub_idx - s, true);
        nst_append_value_vector(vector, str_obj);
        nst_dec_ref(str_obj);
        s_len -= sub_idx - s + sub_len;
        s = sub_idx + sub_len;

        if ( rm_spaces )
        {
            while ( *s == ' ' )
            {
                s++;
                s_len--;
            }
            if ( s_len == 0 )
            {
                break;
            }
        }
    }

    if ( s_len != 0 )
    {
        str_split = new char[s_len + 1];
        memcpy(str_split, s, s_len);
        str_split[s_len] = '\0';
        str_obj = nst_new_string(str_split, s_len, true);
        nst_append_value_vector(vector, str_obj);
        nst_dec_ref(str_obj);
    }

    return OBJ(vector);
}

typedef unsigned long long ull;

static Nst_Int highest_bit(ull n)
{
    ull str_len = 63;
    ull part_size = 32;
    // binary search to find the highest true bit
    for ( int i = 0; i < 6; i++)
    {
        if ( n < (1ull << str_len >> part_size) )
        {
            str_len -= part_size;
        }

        part_size >>= 1;
    }

    // if the highest bit is a 1
    if ( 1ull << 63ull & n )
    {
        return str_len + 1;
    }
    else
    {
        return str_len;
    }
}

NST_FUNC_SIGN(bin_)
{
    Nst_Int n;
    NST_DEF_EXTRACT("i", &n);

    Nst_Int str_len = highest_bit(n) + 1;

    char *buf = new char[str_len];

    for ( Nst_Int i = 0; i < Nst_Int(str_len - 1); i++ )
    {
        if ( 1ll << i & n )
        {
            buf[str_len - i - 2] = '1';
        }
        else
        {
            buf[str_len - i - 2] = '0';
        }
    }
    buf[str_len - 1] = '\0';

    return nst_new_string(buf, str_len - 1, true);
}

NST_FUNC_SIGN(oct_)
{
    Nst_Int n;
    NST_DEF_EXTRACT("i", &n);

    Nst_Int h_bit = highest_bit(n);
    Nst_Int str_len = h_bit / 3;
    if ( h_bit % 3 )
    {
        str_len += 2;
    }
    else
    {
        str_len += 1;
    }

    char *buf = new char[str_len ];

    for ( Nst_Int i = 0; i < Nst_Int(str_len - 1); i++ )
    {
        char ch = char((07ull << (i * 3) & ull(n)) >> (i * 3));
        buf[str_len - i - 2] = '0' + ch;
    }
    buf[str_len - 1] = '\0';

    return nst_new_string(buf, str_len - 1, true);
}

NST_FUNC_SIGN(hex_)
{
    Nst_Int n;
    Nst_Obj *upper_obj;
    NST_DEF_EXTRACT("i?b", &n, &upper_obj);

    const char *digits;
    if ( upper_obj->type == nst_t.Null || !AS_BOOL(upper_obj) )
    {
        digits = "0123456789abcdef";
    }
    else
    {
        digits = "0123456789ABCDEF";
    }

    Nst_Int h_bit = highest_bit(n);
    Nst_Int str_len = h_bit / 4;
    if ( h_bit % 4 )
    {
        str_len += 2;
    }
    else
    {
        str_len += 1;
    }
    
    char *buf = new char[str_len];

    for ( Nst_Int i = 0; i < Nst_Int(str_len - 1); i++ )
    {
        ull ch_idx = (0xfull << (i * 4) & ull(n)) >> (i * 4);
        buf[str_len - i - 2] = digits[ch_idx];
    }
    buf[str_len - 1] = '\0';

    return nst_new_string(buf, str_len - 1, true);
}
