#include <stdlib.h>
#include <string.h>
#include "mem.h"

#undef malloc
#undef calloc
#undef realloc
#undef free

#if defined(_DEBUG) && !defined(NST_NO_ALLOC_COUNT)
#define COUNT_ALLOC
#endif

#ifdef COUNT_ALLOC
static i32 allocation_count = 0;
#endif

void *nst_raw_malloc(usize size)
{
#ifdef COUNT_ALLOC
    allocation_count++;
#endif
    return malloc(size);
}

void *nst_raw_calloc(usize count, usize size)
{
#ifdef COUNT_ALLOC
    allocation_count++;
#endif
    return calloc(count, size);
}

void *nst_raw_realloc(void *block, usize size)
{
    return realloc(block, size);
}

void *nst_malloc(usize count, usize size)
{
    return nst_raw_malloc(count * size);
}

void *nst_calloc(usize count, usize size, void *init_value)
{
    u8 *block = (u8 *)nst_raw_malloc(count * size);

    if ( block == NULL )
    {
        return NULL;
    }

    for ( usize i = 0; i < count; i++ )
    {
        if ( init_value == NULL )
        {
            memset(block + (i * size), 0, size);
        }
        else
        {
            memcpy(block + (i * size), (u8 *)init_value, size);
        }
    }
    return (void *)block;
}

void *nst_realloc(void *prev_block, usize new_count, usize size)
{
    return nst_raw_realloc(prev_block, new_count * size);
}

void *nst_crealloc(void *prev_block,
                   usize new_count,
                   usize size,
                   usize prev_count,
                   void *init_value)
{
    u8 *block = (u8 *)nst_raw_realloc(prev_block, new_count * size);
    if ( block == NULL )
    {
        return NULL;
    }

    if ( new_count <= prev_count )
    {
        return (void *)block;
    }

    for ( usize i = prev_count; i < new_count; i++ )
    {
        if ( init_value == NULL )
        {
            memset(block + (i * size), 0, size);
        }
        else
        {
            memcpy(block + (i * size), (u8 *)init_value, size);
        }
    }
    return (void *)block;
}

void nst_free(void *block)
{
#ifdef COUNT_ALLOC
    allocation_count--;
#endif
    free(block);
}
