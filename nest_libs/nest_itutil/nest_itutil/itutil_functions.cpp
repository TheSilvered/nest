#include "itutil_functions.h"

// --------------------------------- Count ---------------------------------- //
NST_FUNC_SIGN(count_start)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = AS_INT(objs[1]);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(count_advance)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) += AS_INT(objs[2]);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(count_is_done)
{
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(count_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    return nst_new_int(AS_INT(objs[0]));
}

// --------------------------------- Cycle ---------------------------------- //
NST_FUNC_SIGN(cycle_start)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(cycle_advance)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) += 1;
    AS_INT(objs[0]) %= AS_SEQ(objs[1])->len;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(cycle_is_done)
{
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(cycle_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    return nst_get_value_seq(AS_SEQ(objs[1]), AS_INT(objs[0]));
}

// --------------------------------- Repeat --------------------------------- //
NST_FUNC_SIGN(repeat_start)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(repeat_advance)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    if ( AS_INT(objs[2]) >= 0 )
        AS_INT(objs[0]) += 1;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(repeat_is_done)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_Int max_count = AS_INT(objs[2]);
    if ( max_count >= 0 && AS_INT(objs[0]) >= max_count )
        NST_RETURN_TRUE;
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(repeat_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    nst_inc_ref(objs[1]);
    return objs[1];
}

// --------------------------------- Chain ---------------------------------- //
NST_FUNC_SIGN(chain_start)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    AS_INT(objs[1]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(chain_advance)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[1]) += 1;
    Nst_Int idx = AS_INT(objs[0]);
    Nst_SeqObj *seq = AS_SEQ(objs[2]);

    if ( idx >= (Nst_Int)seq->len )
    {
        NST_SET_VALUE_ERROR(_nst_format_idx_error(
            seq->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                   : INDEX_OUT_OF_BOUNDS("Vector"),
            AS_INT(objs[1]),
            seq->len
        ));

        return nullptr;
    }

    if ( AS_INT(objs[1]) >= (Nst_Int)AS_SEQ(seq->objs[idx])->len )
    {
        AS_INT(objs[1]) = 0;
        AS_INT(objs[0]) += 1;
    }
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(chain_is_done)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_SeqObj *seq = AS_SEQ(objs[2]);

    if ( AS_INT(objs[0]) >= (Nst_Int)seq->len )
        NST_RETURN_TRUE;

    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(chain_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_SeqObj *seq = AS_SEQ(objs[2]);
    Nst_Int idx1 = AS_INT(objs[0]);
    Nst_Int idx2 = AS_INT(objs[1]);
    Nst_SeqObj *sub_seq = AS_SEQ(seq->objs[idx1]);

    if ( sub_seq->type != nst_t_arr && sub_seq->type != nst_t_vect )
    {
        NST_SET_VALUE_ERROR(
            _nst_format_type_error(EXPECTED_TYPE("Array' or 'Vector"),
                TYPE_NAME(sub_seq))
        );
        return nullptr;
    }

    Nst_Obj *res = nst_get_value_seq(AS_SEQ(sub_seq), idx2);

    if ( res == NULL )
    {
        NST_SET_VALUE_ERROR(_nst_format_idx_error(
            sub_seq->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                       : INDEX_OUT_OF_BOUNDS("Vector"),
            idx2,
            sub_seq->len
        ));

        return nullptr;
    }

    return res;
}

// ---------------------------------- Zip ----------------------------------- //
NST_FUNC_SIGN(zip_start)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(zip_advance)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) += 1;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(zip_is_done)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_Int idx = AS_INT(objs[0]);

    if ( idx >= (Nst_Int)AS_SEQ(objs[1])->len || idx >= (Nst_Int)AS_SEQ(objs[2])->len )
        NST_RETURN_TRUE;
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(zip_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_Int idx = AS_INT(objs[0]);

    if ( idx >= (Nst_Int)AS_SEQ(objs[1])->len )
    {
        NST_SET_VALUE_ERROR(_nst_format_idx_error(
            AS_SEQ(objs[1])->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                               : INDEX_OUT_OF_BOUNDS("Vector"),
            idx,
            AS_SEQ(objs[1])->len
        ));

        return nullptr;
    }
    else if ( idx >= (Nst_Int)AS_SEQ(objs[2])->len )
    {
        NST_SET_VALUE_ERROR(_nst_format_idx_error(
            AS_SEQ(objs[2])->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
            : INDEX_OUT_OF_BOUNDS("Vector"),
            idx,
            AS_SEQ(objs[2])->len
        ));

        return nullptr;
    }

    Nst_SeqObj *arr = AS_SEQ(nst_new_array(2));
    nst_set_value_seq(arr, 0, AS_SEQ(objs[1])->objs[idx]);
    nst_set_value_seq(arr, 1, AS_SEQ(objs[2])->objs[idx]);

    return (Nst_Obj *)arr;
}

// ------------------------------- Enumerate -------------------------------- //
NST_FUNC_SIGN(enumerate_start)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(enumerate_advance)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) += 1;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(enumerate_is_done)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    if ( AS_INT(objs[0]) >= (Nst_Int)AS_SEQ(objs[1])->len )
        NST_RETURN_TRUE;
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(enumerate_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_Int idx = AS_INT(objs[0]);

    if ( idx >= (Nst_Int)AS_SEQ(objs[1])->len )
    {
        NST_SET_VALUE_ERROR(_nst_format_idx_error(
            AS_SEQ(objs[1])->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
            : INDEX_OUT_OF_BOUNDS("Vector"),
            idx,
            AS_SEQ(objs[1])->len
        ));

        return nullptr;
    }

    Nst_SeqObj *arr = AS_SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_int(idx);
    nst_set_value_seq(arr, 1, AS_SEQ(objs[1])->objs[idx]);

    return (Nst_Obj *)arr;
}

// -------------------------- Keys, values, items --------------------------- //
NST_FUNC_SIGN(kvi_start)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = nst_map_get_next_idx(-1, AS_MAP(objs[1]));
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(kvi_advance)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = nst_map_get_next_idx(AS_INT(objs[0]), AS_MAP(objs[1]));
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(kvi_is_done)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    if ( AS_INT(objs[0]) == -1 )
        NST_RETURN_TRUE;
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(keys_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;

    if ( AS_INT(objs[0]) == -1 )
        NST_RETURN_NULL;

    Nst_MapNode node = AS_MAP(objs[1])->nodes[AS_INT(objs[0])];
    return nst_inc_ref(node.key);
}

NST_FUNC_SIGN(values_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    
    if ( AS_INT(objs[0]) == -1 )
        NST_RETURN_NULL;
    
    Nst_MapNode node = AS_MAP(objs[1])->nodes[AS_INT(objs[0])];
    return nst_inc_ref(node.value);
}

NST_FUNC_SIGN(items_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_MapNode node = AS_MAP(objs[1])->nodes[AS_INT(objs[0])];

    Nst_SeqObj *arr = AS_SEQ(nst_new_array(2));

    if ( AS_INT(objs[0]) == -1 )
    {
        nst_set_value_seq(arr, 0, nst_null);
        nst_set_value_seq(arr, 1, nst_null);
    }
    else
    {
        nst_set_value_seq(arr, 0, node.key);
        nst_set_value_seq(arr, 1, node.value);
    }

    return (Nst_Obj *)arr;
}

// -------------------------------- Reversed -------------------------------- //
NST_FUNC_SIGN(reversed_start)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    AS_INT(objs[0]) = AS_SEQ(objs[1])->len - 1;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(reversed_advance)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_Int len = AS_SEQ(objs[1])->len;
    AS_INT(objs[0]) -= 1;

    if ( AS_INT(objs[0]) >= len )
        AS_INT(objs[0]) = len - 1;

    NST_RETURN_NULL;
}

NST_FUNC_SIGN(reversed_is_done)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    if ( AS_INT(objs[0]) == -1 )
        NST_RETURN_TRUE;
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(reversed_get_val)
{
    Nst_Obj **objs = AS_SEQ(args[0])->objs;
    Nst_Obj *res = nst_get_value_seq(AS_SEQ(objs[1]), AS_INT(objs[0]));

    if ( res == NULL )
    {
        NST_SET_VALUE_ERROR(_nst_format_idx_error(
            AS_SEQ(objs[1])->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                               : INDEX_OUT_OF_BOUNDS("Vector"),
            AS_INT(objs[0]),
            AS_SEQ(objs[1])->len
        ));

        return nullptr;
    }

    return res;
}
