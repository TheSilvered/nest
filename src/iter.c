#include <errno.h>
#include <stdlib.h>
#include "nest.h"

typedef struct _Nst_IterObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_Obj *start;
    Nst_Obj *next;
    Nst_Obj *value;
} Nst_IterObj;

#define ITER(ptr) ((Nst_IterObj *)(ptr))

typedef struct _IterRange {
    i64 count;
    i64 start, stop, step;
} _IterRange;

typedef struct _IterContainer {
    i64 idx;
    Nst_Obj *obj;
} _IterContainer;

static void destroy_iter_container(Nst_Obj *custom_obj)
{
    Nst_dec_ref(((_IterContainer *)Nst_obj_custom_data(custom_obj))->obj);
}

Nst_ObjRef *Nst_iter_new(Nst_ObjRef *start, Nst_ObjRef *next,
                         Nst_ObjRef *value)
{
    Nst_assert(start->type == Nst_t.Func);
    Nst_assert(next->type == Nst_t.Func);
    Nst_IterObj *iter = Nst_obj_alloc(Nst_IterObj, Nst_t.Iter);
    if (iter == NULL) {
        Nst_dec_ref(start);
        Nst_dec_ref(next);
        Nst_dec_ref(value);
        return NULL;
    }

    if (Nst_HAS_FLAG(start, Nst_FLAG_GGC_IS_SUPPORTED)
        || Nst_HAS_FLAG(next, Nst_FLAG_GGC_IS_SUPPORTED)
        || Nst_HAS_FLAG(value, Nst_FLAG_GGC_IS_SUPPORTED))
    {
        Nst_GGC_OBJ_INIT(iter);
    }

    iter->start = start;
    iter->next  = next;
    iter->value = value;

    return NstOBJ(iter);
}

void _Nst_iter_destroy(Nst_Obj *iter)
{
    Nst_assert(iter->type == Nst_t.Iter);

    Nst_dec_ref(ITER(iter)->start);
    Nst_dec_ref(ITER(iter)->next);
    Nst_dec_ref(ITER(iter)->value);
}

void _Nst_iter_traverse(Nst_Obj *iter)
{
    Nst_assert(iter->type == Nst_t.Iter);

    Nst_ggc_obj_reachable(ITER(iter)->start);
    Nst_ggc_obj_reachable(ITER(iter)->next);
    Nst_ggc_obj_reachable(ITER(iter)->value);
}

bool Nst_iter_start(Nst_Obj *iter)
{
    Nst_assert(iter->type == Nst_t.Iter);

    Nst_Obj *result = Nst_func_call(ITER(iter)->start, 1, &ITER(iter)->value);

    if (result == NULL)
        return false;

    Nst_dec_ref(result);
    return true;
}

Nst_Obj *Nst_iter_next(Nst_Obj *iter)
{
    Nst_assert(iter->type == Nst_t.Iter);

    return Nst_func_call(ITER(iter)->next, 1, &ITER(iter)->value);
}

Nst_Obj *Nst_iter_start_func(Nst_Obj *iter)
{
    Nst_assert(iter->type == Nst_t.Iter);
    return ITER(iter)->start;
}

Nst_Obj *Nst_iter_next_func(Nst_Obj *iter)
{
    Nst_assert(iter->type == Nst_t.Iter);
    return ITER(iter)->next;
}

Nst_Obj *Nst_iter_value(Nst_Obj *iter)
{
    Nst_assert(iter->type == Nst_t.Iter);
    return ITER(iter)->value;
}

Nst_ObjRef *Nst_iter_range_new(i64 start, i64 stop, i64 step)
{
    if (step == 0) {
        Nst_error_setc_value("the step cannot be zero");
        return NULL;
    }

    _IterRange data = {
        .count = 0,
        .start = start,
        .stop = stop,
        .step = step,
    };

    Nst_Obj *range_data = Nst_obj_custom(_IterRange, &data);
    if (range_data == NULL)
        return NULL;

    return Nst_iter_new(
        Nst_inc_ref(Nst_itf.range_start),
        Nst_inc_ref(Nst_itf.range_next),
        range_data);
}

Nst_ObjRef *Nst_iter_seq_new(Nst_Obj *seq)
{
    Nst_assert(seq->type == Nst_t.Array || seq->type == Nst_t.Vector);
    _IterContainer data = {
        .idx = 0,
        .obj = Nst_inc_ref(seq)
    };

    Nst_Obj *seq_data = Nst_obj_custom_ex(
        _IterContainer,
        &data,
        destroy_iter_container);
    if (seq_data == NULL)
        return NULL;

    return Nst_iter_new(
        Nst_inc_ref(Nst_itf.seq_start),
        Nst_inc_ref(Nst_itf.seq_next),
        seq_data);
}

Nst_ObjRef *Nst_iter_str_new(Nst_Obj *str)
{
    Nst_assert(str->type == Nst_t.Str);
    _IterContainer data = {
        .idx = 0,
        .obj = Nst_inc_ref(str)
    };

    Nst_Obj *str_data = Nst_obj_custom_ex(
        _IterContainer,
        &data,
        destroy_iter_container);
    if (str_data == NULL)
        return NULL;

    return Nst_iter_new(
        Nst_inc_ref(Nst_itf.str_start),
        Nst_inc_ref(Nst_itf.str_next),
        str_data);
}

Nst_ObjRef *Nst_iter_map_new(Nst_Obj *map)
{
    Nst_assert(map->type == Nst_t.Map);
    _IterContainer data = {
        .idx = 0,
        .obj = Nst_inc_ref(map)
    };

    Nst_Obj *map_data = Nst_obj_custom_ex(
        _IterContainer,
        &data,
        destroy_iter_container);
    if (map_data == NULL)
        return NULL;

    return Nst_iter_new(
        Nst_inc_ref(Nst_itf.map_start),
        Nst_inc_ref(Nst_itf.map_next),
        map_data);
}

Nst_Obj *NstC Nst_iter_range_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    _IterRange *data = Nst_obj_custom_data(args[0]);
    data->count = data->start;
    return Nst_null_ref();
}

Nst_Obj *NstC Nst_iter_range_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    _IterRange *data = Nst_obj_custom_data(args[0]);
    i64 count = data->count;
    i64 stop = data->stop;
    i64 step = data->step;

    if (step > 0 && count >= stop)
        return Nst_iend_ref();
    else if (step <= 0 && count <= stop)
        return Nst_iend_ref();

    Nst_Obj *ob = Nst_int_new(count);
    data->count += step;
    return ob;
}

Nst_Obj *NstC Nst_iter_seq_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    _IterContainer *data = Nst_obj_custom_data(args[0]);
    data->idx = 0;
    return Nst_null_ref();
}

Nst_Obj *NstC Nst_iter_seq_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    _IterContainer *data = Nst_obj_custom_data(args[0]);
    Nst_Obj *seq = data->obj;
    i64 idx = data->idx;

    if (idx >= (i64)Nst_seq_len(seq))
        return Nst_iend_ref();

    Nst_Obj *obj = Nst_seq_getn(seq, idx);
    data->idx += 1;

    if (obj == Nst_c.IEnd_iend)
        return Nst_null_ref();
    return Nst_inc_ref(obj);
}

Nst_Obj *NstC Nst_iter_str_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    _IterContainer *data = Nst_obj_custom_data(args[0]);
    data->idx = -1;
    return Nst_null_ref();
}

Nst_Obj *NstC Nst_iter_str_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    _IterContainer *data = Nst_obj_custom_data(args[0]);
    Nst_Obj *str = data->obj;
    Nst_Obj *ch = Nst_str_next_obj(str, (isize *)&data->idx);
    if (ch != NULL)
        return ch;
    if (data->idx == Nst_STR_LOOP_ERROR)
        return NULL;
    return Nst_iend_ref();
}

Nst_Obj *NstC Nst_iter_map_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    _IterContainer *data = Nst_obj_custom_data(args[0]);
    data->idx = -1;
    return Nst_null_ref();
}

Nst_Obj *NstC Nst_iter_map_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    _IterContainer *data = Nst_obj_custom_data(args[0]);
    isize idx = (isize)data->idx;

    Nst_Obj *key;
    Nst_Obj *value;
    idx = Nst_map_next(idx, data->obj, &key, &value);
    if (idx == -1)
        return Nst_iend_ref();

    Nst_Obj *arr = Nst_array_create_c("OO", key, value);

    data->idx = idx;
    return arr;
}
