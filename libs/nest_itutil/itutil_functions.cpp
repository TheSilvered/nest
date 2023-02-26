#include "itutil_functions.h"

// --------------------------------- Count --------------------------------- //
NST_FUNC_SIGN(count_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = AS_INT(objs[1]);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(count_is_done)
{
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(count_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *ob = nst_int_new(AS_INT(objs[0]));
    AS_INT(objs[0]) += AS_INT(objs[2]);
    return ob;
}

// --------------------------------- Cycle --------------------------------- //
NST_FUNC_SIGN(cycle_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(cycle_is_done)
{
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(cycle_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *ob = nst_seq_get(SEQ(objs[1]), AS_INT(objs[0]));
    AS_INT(objs[0]) += 1;
    AS_INT(objs[0]) %= SEQ(objs[1])->len;
    return ob;
}

// --------------------------------- Repeat -------------------------------- //
NST_FUNC_SIGN(repeat_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(repeat_is_done)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Int max_count = AS_INT(objs[2]);
    if ( max_count >= 0 && AS_INT(objs[0]) >= max_count )
    {
        NST_RETURN_TRUE;
    }
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(repeat_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    if ( AS_INT(objs[2]) >= 0 )
    {
        AS_INT(objs[0]) += 1;
    }
    return nst_inc_ref(objs[1]);
}

// --------------------------------- Chain --------------------------------- //

static Nst_Obj *get_first_iter_val(Nst_IterObj *iter, Nst_OpErr *err)
{
    if ( nst_iter_start(iter, err) )
    {
        return nullptr;
    }
    if ( nst_iter_is_done(iter, err) )
    {
        return nullptr;
    }
    return nst_iter_get_val(iter, err);
}

NST_FUNC_SIGN(chain_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IterObj *main_iter = ITER(objs[0]);
    Nst_Obj *local_val = get_first_iter_val(main_iter, err);
    if ( local_val == nullptr )
    {
        if ( err->name != nullptr )
        {
            return nullptr;
        }
        nst_seq_set(args[0], 3, nst_true());
        NST_RETURN_NULL;
    }
    Nst_IterObj *local_iter = ITER(nst_obj_cast(local_val, nst_type()->Iter, err));
    nst_dec_ref(local_val);
    if ( local_iter == nullptr )
    {
        return nullptr;
    }

    Nst_Obj *val;
    while ( true )
    {
        val = get_first_iter_val(local_iter, err);
        if ( val != nullptr )
        {
            goto end;
        }
        if ( err->name != nullptr )
        {
            return nullptr;
        }
        i32 res = nst_iter_is_done(main_iter, err);
        if ( res == -1 )
        {
            return nullptr;
        }
        else if ( res )
        {
            nst_seq_set(args[0], 3, nst_true());
            NST_RETURN_NULL;
        }

        local_val = nst_iter_get_val(main_iter, err);
        if ( local_val == nullptr )
        {
            return nullptr;
        }
        local_iter = ITER(nst_obj_cast(local_val, nst_type()->Iter, err));
        nst_dec_ref(local_val);
        if ( local_iter == nullptr )
        {
            return nullptr;
        }
    }
end:
    nst_seq_set(args[0], 1, local_iter);
    nst_seq_set(args[0], 2, val);
    nst_dec_ref(local_iter);
    nst_dec_ref(val);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(chain_is_done)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    return nst_inc_ref(objs[3]);
}

NST_FUNC_SIGN(chain_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IterObj *main_iter = ITER(objs[0]);
    Nst_IterObj *local_iter = ITER(objs[1]);
    Nst_Obj *return_ob = nst_inc_ref(objs[2]);
    Nst_Obj *val = nst_inc_ref(nst_null());

    i32 res = nst_iter_is_done(local_iter, err);
    if ( res == -1 )
    {
        return nullptr;
    }
    else if ( !res )
    {
        nst_dec_ref(val);
        val = nst_iter_get_val(local_iter, err);
        if ( val == nullptr )
        {
            return nullptr;
        }
        goto end;
    }
    nst_dec_ref(val);
    do
    {
        i32 res = nst_iter_is_done(main_iter, err);
        if ( res == -1 )
        {
            return nullptr;
        }
        else if ( res )
        {
            nst_seq_set(args[0], 3, nst_true());
            val = nst_inc_ref(nst_null());
            goto end;
        }

        Nst_Obj *local_val = nst_iter_get_val(main_iter, err);
        if ( local_val == nullptr )
        {
            return nullptr;
        }
        local_iter = ITER(nst_obj_cast(local_val, nst_type()->Iter, err));
        nst_dec_ref(local_val);
        if ( local_iter == nullptr )
        {
            return nullptr;
        }
        val = get_first_iter_val(local_iter, err);
        if ( val != nullptr )
        {
            nst_seq_set(args[0], 1, local_iter);
            nst_dec_ref(local_iter);
            goto end;
        }
        if ( err->name != nullptr )
        {
            return nullptr;
        }
    } while ( true );
end:
    nst_seq_set(args[0], 2, val);
    nst_dec_ref(val);
    return return_ob;
}

// ---------------------------------- Zip ---------------------------------- //
NST_FUNC_SIGN(zip_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(zip_is_done)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Int idx = AS_INT(objs[0]);

    if ( idx >= (Nst_Int)SEQ(objs[1])->len ||
         idx >= (Nst_Int)SEQ(objs[2])->len )
    {
        NST_RETURN_TRUE;
    }
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(zip_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Int idx = AS_INT(objs[0]);

    if ( idx >= (Nst_Int)SEQ(objs[1])->len )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            SEQ(objs[1])->type == nst_type()->Array
                ? _NST_EM_INDEX_OUT_OF_BOUNDS("Array")
                : _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            "iu",
            idx,
            SEQ(objs[1])->len));

        return nullptr;
    }
    else if ( idx >= (Nst_Int)SEQ(objs[2])->len )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            SEQ(objs[2])->type == nst_type()->Array
                ? _NST_EM_INDEX_OUT_OF_BOUNDS("Array")
                : _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            "iu",
            idx,
            SEQ(objs[2])->len));

        return nullptr;
    }

    Nst_SeqObj *arr = SEQ(nst_array_new(2));
    nst_seq_set(arr, 0, SEQ(objs[1])->objs[idx]);
    nst_seq_set(arr, 1, SEQ(objs[2])->objs[idx]);
    AS_INT(objs[0]) += 1;
    return OBJ(arr);
}

// ---------------------------- Zip n sequences ---------------------------- //

NST_FUNC_SIGN(zipn_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(zipn_is_done)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Int idx = AS_INT(objs[0]);

    for ( usize i = 1, n = SEQ(args[0])->len; i < n; i++ )
    {
        if ( idx >= (Nst_Int)SEQ(objs[i])->len )
        {
            NST_RETURN_TRUE;
        }
    }

    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(zipn_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Int idx = AS_INT(objs[0]);

    for ( usize i = 1, n = SEQ(args[0])->len; i < n; i++ )
    {
        if ( idx >= (Nst_Int)SEQ(objs[i])->len )
        {
            NST_SET_VALUE_ERROR(nst_format_error(
                SEQ(objs[i])->type == nst_type()->Array
                    ? _NST_EM_INDEX_OUT_OF_BOUNDS("Array")
                    : _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
                "iu",
                idx,
                SEQ(objs[i])->len));

            return nullptr;
        }
    }

    Nst_SeqObj *arr = SEQ(nst_array_new(SEQ(args[0])->len - 1));
    for ( usize i = 1, n = SEQ(args[0])->len; i < n; i++ )
    {
        nst_seq_set(arr, i - 1, SEQ(objs[i])->objs[idx]);
    }
    AS_INT(objs[0]) += 1;
    return OBJ(arr);
}

// ------------------------------- Enumerate ------------------------------- //
NST_FUNC_SIGN(enumerate_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = AS_INT(objs[2]);
    if ( nst_iter_start(objs[1], err) )
    {
        return nullptr;
    }
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(enumerate_is_done)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IterObj *iter = ITER(objs[1]);
    return nst_call_func(iter->is_done, &iter->value, err);
}

NST_FUNC_SIGN(enumerate_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Int idx = AS_INT(objs[0]);
    Nst_IterObj *iter = ITER(objs[1]);

    Nst_Obj *res = nst_call_func(iter->get_val, &iter->value, err);
    if ( res == nullptr )
    {
        return nullptr;
    }

    Nst_SeqObj *arr = SEQ(nst_array_new(2));
    arr->objs[0] = nst_int_new(idx);
    arr->objs[1] = res;
    AS_INT(objs[0]) += AS_INT(objs[3]);
    return OBJ(arr);
}

// -------------------------- Keys, values, items -------------------------- //
NST_FUNC_SIGN(kvi_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = nst_map_get_next_idx(-1, MAP(objs[1]));
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(kvi_is_done)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    if ( AS_INT(objs[0]) == -1 )
    {
        NST_RETURN_TRUE;
    }
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(keys_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;

    if ( AS_INT(objs[0]) == -1 )
    {
        NST_RETURN_NULL;
    }

    Nst_MapNode node = MAP(objs[1])->nodes[AS_INT(objs[0])];
    AS_INT(objs[0]) = nst_map_get_next_idx((i32)AS_INT(objs[0]), MAP(objs[1]));
    return nst_inc_ref(node.key);
}

NST_FUNC_SIGN(values_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;

    if ( AS_INT(objs[0]) == -1 )
    {
        NST_RETURN_NULL;
    }

    Nst_MapNode node = MAP(objs[1])->nodes[AS_INT(objs[0])];
    AS_INT(objs[0]) = nst_map_get_next_idx((i32)AS_INT(objs[0]), MAP(objs[1]));
    return nst_inc_ref(node.value);
}

NST_FUNC_SIGN(items_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_MapNode node = MAP(objs[1])->nodes[AS_INT(objs[0])];

    Nst_SeqObj *arr = SEQ(nst_array_new(2));

    if ( AS_INT(objs[0]) == -1 )
    {
        nst_seq_set(arr, 0, nst_null());
        nst_seq_set(arr, 1, nst_null());
    }
    else
    {
        nst_seq_set(arr, 0, node.key);
        nst_seq_set(arr, 1, node.value);
    }
    AS_INT(objs[0]) = nst_map_get_next_idx((i32)AS_INT(objs[0]), MAP(objs[1]));
    return OBJ(arr);
}

// -------------------------------- Reversed ------------------------------- //
NST_FUNC_SIGN(reversed_start)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = SEQ(objs[1])->len - 1;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(reversed_is_done)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    if ( AS_INT(objs[0]) == -1 )
    {
        NST_RETURN_TRUE;
    }
    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(reversed_get_val)
{
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *res = nst_seq_get(SEQ(objs[1]), AS_INT(objs[0]));

    if ( res == nullptr )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            SEQ(objs[1])->type == nst_type()->Array
                ? _NST_EM_INDEX_OUT_OF_BOUNDS("Array")
                : _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            "iu",
            AS_INT(objs[0]),
            SEQ(objs[1])->len));

        return nullptr;
    }
    Nst_Int len = SEQ(objs[1])->len;
    AS_INT(objs[0]) -= 1;

    if ( AS_INT(objs[0]) >= len )
    {
        AS_INT(objs[0]) = len - 1;
    }
    return res;
}
