#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "str.h"

Nst_Obj *new_str_obj(Nst_string *str)
{
    return make_obj(str, nst_t_str, destroy_string);
}

Nst_string *new_string_raw(char *val, bool allocated)
{
    Nst_string *str = malloc(sizeof(Nst_string));
    if ( str == NULL ) return NULL;

    str->allocated = allocated;
    str->len = strlen(val);
    str->value = val;

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

/* void str_set_raw(Nst_string *str, char *val, bool allocated)
{
    if ( str == NULL ) return;
    str->value = val;
    str->allocated = allocated;
    str->len = strlen(val);
}

void str_set(Nst_string *str, char *val, size_t len, bool allocated)
{
    if ( str == NULL ) return;
    str->value = val;
    str->allocated = allocated;
    str->len = len;
} */

void destroy_string(Nst_string *str)
{
    if ( str == NULL ) return;
    if ( str->allocated )
        free(str->value);
    free(str);
}
