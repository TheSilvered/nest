#include "itutil_functions.h"

// --------------------------------- Count --------------------------------- //
Nst_FUNC_SIGN(count_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = AS_INT(objs[1]);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(count_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(count_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *ob = Nst_int_new(AS_INT(objs[0]));
    AS_INT(objs[0]) += AS_INT(objs[2]);
    return ob;
}

// --------------------------------- Cycle --------------------------------- //
Nst_FUNC_SIGN(cycle_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(cycle_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(cycle_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *ob = Nst_seq_get(SEQ(objs[1]), AS_INT(objs[0]));
    AS_INT(objs[0]) += 1;
    AS_INT(objs[0]) %= SEQ(objs[1])->len;
    return ob;
}

// --------------------------------- Repeat -------------------------------- //
Nst_FUNC_SIGN(repeat_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(repeat_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    i64 max_count = AS_INT(objs[2]);
    if (max_count >= 0 && AS_INT(objs[0]) >= max_count)
        Nst_RETURN_TRUE;
    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(repeat_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    if (AS_INT(objs[2]) >= 0)
        AS_INT(objs[0]) += 1;
    return Nst_inc_ref(objs[1]);
}

// --------------------------------- Chain --------------------------------- //

static Nst_Obj *get_first_iter_val(Nst_IterObj *iter)
{
    if (Nst_iter_start(iter))
        return nullptr;
    if (Nst_iter_is_done(iter))
        return nullptr;
    return Nst_iter_get_val(iter);
}

Nst_FUNC_SIGN(chain_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IterObj *main_iter = ITER(objs[0]);
    Nst_Obj *local_val = get_first_iter_val(main_iter);
    if (local_val == nullptr) {
        if (Nst_error_occurred())
            return nullptr;
        Nst_seq_set(args[0], 3, Nst_true());
        Nst_RETURN_NULL;
    }
    Nst_IterObj *local_iter = ITER(Nst_obj_cast(local_val, Nst_type()->Iter));
    Nst_dec_ref(local_val);
    if (local_iter == nullptr)
        return nullptr;

    Nst_Obj *val;
    while (true) {
        val = get_first_iter_val(local_iter);
        if (val != nullptr)
            goto end;
        if (Nst_error_occurred())
            return nullptr;
        i32 res = Nst_iter_is_done(main_iter);
        if (res == -1)
            return nullptr;
        else if (res) {
            Nst_seq_set(args[0], 3, Nst_true());
            Nst_RETURN_NULL;
        }

        local_val = Nst_iter_get_val(main_iter);
        if (local_val == nullptr)
            return nullptr;
        local_iter = ITER(Nst_obj_cast(local_val, Nst_type()->Iter));
        Nst_dec_ref(local_val);
        if (local_iter == nullptr)
            return nullptr;
    }
end:
    Nst_seq_set(args[0], 1, local_iter);
    Nst_seq_set(args[0], 2, val);
    Nst_dec_ref(local_iter);
    Nst_dec_ref(val);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(chain_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    return Nst_inc_ref(objs[3]);
}

Nst_FUNC_SIGN(chain_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IterObj *main_iter = ITER(objs[0]);
    Nst_IterObj *local_iter = ITER(objs[1]);
    Nst_Obj *return_ob = Nst_inc_ref(objs[2]);
    Nst_Obj *val = Nst_inc_ref(Nst_null());

    i32 res = Nst_iter_is_done(local_iter);
    if (res == -1) {
        Nst_dec_ref(return_ob);
        return nullptr;
    } else if (!res) {
        Nst_dec_ref(val);
        val = Nst_iter_get_val(local_iter);
        if (val == nullptr) {
            Nst_dec_ref(return_ob);
            return nullptr;
        }
        goto end;
    }
    Nst_dec_ref(val);
    do {
        res = Nst_iter_is_done(main_iter);
        if (res == -1) {
            Nst_dec_ref(return_ob);
            return nullptr;
        } else if (res) {
            Nst_seq_set(args[0], 3, Nst_true());
            val = Nst_inc_ref(Nst_null());
            goto end;
        }

        Nst_Obj *local_val = Nst_iter_get_val(main_iter);
        if (local_val == nullptr) {
            Nst_dec_ref(return_ob);
            return nullptr;
        }
        local_iter = ITER(Nst_obj_cast(local_val, Nst_type()->Iter));
        Nst_dec_ref(local_val);
        if (local_iter == nullptr) {
            Nst_dec_ref(return_ob);
            return nullptr;
        }
        val = get_first_iter_val(local_iter);
        if (val != nullptr) {
            Nst_seq_set(args[0], 1, local_iter);
            Nst_dec_ref(local_iter);
            goto end;
        }
        if (Nst_error_occurred()) {
            Nst_dec_ref(return_ob);
            return nullptr;
        }
    } while (true);
end:
    Nst_seq_set(args[0], 2, val);
    Nst_dec_ref(val);
    return return_ob;
}

// ---------------------------------- Zip ---------------------------------- //
Nst_FUNC_SIGN(zip_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    if (Nst_iter_start(objs[0]) || Nst_iter_start(objs[1]))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(zip_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;

    i32 res = Nst_iter_is_done(objs[0]);
    if (res == -1)
        return nullptr;
    else if (res)
        Nst_RETURN_TRUE;

    res = Nst_iter_is_done(objs[1]);
    if (res == -1)
        return nullptr;
    else if (res)
        Nst_RETURN_TRUE;

    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(zip_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;

    Nst_Obj *ob1 = Nst_iter_get_val(objs[0]);
    if (ob1 == nullptr)
        return nullptr;
    Nst_Obj *ob2 = Nst_iter_get_val(objs[1]);
    if (ob2 == nullptr) {
        Nst_dec_ref(ob1);
        return nullptr;
    }

    return Nst_array_create(2, ob1, ob2);
}

// ---------------------------- Zip n sequences ---------------------------- //

Nst_FUNC_SIGN(zipn_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    for (usize i = 0, n = (usize)AS_INT(objs[0]); i < n; i++) {
        if (Nst_iter_start(objs[i + 1]))
            return nullptr;
    }
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(zipn_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;

    for (usize i = 0, n = (usize)AS_INT(objs[0]); i < n; i++) {
        i32 res = Nst_iter_is_done(objs[i + 1]);
        if (res == -1)
            return nullptr;
        else if (res == 1)
            Nst_RETURN_TRUE;
    }

    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(zipn_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;

    Nst_SeqObj *arr = SEQ(Nst_array_new((usize)AS_INT(objs[0])));
    for (usize i = 0, n = (usize)AS_INT(objs[0]); i < n; i++) {
        Nst_Obj *res = Nst_iter_get_val(objs[i + 1]);
        if (res == nullptr) {
            arr->len = i;
            Nst_dec_ref(arr);
            return nullptr;
        }
        arr->objs[i] = res;
    }

    return OBJ(arr);
}

// ------------------------------- Enumerate ------------------------------- //
Nst_FUNC_SIGN(enumerate_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = AS_INT(objs[2]);
    if (Nst_iter_start(objs[1]))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(enumerate_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IterObj *iter = ITER(objs[1]);
    return Nst_call_func(iter->is_done, &iter->value);
}

Nst_FUNC_SIGN(enumerate_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    i64 idx = AS_INT(objs[0]);
    Nst_IterObj *iter = ITER(objs[1]);

    Nst_Obj *res = Nst_call_func(iter->get_val, &iter->value);
    if (res == nullptr)
        return nullptr;

    Nst_SeqObj *arr = SEQ(Nst_array_new(2));
    if (objs[4] == Nst_true()) {
        arr->objs[0] = res;
        arr->objs[1] = Nst_int_new(idx);
    } else {
        arr->objs[0] = Nst_int_new(idx);
        arr->objs[1] = res;
    }

    AS_INT(objs[0]) += AS_INT(objs[3]);
    return OBJ(arr);
}

// ----------------------------- Keys & values ----------------------------- //

Nst_FUNC_SIGN(keys_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;

    if (AS_INT(objs[0]) == -1)
        Nst_RETURN_NULL;

    Nst_MapNode node = MAP(objs[1])->nodes[AS_INT(objs[0])];
    AS_INT(objs[0]) = Nst_map_get_next_idx((i32)AS_INT(objs[0]), MAP(objs[1]));
    return Nst_inc_ref(node.key);
}

Nst_FUNC_SIGN(values_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;

    if (AS_INT(objs[0]) == -1)
        Nst_RETURN_NULL;

    Nst_MapNode node = MAP(objs[1])->nodes[AS_INT(objs[0])];
    AS_INT(objs[0]) = Nst_map_get_next_idx((i32)AS_INT(objs[0]), MAP(objs[1]));
    return Nst_inc_ref(node.value);
}

// -------------------------------- Reversed ------------------------------- //
Nst_FUNC_SIGN(reversed_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = SEQ(objs[1])->len - 1;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(reversed_is_done)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_RETURN_COND(AS_INT(objs[0]) == -1);
}

Nst_FUNC_SIGN(reversed_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *res = Nst_seq_get(SEQ(objs[1]), AS_INT(objs[0]));

    if (res == nullptr) {
        Nst_set_value_error(Nst_sprintf(
            Nst_T(SEQ(objs[1]), Array)
                ? _Nst_EM_INDEX_OUT_OF_BOUNDS("Array")
                : _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            AS_INT(objs[0]),
            SEQ(objs[1])->len));

        return nullptr;
    }
    i64 len = SEQ(objs[1])->len;
    AS_INT(objs[0]) -= 1;

    if (AS_INT(objs[0]) >= len)
        AS_INT(objs[0]) = len - 1;
    return res;
}
