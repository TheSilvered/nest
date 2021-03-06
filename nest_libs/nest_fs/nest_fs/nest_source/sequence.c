#include <stdlib.h>
#include <errno.h>

#include "sequence.h"
#include "obj_ops.h"

Nst_Obj *new_array(size_t len)
{
    Nst_SeqObj *arr = AS_SEQ(alloc_obj(sizeof(Nst_SeqObj), nst_t_arr, destroy_seq));
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

Nst_Obj *new_vector(size_t len)
{
    size_t size = (size_t)(len * VECTOR_GROWTH_RATIO);

    if ( size < VECTOR_MIN_SIZE )
        size = VECTOR_MIN_SIZE;

    Nst_SeqObj *vect = AS_SEQ(alloc_obj(sizeof(Nst_SeqObj), nst_t_vect, destroy_seq));
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

void destroy_seq(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for ( size_t i = 0, n = seq->len; i < n; i++ )
        dec_ref(objs[i]);

    free(objs);
}

void resize_vector(Nst_SeqObj *vect)
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

void append_value_vector(Nst_SeqObj *vect, Nst_Obj *val)
{
    if ( vect->size == vect->len )
        resize_vector(vect);

    if ( errno == ENOMEM )
        return;

    inc_ref(val);
    vect->objs[vect->len] = val;
    vect->len++;
}

bool set_value_seq(Nst_SeqObj *seq, int64_t idx, Nst_Obj *val)
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

Nst_Obj *get_value_seq(Nst_SeqObj *seq, int64_t idx)
{
    if ( idx < 0 )
        idx += seq->len;

    if ( idx < 0 || idx >= (int64_t)seq->len )
        return NULL;

    inc_ref(seq->objs[idx]);
    return seq->objs[idx];
}

Nst_Obj *rem_value_vector(Nst_SeqObj *vect, Nst_Obj *val)
{
    register size_t i = 0;
    register size_t n = vect->len;
    register Nst_Obj **objs = vect->objs;

    for ( ; i < n; i++ )
    {
        if ( obj_eq(val, objs[i], NULL) == nst_true )
            break;
        if ( i + 1 == n )
            return inc_ref(nst_false);
    }

    for ( i++; i < n; i++ )
    {
        set_value_seq(vect, i - 1, objs[i]);
    }

    dec_ref(objs[n - 1]);
    vect->len--;
    resize_vector(vect);

    return inc_ref(nst_true);;
}

Nst_Obj *pop_value_vector(Nst_SeqObj *vect, size_t quantity)
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
