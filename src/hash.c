#include <string.h>
#include "hash.h"
#include "nst_types.h"
#include "var_table.h"
#include "global_consts.h"

#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME 0x00000100000001B3
#define LOWER_HALF 0xffffffff

static int32_t hash_str(Nst_StrObj *str);
static int32_t hash_int(Nst_IntObj *num);
static int32_t hash_byte(Nst_ByteObj *byte);
static int32_t hash_ptr(void *ptr);

int32_t nst_obj_hash(Nst_Obj *obj)
{
    // Hashing floats can lead to unpredictable behaviour
    // caused by floating point imprecision. Because of this
    // it's not natively supported to hash floats

    if ( obj->hash != -1 )
    {
        return obj->hash;
    }

    int32_t hash;
    if ( obj->type == nst_t.Type ||
         obj->type == nst_t.Null ||
         obj->type == nst_t.Bool )
    {
        hash = hash_ptr(obj);
    }
    else if ( obj->type == nst_t.Str )
    {
        hash = hash_str(STR(obj));
    }
    else if ( obj->type == nst_t.Int )
    {
        hash = hash_int((Nst_IntObj*)obj);
    }
    else if ( obj->type == nst_t.Byte )
    {
        hash = hash_byte((Nst_ByteObj*)obj);
    }
    else
    {
        return -1;
    }

    obj->hash = hash;
    return hash;
}

static int32_t hash_ptr(void *ptr)
{
    // taken from https://github.com/python/cpython/blob/main/Python/pyhash.c
    size_t x = (size_t)ptr;
    x = (x >> 4) | (x << (8 * sizeof(void *) - 4));

    return (int32_t)x == -1 ? -2 : (int32_t)x;
}

static int32_t hash_str(Nst_StrObj *str)
{
    // taken from https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    int64_t hash = FNV_OFFSET_BASIS;
    char *s = str->value;

    while ( *s )
    {
        hash ^= *s++;
        hash *= FNV_PRIME;
    }

    return (int32_t)((hash >> 32) ^ (hash & LOWER_HALF));
}

static int32_t hash_int(Nst_IntObj *num)
{
    return num->value == -1 ? -2 : (int32_t)(num->value);
}

static int32_t hash_byte(Nst_ByteObj *byte)
{
    return (int32_t)(byte->value);
}
