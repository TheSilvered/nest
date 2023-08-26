#include <cmath>
#include <cstring>
#include "nest_sequtil.h"

#define FUNC_COUNT 14
#define SORT_RUN_SIZE 32

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
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(sort_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(empty_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(filter_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(contains_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(any_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(all_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(count_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(lscan_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(rscan_, 4);

#if __LINE__ - FUNC_COUNT != 20
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

    Nst_DEF_EXTRACT("A f", &seq, &func);

    if (func->arg_num != 1) {
        Nst_set_value_error_c("the function must take exactly one argument");
        return nullptr;
    }

    Nst_SeqObj *new_seq = Nst_T(seq, Array)
        ? SEQ(Nst_array_new(seq->len))
        : SEQ(Nst_vector_new(seq->len));

    for (usize i = 0, n = seq->len; i < n; i++) {
        Nst_Obj *arg = Nst_seq_get(seq, i);
        Nst_Obj *res = Nst_call_func(func, &arg);

        if (res == nullptr) {
            for (usize j = 0; j < i; j++)
                Nst_dec_ref(new_seq->objs[j]);
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
    i64 idx;
    Nst_Obj *obj;

    Nst_DEF_EXTRACT("v i o", &vect, &idx, &obj);

    i64 new_idx = idx;

    if (idx < 0)
        new_idx = vect->len + idx;

    if (new_idx < 0 || new_idx >= (i64)vect->len) {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            idx,
            vect->len));

        return nullptr;
    }

    // Force the vector to grow
    Nst_vector_append(vect, Nst_null());
    Nst_dec_ref(Nst_null());

    for (i64 i = vect->len - 1; i >= new_idx; i--)
        vect->objs[i + 1] = vect->objs[i];
    vect->objs[new_idx] = Nst_inc_ref(obj);

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(remove_at_)
{
    Nst_SeqObj *vect;
    i64 idx;

    Nst_DEF_EXTRACT("v i", &vect, &idx);

    i64 new_idx = idx;

    if (idx < 0)
        new_idx = vect->len + idx;

    if (new_idx < 0 || new_idx >= (i64)vect->len) {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector"),
            idx,
            vect->len));

        return nullptr;
    }

    Nst_Obj *obj = vect->objs[new_idx];
    vect->len--;
    for (; new_idx < (i64)vect->len; new_idx++)
        vect->objs[new_idx] = vect->objs[new_idx + 1];

    _Nst_vector_resize(vect);
    return obj;
}

Nst_FUNC_SIGN(slice_)
{
    Nst_SeqObj *seq;
    Nst_Obj *start_obj;
    Nst_Obj *stop_obj;
    Nst_Obj *step_obj;

    Nst_DEF_EXTRACT("S ?i ?i ?i", &seq, &start_obj, &stop_obj, &step_obj);

    i64 step = Nst_DEF_VAL(step_obj,  AS_INT(step_obj), 1);
    i64 start = Nst_DEF_VAL(
        start_obj,
        AS_INT(start_obj),
        step > 0 ? 0 : seq->len);
    i64 stop = Nst_DEF_VAL(
        stop_obj,
        AS_INT(stop_obj),
        step > 0 ? seq->len : -1);

    usize seq_len = seq->len;
    Nst_TypeObj *seq_type = args[0]->type;

    if (step == 0) {
        Nst_set_value_error_c("the step cannot be zero");
        return nullptr;
    }

    if (start < 0)
        start += seq_len;

    if (stop < 0 && stop_obj != Nst_null())
        stop += seq_len;

    if (start < 0)
        start = 0;
    else if (start >= (i64)seq_len)
        start = seq_len - 1;

    if (stop < 0 && stop_obj != Nst_null())
        stop = 0;
    else if (stop > (i64)seq_len)
        stop = seq_len;

    isize new_size = (isize)((stop - start) / step);

    if ((stop - start) % step != 0)
        new_size++;

    if (new_size <= 0 || seq_len == 0) {
        Nst_dec_ref(seq);
        if (seq_type == Nst_type()->Str)
            return Nst_string_new((i8 *)"", 0, false);
        else if (seq_type == Nst_type()->Array)
            return Nst_array_new(0);
        else
            return Nst_vector_new(0);
    }

    if (Nst_T(seq_type, Array) || Nst_T(seq_type, Vector)) {
        Nst_Obj *new_seq = Nst_T(seq_type, Array)
            ? Nst_array_new(new_size)
            : Nst_vector_new(new_size);

        for (isize i = 0; i < new_size; i++)
            Nst_seq_set(new_seq, i, seq->objs[i * step + start]);

        Nst_dec_ref(seq);
        return new_seq;
    } else {
        i8 *buf = Nst_malloc_c(new_size + 1, i8);
        if (buf == nullptr)
            return nullptr;

        for (isize i = 0; i < new_size; i++)
            buf[i] = STR(seq->objs[i * step + start])->value[0];
        buf[new_size] = 0;
        Nst_dec_ref(seq);
        return Nst_string_new(buf, new_size, true);
    }
}

Nst_FUNC_SIGN(merge_)
{
    Nst_SeqObj *seq1;
    Nst_SeqObj *seq2;

    Nst_DEF_EXTRACT("A A", &seq1, &seq2);

    Nst_SeqObj *new_seq;

    if (Nst_T(seq1, Vector) || Nst_T(seq2, Vector))
        new_seq = SEQ(Nst_vector_new(seq1->len + seq2->len));
    else
        new_seq = SEQ(Nst_array_new(seq1->len + seq2->len));

    i64 i = 0;

    for (i64 n = (i64)seq1->len; i < n; i++)
        Nst_seq_set(new_seq, i, seq1->objs[i]);

    for (i64 j = i, n = (i64)seq2->len; j - i < n; j++)
        Nst_seq_set(new_seq, j, seq2->objs[j - i]);

    return OBJ(new_seq);
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

Nst_Obj *mapped_sort(Nst_SeqObj *seq, Nst_FuncObj *map_func) {
    if (map_func->arg_num != 1) {
        Nst_set_call_error_c("the function must take exactly one argument");
        return nullptr;
    }

    usize seq_len = seq->len;
    Nst_Obj **objs = seq->objs;
    MappedValue *buf = nullptr;
    usize buf_size = 0;

    MappedValue *values = Nst_malloc_c(seq_len, MappedValue);
    if (values == nullptr)
        return nullptr;

    for (usize i = 0; i < seq_len; i++) {
        Nst_Obj *mapped_value = Nst_call_func(FUNC(map_func), &objs[i]);
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
    return Nst_inc_ref(seq);

fail:
    for (usize i = 0; i < seq_len; i++)
        Nst_dec_ref(values[i].mapped);
    Nst_free(values);
    return nullptr;
}

Nst_FUNC_SIGN(sort_)
{
    Nst_SeqObj *seq;
    Nst_Obj *map_func;

    Nst_DEF_EXTRACT("A ?f", &seq, &map_func);

    if (map_func != Nst_null())
        return mapped_sort(seq, FUNC(map_func));

    usize seq_len = seq->len;
    Nst_Obj **objs = seq->objs;
    Nst_Obj **buf = nullptr;
    usize buf_size = 0;

    for (usize i = 0; i < seq_len; i += SORT_RUN_SIZE) {
        isize right = MIN(i + SORT_RUN_SIZE, seq_len);
        if (!insertion_sort(objs, i, right))
            return nullptr;
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

            if (!merge(objs, left, mid, right, buf, buf_size))
                return nullptr; // buf is freed by merge
        }
    }

    if (buf != nullptr)
        Nst_free(buf);

    return Nst_inc_ref(seq);
}

Nst_FUNC_SIGN(empty_)
{
    Nst_SeqObj *vect;

    Nst_DEF_EXTRACT("v", &vect);

    for (usize i = 0, n = vect->len; i < n; i++)
        Nst_dec_ref(vect->objs[i]);

    vect->len = 0;

    return Nst_inc_ref(vect);
}

Nst_FUNC_SIGN(filter_)
{
    Nst_SeqObj *seq;
    Nst_FuncObj *func;

    Nst_DEF_EXTRACT("A f", &seq, &func);

    if (func->arg_num != 1) {
        Nst_set_call_error_c("the function must take exactly one argument");
        return nullptr;
    }

    Nst_SeqObj *new_seq = SEQ(Nst_vector_new(0));

    for (usize i = 0, n = seq->len; i < n; i++) {
        Nst_Obj *res = Nst_call_func(func, &seq->objs[i]);

        if (res == nullptr)
            return nullptr;

        if (Nst_obj_cast(res, Nst_type()->Bool) == Nst_true()) {
            Nst_vector_append(new_seq, seq->objs[i]);
            Nst_dec_ref(Nst_true());
        } else
            Nst_dec_ref(Nst_false());

        Nst_dec_ref(res);
    }

    if (Nst_T(seq, Array)) {
        Nst_Obj **new_objs = Nst_realloc_c(
            new_seq->objs,
            new_seq->len,
            Nst_Obj *,
            new_seq->cap);
        if (new_objs != new_seq->objs)
            new_seq->cap = new_seq->len;
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

    Nst_DEF_EXTRACT("a|v|m|s o", &container, &object);

    if (Nst_T(container, Array) || Nst_T(container, Vector)) {
        Nst_SeqObj *seq = SEQ(container);

        for (usize i = 0, n = seq->len; i < n; i++) {
            if (Nst_obj_eq(seq->objs[i], object) == Nst_true())
                return Nst_true();
            else
                Nst_dec_ref(Nst_false());
        }

        Nst_RETURN_FALSE;
    } else if (Nst_T(container, Map)) {
        Nst_MapObj *map = MAP(container);

        if (Nst_obj_hash(object) == -1)
            Nst_RETURN_FALSE;

        Nst_Obj *item = Nst_map_get(map, object);
        if (item == nullptr)
            Nst_RETURN_FALSE;
        else {
            Nst_dec_ref(item);
            Nst_RETURN_TRUE;
        }
    } else {
        if (!Nst_T(container, Str))
            Nst_RETURN_FALSE;

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

    for (usize i = 0, n = seq->len; i < n; i++) {
        if (Nst_obj_to_bool(seq->objs[i]))
            Nst_RETURN_TRUE;
    }

    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(all_)
{
    Nst_SeqObj *seq;

    Nst_DEF_EXTRACT("A", &seq);

    for (usize i = 0, n = seq->len; i < n; i++) {
        if (!Nst_obj_to_bool(seq->objs[i]))
            Nst_RETURN_FALSE;
    }

    Nst_RETURN_TRUE;
}

Nst_FUNC_SIGN(count_)
{
    Nst_Obj *container;
    Nst_Obj *obj;

    Nst_DEF_EXTRACT("a|v|s o", &container, &obj);
    usize count = 0;

    if (Nst_T(container, Array) || Nst_T(container, Vector)) {
        Nst_SeqObj *seq = SEQ(container);

        for (usize i = 0, n = seq->len; i < n; i++) {
            if (Nst_obj_eq(seq->objs[i], obj) == Nst_true()) {
                Nst_dec_ref(Nst_true());
                count++;
            } else
                Nst_dec_ref(Nst_false());
        }

        return Nst_int_new(count);
    } else {
        if (!Nst_T(obj, Str) || STR(obj)->len == 0)
            Nst_RETURN_ZERO;

        i8 *str = STR(container)->value;
        usize str_len = STR(container)->len;
        i8 *sub = STR(obj)->value;
        usize sub_len = STR(obj)->len;

        while (true) {
            i8 *res = Nst_string_find(str, str_len, sub, sub_len);
            if (res == nullptr)
                break;
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
    i64 max_items = Nst_DEF_VAL(
        max_items_obj,+
        AS_INT(max_items_obj),
        seq->len + 1);

    if (func->arg_num != 2) {
        Nst_set_value_error_c("the function must take exactly two argument");
        Nst_dec_ref(seq);
        return nullptr;
    }

    if (max_items < 0) {
        Nst_set_value_error_c(
            "the maximum item count must be greater than or equal to zero");
        Nst_dec_ref(seq);
        return nullptr;
    }

    if (max_items > (i64)seq->len + 1)
        max_items = seq->len + 1;

    Nst_SeqObj *new_seq = Nst_T(seq, Array)
        ? SEQ(Nst_array_new((usize)max_items))
        : SEQ(Nst_vector_new((usize)max_items));
    if (max_items == 0) {
        Nst_dec_ref(seq);
        return OBJ(new_seq);
    }

    Nst_inc_ref(prev_val);
    Nst_seq_set(new_seq, 0, prev_val);

    Nst_Obj *func_args[2];

    for (i64 i = 1; i < max_items; i++) {
        func_args[0] = prev_val;
        func_args[1] = seq->objs[i - 1];
        Nst_Obj *new_val = Nst_call_func(func, func_args);
        if (new_val == nullptr) {
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
    i64 max_items = Nst_DEF_VAL(max_items_obj, AS_INT(max_items_obj), seq->len + 1);

    if (func->arg_num != 2) {
        Nst_set_value_error_c("the function must take exactly two argument");
        Nst_dec_ref(seq);
        return nullptr;
    }

    if (max_items < 0) {
        Nst_set_value_error_c(
            "the maximum item count must be greater than or equal to zero");
        Nst_dec_ref(seq);
        return nullptr;
    }

    i64 seq_len = (i64)seq->len;

    if (max_items > seq_len + 1)
        max_items = seq_len + 1;

    Nst_SeqObj *new_seq = Nst_T(seq, Array)
        ? SEQ(Nst_array_new((usize)max_items))
        : SEQ(Nst_vector_new((usize)max_items));
    if (max_items == 0) {
        Nst_dec_ref(seq);
        return OBJ(new_seq);
    }

    Nst_inc_ref(prev_val);
    Nst_seq_set(new_seq, max_items - 1, prev_val);

    Nst_Obj *func_args[2];

    for (i64 i = max_items - 2; i >= 0; i--) {
        func_args[0] = seq->objs[i + seq_len - max_items + 1];
        func_args[1] = prev_val;
        Nst_Obj *new_val = Nst_call_func(func, func_args);
        if (new_val == nullptr) {
            Nst_dec_ref(prev_val);
            for (i64 j = max_items - 1; j > i; j--)
                Nst_dec_ref(new_seq->objs[max_items - j - 1]);
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
