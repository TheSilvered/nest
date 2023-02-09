#include <errno.h>
#include <stdlib.h>
#include "iter.h"
#include "lib_import.h"
#include "sequence.h"
#include "simple_types.h"
#include "str.h"
#include "global_consts.h"
#include "obj_ops.h"

Nst_Obj *nst_new_iter(Nst_FuncObj *start,
                      Nst_FuncObj *is_done,
                      Nst_FuncObj *get_val,
                      Nst_Obj     *value)
{
    Nst_IterObj *iter = ITER(nst_alloc_obj(
        sizeof(Nst_IterObj),
        nst_t.Iter,
        nst_destroy_iter));
    if ( iter == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    iter->start = start;
    iter->is_done = is_done;
    iter->get_val = get_val;
    iter->value = value;

    if ( NST_HAS_FLAG(start,   NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_HAS_FLAG(is_done, NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_HAS_FLAG(get_val, NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_HAS_FLAG(value,   NST_FLAG_GGC_IS_SUPPORTED) )
        NST_GGC_SUPPORT_INIT(iter, nst_traverse_iter, nst_track_iter);

    return OBJ(iter);
}

void nst_destroy_iter(Nst_IterObj *iter)
{
    nst_dec_ref(iter->start);
    nst_dec_ref(iter->is_done);
    nst_dec_ref(iter->get_val);
    nst_dec_ref(iter->value);
}

void nst_track_iter(Nst_IterObj* iter)
{
    if ( NST_HAS_FLAG(iter->start, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_add_tracked_object((Nst_GGCObj*)iter->start);
    }

    if ( NST_HAS_FLAG(iter->is_done, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_add_tracked_object((Nst_GGCObj*)iter->is_done);
    }

    if ( NST_HAS_FLAG(iter->get_val, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_add_tracked_object((Nst_GGCObj*)iter->get_val);
    }

    if ( NST_HAS_FLAG(iter->value, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_add_tracked_object((Nst_GGCObj*)iter->value);
    }
}

void nst_traverse_iter(Nst_IterObj* iter)
{
    NST_SET_FLAG(iter->start,   NST_FLAG_GGC_REACHABLE);
    NST_SET_FLAG(iter->is_done, NST_FLAG_GGC_REACHABLE);
    NST_SET_FLAG(iter->get_val, NST_FLAG_GGC_REACHABLE);
    NST_SET_FLAG(iter->value,   NST_FLAG_GGC_REACHABLE);
}

int _nst_start_iter(Nst_IterObj *iter, Nst_OpErr *err)
{
    Nst_Obj *result = nst_call_func(iter->start, &iter->value, err);

    if ( result == NULL )
    {
        return -1;
    }

    nst_dec_ref(result);
    return 0;
}

int _nst_is_done_iter(Nst_IterObj *iter, Nst_OpErr *err)
{
    Nst_Obj *result = nst_call_func(iter->is_done, &iter->value, err);

    if ( result == NULL )
    {
        return -1;
    }

    if ( nst_obj_cast(result, nst_t.Bool, NULL) == nst_c.b_true )
    {
        nst_dec_ref(nst_c.b_true);
        nst_dec_ref(result);
        return 1;
    }
    nst_dec_ref(nst_c.b_false);
    nst_dec_ref(result);
    return 0;
}

Nst_Obj *_nst_get_val_iter(Nst_IterObj *iter, Nst_OpErr *err)
{
    return nst_call_func(iter->get_val, &iter->value, err);
}

#if defined(_WIN32) || defined(WIN32)
#pragma warning( disable: 4100 )
#endif

NST_FUNC_SIGN(nst_num_iter_start)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = AS_INT(val->objs[1]);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_num_iter_is_done)
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

NST_FUNC_SIGN(nst_num_iter_get_val)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj *ob = nst_new_int(AS_INT(val->objs[0]));
    AS_INT(val->objs[0]) += AS_INT(val->objs[3]);
    return ob;
}

NST_FUNC_SIGN(nst_seq_iter_start)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_seq_iter_is_done)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    size_t seq_len = SEQ(objs[1])->len;

    if ( seq_len == 0 || AS_INT(objs[0]) >= (Nst_Int)seq_len )
    {
        NST_RETURN_TRUE;
    }
    else
    {
        NST_RETURN_FALSE;
    }
}

NST_FUNC_SIGN(nst_seq_iter_get_val)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_SeqObj *seq = SEQ(val->objs[1]);
    size_t idx = (size_t)AS_INT(val->objs[0]);

    if ( seq->len < idx )
    {
        NST_SET_VALUE_ERROR(_nst_format_error(
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

NST_FUNC_SIGN(nst_str_iter_start)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_str_iter_is_done)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    size_t str_len = STR(objs[1])->len;

    if ( str_len == 0 || AS_INT(objs[0]) >= (Nst_Int)str_len )
    {
        NST_RETURN_TRUE;
    }
    else
    {
        NST_RETURN_FALSE;
    }
}

NST_FUNC_SIGN(nst_str_iter_get_val)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    Nst_StrObj *str = STR(objs[1]);
    size_t idx = (size_t)AS_INT(objs[0]);

    if ( idx >= str->len )
    {
        NST_SET_VALUE_ERROR(_nst_format_error(
            _NST_EM_INDEX_OUT_OF_BOUNDS("Str"),
            "iu",
            idx,
            str->len));

        return NULL;
    }
    Nst_Obj *ob = nst_string_get_idx(objs[1], AS_INT(objs[0]));
    AS_INT(val->objs[0]) += 1;
    return ob;
}
