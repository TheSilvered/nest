#include <errno.h>
#include <stdlib.h>
#include "iter.h"
#include "obj_ops.h"
#include "format.h"

typedef struct _Nst_IterObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_Obj *start;
    Nst_Obj *next;
    Nst_Obj *value;
} Nst_IterObj;

#define ITER(ptr) ((Nst_IterObj *)(ptr))

Nst_Obj *Nst_iter_new(Nst_Obj *start, Nst_Obj *next, Nst_Obj *value)
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

    return OBJ(iter);
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


Nst_Obj *NstC Nst_iter_range_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    AS_INT(c_args[0]) = AS_INT(c_args[1]);
    Nst_RETURN_NULL;
}

Nst_Obj *NstC Nst_iter_range_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    i64 idx = AS_INT(c_args[0]);
    i64 stop = AS_INT(c_args[2]);
    i64 step = AS_INT(c_args[3]);

    if (step > 0 && idx >= stop)
        Nst_RETURN_IEND;
    else if (step <= 0 && idx <= stop)
        Nst_RETURN_IEND;

    Nst_Obj *ob = Nst_int_new(idx);
    AS_INT(c_args[0]) += step;
    return ob;
}

Nst_Obj *NstC Nst_iter_seq_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    AS_INT(c_args[0]) = 0;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC Nst_iter_seq_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    Nst_Obj *seq = c_args[1];
    i64 idx = AS_INT(c_args[0]);

    if (idx >= (i64)Nst_seq_len(seq))
        Nst_RETURN_IEND;

    Nst_Obj *obj = Nst_seq_getn(seq, idx);
    AS_INT(c_args[0]) += 1;

    if (obj == Nst_c.IEnd_iend)
        Nst_RETURN_NULL;
    return Nst_inc_ref(obj);
}

Nst_Obj *NstC Nst_iter_str_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    AS_INT(c_args[0]) = -1;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC Nst_iter_str_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    Nst_Obj *str = c_args[1];
    Nst_Obj *ch = Nst_str_next_obj(str, (isize *)&AS_INT(c_args[0]));
    if (ch != NULL)
        return ch;
    if (AS_INT(c_args[0]) == Nst_STR_LOOP_ERROR)
        return NULL;
    Nst_RETURN_IEND;
}

Nst_Obj *NstC Nst_iter_map_start(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    AS_INT(c_args[0]) = -1;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC Nst_iter_map_next(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_Obj **c_args = _Nst_seq_objs(args[0]);
    isize idx = (isize)AS_INT(c_args[0]);

    Nst_Obj *key;
    Nst_Obj *value;
    idx = Nst_map_next(idx, c_args[1], &key, &value);
    if (idx == -1)
        Nst_RETURN_IEND;

    Nst_Obj *arr = Nst_array_create_c("OO", key, value);

    AS_INT(c_args[0]) = idx;
    return arr;
}
