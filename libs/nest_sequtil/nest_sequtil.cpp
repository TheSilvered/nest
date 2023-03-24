#include <cmath>
#include "nest_sequtil.h"

#define FUNC_COUNT 14
#define RUN 32

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;
    Nst_OpErr err = { nullptr, nullptr };

    func_list_[idx++] = NST_MAKE_FUNCDECLR(map_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(insert_at_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(remove_at_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(slice_, 4);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(merge_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(sort_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(empty_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(filter_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(contains_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(any_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(all_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(count_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(lscan_, 4);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rscan_, 4);

#if __LINE__ - FUNC_COUNT != 20
#error
#endif

    lib_init_ = err.name == nullptr;
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

NST_FUNC_SIGN(map_)
{
    Nst_SeqObj *seq;
    Nst_FuncObj *func;

    NST_DEF_EXTRACT("Af", &seq, &func);

    if ( func->arg_num != 1 )
    {
        NST_SET_RAW_VALUE_ERROR("the function must take exactly one argument");
        return nullptr;
    }

    Nst_SeqObj *new_seq = seq->type == nst_type()->Array
        ? SEQ(nst_array_new(seq->len, err))
        : SEQ(nst_vector_new(seq->len, err));

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *arg = nst_seq_get(seq, i);
        Nst_Obj *res = nst_call_func(func, &arg, err);

        if ( res == nullptr )
        {
            for ( usize j = 0; j < i; j++ )
            {
                nst_dec_ref(new_seq->objs[j]);
            }
            nst_free(new_seq->objs);
            nst_free(new_seq);
            return nullptr;
        }

        new_seq->objs[i] = res;
        nst_dec_ref(arg);
    }

    return OBJ(new_seq);
}

NST_FUNC_SIGN(insert_at_)
{
    Nst_SeqObj *vect;
    Nst_Int idx;
    Nst_Obj *obj;

    NST_DEF_EXTRACT("vio", &vect, &idx, &obj);

    Nst_Int new_idx = idx;

    if ( idx < 0 )
    {
        new_idx = vect->len + idx;
    }

    if ( new_idx < 0 || new_idx >= (Nst_Int)vect->len )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            "iu",
            idx,
            vect->len));

        return nullptr;
    }

    // Force the vector to grow
    nst_vector_append(vect, nst_null(), err);
    nst_dec_ref(nst_null());

    for ( Nst_Int i = vect->len - 1; i >= new_idx; i-- )
    {
        vect->objs[i + 1] = vect->objs[i];
    }
    vect->objs[new_idx] = nst_inc_ref(obj);

    NST_RETURN_NULL;
}

NST_FUNC_SIGN(remove_at_)
{
    Nst_SeqObj *vect;
    Nst_Int idx;

    NST_DEF_EXTRACT("vi", &vect, &idx);

    Nst_Int new_idx = idx;

    if ( idx < 0 )
    {
        new_idx = vect->len + idx;
    }

    if ( new_idx < 0 || new_idx >= (Nst_Int)vect->len )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            _NST_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            "iu",
            idx,
            vect->len));

        return nullptr;
    }

    Nst_Obj *obj = vect->objs[new_idx];
    vect->len--;
    for ( ; new_idx < (Nst_Int)vect->len; new_idx++ )
    {
        vect->objs[new_idx] = vect->objs[new_idx + 1];
    }

    _nst_vector_resize(vect, err);
    return obj;
}

NST_FUNC_SIGN(slice_)
{
    Nst_SeqObj *seq;
    Nst_Obj *start_obj;
    Nst_Obj *stop_obj;
    Nst_Obj *step_obj;

    NST_DEF_EXTRACT("S?i?i?i", &seq, &start_obj, &stop_obj, &step_obj);

    Nst_Int step  = NST_DEF_VAL(step_obj,  AS_INT(step_obj), 1);
    Nst_Int start = NST_DEF_VAL(start_obj, AS_INT(start_obj), step > 0 ? 0 : seq->len);
    Nst_Int stop  = NST_DEF_VAL(stop_obj,  AS_INT(stop_obj), step > 0 ? seq->len : -1);

    usize seq_len = seq->len;
    Nst_TypeObj *seq_type = args[0]->type;

    if ( step == 0 )
    {
        NST_SET_RAW_VALUE_ERROR("the step cannot be zero");
        return nullptr;
    }

    if ( start < 0 )
    {
        start += seq_len;
    }

    if ( stop < 0 && stop_obj != nst_null() )
    {
        stop += seq_len;
    }

    if ( start < 0 )
    {
        start = 0;
    }
    else if ( start >= (Nst_Int)seq_len )
    {
        start = seq_len - 1;
    }

    if ( stop < 0 && stop_obj != nst_null() )
    {
        stop = 0;
    }
    else if ( stop > (Nst_Int)seq_len )
    {
        stop = seq_len;
    }

    usize new_size = (usize)((stop - start) / step);

    if ( (stop - start) % step != 0 )
    {
        new_size++;
    }

    if ( new_size <= 0 )
    {
        if ( seq_type == nst_type()->Str )
        {
            return nst_string_new((i8 *)"", 0, false, err);
        }
        else if ( seq_type == nst_type()->Array )
        {
            return nst_array_new(0, err);
        }
        else
        {
            return nst_vector_new(0, err);
        }
    }

    if ( seq_type == nst_type()->Array || seq_type == nst_type()->Vector )
    {
        Nst_Obj *new_seq =
            seq_type == nst_type()->Array ? nst_array_new(new_size, err)
                                          : nst_vector_new(new_size, err);

        for ( usize i = 0; i < new_size; i++ )
        {
            nst_seq_set(new_seq, i, seq->objs[i * step + start]);
        }

        nst_dec_ref(seq);
        return new_seq;
    }
    else
    {
        i8 *buf = (i8 *)nst_malloc(new_size + 1, sizeof(i8), err);
        if ( buf == nullptr )
        {
            return nullptr;
        }

        for ( usize i = 0; i < new_size; i++ )
        {
            buf[i] = STR(seq->objs[i * step + start])->value[0];
        }
        buf[new_size] = 0;
        nst_dec_ref(seq);
        return nst_string_new(buf, new_size, true, err);
    }
}

NST_FUNC_SIGN(merge_)
{
    Nst_SeqObj *seq1;
    Nst_SeqObj *seq2;

    NST_DEF_EXTRACT("AA", &seq1, &seq2);

    Nst_SeqObj *new_seq;

    if ( seq1->type == nst_type()->Vector || seq2->type == nst_type()->Vector )
    {
        new_seq = SEQ(nst_vector_new(seq1->len + seq2->len, err));
    }
    else
    {
        new_seq = SEQ(nst_array_new(seq1->len + seq2->len, err));
    }

    Nst_Int i = 0;

    for ( Nst_Int n = (Nst_Int)seq1->len; i < n; i++ )
    {
        nst_seq_set(new_seq, i, seq1->objs[i]);
    }

    for ( Nst_Int j = i, n = (Nst_Int)seq2->len; j - i < n; j++ )
    {
        nst_seq_set(new_seq, j, seq2->objs[j - i]);
    }

    return OBJ(new_seq);
}

bool insertion_sort(Nst_SeqObj *seq,
                    Nst_Int     left,
                    Nst_Int     right,
                    Nst_OpErr  *err)
{
    for ( Nst_Int i = left + 1; i <= right; i++ )
    {
        Nst_Obj *temp = seq->objs[i];
        Nst_Int j = i - 1;
        while ( j >= left && nst_obj_gt(seq->objs[j], temp, err) == nst_true())
        {
            nst_dec_ref(nst_true());
            seq->objs[j + 1] = seq->objs[j];
            j--;
        }

        if ( err->message != nullptr )
        {
            return false;
        }

        if ( j >= left )
        {
            nst_dec_ref(nst_false());
        }

        seq->objs[j + 1] = temp;
    }

    return true;
}

void merge(Nst_SeqObj *seq, usize l, usize m, usize r, Nst_OpErr *err)
{
    usize len1 = m - l + 1;
    usize len2 = r - m;

    Nst_Obj **left  = (Nst_Obj **)nst_malloc(len1, sizeof(Nst_Obj *), err);
    Nst_Obj **right = (Nst_Obj **)nst_malloc(len2, sizeof(Nst_Obj *), err);

    if ( left == nullptr || right == nullptr )
    {
        nst_free(left);
        nst_free(right);
        return;
    }

    for ( usize i = 0; i < len1; i++ )
    {
        left[i] = seq->objs[l + i];
    }
    for ( usize i = 0; i < len2; i++ )
    {
        right[i] = seq->objs[m + 1 + i];
    }

    usize i = 0;
    usize j = 0;
    usize k = l;

    while ( i < len1 && j < len2 )
    {
        // all objects have already been through nst_obj_gt, no errors can occur
        if ( nst_obj_le(left[i], right[j], nullptr) == nst_true() )
        {
            seq->objs[k] = left[i];
            i++;
            nst_dec_ref(nst_true());
        }
        else
        {
            seq->objs[k] = right[j];
            j++;
            nst_dec_ref(nst_false());
        }
        k++;
    }

    while ( i < len1 )
        seq->objs[k++] = left[i++];

    while ( j < len2 )
        seq->objs[k++] = right[j++];

    nst_free(left);
    nst_free(right);
}

NST_FUNC_SIGN(sort_)
{
    // Implementation of Timsort, adapted from
    // https://www.geeksforgeeks.org/timsort/
    Nst_SeqObj *seq;

    NST_DEF_EXTRACT("A", &seq);

    usize seq_len = seq->len;

    for ( usize i = 0; i < seq_len; i += RUN )
    {
        if ( !insertion_sort(seq, i, MIN((i + RUN - 1), (seq_len - 1)), err) )
        {
            return nullptr;
        }
    }

    for ( usize size = RUN; size < seq_len; size = 2 * size )
    {
        for ( usize left = 0; left < seq_len; left += 2 * size )
        {
            usize mid = left + size - 1;
            usize right = MIN((left + 2 * size - 1), (seq_len - 1));

            if ( mid < right )
            {
                merge(seq, left, mid, right, err);
            }

            if ( err->name != nullptr )
            {
                return nullptr;
            }
        }
    }

    return nst_inc_ref(seq);
}

NST_FUNC_SIGN(empty_)
{
    Nst_SeqObj *vect;

    NST_DEF_EXTRACT("v", &vect);

    for ( usize i = 0, n = vect->len; i < n; i++ )
    {
        nst_dec_ref(vect->objs[i]);
    }

    vect->len = 0;

    return nst_inc_ref(vect);
}

NST_FUNC_SIGN(filter_)
{
    Nst_SeqObj *seq;
    Nst_FuncObj *func;

    NST_DEF_EXTRACT("Af", &seq, &func);

    if ( func->arg_num != 1 )
    {
        NST_SET_RAW_CALL_ERROR("the function must take exactly one argument");
        return nullptr;
    }

    Nst_SeqObj *new_seq = SEQ(nst_vector_new(0, err));

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *res = nst_call_func(func, &seq->objs[i], err);

        if ( res == nullptr )
        {
            return nullptr;
        }

        if ( nst_obj_cast(res, nst_type()->Bool, nullptr) == nst_true() )
        {
            nst_vector_append(new_seq, seq->objs[i], err);
            nst_dec_ref(nst_true());
        }
        else
        {
            nst_dec_ref(nst_false());
        }

        nst_dec_ref(res);
    }

    if ( seq->type == nst_type()->Array )
    {
        Nst_Obj **new_objs = (Nst_Obj **)nst_realloc(
            new_seq->objs,
            new_seq->len,
            sizeof(Nst_Obj *),
            new_seq->size, err);
        if ( new_objs != new_seq->objs )
        {
            new_seq->size = new_seq->len;
        }
        new_seq->objs = new_objs;

        nst_dec_ref(new_seq->type);
        new_seq->type = TYPE(nst_inc_ref(nst_type()->Array));
    }

    return OBJ(new_seq);
}

NST_FUNC_SIGN(contains_)
{
    Nst_Obj *container;
    Nst_Obj *object;

    NST_DEF_EXTRACT("oo", &container, &object);

    if ( container->type == nst_type()->Array || container->type == nst_type()->Vector )
    {
        Nst_SeqObj *seq = SEQ(container);

        for ( usize i = 0, n = seq->len; i < n; i++ )
        {
            if ( nst_obj_eq(seq->objs[i], object, nullptr) == nst_true() )
            {
                return nst_true();
            }
            else
            {
                nst_dec_ref(nst_false());
            }
        }

        NST_RETURN_FALSE;
    }
    else if ( container->type == nst_type()->Map )
    {
        Nst_MapObj *map = MAP(container);

        if ( nst_obj_hash(object) == -1 )
        {
            NST_RETURN_FALSE;
        }

        Nst_Obj *item = nst_map_get(map, object);
        if ( item == nullptr )
        {
            NST_RETURN_FALSE;
        }
        else
        {
            nst_dec_ref(item);
            NST_RETURN_TRUE;
        }
    }
    else if ( container->type == nst_type()->Str && object->type == nst_type()->Str )
    {
        i8 *res = nst_string_find(
            STR(container)->value, STR(container)->len,
            STR(object)->value, STR(object)->len);
        NST_RETURN_COND(res != nullptr);
    }
    else
    {
        NST_SET_TYPE_ERROR(nst_format_error(
            _NST_EM_WRONG_TYPE_FOR_ARG("Array', 'Vector', 'Map' or 'Str"),
            "us",
            1, TYPE_NAME(container)));
        return nullptr;
    }
}

NST_FUNC_SIGN(any_)
{
    Nst_SeqObj *seq;

    NST_DEF_EXTRACT("A", &seq);

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *bool_obj = nst_obj_cast(seq->objs[i], nst_type()->Bool, nullptr);

        if ( bool_obj == nst_true() )
        {
            return nst_true();
        }

        nst_dec_ref(bool_obj);
    }

    NST_RETURN_FALSE;
}

NST_FUNC_SIGN(all_)
{
    Nst_SeqObj *seq;

    NST_DEF_EXTRACT("A", &seq);

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *bool_obj = nst_obj_cast(seq->objs[i], nst_type()->Bool, nullptr);

        if ( bool_obj == nst_false() )
        {
            return nst_false();
        }

        nst_dec_ref(bool_obj);
    }

    NST_RETURN_TRUE;
}

NST_FUNC_SIGN(count_)
{
    Nst_SeqObj *seq;
    Nst_Obj *obj;

    NST_DEF_EXTRACT("So", &seq, &obj);
    usize count = 0;

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *res = nst_obj_eq(obj, seq->objs[i], nullptr);
        if ( res == nst_true() )
        {
            count++;
        }
        nst_dec_ref(res);
    }
    nst_dec_ref(seq);
    return nst_int_new(count, err);
}

NST_FUNC_SIGN(lscan_)
{
    Nst_SeqObj *seq;
    Nst_FuncObj *func;
    Nst_Obj *prev_val;
    Nst_Obj *max_items_obj;

    NST_DEF_EXTRACT("Sfo?i", &seq, &func, &prev_val, &max_items_obj);
    Nst_Int max_items = NST_DEF_VAL(max_items_obj, AS_INT(max_items_obj), seq->len + 1);

    if ( func->arg_num != 2 )
    {
        NST_SET_RAW_VALUE_ERROR("the function must take exactly two argument");
        return nullptr;
    }

    if ( max_items < 0 )
    {
        NST_SET_RAW_VALUE_ERROR("the maximum item count must be greater than or equal to zero");
        return nullptr;
    }

    if ( max_items > (Nst_Int)seq->len + 1 )
    {
        max_items = seq->len + 1;
    }

    Nst_SeqObj *new_seq = seq->type == nst_type()->Array ?
        SEQ(nst_array_new(max_items, err)) : SEQ(nst_vector_new(max_items, err));
    if ( max_items == 0 )
    {
        return OBJ(new_seq);
    }

    nst_inc_ref(prev_val);
    nst_seq_set(new_seq, 0, prev_val);

    Nst_Obj *func_args[2];

    for ( Nst_Int i = 1; i < max_items; i++ )
    {
        func_args[0] = prev_val;
        func_args[1] = seq->objs[i - 1];
        Nst_Obj *new_val = nst_call_func(func, func_args, err);
        if ( new_val == nullptr )
        {
            nst_dec_ref(prev_val);
            new_seq->len = i;
            nst_dec_ref(new_seq);
            return nullptr;
        }
        nst_seq_set(new_seq, i, new_val);
        nst_dec_ref(prev_val);
        prev_val = new_val;
    }
    nst_dec_ref(prev_val);

    return OBJ(new_seq);
}

NST_FUNC_SIGN(rscan_)
{
    Nst_SeqObj *seq;
    Nst_FuncObj *func;
    Nst_Obj *prev_val;
    Nst_Obj *max_items_obj;

    NST_DEF_EXTRACT("Sfo?i", &seq, &func, &prev_val, &max_items_obj);
    Nst_Int max_items = NST_DEF_VAL(max_items_obj, AS_INT(max_items_obj), seq->len + 1);

    if ( func->arg_num != 2 )
    {
        NST_SET_RAW_VALUE_ERROR("the function must take exactly two argument");
        return nullptr;
    }

    if ( max_items < 0 )
    {
        NST_SET_RAW_VALUE_ERROR("the maximum item count must be greater than or equal to zero");
        return nullptr;
    }

    if ( max_items > (Nst_Int)seq->len + 1 )
    {
        max_items = seq->len + 1;
    }

    Nst_SeqObj *new_seq = seq->type == nst_type()->Array ?
        SEQ(nst_array_new(max_items, err)) : SEQ(nst_vector_new(max_items, err));
    if ( max_items == 0 )
    {
        return OBJ(new_seq);
    }

    nst_inc_ref(prev_val);
    nst_seq_set(new_seq, max_items - 1, prev_val);

    Nst_Obj *func_args[2];

    for ( Nst_Int i = max_items - 2; i >= 0; i-- )
    {
        func_args[0] = prev_val;
        func_args[1] = seq->objs[i];
        Nst_Obj *new_val = nst_call_func(func, func_args, err);
        if ( new_val == nullptr )
        {
            nst_dec_ref(prev_val);
            for ( Nst_Int j = max_items - 1; j > i; j-- )
            {
                nst_dec_ref(new_seq->objs[max_items - j - 1]);
            }
            new_seq->len = 0;
            nst_dec_ref(new_seq);
            return nullptr;
        }
        nst_seq_set(new_seq, i, new_val);
        nst_dec_ref(prev_val);
        prev_val = new_val;
    }
    nst_dec_ref(prev_val);

    return OBJ(new_seq);
}
