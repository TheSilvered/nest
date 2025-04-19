#include <string.h>
#include "nest.h"

#define _Nst_DYN_ARRAY_GROWH_RATIO 1.5

bool Nst_da_init(Nst_DynArray *arr, usize unit_size, usize count)
{
    arr->unit_size = unit_size;
    arr->len = 0;

    if (count == 0)
        arr->data = NULL;
    else {
        arr->data = Nst_malloc(count, unit_size);
        if (arr->data == NULL) {
            arr->cap = 0;
            return false;
        }
    }

    arr->cap = count;
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

static void shrink(Nst_DynArray *arr)
{
    if (arr->len <= arr->cap / 4) {
        arr->data = Nst_realloc(
            arr->data,
            arr->cap / 2,
            arr->unit_size,
            arr->cap);
    }
}

bool Nst_da_pop(Nst_DynArray *arr, Nst_Destructor dstr)
{
    if (arr->len == 0)
        return false;
    if (dstr != NULL)
        dstr(Nst_da_get(arr, arr->len - 1));
    arr->len--;
    shrink(arr);

    return true;
}

bool Nst_da_pop_p(Nst_DynArray *arr, Nst_Destructor dstr)
{
    Nst_assert_c(arr->unit_size == sizeof(void *));
    if (arr->len == 0)
        return false;
    if (dstr != NULL)
        dstr(Nst_da_get_p(arr, arr->len - 1));
    arr->len--;
    shrink(arr);

    return true;
}

bool Nst_da_remove_swap(Nst_DynArray *arr, usize index, Nst_Destructor dstr)
{
    if (index >= arr->len)
        return false;
    if (dstr != NULL)
        dstr(Nst_da_get(arr, index));
    usize unit_size = arr->unit_size;
    memcpy(
        (u8 *)arr->data + index * unit_size,
        (u8 *)arr->data + arr->len * unit_size,
        unit_size);
    arr->len--;
    shrink(arr);
    return true;
}

bool Nst_da_remove_swap_p(Nst_DynArray *arr, usize index, Nst_Destructor dstr)
{
    Nst_assert_c(arr->unit_size == sizeof(void *));
    if (index >= arr->len)
        return false;
    if (dstr != NULL)
        dstr(Nst_da_get_p(arr, index));
    memcpy(
        (u8 *)arr->data + index * sizeof(void *),
        (u8 *)arr->data + arr->len * sizeof(void *),
        sizeof(void *));
    arr->len--;
    shrink(arr);
    return true;
}

bool Nst_da_remove_shift(Nst_DynArray *arr, usize index, Nst_Destructor dstr)
{
    if (index >= arr->len)
        return false;
    if (dstr != NULL)
        dstr(Nst_da_get(arr, index));
    usize unit_size = arr->unit_size;
    memmove(
        (u8 *)arr->data + index * unit_size,
        (u8 *)arr->data + (index + 1) * unit_size,
        (arr->len - index - 1) * unit_size);
    arr->len--;
    shrink(arr);
    return true;
}

bool Nst_da_remove_shift_p(Nst_DynArray *arr, usize index, Nst_Destructor dstr)
{
    Nst_assert_c(arr->unit_size == sizeof(void *));
    if (index >= arr->len)
        return false;
    if (dstr != NULL)
        dstr(Nst_da_get_p(arr, index));
    memmove(
        (u8 *)arr->data + index * sizeof(void *),
        (u8 *)arr->data + (index + 1) * sizeof(void *),
        (arr->len - index - 1) * sizeof(void *));
    arr->len--;
    shrink(arr);
    return true;
}

void *Nst_da_get(Nst_DynArray *arr, usize index)
{
    if (index >= arr->len)
        return NULL;
    return (void *)((u8 *)arr->data + (arr->unit_size * index));
}

void *Nst_da_get_p(Nst_DynArray *arr, usize index)
{
    Nst_assert_c(arr->unit_size == sizeof(void *));
    if (index >= arr->len)
        return NULL;
    return ((void **)arr->data)[index];
}

void Nst_da_clear(Nst_DynArray *arr, Nst_Destructor dstr)
{
    if (arr->data != NULL) {
        if (dstr != NULL) {
            for (usize i = 0; i < arr->len; i++)
                dstr(Nst_da_get(arr, i));
        }
        Nst_free(arr->data);
    }
    arr->data = NULL;
    arr->cap = 0;
    arr->len = 0;
}

void Nst_da_clear_p(Nst_DynArray *arr, Nst_Destructor dstr)
{
    if (arr->data != NULL) {
        if (dstr != NULL) {
            for (usize i = 0; i < arr->len; i++)
                dstr(Nst_da_get_p(arr, i));
        }
        Nst_free(arr->data);
    }
    arr->data = NULL;
    arr->cap = 0;
    arr->len = 0;
}
