#include "nest_itutil.h"
#include "itutil_functions.h"

#define FUNC_COUNT 15

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(count_,        2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(cycle_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(repeat_,       2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(chain_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(zip_,          2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(zipn_,         1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(enumerate_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(keys_,         1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(values_,       1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(items_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(reversed_,     1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(iter_start_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(iter_is_done_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(iter_get_val_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(iter_advance_, 1);

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
    Nst_Int step;

    NST_D_EXTRACT("ii", &start, &step);

    // Layout: [idx, start, step]
    Nst_Obj *arr = nst_new_array(3);
    SEQ(arr)->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);
    nst_set_value_seq(arr, 2, args[1]);

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, count_start)),
        FUNC(nst_new_cfunc(1, count_advance)),
        FUNC(nst_new_cfunc(1, count_is_done)),
        FUNC(nst_new_cfunc(1, count_get_val)),
        arr
    );
}

NST_FUNC_SIGN(cycle_)
{
    Nst_Obj *seq;

    NST_D_EXTRACT("S", &seq);

    // Layout: [idx, seq]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq;

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, cycle_start)),
        FUNC(nst_new_cfunc(1, cycle_advance)),
        FUNC(nst_new_cfunc(1, cycle_is_done)),
        FUNC(nst_new_cfunc(1, cycle_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(repeat_)
{
    Nst_Obj *ob;
    Nst_Int times;

    NST_D_EXTRACT("oi", &ob, &times);

    // Layout: [count, item, max_times]
    Nst_Obj *arr = nst_new_array(3);
    SEQ(arr)->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);
    nst_set_value_seq(arr, 2, args[1]);

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, repeat_start)),
        FUNC(nst_new_cfunc(1, repeat_advance)),
        FUNC(nst_new_cfunc(1, repeat_is_done)),
        FUNC(nst_new_cfunc(1, repeat_get_val)),
        arr
    );
}

NST_FUNC_SIGN(chain_)
{
    Nst_SeqObj *seq;

    NST_D_EXTRACT("A", &seq);

    // Layout: [idx_global, idx_local, seq]
    Nst_SeqObj *arr = SEQ(nst_new_array(3));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = nst_new_int(0);
    nst_set_value_seq(arr, 2, args[0]);

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, chain_start)),
        FUNC(nst_new_cfunc(1, chain_advance)),
        FUNC(nst_new_cfunc(1, chain_is_done)),
        FUNC(nst_new_cfunc(1, chain_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(zip_)
{
    Nst_Obj *seq1;
    Nst_Obj *seq2;

    NST_D_EXTRACT("SS", &seq1, &seq2);

    // Layout: [idx, seq1, seq2]
    Nst_SeqObj *arr = SEQ(nst_new_array(3));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq1;
    arr->objs[2] = seq2;

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, zip_start)),
        FUNC(nst_new_cfunc(1, zip_advance)),
        FUNC(nst_new_cfunc(1, zip_is_done)),
        FUNC(nst_new_cfunc(1, zip_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(zipn_)
{
    Nst_SeqObj *seq;

    NST_D_EXTRACT("A", &seq);

    if ( seq->len < 2 )
    {
        NST_SET_RAW_VALUE_ERROR("the sequence must be at least of length two");
        return nullptr;
    }

    Nst_Obj **objs = seq->objs;

    for ( size_t i = 0, n = seq->len; i < n; i++ )
    {
        if ( objs[i]->type != nst_t.Array &&
             objs[i]->type != nst_t.Vector &&
             objs[i]->type != nst_t.Str )
        {
            NST_SET_TYPE_ERROR(_nst_format_error(
                "all objects in the sequence must be of type 'Array', 'Vector'"
                " or 'Str' but the object at index %zi was type '%s'",
                "us",
                i, TYPE_NAME(objs[i])
            ));
            return nullptr;
        }
    }

    // Layout: [idx, seq1, seq2, ...]
    Nst_SeqObj *arr = SEQ(nst_new_array(seq->len + 1));
    arr->objs[0] = nst_new_int(0);

    for ( size_t i = 0, n = seq->len; i < n; i++ )
    {
        if ( objs[i]->type == nst_t.Array || objs[i]->type == nst_t.Vector )
            arr->objs[i + 1] = nst_inc_ref(objs[i]);
        else
            // casting a string to an array always succedes
            arr->objs[i + 1] = nst_obj_cast(objs[i], nst_t.Array, nullptr);
    }

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, zipn_start)),
        FUNC(nst_new_cfunc(1, zipn_advance)),
        FUNC(nst_new_cfunc(1, zipn_is_done)),
        FUNC(nst_new_cfunc(1, zipn_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(enumerate_)
{
    Nst_Obj *seq;

    NST_D_EXTRACT("S", &seq);

    // Layout: [idx, seq]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq;

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, enumerate_start)),
        FUNC(nst_new_cfunc(1, enumerate_advance)),
        FUNC(nst_new_cfunc(1, enumerate_is_done)),
        FUNC(nst_new_cfunc(1, enumerate_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(keys_)
{
    Nst_MapObj *map;

    NST_D_EXTRACT("m", &map);

    // Layout: [idx, map]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, kvi_start)),
        FUNC(nst_new_cfunc(1, kvi_advance)),
        FUNC(nst_new_cfunc(1, kvi_is_done)),
        FUNC(nst_new_cfunc(1, keys_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(values_)
{
    Nst_MapObj *map;

    NST_D_EXTRACT("m", &map);

    // Layout: [idx, map]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, kvi_start)),
        FUNC(nst_new_cfunc(1, kvi_advance)),
        FUNC(nst_new_cfunc(1, kvi_is_done)),
        FUNC(nst_new_cfunc(1, values_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(items_)
{
    Nst_MapObj *map;

    NST_D_EXTRACT("m", &map);

    // Layout: [idx, map]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, kvi_start)),
        FUNC(nst_new_cfunc(1, kvi_advance)),
        FUNC(nst_new_cfunc(1, kvi_is_done)),
        FUNC(nst_new_cfunc(1, items_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(reversed_)
{
    Nst_Obj *seq;

    NST_D_EXTRACT("S", &seq);

    // Layout: [idx, seq]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq;

    return nst_new_iter(
        FUNC(nst_new_cfunc(1, reversed_start)),
        FUNC(nst_new_cfunc(1, reversed_advance)),
        FUNC(nst_new_cfunc(1, reversed_is_done)),
        FUNC(nst_new_cfunc(1, reversed_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(iter_start_)
{
    Nst_IterObj *iter;

    NST_D_EXTRACT("I", &iter);

    return nst_call_func(iter->start, &iter->value, err);
}

NST_FUNC_SIGN(iter_is_done_)
{
    Nst_IterObj *iter;

    NST_D_EXTRACT("I", &iter);

    return nst_call_func(iter->is_done, &iter->value, err);
}

NST_FUNC_SIGN(iter_get_val_)
{
    Nst_IterObj *iter;

    NST_D_EXTRACT("I", &iter);

    return nst_call_func(iter->get_val, &iter->value, err);
}

NST_FUNC_SIGN(iter_advance_)
{
    Nst_IterObj *iter;

    NST_D_EXTRACT("I", &iter);

    return nst_call_func(iter->advance, &iter->value, err);
}
