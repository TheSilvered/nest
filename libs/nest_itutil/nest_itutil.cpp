#include "nest_itutil.h"
#include "itutil_functions.h"

#define FUNC_COUNT 14

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_func_list_new(FUNC_COUNT)) == nullptr )
    {
        return false;
    }

    usize idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(count_,        2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(cycle_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(repeat_,       2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(chain_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(zip_,          2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(zipn_,         1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(enumerate_,    3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(keys_,         1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(values_,       1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(items_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(reversed_,     1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(iter_start_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(iter_is_done_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(iter_get_val_, 1);

#if __LINE__ - FUNC_COUNT != 19
#error FUNC_COUNT does not match the number of lines
#endif

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

NST_FUNC_SIGN(count_)
{
    Nst_Int start;
    Nst_Obj *step_obj;

    NST_DEF_EXTRACT("i?i", &start, &step_obj);
    Nst_IntObj *step;
    NST_SET_DEF(
        step_obj, step,
        (Nst_IntObj *)nst_int_new(1),
        (Nst_IntObj *)nst_inc_ref(step_obj));

    // Layout: [idx, start, step]
    Nst_Obj *arr = nst_array_new(3);
    SEQ(arr)->objs[0] = nst_int_new(0);
    nst_seq_set(arr, 1, args[0]);
    nst_seq_set(arr, 2, step);
    nst_dec_ref(step);

    return nst_iter_new(
        FUNC(nst_func_new_c(1, count_start)),
        FUNC(nst_func_new_c(1, count_is_done)),
        FUNC(nst_func_new_c(1, count_get_val)),
        arr);
}

NST_FUNC_SIGN(cycle_)
{
    Nst_Obj *seq;

    NST_DEF_EXTRACT("S", &seq);

    // Layout: [idx, seq]
    Nst_SeqObj *arr = SEQ(nst_array_new(2));
    arr->objs[0] = nst_int_new(0);
    arr->objs[1] = seq;

    return nst_iter_new(
        FUNC(nst_func_new_c(1, cycle_start)),
        FUNC(nst_func_new_c(1, cycle_is_done)),
        FUNC(nst_func_new_c(1, cycle_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(repeat_)
{
    Nst_Obj *ob;
    Nst_Int times;

    NST_DEF_EXTRACT("oi", &ob, &times);

    // Layout: [count, item, max_times]
    Nst_Obj *arr = nst_array_new(3);
    SEQ(arr)->objs[0] = nst_int_new(0);
    nst_seq_set(arr, 1, args[0]);
    nst_seq_set(arr, 2, args[1]);

    return nst_iter_new(
        FUNC(nst_func_new_c(1, repeat_start)),
        FUNC(nst_func_new_c(1, repeat_is_done)),
        FUNC(nst_func_new_c(1, repeat_get_val)),
        arr);
}

NST_FUNC_SIGN(chain_)
{
    Nst_Obj *iter;

    NST_DEF_EXTRACT("R", &iter);

    // Layout: [main_iter, local_seq, val, is_done]
    Nst_SeqObj *arr = SEQ(nst_array_new(4));
    arr->objs[0] = iter;
    arr->objs[1] = nst_inc_ref(nst_null());
    arr->objs[2] = nst_inc_ref(nst_null());
    arr->objs[3] = nst_inc_ref(nst_false());

    return nst_iter_new(
        FUNC(nst_func_new_c(1, chain_start)),
        FUNC(nst_func_new_c(1, chain_is_done)),
        FUNC(nst_func_new_c(1, chain_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(zip_)
{
    Nst_Obj *seq1;
    Nst_Obj *seq2;

    NST_DEF_EXTRACT("SS", &seq1, &seq2);

    // Layout: [idx, seq1, seq2]
    Nst_SeqObj *arr = SEQ(nst_array_new(3));
    arr->objs[0] = nst_int_new(0);
    arr->objs[1] = seq1;
    arr->objs[2] = seq2;

    return nst_iter_new(
        FUNC(nst_func_new_c(1, zip_start)),
        FUNC(nst_func_new_c(1, zip_is_done)),
        FUNC(nst_func_new_c(1, zip_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(zipn_)
{
    Nst_SeqObj *seq;

    NST_DEF_EXTRACT("A", &seq);

    if ( seq->len < 2 )
    {
        NST_SET_RAW_VALUE_ERROR("the sequence must be at least of length two");
        return nullptr;
    }

    Nst_Obj **objs = seq->objs;

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        if ( objs[i]->type != nst_type()->Array &&
             objs[i]->type != nst_type()->Vector &&
             objs[i]->type != nst_type()->Str )
        {
            NST_SET_TYPE_ERROR(nst_format_error(
                "all objects in the sequence must be of type 'Array', 'Vector'"
                " or 'Str' but the object at index %zi was type '%s'",
                "us",
                i, TYPE_NAME(objs[i])));
            return nullptr;
        }
    }

    // Layout: [idx, seq1, seq2, ...]
    Nst_SeqObj *arr = SEQ(nst_array_new(seq->len + 1));
    arr->objs[0] = nst_int_new(0);

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        if ( objs[i]->type == nst_type()->Array || objs[i]->type == nst_type()->Vector )
        {
            arr->objs[i + 1] = nst_inc_ref(objs[i]);
        }
        else
        {
            // casting a string to an array always succedes
            arr->objs[i + 1] = nst_obj_cast(objs[i], nst_type()->Array, nullptr);
        }
    }

    return nst_iter_new(
        FUNC(nst_func_new_c(1, zipn_start)),
        FUNC(nst_func_new_c(1, zipn_is_done)),
        FUNC(nst_func_new_c(1, zipn_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(enumerate_)
{
    Nst_Obj *ob;
    Nst_Obj *start_ob;
    Nst_Obj *step_ob;

    Nst_Int start, step;

    NST_DEF_EXTRACT("R?i?i", &ob, &start_ob, &step_ob);
    NST_SET_DEF(start_ob, start, 0, AS_INT(start_ob));
    NST_SET_DEF(step_ob,  step,  1, AS_INT(step_ob ));

    // Layout: [idx, iterator, start, step]
    Nst_SeqObj *arr = SEQ(nst_array_new(4));
    arr->objs[0] = nst_int_new(0);
    arr->objs[1] = ob;
    arr->objs[2] = nst_int_new(start);
    arr->objs[3] = nst_int_new(step);

    return nst_iter_new(
        FUNC(nst_func_new_c(1, enumerate_start)),
        FUNC(nst_func_new_c(1, enumerate_is_done)),
        FUNC(nst_func_new_c(1, enumerate_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(keys_)
{
    Nst_MapObj *map;

    NST_DEF_EXTRACT("m", &map);

    // Layout: [idx, map]
    Nst_SeqObj *arr = SEQ(nst_array_new(2));
    arr->objs[0] = nst_int_new(0);
    nst_seq_set(arr, 1, args[0]);

    return nst_iter_new(
        FUNC(nst_func_new_c(1, kvi_start)),
        FUNC(nst_func_new_c(1, kvi_is_done)),
        FUNC(nst_func_new_c(1, keys_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(values_)
{
    Nst_MapObj *map;

    NST_DEF_EXTRACT("m", &map);

    // Layout: [idx, map]
    Nst_SeqObj *arr = SEQ(nst_array_new(2));
    arr->objs[0] = nst_int_new(0);
    nst_seq_set(arr, 1, args[0]);

    return nst_iter_new(
        FUNC(nst_func_new_c(1, kvi_start)),
        FUNC(nst_func_new_c(1, kvi_is_done)),
        FUNC(nst_func_new_c(1, values_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(items_)
{
    Nst_MapObj *map;

    NST_DEF_EXTRACT("m", &map);

    // Layout: [idx, map]
    Nst_SeqObj *arr = SEQ(nst_array_new(2));
    arr->objs[0] = nst_int_new(0);
    nst_seq_set(arr, 1, args[0]);

    return nst_iter_new(
        FUNC(nst_func_new_c(1, kvi_start)),
        FUNC(nst_func_new_c(1, kvi_is_done)),
        FUNC(nst_func_new_c(1, items_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(reversed_)
{
    Nst_Obj *seq;

    NST_DEF_EXTRACT("S", &seq);

    // Layout: [idx, seq]
    Nst_SeqObj *arr = SEQ(nst_array_new(2));
    arr->objs[0] = nst_int_new(0);
    arr->objs[1] = seq;

    return nst_iter_new(
        FUNC(nst_func_new_c(1, reversed_start)),
        FUNC(nst_func_new_c(1, reversed_is_done)),
        FUNC(nst_func_new_c(1, reversed_get_val)),
        OBJ(arr));
}

NST_FUNC_SIGN(iter_start_)
{
    Nst_IterObj *iter;

    NST_DEF_EXTRACT("I", &iter);

    return nst_call_func(iter->start, &iter->value, err);
}

NST_FUNC_SIGN(iter_is_done_)
{
    Nst_IterObj *iter;

    NST_DEF_EXTRACT("I", &iter);

    return nst_call_func(iter->is_done, &iter->value, err);
}

NST_FUNC_SIGN(iter_get_val_)
{
    Nst_IterObj *iter;

    NST_DEF_EXTRACT("I", &iter);

    return nst_call_func(iter->get_val, &iter->value, err);
}
