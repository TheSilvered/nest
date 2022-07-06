#include <stdlib.h>
#include <errno.h>

#include "sequence.h"
#include "obj_ops.h"

Nst_sequence *new_array_empty(size_t len)
{
    Nst_sequence *arr = malloc(sizeof(Nst_sequence));
    Nst_Obj **objs = calloc(len, sizeof(Nst_Obj *));

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

Nst_sequence *new_vector_empty(size_t len)
{
    size_t size = (size_t)(len * VECTOR_GROWTH_RATIO);

    if ( size < VECTOR_MIN_SIZE )
        size = VECTOR_MIN_SIZE;

    Nst_sequence *vect = malloc(sizeof(Nst_sequence));
    Nst_Obj **objs = calloc(size, sizeof(Nst_Obj *));

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

void destroy_seq(Nst_sequence *seq)
{
    Nst_Obj **objs = seq->objs;
    for ( size_t i = 0, n = seq->len; i < n; i++ )
        dec_ref(objs[i]);

    free(objs);
    free(seq);
}

Nst_Obj *new_arr_obj(Nst_sequence *arr)
{
    return make_obj(arr, nst_t_arr, destroy_seq);
}

Nst_Obj *new_vect_obj(Nst_sequence *vect)
{
    return make_obj(vect, nst_t_vect, destroy_seq);
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

    for ( size_t i = len; i < new_size; i++ )
        new_objs[i] = NULL;

    vect->size = new_size;
    vect->objs = new_objs;
}

void append_value_vector(Nst_sequence *vect, Nst_Obj *val)
{
    if ( vect->size == vect->len )
        resize_vector(vect);

    if ( errno == ENOMEM )
        return;

    inc_ref(val);
    vect->objs[vect->len] = val;
    vect->len++;
}

bool set_value_seq(Nst_sequence *seq, int64_t idx, Nst_Obj *val)
{
    if ( idx < 0 )
        idx += seq->len;

    if ( idx < 0 || idx >= (int64_t)seq->len )
        return false;

    inc_ref(val);
    if ( seq->objs[idx] != NULL )
        dec_ref(seq->objs[idx]);
    seq->objs[idx] = val;

    return true;
}

Nst_Obj *get_value_seq(Nst_sequence *seq, int64_t idx)
{
    if ( idx < 0 )
        idx += seq->len;

    if ( idx < 0 || idx >= (int64_t)seq->len )
        return NULL;

    inc_ref(seq->objs[idx]);
    return seq->objs[idx];
}

Nst_Obj *rem_value_vector(Nst_sequence *vect, Nst_Obj *val)
{
    register size_t i = 0;
    register size_t n = vect->len;
    register Nst_Obj **objs = vect->objs;

    for ( ; i < n; i++ )
    {
        if ( obj_eq(val, objs[i], NULL) == nst_true )
            break;
        if ( i + 1 == n )
        {
            inc_ref(nst_false);
            return nst_false;
        }
    }

    for ( i++; i < n; i++ )
    {
        set_value_seq(vect, i - 1, objs[i]);
    }

    dec_ref(objs[n - 1]);
    vect->len--;
    resize_vector(vect);

    inc_ref(nst_true);
    return nst_true;
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

    if ( last_obj == NULL )
    {
        inc_ref(nst_null);
        return nst_null;
    }
    else
    {
        // Has already one more reference because it was in the vector
        return last_obj;
    }
}
