#include <string.h>
#include "hash.h"
#include "global_consts.h"

#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME 0x00000100000001B3
#define LOWER_HALF 0xffffffff

static i32 hash_str(Nst_Obj *str);
static i32 hash_int(Nst_IntObj *num);
static i32 hash_byte(Nst_ByteObj *byte);
static i32 hash_ptr(void *ptr);

i32 Nst_obj_hash(Nst_Obj *obj)
{
    // Hashing floats can lead to unpredictable behaviour caused by floating
    // point imprecision. Because of this it's not natively supported to hash
    // floats

    if (obj->hash != -1)
        return obj->hash;

    i32 hash;
    if (obj->type == Nst_t.Type
        || obj->type == Nst_t.Null
        || obj->type == Nst_t.Bool)
    {
        hash = hash_ptr(obj);
    }
    else if (obj->type == Nst_t.Str)
        hash = hash_str(obj);
    else if (obj->type == Nst_t.Int)
        hash = hash_int((Nst_IntObj *)obj);
    else if (obj->type == Nst_t.Byte)
        hash = hash_byte((Nst_ByteObj *)obj);
    else
        return -1;

    obj->hash = hash;
    return hash;
}

static i32 hash_ptr(void *ptr)
{
    // taken from https://github.com/python/cpython/blob/main/Python/pyhash.c
    usize x = (usize)ptr;
    x = (x >> 4) | (x << (8 * sizeof(void *) - 4));

    return (i32)x == -1 ? -2 : (i32)x;
}

static i32 hash_str(Nst_Obj *str)
{
    // taken from https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    i64 hash = FNV_OFFSET_BASIS;
    i8 *s = Nst_str_value(str);
    i8 *s_end = s + Nst_str_len(str);

    while (s != s_end) {
        hash ^= *s++;
        hash *= FNV_PRIME;
    }

    return (i32)((hash >> 32) ^ (hash & LOWER_HALF));
}

static i32 hash_int(Nst_IntObj *num)
{
    return num->value == -1 ? -2 : (i32)(num->value);
}

static i32 hash_byte(Nst_ByteObj *byte)
{
    return (i32)(byte->value);
}
