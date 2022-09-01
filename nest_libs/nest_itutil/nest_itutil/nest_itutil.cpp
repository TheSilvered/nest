#include "nest_itutil.h"
#include "itutil_functions.h"

#define FUNC_COUNT 10

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(count, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(cycle, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(repeat, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(chain, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(zip, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(enumerate, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(keys, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(values, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(items, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(reversed, 1);

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

Nst_Obj *count(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_Int start;
    Nst_Int step;

    if ( !nst_extract_arg_values("ii", arg_num, args, err, &start, &step) )
        return nullptr;

    // Layout: [idx, start, step]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(3));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);
    nst_set_value_seq(arr, 2, args[1]);

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, count_start)),
        AS_FUNC(new_cfunc(1, count_advance)),
        AS_FUNC(new_cfunc(1, count_is_done)),
        AS_FUNC(new_cfunc(1, count_get_val)),
        (Nst_Obj *)arr
    );
}

Nst_Obj *cycle(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_Obj *seq;

    if ( !nst_extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx, seq]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq;

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, cycle_start)),
        AS_FUNC(new_cfunc(1, cycle_advance)),
        AS_FUNC(new_cfunc(1, cycle_is_done)),
        AS_FUNC(new_cfunc(1, cycle_get_val)),
        (Nst_Obj *)arr
    );
}

Nst_Obj *repeat(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_Int times;

    if ( !nst_extract_arg_values("i", arg_num - 1, args + 1, err, &times) )
        return nullptr;

    // Layout: [count, item, max_times]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(3));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);
    nst_set_value_seq(arr, 2, args[1]);

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, repeat_start)),
        AS_FUNC(new_cfunc(1, repeat_advance)),
        AS_FUNC(new_cfunc(1, repeat_is_done)),
        AS_FUNC(new_cfunc(1, repeat_get_val)),
        (Nst_Obj *)arr
    );
}

Nst_Obj *chain(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_SeqObj *seq;

    if ( !nst_extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx_global, idx_local, seq]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(3));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = nst_new_int(0);
    nst_set_value_seq(arr, 2, args[0]);

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, chain_start)),
        AS_FUNC(new_cfunc(1, chain_advance)),
        AS_FUNC(new_cfunc(1, chain_is_done)),
        AS_FUNC(new_cfunc(1, chain_get_val)),
        (Nst_Obj *)arr
    );
}

Nst_Obj *zip(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_Obj *seq1;
    Nst_Obj *seq2;

    if ( !nst_extract_arg_values("SS", arg_num, args, err, &seq1, &seq2) )
        return nullptr;

    // Layout: [idx, seq1, seq2]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(3));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq1;
    arr->objs[2] = seq2;

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, zip_start)),
        AS_FUNC(new_cfunc(1, zip_advance)),
        AS_FUNC(new_cfunc(1, zip_is_done)),
        AS_FUNC(new_cfunc(1, zip_get_val)),
        (Nst_Obj *)arr
    );
}

Nst_Obj *enumerate(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_Obj *seq;

    if ( !nst_extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx, seq]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq;

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, enumerate_start)),
        AS_FUNC(new_cfunc(1, enumerate_advance)),
        AS_FUNC(new_cfunc(1, enumerate_is_done)),
        AS_FUNC(new_cfunc(1, enumerate_get_val)),
        (Nst_Obj *)arr
    );
}

Nst_Obj *keys(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_MapObj *map;

    if ( !nst_extract_arg_values("m", arg_num, args, err, &map) )
        return nullptr;

    // Layout: [idx, map]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, kvi_start)),
        AS_FUNC(new_cfunc(1, kvi_advance)),
        AS_FUNC(new_cfunc(1, kvi_is_done)),
        AS_FUNC(new_cfunc(1, keys_get_val)),
        (Nst_Obj *)arr
    );
}

Nst_Obj *values(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_MapObj *map;

    if ( !nst_extract_arg_values("m", arg_num, args, err, &map) )
        return nullptr;

    // Layout: [idx, map]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, kvi_start)),
        AS_FUNC(new_cfunc(1, kvi_advance)),
        AS_FUNC(new_cfunc(1, kvi_is_done)),
        AS_FUNC(new_cfunc(1, values_get_val)),
        (Nst_Obj *)arr
    );
}

Nst_Obj *items(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_MapObj *map;

    if ( !nst_extract_arg_values("m", arg_num, args, err, &map) )
        return nullptr;

    // Layout: [idx, map]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    nst_set_value_seq(arr, 1, args[0]);

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, kvi_start)),
        AS_FUNC(new_cfunc(1, kvi_advance)),
        AS_FUNC(new_cfunc(1, kvi_is_done)),
        AS_FUNC(new_cfunc(1, items_get_val)),
        (Nst_Obj *)arr
    );
}

Nst_Obj *reversed(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_Obj *seq;

    if ( !nst_extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx, seq]
    Nst_SeqObj *arr = AS_SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(0);
    arr->objs[1] = seq;

    return nst_new_iter(
        AS_FUNC(new_cfunc(1, reversed_start)),
        AS_FUNC(new_cfunc(1, reversed_advance)),
        AS_FUNC(new_cfunc(1, reversed_is_done)),
        AS_FUNC(new_cfunc(1, reversed_get_val)),
        (Nst_Obj *)arr
    );
}
