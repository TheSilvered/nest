#include "itutil_functions.h"

ItutilFunctions itutil_functions;

bool init_itutil_functions()
{
    itutil_functions.count_start = Nst_func_new_c(1, count_start);
    itutil_functions.count_next = Nst_func_new_c(1, count_next);
    itutil_functions.cycle_start = Nst_func_new_c(1, cycle_start);
    itutil_functions.cycle_str_next = Nst_func_new_c(1, cycle_str_next);
    itutil_functions.cycle_seq_next = Nst_func_new_c(1, cycle_seq_next);
    itutil_functions.cycle_iter_start = Nst_func_new_c(1, cycle_iter_start);
    itutil_functions.cycle_iter_next = Nst_func_new_c(1, cycle_iter_next);
    itutil_functions.repeat_start = Nst_func_new_c(1, repeat_start);
    itutil_functions.repeat_next = Nst_func_new_c(1, repeat_next);
    itutil_functions.chain_start = Nst_func_new_c(1, chain_start);
    itutil_functions.chain_next = Nst_func_new_c(1, chain_next);
    itutil_functions.zip_start = Nst_func_new_c(1, zip_start);
    itutil_functions.zip_next = Nst_func_new_c(1, zip_next);
    itutil_functions.enumerate_start = Nst_func_new_c(1, enumerate_start);
    itutil_functions.enumerate_next = Nst_func_new_c(1, enumerate_next);
    itutil_functions.map_iter_start = Nst_func_new_c(1, map_iter_start);
    itutil_functions.keys_next = Nst_func_new_c(1, keys_next);
    itutil_functions.values_next = Nst_func_new_c(1, values_next);
    itutil_functions.batch_start = Nst_func_new_c(1, batch_start);
    itutil_functions.batch_next = Nst_func_new_c(1, batch_next);

    if (Nst_error_occurred()) {
        free_itutil_functions();
        return false;
    }
    return true;
}

void free_itutil_functions()
{
    Nst_ndec_ref(itutil_functions.count_start);
    Nst_ndec_ref(itutil_functions.count_next);
    Nst_ndec_ref(itutil_functions.cycle_start);
    Nst_ndec_ref(itutil_functions.cycle_str_next);
    Nst_ndec_ref(itutil_functions.cycle_seq_next);
    Nst_ndec_ref(itutil_functions.cycle_iter_start);
    Nst_ndec_ref(itutil_functions.cycle_iter_next);
    Nst_ndec_ref(itutil_functions.repeat_start);
    Nst_ndec_ref(itutil_functions.repeat_next);
    Nst_ndec_ref(itutil_functions.chain_start);
    Nst_ndec_ref(itutil_functions.chain_next);
    Nst_ndec_ref(itutil_functions.zip_start);
    Nst_ndec_ref(itutil_functions.zip_next);
    Nst_ndec_ref(itutil_functions.enumerate_start);
    Nst_ndec_ref(itutil_functions.enumerate_next);
    Nst_ndec_ref(itutil_functions.map_iter_start);
    Nst_ndec_ref(itutil_functions.keys_next);
    Nst_ndec_ref(itutil_functions.values_next);
    Nst_ndec_ref(itutil_functions.batch_start);
    Nst_ndec_ref(itutil_functions.batch_next);
}

void destroy_cycle_data(Nst_Obj *obj)
{
    CycleData *data = (CycleData *)Nst_obj_custom_data(obj);
    Nst_dec_ref(data->iterable);
}

void destroy_repeat_data(Nst_Obj *obj)
{
    RepeatData *data = (RepeatData *)Nst_obj_custom_data(obj);
    Nst_dec_ref(data->item);
}

void destroy_chain_data(Nst_Obj *obj)
{
    ChainData *data = (ChainData *)Nst_obj_custom_data(obj);
    Nst_dec_ref(data->main_iter);
    Nst_ndec_ref(data->current_iter);
}

void destroy_zip_data(Nst_Obj *obj)
{
    ZipData *data = (ZipData *)Nst_obj_custom_data(obj);
    Nst_Obj **iterators = data->iterators;
    for (usize i = 0, n = data->count; i < n; i++)
        Nst_dec_ref(iterators[i]);
}

void destroy_enumerate_data(Nst_Obj *obj)
{
    EnumerateData *data = (EnumerateData *)Nst_obj_custom_data(obj);
    Nst_dec_ref(data->iterator);
}

void destroy_map_iter_data(Nst_Obj *obj)
{
    MapIterData *data = (MapIterData *)Nst_obj_custom_data(obj);
    Nst_dec_ref(data->map);
}

void destroy_batch_data(Nst_Obj *obj)
{
    BatchData *data = (BatchData *)Nst_obj_custom_data(obj);
    Nst_dec_ref(data->iterator);
    Nst_ndec_ref(data->padding_obj);
}

// --------------------------------- Count --------------------------------- //
Nst_Obj *NstC count_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    CountData *data = (CountData *)Nst_obj_custom_data(args[0]);
    data->idx = data->start;
    return Nst_null_ref();
}

Nst_Obj *NstC count_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    CountData *data = (CountData *)Nst_obj_custom_data(args[0]);
    Nst_Obj *ob = Nst_int_new(data->idx);
    data->idx += data->step;
    return ob;
}

// ----------------------------- Cycle Iterable ---------------------------- //
Nst_Obj *NstC cycle_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    CycleData *data = (CycleData *)Nst_obj_custom_data(args[0]);
    data->idx = 0;
    return Nst_null_ref();
}

Nst_Obj *NstC cycle_str_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    CycleData *data = (CycleData *)Nst_obj_custom_data(args[0]);
    Nst_Obj *ob = Nst_str_get_obj(data->iterable, data->idx);
    data->idx += 1;
    data->idx %= Nst_str_char_len(data->iterable);
    return ob;
}

Nst_Obj *NstC cycle_seq_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    CycleData *data = (CycleData *)Nst_obj_custom_data(args[0]);
    Nst_Obj *ob = Nst_seq_get(data->iterable, data->idx);
    data->idx += 1;
    data->idx %= Nst_seq_len(data->iterable);
    if (ob == Nst_iend()) {
        Nst_dec_ref(ob);
        return Nst_null_ref();
    }
    return ob;
}

// ----------------------------- Cycle Iterator ---------------------------- //
Nst_Obj *NstC cycle_iter_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    if (!Nst_iter_start(args[0]))
        return nullptr;
    return Nst_null_ref();
}

Nst_Obj *NstC cycle_iter_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj *obj = Nst_iter_next(args[0]);
    if (obj != Nst_iend())
        return obj;
    Nst_dec_ref(obj);

    if (!Nst_iter_start(args[0]))
        return nullptr;

    obj = Nst_iter_next(args[0]);
    if (obj == Nst_iend()) {
        Nst_dec_ref(obj);
        return Nst_null_ref();
    }
    return obj;
}

// --------------------------------- Repeat -------------------------------- //
Nst_Obj *NstC repeat_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    RepeatData *data = (RepeatData *)Nst_obj_custom_data(args[0]);
    data->count = 0;
    if (data->item == Nst_iend()) {
        Nst_dec_ref(data->item);
        data->item = Nst_null_ref();
    }
    return Nst_null_ref();
}

Nst_Obj *NstC repeat_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    RepeatData *data = (RepeatData *)Nst_obj_custom_data(args[0]);
    i64 max_count = data->max_count;

    if (max_count < 0)
        return Nst_inc_ref(data->item);

    if (data->count >= max_count)
        return Nst_iend_ref();
    data->count += 1;
    return Nst_inc_ref(data->item);
}

// --------------------------------- Chain --------------------------------- //

Nst_Obj *NstC chain_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    ChainData *data = (ChainData *)Nst_obj_custom_data(args[0]);
    Nst_Obj *main_iter = data->main_iter;

    if (!Nst_iter_start(main_iter))
        return nullptr;
    Nst_ndec_ref(data->current_iter);
    data->current_iter = nullptr;

    return Nst_null_ref();
}

Nst_Obj *NstC chain_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    ChainData *data = (ChainData *)Nst_obj_custom_data(args[0]);
    Nst_Obj *main_iter = data->main_iter;
    Nst_Obj *val = data->current_iter == nullptr
        ? Nst_iend_ref()
        : Nst_iter_next(data->current_iter);

    // If `val` is nullptr as a result of an error, the loop does not enter or
    // finishes and nullptr is returned at the end of the function
    while (val == Nst_iend()) {
        Nst_dec_ref(val);
        // current_iter either does not exist or has finished so get a new
        // iterable from main_iter
        Nst_Obj *next_iterable = Nst_iter_next(main_iter);
        // the main iterator has finished, end the chain iterator
        if (next_iterable == Nst_iend())
            return next_iterable;
        else if (next_iterable == nullptr)
            return nullptr;

        Nst_Obj *next_iter = Nst_obj_cast(next_iterable, Nst_type()->Iter);
        Nst_dec_ref(next_iterable);
        if (next_iter == nullptr)
            return nullptr;

        Nst_ndec_ref(data->current_iter);
        data->current_iter = next_iter;
        if (!Nst_iter_start(data->current_iter))
            return nullptr;

        val = Nst_iter_next(data->current_iter);
        // if val is nullptr the loop finishes and nullptr is returned, no need
        // to check
    }

    return val;
}

// ---------------------------------- Zip ---------------------------------- //

Nst_Obj *NstC zip_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    ZipData *data = (ZipData *)Nst_obj_custom_data(args[0]);
    Nst_Obj **iterators = data->iterators;
    for (usize i = 0, n = data->count; i < n; i++) {
        if (!Nst_iter_start(iterators[i]))
            return nullptr;
    }
    return Nst_null_ref();
}

Nst_Obj *NstC zip_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    ZipData *data = (ZipData *)Nst_obj_custom_data(args[0]);

    usize arr_len = data->count;
    Nst_Obj **iterators = data->iterators;
    Nst_Obj *arr = Nst_array_new(arr_len);
    for (usize i = 0; i < arr_len; i++) {
        Nst_Obj *res = Nst_iter_next(iterators[i]);
        if (res == nullptr || res == Nst_iend()) {
            Nst_dec_ref(arr);
            return res;
        }
        Nst_seq_setnf(arr, i, res);
    }

    return arr;
}

// ------------------------------- Enumerate ------------------------------- //
Nst_Obj *NstC enumerate_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    EnumerateData *data = (EnumerateData *)Nst_obj_custom_data(args[0]);
    data->idx = data->start;
    if (!Nst_iter_start(data->iterator))
        return nullptr;
    return Nst_null_ref();
}

Nst_Obj *NstC enumerate_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    EnumerateData *data = (EnumerateData *)Nst_obj_custom_data(args[0]);

    Nst_Obj *res = Nst_iter_next(data->iterator);
    if (res == nullptr || res == Nst_iend())
        return res;

    Nst_Obj *arr;
    if (data->invert_order)
        arr = Nst_array_create_c("oI", res, data->idx);
    else
        arr = Nst_array_create_c("Io", data->idx, res);

    if (arr == nullptr)
        return nullptr;

    data->idx += data->step;
    return arr;
}

// ----------------------------- Map iterators ----------------------------- //

Nst_Obj *NstC map_iter_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    MapIterData *data = (MapIterData *)Nst_obj_custom_data(args[0]);
    data->idx = -1;
    return Nst_null_ref();
}

Nst_Obj *NstC keys_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    MapIterData *data = (MapIterData *)Nst_obj_custom_data(args[0]);

    Nst_Obj *key;
    data->idx = Nst_map_next(data->idx, data->map, &key, nullptr);

    if (data->idx == -1)
        return Nst_iend_ref();

    if (key == Nst_iend())
        return Nst_null_ref();

    return Nst_inc_ref(key);
}

Nst_Obj *NstC values_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    MapIterData *data = (MapIterData *)Nst_obj_custom_data(args[0]);

    Nst_Obj *value;
    data->idx = Nst_map_next(data->idx, data->map, nullptr, &value);

    if (data->idx == -1)
        return Nst_iend_ref();

    if (value == Nst_iend())
        return Nst_null_ref();
    return Nst_inc_ref(value);
}

// --------------------------------- Batch --------------------------------- //

Nst_Obj *NstC batch_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    BatchData *data = (BatchData *)Nst_obj_custom_data(args[0]);
    if (!Nst_iter_start(data->iterator))
        return nullptr;
    data->iter_ended = false;
    return Nst_null_ref();
}

Nst_Obj *NstC batch_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    BatchData *data = (BatchData *)Nst_obj_custom_data(args[0]);

    if (data->iter_ended)
        return Nst_iend_ref();

    Nst_Obj *iter = data->iterator;
    usize batch_size = data->batch_size;

    Nst_Obj *batch = Nst_array_new(batch_size);
    if (batch == nullptr)
        return nullptr;

    usize i = 0;
    for (; i < batch_size; i++) {
        Nst_Obj *obj = Nst_iter_next(iter);
        if (obj == nullptr) {
            Nst_dec_ref(batch);
            return nullptr;
        } else if (obj == Nst_iend()) {
            Nst_dec_ref(obj);
            data->iter_ended = true;
            break;
        }
        Nst_seq_setnf(batch, i, obj);
    }

    if (i == 0) {
        Nst_dec_ref(batch);
        return Nst_iend_ref();
    }

    if (data->padding_obj != nullptr) {
        Nst_Obj *padding = data->padding_obj;
        for (; i < batch_size; i++)
            Nst_seq_setf(batch, i, padding);
    } else {
        Nst_Obj *resized_batch =
            Nst_array_from_objs(i, _Nst_seq_objs(batch));
        Nst_dec_ref(batch);
        batch = resized_batch;
    }
    return batch;
}
