#include "itutil_functions.h"

// --------------------------------- Count ---------------------------------- //
Nst_Obj *count_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = AS_INT(objs[1]);
    return inc_ref(nst_null);
}

Nst_Obj *count_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) += AS_INT(objs[2]);
    return inc_ref(nst_null);
}

Nst_Obj *count_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return inc_ref(nst_false);
}

Nst_Obj *count_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    return new_int_obj(AS_INT(objs[0]));
}

// --------------------------------- Cycle ---------------------------------- //
Nst_Obj *cycle_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    return inc_ref(nst_null);
}

Nst_Obj *cycle_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) += 1;
    AS_INT(objs[0]) %= AS_SEQ(objs[1])->len;
    return inc_ref(nst_null);
}

Nst_Obj *cycle_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return inc_ref(nst_false);
}

Nst_Obj *cycle_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    return get_value_seq(AS_SEQ(objs[1]), AS_INT(objs[0]));
}

// --------------------------------- Repeat --------------------------------- //
Nst_Obj *repeat_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    return inc_ref(nst_null);
}

Nst_Obj *repeat_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    if ( AS_INT(objs[2]) >= 0 )
        AS_INT(objs[0]) += 1;
    return inc_ref(nst_null);
}

Nst_Obj *repeat_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_int max_count = AS_INT(objs[2]);
    if ( max_count >= 0 && AS_INT(objs[0]) >= max_count )
        return inc_ref(nst_true);
    return inc_ref(nst_false);
}

Nst_Obj *repeat_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    inc_ref(objs[1]);
    return objs[1];
}

// --------------------------------- Chain ---------------------------------- //
Nst_Obj *chain_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    AS_INT(objs[1]) = 0;
    return inc_ref(nst_null);
}

Nst_Obj *chain_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[1]) += 1;
    Nst_int idx = AS_INT(objs[0]);
    Nst_sequence *seq = AS_SEQ(objs[2]);

    if ( AS_INT(objs[1]) >= (Nst_int)AS_SEQ(seq->objs[idx])->len )
    {
        AS_INT(objs[1]) = 0;
        AS_INT(objs[0]) += 1;
    }
    return inc_ref(nst_null);
}

Nst_Obj *chain_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_int idx = AS_INT(objs[0]);
    Nst_sequence *seq = AS_SEQ(objs[2]);
    Nst_Obj *sub_seq = seq->objs[idx];

    if ( idx >= (Nst_int)seq->len )
        return inc_ref(nst_true);

    return inc_ref(nst_false);
}

Nst_Obj *chain_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_sequence *seq = AS_SEQ(objs[2]);
    Nst_int idx1 = AS_INT(objs[0]);
    Nst_int idx2 = AS_INT(objs[1]);
    Nst_Obj *sub_seq = seq->objs[idx1];

    if ( sub_seq->type != nst_t_arr && sub_seq->type != nst_t_vect )
    {
        SET_VALUE_ERROR(
            format_type_error(EXPECTED_TYPE("Array' or 'Vector"),
                sub_seq->type_name)
        );
        return nullptr;
    }

    return get_value_seq(AS_SEQ(sub_seq), idx2);
}

// ---------------------------------- Zip ----------------------------------- //
Nst_Obj *zip_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    return inc_ref(nst_null);
}

Nst_Obj *zip_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) += 1;
    return inc_ref(nst_null);
}

Nst_Obj *zip_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_int idx = AS_INT(objs[0]);

    if ( idx >= (Nst_int)AS_SEQ(objs[1])->len || idx >= (Nst_int)AS_SEQ(objs[2])->len )
        return inc_ref(nst_true);
    return inc_ref(nst_false);
}

Nst_Obj *zip_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_int idx = AS_INT(objs[0]);

    Nst_sequence *arr = new_array_empty(2);
    set_value_seq(arr, 0, AS_SEQ(objs[1])->objs[idx]);
    set_value_seq(arr, 1, AS_SEQ(objs[2])->objs[idx]);

    return new_arr_obj(arr);
}

// ------------------------------- Enumerate -------------------------------- //
Nst_Obj *enumerate_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    return inc_ref(nst_null);
}

Nst_Obj *enumerate_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) += 1;
    return inc_ref(nst_null);
}

Nst_Obj *enumerate_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    if ( AS_INT(objs[0]) >= (Nst_int)AS_SEQ(objs[1])->len )
        return inc_ref(nst_true);
    return inc_ref(nst_false);
}

Nst_Obj *enumerate_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_sequence *arr = new_array_empty(2);
    Nst_int idx = AS_INT(objs[0]);

    arr->objs[0] = new_int_obj(idx);
    set_value_seq(arr, 1, AS_SEQ(objs[1])->objs[idx]);

    return new_arr_obj(arr);
}

// -------------------------- Keys, values, items --------------------------- //
Nst_Obj *kvi_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = get_next_idx(-1, AS_MAP(objs[1]));
    return inc_ref(nst_null);
}

Nst_Obj *kvi_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = get_next_idx(AS_INT(objs[0]), AS_MAP(objs[1]));
    return inc_ref(nst_null);
}

Nst_Obj *kvi_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    if ( AS_INT(objs[0]) == -1 )
        return inc_ref(nst_true);
    return inc_ref(nst_false);
}

Nst_Obj *keys_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    MapNode node = AS_MAP(objs[1])->nodes[AS_INT(objs[0])];

    return inc_ref(node.key);
}

Nst_Obj *values_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    MapNode node = AS_MAP(objs[1])->nodes[AS_INT(objs[0])];

    return inc_ref(node.value);
}

Nst_Obj *items_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    MapNode node = AS_MAP(objs[1])->nodes[AS_INT(objs[0])];

    Nst_sequence *arr = new_array_empty(2);
    set_value_seq(arr, 0, node.key);
    set_value_seq(arr, 1, node.value);

    return new_arr_obj(arr);
}

// -------------------------------- Reversed -------------------------------- //
Nst_Obj *reversed_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = AS_SEQ(objs[1])->len - 1;
    return inc_ref(nst_null);
}

Nst_Obj *reversed_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_int len = AS_SEQ(objs[1])->len;
    AS_INT(objs[0]) -= 1;

    if ( AS_INT(objs[0]) >= len )
        AS_INT(objs[0]) = len - 1;

    return inc_ref(nst_null);
}

Nst_Obj *reversed_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    if ( AS_INT(objs[0]) == -1 )
        return inc_ref(nst_true);
    return inc_ref(nst_false);
}

Nst_Obj *reversed_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    return get_value_seq(AS_SEQ(objs[1]), AS_INT(objs[0]));
}
