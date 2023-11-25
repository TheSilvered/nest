#include <errno.h>
#include <stdlib.h>
#include "iter.h"
#include "obj_ops.h"
#include "format.h"

Nst_Obj *Nst_iter_new(Nst_FuncObj *start, Nst_FuncObj *get_val, Nst_Obj *value)
{
    Nst_IterObj *iter = Nst_obj_alloc(Nst_IterObj, Nst_t.Iter);
    if (iter == NULL) {
        Nst_dec_ref(start);
        Nst_dec_ref(get_val);
        Nst_dec_ref(value);
        return NULL;
    }

    if (Nst_HAS_FLAG(start, Nst_FLAG_GGC_IS_SUPPORTED)
        || Nst_HAS_FLAG(get_val, Nst_FLAG_GGC_IS_SUPPORTED)
        || Nst_HAS_FLAG(value, Nst_FLAG_GGC_IS_SUPPORTED))
    {
        Nst_GGC_OBJ_INIT(iter);
    }

    iter->start = start;
    iter->get_val = get_val;
    iter->value = value;

    return OBJ(iter);
}

void _Nst_iter_destroy(Nst_IterObj *iter)
{
    Nst_dec_ref(iter->start);
    Nst_dec_ref(iter->get_val);
    Nst_dec_ref(iter->value);
}

void _Nst_iter_traverse(Nst_IterObj* iter)
{
    Nst_ggc_obj_reachable(iter->start);
    Nst_ggc_obj_reachable(iter->get_val);
    Nst_ggc_obj_reachable(iter->value);
}

i32 _Nst_iter_start(Nst_IterObj *iter)
{
    Nst_Obj *result = Nst_call_func(iter->start, 1, &iter->value);

    if (result == NULL)
        return -1;

    Nst_dec_ref(result);
    return 0;
}

Nst_Obj *_Nst_iter_get_val(Nst_IterObj *iter)
{
    return Nst_call_func(iter->get_val, 1, &iter->value);
}

Nst_FUNC_SIGN(Nst_iter_range_start)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = AS_INT(val->objs[1]);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(Nst_iter_range_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    i64 idx = AS_INT(objs[0]);
    i64 stop = AS_INT(objs[2]);
    i64 step = AS_INT(objs[3]);

    if (step > 0 && idx >= stop)
        return Nst_iend_ref();
    else if (step <= 0 && idx <= stop)
        return Nst_iend_ref();

    Nst_Obj *ob = Nst_int_new(idx);
    AS_INT(objs[0]) += step;
    return ob;
}

Nst_FUNC_SIGN(Nst_iter_seq_start)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(Nst_iter_seq_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_SeqObj *seq = SEQ(val->objs[1]);
    i64 idx = AS_INT(val->objs[0]);

    if (idx >= (i64)seq->len)
        return Nst_iend_ref();

    Nst_Obj *obj = seq->objs[idx];
    AS_INT(val->objs[0]) += 1;

    if (obj == Nst_c.IEnd_iend)
        Nst_RETURN_NULL;
    return Nst_inc_ref(obj);
}

Nst_FUNC_SIGN(Nst_iter_str_start)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(Nst_iter_str_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_SeqObj *val = SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    Nst_StrObj *str = STR(objs[1]);
    Nst_Obj *ch;

    if (!Nst_string_next_ch(str, (isize *)&AS_INT(objs[0]), &ch)) {
        if (AS_INT(objs[0]) == -1)
            return NULL;
        return Nst_iend_ref();
    }

    return ch;
}

Nst_FUNC_SIGN(Nst_iter_map_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = Nst_map_get_next_idx(-1, objs[1]);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(Nst_iter_map_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    i64 idx = AS_INT(objs[0]);

    if (idx == -1)
        return Nst_iend_ref();

    Nst_MapNode node = MAP(objs[1])->nodes[idx];
    Nst_Obj *arr = Nst_array_create_c("OO", node.key, node.value);

    AS_INT(objs[0]) = Nst_map_get_next_idx((i32)idx, MAP(objs[1]));
    return arr;
}
