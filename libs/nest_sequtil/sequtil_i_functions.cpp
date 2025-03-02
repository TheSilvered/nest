#include "sequtil_i_functions.h"

void destroy_slice_data(Nst_Obj *data_obj)
{
    SliceData *data = (SliceData *)Nst_obj_custom_data(data_obj);
    Nst_dec_ref(data->obj);
}

void destroy_reverse_data(Nst_Obj *data_obj)
{
    ReverseData *data = (ReverseData *)Nst_obj_custom_data(data_obj);
    Nst_dec_ref(data->seq);
}

Nst_Obj *NstC map_i_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    if (!Nst_iter_start(c_args[0]))
        return nullptr;
    return Nst_null_ref();
}

Nst_Obj *NstC map_i_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    Nst_Obj *iter = c_args[0];
    Nst_Obj *func = c_args[1];
    Nst_Obj *raw_value = Nst_iter_next(iter);
    if (raw_value == nullptr || raw_value == Nst_iend())
        return raw_value;

    Nst_Obj *mapped_value = Nst_func_call(func, 1, &raw_value);
    Nst_dec_ref(raw_value);
    if (mapped_value == nullptr)
        return nullptr;
    if (mapped_value == Nst_iend()) {
        Nst_dec_ref(mapped_value);
        return Nst_null_ref();
    }
    return mapped_value;
}

Nst_Obj *NstC slice_i_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    SliceData *data = (SliceData *)Nst_obj_custom_data(args[0]);
    data->idx = 0;
    return Nst_null_ref();
}

Nst_Obj *NstC slice_i_seq_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    SliceData *data = (SliceData *)Nst_obj_custom_data(args[0]);
    isize idx = data->idx;
    isize max_idx = data->max_idx;

    if (idx >= max_idx)
        return Nst_iend_ref();

    Nst_Obj *obj = Nst_seq_get(data->obj, idx * data->step + data->start);
    data->idx += 1;

    if (obj == nullptr)
        return obj;
    if (obj == Nst_iend()) {
        Nst_dec_ref(obj);
        return Nst_null_ref();
    }
    return obj;
}

Nst_Obj *NstC slice_i_str_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    SliceData *data = (SliceData *)Nst_obj_custom_data(args[0]);
    isize idx = data->idx;
    isize max_idx = data->max_idx;

    if (idx >= max_idx)
        return Nst_iend_ref();

    Nst_Obj *obj = Nst_str_get_obj(data->obj, idx * data->step + data->start);
    data->idx += 1;

    if (obj == nullptr)
        return obj;
    return obj;
}

Nst_Obj *NstC filter_i_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    if (!Nst_iter_start(c_args[0]))
        return nullptr;
    return Nst_null_ref();
}

Nst_Obj *NstC filter_i_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    Nst_Obj *iter = c_args[0];
    Nst_Obj *func = c_args[1];
    Nst_Obj *value;

    while (true) {
        value = Nst_iter_next(iter);
        if (value == nullptr || value == Nst_iend())
            return value;

        Nst_Obj *result_obj = Nst_func_call(func, 1, &value);
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
    ReverseData *data = (ReverseData *)Nst_obj_custom_data(args[0]);
    data->idx = (isize)Nst_seq_len(data->seq) - 1;
    return Nst_null_ref();
}

Nst_Obj *NstC reverse_i_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    ReverseData *data = (ReverseData *)Nst_obj_custom_data(args[0]);
    i64 idx = data->idx;

    if (idx <= -1)
        return Nst_iend_ref();

    Nst_Obj *res = Nst_seq_get(data->seq, idx);

    if (res == nullptr)
        return nullptr;
    data->idx -= 1;

    if (res == Nst_iend()) {
        Nst_dec_ref(res);
        return Nst_null_ref();
    }
    return res;
}
