#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "sequence.h"
#include "obj_ops.h"
#include "lib_import.h"

static Nst_Obj *new_seq(size_t len, size_t size, Nst_TypeObj *type)
{
    Nst_SeqObj *seq = SEQ(nst_obj_alloc(
        sizeof(Nst_SeqObj),
        type,
        _nst_seq_destroy));
    Nst_Obj **objs = (Nst_Obj **)calloc(size, sizeof(Nst_Obj *));

    if ( seq == NULL || objs == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    seq->len = len;
    seq->size = size;
    seq->objs = objs;

    NST_GGC_OBJ_INIT(seq, _nst_seq_traverse, _nst_seq_track);

    return OBJ(seq);
}

Nst_Obj *nst_array_new(size_t len)
{
    return new_seq(len, len, nst_t.Array);
}

Nst_Obj *nst_vector_new(size_t len)
{
    size_t size = (size_t)(len * _NST_VECTOR_GROWTH_RATIO);

    if ( size < _NST_VECTOR_MIN_SIZE )
    {
        size = _NST_VECTOR_MIN_SIZE;
    }

    return new_seq(len, size, nst_t.Vector);
}

void _nst_seq_destroy(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for ( size_t i = 0, n = seq->len; i < n; i++ )
    {
        nst_dec_ref(objs[i]);
    }

    free(objs);
}

void _nst_seq_traverse(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for ( size_t i = 0, n = seq->len; i < n; i++ )
    {
        NST_FLAG_SET(objs[i], NST_FLAG_GGC_REACHABLE);
    }
}

void _nst_seq_track(Nst_SeqObj* seq)
{
    Nst_Obj **objs = seq->objs;
    for ( size_t i = 0, n = seq->len; i < n; i++ )
    {
        if ( NST_FLAG_HAS(objs[i], NST_FLAG_GGC_IS_SUPPORTED) )
        {
            nst_ggc_track_obj((Nst_GGCObj*)objs[i]);
        }
    }
}

void _nst_vector_resize(Nst_SeqObj *vect)
{
    size_t len = vect->len;
    size_t size = vect->size;
    size_t new_size;

    assert(len <= size);

    if ( size == len )
    {
        new_size = (size_t)(len * _NST_VECTOR_GROWTH_RATIO);
    }
    else if ( size >> 2 >= len ) // if it's three quarters empty or less
    {
        new_size = (size_t)(size / _NST_VECTOR_GROWTH_RATIO);
        if ( new_size < _NST_VECTOR_MIN_SIZE )
        {
            new_size = _NST_VECTOR_MIN_SIZE;
        }

        if ( size == _NST_VECTOR_MIN_SIZE )
        {
            return;
        }
    }
    else
    {
        return;
    }

    Nst_Obj **new_objs = (Nst_Obj **)realloc(
        vect->objs,
        new_size * sizeof(Nst_Obj *));

    if ( new_objs == NULL )
    {
        errno = ENOMEM;
        return;
    }

    for ( size_t i = len; i < new_size; i++ )
    {
        new_objs[i] = NULL;
    }

    vect->size = new_size;
    vect->objs = new_objs;
}

void _nst_vector_append(Nst_SeqObj *vect, Nst_Obj *val)
{
    if ( vect->size == vect->len )
    {
        _nst_vector_resize(vect);
    }

    if ( errno == ENOMEM )
    {
        return;
    }

    vect->objs[vect->len++] = nst_inc_ref(val);;

    if ( NST_OBJ_IS_TRACKED(vect) &&
         NST_FLAG_HAS(val, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_ggc_track_obj((Nst_GGCObj*)val);
    }
}

bool _nst_seq_set(Nst_SeqObj *seq, int64_t idx, Nst_Obj *val)
{
    if ( idx < 0 )
    {
        idx += seq->len;
    }

    if ( idx < 0 || idx >= (int64_t)seq->len )
    {
        return false;
    }

    nst_inc_ref(val);
    if ( seq->objs[idx] != NULL )
    {
        nst_dec_ref(seq->objs[idx]);
    }
    seq->objs[idx] = val;

    if ( NST_OBJ_IS_TRACKED(seq) &&
         NST_FLAG_HAS(val, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_ggc_track_obj((Nst_GGCObj*)val);
    }

    return true;
}

Nst_Obj *_nst_seq_get(Nst_SeqObj *seq, int64_t idx)
{
    if ( idx < 0 )
    {
        idx += seq->len;
    }

    if ( idx < 0 || idx >= (int64_t)seq->len )
    {
        return NULL;
    }

    return nst_inc_ref(seq->objs[idx]);
}

Nst_Obj *_nst_vector_remove(Nst_SeqObj *vect, Nst_Obj *val)
{
    size_t i = 0;
    size_t n = vect->len;
    Nst_Obj **objs = vect->objs;

    for ( ; i < n; i++ )
    {
        if ( nst_obj_eq(val, objs[i], NULL) == nst_c.Bool_true )
        {
            nst_dec_ref(nst_c.Bool_true);
            nst_dec_ref(objs[i]);
            break;
        }
        else
        {
            nst_dec_ref(nst_c.Bool_false);
        }

        if ( i + 1 == n )
        {
            NST_RETURN_FALSE;
        }
    }

    for ( i++; i < n; i++ )
    {
        vect->objs[i - 1] = objs[i];
    }

    vect->len--;
    _nst_vector_resize(vect);

    NST_RETURN_TRUE;
}

Nst_Obj *_nst_vector_pop(Nst_SeqObj *vect, size_t quantity)
{
    if ( quantity > vect->len )
    {
        quantity = vect->len;
    }

    Nst_Obj *last_obj = NULL;
    size_t n = vect->len;

    for ( size_t i = 1; i <= quantity; i++ )
    {
        if ( last_obj != NULL )
        {
            nst_dec_ref(last_obj);
        }
        last_obj = vect->objs[n - i];
        vect->len--;
    }

    if ( last_obj == NULL )
    {
        NST_RETURN_NULL;
    }
    else
    {
        // Has already one more reference because it was in the vector
        return last_obj;
    }
}
