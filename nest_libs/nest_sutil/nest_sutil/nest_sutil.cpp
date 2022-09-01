#include "nest_sutil.h"
#include <cctype>
#include <cstring>
#include <cstdlib>

#define FUNC_COUNT 21

#define EXPECTED_BYTE "expected only type 'Byte', got type '%s' instead"

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_NAMED_FUNCDECLR(nst_lfind, 2, "lfind");
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rfind, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(trim, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ltrim, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rtrim, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ljust, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rjust, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(to_upper, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(to_lower, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_upper, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_lower, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_alpha, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_digit, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_alnum, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_charset, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_printable, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(replace_substr, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(bytearray_to_str, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(repr, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(join, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(split, 2);

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}



Nst_Obj *nst_lfind(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str1 = nullptr;
    Nst_StrObj *str2 = nullptr;

    if ( !nst_extract_arg_values("ss", arg_num, args, err, &str1, &str2) )
        return nullptr;

    if ( str1 == str2 )
        return nst_new_int(0);

    char *s1 = str1->value;
    char *s2 = str2->value;
    char *p1 = nullptr;
    char *p2 = nullptr;

    while ( *s1 )
    {
        p1 = s1++;
        p2 = s2;

        while ( *p1 && *p2 && *p1 == *p2 )
        {
            ++p1;
            ++p2;
        }

        if ( *p2 == 0 )
            return nst_new_int(s1 - str1->value - 1);
    }

    return nst_new_int(-1);
}

Nst_Obj *rfind(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str1;
    Nst_StrObj *str2;

    if ( !nst_extract_arg_values("ss", arg_num, args, err, &str1, &str2) )
        return nullptr;

    if ( str1 == str2 )
        return nst_new_int(0);

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
            return nst_new_int(p1 - s1_start + 1);
    }

    return nst_new_int(-1);
}

Nst_Obj *trim(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    char *s_start = str->value;
    char *s_end = str->value + str->len - 1;
    size_t len = str->len;

    while ( *s_start && isspace(*s_start) )
    {
        ++s_start;
        --len;
    }

    if ( s_start == s_end + 1 )
        return nst_new_string((char *)"", 0, false);

    while ( isspace(*s_end) )
    {
        --s_end;
        --len;
    }

    char *new_str = new char[len + 1];
    strncpy(new_str, s_start, len);

    return nst_new_string(new_str, len, true);
}

Nst_Obj *ltrim(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    char *s_start = str->value;
    size_t len = str->len;

    while ( *s_start && isspace(*s_start) )
    {
        ++s_start;
        --len;
    }

    char *new_str = new char[len + 1];
    strcpy(new_str, s_start);

    return nst_new_string(new_str, len, true);
}

Nst_Obj *rtrim(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    char *s_start = str->value;
    char *s_end = str->value + str->len - 1;
    size_t len = str->len;

    if ( len == 0 )
        return nst_new_string((char *)"", 0, false);

    while ( s_end + 1 != s_start && isspace(*s_end) )
    {
        --s_end;
        --len;
    }

    char *new_str = new char[len + 1];
    strncpy(new_str, s_start, len);

    return nst_new_string(new_str, len, true);
}

Nst_Obj *ljust(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;
    Nst_Int just_len;
    Nst_StrObj *just_char;

    if ( !nst_extract_arg_values("sis", arg_num, args, err, &str, &just_len, &just_char) )
        return nullptr;

    size_t len = str->len;

    if ( just_len <= (Nst_Int)len )
        return inc_ref(args[0]);

    if ( just_char->len != 1 )
    {
        err->name = (char *)"Value Error";
        err->message = (char *)"filling string must be one character long";
        return NULL;
    }

    char *new_str = new char[just_len + 1];
    memcpy(new_str, str->value, len);
    memset(new_str + len, *(just_char->value), just_len - len);
    new_str[just_len] = 0;

    return nst_new_string(new_str, just_len, true);
}

Nst_Obj *rjust(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;
    Nst_Int just_len;
    Nst_StrObj *just_char;

    if ( !nst_extract_arg_values("sis", arg_num, args, err, &str, &just_len, &just_char) )
        return nullptr;

    size_t len = str->len;

    if ( just_len <= (Nst_Int)len )
        return inc_ref(args[0]);

    if ( just_char->len != 1 )
    {
        err->name = (char *)"Value Error";
        err->message = (char *)"filling string must be one character long";
        return NULL;
    }

    char *new_str = new char[just_len + 1];
    memset(new_str, *(just_char->value), just_len - len);
    memcpy(new_str + (just_len - len), str->value, len);
    new_str[just_len] = 0;

    return nst_new_string(new_str, just_len, true);
}

Nst_Obj *to_upper(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    Nst_StrObj *new_str = AS_STR(nst_copy_string(str));
    char *s = new_str->value;

    while ( *s )
    {
        *s = toupper(*s);
        ++s;
    }

    return (Nst_Obj *)new_str;
}

Nst_Obj *to_lower(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    Nst_StrObj *new_str = AS_STR(nst_copy_string(str));
    char *s = new_str->value;

    while ( *s )
    {
        *s = tolower(*s);
        ++s;
    }

    return (Nst_Obj *)new_str;
}

Nst_Obj *is_upper(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    char *s = str->value;

    while ( *s )
    {
        if ( isalpha(*s) && !isupper(*s) )
            return inc_ref(nst_false);
        ++s;
    }

    return inc_ref(nst_true);
}

Nst_Obj *is_lower(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    char *s = str->value;

    while ( *s )
    {
        if ( isalpha(*s) && !islower(*s) )
            return inc_ref(nst_false);
        ++s;
    }

    return inc_ref(nst_true);
}

Nst_Obj *is_alpha(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    char *s = str->value;

    while ( *s )
    {
        if ( !isalpha(*s++) )
            return inc_ref(nst_false);
    }

    return inc_ref(nst_true);
}

Nst_Obj *is_digit(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    char *s = str->value;

    while ( *s )
    {
        if ( !isdigit(*s++) )
            return inc_ref(nst_false);
    }

    return inc_ref(nst_true);
}

Nst_Obj *is_alnum(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    char *s = str->value;

    while ( *s )
    {
        if ( !isalnum(*s++) )
            return inc_ref(nst_false);
    }

    return inc_ref(nst_true);
}

Nst_Obj *is_charset(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str1;
    Nst_StrObj *str2;

    if ( !nst_extract_arg_values("ss", arg_num, args, err, &str1, &str2) )
        return nullptr;

    char *s1 = str1->value;
    char *s2 = str2->value;
    char *p2 = s2;

    while ( *s1 )
    {
        p2 = s2;
        while ( *p2 )
        {
            if ( *s1 == *p2 )
                break;
            ++p2;
        }

        if ( !*p2 )
            return inc_ref(nst_false);
        ++s1;
    }

    return inc_ref(nst_true);
}

Nst_Obj *is_printable(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    char *s = str->value;

    while ( *s )
    {
        if ( !isprint(*s++) )
            return inc_ref(nst_false);
    }

    return inc_ref(nst_true);
}

Nst_Obj *replace_substr(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;
    Nst_StrObj *str1;
    Nst_StrObj *str2;

    if ( !nst_extract_arg_values("sss", arg_num, args, err, &str, &str1, &str2) )
        return nullptr;

    char *s = str->value;
    char *s1 = str1->value;
    char *s2 = str2->value;
    char *sub = strstr(s, s1);

    if ( sub == nullptr || *s1 == 0 )
        return inc_ref(args[0]);

    size_t s_len = str->len;
    size_t s1_len = str1->len;
    size_t s2_len = str2->len;
    size_t new_str_len = 0;
    size_t new_sub_len = 0;
    char *realloc_new_str = nullptr;

    new_sub_len = sub - s + s2_len;
    char *new_str = (char *)malloc(sizeof(char) * new_sub_len + 1);
    if ( new_str == nullptr )
        return nullptr;

    memcpy(new_str, s, sub - s);
    memcpy(new_str + (sub - s), s2, s2_len);
    new_str_len += new_sub_len;
    s_len -= sub - s + s1_len;
    s = sub + s1_len;

    while ( (sub = strstr(s, s1)) != nullptr )
    {
        new_sub_len = sub - s + s2_len;
        realloc_new_str = (char *)realloc(new_str, new_str_len + new_sub_len + 1);
        if ( realloc_new_str == nullptr )
            return nullptr;
        new_str = realloc_new_str;

        memcpy(new_str + new_str_len, s, sub - s);
        memcpy(new_str + (sub - s) + new_str_len, s2, s2_len);
        new_str_len += new_sub_len;
        s_len -= sub - s + s1_len;
        s = sub + s1_len;
    }

    realloc_new_str = (char *)realloc(new_str, new_str_len + s_len + 1);
    if ( realloc_new_str == nullptr )
        return nullptr;
    new_str = realloc_new_str;

    memcpy(new_str + new_str_len, s, s_len);
    new_str_len += s_len;

    new_str[new_str_len] = 0;
    return nst_new_string(new_str, new_str_len, true);
}

Nst_Obj *bytearray_to_str(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_SeqObj *seq;

    if ( !nst_extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    size_t len = seq->len;
    char *new_str = new char[len + 1];
    Nst_Obj **objs = seq->objs;

    for ( size_t i = 0; i < len; i++ )
    {
        if ( objs[i]->type != nst_t_byte )
        {
            NST_SET_TYPE_ERROR(_nst_format_type_error(EXPECTED_BYTE, objs[i]->type_name));
            delete[] new_str;
            return NULL;
        }

        Nst_Byte val = AS_BYTE(objs[i]);

        if ( val == 0 )
        {
            err->name = (char *)"Value Error";
            err->message = (char *)"fount byte zero in byte array";
            delete[] new_str;
            return NULL;
        }
        new_str[i] = val;
    }

    new_str[len] = 0;
    return nst_new_string(new_str, len, true);
}

Nst_Obj *repr(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &str) )
        return nullptr;

    return nst_repr_string(str);
}

Nst_Obj *join(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;
    Nst_SeqObj *seq;

    if ( !nst_extract_arg_values("sA", arg_num, args, err, &str, &seq) )
        return nullptr;

    size_t len = seq->len;
    size_t str_len = str->len;
    size_t tot_len = str_len * (len - 1);
    Nst_Obj **objs = new Nst_Obj *[len];

    for ( size_t i = 0; i < len; i++ )
    {
        objs[i] = nst_obj_cast(seq->objs[i], nst_t_str, NULL);
        tot_len += AS_STR(objs[i])->len;
    }

    char *new_str = new char[tot_len + 1];
    size_t str_idx = 0;

    for ( size_t i = 0; i < len; i++ )
    {
        Nst_StrObj *curr_str = AS_STR(objs[i]);
        memcpy(new_str + str_idx, curr_str->value, curr_str->len);
        str_idx += curr_str->len;
        dec_ref(objs[i]);
        if ( i + 1 == len ) break;
        memcpy(new_str + str_idx, str->value, str_len);
        str_idx += str_len;
    }

    new_str[tot_len] = 0;
    return nst_new_string(new_str, tot_len, true);
}

Nst_Obj *split(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_StrObj *str;
    Nst_StrObj *substr;

    if ( !nst_extract_arg_values("ss", arg_num, args, err, &str, &substr) )
        return nullptr;

    if ( substr->len == 0 )
    {
        NST_SET_VALUE_ERROR("separator must be at least one character long");
        return nullptr;
    }

    Nst_SeqObj *vector = AS_SEQ(nst_new_vector(0));

    char *s = str->value;
    char *sub = substr->value;
    char *sub_idx = s;
    char *str_split;
    size_t s_len = str->len;
    Nst_Obj *str_obj;

    while ( (sub_idx = strstr(s, sub)) != NULL )
    {
        str_split = new char[sub_idx - s + 1];
        memcpy(str_split, s, sub_idx - s);
        str_split[sub_idx - s] = 0;
        str_obj = nst_new_string(str_split, sub_idx - s, true);
        nst_append_value_vector(vector, str_obj);
        dec_ref(str_obj);
        s_len -= sub_idx - s + substr->len;
        s = sub_idx + substr->len;
    }

    str_split = new char[s_len + 1];
    memcpy(str_split, s, s_len);
    str_split[s_len] = 0;
    str_obj = nst_new_string(str_split, s_len, true);
    nst_append_value_vector(vector, str_obj);
    dec_ref(str_obj);

    return (Nst_Obj *)vector;
}
