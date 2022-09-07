#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "str.h"

#define IS_WHITESPACE(ch) \
        (  ch == ' ' \
        || ch == '\n' \
        || ch == '\t' \
        || ch == '\r' \
        || ch == '\v' \
        || ch == '\f')

#define RETURN_INT_ERR do { \
    err->name = "Value Error"; \
    err->message = "invalid integer literal"; \
    return NULL; \
    } while (0)

#define RETURN_REAL_ERR do { \
    err->name = "Value Error"; \
    err->message = "invalid real literal"; \
    return NULL; \
    } while (0)

Nst_Obj *nst_new_string_raw(const char *val, bool allocated)
{
    Nst_StrObj *str = AS_STR(nst_alloc_obj(
        sizeof(Nst_StrObj),
        nst_t_str,
        nst_destroy_string
    ));
    if ( str == NULL ) return NULL;

    str->allocated = allocated;
    str->len = strlen(val);
    str->value = (char *)val;

    return (Nst_Obj *)str;
}

Nst_Obj *nst_new_string(char *val, size_t len, bool allocated)
{
    Nst_StrObj *str = AS_STR(nst_alloc_obj(
        sizeof(Nst_StrObj),
        nst_t_str,
        nst_destroy_string
    ));
    if ( str == NULL ) return NULL;

    str->allocated = allocated;
    str->len = len;
    str->value = val;

    return (Nst_Obj *)str;
}

Nst_Obj *nst_new_type_obj(const char *val, size_t len)
{
    Nst_StrObj *str = AS_STR(nst_alloc_obj(
        sizeof(Nst_StrObj),
        nst_t_type,
        nst_destroy_string
    ));
    if ( str == NULL ) return NULL;

    str->allocated = false;
    str->len = len;
    str->value = (char *)val;

    return (Nst_Obj *)str;
}

Nst_Obj *_nst_copy_string(Nst_StrObj *src)
{
    char *buffer = malloc(sizeof(char) * (src->len + 1));
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
            if ( isprint(orig[i]) )
                new_size += 1;
            else
                new_size += 4;
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

    char *new_str = malloc(sizeof(char) * (new_size + 1));
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
            if ( !using_doub ) new_str[i++] = '\\';
            new_str[i++] = '\'';
            break;
        case '"':
            if ( using_doub ) new_str[i++] = '\\';
            new_str[i++] = '"';
            break;
        default:
            if ( isprint(orig[j]) )
                new_str[i++] = orig[j];
            else
            {
                sprintf(&new_str[i], "\\x%02x", orig[j]);
                i += 4;
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
        idx += str->len;

    if ( idx < 0 || idx >= (int64_t)str->len )
        return NULL;

    char *ch = malloc(2 * sizeof(char));
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
    if ( str == NULL ) return;
    if ( str->allocated )
        free(str->value);
}

Nst_Obj *nst_parse_int(char *str, Nst_OpErr *err)
{
    register char *s = str;
    register Nst_Int num = 0;
    register Nst_Int digit = 0;
    register Nst_Int sign = 1;

    if ( *s == 0 ) RETURN_INT_ERR;

    while ( IS_WHITESPACE(*s) )
        ++s;

    if ( *s == 0 ) RETURN_INT_ERR;

    if ( *s == '-' )
    {
        sign = -1;
        ++s;
    }

    if ( *s == 0 ) RETURN_INT_ERR;

    while ( *s )
    {
        digit = *s - '0';
        if ( digit < 0 || digit > 9 )
        {
            while ( IS_WHITESPACE(*s) )
                ++s;

            if ( *s == 0 )
                return nst_new_int(num * sign);
            else
                RETURN_INT_ERR;
        }

        num = num * 10 + digit;
        ++s;
    }

    return nst_new_int(num * sign);
}

Nst_Obj *nst_parse_real(char *str, Nst_OpErr *err)
{
    register char *s = str;
    register Nst_Real num = 0;
    register Nst_Real sign = 1.0;
    register Nst_Real pow = 10;
    register Nst_Int digit = 0;

    if ( *s == 0 ) RETURN_REAL_ERR;

    while ( IS_WHITESPACE(*s) )
        ++s;

    if ( *s == 0 ) RETURN_REAL_ERR;

    if ( *s == '-' )
    {
        sign = -1.0;
        ++s;
    }

    if ( *s == 0 ) RETURN_REAL_ERR;

    while ( *s && *s != '.' )
    {
        digit = *s - '0';
        if ( digit < 0 || digit > 9 )
        {
            while ( IS_WHITESPACE(*s) )
                ++s;

            if ( *s == 0 )
                return nst_new_real(num * sign);
            else
                RETURN_REAL_ERR;
        }

        num = num * 10 + digit;
        ++s;
    }

    if ( *s == 0 )
        return nst_new_real(num * sign);

    // here, there can only be a dot
    ++s;

    while ( *s )
    {
        digit = *s - '0';
        if ( digit < 0 || digit > 9 )
        {
            while ( IS_WHITESPACE(*s) )
                ++s;

            if ( *s == 0 )
                return nst_new_real(num * sign);
            else
                RETURN_REAL_ERR;
        }

        num += digit / pow;
        ++s;
        pow *= 10;
    }

    return nst_new_real(num * sign);
}
