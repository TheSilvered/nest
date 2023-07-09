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

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(map_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(insert_at_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(remove_at_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(slice_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(merge_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(sort_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(empty_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(filter_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(contains_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(any_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(all_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(count_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(lscan_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(rscan_, 4);

#if __LINE__ - FUNC_COUNT != 19
#error
#endif

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

Nst_FUNC_SIGN(map_)
{
    Nst_SeqObj *seq;
    Nst_FuncObj *func;

    Nst_DEF_EXTRACT("Af", &seq, &func);

    if ( func->arg_num != 1 )
    {
        Nst_set_value_error_c("the function must take exactly one argument");
        return nullptr;
    }

    Nst_SeqObj *new_seq = seq->type == Nst_type()->Array
        ? SEQ(Nst_array_new(seq->len))
        : SEQ(Nst_vector_new(seq->len));

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *arg = Nst_seq_get(seq, i);
        Nst_Obj *res = Nst_call_func(func, &arg);

        if ( res == nullptr )
        {
            for ( usize j = 0; j < i; j++ )
            {
                Nst_dec_ref(new_seq->objs[j]);
            }
            Nst_free(new_seq->objs);
            Nst_free(new_seq);
            return nullptr;
        }

        new_seq->objs[i] = res;
        Nst_dec_ref(arg);
    }

    return OBJ(new_seq);
}

Nst_FUNC_SIGN(insert_at_)
{
    Nst_SeqObj *vect;
    Nst_Int idx;
    Nst_Obj *obj;

    Nst_DEF_EXTRACT("vio", &vect, &idx, &obj);

    Nst_Int new_idx = idx;

    if ( idx < 0 )
    {
        new_idx = vect->len + idx;
    }

    if ( new_idx < 0 || new_idx >= (Nst_Int)vect->len )
    {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            idx,
            vect->len));

        return nullptr;
    }

    // Force the vector to grow
    Nst_vector_append(vect, Nst_null());
    Nst_dec_ref(Nst_null());

    for ( Nst_Int i = vect->len - 1; i >= new_idx; i-- )
    {
        vect->objs[i + 1] = vect->objs[i];
    }
    vect->objs[new_idx] = Nst_inc_ref(obj);

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(remove_at_)
{
    Nst_SeqObj *vect;
    Nst_Int idx;

    Nst_DEF_EXTRACT("vi", &vect, &idx);

    Nst_Int new_idx = idx;

    if ( idx < 0 )
    {
        new_idx = vect->len + idx;
    }

    if ( new_idx < 0 || new_idx >= (Nst_Int)vect->len )
    {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector"),
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

    _Nst_vector_resize(vect);
    return obj;
}

Nst_FUNC_SIGN(slice_)
{
    Nst_SeqObj *seq;
    Nst_Obj *start_obj;
    Nst_Obj *stop_obj;
    Nst_Obj *step_obj;

    Nst_DEF_EXTRACT("S?i?i?i", &seq, &start_obj, &stop_obj, &step_obj);

    Nst_Int step  = Nst_DEF_VAL(step_obj,  AS_INT(step_obj), 1);
    Nst_Int start = Nst_DEF_VAL(start_obj, AS_INT(start_obj), step > 0 ? 0 : seq->len);
    Nst_Int stop  = Nst_DEF_VAL(stop_obj,  AS_INT(stop_obj), step > 0 ? seq->len : -1);

    usize seq_len = seq->len;
    Nst_TypeObj *seq_type = args[0]->type;

    if ( step == 0 )
    {
        Nst_set_value_error_c("the step cannot be zero");
        return nullptr;
    }

    if ( start < 0 )
    {
        start += seq_len;
    }

    if ( stop < 0 && stop_obj != Nst_null() )
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

    if ( stop < 0 && stop_obj != Nst_null() )
    {
        stop = 0;
    }
    else if ( stop > (Nst_Int)seq_len )
    {
        stop = seq_len;
    }

    isize new_size = (isize)((stop - start) / step);

    if ( (stop - start) % step != 0 )
    {
        new_size++;
    }

    if ( new_size <= 0 )
    {
        Nst_dec_ref(seq);
        if ( seq_type == Nst_type()->Str )
        {
            return Nst_string_new((i8 *)"", 0, false);
        }
        else if ( seq_type == Nst_type()->Array )
        {
            return Nst_array_new(0);
        }
        else
        {
            return Nst_vector_new(0);
        }
    }

    if ( seq_type == Nst_type()->Array || seq_type == Nst_type()->Vector )
    {
        Nst_Obj *new_seq =
            seq_type == Nst_type()->Array ? Nst_array_new(new_size)
                                          : Nst_vector_new(new_size);

        for ( isize i = 0; i < new_size; i++ )
        {
            Nst_seq_set(new_seq, i, seq->objs[i * step + start]);
        }

        Nst_dec_ref(seq);
        return new_seq;
    }
    else
    {
        i8 *buf = Nst_malloc_c(new_size + 1, i8);
        if ( buf == nullptr )
        {
            return nullptr;
        }

        for ( isize i = 0; i < new_size; i++ )
        {
            buf[i] = STR(seq->objs[i * step + start])->value[0];
        }
        buf[new_size] = 0;
        Nst_dec_ref(seq);
        return Nst_string_new(buf, new_size, true);
    }
}

Nst_FUNC_SIGN(merge_)
{
    Nst_SeqObj *seq1;
    Nst_SeqObj *seq2;

    Nst_DEF_EXTRACT("AA", &seq1, &seq2);

    Nst_SeqObj *new_seq;

    if ( seq1->type == Nst_type()->Vector || seq2->type == Nst_type()->Vector )
    {
        new_seq = SEQ(Nst_vector_new(seq1->len + seq2->len));
    }
    else
    {
        new_seq = SEQ(Nst_array_new(seq1->len + seq2->len));
    }

    Nst_Int i = 0;

    for ( Nst_Int n = (Nst_Int)seq1->len; i < n; i++ )
    {
        Nst_seq_set(new_seq, i, seq1->objs[i]);
    }

    for ( Nst_Int j = i, n = (Nst_Int)seq2->len; j - i < n; j++ )
    {
        Nst_seq_set(new_seq, j, seq2->objs[j - i]);
    }

    return OBJ(new_seq);
}

bool insertion_sort(Nst_SeqObj *seq,
                    Nst_Int     left,
                    Nst_Int     right)
{
    for ( Nst_Int i = left + 1; i <= right; i++ )
    {
        Nst_Obj *temp = seq->objs[i];
        Nst_Int j = i - 1;
        while ( j >= left && Nst_obj_gt(seq->objs[j], temp) == Nst_true())
        {
            Nst_dec_ref(Nst_true());
            seq->objs[j + 1] = seq->objs[j];
            j--;
        }

        if ( Nst_error_occurred() )
        {
            return false;
        }

        if ( j >= left )
        {
            Nst_dec_ref(Nst_false());
        }

        seq->objs[j + 1] = temp;
    }

    return true;
}

void merge(Nst_SeqObj *seq, usize l, usize m, usize r)
{
    usize len1 = m - l + 1;
    usize len2 = r - m;

    Nst_Obj **left  = Nst_malloc_c(len1, Nst_Obj *);
    Nst_Obj **right = Nst_malloc_c(len2, Nst_Obj *);

    if ( left == nullptr || right == nullptr )
    {
        Nst_free(left);
        Nst_free(right);
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
        // all objects have already been through Nst_obj_gt, no errors can occur
        if ( Nst_obj_le(left[i], right[j]) == Nst_true() )
        {
            seq->objs[k] = left[i];
            i++;
            Nst_dec_ref(Nst_true());
        }
        else
        {
            seq->objs[k] = right[j];
            j++;
            Nst_dec_ref(Nst_false());
        }
        k++;
    }

    while ( i < len1 )
        seq->objs[k++] = left[i++];

    while ( j < len2 )
        seq->objs[k++] = right[j++];

    Nst_free(left);
    Nst_free(right);
}

Nst_FUNC_SIGN(sort_)
{
    // Implementation of Timsort, adapted from
    // https://www.geeksforgeeks.org/timsort/
    Nst_SeqObj *seq;

    Nst_DEF_EXTRACT("A", &seq);

    usize seq_len = seq->len;

    for ( usize i = 0; i < seq_len; i += RUN )
    {
        if ( !insertion_sort(seq, i, MIN((i + RUN - 1), (seq_len - 1))) )
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
                merge(seq, left, mid, right);
            }

            if ( Nst_error_occurred() )
            {
                return nullptr;
            }
        }
    }

    return Nst_inc_ref(seq);
}

Nst_FUNC_SIGN(empty_)
{
    Nst_SeqObj *vect;

    Nst_DEF_EXTRACT("v", &vect);

    for ( usize i = 0, n = vect->len; i < n; i++ )
    {
        Nst_dec_ref(vect->objs[i]);
    }

    vect->len = 0;

    return Nst_inc_ref(vect);
}

Nst_FUNC_SIGN(filter_)
{
    Nst_SeqObj *seq;
    Nst_FuncObj *func;

    Nst_DEF_EXTRACT("Af", &seq, &func);

    if ( func->arg_num != 1 )
    {
        Nst_set_call_error_c("the function must take exactly one argument");
        return nullptr;
    }

    Nst_SeqObj *new_seq = SEQ(Nst_vector_new(0));

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *res = Nst_call_func(func, &seq->objs[i]);

        if ( res == nullptr )
        {
            return nullptr;
        }

        if ( Nst_obj_cast(res, Nst_type()->Bool) == Nst_true() )
        {
            Nst_vector_append(new_seq, seq->objs[i]);
            Nst_dec_ref(Nst_true());
        }
        else
        {
            Nst_dec_ref(Nst_false());
        }

        Nst_dec_ref(res);
    }

    if ( seq->type == Nst_type()->Array )
    {
        Nst_Obj **new_objs = Nst_realloc_c(
            new_seq->objs,
            new_seq->len,
            Nst_Obj *,
            new_seq->size);
        if ( new_objs != new_seq->objs )
        {
            new_seq->size = new_seq->len;
        }
        new_seq->objs = new_objs;

        Nst_dec_ref(new_seq->type);
        new_seq->type = TYPE(Nst_inc_ref(Nst_type()->Array));
    }

    return OBJ(new_seq);
}

Nst_FUNC_SIGN(contains_)
{
    Nst_Obj *container;
    Nst_Obj *object;

    Nst_DEF_EXTRACT("a|v|m|so", &container, &object);

    if ( container->type == Nst_type()->Array || container->type == Nst_type()->Vector )
    {
        Nst_SeqObj *seq = SEQ(container);

        for ( usize i = 0, n = seq->len; i < n; i++ )
        {
            if ( Nst_obj_eq(seq->objs[i], object) == Nst_true() )
            {
                return Nst_true();
            }
            else
            {
                Nst_dec_ref(Nst_false());
            }
        }

        Nst_RETURN_FALSE;
    }
    else if ( container->type == Nst_type()->Map )
    {
        Nst_MapObj *map = MAP(container);

        if ( Nst_obj_hash(object) == -1 )
        {
            Nst_RETURN_FALSE;
        }

        Nst_Obj *item = Nst_map_get(map, object);
        if ( item == nullptr )
        {
            Nst_RETURN_FALSE;
        }
        else
        {
            Nst_dec_ref(item);
            Nst_RETURN_TRUE;
        }
    }
    else
    {
        if ( object->type != Nst_type()->Str )
        {
            Nst_RETURN_FALSE;
        }

        i8 *res = Nst_string_find(
            STR(container)->value, STR(container)->len,
            STR(object)->value, STR(object)->len);
        Nst_RETURN_COND(res != nullptr);
    }
}

Nst_FUNC_SIGN(any_)
{
    Nst_SeqObj *seq;

    Nst_DEF_EXTRACT("A", &seq);

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *bool_obj = Nst_obj_cast(seq->objs[i], Nst_type()->Bool);

        if ( bool_obj == Nst_true() )
        {
            return Nst_true();
        }

        Nst_dec_ref(bool_obj);
    }

    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(all_)
{
    Nst_SeqObj *seq;

    Nst_DEF_EXTRACT("A", &seq);

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *bool_obj = Nst_obj_cast(seq->objs[i], Nst_type()->Bool);

        if ( bool_obj == Nst_false() )
        {
            return Nst_false();
        }

        Nst_dec_ref(bool_obj);
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(count_)
{
    Nst_Obj *container;
    Nst_Obj *obj;

    Nst_DEF_EXTRACT("a|v|so", &container, &obj);
    usize count = 0;

    if ( container->type == Nst_type()->Array || container->type == Nst_type()->Vector )
    {
        Nst_SeqObj *seq = SEQ(container);

        for ( usize i = 0, n = seq->len; i < n; i++ )
        {
            if ( Nst_obj_eq(seq->objs[i], obj) == Nst_true() )
            {
                Nst_dec_ref(Nst_true());
                count++;
            }
            else
            {
                Nst_dec_ref(Nst_false());
            }
        }

        return Nst_int_new(count);
    }
    else
    {
        if ( obj->type != Nst_type()->Str || STR(obj)->len == 0 )
        {
            Nst_RETURN_ZERO;
        }

        i8 *str = STR(container)->value;
        usize str_len = STR(container)->len;
        i8 *sub = STR(obj)->value;
        usize sub_len = STR(obj)->len;

        while ( true )
        {
            i8 *res = Nst_string_find(str, str_len, sub, sub_len);
            if ( res == nullptr )
            {
                break;
            }
            str_len -= res - str + sub_len;
            str = res + sub_len;
            count++;
        }
        return Nst_int_new(count);
    }
}

Nst_FUNC_SIGN(lscan_)
{
    Nst_SeqObj *seq;
    Nst_FuncObj *func;
    Nst_Obj *prev_val;
    Nst_Obj *max_items_obj;

    Nst_DEF_EXTRACT("Sfo?i", &seq, &func, &prev_val, &max_items_obj);
    Nst_Int max_items = Nst_DEF_VAL(max_items_obj, AS_INT(max_items_obj), seq->len + 1);

    if ( func->arg_num != 2 )
    {
        Nst_set_value_error_c("the function must take exactly two argument");
        Nst_dec_ref(seq);
        return nullptr;
    }

    if ( max_items < 0 )
    {
        Nst_set_value_error_c("the maximum item count must be greater than or equal to zero");
        Nst_dec_ref(seq);
        return nullptr;
    }

    if ( max_items > (Nst_Int)seq->len + 1 )
    {
        max_items = seq->len + 1;
    }

    Nst_SeqObj *new_seq = seq->type == Nst_type()->Array
        ? SEQ(Nst_array_new((usize)max_items))
        : SEQ(Nst_vector_new((usize)max_items));
    if ( max_items == 0 )
    {
        Nst_dec_ref(seq);
        return OBJ(new_seq);
    }

    Nst_inc_ref(prev_val);
    Nst_seq_set(new_seq, 0, prev_val);

    Nst_Obj *func_args[2];

    for ( Nst_Int i = 1; i < max_items; i++ )
    {
        func_args[0] = prev_val;
        func_args[1] = seq->objs[i - 1];
        Nst_Obj *new_val = Nst_call_func(func, func_args);
        if ( new_val == nullptr )
        {
            Nst_dec_ref(prev_val);
            new_seq->len = (usize)i;
            Nst_dec_ref(new_seq);
            Nst_dec_ref(seq);
            return nullptr;
        }
        Nst_seq_set(new_seq, i, new_val);
        Nst_dec_ref(prev_val);
        prev_val = new_val;
    }
    Nst_dec_ref(prev_val);
    Nst_dec_ref(seq);
    return OBJ(new_seq);
}

Nst_FUNC_SIGN(rscan_)
{
    Nst_SeqObj *seq;
    Nst_FuncObj *func;
    Nst_Obj *prev_val;
    Nst_Obj *max_items_obj;

    Nst_DEF_EXTRACT("Sfo?i", &seq, &func, &prev_val, &max_items_obj);
    Nst_Int max_items = Nst_DEF_VAL(max_items_obj, AS_INT(max_items_obj), seq->len + 1);

    if ( func->arg_num != 2 )
    {
        Nst_set_value_error_c("the function must take exactly two argument");
        Nst_dec_ref(seq);
        return nullptr;
    }

    if ( max_items < 0 )
    {
        Nst_set_value_error_c("the maximum item count must be greater than or equal to zero");
        Nst_dec_ref(seq);
        return nullptr;
    }

    Nst_Int seq_len = (Nst_Int)seq->len;

    if ( max_items > seq_len + 1 )
    {
        max_items = seq_len + 1;
    }

    Nst_SeqObj *new_seq = seq->type == Nst_type()->Array
        ? SEQ(Nst_array_new((usize)max_items))
        : SEQ(Nst_vector_new((usize)max_items));
    if ( max_items == 0 )
    {
        Nst_dec_ref(seq);
        return OBJ(new_seq);
    }

    Nst_inc_ref(prev_val);
    Nst_seq_set(new_seq, max_items - 1, prev_val);

    Nst_Obj *func_args[2];

    for ( Nst_Int i = max_items - 2; i >= 0; i-- )
    {
        func_args[0] = seq->objs[i + seq_len - max_items + 1];
        func_args[1] = prev_val;
        Nst_Obj *new_val = Nst_call_func(func, func_args);
        if ( new_val == nullptr )
        {
            Nst_dec_ref(prev_val);
            for ( Nst_Int j = max_items - 1; j > i; j-- )
            {
                Nst_dec_ref(new_seq->objs[max_items - j - 1]);
            }
            new_seq->len = 0;
            Nst_dec_ref(new_seq);
            Nst_dec_ref(seq);
            return nullptr;
        }
        Nst_seq_set(new_seq, i, new_val);
        Nst_dec_ref(prev_val);
        prev_val = new_val;
    }
    Nst_dec_ref(prev_val);
    Nst_dec_ref(seq);
    return OBJ(new_seq);
}
