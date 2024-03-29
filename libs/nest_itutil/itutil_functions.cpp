#include "itutil_functions.h"

// --------------------------------- Count --------------------------------- //
Nst_FUNC_SIGN(count_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = AS_INT(objs[1]);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(count_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *ob = Nst_int_new(AS_INT(objs[0]));
    AS_INT(objs[0]) += AS_INT(objs[2]);
    return ob;
}

// ------------------------------ Cycle String ----------------------------- //
Nst_FUNC_SIGN(cycle_str_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(cycle_str_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *ob = Nst_string_get(objs[1], AS_INT(objs[0]));
    AS_INT(objs[0]) += 1;
    AS_INT(objs[0]) %= STR(objs[1])->len;
    return ob;
}

// ----------------------------- Cycle Sequence ---------------------------- //
Nst_FUNC_SIGN(cycle_seq_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    AS_INT(objs[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(cycle_seq_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *ob = Nst_seq_get(objs[1], AS_INT(objs[0]));
    AS_INT(objs[0]) += 1;
    AS_INT(objs[0]) %= SEQ(objs[1])->len;
    return ob;
}

// ----------------------------- Cycle Iterator ---------------------------- //
Nst_FUNC_SIGN(cycle_iter_start)
{
    Nst_UNUSED(arg_num);
    if (Nst_iter_start(args[0]))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(cycle_iter_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj *obj = Nst_iter_get_val(args[0]);
    if (obj != Nst_iend())
        return obj;
    Nst_dec_ref(obj);

    if (Nst_iter_start(args[0]))
        return nullptr;

    obj = Nst_iter_get_val(args[0]);
    if (obj != Nst_iend())
        return obj;
    Nst_dec_ref(obj);
    Nst_RETURN_NULL;
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

    i64 max_count = AS_INT(objs[2]);
    if (max_count >= 0 && AS_INT(objs[0]) >= max_count)
        return Nst_iend_ref();

    if (max_count >= 0)
        AS_INT(objs[0]) += 1;
    return Nst_inc_ref(objs[1]);
}

// --------------------------------- Chain --------------------------------- //

Nst_FUNC_SIGN(chain_start)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_IterObj *main_iter = ITER(objs[0]);

    if (Nst_iter_start(main_iter))
        return nullptr;

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(chain_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    Nst_Obj *main_iter = objs[0];
    Nst_Obj *current_iter = Nst_inc_ref(objs[1]);
    Nst_Obj *val = OBJ(current_iter) == Nst_null()
        ? Nst_iend_ref()
        : Nst_iter_get_val(current_iter);

    while (val == Nst_iend()) {
        Nst_dec_ref(val);
        Nst_Obj *next_iterable = Nst_iter_get_val(main_iter);
        if (next_iterable == Nst_iend())
            return next_iterable;
        else if (next_iterable == nullptr)
            return nullptr;

        Nst_Obj *next_iter = Nst_obj_cast(next_iterable, Nst_type()->Iter);
        Nst_dec_ref(next_iterable);
        if (next_iter == nullptr)
            return nullptr;

        Nst_dec_ref(current_iter);
        current_iter = next_iter;
        if (Nst_iter_start(current_iter))
            return nullptr;

        val = Nst_iter_get_val(current_iter);
    }

    Nst_seq_set(args[0], 1, current_iter);
    Nst_dec_ref(current_iter);
    return val;
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

Nst_FUNC_SIGN(zip_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;

    Nst_Obj *ob1 = Nst_iter_get_val(objs[0]);
    if (ob1 == nullptr || ob1 == Nst_iend())
        return ob1;

    Nst_Obj *ob2 = Nst_iter_get_val(objs[1]);
    if (ob2 == nullptr || ob2 == Nst_iend()) {
        Nst_dec_ref(ob1);
        return ob2;
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

Nst_FUNC_SIGN(zipn_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;

    Nst_SeqObj *arr = SEQ(Nst_array_new((usize)AS_INT(objs[0])));
    for (usize i = 0, n = (usize)AS_INT(objs[0]); i < n; i++) {
        Nst_Obj *res = Nst_iter_get_val(objs[i + 1]);
        if (res == nullptr || res == Nst_iend()) {
            arr->len = i;
            Nst_dec_ref(arr);
            return res;
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

Nst_FUNC_SIGN(enumerate_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    i64 idx = AS_INT(objs[0]);
    Nst_IterObj *iter = ITER(objs[1]);

    Nst_Obj *res = Nst_iter_get_val(iter);
    if (res == nullptr || res == Nst_iend())
        return res;

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
    i64 idx = AS_INT(objs[0]);

    if (idx == -1)
        return Nst_iend_ref();

    Nst_MapNode node = MAP(objs[1])->nodes[idx];
    AS_INT(objs[0]) = Nst_map_get_next_idx((i32)idx, MAP(objs[1]));
    if (node.key == Nst_iend())
        Nst_RETURN_NULL;
    return Nst_inc_ref(node.key);
}

Nst_FUNC_SIGN(values_get_val)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **objs = SEQ(args[0])->objs;
    i64 idx = AS_INT(objs[0]);

    if (idx == -1)
        return Nst_iend_ref();

    Nst_MapNode node = MAP(objs[1])->nodes[idx];
    AS_INT(objs[0]) = Nst_map_get_next_idx((i32)idx, MAP(objs[1]));
    if (node.value == Nst_iend())
        Nst_RETURN_NULL;
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
    Nst_SeqObj *seq = SEQ(objs[1]);
    i64 idx = AS_INT(objs[0]);

    if (idx <= -1)
        return Nst_iend_ref();

    Nst_Obj *res = Nst_seq_get(seq, idx);

    if (res == nullptr)
        return nullptr;

    i64 len = SEQ(objs[1])->len;
    AS_INT(objs[0]) -= 1;

    if (AS_INT(objs[0]) >= len)
        AS_INT(objs[0]) = len - 1;
    return res;
}
