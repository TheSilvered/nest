#include <string.h>
#include "hash.h"
#include "nst_types.h"
#include "var_table.h"

size_t hash_str(Nst_string *str);
size_t hash_int(Nst_int *num);
size_t hash_ptr(void *ptr);
static inline size_t murmur_32_scramble(uint32_t k);

size_t hash_obj(Nst_Obj *obj)
{
    // Hashing floats can lead to unpredictable behaviour
    // caused by floating point imprecision. Because of this
    // it's not natively supported to hash floats
    if ( obj->type == nst_t_type ||
         obj->type == nst_t_null ||
         obj->type == nst_t_bool )
        return hash_ptr(obj);
    else if ( obj->type == nst_t_str )
        return hash_str(obj->value);
    else if ( obj->type == nst_t_int )
        return hash_int(obj->value);
    else
        return -1;
}

size_t hash_ptr(void *ptr)
{
    // taken from https://github.com/python/cpython/blob/main/Python/pyhash.c
    size_t x = (size_t)ptr;
    x = (x >> 4) | (x << (8 * sizeof(void *) - 4));

    if ( x == -1 ) x = -2;
    return x;
}

static inline size_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

size_t hash_str(Nst_string *str)
{
    // MurmurHash 3, implementation taken from
    // https://en.wikipedia.org/wiki/MurmurHash

    const uint8_t *key = str->value;
    size_t len = str->len;

    register uint32_t h = 0xcd8837c; // seed
    uint32_t k;

    for ( size_t i = len >> 2; i; i-- ) {
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }

    k = 0;
    for ( size_t i = len & 3; i; i-- ) {
        k <<= 8;
        k |= key[i - 1];
    }

    h ^= murmur_32_scramble(k);

    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    if ( h == -1 ) return -2;
    return h;
}

size_t hash_int(Nst_int *num)
{
    if ( *num == -1 )
        return -2;
    return *num;
}
