#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include "mem.h"
#include "sequence.h"
#include "obj_ops.h"
#include "lib_import.h"
#include "format.h"
#include "type.h"

static Nst_Obj *new_seq(usize len, usize size, Nst_TypeObj *type)
{
    Nst_SeqObj *seq = Nst_obj_alloc(Nst_SeqObj, type);
    if (seq == NULL)
        return NULL;

    Nst_GGC_OBJ_INIT(seq);

    Nst_Obj **objs = Nst_calloc_c(size, Nst_Obj *, NULL);
    if (objs == NULL) {
        Nst_free(seq);
        return NULL;
    }

    seq->len = len;
    seq->cap = size;
    seq->objs = objs;

    return OBJ(seq);
}

Nst_Obj *Nst_array_new(usize len)
{
    return new_seq(len, len, Nst_t.Array);
}

Nst_Obj *Nst_vector_new(usize len)
{
    usize size = (usize)(len * _Nst_VECTOR_GROWTH_RATIO);

    if (size < _Nst_VECTOR_MIN_SIZE)
        size = _Nst_VECTOR_MIN_SIZE;

    return new_seq(len, size, Nst_t.Vector);
}

void _Nst_seq_destroy(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for (usize i = 0, n = seq->len; i < n; i++)
        Nst_dec_ref(objs[i]);

    if (objs != NULL)
        Nst_free(objs);
}

void _Nst_seq_traverse(Nst_SeqObj *seq)
{
    Nst_Obj **objs = seq->objs;
    for (usize i = 0, n = seq->len; i < n; i++)
        Nst_ggc_obj_reachable(objs[i]);
}

bool _Nst_vector_resize(Nst_SeqObj *vect)
{
    usize len = vect->len;
    usize size = vect->cap;
    usize new_size;

    assert(len <= size);

    if (size == len)
        new_size = (usize)(len * _Nst_VECTOR_GROWTH_RATIO);
    else if (size >> 2 >= len) { // if it's three quarters empty or less
        new_size = (usize)(size / _Nst_VECTOR_GROWTH_RATIO);
        if (new_size < _Nst_VECTOR_MIN_SIZE)
            new_size = _Nst_VECTOR_MIN_SIZE;

        if (size == _Nst_VECTOR_MIN_SIZE)
            return true;
    } else
        return true;

    Nst_Obj **new_objs = Nst_realloc_c(
        vect->objs,
        new_size,
        Nst_Obj *,
        size);

    if (new_objs == NULL)
        return false;

    for (usize i = len; i < new_size; i++)
        new_objs[i] = NULL;

    vect->cap = new_size;
    vect->objs = new_objs;
    return true;
}

bool _Nst_vector_append(Nst_SeqObj *vect, Nst_Obj *val)
{
    if (!_Nst_vector_resize(vect))
        return false;

    vect->objs[vect->len++] = Nst_inc_ref(val);

    return true;
}

bool _Nst_seq_set(Nst_SeqObj *seq, i64 idx, Nst_Obj *val)
{
    if (idx < 0)
        idx += seq->len;

    if (idx < 0 || idx >= (i64)seq->len) {
        const i8 *fmt = seq->type == Nst_t.Array
          ? _Nst_EM_INDEX_OUT_OF_BOUNDS("Array")
          : _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector");
        Nst_set_value_errorf(fmt, idx, seq->len);
        return false;
    }

    Nst_inc_ref(val);
    if (seq->objs[idx] != NULL)
        Nst_dec_ref(seq->objs[idx]);
    seq->objs[idx] = val;

    return true;
}

Nst_Obj *_Nst_seq_get(Nst_SeqObj *seq, i64 idx)
{
    if (idx < 0)
        idx += seq->len;

    if (idx < 0 || idx >= (i64)seq->len) {
        const i8 *fmt = seq->type == Nst_t.Array
          ? _Nst_EM_INDEX_OUT_OF_BOUNDS("Array")
          : _Nst_EM_INDEX_OUT_OF_BOUNDS("Vector");
        Nst_set_value_errorf(fmt, idx, seq->len);
        return NULL;
    }

    return Nst_inc_ref(seq->objs[idx]);
}

bool _Nst_vector_remove(Nst_SeqObj *vect, Nst_Obj *val)
{
    usize i = 0;
    usize n = vect->len;
    Nst_Obj **objs = vect->objs;

    for (; i < n; i++) {
        if (Nst_obj_eq(val, objs[i]) == Nst_c.Bool_true) {
            Nst_dec_ref(Nst_c.Bool_true);
            Nst_dec_ref(objs[i]);
            break;
        } else
            Nst_dec_ref(Nst_c.Bool_false);

        if (i + 1 == n)
            return false;
    }

    for (i++; i < n; i++)
        vect->objs[i - 1] = objs[i];

    vect->len--;
    _Nst_vector_resize(vect);

    return true;
}

Nst_Obj *_Nst_vector_pop(Nst_SeqObj *vect, usize quantity)
{
    if (quantity > vect->len)
        quantity = vect->len;

    Nst_Obj *last_obj = NULL;
    usize n = vect->len;

    for (usize i = 1; i <= quantity; i++) {
        if (last_obj != NULL)
            Nst_dec_ref(last_obj);
        last_obj = vect->objs[n - i];
        vect->len--;
    }

    _Nst_vector_resize(vect);

    if (last_obj == NULL)
        Nst_RETURN_NULL;
    else
        // The reference is moved from the vector to the return, no need to add
        // a new one
        return last_obj;
}

static void seq_create(usize len, Nst_Obj *seq, va_list args)
{
    if (seq == NULL) {
        for (usize i = 0; i < len; i++) {
            Nst_Obj *arg = va_arg(args, Nst_Obj *);
            Nst_dec_ref(arg);
        }
        return;
    }

    Nst_Obj **objs = SEQ(seq)->objs;
    for (usize i = 0; i < len; i++) {
        Nst_Obj *arg = va_arg(args, Nst_Obj *);
        objs[i] = arg;
    }
}

Nst_Obj *Nst_vector_create(usize len, ...)
{
    Nst_Obj *vector = Nst_vector_new(len);
    va_list args;
    va_start(args, len);
    seq_create(len, vector, args);
    va_end(args);
    return vector;
}

Nst_Obj *Nst_array_create(usize len, ...)
{
    Nst_Obj *array = Nst_array_new(len);
    va_list args;
    va_start(args, len);
    seq_create(len, array, args);
    va_end(args);
    return array;
}

Nst_Obj *seq_create_c(Nst_Obj *seq, const i8 *fmt, va_list args)
{
    if (seq == NULL)
        return NULL;

    i8 *p = (i8 *)fmt;
    usize i = 0;
    Nst_Obj **objs = SEQ(seq)->objs;
    while (*p) {
        switch (*p++) {
        case 'I': {
            i64 value = va_arg(args, i64);
            Nst_Obj *obj = Nst_int_new(value);
            if (obj == NULL)
                goto failed;
            objs[i] = obj;
            break;
        }
        case 'i': {
            i32 value = va_arg(args, i32);
            Nst_Obj *obj = Nst_int_new((i64)value);
            if (obj == NULL)
                goto failed;
            objs[i] = obj;
            break;
        }
        case 'f':
        case 'F': {
            f64 value = va_arg(args, f64);
            Nst_Obj *obj = Nst_real_new(value);
            if (obj == NULL)
                goto failed;
            objs[i] = obj;
            break;
        }
        case 'b': {
            int value = va_arg(args, int);
            if (value)
                objs[i] = Nst_inc_ref(Nst_c.Bool_true);
            else
                objs[i] = Nst_inc_ref(Nst_c.Bool_false);
            break;
        }
        case 'B': {
            int value = va_arg(args, int);
            Nst_Obj *obj = Nst_byte_new((u8)value);
            if (obj == NULL)
                goto failed;
            objs[i] = obj;
            break;
        }
        case 'o': {
            Nst_Obj *obj = va_arg(args, Nst_Obj *);
            objs[i] = obj;
            break;
        }
        case 'O': {
            Nst_Obj *obj = va_arg(args, Nst_Obj *);
            objs[i] = Nst_inc_ref(obj);
            break;
        }
        case 'n': {
            (void)va_arg(args, void *);
            objs[i] = Nst_inc_ref(Nst_c.Null_null);
            break;
        }
        default:
            if (seq->type == Nst_t.Vector) {
                Nst_set_value_error_c(
                    _Nst_EM_INVALID_TYPE_LETTER("Nst_vector_create_c"));
            } else {
                Nst_set_value_error_c(
                    _Nst_EM_INVALID_TYPE_LETTER("Nst_array_create_c"));
            }
            goto failed;
        }
        i++;
    }

    return seq;
failed:
    SEQ(seq)->len = i;
    Nst_dec_ref(seq);
    return NULL;
}

Nst_Obj *Nst_vector_create_c(const i8 *fmt, ...)
{
    usize len = strlen(fmt);
    Nst_Obj *vector = Nst_vector_new(len);
    va_list args;
    va_start(args, fmt);
    vector = seq_create_c(vector, fmt, args);
    va_end(args);
    return vector;
}

Nst_Obj *Nst_array_create_c(const i8 *fmt, ...)
{
    usize len = strlen(fmt);
    Nst_Obj *array = Nst_array_new(len);
    va_list args;
    va_start(args, fmt);
    array = seq_create_c(array, fmt, args);
    va_end(args);
    return array;
}
