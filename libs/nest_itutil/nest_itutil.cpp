#include "nest_itutil.h"
#include "itutil_functions.h"

#define RETURN_NEW_ITER(start, next, value) do {                           \
    Nst_Obj *iter__ = Nst_iter_new(                                           \
        Nst_inc_ref(itutil_functions.start),                                  \
        Nst_inc_ref(itutil_functions.next),                                \
        value);                                                               \
    if (iter__ == nullptr) {                                                  \
        Nst_dec_ref(itutil_functions.start);                                  \
        Nst_dec_ref(itutil_functions.next);                                \
        Nst_dec_ref(value);                                                   \
        return nullptr;                                                       \
    }                                                                         \
    return iter__;                                                            \
    } while (0)

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(count_,        2),
    Nst_FUNCDECLR(cycle_,        1),
    Nst_FUNCDECLR(repeat_,       2),
    Nst_FUNCDECLR(chain_,        1),
    Nst_FUNCDECLR(zip_,          2),
    Nst_FUNCDECLR(enumerate_,    4),
    Nst_FUNCDECLR(keys_,         1),
    Nst_FUNCDECLR(values_,       1),
    Nst_FUNCDECLR(batch_,        2),
    Nst_FUNCDECLR(batch_padded_, 3),
    Nst_FUNCDECLR(new_iterator_, 3),
    Nst_FUNCDECLR(iter_start_,   1),
    Nst_FUNCDECLR(iter_next_, 1),
    Nst_FUNCDECLR(IEND_,         1),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    if (!init_itutil_functions())
        return nullptr;

    return obj_list_;
}

void lib_quit()
{
    free_itutil_functions();
}

Nst_Obj *NstC count_(usize arg_num, Nst_Obj **args)
{
    i64 start;
    Nst_Obj *step_obj;

    if (!Nst_extract_args("i ?i", arg_num, args, &start, &step_obj))
        return nullptr;
    i64 step = Nst_DEF_VAL(step_obj, Nst_int_i64(step_obj), 1);

    CountData data = {
        .idx = 0,
        .start = start,
        .step = step
    };

    Nst_Obj *count_data = Nst_obj_custom(CountData, &data);
    if (count_data == nullptr)
        return nullptr;

    RETURN_NEW_ITER(count_start, count_next, count_data);
}

Nst_Obj *NstC cycle_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *iterable;

    if (!Nst_extract_args("A|s|I", arg_num, args, &iterable))
        return nullptr;

    if (Nst_T(iterable, Iter)) {
        RETURN_NEW_ITER(
            cycle_iter_start,
            cycle_iter_next,
            Nst_inc_ref(iterable));
    }

    CycleData data = {
        .idx = 0,
        .iterable = Nst_inc_ref(iterable)
    };

    Nst_Obj *cycle_data = Nst_obj_custom_ex(
        CycleData,
        &data,
        destroy_cycle_data);
    if (cycle_data == nullptr) {
        Nst_dec_ref(iterable);
        return nullptr;
    }

    if (Nst_T(iterable, Str))
        RETURN_NEW_ITER(cycle_start, cycle_str_next, cycle_data);

    RETURN_NEW_ITER(cycle_start, cycle_seq_next, cycle_data);
}

Nst_Obj *NstC repeat_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *ob;
    Nst_Obj *times_obj;

    if (!Nst_extract_args("o ?i", arg_num, args, &ob, &times_obj))
        return nullptr;

    i64 times = Nst_DEF_VAL(times_obj, Nst_int_i64(times_obj), -1);

    RepeatData data = {
        .count = 0,
        .max_count = times,
        .item = Nst_inc_ref(ob)
    };

    // Layout: [count, item, max_times]
    Nst_Obj *repeat_data = Nst_obj_custom_ex(
        RepeatData,
        &data,
        destroy_repeat_data);

    if (repeat_data == nullptr) {
        Nst_dec_ref(ob);
        return nullptr;
    }

    RETURN_NEW_ITER(repeat_start, repeat_next, repeat_data);
}

Nst_Obj *NstC chain_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *iter;

    if (!Nst_extract_args("R", arg_num, args, &iter))
        return nullptr;

    ChainData data = {
        .main_iter = iter,
        .current_iter = nullptr
    };

    Nst_Obj *chain_data = Nst_obj_custom_ex(
        ChainData,
        &data,
        destroy_chain_data);

    if (chain_data == nullptr) {
        Nst_dec_ref(iter);
        return nullptr;
    }

    RETURN_NEW_ITER(chain_start, chain_next, chain_data);
}

Nst_Obj *zipn_(Nst_Obj *seq)
{
    if (!Nst_extract_args("A.I|a|v|s", 1, (Nst_Obj **)&seq, &seq))
        return nullptr;

    usize seq_len = Nst_seq_len(seq);
    if (seq_len < 2) {
        Nst_set_value_error_c("the sequence must be at least of length two");
        return nullptr;
    }

    ZipData init_data = {
        .count = 0,
        .iterators = nullptr
    };

    Nst_Obj *zip_data = _Nst_obj_custom_ex(
        sizeof(ZipData) + sizeof(Nst_Obj *) * seq_len,
        &init_data,
        "ZipData",
        destroy_zip_data);

    if (zip_data == nullptr)
        return nullptr;

    // The iterators array is stored after data
    ZipData *data = (ZipData *)Nst_obj_custom_data(zip_data);
    Nst_Obj **iterators = (Nst_Obj **)(data + 1);
    data->iterators = iterators;

    for (usize i = 0; i < seq_len; i++) {
        Nst_Obj *iter = Nst_obj_cast(Nst_seq_getnf(seq, i), Nst_type()->Iter);
        if (iter == nullptr) {
            Nst_dec_ref(zip_data);
            return nullptr;
        }
        iterators[i] = iter;
        data->count++;
    }

    RETURN_NEW_ITER(zip_start, zip_next, zip_data);
}

Nst_Obj *NstC zip_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq1;
    Nst_Obj *seq2;

    if (!Nst_extract_args("I|a|v|s ?R:I", arg_num, args, &seq1, &seq2))
        return nullptr;

    if (seq2 == Nst_null())
        return zipn_(seq1);

    seq1 = Nst_obj_cast(seq1, Nst_type()->Iter);
    if (seq1 == nullptr) {
        Nst_dec_ref(seq2);
        return nullptr;
    }

    ZipData init_data = {
        .count = 0,
        .iterators = nullptr
    };

    Nst_Obj *zip_data = _Nst_obj_custom_ex(
        sizeof(ZipData) + sizeof(Nst_Obj *) * 2,
        &init_data,
        "ZipData",
        destroy_zip_data);

    if (zip_data == nullptr) {
        Nst_dec_ref(seq1);
        Nst_dec_ref(seq2);
        return nullptr;
    }

    // The iterators array is stored after data
    ZipData *data = (ZipData *)Nst_obj_custom_data(zip_data);
    Nst_Obj **iterators = (Nst_Obj **)(data + 1);
    data->count = 2;
    data->iterators = iterators;

    iterators[0] = seq1;
    iterators[1] = seq2;

    RETURN_NEW_ITER(zip_start, zip_next, zip_data);
}

Nst_Obj *NstC enumerate_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *ob;
    Nst_Obj *start_ob;
    Nst_Obj *step_ob;
    bool invert_order;

    if (!Nst_extract_args(
            "R ?i ?i y",
            arg_num, args,
            &ob, &start_ob, &step_ob, &invert_order))
    {
        return nullptr;
    }
    i64 start = Nst_DEF_VAL(start_ob, Nst_int_i64(start_ob), 0);
    i64 step = Nst_DEF_VAL(step_ob, Nst_int_i64(step_ob), 1);

    EnumerateData data = {
        .idx = 0,
        .start = start,
        .step = step,
        .iterator = ob,
        .invert_order = invert_order
    };

    Nst_Obj *enumerate_data = Nst_obj_custom_ex(
        EnumerateData,
        &data,
        destroy_enumerate_data);

    if (enumerate_data == nullptr) {
        Nst_dec_ref(ob);
        return nullptr;
    }

    RETURN_NEW_ITER(enumerate_start, enumerate_next, enumerate_data);
}

Nst_Obj *NstC keys_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *map;
    if (!Nst_extract_args("m", arg_num, args, &map))
        return nullptr;

    MapIterData data = {
        .idx = -1,
        .map = Nst_inc_ref(map)
    };

    Nst_Obj *map_iter_data = Nst_obj_custom_ex(
        MapIterData,
        &data,
        destroy_map_iter_data);

    if (map_iter_data == nullptr) {
        Nst_dec_ref(map);
        return nullptr;
    }

    RETURN_NEW_ITER(map_iter_start, keys_next, map_iter_data);
}

Nst_Obj *NstC values_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *map;

    if (!Nst_extract_args("m", arg_num, args, &map))
        return nullptr;

    MapIterData data = {
        .idx = -1,
        .map = Nst_inc_ref(map)
    };

    Nst_Obj *map_iter_data = Nst_obj_custom_ex(
        MapIterData,
        &data,
        destroy_map_iter_data);

    if (map_iter_data == nullptr) {
        Nst_dec_ref(map);
        return nullptr;
    }

    RETURN_NEW_ITER(map_iter_start, values_next, map_iter_data);
}

Nst_Obj *NstC batch_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    i64 batch_size;

    if (!Nst_extract_args("R i", arg_num, args, &seq, &batch_size))
        return nullptr;

    if (batch_size <= 0) {
        Nst_set_value_error_c("the batch size must be a positive integer");
        Nst_dec_ref(seq);
        return nullptr;
    }

    BatchData data = {
        .iterator = seq,
        .padding_obj = nullptr,
        .batch_size = (usize)batch_size,
        .iter_ended = false
    };

    Nst_Obj *batch_data = Nst_obj_custom_ex(
        BatchData,
        &data,
        destroy_batch_data);

    if (batch_data == nullptr) {
        Nst_dec_ref(seq);
        return nullptr;
    }

    RETURN_NEW_ITER(batch_start, batch_next, batch_data);
}

Nst_Obj *NstC batch_padded_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    i64 batch_size;
    Nst_Obj *padding;

    if (!Nst_extract_args(
            "R i o",
            arg_num, args,
            &seq, &batch_size, &padding))
    {
        return nullptr;
    }

    if (batch_size <= 0) {
        Nst_set_value_error_c("the batch size must be a positive integer");
        Nst_dec_ref(seq);
        return nullptr;
    }

    BatchData data = {
        .iterator = seq,
        .padding_obj = Nst_inc_ref(padding),
        .batch_size = (usize)batch_size,
        .iter_ended = false
    };

    Nst_Obj *batch_data = Nst_obj_custom_ex(
        BatchData,
        &data,
        destroy_batch_data);

    if (batch_data == nullptr) {
        Nst_dec_ref(padding);
        Nst_dec_ref(seq);
        return nullptr;
    }

    RETURN_NEW_ITER(batch_start, batch_next, batch_data);
}

Nst_Obj *NstC new_iterator_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *start;
    Nst_Obj *next;
    Nst_Obj *data;

    if (!Nst_extract_args(
            "f:o f:o o:o",
            arg_num, args,
            &start, &next, &data))
    {
        return nullptr;
    }

    if (Nst_func_arg_num(start) != 1 || Nst_func_arg_num(next) != 1)
    {
        Nst_set_value_error_c(
            "all the functions must accept exactly one argument");
        Nst_dec_ref(start);
        Nst_dec_ref(next);
        Nst_dec_ref(data);
        return nullptr;
    }

    Nst_Obj *iter = Nst_iter_new(start, next, data);
    if (iter == nullptr) {
        Nst_dec_ref(start);
        Nst_dec_ref(next);
        Nst_dec_ref(data);
        return nullptr;
    }
    return iter;
}

Nst_Obj *NstC iter_start_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *iter;

    if (!Nst_extract_args("I", arg_num, args, &iter))
        return nullptr;

    Nst_Obj *start = Nst_iter_start_func(iter);
    Nst_Obj *value = Nst_iter_value(iter);

    return Nst_func_call(start, 1, &value);
}

Nst_Obj *NstC iter_next_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *iter;

    if (!Nst_extract_args("I", arg_num, args, &iter))
        return nullptr;

    Nst_Obj *next = Nst_iter_next_func(iter);
    Nst_Obj *value = Nst_iter_value(iter);

    return Nst_func_call(next, 1, &value);
}

Nst_Obj *NstC IEND_()
{
    return Nst_iend_ref();
}
