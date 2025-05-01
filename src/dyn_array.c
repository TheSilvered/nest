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
        arr->cap /= 2;
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

void *Nst_da_get(Nst_DynArray *arr, usize index)
{
    if (index >= arr->len)
        return NULL;
    return (void *)((u8 *)arr->data + (arr->unit_size * index));
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

void Nst_da_set(Nst_DynArray *arr, usize index, void *element,
                Nst_Destructor dstr)
{
    if (index >= arr->len)
        return;
    void *arr_element = (void *)((u8 *)arr->data + (index * arr->unit_size));
    if (dstr != NULL)
        dstr(arr_element);
    memcpy(arr_element, element, arr->unit_size);
}

bool Nst_pa_init(Nst_PtrArray *arr, usize reserve)
{
    arr->len = 0;

    if (reserve == 0)
        arr->data = NULL;
    else {
        arr->data = Nst_malloc(reserve, sizeof(void *));
        if (arr->data == NULL) {
            arr->cap = 0;
            return false;
        }
    }

    arr->cap = reserve;
    return true;
}

bool Nst_pa_init_copy(Nst_PtrArray *src, Nst_PtrArray *dst)
{
    void *new_data = Nst_calloc(1, src->len, src->data);
    if (new_data == NULL)
        return false;

    dst->cap = src->len;
    dst->len = src->len;
    dst->data = new_data;
    return true;
}

bool Nst_pa_reserve(Nst_PtrArray *arr, usize amount)
{
    if (arr->len + amount <= arr->cap)
        return true;

    usize new_size = (usize)((double)(arr->len + amount) * 1.5);
    void *new_data = Nst_realloc(arr->data, new_size, sizeof(void *), 0);
    if (new_data == NULL)
        return false;

    arr->data = new_data;
    arr->cap = new_size;
    return true;
}

bool Nst_pa_append(Nst_PtrArray *arr, void *element)
{
    if (!Nst_pa_reserve(arr, 1))
        return false;

    arr->data[arr->len++] = element;
    return true;
}

static void shrink_p(Nst_PtrArray *arr)
{
    if (arr->len <= arr->cap / 4) {
        arr->data = Nst_realloc(
            arr->data,
            arr->cap / 2,
            sizeof(void *),
            arr->cap);
        arr->cap /= 2;
    }
}

bool Nst_pa_pop(Nst_PtrArray *arr, Nst_Destructor dstr)
{
    if (arr->len == 0)
        return false;
    if (dstr != NULL)
        dstr(arr->data[arr->len - 1]);
    arr->len--;
    shrink_p(arr);

    return true;
}

bool Nst_pa_remove_swap(Nst_PtrArray *arr, usize index, Nst_Destructor dstr)
{
    if (index >= arr->len)
        return false;
    if (dstr != NULL)
        dstr(arr->data[index]);
    arr->data[index] = arr->data[arr->len--];
    shrink_p(arr);
    return true;
}

bool Nst_pa_remove_shift(Nst_PtrArray *arr, usize index, Nst_Destructor dstr)
{
    if (index >= arr->len)
        return false;
    if (dstr != NULL)
        dstr(arr->data[index]);
    memmove(
        arr->data + index,
        arr->data + index + 1,
        (arr->len - index - 1) * sizeof(void *));
    arr->len--;
    shrink_p(arr);
    return true;
}

void *Nst_pa_get(Nst_PtrArray *arr, usize index)
{
    if (index >= arr->len)
        return NULL;
    return arr->data[index];
}

void Nst_pa_clear(Nst_PtrArray *arr, Nst_Destructor dstr)
{
    if (arr->data != NULL) {
        if (dstr != NULL) {
            for (usize i = 0; i < arr->len; i++)
                dstr(arr->data[i]);
        }
        Nst_free(arr->data);
    }
    arr->data = NULL;
    arr->cap = 0;
    arr->len = 0;
}

void Nst_pa_set(Nst_PtrArray *arr, usize index, void *element,
                Nst_Destructor dstr)
{
    if (index >= arr->len)
        return;
    if (dstr != NULL)
        dstr(arr->data[index]);
    arr->data[index] = element;
}
