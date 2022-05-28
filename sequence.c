#include <stdlib.h>
#include <errno.h>

#include "sequence.h"
#include "obj_ops.h"

Nst_sequence *new_array_empty(size_t len)
{
    Nst_sequence *arr = malloc(sizeof(Nst_sequence));
    Nst_Obj **objs = malloc(len * sizeof(Nst_Obj *));

    if ( arr == NULL || objs == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    arr->len = len;
    arr->size = len;
    arr->objs = objs;

    return arr;
}

Nst_sequence *new_array_values(LList *values)
{
    Nst_sequence *arr = malloc(sizeof(Nst_sequence));
    Nst_Obj **objs = malloc(values->size * sizeof(Nst_Obj *));

    if ( arr == NULL || objs == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    size_t idx = 0;
    for ( LLNode *cursor = values->head;
          cursor != NULL;
          cursor = cursor->next )
        objs[idx++] = cursor->value;

    arr->len = values->size;
    arr->size = values->size;
    arr->objs = objs;

    return arr;
}

Nst_sequence *new_vector_empty(size_t len)
{
    size_t size = (size_t)(len * VECTOR_GROWTH_RATIO);

    if ( size < VECTOR_MIN_SIZE )
        size = VECTOR_MIN_SIZE;

    Nst_sequence *vect = malloc(sizeof(Nst_sequence));
    Nst_Obj **objs = malloc(size * sizeof(Nst_Obj *));

    if ( vect == NULL || objs == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    vect->len = len;
    vect->size = size;
    vect->objs = objs;

    return vect;
}

Nst_sequence *new_vector_values(LList *values)
{
    size_t size = (size_t)(values->size * VECTOR_GROWTH_RATIO);

    if ( size < VECTOR_MIN_SIZE )
        size = VECTOR_MIN_SIZE;

    Nst_sequence *vect = malloc(sizeof(Nst_sequence));
    Nst_Obj **objs = malloc(values->size * sizeof(Nst_Obj *));

    if ( vect == NULL || objs == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    size_t idx = 0;
    for ( LLNode *cursor = values->head; cursor != NULL; cursor = cursor->next )
        objs[idx++] = cursor->value;

    vect->len = values->size;
    vect->size = values->size;
    vect->objs = objs;

    return vect;
}

void resize_vector(Nst_sequence *vect)
{
    register size_t len = vect->len;
    register size_t size = vect->size;

    size_t new_size;

    if ( size == len )
        new_size = (size_t)(len * VECTOR_GROWTH_RATIO);
    else if ( len << 2 <= size ) // if it's half empty or less
    {
        new_size = (size_t)(len / VECTOR_GROWTH_RATIO);
        if ( new_size < VECTOR_MIN_SIZE )
            new_size = VECTOR_MIN_SIZE;
    }
    else
        return;

    Nst_Obj **new_objs = realloc(vect->objs, new_size * sizeof(Nst_Obj *));

    if ( new_objs == NULL )
    {
        errno = ENOMEM;
        return;
    }

    vect->size = new_size;
    vect->objs = new_objs;
}

void append_value_vector(Nst_sequence *vect, Nst_Obj *val)
{
    if ( vect->size == vect->len )
        resize_vector(vect);

    if ( errno == ENOMEM )
        return;

    vect->objs[vect->len] = val;
    vect->len++;
}

void set_value_seq(Nst_sequence *seq, size_t idx, Nst_Obj *val)
{
    inc_ref(val);
    dec_ref(seq->objs[idx]);
    seq->objs[idx] = val;
}

void rem_value_vector(Nst_sequence *vect, Nst_Obj *val)
{
    register size_t i = 0;
    register size_t n = vect->len;
    register Nst_Obj **objs = vect->objs;

    for ( ; i < n; i++ )
    {
        if ( obj_eq(val, objs[i], NULL) == nst_true )
            break;
        if ( i + 1 == n )
            return;
    }

    for ( i++; i < n; i++ )
    {
        set_value_seq(vect, i - 1, objs[i]);
    }

    dec_ref(objs[n - 1]);
    vect->len--;
    resize_vector(vect);
}

Nst_Obj *pop_value_vector(Nst_sequence *vect, size_t quantity)
{
    if ( quantity > vect->len )
        quantity = vect->len;

    register Nst_Obj *last_obj = NULL;
    register size_t n = vect->len;

    for ( size_t i = 1; i <= quantity; i++ )
    {
        if ( last_obj != NULL )
            dec_ref(last_obj);
        last_obj = vect->objs[n - i];
        vect->len--;
    }

    return last_obj ? last_obj : nst_null;
}
