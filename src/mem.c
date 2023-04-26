#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "lib_import.h"

#if defined(_DEBUG) && !defined(NST_NO_ALLOC_COUNT)
#define COUNT_ALLOC
#endif

#ifdef COUNT_ALLOC
static i32 allocation_count = 0;
#endif

void *nst_raw_malloc(usize size)
{
#ifdef COUNT_ALLOC
    void *ptr = malloc(size);
    if ( ptr != NULL )
    {
        allocation_count++;
    }
    return ptr;
#else
    return malloc(size);
#endif
}

void *nst_raw_calloc(usize count, usize size)
{
#ifdef COUNT_ALLOC
    void *ptr = calloc(count, size);
    if ( ptr != NULL )
    {
        allocation_count++;
    }
    return ptr;
#else
    return calloc(count, size);
#endif
}

void *nst_raw_realloc(void *block, usize size)
{
    return realloc(block, size);
}

void *nst_malloc(usize count, usize size, Nst_OpErr *err)
{
    void *ptr = nst_raw_malloc(count * size);
    if ( ptr == NULL )
    {
        NST_FAILED_ALLOCATION;
    }
    return ptr;
}

void *nst_calloc(usize count, usize size, void *init_value, Nst_OpErr *err)
{
    u8 *block = (u8 *)nst_raw_malloc(count * size);

    if ( block == NULL )
    {
        NST_FAILED_ALLOCATION;
        return NULL;
    }
    if ( init_value == NULL )
    {
        memset(block, 0, count * size);
        return (void *)block;
    }

    for ( usize i = 0; i < count; i++ )
    {
        memcpy(block + (i * size), (u8 *)init_value, size);
    }
    return (void *)block;
}

void *nst_realloc(void *prev_block,
                  usize new_count,
                  usize size,
                  usize prev_count,
                  Nst_OpErr *err)
{
    if ( new_count == prev_count )
    {
        return prev_block;
    }

    void *block = nst_raw_realloc(prev_block, new_count * size);
    if ( block == NULL && new_count > prev_count )
    {
        NST_FAILED_ALLOCATION;
        return NULL;
    }
    return block ? block : prev_block;
}

void *nst_crealloc(void *prev_block,
                   usize new_count,
                   usize size,
                   usize prev_count,
                   void *init_value,
                   Nst_OpErr *err)
{
    if ( new_count == prev_count )
    {
        return prev_block;
    }

    u8 *block = (u8 *)nst_raw_realloc(prev_block, new_count * size);
    if ( new_count <= prev_count )
    {
        return block ? (void *)block : prev_block;
    }

    if ( block == NULL )
    {
        NST_FAILED_ALLOCATION;
        return NULL;
    }

    if ( init_value == NULL )
    {
        memset(block + (prev_count * size), 0, (new_count - prev_count) * size);
        return (void *)block;
    }

    for ( usize i = prev_count; i < new_count; i++ )
    {
        memcpy(block + (i * size), (u8 *)init_value, size);
    }
    return (void *)block;
}

void nst_free(void *block)
{
#ifdef COUNT_ALLOC
    if ( block != NULL )
    {
        allocation_count--;
    }
#endif
    free(block);
}

bool nst_buffer_init(Nst_Buffer *buf, usize initial_size, Nst_OpErr *err)
{
    i8 *data = nst_malloc(initial_size, sizeof(i8), err);
    if ( data == NULL )
    {
        return false;
    }
    data[0] = '\0';
    buf->data = data;
    buf->size = initial_size;
    buf->len = 0;
    return true;
}

bool nst_buffer_expand_by(Nst_Buffer *buf, usize amount, Nst_OpErr *err)
{
    return nst_buffer_expand_to(buf, buf->len + amount + 1, err);
}

bool nst_buffer_expand_to(Nst_Buffer *buf, usize size, Nst_OpErr *err)
{
    if ( buf->size > size )
    {
        return true;
    }

    usize new_size = (usize)(size * 1.5);
    i8 *new_data = (i8 *)nst_realloc(buf->data, new_size, sizeof(i8), 0, err);
    if ( new_data == NULL )
    {
        return false;
    }
    buf->data = new_data;
    buf->size = new_size;
    return true;
}

void nst_buffer_fit(Nst_Buffer *buf)
{
    buf->data = nst_realloc(buf->data, buf->len + 1, sizeof(i8), buf->size, NULL);
    buf->size = buf->len + 1;
}

bool nst_buffer_append(Nst_Buffer *buf, Nst_StrObj *str, Nst_OpErr *err)
{
    usize str_len = str->len;
    if ( !nst_buffer_expand_by(buf, str_len, err) )
    {
        return false;
    }
    memcpy(buf->data + buf->len, str->value, str_len + 1);
    buf->len += str_len;
    return true;
}

bool nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str, Nst_OpErr *err)
{
    usize str_len = strlen(str);
    if ( !nst_buffer_expand_by(buf, str_len, err) )
    {
        return false;
    }
    memcpy(buf->data + buf->len, str, str_len + 1);
    buf->len += str_len;
    return true;
}

bool nst_buffer_append_char(Nst_Buffer *buf, i8 ch, Nst_OpErr *err)
{
    if ( !nst_buffer_expand_by(buf, 1, err) )
    {
        return false;
    }
    buf->data[buf->len++] = ch;
    buf->data[buf->len] = '\0';
    return true;
}

Nst_StrObj *nst_buffer_to_string(Nst_Buffer *buf, Nst_OpErr *err)
{
    nst_buffer_fit(buf);
    Nst_StrObj *str = STR(nst_string_new(buf->data, buf->len, true, err));
    if ( str == NULL )
    {
        nst_free(buf->data);
    }
    buf->data = NULL;
    buf->size = 0;
    buf->len = 0;
    return str;
}

void nst_buffer_destroy(Nst_Buffer *buf)
{
    if ( buf->data != NULL )
    {
        nst_free(buf->data);
    }
    buf->data = NULL;
    buf->size = 0;
    buf->len = 0;
}
