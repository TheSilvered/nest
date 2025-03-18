#include <stdlib.h>
#include <string.h>
#include "nest.h"

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
        printf("    Size: %4zi bytes, Pointer: %p (%p)\n",
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
