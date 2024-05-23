#include "sequtil_i_functions.h"

Nst_FUNC_SIGN(map_i_start)
{
    Nst_UNUSED(arg_num);
    if (!Nst_iter_start(SEQ(args[0])->objs[0]))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(map_i_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj *iter = SEQ(args[0])->objs[0];
    Nst_Obj *func = SEQ(args[0])->objs[1];
    Nst_Obj *raw_value = Nst_iter_get_val(iter);
    if (raw_value == nullptr || raw_value == Nst_iend())
        return raw_value;

    Nst_Obj *mapped_value = Nst_call_func(FUNC(func), 1, &raw_value);
    Nst_dec_ref(raw_value);
    if (mapped_value == nullptr)
        return nullptr;
    if (mapped_value == Nst_iend()) {
        Nst_dec_ref(mapped_value);
        Nst_RETURN_NULL;
    }
    return mapped_value;
}

Nst_FUNC_SIGN(slice_i_start)
{
    Nst_UNUSED(arg_num);
    AS_INT(SEQ(args[0])->objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(slice_i_seq_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    i64 i = AS_INT(objs[0]);
    i64 max_i = AS_INT(objs[3]);

    if (i >= max_i)
        return Nst_iend_ref();

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

Nst_FUNC_SIGN(slice_i_str_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    i64 i = AS_INT(objs[0]);
    i64 max_i = AS_INT(objs[3]);

    if (i >= max_i)
        return Nst_iend_ref();

    i64 start = AS_INT(objs[1]);
    i64 step = AS_INT(objs[2]);
    Nst_StrObj *seq = STR(objs[4]);

    Nst_Obj *obj = Nst_string_get(seq, i * step + start);
    AS_INT(objs[0]) += 1;

    if (obj == nullptr)
        return obj;
    return obj;
}

Nst_FUNC_SIGN(filter_i_start)
{
    Nst_UNUSED(arg_num);
    if (!Nst_iter_start(SEQ(args[0])->objs[0]))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(filter_i_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj *iter = SEQ(args[0])->objs[0];
    Nst_Obj *func = SEQ(args[0])->objs[1];
    Nst_Obj *value;

    while (true) {
        value = Nst_iter_get_val(iter);
        if (value == nullptr || value == Nst_iend())
            return value;

        Nst_Obj *result_obj = Nst_call_func(FUNC(func), 1, &value);
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
