#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include "mem.h"
#include "sequence.h"
#include "obj_ops.h"
#include "lib_import.h"

static Nst_Obj *new_seq(usize len, usize size, Nst_TypeObj *type, Nst_OpErr *err)
{
    Nst_SeqObj *seq = nst_obj_alloc(
        Nst_SeqObj,
        type,
        _nst_seq_destroy,
        err);
    Nst_Obj **objs = (Nst_Obj **)nst_calloc(size, sizeof(Nst_Obj *), NULL, err);

    if ( seq == NULL || objs == NULL )
    {
        return NULL;
    }

    seq->len = len;
    seq->size = size;
    seq->objs = objs;

    NST_GGC_OBJ_INIT(seq, _nst_seq_traverse, _nst_seq_track);

    return OBJ(seq);
}

Nst_Obj *nst_array_new(usize len, Nst_OpErr *err)
{
    return new_seq(len, len, nst_t.Array, err);
}

Nst_Obj *nst_vector_new(usize len, Nst_OpErr *err)
{
    usize size = (usize)(len * _NST_VECTOR_GROWTH_RATIO);

    if ( size < _NST_VECTOR_MIN_SIZE )
    {
        size = _NST_VECTOR_MIN_SIZE;
    }

    return new_seq(len, size, nst_t.Vector, err);
}

void _nst_seq_destroy(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        nst_dec_ref(objs[i]);
    }

    nst_free(objs);
}

void _nst_seq_traverse(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        NST_FLAG_SET(objs[i], NST_FLAG_GGC_REACHABLE);
    }
}

void _nst_seq_track(Nst_SeqObj* seq)
{
    Nst_Obj **objs = seq->objs;
    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        if ( NST_FLAG_HAS(objs[i], NST_FLAG_GGC_IS_SUPPORTED) )
        {
            nst_ggc_track_obj((Nst_GGCObj*)objs[i]);
        }
    }
}

bool _nst_vector_resize(Nst_SeqObj *vect, Nst_OpErr *err)
{
    usize len = vect->len;
    usize size = vect->size;
    usize new_size;

    assert(len <= size);

    if ( size == len )
    {
        new_size = (usize)(len * _NST_VECTOR_GROWTH_RATIO);
    }
    else if ( size >> 2 >= len ) // if it's three quarters empty or less
    {
        new_size = (usize)(size / _NST_VECTOR_GROWTH_RATIO);
        if ( new_size < _NST_VECTOR_MIN_SIZE )
        {
            new_size = _NST_VECTOR_MIN_SIZE;
        }

        if ( size == _NST_VECTOR_MIN_SIZE )
        {
            return true;
        }
    }
    else
    {
        return true;
    }

    Nst_Obj **new_objs = (Nst_Obj **)nst_realloc(
        vect->objs,
        new_size,
        sizeof(Nst_Obj *),
        size,
        err);

    if ( new_objs == NULL )
    {
        return false;
    }

    for ( usize i = len; i < new_size; i++ )
    {
        new_objs[i] = NULL;
    }

    vect->size = new_size;
    vect->objs = new_objs;
    return true;
}

bool _nst_vector_append(Nst_SeqObj *vect, Nst_Obj *val, Nst_OpErr *err)
{
    if ( !_nst_vector_resize(vect, err) )
    {
        return false;
    }

    vect->objs[vect->len++] = nst_inc_ref(val);

    if ( NST_OBJ_IS_TRACKED(vect) &&
         NST_FLAG_HAS(val, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_ggc_track_obj((Nst_GGCObj*)val);
    }
    return true;
}

bool _nst_seq_set(Nst_SeqObj *seq, i64 idx, Nst_Obj *val)
{
    if ( idx < 0 )
    {
        idx += seq->len;
    }

    if ( idx < 0 || idx >= (i64)seq->len )
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

Nst_Obj *_nst_seq_get(Nst_SeqObj *seq, i64 idx)
{
    if ( idx < 0 )
    {
        idx += seq->len;
    }

    if ( idx < 0 || idx >= (i64)seq->len )
    {
        return NULL;
    }

    return nst_inc_ref(seq->objs[idx]);
}

Nst_Obj *_nst_vector_remove(Nst_SeqObj *vect, Nst_Obj *val)
{
    usize i = 0;
    usize n = vect->len;
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
    _nst_vector_resize(vect, NULL);

    NST_RETURN_TRUE;
}

Nst_Obj *_nst_vector_pop(Nst_SeqObj *vect, usize quantity)
{
    if ( quantity > vect->len )
    {
        quantity = vect->len;
    }

    Nst_Obj *last_obj = NULL;
    usize n = vect->len;

    for ( usize i = 1; i <= quantity; i++ )
    {
        if ( last_obj != NULL )
        {
            nst_dec_ref(last_obj);
        }
        last_obj = vect->objs[n - i];
        vect->len--;
    }

    _nst_vector_resize(vect, NULL);

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

void seq_create(usize len, Nst_Obj *seq, va_list args)
{
    if ( seq == NULL )
    {
        for ( usize i = 0; i < len; i++ )
        {
            Nst_Obj *arg = va_arg(args, Nst_Obj *);
            nst_dec_ref(arg);
        }
        return;
    }

    Nst_Obj **objs = SEQ(seq)->objs;
    for ( usize i = 0; i < len; i++ )
    {
        Nst_Obj *arg = va_arg(args, Nst_Obj *);
        objs[i] = arg;
    }
}

Nst_Obj *nst_vector_create(usize len, Nst_OpErr *err, ...)
{
    Nst_Obj *vector = nst_vector_new(len, err);
    va_list args;
    va_start(args, err);
    seq_create(len, vector, args);
    va_end(args);
    return vector;
}

Nst_Obj *nst_array_create(usize len, Nst_OpErr *err, ...)
{
    Nst_Obj *array = nst_array_new(len, err);
    va_list args;
    va_start(args, err);
    seq_create(len, array, args);
    va_end(args);
    return array;
}

Nst_Obj *seq_create_c(usize      len,
                      Nst_Obj   *seq,
                      const i8  *fmt,
                      Nst_OpErr *err,
                      va_list    args)
{
    if ( seq == NULL )
    {
        return NULL;
    }

    i8 *p = (i8 *)fmt;
    usize i = 0;
    Nst_Obj **objs = SEQ(seq)->objs;
    while ( *p )
    {
        switch ( *p++ )
        {
        case 'I':
        {
            i64 value = va_arg(args, i64);
            Nst_Obj *obj = nst_int_new(value, err);
            if ( obj == NULL )
            {
                goto failed;
            }
            objs[i] = obj;
            break;
        }
        case 'i':
        {
            i32 value = va_arg(args, i32);
            Nst_Obj *obj = nst_int_new((i64)value, err);
            if ( obj == NULL )
            {
                goto failed;
            }
            objs[i] = obj;
            break;
        }
        case 'f':
        case 'F':
        {
            f64 value = va_arg(args, f64);
            Nst_Obj *obj = nst_real_new(value, err);
            if ( obj == NULL )
            {
                goto failed;
            }
            objs[i] = obj;
            break;
        }
        case 'b':
        {
            int value = va_arg(args, int);
            if ( value )
            {
                objs[i] = nst_inc_ref(nst_c.Bool_true);
            }
            else
            {
                objs[i] = nst_inc_ref(nst_c.Bool_false);
            }
            break;
        }
        case 'B':
        {
            u8 value = va_arg(args, u8);
            Nst_Obj *obj = nst_byte_new(value, err);
            if ( obj == NULL )
            {
                goto failed;
            }
            objs[i] = obj;
            break;
        }
        case 'o':
        {
            Nst_Obj *obj = va_arg(args, Nst_Obj *);
            objs[i] = obj;
            break;
        }
        case 'O':
        {
            Nst_Obj *obj = va_arg(args, Nst_Obj *);
            objs[i] = nst_inc_ref(obj);
            break;
        }
        case 'n':
        {
            (void)va_arg(args, void *);
            objs[i] = nst_inc_ref(nst_c.Null_null);
            break;
        }
        default:
            if ( seq->type == nst_t.Vector )
            {
                NST_SET_RAW_VALUE_ERROR(
                    _NST_EM_INVALID_TYPE_LETTER("nst_vector_create_c"));
            }
            else
            {
                NST_SET_RAW_VALUE_ERROR(
                    _NST_EM_INVALID_TYPE_LETTER("nst_array_create_c"));
            }
            goto failed;
        }
        i++;
    }

    return seq;
failed:
    SEQ(seq)->len = i;
    nst_dec_ref(seq);
    return NULL;
}

Nst_Obj *nst_vector_create_c(const i8 *fmt, Nst_OpErr *err, ...)
{
    usize len = strlen(fmt);
    Nst_Obj *vector = nst_vector_new(len, err);
    va_list args;
    va_start(args, err);
    vector = seq_create_c(len, vector, fmt, err, args);
    va_end(args);
    return vector;
}

Nst_Obj *nst_array_create_c(const i8 *fmt, Nst_OpErr *err, ...)
{
    usize len = strlen(fmt);
    Nst_Obj *array = nst_array_new(len, err);
    va_list args;
    va_start(args, err);
    array = seq_create_c(len, array, fmt, err, args);
    va_end(args);
    return array;
}
