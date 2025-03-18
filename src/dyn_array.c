#include <string.h>
#include "nest.h"

#define _Nst_DYN_ARRAY_GROWH_RATIO 1.5

bool Nst_da_init(Nst_DynArray *arr, usize unit_size, usize count)
{
    if (count == 0)
        arr->data = NULL;
    else {
        arr->data = Nst_malloc(count, unit_size);
        if (arr->data == NULL)
            return false;
    }

    arr->cap = count;
    arr->unit_size = unit_size;
    arr->len = 0;
    return true;
}

bool Nst_da_init_copy(Nst_DynArray *src, Nst_DynArray *dst)
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

bool Nst_da_reserve(Nst_DynArray *arr, usize amount)
{
    if (arr->len + amount <= arr->cap)
        return true;

    usize new_size = (usize)((double)(arr->len + amount) * 1.5);
    void *new_data = Nst_realloc(arr->data, new_size, arr->unit_size, 0);
    if (new_data == NULL)
        return false;

    arr->data = new_data;
    arr->cap = new_size;
    return true;
}

bool Nst_da_append(Nst_DynArray *arr, void *element)
{
    if (!Nst_da_reserve(arr, 1))
        return false;

    void *data_end = (void *)((u8 *)arr->data + (arr->len * arr->unit_size));
    memcpy(data_end, element, arr->unit_size);
    arr->len++;
    return true;
}

bool Nst_da_pop(Nst_DynArray *arr)
{
    if (arr->len == 0)
        return false;
    arr->len--;
    if (arr->len <= arr->cap / 4) {
        arr->data = Nst_realloc(
            arr->data,
            arr->cap / 2,
            arr->unit_size,
            arr->cap);
    }

    return true;
}

bool Nst_da_remove_swap(Nst_DynArray *arr, usize index)
{
    if (index >= arr->len)
        return false;
    usize unit_size = arr->unit_size;
    memcpy(
        (u8 *)arr->data + index * unit_size,
        (u8 *)arr->data + arr->len * unit_size,
        unit_size);
    return true;
}

bool Nst_da_remove_shift(Nst_DynArray *arr, usize index)
{
    if (index >= arr->len)
        return false;
    usize unit_size = arr->unit_size;
    memmove(
        (u8 *)arr->data + index * unit_size,
        (u8 *)arr->data + (index + 1) * unit_size,
        (arr->len - index - 1) * unit_size);
    return true;
}

void *Nst_da_at(Nst_DynArray *arr, usize index)
{
    if (index >= arr->len)
        return NULL;
    return (void *)((u8 *)arr->data + (arr->unit_size * index));
}

void Nst_da_clear(Nst_DynArray *arr)
{
    if (arr->data != NULL)
        Nst_free(arr->data);
    arr->data = NULL;
    arr->cap = 0;
    arr->len = 0;
}
