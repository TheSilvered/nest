#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "lib_import.h"

#ifdef Nst_COUNT_ALLOC
static i32 allocation_count = 0;

void *Nst_raw_malloc(usize size)
{
    void *ptr = malloc(size);
    if (ptr != NULL)
        allocation_count++;
    return ptr;
}

void *Nst_raw_calloc(usize count, usize size)
{
    void *ptr = calloc(count, size);
    if (ptr != NULL)
        allocation_count++;
    return ptr;
}

void *Nst_raw_realloc(void *block, usize size)
{
    return realloc(block, size);
}

void Nst_free(void *block)
{
    if (block != NULL)
        allocation_count--;
    free(block);
}

#endif // !COUNT_ALLOC

void *Nst_malloc(usize count, usize size)
{
    void *ptr = Nst_raw_malloc(count * size);
    if (ptr == NULL)
        Nst_failed_allocation();
    return ptr;
}

void *Nst_calloc(usize count, usize size, void *init_value)
{
    u8 *block = (u8 *)Nst_raw_malloc(count * size);

    if (block == NULL) {
        Nst_failed_allocation();
        return NULL;
    }
    if (init_value == NULL) {
        memset(block, 0, count * size);
        return (void *)block;
    }

    for (usize i = 0; i < count; i++)
        memcpy(block + (i * size), (u8 *)init_value, size);
    return (void *)block;
}

void *Nst_realloc(void *prev_block, usize new_count, usize size,
                  usize prev_count)
{
    if (new_count == prev_count)
        return prev_block;

    void *block = Nst_raw_realloc(prev_block, new_count * size);
    if (block == NULL && new_count > prev_count && size != 0) {
        Nst_failed_allocation();
        return NULL;
    }
    return block || new_count == 0 || size == 0 ? block : prev_block;
}

void *Nst_crealloc(void *prev_block, usize new_count, usize size,
                   usize prev_count, void *init_value)
{
    if (new_count == prev_count)
        return prev_block;

    u8 *block = (u8 *)Nst_raw_realloc(prev_block, new_count * size);
    if (new_count == 0 || size == 0)
        return NULL;
    else if (new_count <= prev_count)
        return block ? (void *)block : prev_block;

    if (block == NULL) {
        Nst_failed_allocation();
        return NULL;
    }

    if (init_value == NULL) {
        memset(block + (prev_count * size), 0, (new_count - prev_count) * size);
        return (void *)block;
    } else if (size == 1) {
        u8 value = *(u8 *)init_value;
        memset(
            block + prev_count,
            value,
            new_count - prev_count);
        return (void *)block;
    }

    for (usize i = prev_count; i < new_count; i++)
        memcpy(block + (i * size), (u8 *)init_value, size);
    return (void *)block;
}

bool Nst_sbuffer_init(Nst_SizedBuffer *buf, usize unit_size, usize count)
{
    void *data = Nst_malloc(count, unit_size);
    if (data == NULL)
        return false;

    buf->data = data;
    buf->cap = count;
    buf->unit_size = unit_size;
    buf->len = 0;
    return true;
}

bool Nst_sbuffer_expand_by(Nst_SizedBuffer *buf, usize amount)
{
    return Nst_sbuffer_expand_to(buf, buf->len + amount);
}

bool Nst_sbuffer_expand_to(Nst_SizedBuffer *buf, usize count)
{
    if (buf->cap >= count)
        return true;

    usize new_size = (usize)(count * 1.5);
    void *new_data = Nst_realloc(buf->data, new_size, buf->unit_size, 0);
    if (new_data == NULL)
        return false;

    buf->data = new_data;
    buf->cap = new_size;
    return true;
}

void Nst_sbuffer_fit(Nst_SizedBuffer *buf)
{
    if ((buf->cap - buf->len) * buf->unit_size < sizeof(usize))
        return;

    buf->data = Nst_realloc(buf->data, buf->len, buf->unit_size, buf->cap);
    buf->cap = buf->len;
}

bool Nst_sbuffer_append(Nst_SizedBuffer *buf, void *element)
{
    if (!Nst_sbuffer_expand_by(buf, 1))
        return false;

    void *data_end = (void *)((i8 *)buf->data + (buf->len * buf->unit_size));
    memcpy(data_end, element, buf->unit_size);
    buf->len++;
    return true;
}

void Nst_sbuffer_destroy(Nst_SizedBuffer *buf)
{
    if (buf->data != NULL)
        Nst_free(buf->data);
    buf->data = NULL;
    buf->cap = 0;
    buf->len = 0;
    buf->unit_size = 0;
}

bool Nst_buffer_init(Nst_Buffer *buf, usize initial_size)
{
    if (!Nst_sbuffer_init((Nst_SizedBuffer *)buf, sizeof(i8), initial_size))
        return false;

    if (initial_size > 0)
        buf->data[0] = '\0';

    return true;
}

bool Nst_buffer_expand_by(Nst_Buffer *buf, usize amount)
{
    return Nst_sbuffer_expand_to((Nst_SizedBuffer *)buf, buf->len + amount + 1);
}

bool Nst_buffer_expand_to(Nst_Buffer *buf, usize size)
{
    return Nst_sbuffer_expand_to((Nst_SizedBuffer *)buf, size + 1);
}

void Nst_buffer_fit(Nst_Buffer *buf)
{
    if ((buf->cap - buf->len) * buf->unit_size < sizeof(usize))
        return;

    buf->data = Nst_realloc(buf->data, buf->len + 1, buf->unit_size, buf->cap);
    buf->cap = buf->len + 1;
}

bool Nst_buffer_append(Nst_Buffer *buf, Nst_StrObj *str)
{
    usize str_len = str->len;
    if (!Nst_buffer_expand_by(buf, str_len))
        return false;

    memcpy(buf->data + buf->len, str->value, str_len + 1);
    buf->len += str_len;
    return true;
}

bool Nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str)
{
    if (*str == 0)
        return true;

    usize str_len = strlen(str);
    if (!Nst_buffer_expand_by(buf, str_len))
        return false;

    memcpy(buf->data + buf->len, str, str_len + 1);
    buf->len += str_len;
    return true;
}

bool Nst_buffer_append_char(Nst_Buffer *buf, i8 ch)
{
    if (!Nst_buffer_expand_by(buf, 1))
        return false;

    buf->data[buf->len++] = ch;
    buf->data[buf->len] = '\0';
    return true;
}

Nst_StrObj *Nst_buffer_to_string(Nst_Buffer *buf)
{
    Nst_buffer_fit(buf);
    Nst_StrObj *str = STR(Nst_string_new(buf->data, buf->len, true));
    if (str == NULL)
        Nst_free(buf->data);
    buf->data = NULL;
    buf->cap = 0;
    buf->len = 0;
    return str;
}

void Nst_buffer_destroy(Nst_Buffer *buf)
{
    Nst_sbuffer_destroy((Nst_SizedBuffer *)buf);
}
