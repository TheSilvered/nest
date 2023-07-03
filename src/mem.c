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

void *nst_malloc(usize count, usize size)
{
    void *ptr = nst_raw_malloc(count * size);
    if ( ptr == NULL )
    {
        nst_failed_allocation();
    }
    return ptr;
}

void *nst_calloc(usize count, usize size, void *init_value)
{
    u8 *block = (u8 *)nst_raw_malloc(count * size);

    if ( block == NULL )
    {
        nst_failed_allocation();
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
                  usize prev_count)
{
    if ( new_count == prev_count )
    {
        return prev_block;
    }

    void *block = nst_raw_realloc(prev_block, new_count * size);
    if ( block == NULL && new_count > prev_count )
    {
        nst_failed_allocation();
        return NULL;
    }
    return block ? block : prev_block;
}

void *nst_crealloc(void *prev_block,
                   usize new_count,
                   usize size,
                   usize prev_count,
                   void *init_value)
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
        nst_failed_allocation();
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

bool nst_sbuffer_init(Nst_SizedBuffer *buf, usize unit_size, usize count)
{
    void *data = nst_malloc(count, unit_size);
    if ( data == NULL )
    {
        return false;
    }

    buf->data = data;
    buf->size = count;
    buf->unit_size = unit_size;
    buf->len = 0;
    return true;
}

bool nst_sbuffer_expand_by(Nst_SizedBuffer *buf, usize amount)
{
    return nst_sbuffer_expand_to(buf, buf->len + amount + 1);
}

bool nst_sbuffer_expand_to(Nst_SizedBuffer *buf, usize count)
{
    if ( buf->size > count )
    {
        return true;
    }

    usize new_size = (usize)(count * 1.5);
    void *new_data = nst_realloc(buf->data, new_size, buf->unit_size, 0);
    if ( new_data == NULL )
    {
        return false;
    }
    buf->data = new_data;
    buf->size = new_size;
    return true;
}

void nst_sbuffer_fit(Nst_SizedBuffer *buf)
{
    if ( (buf->size - buf->len) * buf->unit_size < 20 )
    {
        return;
    }

    buf->data = nst_realloc(buf->data, buf->len + 1, buf->unit_size, buf->size);
    buf->size = buf->len + 1;
}

bool nst_sbuffer_append(Nst_SizedBuffer *buf, void *element)
{
    if ( !nst_sbuffer_expand_by(buf, 1) )
    {
        return false;
    }
    memcpy((i8 *)buf->data + buf->size, element, buf->unit_size);
    return true;
}

void nst_sbuffer_destroy(Nst_SizedBuffer *buf)
{
    if ( buf->data != NULL )
    {
        nst_free(buf->data);
    }
    buf->data = NULL;
    buf->size = 0;
    buf->len = 0;
    buf->unit_size = 0;
}

bool nst_buffer_init(Nst_Buffer *buf, usize initial_size)
{
    if ( !nst_sbuffer_init((Nst_SizedBuffer *)buf, sizeof(i8), initial_size) )
    {
        return false;
    }

    if ( initial_size > 0 )
    {
        buf->data[0] = '\0';
    }
    return true;
}

bool nst_buffer_expand_by(Nst_Buffer *buf, usize amount)
{
    return nst_sbuffer_expand_to((Nst_SizedBuffer *)buf, buf->len + amount + 1);
}

bool nst_buffer_expand_to(Nst_Buffer *buf, usize size)
{
    return nst_sbuffer_expand_to((Nst_SizedBuffer *)buf, size);
}

void nst_buffer_fit(Nst_Buffer *buf)
{
    nst_sbuffer_fit((Nst_SizedBuffer *)buf);
}

bool nst_buffer_append(Nst_Buffer *buf, Nst_StrObj *str)
{
    usize str_len = str->len;
    if ( !nst_buffer_expand_by(buf, str_len) )
    {
        return false;
    }
    memcpy(buf->data + buf->len, str->value, str_len + 1);
    buf->len += str_len;
    return true;
}

bool nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str)
{
    usize str_len = strlen(str);
    if ( !nst_buffer_expand_by(buf, str_len) )
    {
        return false;
    }
    memcpy(buf->data + buf->len, str, str_len + 1);
    buf->len += str_len;
    return true;
}

bool nst_buffer_append_char(Nst_Buffer *buf, i8 ch)
{
    if ( !nst_buffer_expand_by(buf, 1) )
    {
        return false;
    }
    buf->data[buf->len++] = ch;
    buf->data[buf->len] = '\0';
    return true;
}

Nst_StrObj *nst_buffer_to_string(Nst_Buffer *buf)
{
    nst_buffer_fit(buf);
    Nst_StrObj *str = STR(nst_string_new(buf->data, buf->len, true));
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
    nst_sbuffer_destroy((Nst_SizedBuffer *)buf);
}
