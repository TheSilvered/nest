#include "itutil_functions.h"

ItutilFunctions itutil_functions;

bool init_itutil_functions()
{
    itutil_functions.count_start =
        Nst_func_new_c(1, count_start);
    itutil_functions.count_next =
        Nst_func_new_c(1, count_next);
    itutil_functions.cycle_str_start =
        Nst_func_new_c(1, cycle_str_start);
    itutil_functions.cycle_str_next =
        Nst_func_new_c(1, cycle_str_next);
    itutil_functions.cycle_seq_start =
        Nst_func_new_c(1, cycle_seq_start);
    itutil_functions.cycle_seq_next =
        Nst_func_new_c(1, cycle_seq_next);
    itutil_functions.cycle_iter_start =
        Nst_func_new_c(1, cycle_iter_start);
    itutil_functions.cycle_iter_next =
        Nst_func_new_c(1, cycle_iter_next);
    itutil_functions.repeat_start =
        Nst_func_new_c(1, repeat_start);
    itutil_functions.repeat_next =
        Nst_func_new_c(1, repeat_next);
    itutil_functions.chain_start =
        Nst_func_new_c(1, chain_start);
    itutil_functions.chain_next =
        Nst_func_new_c(1, chain_next);
    itutil_functions.zip_start =
        Nst_func_new_c(1, zip_start);
    itutil_functions.zip_next =
        Nst_func_new_c(1, zip_next);
    itutil_functions.zipn_start =
        Nst_func_new_c(1, zipn_start);
    itutil_functions.zipn_next =
        Nst_func_new_c(1, zipn_next);
    itutil_functions.enumerate_start =
        Nst_func_new_c(1, enumerate_start);
    itutil_functions.enumerate_next =
        Nst_func_new_c(1, enumerate_next);
    itutil_functions.keys_next =
        Nst_func_new_c(1, keys_next);
    itutil_functions.values_next =
        Nst_func_new_c(1, values_next);
    itutil_functions.batch_start =
        Nst_func_new_c(1, batch_start);
    itutil_functions.batch_next =
        Nst_func_new_c(1, batch_next);
    itutil_functions.batch_padded_next =
        Nst_func_new_c(1, batch_padded_next);

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
    Nst_ndec_ref(itutil_functions.cycle_str_start);
    Nst_ndec_ref(itutil_functions.cycle_str_next);
    Nst_ndec_ref(itutil_functions.cycle_seq_start);
    Nst_ndec_ref(itutil_functions.cycle_seq_next);
    Nst_ndec_ref(itutil_functions.cycle_iter_start);
    Nst_ndec_ref(itutil_functions.cycle_iter_next);
    Nst_ndec_ref(itutil_functions.repeat_start);
    Nst_ndec_ref(itutil_functions.repeat_next);
    Nst_ndec_ref(itutil_functions.chain_start);
    Nst_ndec_ref(itutil_functions.chain_next);
    Nst_ndec_ref(itutil_functions.zip_start);
    Nst_ndec_ref(itutil_functions.zip_next);
    Nst_ndec_ref(itutil_functions.zipn_start);
    Nst_ndec_ref(itutil_functions.zipn_next);
    Nst_ndec_ref(itutil_functions.enumerate_start);
    Nst_ndec_ref(itutil_functions.enumerate_next);
    Nst_ndec_ref(itutil_functions.keys_next);
    Nst_ndec_ref(itutil_functions.values_next);
    Nst_ndec_ref(itutil_functions.batch_start);
    Nst_ndec_ref(itutil_functions.batch_next);
    Nst_ndec_ref(itutil_functions.batch_padded_next);
}

// --------------------------------- Count --------------------------------- //
Nst_Obj *NstC count_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    AS_INT(c_args[0]) = AS_INT(c_args[1]);
    Nst_RETURN_NULL;
}

Nst_Obj *NstC count_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    Nst_Obj *ob = Nst_int_new(AS_INT(c_args[0]));
    AS_INT(c_args[0]) += AS_INT(c_args[2]);
    return ob;
}

// ------------------------------ Cycle String ----------------------------- //
Nst_Obj *NstC cycle_str_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    AS_INT(c_args[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC cycle_str_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    Nst_Obj *ob = Nst_str_get(c_args[1], AS_INT(c_args[0]));
    AS_INT(c_args[0]) += 1;
    AS_INT(c_args[0]) %= STR(c_args[1])->char_len;
    return ob;
}

// ----------------------------- Cycle Sequence ---------------------------- //
Nst_Obj *NstC cycle_seq_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    AS_INT(c_args[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC cycle_seq_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    Nst_Obj *ob = Nst_seq_get(c_args[1], AS_INT(c_args[0]));
    AS_INT(c_args[0]) += 1;
    AS_INT(c_args[0]) %= Nst_seq_len(c_args[1]);
    return ob;
}

// ----------------------------- Cycle Iterator ---------------------------- //
Nst_Obj *NstC cycle_iter_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    if (!Nst_iter_start(args[0]))
        return nullptr;
    Nst_RETURN_NULL;
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
    if (obj != Nst_iend())
        return obj;
    Nst_dec_ref(obj);
    Nst_RETURN_NULL;
}

// --------------------------------- Repeat -------------------------------- //
Nst_Obj *NstC repeat_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    AS_INT(c_args[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC repeat_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);

    i64 max_count = AS_INT(c_args[2]);
    if (max_count >= 0 && AS_INT(c_args[0]) >= max_count)
        Nst_RETURN_IEND;

    if (max_count >= 0)
        AS_INT(c_args[0]) += 1;
    return Nst_inc_ref(c_args[1]);
}

// --------------------------------- Chain --------------------------------- //

Nst_Obj *NstC chain_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    Nst_Obj *main_iter = c_args[0];

    if (!Nst_iter_start(main_iter))
        return nullptr;
    Nst_dec_ref(c_args[1]);
    c_args[1] = Nst_null_ref();

    Nst_RETURN_NULL;
}

Nst_Obj *NstC chain_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    Nst_Obj *main_iter = c_args[0];
    Nst_Obj *current_iter = Nst_inc_ref(c_args[1]);
    Nst_Obj *val = OBJ(current_iter) == Nst_null()
        ? Nst_iend_ref()
        : Nst_iter_next(current_iter);

    while (val == Nst_iend()) {
        Nst_dec_ref(val);
        Nst_dec_ref(current_iter);
        Nst_Obj *next_iterable = Nst_iter_next(main_iter);
        if (next_iterable == Nst_iend())
            return next_iterable;
        else if (next_iterable == nullptr)
            return nullptr;

        Nst_Obj *next_iter = Nst_obj_cast(next_iterable, Nst_type()->Iter);
        Nst_dec_ref(next_iterable);
        if (next_iter == nullptr)
            return nullptr;

        current_iter = next_iter;
        if (!Nst_iter_start(current_iter))
            return nullptr;

        val = Nst_iter_next(current_iter);
    }

    Nst_seq_set(args[0], 1, current_iter);
    Nst_dec_ref(current_iter);
    return val;
}

// ---------------------------------- Zip ---------------------------------- //
Nst_Obj *NstC zip_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    if (!Nst_iter_start(c_args[0]) || !Nst_iter_start(c_args[1]))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC zip_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);

    Nst_Obj *ob1 = Nst_iter_next(c_args[0]);
    if (ob1 == nullptr || ob1 == Nst_iend())
        return ob1;

    Nst_Obj *ob2 = Nst_iter_next(c_args[1]);
    if (ob2 == nullptr || ob2 == Nst_iend()) {
        Nst_dec_ref(ob1);
        return ob2;
    }

    return Nst_array_create(2, ob1, ob2);
}

// ---------------------------- Zip n sequences ---------------------------- //

Nst_Obj *NstC zipn_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    for (usize i = 0, n = (usize)AS_INT(c_args[0]); i < n; i++) {
        if (!Nst_iter_start(c_args[i + 1]))
            return nullptr;
    }
    Nst_RETURN_NULL;
}

Nst_Obj *NstC zipn_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);

    usize arr_len = (usize)AS_INT(c_args[0]);
    Nst_Obj *arr = Nst_array_new(arr_len);
    for (usize i = 0; i < arr_len; i++) {
        Nst_Obj *res = Nst_iter_next(c_args[i + 1]);
        if (res == nullptr || res == Nst_iend()) {
            Nst_dec_ref(arr);
            return res;
        }
        Nst_seq_setnf(arr, i, res);
    }

    return OBJ(arr);
}

// ------------------------------- Enumerate ------------------------------- //
Nst_Obj *NstC enumerate_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    AS_INT(c_args[0]) = AS_INT(c_args[2]);
    if (!Nst_iter_start(c_args[1]))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC enumerate_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    i64 idx = AS_INT(c_args[0]);
    Nst_Obj *iter = c_args[1];

    Nst_Obj *res = Nst_iter_next(iter);
    if (res == nullptr || res == Nst_iend())
        return res;

    Nst_Obj *arr;
    if (c_args[4] == Nst_true())
        arr = Nst_array_create_c("oI", res, idx);
    else
        arr = Nst_array_create_c("Io", idx, res);

    AS_INT(c_args[0]) += AS_INT(c_args[3]);
    return OBJ(arr);
}

// ----------------------------- Keys & values ----------------------------- //

Nst_Obj *NstC keys_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    isize idx = (isize)AS_INT(c_args[0]);

    Nst_Obj *key;
    idx = Nst_map_next(idx, c_args[1], &key, nullptr);

    if (idx == -1)
        Nst_RETURN_IEND;

    AS_INT(c_args[0]) = idx;
    if (key == Nst_iend())
        Nst_RETURN_NULL;

    return Nst_inc_ref(key);
}

Nst_Obj *NstC values_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    i64 idx = AS_INT(c_args[0]);

    Nst_Obj *value;
    idx = Nst_map_next(idx, c_args[1], nullptr, &value);

    if (idx == -1)
        Nst_RETURN_IEND;

    AS_INT(c_args[0]) = idx;
    if (value == Nst_iend())
        Nst_RETURN_NULL;
    return Nst_inc_ref(value);
}

// --------------------------------- Batch --------------------------------- //

Nst_Obj *NstC batch_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    if (!Nst_iter_start(c_args[0]))
        return nullptr;
    Nst_dec_ref(c_args[2]);
    c_args[2] = Nst_false_ref();
    Nst_RETURN_NULL;
}

Nst_Obj *NstC batch_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);

    if (c_args[2] == Nst_true())
        Nst_RETURN_IEND;

    Nst_Obj *iter = c_args[0];
    i64 batch_size = AS_INT(c_args[1]);

    Nst_Obj *batch = Nst_array_new((usize)batch_size);
    if (batch == nullptr)
        return nullptr;

    for (i64 i = 0; i < batch_size; i++) {
        Nst_Obj *obj = Nst_iter_next(iter);
        if (obj == nullptr) {
            Nst_dec_ref(batch);
            return nullptr;
        } else if (obj == Nst_iend()) {
            Nst_dec_ref(c_args[2]);
            c_args[2] = Nst_true_ref();
            Nst_dec_ref(obj);
            if (i == 0) {
                Nst_dec_ref(batch);
                Nst_RETURN_IEND;
            }
            // Make the batch array the correct size
            Nst_Obj *correct_batch =
                Nst_array_from_objs(i, _Nst_seq_objs(batch));
            Nst_dec_ref(batch);
            return correct_batch;
        }
        Nst_seq_setnf(batch, i, obj);
    }
    return batch;
}

Nst_Obj *NstC batch_padded_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);

    if (c_args[2] == Nst_true())
        Nst_RETURN_IEND;

    Nst_Obj *iter = c_args[0];
    i64 batch_size = AS_INT(c_args[1]);
    Nst_Obj *padding = c_args[3];

    Nst_Obj *batch = Nst_array_new((usize)batch_size);
    if (batch == nullptr)
        return nullptr;

    i64 i = 0;
    for (; i < batch_size; i++) {
        Nst_Obj *obj = Nst_iter_next(iter);
        if (obj == nullptr) {
            Nst_dec_ref(batch);
            return nullptr;
        } else if (obj == Nst_iend()) {
            Nst_dec_ref(c_args[2]);
            c_args[2] = Nst_true_ref();
            Nst_dec_ref(obj);
            if (i == 0) {
                Nst_dec_ref(batch);
                Nst_RETURN_IEND;
            }
            break;
        }
        Nst_seq_setnf(batch, i, obj);
    }

    for (; i < batch_size; i++)
        Nst_seq_setf(batch, i, padding);

    return batch;
}
