#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include "nest.h"

#define assert_sequence(seq)                                                  \
    Nst_assert((seq)->type == Nst_t.Array || (seq)->type == Nst_t.Vector)
#define assert_vector(vect) Nst_assert((vect)->type == Nst_t.Vector)

typedef struct _Nst_SeqObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_Obj **objs;
    usize len;
    usize cap;
} Nst_SeqObj;

#define SEQ(ptr) ((Nst_SeqObj *)(ptr))

static bool resize_vector(Nst_Obj *vect);
static Nst_SeqObj *new_seq_empty(usize len, usize size, Nst_Obj *type);
static Nst_SeqObj *new_array(usize len);
static Nst_SeqObj *new_vector(usize len);
static Nst_ObjRef *new_seq(usize len, usize size, Nst_Obj *type);
static void seq_from_objs(usize len, Nst_SeqObj *seq, Nst_Obj **objs, bool ref);
static void seq_create(usize len, Nst_SeqObj *seq, va_list args);
static Nst_SeqObj *seq_create_c(Nst_SeqObj *seq, const char *fmt, va_list args);

static Nst_SeqObj *new_seq_empty(usize len, usize size, Nst_Obj *type)
{
    Nst_SeqObj *seq = Nst_obj_alloc(Nst_SeqObj, type);
    if (seq == NULL)
        return NULL;

    Nst_Obj **objs = Nst_calloc_c(size, Nst_Obj *, NULL);
    if (objs == NULL) {
        Nst_free(seq);
        return NULL;
    }

    seq->len = len;
    seq->cap = size;
    seq->objs = objs;

    Nst_GGC_OBJ_INIT(seq);

    return seq;
}

static Nst_SeqObj *new_array(usize len)
{
    return new_seq_empty(len, len, Nst_t.Array);
}

static Nst_SeqObj *new_vector(usize len)
{
    usize size = (usize)(len * _Nst_VECTOR_GROWTH_RATIO);

    if (size < _Nst_VECTOR_MIN_CAP)
        size = _Nst_VECTOR_MIN_CAP;

    return new_seq_empty(len, size, Nst_t.Vector);
}

static Nst_ObjRef *new_seq(usize len, usize size, Nst_Obj *type)
{
    Nst_SeqObj *seq = new_seq_empty(len, size, type);
    if (seq == NULL)
        return NULL;
    for (usize i = 0; i < len; i++)
        seq->objs[i] = Nst_null_ref();
    return NstOBJ(seq);
}

Nst_ObjRef *Nst_array_new(usize len)
{
    return new_seq(len, len, Nst_t.Array);
}

Nst_ObjRef *Nst_vector_new(usize len)
{
    usize size = (usize)(len * _Nst_VECTOR_GROWTH_RATIO);

    if (size < _Nst_VECTOR_MIN_CAP)
        size = _Nst_VECTOR_MIN_CAP;

    return new_seq(len, size, Nst_t.Vector);
}

Nst_ObjRef *_Nst_array_new_empty(usize len)
{
    return NstOBJ(new_array(len));
}

Nst_ObjRef *_Nst_vector_new_empty(usize len)
{
    return NstOBJ(new_vector(len));
}

static void seq_from_objs(usize len, Nst_SeqObj *seq, Nst_Obj **objs, bool ref)
{
    memcpy(seq->objs, objs, len * sizeof(Nst_Obj *));
    if (!ref)
        return;
    for (usize i = 0; i < len; i++)
        Nst_inc_ref(objs[i]);
}

Nst_ObjRef *Nst_array_from_objs(usize len, Nst_Obj **objs)
{
    Nst_SeqObj *array = new_array(len);
    if (array == NULL)
        return NULL;
    seq_from_objs(len, array, objs, true);
    return NstOBJ(array);
}

Nst_ObjRef *Nst_vector_from_objs(usize len, Nst_Obj **objs)
{
    Nst_SeqObj *vect = new_vector(len);
    if (vect == NULL)
        return NULL;
    seq_from_objs(len, vect, objs, true);
    return NstOBJ(vect);
}

Nst_ObjRef *Nst_array_from_objsn(usize len, Nst_ObjRef **objs)
{
    Nst_SeqObj *array = new_array(len);
    if (array == NULL)
        return NULL;
    seq_from_objs(len, array, objs, false);
    return NstOBJ(array);
}

Nst_ObjRef *Nst_vector_from_objsn(usize len, Nst_ObjRef **objs)
{
    Nst_SeqObj *vect = new_vector(len);
    if (vect == NULL)
        return NULL;
    seq_from_objs(len, vect, objs, false);
    return NstOBJ(vect);
}

static void seq_create(usize len, Nst_SeqObj *seq, va_list args)
{
    if (seq == NULL) {
        for (usize i = 0; i < len; i++) {
            Nst_Obj *arg = va_arg(args, Nst_Obj *);
            Nst_dec_ref(arg);
        }
        return;
    }

    Nst_Obj **objs = seq->objs;
    for (usize i = 0; i < len; i++) {
        Nst_Obj *arg = va_arg(args, Nst_Obj *);
        objs[i] = arg;
    }
}

Nst_ObjRef *Nst_array_create(usize len, ...)
{
    Nst_SeqObj *array = new_array(len);
    va_list args;
    va_start(args, len);
    seq_create(len, array, args);
    va_end(args);
    return NstOBJ(array);
}

Nst_ObjRef *Nst_vector_create(usize len, ...)
{
    Nst_SeqObj *vector = new_vector(len);
    va_list args;
    va_start(args, len);
    seq_create(len, vector, args);
    va_end(args);
    return NstOBJ(vector);
}

static Nst_SeqObj *seq_create_c(Nst_SeqObj *seq, const char *fmt, va_list args)
{
    if (seq == NULL)
        return NULL;

    usize i = 0;
    Nst_Obj **objs = seq->objs;
    while (*fmt) {
        switch (*fmt++) {
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
        case 's': {
            const char *str = va_arg(args, const char *);
            // Force the contents to be copied
            Nst_Obj *obj = Nst_str_from_sv(Nst_sv_new_c(str));
            if (obj == NULL)
                goto failed;
            objs[i] = obj;
            break;
        }
        case 'S': {
            Nst_StrView sv = va_arg(args, Nst_StrView);
            Nst_Obj *obj = Nst_str_from_sv(sv);
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
            Nst_assert(0);
        }
        i++;
    }

    return seq;
failed:
    seq->len = i;
    Nst_dec_ref(NstOBJ(seq));
    return NULL;
}

Nst_ObjRef *Nst_array_create_c(const char *fmt, ...)
{
    usize len = strlen(fmt);
    Nst_SeqObj *array = new_array(len);
    va_list args;
    va_start(args, fmt);
    array = seq_create_c(array, fmt, args);
    va_end(args);
    return NstOBJ(array);
}

Nst_ObjRef *Nst_vector_create_c(const char *fmt, ...)
{
    usize len = strlen(fmt);
    Nst_SeqObj *vector = new_vector(len);
    va_list args;
    va_start(args, fmt);
    vector = seq_create_c(vector, fmt, args);
    va_end(args);
    return NstOBJ(vector);
}

Nst_ObjRef *Nst_seq_copy(Nst_Obj *seq)
{
    assert_sequence(seq);
    return seq->type == Nst_t.Array
        ? Nst_array_from_objs(SEQ(seq)->len, SEQ(seq)->objs)
        : Nst_vector_from_objs(SEQ(seq)->len, SEQ(seq)->objs);
}

void _Nst_seq_destroy(Nst_Obj *seq)
{
    assert_sequence(seq);
    Nst_Obj **objs = SEQ(seq)->objs;
    for (usize i = 0, n = SEQ(seq)->len; i < n; i++)
        Nst_dec_ref(objs[i]);

    if (objs != NULL)
        Nst_free(objs);
}

void _Nst_seq_traverse(Nst_Obj *seq)
{
    assert_sequence(seq);
    Nst_Obj **objs = SEQ(seq)->objs;
    for (usize i = 0, n = SEQ(seq)->len; i < n; i++)
        Nst_ggc_obj_reachable(objs[i]);
}

usize Nst_seq_len(Nst_Obj *seq)
{
    assert_sequence(seq);
    return SEQ(seq)->len;
}

usize Nst_vector_cap(Nst_Obj *vect)
{
    assert_vector(vect);
    return SEQ(vect)->cap;
}

Nst_Obj **Nst_seq_objs(Nst_Obj *seq)
{
    assert_sequence(seq);
    return SEQ(seq)->objs;
}

bool Nst_seq_set(Nst_Obj *seq, i64 idx, Nst_Obj *val)
{
    assert_sequence(seq);
    if (idx < 0)
        idx += SEQ(seq)->len;

    if (idx < 0 || idx >= (i64)SEQ(seq)->len) {
        const char *fmt = seq->type == Nst_t.Array
          ? "index %lli out of bounds for 'Array' of size %zi"
          : "index %lli out of bounds for 'Vector' of size %zi";
        Nst_error_setf_value(fmt, idx, SEQ(seq)->len);
        return false;
    }

    Nst_assert_c(SEQ(seq)->objs[idx] != NULL);

    Nst_inc_ref(val);
    Nst_dec_ref(SEQ(seq)->objs[idx]);
    SEQ(seq)->objs[idx] = val;

    return true;
}

void Nst_seq_setf(Nst_Obj *seq, usize idx, Nst_Obj *val)
{
    assert_sequence(seq);
    Nst_assert(idx < SEQ(seq)->len);
    Nst_inc_ref(val);
    Nst_dec_ref(SEQ(seq)->objs[idx]);
    SEQ(seq)->objs[idx] = val;
}

bool Nst_seq_setn(Nst_Obj *seq, i64 idx, Nst_ObjRef *val)
{
    assert_sequence(seq);
    if (idx < 0)
        idx += SEQ(seq)->len;

    if (idx < 0 || idx >= (i64)SEQ(seq)->len) {
        const char *fmt = seq->type == Nst_t.Array
            ? "index %lli out of bounds for 'Array' of size %zi"
            : "index %lli out of bounds for 'Vector' of size %zi";
        Nst_error_setf_value(fmt, idx, SEQ(seq)->len);
        return false;
    }

    Nst_assert_c(SEQ(seq)->objs[idx] != NULL);
    Nst_dec_ref(SEQ(seq)->objs[idx]);
    SEQ(seq)->objs[idx] = val;

    return true;
}

void Nst_seq_setnf(Nst_Obj *seq, usize idx, Nst_ObjRef *val)
{
    assert_sequence(seq);
    Nst_assert(idx < SEQ(seq)->len);
    Nst_dec_ref(SEQ(seq)->objs[idx]);
    SEQ(seq)->objs[idx] = val;
}

Nst_ObjRef *Nst_seq_get(Nst_Obj *seq, i64 idx)
{
    assert_sequence(seq);
    if (idx < 0)
        idx += SEQ(seq)->len;

    if (idx < 0 || idx >= (i64)SEQ(seq)->len) {
        const char *fmt = seq->type == Nst_t.Array
          ? "index %lli out of bounds for 'Array' of size %zi"
          : "index %lli out of bounds for 'Vector' of size %zi";
        Nst_error_setf_value(fmt, idx, SEQ(seq)->len);
        return NULL;
    }

    return Nst_inc_ref(SEQ(seq)->objs[idx]);
}

Nst_ObjRef *Nst_seq_getf(Nst_Obj *seq, usize idx)
{
    assert_sequence(seq);
    Nst_assert(idx < SEQ(seq)->len);
    return Nst_inc_ref(SEQ(seq)->objs[idx]);
}

Nst_Obj *Nst_seq_getn(Nst_Obj *seq, i64 idx)
{
    assert_sequence(seq);
    if (idx < 0)
        idx += SEQ(seq)->len;

    if (idx < 0 || idx >= (i64)SEQ(seq)->len) {
        const char *fmt = seq->type == Nst_t.Array
            ? "index %lli out of bounds for 'Array' of size %zi"
            : "index %lli out of bounds for 'Vector' of size %zi";
        Nst_error_setf_value(fmt, idx, SEQ(seq)->len);
        return NULL;
    }

    return SEQ(seq)->objs[idx];
}

Nst_Obj *Nst_seq_getnf(Nst_Obj *seq, usize idx)
{
    assert_sequence(seq);
    Nst_assert(idx < SEQ(seq)->len);
    return SEQ(seq)->objs[idx];
}

static bool resize_vector(Nst_Obj *vect)
{
    assert_vector(vect);

    usize len = SEQ(vect)->len;
    usize size = SEQ(vect)->cap;
    usize new_size;

    Nst_assert(len <= size);

    if (size == len)
        new_size = (usize)(len * _Nst_VECTOR_GROWTH_RATIO);
    else if (size >> 2 >= len) { // if it's three quarters empty or less
        new_size = (usize)(size / _Nst_VECTOR_GROWTH_RATIO);
        if (new_size < _Nst_VECTOR_MIN_CAP)
            new_size = _Nst_VECTOR_MIN_CAP;

        if (size == _Nst_VECTOR_MIN_CAP)
            return true;
    } else
        return true;

    Nst_Obj **new_objs = Nst_realloc_c(
        SEQ(vect)->objs,
        new_size,
        Nst_Obj *,
        size);

    if (new_objs == NULL)
        return false;

    for (usize i = len; i < new_size; i++)
        new_objs[i] = NULL;

    SEQ(vect)->cap = new_size;
    SEQ(vect)->objs = new_objs;
    return true;
}

bool Nst_vector_append(Nst_Obj *vect, Nst_Obj *val)
{
    assert_vector(vect);
    if (!resize_vector(vect))
        return false;

    SEQ(vect)->objs[SEQ(vect)->len++] = Nst_inc_ref(val);

    return true;
}

bool Nst_vector_remove(Nst_Obj *vect, Nst_Obj *val)
{
    assert_vector(vect);
    usize i = 0;
    usize n = SEQ(vect)->len;
    Nst_Obj **objs = SEQ(vect)->objs;

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
        SEQ(vect)->objs[i - 1] = objs[i];

    SEQ(vect)->len--;
    resize_vector(vect);

    return true;
}

Nst_ObjRef *Nst_vector_pop(Nst_Obj *vect, usize quantity)
{
    assert_vector(vect);
    if (SEQ(vect)->len == 0)
        return Nst_null_ref();

    if (quantity > SEQ(vect)->len)
        quantity = SEQ(vect)->len;

    usize vec_len = SEQ(vect)->len;
    for (usize i = 0; i + 1 < quantity; i++) {
        Nst_dec_ref(SEQ(vect)->objs[vec_len - i - 1]);
    }

    Nst_Obj *last_obj = SEQ(vect)->objs[vec_len - quantity];
    SEQ(vect)->len -= quantity;

    resize_vector(vect);
    return last_obj;
}
