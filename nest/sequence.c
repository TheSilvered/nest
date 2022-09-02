#include <stdlib.h>
#include <errno.h>

#include "sequence.h"
#include "obj_ops.h"

Nst_Obj *nst_new_array(size_t len)
{
    Nst_SeqObj *arr = AS_SEQ(nst_alloc_obj(
        sizeof(Nst_SeqObj),
        nst_t_arr,
        nst_destroy_seq
    ));
    Nst_Obj **objs = calloc(len, sizeof(Nst_Obj *));

    if ( arr == NULL || objs == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    arr->len = len;
    arr->size = len;
    arr->objs = objs;

    return (Nst_Obj *)arr;
}

Nst_Obj *nst_new_vector(size_t len)
{
    size_t size = (size_t)(len * VECTOR_GROWTH_RATIO);

    if ( size < VECTOR_MIN_SIZE )
        size = VECTOR_MIN_SIZE;

    Nst_SeqObj *vect = AS_SEQ(nst_alloc_obj(
        sizeof(Nst_SeqObj),
        nst_t_vect,
        nst_destroy_seq
    ));
    Nst_Obj **objs = calloc(size, sizeof(Nst_Obj *));

    if ( vect == NULL || objs == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    vect->len = len;
    vect->size = size;
    vect->objs = objs;

    return (Nst_Obj *)vect;
}

void nst_destroy_seq(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for ( size_t i = 0, n = seq->len; i < n; i++ )
        nst_dec_ref(objs[i]);

    free(objs);
}

void _nst_resize_vector(Nst_SeqObj *vect)
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

void _nst_append_value_vector(Nst_SeqObj *vect, Nst_Obj *val)
{
    if ( vect->size == vect->len )
        _nst_resize_vector(vect);

    if ( errno == ENOMEM )
        return;

    nst_inc_ref(val);
    vect->objs[vect->len] = val;
    vect->len++;
}

bool _nst_set_value_seq(Nst_SeqObj *seq, int64_t idx, Nst_Obj *val)
{
    if ( idx < 0 )
        idx += seq->len;

    if ( idx < 0 || idx >= (int64_t)seq->len )
        return false;

    nst_inc_ref(val);
    if ( seq->objs[idx] != NULL )
        nst_dec_ref(seq->objs[idx]);
    seq->objs[idx] = val;

    return true;
}

Nst_Obj *_nst_get_value_seq(Nst_SeqObj *seq, int64_t idx)
{
    if ( idx < 0 )
        idx += seq->len;

    if ( idx < 0 || idx >= (int64_t)seq->len )
        return NULL;

    nst_inc_ref(seq->objs[idx]);
    return seq->objs[idx];
}

Nst_Obj *_nst_rem_value_vector(Nst_SeqObj *vect, Nst_Obj *val)
{
    register size_t i = 0;
    register size_t n = vect->len;
    register Nst_Obj **objs = vect->objs;

    for ( ; i < n; i++ )
    {
        if ( nst_obj_eq(val, objs[i], NULL) == nst_true )
            break;
        if ( i + 1 == n )
            return nst_inc_ref(nst_false);
    }

    for ( i++; i < n; i++ )
    {
        _nst_set_value_seq(vect, i - 1, objs[i]);
    }

    nst_dec_ref(objs[n - 1]);
    vect->len--;
    _nst_resize_vector(vect);

    return nst_inc_ref(nst_true);;
}

Nst_Obj *_nst_pop_value_vector(Nst_SeqObj *vect, size_t quantity)
{
    if ( quantity > vect->len )
        quantity = vect->len;

    register Nst_Obj *last_obj = NULL;
    register size_t n = vect->len;

    for ( size_t i = 1; i <= quantity; i++ )
    {
        if ( last_obj != NULL )
            nst_dec_ref(last_obj);
        last_obj = vect->objs[n - i];
        vect->len--;
    }

    if ( last_obj == NULL )
    {
        nst_inc_ref(nst_null);
        return nst_null;
    }
    else
    {
        // Has already one more reference because it was in the vector
        return last_obj;
    }
}
