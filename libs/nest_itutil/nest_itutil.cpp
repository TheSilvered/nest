#include "nest_itutil.h"
#include "itutil_functions.h"

#define FUNC_COUNT 13

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(count_,        2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(cycle_,        1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(repeat_,       2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(chain_,        1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(zip_,          2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(enumerate_,    4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(keys_,         1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(values_,       1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(items_,        1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(reversed_,     1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(iter_start_,   1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(iter_is_done_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(iter_get_val_, 1);

#if __LINE__ - FUNC_COUNT != 15
#error
#endif

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

Nst_FUNC_SIGN(count_)
{
    Nst_Obj *start;
    Nst_Obj *step_obj;

    Nst_DEF_EXTRACT("i:i ?i", &start, &step_obj);
    Nst_IntObj *step = Nst_DEF_VAL(
        step_obj,
        (Nst_IntObj *)Nst_inc_ref(step_obj),
        (Nst_IntObj *)Nst_int_new(1));

    // Layout: [idx, start, step]
    Nst_Obj *arr = Nst_array_create_c("ioo", 0, start, step);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, count_start)),
        FUNC(Nst_func_new_c(1, count_is_done)),
        FUNC(Nst_func_new_c(1, count_get_val)),
        arr);
}

Nst_FUNC_SIGN(cycle_)
{
    Nst_Obj *seq;

    Nst_DEF_EXTRACT("S", &seq);

    // Layout: [idx, seq]
    Nst_Obj *arr = Nst_array_create_c("io", 0, seq);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, cycle_start)),
        FUNC(Nst_func_new_c(1, cycle_is_done)),
        FUNC(Nst_func_new_c(1, cycle_get_val)),
        arr);
}

Nst_FUNC_SIGN(repeat_)
{
    Nst_Obj *ob;
    Nst_Obj *times;

    Nst_DEF_EXTRACT("o i:i", &ob, &times);

    if (AS_INT(times) < 0) {
        Nst_dec_ref(times);
        Nst_set_value_error_c("cannot repeat a negative number of times");
        return nullptr;
    }

    // Layout: [count, item, max_times]
    Nst_Obj *arr = Nst_array_create_c("iOo", 0, ob, times);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, repeat_start)),
        FUNC(Nst_func_new_c(1, repeat_is_done)),
        FUNC(Nst_func_new_c(1, repeat_get_val)),
        arr);
}

Nst_FUNC_SIGN(chain_)
{
    Nst_Obj *iter;

    Nst_DEF_EXTRACT("R", &iter);

    // Layout: [main_iter, local_seq, val, is_done]
    Nst_Obj *arr = Nst_array_create_c("onnb", iter, nullptr, nullptr, false);
    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, chain_start)),
        FUNC(Nst_func_new_c(1, chain_is_done)),
        FUNC(Nst_func_new_c(1, chain_get_val)),
        arr);
}

Nst_Obj *zipn_(Nst_SeqObj *seq)
{
    if (!Nst_extract_arg_values("A.I|a|v|s", 1, (Nst_Obj **)&seq, &seq))
        return nullptr;

    if (seq->len < 2) {
        Nst_set_value_error_c("the sequence must be at least of length two");
        return nullptr;
    }

    Nst_Obj **objs = seq->objs;

    // Layout: [count, iter1, iter2, ...]
    Nst_SeqObj *arr = SEQ(Nst_array_new(seq->len + 1));
    arr->objs[0] = Nst_int_new(seq->len);

    for (usize i = 0, n = seq->len; i < n; i++)
        arr->objs[i + 1] = Nst_obj_cast(objs[i], Nst_type()->Iter);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, zipn_start)),
        FUNC(Nst_func_new_c(1, zipn_is_done)),
        FUNC(Nst_func_new_c(1, zipn_get_val)),
        OBJ(arr));
}

Nst_FUNC_SIGN(zip_)
{
    Nst_Obj *seq1;
    Nst_Obj *seq2;

    Nst_DEF_EXTRACT("I|a|v|s?R", &seq1, &seq2);

    if (seq2 == Nst_null())
        return zipn_((Nst_SeqObj *)seq1);

    seq1 = Nst_obj_cast(seq1, Nst_type()->Iter);

    // Layout: [iter1, iter2]
    Nst_Obj *arr = Nst_array_create(2, seq1, seq2);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, zip_start)),
        FUNC(Nst_func_new_c(1, zip_is_done)),
        FUNC(Nst_func_new_c(1, zip_get_val)),
        arr);
}

Nst_FUNC_SIGN(enumerate_)
{
    Nst_Obj *ob;
    Nst_Obj *start_ob;
    Nst_Obj *step_ob;
    Nst_Obj *invert_order;

    Nst_DEF_EXTRACT("R ?i ?i o:b", &ob, &start_ob, &step_ob, &invert_order);
    i64 start = Nst_DEF_VAL(start_ob, AS_INT(start_ob), 0);
    i64 step = Nst_DEF_VAL(step_ob, AS_INT(step_ob), 1);

    // Layout: [idx, iterator, start, step, invert_order]
    Nst_Obj *arr = Nst_array_create_c(
        "ioiio",
        0, ob, start, step, invert_order);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, enumerate_start)),
        FUNC(Nst_func_new_c(1, enumerate_is_done)),
        FUNC(Nst_func_new_c(1, enumerate_get_val)),
        arr);
}

Nst_FUNC_SIGN(keys_)
{
    Nst_Obj *map;
    Nst_DEF_EXTRACT("m", &map);

    // Layout: [idx, map]
    Nst_Obj *arr = Nst_array_create_c("iO", 0, map);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, kvi_start)),
        FUNC(Nst_func_new_c(1, kvi_is_done)),
        FUNC(Nst_func_new_c(1, keys_get_val)),
        arr);
}

Nst_FUNC_SIGN(values_)
{
    Nst_MapObj *map;

    Nst_DEF_EXTRACT("m", &map);

    // Layout: [idx, map]
    Nst_Obj *arr = Nst_array_create_c("iO", 0, map);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, kvi_start)),
        FUNC(Nst_func_new_c(1, kvi_is_done)),
        FUNC(Nst_func_new_c(1, values_get_val)),
        arr);
}

Nst_FUNC_SIGN(items_)
{
    Nst_MapObj *map;

    Nst_DEF_EXTRACT("m", &map);

    // Layout: [idx, map]
    Nst_Obj *arr = Nst_array_create_c("iO", 0, map);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, kvi_start)),
        FUNC(Nst_func_new_c(1, kvi_is_done)),
        FUNC(Nst_func_new_c(1, items_get_val)),
        arr);
}

Nst_FUNC_SIGN(reversed_)
{
    Nst_Obj *seq;

    Nst_DEF_EXTRACT("S", &seq);

    // Layout: [idx, seq]
    Nst_Obj *arr = Nst_array_create_c("io", 0, seq);

    return Nst_iter_new(
        FUNC(Nst_func_new_c(1, reversed_start)),
        FUNC(Nst_func_new_c(1, reversed_is_done)),
        FUNC(Nst_func_new_c(1, reversed_get_val)),
        arr);
}

Nst_FUNC_SIGN(iter_start_)
{
    Nst_IterObj *iter;

    Nst_DEF_EXTRACT("I", &iter);

    return Nst_call_func(iter->start, &iter->value);
}

Nst_FUNC_SIGN(iter_is_done_)
{
    Nst_IterObj *iter;

    Nst_DEF_EXTRACT("I", &iter);

    return Nst_call_func(iter->is_done, &iter->value);
}

Nst_FUNC_SIGN(iter_get_val_)
{
    Nst_IterObj *iter;

    Nst_DEF_EXTRACT("I", &iter);

    return Nst_call_func(iter->get_val, &iter->value);
}
