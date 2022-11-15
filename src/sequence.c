#include <stdlib.h>
#include <errno.h>

#include "sequence.h"
#include "obj_ops.h"
#include "lib_import.h"

static Nst_Obj *new_seq(size_t len, size_t size, Nst_TypeObj *type)
{
    Nst_SeqObj *seq = SEQ(nst_alloc_obj(
        sizeof(Nst_SeqObj),
        type,
        nst_destroy_seq
    ));
    Nst_Obj **objs = (Nst_Obj **)calloc(size, sizeof(Nst_Obj *));

    if ( seq == NULL || objs == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    seq->len = len;
    seq->size = size;
    seq->objs = objs;

    NST_GGC_SUPPORT_INIT(seq, nst_traverse_seq, nst_track_seq);

    return OBJ(seq);
}

Nst_Obj *nst_new_array(size_t len)
{
    return new_seq(len, len, nst_t.Array);
}

Nst_Obj *nst_new_vector(size_t len)
{
    size_t size = (size_t)(len * VECTOR_GROWTH_RATIO);

    if ( size < VECTOR_MIN_SIZE )
        size = VECTOR_MIN_SIZE;

    return new_seq(len, size, nst_t.Vector);
}

void nst_destroy_seq(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for ( size_t i = 0, n = seq->len; i < n; i++ )
        nst_dec_ref(objs[i]);

    free(objs);
}

void nst_traverse_seq(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for ( size_t i = 0, n = seq->len; i < n; i++ )
        NST_SET_FLAG(objs[i], NST_FLAG_GGC_REACHABLE);
}

void nst_track_seq(Nst_SeqObj* seq)
{
    if ( NST_OBJ_IS_TRACKED(seq) )
        return;

    Nst_Obj **objs = seq->objs;
    for ( size_t i = 0, n = seq->len; i < n; i++ )
    {
        if ( NST_HAS_FLAG(objs[i], NST_FLAG_GGC_IS_SUPPORTED) )
            nst_add_tracked_object((Nst_GGCObj *)objs[i]);
    }
}

void _nst_resize_vector(Nst_SeqObj *vect)
{
    size_t len = vect->len;
    size_t size = vect->size;

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

    Nst_Obj **new_objs = (Nst_Obj **)realloc(vect->objs, new_size * sizeof(Nst_Obj *));

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

    if ( NST_OBJ_IS_TRACKED(vect) && NST_HAS_FLAG(val, NST_FLAG_GGC_IS_SUPPORTED) )
        nst_add_tracked_object((Nst_GGCObj *)val);
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

    if ( NST_OBJ_IS_TRACKED(seq) && NST_HAS_FLAG(val, NST_FLAG_GGC_IS_SUPPORTED) )
        nst_add_tracked_object((Nst_GGCObj *)val);

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
    size_t i = 0;
    size_t n = vect->len;
    Nst_Obj **objs = vect->objs;

    for ( ; i < n; i++ )
    {
        if ( nst_obj_eq(val, objs[i], NULL) == nst_c.b_true )
        {
            nst_dec_ref(objs[i]);
            break;
        }
        if ( i + 1 == n )
            NST_RETURN_FALSE;
    }

    for ( i++; i < n; i++ )
        vect->objs[i - 1] = objs[i];

    vect->len--;
    _nst_resize_vector(vect);

    NST_RETURN_TRUE;
}

Nst_Obj *_nst_pop_value_vector(Nst_SeqObj *vect, size_t quantity)
{
    if ( quantity > vect->len )
        quantity = vect->len;

    Nst_Obj *last_obj = NULL;
    size_t n = vect->len;

    for ( size_t i = 1; i <= quantity; i++ )
    {
        if ( last_obj != NULL )
            nst_dec_ref(last_obj);
        last_obj = vect->objs[n - i];
        vect->len--;
    }

    if ( last_obj == NULL )
    {
        nst_inc_ref(nst_c.null);
        return nst_c.null;
    }
    else
    {
        // Has already one more reference because it was in the vector
        return last_obj;
    }
}
