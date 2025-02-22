#include <cmath>
#include <cstring>
#include "nest_sequtil.h"
#include "sequtil_i_functions.h"

#define SORT_RUN_SIZE 32

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(map_, 3),
    Nst_FUNCDECLR(map_i_, 2),
    Nst_FUNCDECLR(insert_at_, 3),
    Nst_FUNCDECLR(remove_at_, 2),
    Nst_FUNCDECLR(slice_, 4),
    Nst_FUNCDECLR(slice_i_, 4),
    Nst_FUNCDECLR(merge_, 2),
    Nst_FUNCDECLR(extend_, 2),
    Nst_FUNCDECLR(sort_, 3),
    Nst_FUNCDECLR(empty_, 1),
    Nst_FUNCDECLR(filter_, 2),
    Nst_FUNCDECLR(filter_i_, 2),
    Nst_FUNCDECLR(any_, 1),
    Nst_FUNCDECLR(all_, 1),
    Nst_FUNCDECLR(count_, 2),
    Nst_FUNCDECLR(lscan_, 4),
    Nst_FUNCDECLR(rscan_, 4),
    Nst_FUNCDECLR(copy_, 1),
    Nst_FUNCDECLR(deep_copy_, 1),
    Nst_FUNCDECLR(enum_, 2),
    Nst_FUNCDECLR(reverse_, 2),
    Nst_FUNCDECLR(reverse_i_, 1),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    return obj_list_;
}

Nst_Obj *NstC map_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *func;
    bool map_in_place;

    if (!Nst_extract_args("A f y", arg_num, args, &seq, &func, &map_in_place))
        return nullptr;

    if (Nst_func_arg_num(func) != 1) {
        Nst_set_value_error_c("the function must take exactly one argument");
        return nullptr;
    }

    usize seq_len = Nst_seq_len(seq);
    Nst_Obj *new_seq;

    if (map_in_place)
        new_seq = seq;
    else {
        new_seq = Nst_T(seq, Array)
            ? Nst_array_new(seq_len)
            : Nst_vector_new(seq_len);
    }

    for (usize i = 0; i < seq_len; i++) {
        Nst_Obj *arg = Nst_seq_get(seq, i);
        Nst_Obj *res = Nst_func_call(func, 1, &arg);
        Nst_dec_ref(arg);

        if (res == nullptr) {
            if (!map_in_place)
                Nst_dec_ref(new_seq);
            return nullptr;
        }

        Nst_seq_setnf(new_seq, i, res);
    }

    return map_in_place ? Nst_inc_ref(new_seq) : new_seq;
}

Nst_Obj *NstC map_i_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *iter;
    Nst_Obj *func;

    if (!Nst_extract_args("R f:o", arg_num, args, &iter, &func))
        return nullptr;

    if (Nst_func_arg_num(func) != 1) {
        Nst_set_call_error_c("the function must take exactly one argument");
        return nullptr;
    }

    Nst_Obj *arr = Nst_array_create(2, iter, func);

    return Nst_iter_new(
        Nst_func_new_c(1, map_i_start),
        Nst_func_new_c(1, map_i_next),
        arr);
}

Nst_Obj *NstC insert_at_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *vect;
    i64 idx;
    Nst_Obj *obj;

    if (!Nst_extract_args("v i o", arg_num, args, &vect, &idx, &obj))
        return nullptr;

    i64 new_idx = idx;
    usize vect_len = Nst_seq_len(vect);

    if (idx < 0)
        new_idx = vect_len + idx;

    if (new_idx < 0 || new_idx >= (i64)vect_len) {
        Nst_set_value_errorf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            idx,
            vect_len);

        return nullptr;
    }

    // Expand the vector
    Nst_vector_append(vect, Nst_null());
    Nst_dec_ref(Nst_null());
    Nst_Obj **objs = _Nst_seq_objs(vect);

    for (i64 i = vect_len - 1; i >= new_idx; i--)
        objs[i + 1] = objs[i];
    objs[new_idx] = Nst_inc_ref(obj);

    return Nst_null_ref();
}

Nst_Obj *NstC remove_at_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *vect;
    i64 idx;

    if (!Nst_extract_args("v i", arg_num, args, &vect, &idx))
        return nullptr;

    i64 new_idx = idx;
    usize vect_len = Nst_seq_len(vect);

    if (idx < 0)
        new_idx = vect_len + idx;

    if (new_idx < 0 || new_idx >= (i64)vect_len) {
        Nst_set_value_errorf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            idx,
            vect_len);

        return nullptr;
    }

    // Move the object to remove to the end
    Nst_Obj **objs = _Nst_seq_objs(vect);
    Nst_Obj *obj = objs[new_idx];
    for (; new_idx < (i64)vect_len; new_idx++)
        objs[new_idx] = objs[new_idx + 1];
    objs[vect_len - 1] = obj;

    Nst_vector_pop(vect, 1);
    return obj;
}

static isize clamp_slice_arguments(usize seq_len, Nst_Obj *start_obj,
                                   Nst_Obj *stop_obj, Nst_Obj *step_obj,
                                   i64 &start, i64 &step)
{
    step = Nst_DEF_VAL(step_obj,  AS_INT(step_obj), 1);

    if (step == 0) {
        Nst_set_value_error_c("the step cannot be zero");
        return -1;
    }

    if (seq_len == 0)
        return 0;

    start = Nst_DEF_VAL(
        start_obj,
        AS_INT(start_obj),
        step > 0 ? 0 : seq_len - 1);
    i64 stop = Nst_DEF_VAL(
        stop_obj,
        AS_INT(stop_obj),
        step > 0 ? seq_len : -(i64)seq_len - 1);

    if (start < 0)
        start += seq_len;

    if (stop < 0)
        stop += seq_len;

    if (step > 0) {
        if (start < 0)
            start = 0;

        if (stop > (i64)seq_len)
            stop = seq_len;

        if (stop <= start || start >= (i64)seq_len)
            return 0;
    } else {
        if (stop < -1)
            stop = -1;

        if (start >= (i64)seq_len)
            start = seq_len - 1;

        if (stop >= start || start < 0)
            return 0;
    }

    isize new_size = (isize)((stop - start) / step);
    if ((stop - start) % step != 0)
        new_size++;

    if (new_size <= 0)
        return 0;
    return new_size;
}

Nst_Obj *NstC slice_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *start_obj;
    Nst_Obj *stop_obj;
    Nst_Obj *step_obj;

    if (!Nst_extract_args(
            "S ?i ?i ?i",
            arg_num, args,
            &seq, &start_obj, &stop_obj, &step_obj))
    {
        return nullptr;
    }

    i64 start, step;
    isize new_size = clamp_slice_arguments(
        Nst_seq_len(seq),
        start_obj, stop_obj, step_obj,
        start, step);

    Nst_Obj *seq_t = args[0]->type;

    if (new_size == -1) {
        Nst_dec_ref(seq);
        return nullptr;
    }
    else if (new_size == 0) {
        Nst_dec_ref(seq);
        if (seq_t == Nst_type()->Str)
            return Nst_str_new((i8 *)"", 0, false);
        else if (seq_t == Nst_type()->Array)
            return Nst_array_new(0);
        else
            return Nst_vector_new(0);
    }

    Nst_Obj **objs = _Nst_seq_objs(seq);
    if (seq_t == Nst_type()->Array || seq_t == Nst_type()->Vector) {
        Nst_Obj *new_seq = seq_t == Nst_type()->Array
            ? Nst_array_new(new_size)
            : Nst_vector_new(new_size);

        for (isize i = 0; i < new_size; i++)
            Nst_seq_setf(new_seq, i, objs[i * step + start]);

        Nst_dec_ref(seq);
        return new_seq;
    } else {
        isize new_len = 0;
        for (isize i = 0; i < new_size; i++)
            new_len += Nst_str_len(objs[i * step + start]);

        i8 *buf = Nst_malloc_c(new_len + 1, i8);
        if (buf == nullptr) {
            Nst_dec_ref(seq);
            return nullptr;
        }

        for (isize i = 0; i < new_len;) {
            Nst_Obj *s = objs[i * step + start];
            memcpy(buf + i, Nst_str_value(s), Nst_str_len(s));
            i += Nst_str_len(s);
        }
        buf[new_len] = 0;
        Nst_dec_ref(seq);
        return Nst_str_new(buf, new_len, true);
    }
}

Nst_Obj *NstC slice_i_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *start_obj;
    Nst_Obj *stop_obj;
    Nst_Obj *step_obj;

    if (!Nst_extract_args(
            "s|a|v ?i ?i ?i",
            arg_num, args,
            &seq, &start_obj, &stop_obj, &step_obj))
    {
        return nullptr;
    }

    i64 start, step;
    isize new_size = clamp_slice_arguments(
        Nst_T(seq, Str) ? Nst_str_len(seq) : Nst_seq_len(seq),
        start_obj, stop_obj, step_obj,
        start, step);

    if (new_size == -1)
        return nullptr;

    Nst_Obj *arr = Nst_array_create_c(
        "iIIIO",
        0, start, step, i64(new_size), seq);

    if (Nst_T(seq, Str)) {
        return Nst_iter_new(
            Nst_func_new_c(1, slice_i_start),
            Nst_func_new_c(1, slice_i_str_next),
            arr);
    } else {
        return Nst_iter_new(
            Nst_func_new_c(1, slice_i_start),
            Nst_func_new_c(1, slice_i_seq_next),
            arr);
    }
}

Nst_Obj *NstC merge_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq1;
    Nst_Obj *seq2;

    if (!Nst_extract_args("A A", arg_num, args, &seq1, &seq2))
        return nullptr;

    Nst_Obj *new_seq;
    usize len1 = Nst_seq_len(seq1);
    usize len2 = Nst_seq_len(seq2);

    if (Nst_T(seq1, Vector) || Nst_T(seq2, Vector))
        new_seq = Nst_vector_new(len1 + len2);
    else
        new_seq = Nst_array_new(len1 + len2);

    i64 i = 0;

    for (i64 n = (i64)len1; i < n; i++)
        Nst_seq_setf(new_seq, i, Nst_seq_getnf(seq1, i));

    for (i64 j = i, n = (i64)len1; j - i < n; j++)
        Nst_seq_setf(new_seq, j, Nst_seq_getnf(seq2, j - i));

    return new_seq;
}

Nst_Obj *NstC extend_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *vec;
    Nst_Obj *seq;

    if (!Nst_extract_args("v v|a|s|I:a", arg_num, args, &vec, &seq))
        return nullptr;

    for (usize i = 0, n = Nst_seq_len(seq); i < n; i++)
        Nst_vector_append(vec, Nst_seq_getnf(seq, i));

    return Nst_inc_ref(vec);
}

typedef struct _MappedValue {
    Nst_Obj *mapped;
    Nst_Obj *original;
} MappedValue;

bool mapped_insertion_sort(MappedValue *values, isize left, isize right)
{
    for (isize i = left + 1; i < right; i++) {
        MappedValue temp = values[i];
        isize j = i - 1;

        while (true) {
            if (j < left)
                break;

            Nst_Obj *result = Nst_obj_gt(values[j].mapped, temp.mapped);
            if (result == nullptr) {
                return false;
            }
            bool c_result = Nst_obj_to_bool(result);
            Nst_dec_ref(result);
            if (!c_result)
                break;
            j--;
        }
        memmove(
            values + j + 2,
            values + j + 1,
            sizeof(MappedValue) * (i - j -1));
        values[j + 1] = temp;
    }
    return true;
}

bool insertion_sort(Nst_Obj **values, isize left, isize right)
{
    for (isize i = left + 1; i < right; i++) {
        Nst_Obj *temp = values[i];
        isize j = i - 1;

        while (true) {
            if (j < left)
                break;

            Nst_Obj *result = Nst_obj_gt(values[j], temp);
            if (result == nullptr) {
                return false;
            }
            bool c_result = Nst_obj_to_bool(result);
            Nst_dec_ref(result);
            if (!c_result)
                break;
            j--;
        }
        memmove(
            values + j + 2,
            values + j + 1,
            sizeof(Nst_Obj *) * (i - j -1));
        values[j + 1] = temp;
    }
    return true;
}

bool mapped_merge(MappedValue *values, usize left, usize mid, usize right,
                  MappedValue *&buf, usize &buf_size)
{
    usize new_size = right - left;

    if (new_size > buf_size) {
        MappedValue *new_buf = Nst_realloc_c(buf, new_size, MappedValue, 0);
        if (new_buf == nullptr) {
            if (buf != nullptr)
                Nst_free(buf);
            return false;
        }
        buf = new_buf;
        buf_size = new_size;
    }

    usize l_idx = left;
    usize r_idx = mid;
    usize idx = 0;

    while (l_idx < mid && r_idx < right) {
        Nst_Obj *result = Nst_obj_le(
            values[l_idx].mapped,
            values[r_idx].mapped);
        if (result == nullptr) {
            Nst_free(buf);
            return false;
        }
        bool c_result = Nst_obj_to_bool(result);
        Nst_dec_ref(result);

        if (c_result)
            buf[idx] = values[l_idx++];
        else
            buf[idx] = values[r_idx++];
        idx++;
    }

    while (l_idx < mid)
        buf[idx++] = values[l_idx++];

    while (r_idx < right)
        buf[idx++] = values[r_idx++];

    memcpy(values + left, buf, new_size * sizeof(MappedValue));
    return true;
}

bool merge(Nst_Obj **values, usize left, usize mid, usize right,
           Nst_Obj **&buf, usize &buf_size)
{
    usize new_size = right - left;

    if (new_size > buf_size) {
        Nst_Obj **new_buf = Nst_realloc_c(buf, new_size, Nst_Obj *, 0);
        if (new_buf == nullptr) {
            if (buf != nullptr)
                Nst_free(buf);
            return false;
        }
        buf = new_buf;
        buf_size = new_size;
    }

    usize l_idx = left;
    usize r_idx = mid;
    usize idx = 0;

    while (l_idx < mid && r_idx < right) {
        Nst_Obj *result = Nst_obj_le(values[l_idx], values[r_idx]);
        if (result == nullptr) {
            Nst_free(buf);
            return false;
        }
        bool c_result = Nst_obj_to_bool(result);
        Nst_dec_ref(result);

        if (c_result)
            buf[idx] = values[l_idx++];
        else
            buf[idx] = values[r_idx++];
        idx++;
    }

    while (l_idx < mid)
        buf[idx++] = values[l_idx++];

    while (r_idx < right)
        buf[idx++] = values[r_idx++];

    memcpy(values + left, buf, new_size * sizeof(Nst_Obj *));
    return true;
}

static Nst_Obj *mapped_sort(Nst_Obj *seq, Nst_Obj *map_func, bool new_seq)
{
    if (Nst_func_arg_num(map_func) != 1) {
        Nst_set_call_error_c("the function must take exactly one argument");
        return nullptr;
    }

    if (new_seq)
        seq = Nst_seq_copy(seq);

    usize seq_len = Nst_seq_len(seq);
    Nst_Obj **objs = _Nst_seq_objs(seq);
    MappedValue *buf = nullptr;
    usize buf_size = 0;

    MappedValue *values = Nst_malloc_c(seq_len, MappedValue);
    if (values == nullptr)
        return nullptr;

    for (usize i = 0; i < seq_len; i++) {
        Nst_Obj *mapped_value = Nst_func_call(map_func, 1, objs + i);
        if (mapped_value == nullptr) {
            for (usize j = 0; j < i; j++)
                Nst_dec_ref(values[j].mapped);
            return nullptr;
        }
        values[i].mapped = mapped_value;
        values[i].original = objs[i];
    }

    for (usize i = 0; i < seq_len; i += SORT_RUN_SIZE) {
        isize right = MIN(i + SORT_RUN_SIZE, seq_len);
        if (!mapped_insertion_sort(values, i, right))
            goto fail;
    }

    for (usize chunk_size = SORT_RUN_SIZE;
         chunk_size < seq_len;
         chunk_size *= 2)
    {
        for (usize left = 0; left < seq_len; left += 2 * chunk_size) {
            usize mid = left + chunk_size;
            usize right = MIN(left + 2*chunk_size, seq_len);

            if (mid >= right)
                continue;

            if (!mapped_merge(values, left, mid, right, buf, buf_size))
                goto fail; // buf is freed by mapped_merge
        }
    }

    if (buf != nullptr)
        Nst_free(buf);

    for (usize i = 0; i < seq_len; i++) {
        objs[i] = values[i].original;
        Nst_dec_ref(values[i].mapped);
    }
    Nst_free(values);
    return new_seq ? seq : Nst_inc_ref(seq);

fail:
    for (usize i = 0; i < seq_len; i++)
        Nst_dec_ref(values[i].mapped);
    Nst_free(values);
    if (new_seq)
        Nst_dec_ref(seq);
    return nullptr;
}

Nst_Obj *NstC sort_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *map_func;
    bool new_seq;

    if (!Nst_extract_args(
            "A ?f y",
            arg_num, args, &seq, &map_func, &new_seq))
    {
        return nullptr;
    }

    if (map_func != Nst_null())
        return mapped_sort(seq, map_func, new_seq);

    if (new_seq)
        seq = Nst_seq_copy(seq);

    usize seq_len = Nst_seq_len(seq);
    Nst_Obj **objs = _Nst_seq_objs(seq);
    Nst_Obj **buf = nullptr;
    usize buf_size = 0;

    for (usize i = 0; i < seq_len; i += SORT_RUN_SIZE) {
        isize right = MIN(i + SORT_RUN_SIZE, seq_len);
        if (!insertion_sort(objs, i, right)) {
            if (new_seq)
                Nst_dec_ref(seq);
            return nullptr;
        }
    }

    for (usize chunk_size = SORT_RUN_SIZE;
         chunk_size < seq_len;
         chunk_size *= 2)
    {
        for (usize left = 0; left < seq_len; left += 2 * chunk_size) {
            usize mid = left + chunk_size;
            usize right = MIN(left + 2*chunk_size, seq_len);

            if (mid >= right)
                continue;

            if (!merge(objs, left, mid, right, buf, buf_size)) {
                if (new_seq)
                    Nst_dec_ref(seq);
                return nullptr; // buf is freed by merge
            }
        }
    }

    if (buf != nullptr)
        Nst_free(buf);

    return new_seq ? seq : Nst_inc_ref(seq);
}

Nst_Obj *NstC empty_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *vect;

    if (!Nst_extract_args("v", arg_num, args, &vect))
        return nullptr;

    Nst_Obj *popped_obj = Nst_vector_pop(vect, Nst_seq_len(vect));
    Nst_ndec_ref(popped_obj);

    return Nst_inc_ref(vect);
}

Nst_Obj *NstC filter_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *func;

    if (!Nst_extract_args("A f", arg_num, args, &seq, &func))
        return nullptr;

    if (Nst_func_arg_num(func) != 1) {
        Nst_set_call_error_c("the function must take exactly one argument");
        return nullptr;
    }

    Nst_Obj *new_seq = Nst_vector_new(0);

    for (usize i = 0, n = Nst_seq_len(seq); i < n; i++) {
        Nst_Obj *arg = Nst_seq_getnf(seq, i);
        Nst_Obj *res = Nst_func_call(func, 1, &arg);

        if (res == nullptr)
            return nullptr;

        if (Nst_obj_to_bool(res))
            Nst_vector_append(new_seq, arg);

        Nst_dec_ref(res);
    }

    if (Nst_T(seq, Array)) {
        Nst_dec_ref(new_seq->type);
        new_seq->type = Nst_inc_ref(Nst_type()->Array);
    }

    return new_seq;
}

Nst_Obj *NstC filter_i_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *iter;
    Nst_Obj *func;

    if (!Nst_extract_args("R f:o", arg_num, args, &iter, &func))
        return nullptr;

    if (Nst_func_arg_num(func) != 1) {
        Nst_set_call_error_c("the function must take exactly one argument");
        return nullptr;
    }

    Nst_Obj *arr = Nst_array_create(2, iter, func);

    return Nst_iter_new(
        Nst_func_new_c(1, filter_i_start),
        Nst_func_new_c(1, filter_i_next),
        arr);
}

Nst_Obj *NstC any_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;

    if (!Nst_extract_args("A", arg_num, args, &seq))
        return nullptr;

    for (usize i = 0, n = Nst_seq_len(seq); i < n; i++) {
        if (Nst_obj_to_bool(Nst_seq_getnf(seq, i)))
            return Nst_true_ref();
    }

    return Nst_false_ref();
}

Nst_Obj *NstC all_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;

    if (!Nst_extract_args("A", arg_num, args, &seq))
        return nullptr;

    for (usize i = 0, n = Nst_seq_len(seq); i < n; i++) {
        if (!Nst_obj_to_bool(Nst_seq_getnf(seq, i)))
            return Nst_false_ref();
    }

    return Nst_true_ref();
}

Nst_Obj *NstC count_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *container;
    Nst_Obj *obj;

    if (!Nst_extract_args("a|v|s o", arg_num, args, &container, &obj))
        return nullptr;
    usize count = 0;

    if (Nst_T(container, Array) || Nst_T(container, Vector)) {
        for (usize i = 0, n = Nst_seq_len(container); i < n; i++) {
            if (Nst_obj_eq(Nst_seq_getnf(container, i), obj) == Nst_true()) {
                Nst_dec_ref(Nst_true());
                count++;
            } else
                Nst_dec_ref(Nst_false());
        }

        return Nst_int_new(count);
    } else {
        if (!Nst_T(obj, Str) || Nst_str_len(obj) == 0)
            return Nst_inc_ref(Nst_const()->Int_0);

        Nst_StrView str = Nst_sv_from_str(container);
        Nst_StrView sub = Nst_sv_from_str(obj);

        while (true) {
            isize res = Nst_sv_lfind(str, sub);
            if (res == -1)
                break;
            str = Nst_sv_new(
                str.value + res + sub.len,
                str.len - res - sub.len);
            count++;
        }
        return Nst_int_new(count);
    }
}

static i64 check_scan_args(usize seq_len, usize func_arg_num,
                           Nst_Obj *max_items_obj)
{
    i64 max_items = Nst_DEF_VAL(
        max_items_obj,
        AS_INT(max_items_obj),
        seq_len + 1);

    if (func_arg_num != 2) {
        Nst_set_value_error_c("the function must take exactly two argument");
        return -1;
    }

    if (max_items < 0) {
        Nst_set_value_error_c(
            "the maximum item count must be greater than or equal to zero");
        return -1;
    }

    if (max_items > (i64)seq_len + 1)
        max_items = seq_len + 1;
    return max_items;
}

Nst_Obj *NstC lscan_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *func;
    Nst_Obj *prev_val;
    Nst_Obj *max_items_obj;

    if (!Nst_extract_args(
            "S f o ?i",
            arg_num, args,
            &seq, &func, &prev_val, &max_items_obj))
    {
        return nullptr;
    }
    i64 max_items = check_scan_args(
        Nst_seq_len(seq),
        Nst_func_arg_num(func),
        max_items_obj);

    if (max_items < 0) {
        Nst_dec_ref(seq);
        return nullptr;
    }

    Nst_Obj *new_seq = Nst_T(seq, Array)
        ? Nst_array_new((usize)max_items)
        : Nst_vector_new((usize)max_items);
    if (max_items == 0) {
        Nst_dec_ref(seq);
        return new_seq;
    }

    Nst_inc_ref(prev_val);
    Nst_seq_setf(new_seq, 0, prev_val);

    Nst_Obj *func_args[2];

    for (i64 i = 1; i < max_items; i++) {
        func_args[0] = prev_val;
        func_args[1] = Nst_seq_getnf(seq, i - 1);
        Nst_Obj *new_val = Nst_func_call(func, 2, func_args);
        if (new_val == nullptr) {
            Nst_dec_ref(prev_val);
            Nst_dec_ref(new_seq);
            Nst_dec_ref(seq);
            return nullptr;
        }
        Nst_seq_setf(new_seq, i, new_val);
        Nst_dec_ref(prev_val);
        prev_val = new_val;
    }
    Nst_dec_ref(prev_val);
    Nst_dec_ref(seq);
    return new_seq;
}

Nst_Obj *NstC rscan_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *func;
    Nst_Obj *prev_val;
    Nst_Obj *max_items_obj;

    if (!Nst_extract_args(
            "S f o ?i",
            arg_num, args,
            &seq, &func, &prev_val, &max_items_obj))
    {
        return nullptr;
    }
    i64 max_items = check_scan_args(
        Nst_seq_len(seq),
        Nst_func_arg_num(func),
        max_items_obj);

    if (max_items < 0) {
        Nst_dec_ref(seq);
        return nullptr;
    }

    Nst_Obj *new_seq = Nst_T(seq, Array)
        ? Nst_array_new((usize)max_items)
        : Nst_vector_new((usize)max_items);
    if (max_items == 0) {
        Nst_dec_ref(seq);
        return new_seq;
    }

    Nst_inc_ref(prev_val);
    Nst_seq_setf(new_seq, max_items - 1, prev_val);

    Nst_Obj *func_args[2];
    i64 seq_len = (i64)Nst_seq_len(seq);

    for (i64 i = 1; i < max_items; i++) {
        func_args[0] = Nst_seq_getnf(seq, seq_len - i);
        func_args[1] = prev_val;
        Nst_Obj *new_val = Nst_func_call(func, 2, func_args);
        if (new_val == nullptr) {
            Nst_dec_ref(prev_val);
            Nst_dec_ref(new_seq);
            Nst_dec_ref(seq);
            return nullptr;
        }
        Nst_seq_setf(new_seq, max_items - i - 1, new_val);
        Nst_dec_ref(prev_val);
        prev_val = new_val;
    }
    Nst_dec_ref(prev_val);
    Nst_dec_ref(seq);
    return new_seq;
}

Nst_Obj *NstC copy_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *obj;
    if (!Nst_extract_args("a|v|m", arg_num, args, &obj))
        return nullptr;

    if (Nst_T(obj, Map))
        return Nst_map_copy(obj);

    return Nst_seq_copy(obj);
}

static Nst_Obj *obj_deep_copy(Nst_Obj *obj, Nst_Obj *cont_map);
static Nst_Obj *seq_deep_copy(Nst_Obj *seq, Nst_Obj *cont_map);
static Nst_Obj *map_deep_copy(Nst_Obj *map, Nst_Obj *cont_map);

static Nst_Obj *seq_deep_copy(Nst_Obj *seq, Nst_Obj *cont_map)
{
    Nst_Obj **old_objs;

    Nst_Obj *seq_id = Nst_int_new((i64)seq);
    if (seq_id == nullptr)
        return nullptr;

    Nst_Obj *new_seq = Nst_map_get(cont_map, seq_id);
    if (new_seq != nullptr)
        goto end;

    new_seq = Nst_T(seq, Array)
        ? Nst_array_new(Nst_seq_len(seq))
        : Nst_vector_new(Nst_seq_len(seq));

    if (new_seq == nullptr)
        goto end;

    if (!Nst_map_set(cont_map, seq_id, new_seq)) {
        Nst_dec_ref(new_seq);
        new_seq = nullptr;
        goto end;
    }

    old_objs = _Nst_seq_objs(seq);
    for (usize i = 0, n = Nst_seq_len(new_seq); i < n; i++) {
        Nst_Obj *copied_obj = obj_deep_copy(old_objs[i], cont_map);
        if (copied_obj == nullptr) {
            Nst_dec_ref(new_seq);
            new_seq = nullptr;
            goto end;
        }
        Nst_seq_setnf(new_seq, i, copied_obj);
    }

end:
    Nst_dec_ref(seq_id);
    return new_seq;
}

static Nst_Obj *map_deep_copy(Nst_Obj *map, Nst_Obj *cont_map)
{
    Nst_Obj *seq_id = Nst_int_new((i64)map);
    if (seq_id == nullptr)
        return nullptr;

    Nst_Obj *new_map = Nst_map_get(cont_map, seq_id);
    if (new_map != nullptr)
        goto end;

    new_map = Nst_map_new();

    if (new_map == nullptr)
        goto end;

    if (!Nst_map_set(cont_map, seq_id, new_map)) {
        Nst_dec_ref(new_map);
        new_map = nullptr;
        goto end;
    }

    Nst_Obj *key;
    Nst_Obj *value;
    for (isize i = Nst_map_next(-1, map, &key, &value);
         i != -1;
         i = Nst_map_next(i, map, &key, &value))
    {
        value = obj_deep_copy(value, cont_map);

        if (value == nullptr) {
            Nst_dec_ref(new_map);
            new_map = nullptr;
            goto end;
        }

        if (!Nst_map_set(new_map, key, value)) {
            Nst_dec_ref(value);
            Nst_dec_ref(new_map);
            new_map = nullptr;
            goto end;
        }
        Nst_dec_ref(value);
    }

end:
    Nst_dec_ref(seq_id);
    return new_map;
}

static Nst_Obj *obj_deep_copy(Nst_Obj *obj, Nst_Obj *cont_map)
{
    if (Nst_T(obj, Map))
        return map_deep_copy(obj, cont_map);
    else if (Nst_T(obj, Array) || Nst_T(obj, Vector))
        return seq_deep_copy(obj, cont_map);
    return Nst_inc_ref(obj);
}

Nst_Obj *NstC deep_copy_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *obj;
    if (!Nst_extract_args("a|v|m", arg_num, args, &obj))
        return nullptr;

    Nst_Obj *cont_map = Nst_map_new();
    if (cont_map == nullptr)
        return nullptr;

    Nst_Obj *res;

    if (Nst_T(obj, Map))
        res = map_deep_copy(obj, cont_map);
    else
        res = seq_deep_copy(obj, cont_map);

    Nst_dec_ref(cont_map);
    return res;
}

Nst_Obj *NstC enum_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    Nst_Obj *start_obj;
    if (!Nst_extract_args("A.s ?i", arg_num, args, &seq, &start_obj))
        return nullptr;

    i64 start = Nst_DEF_VAL(start_obj, AS_INT(start_obj), 0);
    Nst_Obj **objs = _Nst_seq_objs(seq);
    isize len = (isize)Nst_seq_len(seq);

    Nst_Obj *enum_map = Nst_map_new();
    if (enum_map == NULL)
        return NULL;
    for (i64 i = 0; i < len; i++) {
        Nst_Obj *prev_value = Nst_map_get(enum_map, objs[i]);
        if (prev_value != NULL) {
            Nst_dec_ref(prev_value);
            Nst_set_value_errorf(
                "repeated element '%.100s'",
                Nst_str_value(objs[i]));
            Nst_dec_ref(enum_map);
            return NULL;
        }

        Nst_Obj *value = Nst_int_new(start++);
        if (!Nst_map_set(enum_map, objs[i], value)) {
            Nst_dec_ref(value);
            Nst_dec_ref(enum_map);
            return NULL;
        }
        Nst_dec_ref(value);
    }
    return enum_map;
}

Nst_Obj *reverse_string(Nst_Obj *str_obj)
{
    usize old_str_len = Nst_str_len(str_obj);

    i8 *new_str = Nst_malloc_c(old_str_len + 1, i8);

    usize i = old_str_len;
    i8 ch_buf[4] = { 0 };
    isize str_idx = -1;

    for (i32 ch_len = Nst_str_next_utf8(str_obj, &str_idx, ch_buf);
         ch_len != 0;
         ch_len = Nst_str_next_utf8(str_obj, &str_idx, ch_buf))
    {
        i -= ch_len;
        memcpy(new_str + i, ch_buf, ch_len * sizeof(u8));
    }
    new_str[old_str_len] = '\0';

    Nst_Obj *new_str_obj = Nst_str_new_len(
        new_str,
        old_str_len,
        Nst_str_char_len(str_obj),
        true);
    if (new_str_obj == nullptr) {
        Nst_free(new_str_obj);
        return nullptr;
    }
    return new_str_obj;
}

Nst_Obj *reverse_in_place(Nst_Obj *seq)
{
    usize seq_len = Nst_seq_len(seq);
    Nst_Obj **objs = _Nst_seq_objs(seq);
    for (usize i = 0, n = seq_len / 2; i < n; i++) {
        Nst_Obj *temp = objs[i];
        objs[i] = objs[seq_len - i - 1];
        objs[seq_len - i - 1] = temp;
    }
    return Nst_inc_ref(seq);
}

Nst_Obj *reverse_new_obj(Nst_Obj *seq)
{
    usize seq_len = Nst_seq_len(seq);
    Nst_Obj *new_seq = Nst_T(seq, Array)
        ? Nst_array_new(seq_len)
        : Nst_vector_new(seq_len);
    if (new_seq == nullptr)
        return nullptr;
    Nst_Obj **objs = _Nst_seq_objs(seq);
    for (usize i = 0; i < seq_len; i++)
        Nst_seq_setf(new_seq, seq_len - i - 1, objs[i]);
    return new_seq;
}

Nst_Obj *NstC reverse_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;
    bool in_place;
    if (!Nst_extract_args("A|s y", arg_num, args, &seq, &in_place))
        return nullptr;

    if (Nst_T(seq, Str)) {
        if (in_place) {
            Nst_set_type_error_c("impossible to reverse a Str in-place");
            return nullptr;
        }
        return reverse_string(seq);
    }

    if (in_place)
        return reverse_in_place(seq);
    else
        return reverse_new_obj(seq);
}

Nst_Obj *NstC reverse_i_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;

    if (!Nst_extract_args("S", arg_num, args, &seq))
        return nullptr;

    // Layout: [idx, seq]
    Nst_Obj *arr = Nst_array_create_c("io", 0, seq);

    if (arr == nullptr) {
        Nst_dec_ref(seq);
        return nullptr;
    }

    return Nst_iter_new(
        Nst_func_new_c(1, reverse_i_start),
        Nst_func_new_c(1, reverse_i_next),
        arr);
}
