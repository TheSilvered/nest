#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "sequence.h" // _Nst_VECTOR_GROWTH_RATIO, _Nst_VECTOR_MIN_CAP
#include "global_consts.h"
#include "interpreter.h"

#ifdef Nst_DBG_COUNT_ALLOC

typedef struct AllocHeader {
    usize size;
    struct AllocHeader *next;
    struct AllocHeader *prev;
} AllocHeader;

static i32 allocation_count = 0;
AllocHeader *allocs_head = NULL;

#ifdef Nst_MSVC
#pragma warning(push)
#pragma warning(disable: 4995)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

void Nst_log_alloc_count(void)
{
    printf("\nAllocation count: %" PRIi32 "\n", allocation_count);
}

void Nst_log_alloc_info(void)
{
    if (allocs_head == NULL) {
        printf("\nAllocation info: no allocations.\n");
        return;
    }
    printf("\nAllocation info:\n");
    AllocHeader *head = allocs_head;
    while (head != NULL) {
        printf("    Size: %zi bytes, Pointer: %p (%p)\n",
            head->size,
            head + 1,
            head);
        head = head->next;
    }
}

void add_header(AllocHeader *header)
{
    if (allocs_head == NULL)
        allocs_head = header;
    else {
        allocs_head->prev = header;
        header->next = allocs_head;
        allocs_head = header;
    }
}

void remove_header(AllocHeader *header)
{
    if (header->next != NULL)
        header->next->prev = header->prev;
    if (header->prev != NULL)
        header->prev->next = header->next;
    if (allocs_head == header)
        allocs_head = header->next;
    header->next = NULL;
    header->prev = NULL;
}

void *Nst_raw_malloc(usize size)
{
    AllocHeader *header = malloc(size + sizeof(AllocHeader));
    if (header == NULL)
        return NULL;

    allocation_count++;

    header->size = size;
    header->prev = NULL;
    header->next = NULL;

    add_header(header);

    return (void *)(header + 1);
}

void *Nst_raw_calloc(usize count, usize size)
{
    void *ptr = Nst_raw_malloc(count * size);
    if (ptr == NULL)
        return NULL;
    memset(ptr, 0, count * size);
    return ptr;
}

void *Nst_raw_realloc(void *block, usize size)
{
    AllocHeader *header;

    if (block == NULL) {
        allocation_count++;
        header = NULL;
    } else {
        header = (AllocHeader *)block - 1;
        remove_header(header);
    }

    if (size == 0) {
        if (block != NULL)
            allocation_count--;
        return realloc(header, size);
    }

    AllocHeader *new_header = realloc(header, size + sizeof(AllocHeader));
    if (new_header == NULL) {
        add_header(header);
        return NULL;
    }
    new_header->size = size;
    new_header->next = NULL;
    new_header->prev = NULL;
    add_header(new_header);
    return (void *)(new_header + 1);
}

void Nst_raw_free(void *block)
{
    if (block == NULL)
        return;
    allocation_count--;
    AllocHeader *header = (AllocHeader *)block - 1;
    remove_header(header);
    free(header);
}

#ifdef Nst_MSVC
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

#endif // !COUNT_ALLOC

void *Nst_malloc(usize count, usize size)
{
    void *ptr = Nst_raw_malloc(count * size);
    if (ptr == NULL)
        Nst_error_failed_alloc();
    return ptr;
}

void *Nst_calloc(usize count, usize size, void *init_value)
{
    u8 *block = (u8 *)Nst_raw_malloc(count * size);

    if (block == NULL) {
        Nst_error_failed_alloc();
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
        Nst_error_failed_alloc();
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
        Nst_error_failed_alloc();
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

void Nst_memset(void *block, usize size, usize count, void *value)
{
    if (value == NULL) {
        memset(block, 0, size * count);
        return;
    }
    for (usize i = 0; i < count; i++)
        memcpy((u8 *)block + i * size, value, size);
}

bool Nst_sbuffer_init(Nst_SBuffer *buf, usize unit_size, usize count)
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

bool Nst_sbuffer_expand_by(Nst_SBuffer *buf, usize amount)
{
    return Nst_sbuffer_expand_to(buf, buf->len + amount);
}

bool Nst_sbuffer_expand_to(Nst_SBuffer *buf, usize count)
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

void Nst_sbuffer_fit(Nst_SBuffer *buf)
{
    usize len = buf->len;
    if (len == 0)
        len = 1;

    buf->data = Nst_realloc(buf->data, len, buf->unit_size, buf->cap);
    buf->cap = len;
}

bool Nst_sbuffer_append(Nst_SBuffer *buf, void *element)
{
    if (!Nst_sbuffer_expand_by(buf, 1))
        return false;

    void *data_end = (void *)((u8 *)buf->data + (buf->len * buf->unit_size));
    memcpy(data_end, element, buf->unit_size);
    buf->len++;
    return true;
}

bool Nst_sbuffer_pop(Nst_SBuffer *buf)
{
    if (buf->len == 0)
        return false;
    buf->len--;
    return true;
}

void *Nst_sbuffer_at(Nst_SBuffer *buf, usize index)
{
    if (index >= buf->len)
        return NULL;
    return (void *)((u8 *)buf->data + (buf->unit_size * index));
}

void Nst_sbuffer_shrink_auto(Nst_SBuffer *buf)
{
    if (buf->cap >> 2 < buf->len)
        return;
    usize new_cap = (usize)(buf->cap / _Nst_VECTOR_GROWTH_RATIO);
    if (new_cap < _Nst_VECTOR_MIN_CAP)
        return;
    buf->data = Nst_realloc(buf->data, new_cap, buf->unit_size, buf->cap);
    buf->cap = new_cap;
}

bool Nst_sbuffer_copy(Nst_SBuffer *src, Nst_SBuffer *dst)
{
    void *new_data = Nst_calloc(1, src->len, src->data);
    if (new_data == NULL)
        return false;

    dst->cap = src->len;
    dst->len = src->len;
    dst->unit_size = src->unit_size;
    dst->data = new_data;
    return true;
}

void Nst_sbuffer_destroy(Nst_SBuffer *buf)
{
    if (buf->data != NULL)
        Nst_free(buf->data);
    buf->data = NULL;
    buf->cap = 0;
    buf->len = 0;
    buf->unit_size = 0;
}
