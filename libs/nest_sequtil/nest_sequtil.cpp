#include <cmath>
#include "nest_sequtil.h"

#define FUNC_COUNT 12
#define RUN 32

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_func_list_new(FUNC_COUNT)) == nullptr )
    {
        return false;
    }

    usize idx = 0;

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

#if __LINE__ - FUNC_COUNT != 23
#error FUNC_COUNT does not match the number of lines
#endif

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
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

    Nst_SeqObj *new_seq = seq->type == nst_t.Array
        ? SEQ(nst_array_new(seq->len))
        : SEQ(nst_vector_new(seq->len));

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
            free(new_seq->objs);
            free(new_seq);
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
    nst_vector_append(vect, nst_c.Null_null);
    nst_dec_ref(nst_c.Null_null);

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

    nst_vector_resize(vect);
    return obj;
}

NST_FUNC_SIGN(slice_)
{
    Nst_SeqObj *seq;
    Nst_Obj *start_obj;
    Nst_Obj *stop_obj;
    Nst_Obj *step_obj;

    NST_DEF_EXTRACT("S?i?i?i", &seq, &start_obj, &stop_obj, &step_obj);

    Nst_Int start, stop, step;

    NST_SET_DEF(step_obj, step, 1, AS_INT(step_obj));
    NST_SET_DEF(start_obj, start, step > 0 ? 0 : seq->len, AS_INT(start_obj));
    NST_SET_DEF(stop_obj, stop, step > 0 ? seq->len : -1, AS_INT(stop_obj));

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

    if ( stop < 0 && stop_obj != nst_c.Null_null )
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

    if ( stop < 0 && stop_obj != nst_c.Null_null )
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
        if ( seq_type == nst_t.Str )
        {
            return nst_string_new((i8 *)"", 0, false);
        }
        else if ( seq_type == nst_t.Array )
        {
            return nst_array_new(0);
        }
        else
        {
            return nst_vector_new(0);
        }
    }

    if ( seq_type == nst_t.Array || seq_type == nst_t.Vector )
    {
        Nst_Obj *new_seq = seq_type == nst_t.Array ? nst_array_new(new_size)
                                                   : nst_vector_new(new_size);

        for ( usize i = 0; i < new_size; i++ )
        {
            nst_seq_set(new_seq, i, seq->objs[i * step + start]);
        }

        nst_dec_ref(seq);
        return new_seq;
    }
    else
    {
        i8 *buf = new i8[new_size + 1];

        for ( usize i = 0; i < new_size; i++ )
        {
            buf[i] = STR(seq->objs[i * step + start])->value[0];
        }
        buf[new_size] = 0;
        nst_dec_ref(seq);
        return nst_string_new(buf, new_size, true);
    }
}

NST_FUNC_SIGN(merge_)
{
    Nst_SeqObj *seq1;
    Nst_SeqObj *seq2;

    NST_DEF_EXTRACT("AA", &seq1, &seq2);

    Nst_SeqObj *new_seq;

    if ( seq1->type == nst_t.Vector || seq2->type == nst_t.Vector )
    {
        new_seq = SEQ(nst_vector_new(seq1->len + seq2->len));
    }
    else
    {
        new_seq = SEQ(nst_array_new(seq1->len + seq2->len));
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
        while ( j >= left && nst_obj_gt(seq->objs[j], temp, err) == nst_c.Bool_true)
        {
            nst_dec_ref(nst_c.Bool_true);
            seq->objs[j + 1] = seq->objs[j];
            j--;
        }

        if ( err->message != nullptr )
        {
            return false;
        }

        if ( j >= left )
        {
            nst_dec_ref(nst_c.Bool_false);
        }

        seq->objs[j + 1] = temp;
    }

    return true;
}

// Merge function merges the sorted runs
void merge(Nst_SeqObj *seq, usize l, usize m, usize r)
{
    usize len1 = m - l + 1;
    usize len2 = r - m;

    Nst_Obj **left  = new Nst_Obj *[len1];
    Nst_Obj **right = new Nst_Obj *[len2];

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
        // all objects passed through nst_obj_gt, no errors can occur
        if ( nst_obj_le(left[i], right[j], nullptr) == nst_c.Bool_true )
        {
            seq->objs[k] = left[i];
            i++;
            nst_dec_ref(nst_c.Bool_true);
        }
        else
        {
            seq->objs[k] = right[j];
            j++;
            nst_dec_ref(nst_c.Bool_false);
        }
        k++;
    }

    while ( i < len1 )
        seq->objs[k++] = left[i++];

    while ( j < len2 )
        seq->objs[k++] = right[j++];

    delete[] left;
    delete[] right;
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
                merge(seq, left, mid, right);
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

    Nst_SeqObj *new_seq = SEQ(nst_vector_new(0));

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        Nst_Obj *res = nst_call_func(func, &seq->objs[i], err);

        if ( res == nullptr )
        {
            return nullptr;
        }

        if ( nst_obj_cast(res, nst_t.Bool, nullptr) == nst_c.Bool_true )
        {
            nst_vector_append(new_seq, seq->objs[i]);
            nst_dec_ref(nst_c.Bool_true);
        }
        else
        {
            nst_dec_ref(nst_c.Bool_false);
        }

        nst_dec_ref(res);
    }

    if ( seq->type == nst_t.Array )
    {
        Nst_Obj **new_objs = (Nst_Obj **)realloc(
            new_seq->objs,
            sizeof(Nst_Obj *) * new_seq->len);
        if ( new_objs != nullptr )
        {
            new_seq->objs = new_objs;
            new_seq->size = new_seq->len;
        }

        nst_dec_ref(new_seq->type);
        new_seq->type = TYPE(nst_inc_ref(nst_t.Array));
    }

    return OBJ(new_seq);
}

NST_FUNC_SIGN(contains_)
{
    Nst_Obj *container;
    Nst_Obj *object;

    NST_DEF_EXTRACT("oo", &container, &object);

    if ( container->type == nst_t.Array || container->type == nst_t.Vector )
    {
        Nst_SeqObj *seq = SEQ(container);

        for ( usize i = 0, n = seq->len; i < n; i++ )
        {
            if ( nst_obj_eq(seq->objs[i], object, nullptr) == nst_c.Bool_true )
            {
                return nst_c.Bool_true;
            }
            else
            {
                nst_dec_ref(nst_c.Bool_false);
            }
        }

        NST_RETURN_FALSE;
    }
    else if ( container->type == nst_t.Map )
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
    else if ( container->type == nst_t.Str && object->type == nst_t.Str )
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
        Nst_Obj *bool_obj = nst_obj_cast(seq->objs[i], nst_t.Bool, nullptr);

        if ( bool_obj == nst_c.Bool_true )
        {
            return nst_c.Bool_true;
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
        Nst_Obj *bool_obj = nst_obj_cast(seq->objs[i], nst_t.Bool, nullptr);

        if ( bool_obj == nst_c.Bool_false )
        {
            return nst_c.Bool_false;
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
        if ( res == nst_c.Bool_true )
        {
            count++;
        }
        nst_dec_ref(res);
    }
    nst_dec_ref(seq);
    return nst_int_new(count);
}
