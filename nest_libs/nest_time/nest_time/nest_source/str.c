#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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

Nst_Obj *new_str_obj(Nst_string *str)
{
    return make_obj(str, nst_t_str, destroy_string);
}

Nst_string *new_string_raw(const char *val, bool allocated)
{
    Nst_string *str = malloc(sizeof(Nst_string));
    if ( str == NULL ) return NULL;

    str->allocated = allocated;
    str->len = strlen(val);
    str->value = (char *)val;

    return str;
}

Nst_string *new_string(char *val, size_t len, bool allocated)
{
    Nst_string *str = malloc(sizeof(Nst_string));
    if ( str == NULL ) return NULL;

    str->allocated = allocated;
    str->len = len;
    str->value = val;

    return str;
}

Nst_string *copy_string(Nst_string *src)
{
    char *buffer = malloc(sizeof(char) * (src->len + 1));
    if ( buffer == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    strcpy(buffer, src->value);

    return new_string(buffer, src->len, true);
}

void destroy_string(Nst_string *str)
{
    if ( str == NULL ) return;
    if ( str->allocated )
        free(str->value);
    free(str);
}

Nst_int *parse_int(char *str, OpErr *err)
{
    register char *s = str;
    register Nst_int num = 0;
    register Nst_int digit = 0;
    register Nst_int sign = 1;

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
                return new_int(num * sign);
            else
                RETURN_INT_ERR;
        }

        num = num * 10 + digit;
        ++s;
    }

    return new_int(num * sign);
}

Nst_real *parse_real(char *str, OpErr *err)
{
    register char *s = str;
    register Nst_real num = 0;
    register Nst_real sign = 1.0;
    register Nst_real pow = 10;
    register Nst_int digit = 0;

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
                return new_real(num * sign);
            else
                RETURN_REAL_ERR;
        }

        num = num * 10 + digit;
        ++s;
    }

    if ( *s == 0 )
        return new_real(num * sign);

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
                return new_real(num * sign);
            else
                RETURN_REAL_ERR;
        }

        num += digit / pow;
        ++s;
        pow *= 10;
    }

    return new_real(num * sign);
}