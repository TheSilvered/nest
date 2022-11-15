#include "nest_itutil.h"
#include "itutil_functions.h"

#define FUNC_COUNT 14

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

    if ( !nst_extract_arg_values("ii", arg_num, args, err, &start, &step) )
        return nullptr;

    // Layout: [idx, start, step]
    Nst_Obj *arr = nst_new_array(3);
    SEQ(arr)->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);
    nst_set_value_seq(arr, 2, args[1]);

    return nst_new_iter(
        FUNC(new_cfunc(1, count_start)),
        FUNC(new_cfunc(1, count_advance)),
        FUNC(new_cfunc(1, count_is_done)),
        FUNC(new_cfunc(1, count_get_val)),
        arr
    );
}

NST_FUNC_SIGN(cycle_)
{
    Nst_Obj *seq;

    if ( !nst_extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx, seq]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq;

    return nst_new_iter(
        FUNC(new_cfunc(1, cycle_start)),
        FUNC(new_cfunc(1, cycle_advance)),
        FUNC(new_cfunc(1, cycle_is_done)),
        FUNC(new_cfunc(1, cycle_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(repeat_)
{
    Nst_Int times;

    if ( !nst_extract_arg_values("i", arg_num - 1, args + 1, err, &times) )
        return nullptr;

    // Layout: [count, item, max_times]
    Nst_Obj *arr = nst_new_array(3);
    SEQ(arr)->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);
    nst_set_value_seq(arr, 2, args[1]);

    return nst_new_iter(
        FUNC(new_cfunc(1, repeat_start)),
        FUNC(new_cfunc(1, repeat_advance)),
        FUNC(new_cfunc(1, repeat_is_done)),
        FUNC(new_cfunc(1, repeat_get_val)),
        arr
    );
}

NST_FUNC_SIGN(chain_)
{
    Nst_SeqObj *seq;

    if ( !nst_extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx_global, idx_local, seq]
    Nst_SeqObj *arr = SEQ(nst_new_array(3));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = nst_new_int(0);
    nst_set_value_seq(arr, 2, args[0]);

    return nst_new_iter(
        FUNC(new_cfunc(1, chain_start)),
        FUNC(new_cfunc(1, chain_advance)),
        FUNC(new_cfunc(1, chain_is_done)),
        FUNC(new_cfunc(1, chain_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(zip_)
{
    Nst_Obj *seq1;
    Nst_Obj *seq2;

    if ( !nst_extract_arg_values("SS", arg_num, args, err, &seq1, &seq2) )
        return nullptr;

    // Layout: [idx, seq1, seq2]
    Nst_SeqObj *arr = SEQ(nst_new_array(3));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq1;
    arr->objs[2] = seq2;

    return nst_new_iter(
        FUNC(new_cfunc(1, zip_start)),
        FUNC(new_cfunc(1, zip_advance)),
        FUNC(new_cfunc(1, zip_is_done)),
        FUNC(new_cfunc(1, zip_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(enumerate_)
{
    Nst_Obj *seq;

    if ( !nst_extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx, seq]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq;

    return nst_new_iter(
        FUNC(new_cfunc(1, enumerate_start)),
        FUNC(new_cfunc(1, enumerate_advance)),
        FUNC(new_cfunc(1, enumerate_is_done)),
        FUNC(new_cfunc(1, enumerate_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(keys_)
{
    Nst_MapObj *map;

    if ( !nst_extract_arg_values("m", arg_num, args, err, &map) )
        return nullptr;

    // Layout: [idx, map]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);

    return nst_new_iter(
        FUNC(new_cfunc(1, kvi_start)),
        FUNC(new_cfunc(1, kvi_advance)),
        FUNC(new_cfunc(1, kvi_is_done)),
        FUNC(new_cfunc(1, keys_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(values_)
{
    Nst_MapObj *map;

    if ( !nst_extract_arg_values("m", arg_num, args, err, &map) )
        return nullptr;

    // Layout: [idx, map]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);

    return nst_new_iter(
        FUNC(new_cfunc(1, kvi_start)),
        FUNC(new_cfunc(1, kvi_advance)),
        FUNC(new_cfunc(1, kvi_is_done)),
        FUNC(new_cfunc(1, values_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(items_)
{
    Nst_MapObj *map;

    if ( !nst_extract_arg_values("m", arg_num, args, err, &map) )
        return nullptr;

    // Layout: [idx, map]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);

    return nst_new_iter(
        FUNC(new_cfunc(1, kvi_start)),
        FUNC(new_cfunc(1, kvi_advance)),
        FUNC(new_cfunc(1, kvi_is_done)),
        FUNC(new_cfunc(1, items_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(reversed_)
{
    Nst_Obj *seq;

    if ( !nst_extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx, seq]
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq;

    return nst_new_iter(
        FUNC(new_cfunc(1, reversed_start)),
        FUNC(new_cfunc(1, reversed_advance)),
        FUNC(new_cfunc(1, reversed_is_done)),
        FUNC(new_cfunc(1, reversed_get_val)),
        OBJ(arr)
    );
}

NST_FUNC_SIGN(iter_start_)
{
    Nst_IterObj *iter;

    if ( !nst_extract_arg_values("I", arg_num, args, err, &iter) )
        return nullptr;

    return nst_call_func(iter->start, &iter->value, err);
}

NST_FUNC_SIGN(iter_is_done_)
{
    Nst_IterObj *iter;

    if ( !nst_extract_arg_values("I", arg_num, args, err, &iter) )
        return nullptr;

    return nst_call_func(iter->is_done, &iter->value, err);
}

NST_FUNC_SIGN(iter_get_val_)
{
    Nst_IterObj *iter;

    if ( !nst_extract_arg_values("I", arg_num, args, err, &iter) )
        return nullptr;

    return nst_call_func(iter->get_val, &iter->value, err);
}

NST_FUNC_SIGN(iter_advance_)
{
    Nst_IterObj *iter;

    if ( !nst_extract_arg_values("I", arg_num, args, err, &iter) )
        return nullptr;

    return nst_call_func(iter->advance, &iter->value, err);
}
