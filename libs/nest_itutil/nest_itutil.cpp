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
    Nst_Obj *start;
    Nst_Obj *step_obj;

    if (!Nst_extract_args("i:i ?i", arg_num, args, &start, &step_obj))
        return nullptr;
    Nst_Obj *step = Nst_DEF_VAL(
        step_obj,
        Nst_inc_ref(step_obj),
        Nst_int_new(1));

    // Layout: [idx, start, step]
    Nst_Obj *arr = Nst_array_create_c("ioo", 0, start, step);

    if (arr == nullptr) {
        Nst_dec_ref(start);
        Nst_dec_ref(step);
        return nullptr;
    }

    RETURN_NEW_ITER(count_start, count_next, arr);
}

Nst_Obj *NstC cycle_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *iterable;

    if (!Nst_extract_args("A|s|I:o", arg_num, args, &iterable))
        return nullptr;

    if (Nst_T(iterable, Iter))
        RETURN_NEW_ITER(cycle_iter_start, cycle_iter_next, iterable);

    // Layout: [idx, iterable]
    Nst_Obj *arr = Nst_array_create_c("io", 0, iterable);
    if (arr == nullptr) {
        Nst_dec_ref(iterable);
        return nullptr;
    }

    if (Nst_T(iterable, Str))
        RETURN_NEW_ITER(cycle_str_start, cycle_str_next, arr);

    RETURN_NEW_ITER(cycle_seq_start, cycle_seq_next, arr);
}

Nst_Obj *NstC repeat_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *ob;
    Nst_Obj *times_obj;

    if (!Nst_extract_args("o ?i", arg_num, args, &ob, &times_obj))
        return nullptr;

    i64 times = Nst_DEF_VAL(times_obj, AS_INT(times_obj), -1);

    // Layout: [count, item, max_times]
    Nst_Obj *arr = Nst_array_create_c("iOI", 0, ob, times);

    if (arr == nullptr)
        return nullptr;

    RETURN_NEW_ITER(repeat_start, repeat_next, arr);
}

Nst_Obj *NstC chain_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *iter;

    if (!Nst_extract_args("R", arg_num, args, &iter))
        return nullptr;

    // Layout: [main_iter, current_iter]
    Nst_Obj *arr = Nst_array_create_c("on", iter, nullptr);

    if (arr == nullptr) {
        Nst_dec_ref(iter);
        return nullptr;
    }

    RETURN_NEW_ITER(chain_start, chain_next, arr);
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

    // Layout: [count, iter1, iter2, ...]
    Nst_Obj *arr = Nst_array_new(seq_len + 1);
    if (arr == nullptr)
        return nullptr;

    Nst_Obj *seq_len_obj = Nst_int_new(seq_len);
    if (seq_len_obj == nullptr) {
        Nst_dec_ref(arr);
        return nullptr;
    }

    Nst_seq_setnf(arr, 0, seq_len_obj);
    for (usize i = 0; i < seq_len; i++) {
        Nst_Obj *iter = Nst_obj_cast(Nst_seq_getnf(seq, i), Nst_type()->Iter);
        Nst_seq_setnf(arr, i + 1, iter);
    }

    RETURN_NEW_ITER(zipn_start, zipn_next, arr);
}

Nst_Obj *NstC zip_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq1;
    Nst_Obj *seq2;

    if (!Nst_extract_args("I|a|v|s ?R:I", arg_num, args, &seq1, &seq2))
        return nullptr;

    if (seq2 == Nst_null())
        return zipn_(seq1);

    // successful cast guaranteed by the type in Nst_extract_args
    seq1 = Nst_obj_cast(seq1, Nst_type()->Iter);

    // Layout: [iter1, iter2]
    Nst_Obj *arr = Nst_array_create(2, seq1, seq2);
    if (arr == nullptr) {
        Nst_dec_ref(seq1);
        Nst_dec_ref(seq2);
        return nullptr;
    }

    RETURN_NEW_ITER(zip_start, zip_next, arr);
}

Nst_Obj *NstC enumerate_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *ob;
    Nst_Obj *start_ob;
    Nst_Obj *step_ob;
    Nst_Obj *invert_order;

    if (!Nst_extract_args(
            "R ?i ?i o:b",
            arg_num, args,
            &ob, &start_ob, &step_ob, &invert_order))
    {
        return nullptr;
    }
    i64 start = Nst_DEF_VAL(start_ob, AS_INT(start_ob), 0);
    i64 step = Nst_DEF_VAL(step_ob, AS_INT(step_ob), 1);

    // Layout: [idx, iterator, start, step, invert_order]
    Nst_Obj *arr = Nst_array_create_c(
        "ioIIo",
        0, ob, start, step, invert_order);

    if (arr == nullptr) {
        Nst_dec_ref(ob);
        Nst_dec_ref(invert_order);
        return nullptr;
    }

    RETURN_NEW_ITER(enumerate_start, enumerate_next, arr);
}

Nst_Obj *NstC keys_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *map;
    if (!Nst_extract_args("m", arg_num, args, &map))
        return nullptr;

    // Layout: [idx, map]
    Nst_Obj *arr = Nst_array_create_c("iO", 0, map);

    if (arr == nullptr)
        return nullptr;

    Nst_Obj *iter = Nst_iter_new(
        Nst_inc_ref(Nst_iter_func()->map_start),
        Nst_inc_ref(itutil_functions.keys_next),
        arr);
    if (iter == nullptr) {
        Nst_dec_ref(Nst_iter_func()->map_start);
        Nst_dec_ref(itutil_functions.keys_next);
        Nst_dec_ref(arr);
        return nullptr;
    }
    return iter;
}

Nst_Obj *NstC values_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *map;

    if (!Nst_extract_args("m", arg_num, args, &map))
        return nullptr;

    // Layout: [idx, map]
    Nst_Obj *arr = Nst_array_create_c("iO", 0, map);

    if (arr == nullptr)
        return nullptr;

    Nst_Obj *iter = Nst_iter_new(
        Nst_inc_ref(Nst_iter_func()->map_start),
        Nst_inc_ref(itutil_functions.values_next),
        arr);
    if (iter == nullptr) {
        Nst_dec_ref(Nst_iter_func()->map_start);
        Nst_dec_ref(itutil_functions.values_next);
        Nst_dec_ref(arr);
        return nullptr;
    }
    return iter;
}

Nst_Obj *NstC batch_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_IntObj *batch_size;

    if (!Nst_extract_args("R i|i", arg_num, args, &seq, &batch_size))
        return nullptr;

    if (AS_INT(batch_size) <= 0) {
        Nst_set_value_error_c("the batch size must be a positive integer");
        Nst_dec_ref(seq);
        return nullptr;
    }

    // Layout: [iterator, batch_size, iter_ended]
    Nst_Obj *arr = Nst_array_create_c("oOb", seq, batch_size, false);

    if (arr == nullptr) {
        Nst_dec_ref(seq);
        return nullptr;
    }

    RETURN_NEW_ITER(batch_start, batch_next, arr);
}

Nst_Obj *NstC batch_padded_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_IntObj *batch_size;
    Nst_Obj *padding;

    if (!Nst_extract_args(
            "R i|i o",
            arg_num, args,
            &seq, &batch_size, &padding))
    {
        return nullptr;
    }

    if (AS_INT(batch_size) <= 0) {
        Nst_set_value_error_c("the batch size must be a positive integer");
        Nst_dec_ref(seq);
        return nullptr;
    }

    // Layout: [iterator, batch_size, iter_ended, padding_object]
    Nst_Obj *arr = Nst_array_create_c("oObO", seq, batch_size, false, padding);

    if (arr == nullptr) {
        Nst_dec_ref(seq);
        return nullptr;
    }

    RETURN_NEW_ITER(batch_start, batch_padded_next, arr);
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
