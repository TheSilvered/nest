#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "str.h"
#include "error.h"
#include "lib_import.h"
#include "lexer.h"

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

#define ERR_IF_END(s, end, err_macro) do { \
    if ( s == end ) \
    { \
        err_macro; \
    } \
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

static int is_unicode_escape(unsigned char b1, unsigned char b2)
{
    int v = ((b1 & 0x1f) << 6) + (b2 & 0x3f);
    return v >= 0x80 && v <= 0x9f ? v : 0;
}

Nst_Obj *_nst_repr_string(Nst_StrObj *src)
{
    const char *hex_chars = "0123456789abcdef";
    unsigned char *orig = (unsigned char *)src->value;
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
        case '\x1b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v': new_size += 2; break;
        case '\'': single_quotes_count += 1; break;
        case '"': double_quotes_count += 1; break;
        default:
            if ( isprint(orig[i]) )
            {
                new_size += 1;
                continue;
            }
            else if ( orig[i] <= 0b01111111 )
            {
                new_size += 4;
            }
            else
            {
                int res = nst_check_utf8_bytes(orig + i, l - i);
                if ( res == -1 )
                {
                    new_size += 4;
                }
                else if ( res == 2 && is_unicode_escape(orig[i], orig[i + 1]) )
                {
                    new_size += 6;
                    i++;
                }
                else
                {
                    new_size += res;
                    i += res - 1;
                }
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
        case '\\': new_str[i++] = '\\'; new_str[i++] = '\\';break;
        case '\a': new_str[i++] = '\\'; new_str[i++] = 'a'; break;
        case '\b': new_str[i++] = '\\'; new_str[i++] = 'b'; break;
        case'\x1b':new_str[i++] = '\\'; new_str[i++] = 'e'; break;
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
            else if ( orig[j] <= 0b01111111 ||
                      nst_check_utf8_bytes(orig + j, l - j) == -1 )
            {
                new_str[i++] = '\\';
                new_str[i++] = 'x';
                new_str[i++] = hex_chars[(unsigned char)orig[j] >> 4];
                new_str[i++] = hex_chars[(unsigned char)orig[j] & 0xf];
            }
            else
            {
                int res = nst_check_utf8_bytes(orig + j, l - j);
                if ( res == 2 && is_unicode_escape(orig[j], orig[j + 1]) )
                {
                    res = is_unicode_escape(orig[j], orig[j + 1]);
                    j++;
                    new_str[i++] = '\\';
                    new_str[i++] = 'u';
                    new_str[i++] = '0';
                    new_str[i++] = '0';
                    new_str[i++] = hex_chars[res >> 4];
                    new_str[i++] = hex_chars[res & 0xf];
                    continue;
                }

                for ( ; res > 0; res-- )
                {
                    new_str[i++] = orig[j++];
                }
                j--;
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

Nst_Obj *nst_parse_int(Nst_StrObj *str, int base, struct _Nst_OpErr *err)
{
    char *s = str->value;
    char *end = s + str->len;
    char ch;
    int ch_val;
    int sign = 1;
    Nst_Int num = 0;
    Nst_Int cut_off = 0;
    Nst_Int cut_lim = 0;

    if ( (base < 2 || base > 36) && base != 0 )
    {
        NST_SET_RAW_VALUE_ERROR(_NST_EM_BAD_INT_BASE);
        return NULL;
    }
    ERR_IF_END(s, end, RETURN_INT_ERR);

    ch = *s;
    while ( IS_WHITESPACE(ch)  )
    {
        ch = *++s;
    }
    ERR_IF_END(s, end, RETURN_INT_ERR);

    if ( ch == '-' || ch == '+' )
    {
        sign = ch == '-' ? -1 : 1;
        ch = *++s;
    }
    ERR_IF_END(s, end, RETURN_INT_ERR);

    if ( ch == '0' )
    {
        ch = *++s;
        switch ( ch )
        {
        case 'B':
        case 'b':
            if ( base != 2 && base != 0 )
            {
                RETURN_INT_ERR;
            }
            base = 2;
            ch = *++s;
            break;
        case 'O':
        case 'o':
            if ( base != 8 && base != 0 )
            {
                RETURN_INT_ERR;
            }
            base = 8;
            ch = *++s;
            break;
        case 'X':
        case 'x':
            if ( base != 16 && base != 0 )
            {
                RETURN_INT_ERR;
            }
            base = 16;
            ch = *++s;
            break;
        default:
            ch = *--s;
            break;
        }
    }
    if ( base == 0 )
    {
        base = 10;
    }

    cut_off = sign == -1 ? -9223372036854775807 - 1 : 9223372036854775807;
    cut_lim = sign * (cut_off % base);
    cut_off /= sign * base;
    while ( true )
    {
        if ( ch >= '0' && ch <= '9' )
        {
            ch_val = ch - '0';
        }
        else if ( ch >= 'a' && ch <= 'z' )
        {
            ch_val = ch - 'a' + 10;
        }
        else if ( ch >= 'A' && ch <= 'Z' )
        {
            ch_val = ch - 'A' + 10;
        }
        else if ( ch == '_' )
        {
            ch = *++s;
            continue;
        }
        else
        {
            break;
        }

        if ( ch_val < 0 || ch_val > base - 1 )
        {
            RETURN_INT_ERR;
        }

        if ( num > cut_off || (num == cut_off && ch_val > cut_lim) )
        {
            NST_SET_RAW_MEMORY_ERROR(_NST_EM_INT_TOO_BIG);
            return NULL;
        }
        num *= base;
        num += ch_val;
        ch = *++s;
    }

    while ( IS_WHITESPACE(ch) )
    {
        ch = *++s;
    }

    if ( s != end )
    {
        RETURN_INT_ERR;
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
    char ch = *s;
    int num = 0;
    int ch_val = 0;
    int sign = 1;
    int base = 10;

    if ( s == end )
    {
        RETURN_BYTE_ERR;
    }

    while ( IS_WHITESPACE(ch) )
    {
        ch = *++s;
    }

    if ( s == end )
    {
        RETURN_BYTE_ERR;
    }

    if ( ch == '-' )
    {
        sign = -1;
        ch = *++s;
    }
    else if ( ch == '+' )
    {
        ch = *++s;
    }

    if ( s == end )
    {
        RETURN_BYTE_ERR;
    }

    if ( ch == '0' )
    {
        ch = *++s;
        switch ( ch )
        {
        case 'b':
        case 'B':
            base = 2;
            ch = *++s;

            if ( IS_WHITESPACE(ch) )
            {
                while ( IS_WHITESPACE(ch) )
                {
                    ch = *++s;
                }

                if ( s == end )
                {
                    return nst_new_byte(0);
                }
                else
                {
                    RETURN_BYTE_ERR;
                }
            }
            break;
        case 'o':
        case 'O':
            base = 8;
            ch = *++s;
            break;
        case 'h':
        case 'H':
            base = 16;
            ch = *++s;
            break;
        default:
            ch = *--s;
        }
    }

    while ( true )
    {
        if ( ch >= '0' && ch <= '9' )
        {
            ch_val = ch - '0';
        }
        else if ( ch >= 'a' && ch <= 'z' && base == 16 )
        {
            ch_val = ch - 'a' + 10;
        }
        else if ( ch >= 'A' && ch <= 'Z' && base == 16 )
        {
            ch_val = ch - 'A' + 10;
        }
        else if ( ch == '_' )
        {
            ch = *++s;
            continue;
        }
        else
        {
            break;
        }
        num *= base;
        num += ch_val;
        num %= 256;
        ch = *++s;
    }
    num *= sign;
    if ( base != 16 && ch != 'b' )
    {
        RETURN_BYTE_ERR;
    }
    ch = *++s;
    while ( IS_WHITESPACE(ch) )
    {
        ch = *++s;
    }
    if ( s != end )
    {
        RETURN_BYTE_ERR;
    }
    return nst_new_byte(num & 0xff);
}

Nst_Obj *nst_parse_real(Nst_StrObj *str, struct _Nst_OpErr *err)
{
    // \s*[+-]?\d+\.\d+(?:[eE][+-]?\d+)

    // strtod accepts also things like .5, 1e2, -1., ecc. that I do not
    // so I need to check if the literal is valid first
    char *s = str->value;
    char *end = s + str->len;
    char *start = s;
    size_t len = 0;
    char ch = *s;
    char *buf;
    Nst_Real res;

    if ( s == end )
    {
        RETURN_REAL_ERR;
    }

    while ( IS_WHITESPACE(ch) )
    {
        ch = *++s;
        start++;
    }
    ERR_IF_END(s, end, RETURN_REAL_ERR);

    if ( ch == '+' || ch == '-' )
    {
        ch = *++s;
        len++;
    }
    ERR_IF_END(s, end, RETURN_REAL_ERR);

    if ( ch < '0' || ch > '9' )
    {
        RETURN_REAL_ERR;
    }
    while ( (ch >= '0' && ch <= '9') || ch == '_' )
    {
        ch = *++s;
        len++;
    }

    if ( ch != '.' )
    {
        while ( IS_WHITESPACE(ch) )
        {
            ch = *++s;
        }
        if ( s != end )
        {
            RETURN_REAL_ERR;
        }
        goto end;
    }
    ch = *++s;
    len++;

    if ( ch < '0' || ch > '9' )
    {
        RETURN_REAL_ERR;
    }
    while ( (ch >= '0' && ch <= '9') || ch == '_' )
    {
        ch = *++s;
        len++;
    }

    if ( ch == 'e' || ch == 'E' )
    {
        ch = *++s;
        len++;

        if ( ch == '+' || ch == '-' )
        {
            ch = *++s;
            len++;
        }
        ERR_IF_END(s, end, RETURN_REAL_ERR);

        if ( ch < '0' || ch > '9' )
        {
            RETURN_REAL_ERR;
        }
        while ( (ch >= '0' && ch <= '9') || ch == '_' )
        {
            ch = *++s;
            len++;
        }
    }

    while ( IS_WHITESPACE(ch) )
    {
        ch = *++s;
    }
    if ( s != end )
    {
        RETURN_REAL_ERR;
    }
end:
    buf = (char *)malloc(len + 1);
    if ( buf == NULL )
    {
        NST_FAILED_ALLOCATION;
        return NULL;
    }
    s = buf;
    while ( len-- )
    {
        if ( (ch = *start++) != '_' )
        {
            *s++ = ch;
        }
    }
    *s = '\0';
    res = strtod(buf, NULL);
    free(buf);
    return nst_new_real(res);
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
