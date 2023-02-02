#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "str.h"
#include "error.h"
#include "lib_import.h"

#define IS_WHITESPACE(ch) \
        (ch == ' '  || \
         ch == '\n' || \
         ch == '\t' || \
         ch == '\r' || \
         ch == '\v' || \
         ch == '\f')

#define RETURN_INT_ERR do { \
    NST_SET_RAW_VALUE_ERROR(_NST_EM_BAD_INT_LITERAL); \
    return NULL; \
    } while ( 0 )

#define RETURN_BYTE_ERR do { \
    NST_SET_RAW_VALUE_ERROR(_NST_EM_BAD_BYTE_LITERAL); \
    return NULL; \
    } while ( 0 )

#define RETURN_REAL_ERR do { \
    NST_SET_RAW_VALUE_ERROR(_NST_EM_BAD_REAL_LITERAL); \
    return NULL; \
    } while ( 0 )

Nst_Obj *nst_new_cstring_raw(const char *val, bool allocated)
{
    return nst_new_string((char *)val, strlen(val), allocated);
}

Nst_Obj *nst_new_cstring(const char *val, size_t len, bool allocated)
{
    return nst_new_string((char *)val, len, allocated);
}

Nst_Obj *nst_new_string(char *val, size_t len, bool allocated)
{
    Nst_StrObj *str = STR(nst_alloc_obj(
        sizeof(Nst_StrObj),
        nst_t.Str,
        nst_destroy_string));
    if ( str == NULL )
    {
        return NULL;
    }

    if ( allocated )
    {
        str->flags |= NST_FLAG_STR_IS_ALLOC;
    }
    str->len = len;
    str->value = val;

    return OBJ(str);
}

Nst_TypeObj *nst_new_type_obj(const char *val, size_t len)
{
    Nst_TypeObj *str = STR(nst_alloc_obj(
        sizeof(Nst_StrObj),
        nst_t.Type,
        nst_destroy_string));
    if ( str == NULL )
    {
        return NULL;
    }

    str->len = len;
    str->value = (char *)val;

    return str;
}

Nst_Obj *_nst_copy_string(Nst_StrObj *src)
{
    char *buffer = (char *)malloc(sizeof(char) * (src->len + 1));
    if ( buffer == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    strcpy(buffer, src->value);

    return nst_new_string(buffer, src->len, true);
}

Nst_Obj *_nst_repr_string(Nst_StrObj *src)
{
    const char *hex_chars = "0123456789abcdef";
    char *orig = src->value;
    size_t l = src->len;
    size_t new_size = 2;
    int double_quotes_count = 0;
    int single_quotes_count = 0;
    bool using_doub = false;

    for ( size_t i = 0; i < l; i++ )
    {
        switch ( orig[i] )
        {
        case '\\':
        case '\a':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v': new_size += 2; break;
        case '\'': single_quotes_count += 1; break;
        case '"': double_quotes_count += 1; break;
        default:
            if ( isprint((unsigned char)orig[i]) )
            {
                new_size += 1;
            }
            else
            {
                new_size += 4;
            }
        }
    }

    if ( single_quotes_count > double_quotes_count )
    {
        using_doub = true;
        new_size += double_quotes_count * 2;
        new_size += single_quotes_count;
    }
    else
    {
        new_size += single_quotes_count * 2;
        new_size += double_quotes_count;
    }

    char *new_str = (char *)malloc(sizeof(char) * (new_size + 1));
    if ( new_str == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    *new_str = using_doub ? '"' : '\'';

    size_t i = 1;
    for ( size_t j = 0; j < l; j++ )
    {
        switch ( orig[j] )
        {
        case '\\': new_str[i++] = '\\'; new_str[i++] = '\\'; break;
        case '\a': new_str[i++] = '\\'; new_str[i++] = 'a'; break;
        case '\b': new_str[i++] = '\\'; new_str[i++] = 'b'; break;
        case '\f': new_str[i++] = '\\'; new_str[i++] = 'f'; break;
        case '\n': new_str[i++] = '\\'; new_str[i++] = 'n'; break;
        case '\r': new_str[i++] = '\\'; new_str[i++] = 'r'; break;
        case '\t': new_str[i++] = '\\'; new_str[i++] = 't'; break;
        case '\v': new_str[i++] = '\\'; new_str[i++] = 'v'; break;
        case '\'':
            if ( !using_doub )
            {
                new_str[i++] = '\\';
            }
            new_str[i++] = '\'';
            break;
        case '"':
            if ( using_doub )
            {
                new_str[i++] = '\\';
            }
            new_str[i++] = '"';
            break;
        default:
            if ( isprint((unsigned char)orig[j]) )
            {
                new_str[i++] = orig[j];
            }
            else
            {
                new_str[i++] = '\\';
                new_str[i++] = 'x';
                new_str[i++] = hex_chars[(unsigned char)orig[j] >> 4];
                new_str[i++] = hex_chars[(unsigned char)orig[j] & 0xf];
            }
        }
    }

    new_str[new_size - 1] = using_doub ? '"' : '\'';
    new_str[new_size] = 0;

    return nst_new_string(new_str, new_size, true);
}

Nst_Obj *_nst_string_get_idx(Nst_StrObj *str, Nst_Int idx)
{
    if ( idx < 0 )
    {
        idx += str->len;
    }

    if ( idx < 0 || idx >= (int64_t)str->len )
    {
        return NULL;
    }

    char *ch = (char *)malloc(2 * sizeof(char));
    if ( ch == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    ch[0] = str->value[idx];
    ch[1] = 0;

    return nst_new_string(ch, 1, true);
}

void nst_destroy_string(Nst_StrObj *str)
{
    if ( str == NULL )
    {
        return;
    }
    if ( NST_STR_IS_ALLOC(str) )
    {
        free(str->value);
    }
}

Nst_Obj *nst_parse_int(Nst_StrObj *str, struct _Nst_OpErr *err)
{
    char *s = str->value;
    char *end = s + str->len;
    Nst_Int num = 0;
    Nst_Int digit = 0;
    Nst_Int sign = 1;

    if ( s == end )
    {
        RETURN_INT_ERR;
    }

    while ( IS_WHITESPACE(*s) )
        ++s;

    if ( s == end )
    {
        RETURN_INT_ERR;
    }

    if ( *s == '-' )
    {
        sign = -1;
        ++s;
    }
    else if ( *s == '+' )
    {
        ++s;
    }

    if ( s == end )
    {
        RETURN_INT_ERR;
    }

    while ( s != end )
    {
        digit = *s - '0';
        if ( digit < 0 || digit > 9 )
        {
            while ( IS_WHITESPACE(*s) )
                ++s;

            if ( s == end )
            {
                return nst_new_int(num * sign);
            }
            else
            {
                RETURN_INT_ERR;
            }
        }

        num = num * 10 + digit;
        ++s;
    }

    return nst_new_int(num * sign);
}

Nst_Obj *nst_parse_byte(Nst_StrObj *str, struct _Nst_OpErr *err)
{
    if ( str->len == 1 )
    {
        return nst_new_byte(str->value[0]);
    }

    char* s = str->value;
    char* end = s + str->len;
    Nst_Int num = 0;
    Nst_Int digit = 0;
    Nst_Int sign = 1;

    if (s == end) RETURN_BYTE_ERR;

    while ( IS_WHITESPACE(*s) )
        ++s;

    if (s == end) RETURN_BYTE_ERR;

    if (*s == '-')
    {
        sign = -1;
        ++s;
    }
    else if (*s == '+')
        ++s;

    // No error checking since the string cannot be of length one

    while (s != end)
    {
        digit = *s - '0';
        if ( digit < 0 || digit > 9 )
        {
            if ( *s == 'b' || *s == 'B' )
            {
                ++s;
            }
            else
            {
                RETURN_BYTE_ERR;
            }

            while (IS_WHITESPACE(*s))
                ++s;

            if ( s == end )
            {
                return nst_new_byte((Nst_Byte)((num * sign) & 0xff));
            }
            else
            {
                RETURN_BYTE_ERR;
            }
        }

        num = num * 10 + digit;
        ++s;
    }

    // The byte literal must end with b or B
    RETURN_BYTE_ERR;
}

Nst_Obj *nst_parse_real(Nst_StrObj *str, struct _Nst_OpErr *err)
{
    char *s = str->value;
    char *end = s + str->len;
    Nst_Real num = 0;
    Nst_Real sign = 1.0;
    Nst_Real pow = 10;
    Nst_Int digit = 0;

    if ( s == end )
    {
        RETURN_REAL_ERR;
    }

    while ( IS_WHITESPACE(*s) )
        ++s;

    if ( s == end )
    {
        RETURN_REAL_ERR;
    }

    if ( *s == '-' )
    {
        sign = -1.0;
        ++s;
    }
    else if ( *s == '+' )
    {
        ++s;
    }

    if ( s == end )
    {
        RETURN_REAL_ERR;
    }

    while ( s != end && *s != '.' )
    {
        digit = *s - '0';
        if ( digit < 0 || digit > 9 )
        {
            while ( IS_WHITESPACE(*s) )
                ++s;

            if ( s == end )
            {
                return nst_new_real(num * sign);
            }
            else
            {
                RETURN_REAL_ERR;
            }
        }

        num = num * 10 + digit;
        ++s;
    }

    if ( s == end )
    {
        return nst_new_real(num * sign);
    }

    // at this point there can only be a dot
    ++s;

    while ( s != end )
    {
        digit = *s - '0';
        if ( digit < 0 || digit > 9 )
        {
            while ( IS_WHITESPACE(*s) )
                ++s;

            if ( s == end )
            {
                return nst_new_real(num * sign);
            }
            else
            {
                RETURN_REAL_ERR;
            }
        }

        num += digit / pow;
        ++s;
        pow *= 10;
    }

    return nst_new_real(num * sign);
}

int nst_compare_strings(Nst_StrObj *str1, Nst_StrObj *str2)
{
    char *p1 = str1->value;
    char *p2 = str2->value;
    char *end1 = p1 + str1->len;
    char *end2 = p2 + str2->len;

    while ( p1 != end1 && p2 != end2 )
    {
        if ( *p1 != *p2 )
        {
            return (int)(*p1 - *p2);
        }
        else
        {
            ++p1;
            ++p2;
        }
    }

    return (int)((Nst_Int)str1->len - (Nst_Int)str2->len);
}
