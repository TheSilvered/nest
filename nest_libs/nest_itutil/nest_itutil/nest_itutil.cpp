#include "nest_itutil.h"
#include "itutil_functions.h"

#define FUNC_COUNT 10

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = MAKE_FUNCDECLR(count, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(cycle, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(repeat, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(chain, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(zip, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(enumerate, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(keys, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(values, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(items, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(reversed, 1);

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

Nst_Obj *count(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_int start;
    Nst_int step;

    if ( !extract_arg_values("ii", arg_num, args, err, &start, &step) )
        return nullptr;

    // Layout: [idx, start, step]
    Nst_sequence *arr = new_array_empty(3);
    arr->objs[0] = new_int_obj(0);
    set_value_seq(arr, 1, args[0]);
    set_value_seq(arr, 2, args[1]);

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, count_start)),
        new_func_obj(new_cfunc(1, count_advance)),
        new_func_obj(new_cfunc(1, count_is_done)),
        new_func_obj(new_cfunc(1, count_get_val)),
        new_arr_obj(arr)
    ));
}

Nst_Obj *cycle(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *seq;

    if ( !extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx, seq]
    Nst_sequence *arr = new_array_empty(2);
    arr->objs[0] = new_int_obj(0);
    arr->objs[1] = seq;

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, cycle_start)),
        new_func_obj(new_cfunc(1, cycle_advance)),
        new_func_obj(new_cfunc(1, cycle_is_done)),
        new_func_obj(new_cfunc(1, cycle_get_val)),
        new_arr_obj(arr)
    ));
}

Nst_Obj *repeat(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_int times;

    if ( !extract_arg_values("i", arg_num - 1, args + 1, err, &times) )
        return nullptr;

    // Layout: [count, item, max_times]
    Nst_sequence *arr = new_array_empty(3);
    arr->objs[0] = new_int_obj(0);
    set_value_seq(arr, 1, args[0]);
    set_value_seq(arr, 2, args[1]);

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, repeat_start)),
        new_func_obj(new_cfunc(1, repeat_advance)),
        new_func_obj(new_cfunc(1, repeat_is_done)),
        new_func_obj(new_cfunc(1, repeat_get_val)),
        new_arr_obj(arr)
    ));
}

Nst_Obj *chain(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_sequence *seq;

    if ( !extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx_global, idx_local, seq]
    Nst_sequence *arr = new_array_empty(3);
    arr->objs[0] = new_int_obj(0);
    arr->objs[1] = new_int_obj(0);
    set_value_seq(arr, 2, args[0]);

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, chain_start)),
        new_func_obj(new_cfunc(1, chain_advance)),
        new_func_obj(new_cfunc(1, chain_is_done)),
        new_func_obj(new_cfunc(1, chain_get_val)),
        new_arr_obj(arr)
    ));
}

Nst_Obj *zip(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *seq1;
    Nst_Obj *seq2;

    if ( !extract_arg_values("SS", arg_num, args, err, &seq1, &seq2) )
        return nullptr;

    // Layout: [idx, seq1, seq2]
    Nst_sequence *arr = new_array_empty(3);
    arr->objs[0] = new_int_obj(0);
    arr->objs[1] = seq1;
    arr->objs[2] = seq2;

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, zip_start)),
        new_func_obj(new_cfunc(1, zip_advance)),
        new_func_obj(new_cfunc(1, zip_is_done)),
        new_func_obj(new_cfunc(1, zip_get_val)),
        new_arr_obj(arr)
    ));
}

Nst_Obj *enumerate(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *seq;

    if ( !extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx, seq]
    Nst_sequence *arr = new_array_empty(2);
    arr->objs[0] = new_int_obj(0);
    arr->objs[1] = seq;

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, enumerate_start)),
        new_func_obj(new_cfunc(1, enumerate_advance)),
        new_func_obj(new_cfunc(1, enumerate_is_done)),
        new_func_obj(new_cfunc(1, enumerate_get_val)),
        new_arr_obj(arr)
    ));
}

Nst_Obj *keys(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_map *map;

    if ( !extract_arg_values("m", arg_num, args, err, &map) )
        return nullptr;

    // Layout: [idx, map]
    Nst_sequence *arr = new_array_empty(2);
    arr->objs[0] = new_int_obj(0);
    set_value_seq(arr, 1, args[0]);

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, kvi_start)),
        new_func_obj(new_cfunc(1, kvi_advance)),
        new_func_obj(new_cfunc(1, kvi_is_done)),
        new_func_obj(new_cfunc(1, keys_get_val)),
        new_arr_obj(arr)
    ));
}

Nst_Obj *values(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_map *map;

    if ( !extract_arg_values("m", arg_num, args, err, &map) )
        return nullptr;

    // Layout: [idx, map]
    Nst_sequence *arr = new_array_empty(2);
    arr->objs[0] = new_int_obj(0);
    set_value_seq(arr, 1, args[0]);

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, kvi_start)),
        new_func_obj(new_cfunc(1, kvi_advance)),
        new_func_obj(new_cfunc(1, kvi_is_done)),
        new_func_obj(new_cfunc(1, values_get_val)),
        new_arr_obj(arr)
    ));
}

Nst_Obj *items(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_map *map;

    if ( !extract_arg_values("m", arg_num, args, err, &map) )
        return nullptr;

    // Layout: [idx, map]
    Nst_sequence *arr = new_array_empty(2);
    arr->objs[0] = new_int_obj(0);
    set_value_seq(arr, 1, args[0]);

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, kvi_start)),
        new_func_obj(new_cfunc(1, kvi_advance)),
        new_func_obj(new_cfunc(1, kvi_is_done)),
        new_func_obj(new_cfunc(1, items_get_val)),
        new_arr_obj(arr)
    ));
}

Nst_Obj *reversed(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *seq;

    if ( !extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    // Layout: [idx, seq]
    Nst_sequence *arr = new_array_empty(2);
    arr->objs[0] = new_int_obj(0);
    arr->objs[1] = seq;

    return new_iter_obj(new_iter(
        new_func_obj(new_cfunc(1, reversed_start)),
        new_func_obj(new_cfunc(1, reversed_advance)),
        new_func_obj(new_cfunc(1, reversed_is_done)),
        new_func_obj(new_cfunc(1, reversed_get_val)),
        new_arr_obj(arr)
    ));
}
