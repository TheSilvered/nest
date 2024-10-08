#include "sequtil_i_functions.h"

Nst_Obj *NstC map_i_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    if (!Nst_iter_start(SEQ(args[0])->objs[0]))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC map_i_get_val(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj *iter = SEQ(args[0])->objs[0];
    Nst_Obj *func = SEQ(args[0])->objs[1];
    Nst_Obj *raw_value = Nst_iter_get_val(iter);
    if (raw_value == nullptr || raw_value == Nst_iend())
        return raw_value;

    Nst_Obj *mapped_value = Nst_func_call(FUNC(func), 1, &raw_value);
    Nst_dec_ref(raw_value);
    if (mapped_value == nullptr)
        return nullptr;
    if (mapped_value == Nst_iend()) {
        Nst_dec_ref(mapped_value);
        Nst_RETURN_NULL;
    }
    return mapped_value;
}

Nst_Obj *NstC slice_i_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    AS_INT(SEQ(args[0])->objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC slice_i_seq_get_val(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    i64 i = AS_INT(objs[0]);
    i64 max_i = AS_INT(objs[3]);

    if (i >= max_i)
        Nst_RETURN_IEND;

    i64 start = AS_INT(objs[1]);
    i64 step = AS_INT(objs[2]);
    Nst_SeqObj *seq = SEQ(objs[4]);

    Nst_Obj *obj = Nst_seq_get(seq, i * step + start);
    AS_INT(objs[0]) += 1;

    if (obj == nullptr)
        return obj;
    if (obj == Nst_iend()) {
        Nst_dec_ref(obj);
        Nst_RETURN_NULL;
    }
    return obj;
}

Nst_Obj *NstC slice_i_str_get_val(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    i64 i = AS_INT(objs[0]);
    i64 max_i = AS_INT(objs[3]);

    if (i >= max_i)
        Nst_RETURN_IEND;

    i64 start = AS_INT(objs[1]);
    i64 step = AS_INT(objs[2]);
    Nst_StrObj *seq = STR(objs[4]);

    Nst_Obj *obj = Nst_str_get(seq, i * step + start);
    AS_INT(objs[0]) += 1;

    if (obj == nullptr)
        return obj;
    return obj;
}

Nst_Obj *NstC filter_i_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    if (!Nst_iter_start(SEQ(args[0])->objs[0]))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC filter_i_get_val(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj *iter = SEQ(args[0])->objs[0];
    Nst_Obj *func = SEQ(args[0])->objs[1];
    Nst_Obj *value;

    while (true) {
        value = Nst_iter_get_val(iter);
        if (value == nullptr || value == Nst_iend())
            return value;

        Nst_Obj *result_obj = Nst_func_call(FUNC(func), 1, &value);
        if (result_obj == nullptr)
            return nullptr;
        bool result = Nst_obj_to_bool(result_obj);
        Nst_dec_ref(result_obj);
        if (result)
            break;
        Nst_dec_ref(value);
    }

    return value;
}

Nst_Obj *NstC reverse_i_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = (isize)SEQ(objs[1])->len - 1;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC reverse_i_get_val(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_SeqObj *seq = SEQ(objs[1]);
    i64 idx = AS_INT(objs[0]);

    if (idx <= -1)
        Nst_RETURN_IEND;

    Nst_Obj *res = Nst_seq_get(seq, idx);

    if (res == nullptr)
        return nullptr;

    i64 len = SEQ(objs[1])->len;
    AS_INT(objs[0]) -= 1;

    if (AS_INT(objs[0]) >= len)
        AS_INT(objs[0]) = len - 1;

    if (res == Nst_iend()) {
        Nst_dec_ref(res);
        Nst_RETURN_NULL;
    }
    return res;
}
