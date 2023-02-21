#include <errno.h>
#include <stdlib.h>
#include "iter.h"
#include "obj_ops.h"

Nst_Obj *nst_iter_new(Nst_FuncObj *start,
                      Nst_FuncObj *is_done,
                      Nst_FuncObj *get_val,
                      Nst_Obj     *value)
{
    Nst_IterObj *iter = ITER(nst_obj_alloc(
        sizeof(Nst_IterObj),
        nst_t.Iter,
        _nst_iter_destroy));
    if ( iter == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    iter->start = start;
    iter->is_done = is_done;
    iter->get_val = get_val;
    iter->value = value;

    if ( NST_FLAG_HAS(start,   NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_FLAG_HAS(is_done, NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_FLAG_HAS(get_val, NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_FLAG_HAS(value,   NST_FLAG_GGC_IS_SUPPORTED) )
        NST_GGC_OBJ_INIT(iter, _nst_iter_traverse, _nst_iter_track);

    return OBJ(iter);
}

void _nst_iter_destroy(Nst_IterObj *iter)
{
    nst_dec_ref(iter->start);
    nst_dec_ref(iter->is_done);
    nst_dec_ref(iter->get_val);
    nst_dec_ref(iter->value);
}

void _nst_iter_track(Nst_IterObj* iter)
{
    if ( NST_FLAG_HAS(iter->start, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_ggc_track_obj((Nst_GGCObj*)iter->start);
    }

    if ( NST_FLAG_HAS(iter->is_done, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_ggc_track_obj((Nst_GGCObj*)iter->is_done);
    }

    if ( NST_FLAG_HAS(iter->get_val, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_ggc_track_obj((Nst_GGCObj*)iter->get_val);
    }

    if ( NST_FLAG_HAS(iter->value, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_ggc_track_obj((Nst_GGCObj*)iter->value);
    }
}

void _nst_iter_traverse(Nst_IterObj* iter)
{
    NST_FLAG_SET(iter->start,   NST_FLAG_GGC_REACHABLE);
    NST_FLAG_SET(iter->is_done, NST_FLAG_GGC_REACHABLE);
    NST_FLAG_SET(iter->get_val, NST_FLAG_GGC_REACHABLE);
    NST_FLAG_SET(iter->value,   NST_FLAG_GGC_REACHABLE);
}

i32 _nst_iter_start(Nst_IterObj *iter, Nst_OpErr *err)
{
    Nst_Obj *result = nst_call_func(iter->start, &iter->value, err);

    if ( result == NULL )
    {
        return -1;
    }

    nst_dec_ref(result);
    return 0;
}

i32 _nst_iter_is_done(Nst_IterObj *iter, Nst_OpErr *err)
{
    Nst_Obj *result = nst_call_func(iter->is_done, &iter->value, err);

    if ( result == NULL )
    {
        return -1;
    }

    if ( nst_obj_cast(result, nst_t.Bool, NULL) == nst_c.Bool_true )
    {
        nst_dec_ref(nst_c.Bool_true);
        nst_dec_ref(result);
        return 1;
    }
    nst_dec_ref(nst_c.Bool_false);
    nst_dec_ref(result);
    return 0;
}

Nst_Obj *_nst_iter_get_val(Nst_IterObj *iter, Nst_OpErr *err)
{
    return nst_call_func(iter->get_val, &iter->value, err);
}

#if defined(_WIN32) || defined(WIN32)
#pragma warning( disable: 4100 )
#endif

NST_FUNC_SIGN(nst_iter_range_start)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = AS_INT(val->objs[1]);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_iter_range_is_done)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    Nst_Int idx = AS_INT(objs[0]);
    Nst_Int stop = AS_INT(objs[2]);
    Nst_Int step = AS_INT(objs[3]);

    if ( step > 0 )
    {
        NST_RETURN_COND(idx >= stop);
    }
    else
    {
        NST_RETURN_COND(idx <= stop);
    }
}

NST_FUNC_SIGN(nst_iter_range_get_val)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj *ob = nst_int_new(AS_INT(val->objs[0]));
    AS_INT(val->objs[0]) += AS_INT(val->objs[3]);
    return ob;
}

NST_FUNC_SIGN(nst_iter_seq_start)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_iter_seq_is_done)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    usize seq_len = SEQ(objs[1])->len;

    if ( seq_len == 0 || AS_INT(objs[0]) >= (Nst_Int)seq_len )
    {
        NST_RETURN_TRUE;
    }
    else
    {
        NST_RETURN_FALSE;
    }
}

NST_FUNC_SIGN(nst_iter_seq_get_val)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_SeqObj *seq = SEQ(val->objs[1]);
    usize idx = (usize)AS_INT(val->objs[0]);

    if ( seq->len < idx )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            seq->type == nst_t.Array ? _NST_EM_INDEX_OUT_OF_BOUNDS("Array")
                                     : _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            "iu",
            idx,
            seq->len));

        return NULL;
    }

    Nst_Obj *obj = SEQ(val->objs[1])->objs[AS_INT(val->objs[0])];
    AS_INT(val->objs[0]) += 1;
    return nst_inc_ref(obj);
}

NST_FUNC_SIGN(nst_iter_str_start)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_iter_str_is_done)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    usize str_len = STR(objs[1])->len;

    if ( str_len == 0 || AS_INT(objs[0]) >= (Nst_Int)str_len )
    {
        NST_RETURN_TRUE;
    }
    else
    {
        NST_RETURN_FALSE;
    }
}

NST_FUNC_SIGN(nst_iter_str_get_val)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    Nst_StrObj *str = STR(objs[1]);
    usize idx = (usize)AS_INT(objs[0]);

    if ( idx >= str->len )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            _NST_EM_INDEX_OUT_OF_BOUNDS("Str"),
            "iu",
            idx,
            str->len));

        return NULL;
    }
    Nst_Obj *ob = nst_string_get(objs[1], AS_INT(objs[0]));
    AS_INT(val->objs[0]) += 1;
    return ob;
}
