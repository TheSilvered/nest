#include <errno.h>
#include <stdlib.h>
#include "iter.h"
#include "lib_import.h"
#include "sequence.h"
#include "simple_types.h"
#include "str.h"
#include "global_consts.h"

Nst_Obj *nst_new_iter(
    Nst_FuncObj *start,
    Nst_FuncObj *advance,
    Nst_FuncObj *is_done,
    Nst_FuncObj *get_val,
    Nst_Obj *value)
{
    Nst_IterObj *iter = ITER(nst_alloc_obj(
        sizeof(Nst_IterObj),
        nst_t.Iter,
        nst_destroy_iter
    ));
    if ( iter == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    iter->start = start;
    iter->advance = advance;
    iter->is_done = is_done;
    iter->get_val = get_val;
    iter->value = value;

    if ( NST_HAS_FLAG(start,   NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_HAS_FLAG(advance, NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_HAS_FLAG(is_done, NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_HAS_FLAG(get_val, NST_FLAG_GGC_IS_SUPPORTED) ||
         NST_HAS_FLAG(value,   NST_FLAG_GGC_IS_SUPPORTED) )
        NST_GGC_SUPPORT_INIT(iter, nst_traverse_iter, nst_track_iter);

    return OBJ(iter);
}

void nst_destroy_iter(Nst_IterObj *iter)
{
    nst_dec_ref(iter->start);
    nst_dec_ref(iter->advance);
    nst_dec_ref(iter->is_done);
    nst_dec_ref(iter->get_val);
    nst_dec_ref(iter->value);
}

void nst_traverse_iter(Nst_IterObj* iter)
{
    if ( NST_HAS_FLAG(iter->start, NST_FLAG_GGC_IS_SUPPORTED) )
        iter->start->traverse_func(OBJ(iter->start));

    if ( NST_HAS_FLAG(iter->advance, NST_FLAG_GGC_IS_SUPPORTED) )
        iter->advance->traverse_func(OBJ(iter->advance));

    if ( NST_HAS_FLAG(iter->is_done, NST_FLAG_GGC_IS_SUPPORTED) )
        iter->is_done->traverse_func(OBJ(iter->is_done));

    if ( NST_HAS_FLAG(iter->get_val, NST_FLAG_GGC_IS_SUPPORTED) )
        iter->get_val->traverse_func(OBJ(iter->get_val));

    if ( NST_HAS_FLAG(iter->value, NST_FLAG_GGC_IS_SUPPORTED) )
        ((Nst_GGCObj *)iter->value)->traverse_func(iter->value);
}

void nst_track_iter(Nst_IterObj* iter)
{
    if ( NST_HAS_FLAG(iter->start, NST_FLAG_GGC_IS_SUPPORTED) )
        nst_add_tracked_object((Nst_GGCObj *)iter->start);

    if ( NST_HAS_FLAG(iter->advance, NST_FLAG_GGC_IS_SUPPORTED) )
        nst_add_tracked_object((Nst_GGCObj *)iter->advance);

    if ( NST_HAS_FLAG(iter->is_done, NST_FLAG_GGC_IS_SUPPORTED) )
        nst_add_tracked_object((Nst_GGCObj *)iter->is_done);

    if ( NST_HAS_FLAG(iter->get_val, NST_FLAG_GGC_IS_SUPPORTED) )
        nst_add_tracked_object((Nst_GGCObj *)iter->get_val);

    if ( NST_HAS_FLAG(iter->value, NST_FLAG_GGC_IS_SUPPORTED) )
        nst_add_tracked_object((Nst_GGCObj *)iter->value);
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

NST_FUNC_SIGN(nst_num_iter_advance)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) += AS_INT(val->objs[3]);
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
        NST_RETURN_COND(idx >= stop);
    else
        NST_RETURN_COND(idx <= stop);
}

NST_FUNC_SIGN(nst_num_iter_get_val)
{
    Nst_SeqObj *val = SEQ(args[0]);
    return nst_new_int(AS_INT(val->objs[0]));
}

NST_FUNC_SIGN(nst_seq_iter_start)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_seq_iter_advance)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) += 1;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_seq_iter_is_done)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    size_t seq_len = SEQ(objs[1])->len;

    if ( seq_len == 0 || AS_INT(objs[0]) >= (Nst_Int)seq_len )
        NST_RETURN_TRUE;
    else
        NST_RETURN_FALSE;
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
            seq->len
        ));

        return NULL;
    }

    Nst_Obj *obj = SEQ(val->objs[1])->objs[AS_INT(val->objs[0])];
    return nst_inc_ref(obj);
}

NST_FUNC_SIGN(nst_str_iter_start)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_str_iter_advance)
{
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) += 1;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_str_iter_is_done)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    size_t str_len = STR(objs[1])->len;

    if ( str_len == 0 || AS_INT(objs[0]) >= (Nst_Int)str_len )
        NST_RETURN_TRUE;
    else
        NST_RETURN_FALSE;
}

NST_FUNC_SIGN(nst_str_iter_get_val)
{
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    Nst_StrObj *str = STR(objs[1]);
    size_t idx = (size_t)AS_INT(objs[0]);

    if ( str->len < idx )
    {
        NST_SET_VALUE_ERROR(_nst_format_error(
            _NST_EM_INDEX_OUT_OF_BOUNDS("Str"),
            "iu",
            idx,
            str->len
        ));

        return NULL;
    }

    return nst_string_get_idx(objs[1], AS_INT(objs[0]));
}
