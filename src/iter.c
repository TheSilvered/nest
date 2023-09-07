#include <errno.h>
#include <stdlib.h>
#include "iter.h"
#include "obj_ops.h"
#include "format.h"

Nst_Obj *Nst_iter_new(Nst_FuncObj *start, Nst_FuncObj *is_done,
                      Nst_FuncObj *get_val, Nst_Obj *value)
{
    Nst_IterObj *iter = Nst_obj_alloc(Nst_IterObj, Nst_t.Iter);
    if (iter == NULL) {
        Nst_dec_ref(start);
        Nst_dec_ref(is_done);
        Nst_dec_ref(get_val);
        Nst_dec_ref(value);
        return NULL;
    }

    if (Nst_HAS_FLAG(start, Nst_FLAG_GGC_IS_SUPPORTED)
        || Nst_HAS_FLAG(is_done, Nst_FLAG_GGC_IS_SUPPORTED)
        || Nst_HAS_FLAG(get_val, Nst_FLAG_GGC_IS_SUPPORTED)
        || Nst_HAS_FLAG(value, Nst_FLAG_GGC_IS_SUPPORTED))
    {
        Nst_GGC_OBJ_INIT(iter);
    }

    iter->start = start;
    iter->is_done = is_done;
    iter->get_val = get_val;
    iter->value = value;

    return OBJ(iter);
}

void _Nst_iter_destroy(Nst_IterObj *iter)
{
    Nst_dec_ref(iter->start);
    Nst_dec_ref(iter->is_done);
    Nst_dec_ref(iter->get_val);
    Nst_dec_ref(iter->value);
}

void _Nst_iter_traverse(Nst_IterObj* iter)
{
    Nst_ggc_obj_reachable(iter->start);
    Nst_ggc_obj_reachable(iter->is_done);
    Nst_ggc_obj_reachable(iter->get_val);
    Nst_ggc_obj_reachable(iter->value);
}

i32 _Nst_iter_start(Nst_IterObj *iter)
{
    Nst_Obj *result = Nst_call_func(iter->start, &iter->value);

    if (result == NULL)
        return -1;

    Nst_dec_ref(result);
    return 0;
}

i32 _Nst_iter_is_done(Nst_IterObj *iter)
{
    Nst_Obj *result = Nst_call_func(iter->is_done, &iter->value);

    if (result == NULL)
        return -1;

    if (Nst_obj_to_bool(result)) {
        Nst_dec_ref(result);
        return 1;
    }
    Nst_dec_ref(result);
    return 0;
}

Nst_Obj *_Nst_iter_get_val(Nst_IterObj *iter)
{
    return Nst_call_func(iter->get_val, &iter->value);
}

Nst_FUNC_SIGN(Nst_iter_range_start)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = AS_INT(val->objs[1]);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(Nst_iter_range_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    i64 idx = AS_INT(objs[0]);
    i64 stop = AS_INT(objs[2]);
    i64 step = AS_INT(objs[3]);

    if (step > 0)
        Nst_RETURN_COND(idx >= stop);
    else
        Nst_RETURN_COND(idx <= stop);
}

Nst_FUNC_SIGN(Nst_iter_range_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj *ob = Nst_int_new(AS_INT(val->objs[0]));
    AS_INT(val->objs[0]) += AS_INT(val->objs[3]);
    return ob;
}

Nst_FUNC_SIGN(Nst_iter_seq_start)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(Nst_iter_seq_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    usize seq_len = SEQ(objs[1])->len;

    if (seq_len == 0 || AS_INT(objs[0]) >= (i64)seq_len)
        Nst_RETURN_TRUE;
    else
        Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(Nst_iter_seq_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_SeqObj *seq = SEQ(val->objs[1]);
    i64 idx = AS_INT(val->objs[0]);

    if ((i64)seq->len < idx) {
        Nst_set_value_error(Nst_sprintf(
            seq->type == Nst_t.Array ? _Nst_EM_INDEX_OUT_OF_BOUNDS("Array")
                                     : _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            idx,
            seq->len));

        return NULL;
    }

    Nst_Obj *obj = SEQ(val->objs[1])->objs[AS_INT(val->objs[0])];
    AS_INT(val->objs[0]) += 1;
    return Nst_inc_ref(obj);
}

Nst_FUNC_SIGN(Nst_iter_str_start)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(Nst_iter_str_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    usize str_len = STR(objs[1])->len;

    if (str_len == 0 || AS_INT(objs[0]) >= (i64)str_len)
        Nst_RETURN_TRUE;
    else
        Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(Nst_iter_str_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    Nst_StrObj *str = STR(objs[1]);
    i64 idx = AS_INT(objs[0]);

    if (idx >= (i64)str->len) {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Str"),
            idx,
            str->len));

        return NULL;
    }
    Nst_Obj *ob = Nst_string_get(objs[1], AS_INT(objs[0]));
    AS_INT(val->objs[0]) += 1;
    return ob;
}

Nst_FUNC_SIGN(Nst_iter_map_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = Nst_map_get_next_idx(-1, objs[1]);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(Nst_iter_map_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    if (AS_INT(objs[0]) == -1)
        Nst_RETURN_TRUE;
    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(Nst_iter_map_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;

    Nst_SeqObj *arr = SEQ(Nst_array_new(2));

    if (AS_INT(objs[0]) == -1) {
        Nst_seq_set(arr, 0, Nst_null());
        Nst_seq_set(arr, 1, Nst_null());
    } else {
        Nst_MapNode node = MAP(objs[1])->nodes[AS_INT(objs[0])];
        Nst_seq_set(arr, 0, node.key);
        Nst_seq_set(arr, 1, node.value);
    }
    AS_INT(objs[0]) = Nst_map_get_next_idx((i32)AS_INT(objs[0]), MAP(objs[1]));
    return OBJ(arr);
}
