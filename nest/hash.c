#include <string.h>
#include "hash.h"
#include "nst_types.h"
#include "var_table.h"

#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME 0x00000100000001B3
#define LOWER_HALF 0xffffffff

int32_t hash_str(Nst_StrObj *str);
int32_t hash_int(Nst_IntObj *num);
int32_t hash_ptr(void *ptr);

int32_t nst_hash_obj(Nst_Obj *obj)
{
    // Hashing floats can lead to unpredictable behaviour
    // caused by floating point imprecision. Because of this
    // it's not natively supported to hash floats

    int32_t hash;
    if ( obj->type == nst_t_type ||
         obj->type == nst_t_null ||
         obj->type == nst_t_bool )
        hash = hash_ptr(obj);
    else if ( obj->type == nst_t_str )
        hash = hash_str(AS_STR(obj));
    else if ( obj->type == nst_t_int )
        hash = hash_int((Nst_IntObj *)obj);
    else
        return -1;

    obj->hash = hash;
    return hash;
}

int32_t hash_ptr(void *ptr)
{
    // taken from https://github.com/python/cpython/blob/main/Python/pyhash.c
    size_t x = (size_t)ptr;
    x = (x >> 4) | (x << (8 * sizeof(void *) - 4));

    if ( x == -1 ) x = -2;
    return (int32_t)x;
}

int32_t hash_str(Nst_StrObj *str)
{
    // taken from https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    register int64_t hash = FNV_OFFSET_BASIS;
    register char *s = str->value;

    while ( *s )
    {
        hash ^= *s++;
        hash *= FNV_PRIME;
    }

    return (int32_t)((hash >> 32) ^ (hash & LOWER_HALF));
}

int32_t hash_int(Nst_IntObj *num)
{
    if ( num->value == -1 )
        return -2;
    return (int32_t)(num->value);
}
