#include <string.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include "nest.h"

#ifdef Nst_MSVC

#include <windows.h>
#define dlsym GetProcAddress
#define dlopen LoadLibraryA
#define dlclose FreeLibrary
typedef HMODULE lib_t;

#else

#include <dlfcn.h>
typedef void * lib_t;
#define dlopen(lib) dlopen(lib, RTLD_LAZY)

#endif // !__cplusplus
#include "mem.h"
#define MAX_INT_CHAR_COUNT 21

// the length of +9_123_456_789_012_345.0, which is one more than
// +1.123456789012345e+308
#define MAX_REAL_CHAR_COUNT 25
#define MAX_BYTE_CHAR_COUNT 5
#define REAL_EPSILON 9.9e-15
#define REAL_PRECISION 16

#define IS_NUM(obj)                                                           \
    (obj->type == Nst_t.Int                                                   \
     || obj->type == Nst_t.Real                                               \
     || obj->type == Nst_t.Byte)
#define IS_INT(obj) (obj->type == Nst_t.Int || obj->type == Nst_t.Byte)
#define IS_SEQ(obj) (obj->type == Nst_t.Array || obj->type == Nst_t.Vector)
#define ARE_TYPE(type_obj) (ob1->type == type_obj && ob2->type == type_obj)

#define RETURN_STACK_OP_TYPE_ERROR(operand) do {                              \
    Nst_error_setf_type(                                                      \
        "invalid types '%s' and '%s' for '" operand "'",                      \
        Nst_type_name(ob1->type).value, Nst_type_name(ob2->type).value);      \
    return NULL;                                                              \
    } while (0)

#define RETURN_CAST_TYPE_ERROR(t) do {                                        \
    Nst_error_setf_type(                                                      \
        "invalid type cast from '%s' to '%s'",                                \
        Nst_type_name(ob->type).value, Nst_type_name(t).value);               \
    return NULL;                                                              \
    } while (0)

#define RETURN_LOCAL_OP_TYPE_ERROR(operand) do {                              \
    Nst_error_setf_type(                                                      \
        "invalid type '%s' for '" operand "'",                                \
        Nst_type_name(ob->type).value);                                       \
    return NULL;                                                              \
    } while (0)

#define CHECK_BUFFER(buf) do {                                                \
        if (buf == NULL)                                                      \
            return NULL;                                                      \
    } while (0)

static Nst_Obj *seq_eq(Nst_Obj *seq1, Nst_Obj *seq2,
                       Nst_LList *containers);
static Nst_Obj *map_eq(Nst_Obj *map1, Nst_Obj *map2, Nst_LList *containers);
static Nst_Obj *import_nest_lib(Nst_Obj *file_path);
static Nst_Obj *import_c_lib(Nst_Obj *file_path);
static bool add_to_handle_map(Nst_Obj *path, Nst_Obj *map,
                              Nst_SourceText *src_txt);

bool Nst_obj_eq_c(Nst_Obj *ob1, Nst_Obj *ob2)
{
    Nst_Obj *result_obj = Nst_obj_eq(ob1, ob2);
    bool result = Nst_obj_to_bool(result_obj);
    Nst_dec_ref(result_obj);
    return result;
}

bool Nst_obj_ne_c(Nst_Obj *ob1, Nst_Obj *ob2)
{
    Nst_Obj *result_obj = Nst_obj_ne(ob1, ob2);
    bool result = Nst_obj_to_bool(result_obj);
    Nst_dec_ref(result_obj);
    return result;
}

// Comparisons
Nst_Obj *Nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1 == ob2)
        return Nst_true_ref();
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        Nst_RETURN_BOOL(v1 == v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);
        if (isnan(v1) || isnan(v2))
            return Nst_false_ref();
        Nst_RETURN_BOOL(fabs(v1 - v2) < REAL_EPSILON);
    } else if (ARE_TYPE(Nst_t.Str)) {
        Nst_RETURN_BOOL(Nst_str_len(ob1) == Nst_str_len(ob2) &&
            Nst_str_compare(ob1, ob2) == 0);
    } else if (ARE_TYPE(Nst_t.Bool))
        Nst_RETURN_BOOL(ob1 == ob2);
    else if (IS_SEQ(ob1) && IS_SEQ(ob2)) {
        Nst_LList *containers = Nst_llist_new();
        if (containers == NULL) {
            Nst_error_clear();
            return Nst_false_ref();
        }
        Nst_Obj *res = seq_eq(ob1, ob2, containers);
        Nst_llist_destroy(containers, NULL);
        return res;
    } else if (ARE_TYPE(Nst_t.Map)) {
        Nst_LList *containers = Nst_llist_new();
        if (containers == NULL)
            return NULL;
        Nst_Obj *res = map_eq(ob1, ob2, containers);
        Nst_llist_destroy(containers, NULL);
        return res;
    } else
        return Nst_false_ref();
}

static Nst_Obj *seq_eq(Nst_Obj *seq1, Nst_Obj *seq2,
                       Nst_LList  *containers)
{
    if (Nst_seq_len(seq1) != Nst_seq_len(seq2))
        return Nst_false_ref();

    for (Nst_LLNode *n = containers->head; n != NULL; n = n->next) {
        if (n->value == seq1 && n->next->value == seq2)
            return Nst_true_ref();
        if (n->value == seq1 || n->value == seq2)
            return Nst_false_ref();
    }

    if (!Nst_llist_append(containers, seq1, false)) {
        Nst_error_clear();
        return Nst_false_ref();
    }

    if (!Nst_llist_append(containers, seq2, false)) {
        Nst_error_clear();
        return Nst_false_ref();
    }

    Nst_Obj *ob1 = NULL;
    Nst_Obj *ob2 = NULL;
    Nst_Obj *result = NULL;
    for (usize i = 0, n = Nst_seq_len(seq1); i < n; i++) {
        ob1 = Nst_seq_getnf(seq1, i);
        ob2 = Nst_seq_getnf(seq2, i);

        if (IS_SEQ(ob1) && IS_SEQ(ob2))
            result = seq_eq(ob1, ob2, containers);
        else if (ARE_TYPE(Nst_t.Map))
            result = map_eq(ob1, ob2, containers);
        else
            result = Nst_obj_eq(ob1, ob2);

        if (result == NULL)
            return NULL;

        if (result == Nst_c.Bool_false)
            return Nst_c.Bool_false;
        else
            Nst_dec_ref(Nst_c.Bool_true);
    }

    Nst_llist_pop(containers); // pops seq1
    Nst_llist_pop(containers); // pops seq2

    return Nst_true_ref();
}

static Nst_Obj *map_eq(Nst_Obj *map1, Nst_Obj *map2,
                       Nst_LList *containers)
{
    if (Nst_map_len(map1) != Nst_map_len(map2))
        return Nst_false_ref();

    for (Nst_LLNode *n = containers->head; n != NULL; n = n->next) {
        if (n->value == map1 && n->next->value == map2)
            return Nst_true_ref();
        if (n->value == map1 || n->value == map2)
            return Nst_false_ref();
    }

    if (!Nst_llist_append(containers, map1, false)) {
        Nst_error_clear();
        return Nst_false_ref();
    }

    if (!Nst_llist_append(containers, map2, false)) {
        Nst_error_clear();
        return Nst_false_ref();
    }

    Nst_Obj *key = NULL;
    Nst_Obj *ob1 = NULL;
    Nst_Obj *ob2 = NULL;
    Nst_Obj *result = NULL;
    for (isize i = Nst_map_next(-1, map1, &key, &ob1);
         i != -1;
         i = Nst_map_next(i, map1, &key, &ob1))
    {
        ob2 = Nst_map_get(map2, key);
        if (ob2 == NULL)
            return Nst_false_ref();
        else
            Nst_dec_ref(ob2);

        if (IS_SEQ(ob1) && IS_SEQ(ob2))
            result = seq_eq(ob1, ob2, containers);
        else if (ARE_TYPE(Nst_t.Map))
            result = map_eq(ob1, ob2, containers);
        else
            result = Nst_obj_eq(ob1, ob2);

        if (result == Nst_c.Bool_false)
            return Nst_c.Bool_false;
        else if (result == NULL)
            return NULL;
        else
            Nst_dec_ref(Nst_c.Bool_true);
    }

    Nst_llist_pop(containers); // pops seq1
    Nst_llist_pop(containers); // pops seq2

    return Nst_true_ref();
}

Nst_Obj *Nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (Nst_obj_eq(ob1, ob2) == Nst_c.Bool_true) {
        Nst_dec_ref(Nst_c.Bool_true);
        return Nst_false_ref();
    } else {
        Nst_dec_ref(Nst_c.Bool_false);
        return Nst_true_ref();
    }
}

Nst_Obj *Nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Str))
        Nst_RETURN_BOOL(Nst_str_compare(ob1, ob2) > 0);
    else if (ARE_TYPE(Nst_t.Byte))
        Nst_RETURN_BOOL(Nst_byte_u8(ob1) > Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        Nst_RETURN_BOOL(v1 > v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);
        if (isnan(v1) || isnan(v2))
            return Nst_false_ref();
        Nst_RETURN_BOOL(v1 > v2 && !(fabs(v1 - v2) < REAL_EPSILON));
    } else
        RETURN_STACK_OP_TYPE_ERROR(">");
}

Nst_Obj *Nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Str))
        Nst_RETURN_BOOL(Nst_str_compare(ob1, ob2) < 0);
    else if (ARE_TYPE(Nst_t.Byte))
        Nst_RETURN_BOOL(Nst_byte_u8(ob1) < Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        Nst_RETURN_BOOL(v1 < v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);
        if (isnan(v1) || isnan(v2))
            return Nst_false_ref();
        Nst_RETURN_BOOL(v1 < v2 && !(fabs(v1 - v2) < REAL_EPSILON));
    } else
        RETURN_STACK_OP_TYPE_ERROR("<");
}

Nst_Obj *Nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2)
{
    Nst_Obj *res = Nst_obj_eq(ob1, ob2);
    if (res == NULL)
        return NULL;
    else if (res == Nst_c.Bool_true)
        return Nst_c.Bool_true;
    else
        Nst_dec_ref(Nst_c.Bool_false);

    res = Nst_obj_gt(ob1, ob2);

    if (Nst_error_occurred()) {
        Nst_error_clear();
        RETURN_STACK_OP_TYPE_ERROR(">=");
    }
    return res;
}

Nst_Obj *Nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2)
{
    Nst_Obj *res = Nst_obj_eq(ob1, ob2);
    if (res == Nst_c.Bool_true)
        return Nst_c.Bool_true;
    else if (res == NULL)
        return NULL;
    else
        Nst_dec_ref(Nst_c.Bool_false);

    res = Nst_obj_lt(ob1, ob2);

    if (Nst_error_occurred()) {
        Nst_error_clear();
        RETURN_STACK_OP_TYPE_ERROR("<=");
    }
    return res;
}

// Arithmetic operations
Nst_Obj *Nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Vector) {
        if (!Nst_vector_append(ob1, ob2))
            return NULL;
        return Nst_inc_ref(ob1);
    } else if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(Nst_byte_u8(ob1) + Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 + v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);
        return Nst_real_new(v1 + v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("+");
}

Nst_Obj *Nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Vector) {
        Nst_vector_remove(ob1, ob2);
        return Nst_inc_ref(ob1);
    } else if (ob1->type == Nst_t.Map) {
        Nst_Obj *res = Nst_map_drop(ob1, ob2);
        if (res == NULL && ob2->hash == -1) {
            Nst_error_setf_type(
                "type '%s' is not hashable",
                Nst_type_name(ob2->type).value);
            return NULL;
        }

        Nst_ndec_ref(res);
        return Nst_inc_ref(ob1);
    } else if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(Nst_byte_u8(ob1) - Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 - v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);
        return Nst_real_new(v1 - v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("-");
}

Nst_Obj *Nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Vector && ob2->type == Nst_t.Int) {
        if (Nst_int_i64(ob2) == 0)
            return Nst_vector_new(0);

        usize seq_len = Nst_seq_len(ob1);
        Nst_Obj **v_objs = _Nst_seq_objs(ob1);
        for (usize i = 0, n = Nst_int_i64(ob2) - 1; i < n; i++) {
            for (usize j = 0; j < seq_len; j++) {
                if (!Nst_vector_append(ob1, v_objs[j]))
                    return NULL;
            }
        }
        return Nst_inc_ref(ob1);
    } else if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(Nst_byte_u8(ob1) * Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 * v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);
        return Nst_real_new(v1 * v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("*");
}

Nst_Obj *Nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Vector && ob2->type == Nst_t.Int) {
        i64 count = Nst_int_i64(ob2);
        if (count < 0) {
            Nst_error_setc_value("cannot pop a negative number of items");
            return NULL;
        }
        return Nst_vector_pop(ob1, (usize)count);
    } else if (ARE_TYPE(Nst_t.Byte)) {
        if (Nst_byte_u8(ob2) == 0) {
            Nst_error_setc_math("division by zero");
            return NULL;
        }

        return Nst_byte_new(Nst_byte_u8(ob1) / Nst_byte_u8(ob2));
    } else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);

        if (v2 == 0) {
            Nst_error_setc_math("division by zero");
            return NULL;
        }

        return Nst_int_new(v1 / v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);

        if (v2 == 0.0) {
            Nst_error_setc_math("division by zero");
            return NULL;
        }

        return Nst_real_new(v1 / v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("/");
}

static u8 fast_byte_pow(u8 base, u8 exp)
{
    if (exp == 1)
        return base;
    else if (exp == 0)
        return 1;
    else if (exp % 2 == 0) {
        u8 part = fast_byte_pow(base, exp / 2);
        return part * part;
    } else {
        u8 part = fast_byte_pow(base, (exp - 1) / 2);
        return base * part * part;
    }
}

static i64 fast_int_pow(i64 base, i64 exp)
{
    if (exp == 1)
        return base;
    else if (exp == 0)
        return 1;
    else if (exp % 2 == 0) {
        i64 part = fast_int_pow(base, exp / 2);
        return part * part;
    } else {
        i64 part = fast_int_pow(base, (exp - 1) / 2);
        return base * part * part;
    }
}

Nst_Obj *Nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte)) {
        u8 base = Nst_byte_u8(ob1);
        u8 exp = Nst_byte_u8(ob2);
        return Nst_byte_new(fast_byte_pow(base, exp));
    } else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 base = Nst_number_to_i64(ob1);
        i64 exp = Nst_number_to_i64(ob2);
        if (exp < 0)
            return Nst_inc_ref(Nst_c.Int_0);
        else if (exp == 0)
            return Nst_inc_ref(Nst_c.Int_1);
        return Nst_int_new(fast_int_pow(base, exp));
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);

        // any root of a negative number gives -nan as a result
        if (v1 < 0 && floorl(v2) != v2) {
            Nst_error_setc_math("fractional power of a negative number");
            return NULL;
        }

        return Nst_real_new((f64)powl(v1, v2));
    } else
        RETURN_STACK_OP_TYPE_ERROR("^");
}

Nst_Obj *Nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte)) {
        if (Nst_byte_u8(ob2) == 0) {
            Nst_error_setc_math("modulo by zero");
            return NULL;
        }

        return Nst_byte_new(Nst_byte_u8(ob1) % Nst_byte_u8(ob2));
    } else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);

        if (v2 == 0) {
            Nst_error_setc_math("modulo by zero");
            return NULL;
        }

        return Nst_int_new(v1 % v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);

        if (v2 == 0.0) {
            Nst_error_setc_math("modulo by zero");
            return NULL;
        }

        return Nst_real_new(fmod(v1, v2));
    } else
        RETURN_STACK_OP_TYPE_ERROR("%%");
}

// Bitwise operations
Nst_Obj *Nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(Nst_byte_u8(ob1) | Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 | v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("|");
}

Nst_Obj *Nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(Nst_byte_u8(ob1) & Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 & v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("&");
}

Nst_Obj *Nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(Nst_byte_u8(ob1) ^ Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 ^ v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("^^");
}

Nst_Obj *Nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(Nst_byte_u8(ob1) << Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 << v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("<<");
}

Nst_Obj *Nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(Nst_byte_u8(ob1) >> Nst_byte_u8(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 >> v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR(">>");
}

// Logical operations
Nst_Obj *Nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (Nst_obj_to_bool(ob1))
        return Nst_inc_ref(ob1);
    else
        return Nst_inc_ref(ob2);
}

Nst_Obj *Nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (!Nst_obj_to_bool(ob1))
        return Nst_inc_ref(ob1);
    else
        return Nst_inc_ref(ob2);
}

Nst_Obj *Nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2)
{
    bool v1 = Nst_obj_to_bool(ob1);
    bool v2 = Nst_obj_to_bool(ob2);

    Nst_RETURN_BOOL((v1 && !v2) || (!v1  && v2));
}

// Other

Nst_Obj *_Nst_repr_str_cast(Nst_Obj *ob)
{
    Nst_Obj *ob_t = ob->type;

    if (ob_t == Nst_t.Str)
        return Nst_str_repr(ob);
    else if (ob_t == Nst_t.Byte) {
        char *str = Nst_calloc_c(5, char, NULL);
        CHECK_BUFFER(str);

        sprintf(str, "%ib", (int)Nst_byte_u8(ob));
        return Nst_str_new_c_raw((const char *)str, true);
    } else
        return Nst_obj_cast(ob, Nst_t.Str);
}

Nst_Obj *_Nst_obj_str_cast_map(Nst_Obj *map_obj, Nst_LList *all_objs);

Nst_Obj *_Nst_obj_str_cast_seq(Nst_Obj *seq_obj, Nst_LList *all_objs)
{
    bool is_vect = seq_obj->type == Nst_t.Vector;
    const char *recursive;
    const char *empty;
    const char *open;
    const char *close;

    if (is_vect) {
        recursive = "<{.}>";
        empty = "<{}>";
        open = "<{";
        close = "}>";
    } else {
        recursive = "{.}";
        empty = "{,}";
        open = "{";
        close = "}";
    }

    for (Nst_LLNode *n = all_objs->head; n != NULL; n = n->next) {
        if (seq_obj == n->value)
            return Nst_str_new_c_raw(recursive, false);
    }

    if (Nst_seq_len(seq_obj) == 0)
        return Nst_str_new_c_raw(empty, false);

    if (!Nst_llist_push(all_objs, seq_obj, false))
        return NULL;

    usize seq_len = Nst_seq_len(seq_obj);

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, 6))
        return NULL;

    Nst_sb_push_c(&sb, open);

    for (usize i = 0; i < seq_len; i++) {
        Nst_Obj *ob = Nst_seq_getnf(seq_obj, i);
        Nst_Obj *ob_str;

        if (IS_SEQ(ob))
            ob_str = _Nst_obj_str_cast_seq(ob, all_objs);
        else if (ob->type == Nst_t.Map)
            ob_str = _Nst_obj_str_cast_map(ob, all_objs);
        else
            ob_str = _Nst_repr_str_cast(ob);
        if (ob_str == NULL) {
            Nst_sb_destroy(&sb);
            return NULL;
        }

        if (!Nst_sb_push_str(&sb, ob_str)) {
            Nst_sb_destroy(&sb);
            return NULL;
        }

        Nst_dec_ref(ob_str);

        if (i == seq_len - 1)
            break;

        if (!Nst_sb_push_c(&sb, ", ")) {
            Nst_sb_destroy(&sb);
            return NULL;
        }
    }

    if (!Nst_sb_push_c(&sb, close)) {
        Nst_sb_destroy(&sb);
        return NULL;
    }

    Nst_llist_pop(all_objs);
    return Nst_str_from_sb(&sb);
}

Nst_Obj *_Nst_obj_str_cast_map(Nst_Obj *map_obj, Nst_LList *all_objs)
{
    for (Nst_LLNode *n = all_objs->head; n != NULL; n = n->next) {
        if (map_obj == n->value)
            return Nst_str_new_c("{.}", 3, false);
    }

    if (Nst_map_len(map_obj) == 0)
        return Nst_str_new_c("{}", 2, false);

    if (!Nst_llist_push(all_objs, map_obj, false))
        return NULL;

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, 7))
        return NULL;
    Nst_sb_push_char(&sb, '{');

    Nst_Obj *key;
    Nst_Obj *val;
    for (isize idx = Nst_map_next(-1, map_obj, &key, &val);
         idx != -1;
         idx = Nst_map_next(idx, map_obj, &key, &val))
    {
        // Key cannot be a vector, an array or a map
        Nst_Obj *key_str = _Nst_repr_str_cast(key);
        Nst_Obj *val_str;

        if (key_str == NULL) {
            Nst_sb_destroy(&sb);
            return NULL;
        }

        if (IS_SEQ(val))
            val_str = _Nst_obj_str_cast_seq(val, all_objs);
        else if (val->type == Nst_t.Map)
            val_str = _Nst_obj_str_cast_map(val, all_objs);
        else
            val_str = _Nst_repr_str_cast(val);

        if (key_str == NULL) {
            Nst_dec_ref(key_str);
            Nst_sb_destroy(&sb);
            return NULL;
        }

        usize expantion_amount = Nst_str_len(key_str)
                               + Nst_str_len(val_str)
                               + 4;
        if (!Nst_sb_reserve(&sb, expantion_amount)) {
            Nst_sb_destroy(&sb);
            return NULL;
        }

        Nst_sb_push_str(&sb, key_str);
        Nst_sb_push_c(&sb, ": ");
        Nst_sb_push_str(&sb, val_str);
        Nst_sb_push_c(&sb, ", ");

        Nst_dec_ref(key_str);
        Nst_dec_ref(val_str);
    }

    sb.len -= 2;
    Nst_sb_push_char(&sb, '}');
    Nst_llist_pop(all_objs);

    return Nst_str_from_sb(&sb);
}

#ifndef Nst_MSVC
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

static Nst_Obj *obj_to_str(Nst_Obj *ob)
{
    Nst_Obj *ob_t = ob->type;

    if (ob_t == Nst_t.Int) {
        usize str_len;
        u8 *str = Nst_fmt("{L}", 3, &str_len, Nst_int_i64(ob));
        CHECK_BUFFER(str);
        return Nst_str_new_allocated(str, str_len);
    } else if (ob_t == Nst_t.Real) {
        usize str_len;
        u8 *str = Nst_fmt(
            "{f:Gp.*}", 8,
            &str_len,
            Nst_real_f64(ob),
            REAL_PRECISION);
        CHECK_BUFFER(str);
        return Nst_str_new_allocated(str, str_len);
    } else if (ob_t == Nst_t.Bool) {
        if (ob == Nst_c.Bool_true)
            return Nst_inc_ref(Nst_s.c_true);
        else
            return Nst_inc_ref(Nst_s.c_false);
    } else if (ob_t == Nst_t.Type) {
        return Nst_str_from_sv(Nst_type_name(ob));
    } else if (ob_t == Nst_t.Byte) {
        u8 value = Nst_byte_u8(ob);
        u8 *str = Nst_calloc_c(3, u8, NULL);
        CHECK_BUFFER(str);
        if (value <= 0x7f) {
            str[0] = value;
            return Nst_str_new_allocated(str, 1);
        }
        str[0] = 0b11000000 | (value >> 6);
        str[1] = 0b10000000 | (value & 0x3f);
        return Nst_str_new_allocated(str, 2);
    } else if (ob_t == Nst_t.Array || ob_t == Nst_t.Vector) {
        Nst_LList *all_objs = Nst_llist_new();
        if (all_objs == NULL)
            return NULL;
        Nst_Obj *str = _Nst_obj_str_cast_seq(ob, all_objs);
        Nst_llist_destroy(all_objs, NULL);
        return str;
    } else if (ob_t == Nst_t.Map) {
        Nst_LList *all_objs = Nst_llist_new();
        if (all_objs == NULL)
            return NULL;
        Nst_Obj *str = _Nst_obj_str_cast_map(ob, all_objs);
        Nst_llist_destroy(all_objs, NULL);
        return str;
    } else if (ob_t == Nst_t.Null)
        return Nst_inc_ref(Nst_s.c_null);
    else if (ob_t == Nst_t.IOFile) {
        const char *empty_val = "<IOFile[-----]>";
        u8 *buffer = (u8 *)Nst_calloc(
            1, sizeof(u8) * strlen(empty_val),
            (void *)empty_val);
        CHECK_BUFFER(buffer);
        if (Nst_IOF_CAN_READ(ob))
            buffer[8] = 'r';
        if (Nst_IOF_CAN_WRITE(ob))
            buffer[9] = 'w';
        if (Nst_IOF_IS_BIN(ob))
            buffer[10]= 'b';
        if (Nst_IOF_CAN_SEEK(ob))
            buffer[11]= 's';
        if (Nst_IOF_IS_TTY(ob))
            buffer[12]= 't';
        return Nst_str_new_allocated(buffer, strlen(empty_val));
    } else if (ob_t == Nst_t.Func) {
        usize str_len;
        usize arg_num = Nst_func_arg_num(ob);
        u8 *str = Nst_fmt(
            "<Func {z} arg{s}>", 17, &str_len,
            arg_num, arg_num == 1 ? "" : "s");
        CHECK_BUFFER(str);
        return Nst_str_new_allocated(str, str_len);
    } else {
        usize str_len;
        u8 *str = Nst_fmt(
            "<{s} object at {p:0X}>", 22, &str_len,
            Nst_type_name(ob->type).value, ob);
        CHECK_BUFFER(str);
        return Nst_str_new_allocated(str, str_len);
    }
}

static Nst_Obj *obj_to_bool(Nst_Obj *ob)
{
    Nst_Obj *ob_t = ob->type;
    if (ob_t == Nst_t.Int)
        Nst_RETURN_BOOL(Nst_int_i64(ob) != 0);
    else if (ob_t == Nst_t.Real)
        Nst_RETURN_BOOL(Nst_real_f64(ob) != 0.0);
    else if (ob_t == Nst_t.Str)
        Nst_RETURN_BOOL(Nst_str_len(ob) != 0);
    else if (ob_t == Nst_t.Map)
        Nst_RETURN_BOOL(Nst_map_len(ob) != 0);
    else if (ob_t == Nst_t.Array || ob_t == Nst_t.Vector)
        Nst_RETURN_BOOL(Nst_seq_len(ob) != 0);
    else if (ob_t == Nst_t.Null)
        return Nst_false_ref();
    else if (ob_t == Nst_t.Byte)
        Nst_RETURN_BOOL(Nst_byte_u8(ob) != 0);
    else if (ob_t == Nst_t.IOFile)
        Nst_RETURN_BOOL(!Nst_IOF_IS_CLOSED(ob));
    else
        return Nst_true_ref();
}

static Nst_Obj *obj_to_byte(Nst_Obj *ob)
{
    Nst_Obj *ob_t = ob->type;

    if (ob_t == Nst_t.Real) {
        f64 val = Nst_real_f64(ob);
        if (isnan(val)) {
            Nst_error_setc_value("cannot cast a NaN to a Byte");
            return NULL;
        } else if (isinf(val)) {
            Nst_error_setc_value("cannot cast an infinity to a Byte");
            return NULL;
        }
        return Nst_byte_new((i64)val & 0xff);
    }
    else if (ob_t == Nst_t.Int)
        return Nst_byte_new(Nst_int_i64(ob) & 0xff);
    else if (ob_t == Nst_t.Str)
        return Nst_str_parse_byte(ob);
    RETURN_CAST_TYPE_ERROR(Nst_t.Byte);
}

static Nst_Obj *obj_to_int(Nst_Obj *ob)
{
    Nst_Obj *ob_t = ob->type;

    if (ob_t == Nst_t.Real) {
        f64 val = Nst_real_f64(ob);
        if (isnan(val)) {
            Nst_error_setc_value("cannot cast a NaN to an Int");
            return NULL;
        } else if (isinf(val)) {
            Nst_error_setc_value("cannot cast an infinity to an Int");
            return NULL;
        }
        return Nst_int_new((i64)val);
    } else if (ob_t == Nst_t.Byte)
        return Nst_int_new((i64)Nst_byte_u8(ob));
    else if (ob_t == Nst_t.Str)
        return Nst_str_parse_int(ob, 0);
    RETURN_CAST_TYPE_ERROR(Nst_t.Int);
}

static Nst_Obj *obj_to_real(Nst_Obj *ob)
{
    Nst_Obj *ob_t = ob->type;

    if (ob_t == Nst_t.Int)
        return Nst_real_new((f64)Nst_int_i64(ob));
    else if (ob_t == Nst_t.Byte)
        return Nst_real_new((f64)Nst_byte_u8(ob));
    else if (ob_t == Nst_t.Str)
        return Nst_str_parse_real(ob);
    RETURN_CAST_TYPE_ERROR(Nst_t.Real);
}

static Nst_Obj *seq_to_seq(Nst_Obj *ob, bool is_vect)
{
    usize seq_len = Nst_seq_len(ob);
    Nst_Obj *seq = is_vect ? Nst_vector_new(seq_len)
                           : Nst_array_new(seq_len);
    if (seq == NULL)
        return NULL;
    Nst_Obj **objs = _Nst_seq_objs(ob);
    for (usize i = 0; i < seq_len; i++)
        Nst_seq_setf(seq, i, objs[i]);

    return seq;
}

static Nst_Obj *str_to_seq(Nst_Obj *ob, bool is_vect)
{
    usize str_len = Nst_str_char_len(ob);
    Nst_Obj *seq = is_vect ? Nst_vector_new(str_len)
                           : Nst_array_new(str_len);
    if (seq == NULL)
        return NULL;

    isize str_idx = -1;
    isize i = 0;

    for (Nst_Obj *ch = Nst_str_next_obj(ob, &str_idx);
         ch != NULL;
         ch = Nst_str_next_obj(ob, &str_idx))
    {
        Nst_seq_setnf(seq, i++, ch);
    }

    if (str_idx == Nst_STR_LOOP_ERROR) {
        Nst_dec_ref(seq);
        return NULL;
    }

    return seq;
}

static Nst_Obj *iter_to_seq(Nst_Obj *ob, bool is_vect)
{
    Nst_Obj *seq = Nst_vector_new(0);
    if (seq == NULL)
        return NULL;

    if (!Nst_iter_start(ob)) {
        Nst_dec_ref(seq);
        return NULL;
    }

    while (true) {
        Nst_Obj *result = Nst_iter_next(ob);
        if (result == NULL) {
            Nst_dec_ref(seq);
            return NULL;
        } else if (result == Nst_c.IEnd_iend) {
            Nst_dec_ref(result);
            break;
        }
        if (!Nst_vector_append(seq, result)) {
            Nst_dec_ref(seq);
            return NULL;
        }
        Nst_dec_ref(result);
    }

    if (is_vect)
        return seq;

    seq->type = Nst_inc_ref(Nst_t.Array);
    Nst_dec_ref(Nst_t.Vector);
    return seq;
}

static Nst_Obj *map_to_seq(Nst_Obj *ob, bool is_vect)
{
    usize seq_len = Nst_map_len(ob);
    Nst_Obj *seq = is_vect ? Nst_vector_new(seq_len)
                           : Nst_array_new(seq_len);

    usize seq_i = 0;
    Nst_Obj *key;
    Nst_Obj *val;
    for (isize i = Nst_map_next(-1, ob, &key, &val);
         i != -1;
         i = Nst_map_next(i, ob, &key, &val))
    {
        Nst_Obj *node_arr = Nst_array_create_c("OO", key, val);
        if (node_arr == NULL) {
            Nst_dec_ref(seq);
            return NULL;
        }
        Nst_seq_setnf(seq, seq_i++, node_arr);
    }

    return seq;
}

static Nst_Obj *obj_to_seq(Nst_Obj *ob, bool is_vect)
{
    Nst_Obj *ob_t = ob->type;

    if (ob_t == Nst_t.Array || ob_t == Nst_t.Vector)
        return seq_to_seq(ob, is_vect);
    else if (ob_t == Nst_t.Str)
        return str_to_seq(ob, is_vect);
    else if (ob_t == Nst_t.Iter)
        return iter_to_seq(ob, is_vect);
    else if (ob_t == Nst_t.Map)
        return map_to_seq(ob, is_vect);
    RETURN_CAST_TYPE_ERROR(is_vect ? Nst_t.Vector : Nst_t.Array);
}

static Nst_Obj *obj_to_iter(Nst_Obj *ob)
{
    Nst_Obj *ob_t = ob->type;

    if (ob_t == Nst_t.Str)
        return Nst_iter_str_new(ob);
    else if (ob_t == Nst_t.Array || ob_t == Nst_t.Vector)
        return Nst_iter_seq_new(ob);
    else if (ob_t == Nst_t.Map)
        return Nst_iter_map_new(ob);
    else
        RETURN_CAST_TYPE_ERROR(Nst_t.Iter);
}

static Nst_Obj *seq_to_map(Nst_Obj *ob)
{
    Nst_Obj **objs = _Nst_seq_objs(ob);
    Nst_Obj *map = Nst_map_new();
    if (map == NULL)
        return NULL;

    for (usize i = 0, n = Nst_seq_len(ob); i < n; i++) {
        if (objs[i]->type != Nst_t.Array && objs[i]->type != Nst_t.Vector) {
            Nst_error_setf_type(
                "expected each element to be a Vector or an Array, found a "
                "'%s' instead at index %zi",
                Nst_type_name(objs[i]->type).value, i);
            Nst_dec_ref(map);
            return NULL;
        }

        if (Nst_seq_len(objs[i]) != 2) {
            Nst_error_setf_type(
                "expected a sequence of length 2, found one of length %zi "
                "instead at index %zi",
                Nst_seq_len(objs[i]), i);
            Nst_dec_ref(map);
            return NULL;
        }

        Nst_Obj *key = Nst_seq_getnf(objs[i], 0);
        Nst_Obj *val = Nst_seq_getnf(objs[i], 1);

        i32 hash = Nst_obj_hash(key);

        if (hash == -1) {
            Nst_error_setf_type("non-hashable object found at index %zi", i);
            Nst_dec_ref(map);
            return NULL;
        }

        if (!Nst_map_set(map, key, val)) {
            Nst_dec_ref(map);
            return NULL;
        }
    }

    return map;
}

static Nst_Obj *iter_to_map(Nst_Obj *ob)
{
    Nst_Obj *map = Nst_map_new();
    if (map == NULL)
        return NULL;

    if (!Nst_iter_start(ob)) {
        Nst_dec_ref(map);
        return NULL;
    }

    usize iter_count = 1;

    while (true) {
        Nst_Obj *result = Nst_iter_next(ob);
        if (result == NULL) {
            Nst_dec_ref(map);
            return NULL;
        } else if (result == Nst_c.IEnd_iend) {
            Nst_dec_ref(result);
            break;
        }

        if (result->type != Nst_t.Array && result->type != Nst_t.Vector) {
            Nst_error_setf_type(
                "expected each element to be a Vector or an Array, found a "
                "'%s' instead at iteration %zi",
                Nst_type_name(result->type).value, iter_count);
            Nst_dec_ref(map);
            return NULL;
        }

        if (Nst_seq_len(result) != 2) {
            Nst_error_setf_type(
                "expected a sequence of length 2, found one of length %zi "
                "instead at iteration %zi",
                Nst_seq_len(result), iter_count);
            Nst_dec_ref(map);
            return NULL;
        }

        Nst_Obj *key = Nst_seq_getnf(result, 0);
        Nst_Obj *val = Nst_seq_getnf(result, 1);

        i32 hash = Nst_obj_hash(key);

        if (hash == -1) {
            Nst_error_setf_type(
                "non-hashable object found at iteration %zi",
                iter_count);
            Nst_dec_ref(map);
            return NULL;
        }

        if (!Nst_map_set(map, key, val)) {
            Nst_dec_ref(map);
            return NULL;
        }
        Nst_dec_ref(result);
        iter_count++;
    }

    return map;
}

static Nst_Obj *obj_to_map(Nst_Obj *ob)
{
    Nst_Obj *ob_t = ob->type;

    if (ob_t == Nst_t.Array || ob_t == Nst_t.Vector)
        return seq_to_map(ob);
    else if (ob_t == Nst_t.Iter)
        return iter_to_map(ob);
    RETURN_CAST_TYPE_ERROR(Nst_t.Map);
}

Nst_Obj *Nst_obj_cast(Nst_Obj *ob, Nst_Obj *type)
{
    Nst_assert(type->type == Nst_t.Type);

    if (ob->type == type)
        return Nst_inc_ref(ob);

    if (type == Nst_t.Str)
        return obj_to_str(ob);
    else if (type == Nst_t.Bool)
        return obj_to_bool(ob);
    else if (type == Nst_t.Byte)
        return obj_to_byte(ob);
    else if (type == Nst_t.Int)
        return obj_to_int(ob);
    else if (type == Nst_t.Real)
        return obj_to_real(ob);
    else if (type == Nst_t.Iter)
        return obj_to_iter(ob);
    else if (type == Nst_t.Array || type == Nst_t.Vector)
        return obj_to_seq(ob, type == Nst_t.Vector);
    else if (type == Nst_t.Map)
        return obj_to_map(ob);
    RETURN_CAST_TYPE_ERROR(type);
}

Nst_Obj *Nst_obj_contains(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Array || ob1->type == Nst_t.Vector) {
        Nst_Obj **objs = _Nst_seq_objs(ob1);
        for (usize i = 0, n = Nst_seq_len(ob1); i < n; i++) {
            if (Nst_obj_eq_c(objs[i], ob2))
                return Nst_true_ref();
        }
        return Nst_false_ref();
    } else if (ob1->type == Nst_t.Map) {
        if (Nst_obj_hash(ob2) == -1)
            return Nst_false_ref();

        Nst_Obj *item = Nst_map_get(ob1, ob2);
        if (item == NULL)
            return Nst_false_ref();
        else {
            Nst_dec_ref(item);
            return Nst_true_ref();
        }
    } else if (ob1->type == Nst_t.Str && ob2->type == Nst_t.Str) {
        isize idx = Nst_sv_lfind(Nst_sv_from_str(ob1), Nst_sv_from_str(ob2));
        Nst_RETURN_BOOL(idx != -1);
    } else
        RETURN_STACK_OP_TYPE_ERROR("<.>");
}

Nst_Obj *Nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2)
{
    ob1 = Nst_obj_cast(ob1, Nst_t.Str);
    ob2 = Nst_obj_cast(ob2, Nst_t.Str);

    u8 *s1 = Nst_str_value(ob1);
    u8 *s2 = Nst_str_value(ob2);
    usize len1 = Nst_str_len(ob1);
    usize len2 = Nst_str_len(ob2);
    usize tot_len = len1 + len2;

    u8 *buffer = Nst_malloc_c(tot_len + 1, u8);

    CHECK_BUFFER(buffer);

    memcpy(buffer, s1, len1);
    memcpy(buffer + len1, s2, len2);
    buffer[tot_len] = '\0';

    Nst_Obj *new_obj = Nst_str_new_len(
        buffer,
        tot_len,
        Nst_str_char_len(ob1) + Nst_str_char_len(ob2),
        true);

    if (new_obj == NULL)
        Nst_free(buffer);

    Nst_dec_ref(ob1);
    Nst_dec_ref(ob2);

    return new_obj;
}

Nst_Obj *Nst_obj_range(Nst_Obj *start, Nst_Obj *stop, Nst_Obj *step)
{
    if (start->type != Nst_t.Int) {
        Nst_error_setf_type(
            "invalid type '%s' for '->'",
            Nst_type_name(start->type).value);
        return NULL;
    }

    if (stop->type != Nst_t.Int) {
        Nst_error_setf_type(
            "invalid type '%s' for '->'",
            Nst_type_name(stop->type).value);
        return NULL;
    }

    if (step->type != Nst_t.Int) {
        Nst_error_setf_type(
            "invalid type '%s' for '->'",
            Nst_type_name(step->type).value);
        return NULL;
    }

    return Nst_iter_range_new(
        Nst_int_i64(start),
        Nst_int_i64(stop),
        Nst_int_i64(step));
}

// Local operations
Nst_Obj *Nst_obj_neg(Nst_Obj *ob)
{
    if (ob->type == Nst_t.Byte)
        return Nst_byte_new(-Nst_byte_u8(ob));
    else if (ob->type == Nst_t.Int)
        return Nst_int_new(-Nst_int_i64(ob));
    else if (ob->type == Nst_t.Real)
        return Nst_real_new(-Nst_real_f64(ob));
    else
        RETURN_LOCAL_OP_TYPE_ERROR("-:");
}

Nst_Obj *Nst_obj_len(Nst_Obj *ob)
{
    if (ob->type == Nst_t.Str)
        return Nst_int_new(Nst_str_char_len(ob));
    else if (ob->type == Nst_t.Map)
        return Nst_int_new(Nst_map_len(ob));
    else if (IS_SEQ(ob))
        return Nst_int_new(Nst_seq_len(ob));
    else if (ob->type == Nst_t.Func)
        return Nst_int_new((i64)Nst_func_arg_num(ob));
    else
        RETURN_LOCAL_OP_TYPE_ERROR("$");
}

Nst_Obj *Nst_obj_bwnot(Nst_Obj *ob)
{
    if (ob->type == Nst_t.Byte)
        return Nst_byte_new(~Nst_byte_u8(ob));
    else if (ob->type == Nst_t.Int)
        return Nst_int_new(~Nst_int_i64(ob));
    else
        RETURN_LOCAL_OP_TYPE_ERROR("~");
}

Nst_Obj *Nst_obj_lgnot(Nst_Obj *ob)
{
    if (Nst_obj_to_bool(ob))
        return Nst_false_ref();
    else
        return Nst_true_ref();
}

Nst_Obj *Nst_obj_stdout(Nst_Obj *ob)
{
    if (Nst_IOF_IS_CLOSED(Nst_io.out))
        return Nst_inc_ref(ob);

    Nst_Obj *str = Nst_obj_cast(ob, Nst_t.Str);

    Nst_IOResult result = Nst_fwrite(
        Nst_str_value(str),
        Nst_str_len(str),
        NULL,
        Nst_io.out);

    Nst_dec_ref(str);

    if (result != Nst_IO_SUCCESS) {
        switch (result) {
        case Nst_IO_CLOSED:
            Nst_error_setc_value("the file '@@io._get_stdout' was closed");
            break;
        case Nst_IO_INVALID_DECODING: {
            u32 ch;
            const char *name;
            Nst_io_result_get_details(&ch, NULL, &name);
            Nst_error_setf_value(
                "could not decode code point U+%06X for %s encoding",
                (int)ch, name);
            break;
        }
        case Nst_IO_INVALID_ENCODING: {
            u32 ch;
            const char *name;
            Nst_io_result_get_details(&ch, NULL, &name);
            Nst_error_setf_value(
                "could not encode byte %ib for %s encoding",
                (u8)ch, name);
            break;
        }
        case Nst_IO_OP_FAILED:
        case Nst_IO_ERROR:
#ifdef _DEBUG
            Nst_error_setc_value("failed to write to the file");
            break;
        default:
            Nst_assert(false);
#else
        default:
            Nst_error_setc_value("failed to write to the file");
#endif
        }
        return NULL;
    }

    return Nst_inc_ref(ob);
}

static inline u8 get_one_char(u8 *ch)
{
    ch[0] = 0; ch[1] = 0; ch[2] = 0; ch[3] = 0; ch[4] = 0;

    Nst_IOResult result = Nst_fread(ch, 5, 1, NULL, Nst_io.in);
    if (result == Nst_IO_SUCCESS)
        return true;
    else
        return false;
}

Nst_Obj *Nst_obj_stdin(Nst_Obj *ob)
{
    if (Nst_IOF_IS_CLOSED(Nst_io.in))
        return Nst_str_new_c("", 0, false);

    ob = Nst_obj_cast(ob, Nst_t.Str);
    Nst_fwrite(Nst_str_value(ob), Nst_str_len(ob), NULL, Nst_io.out);
    Nst_fflush(Nst_io.out);
    Nst_dec_ref(ob);

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, 4))
        return NULL;

    u8 ch[5];
    while (get_one_char(ch)) {
        if (ch[0] == '\r')
            continue;
        if (ch[0] == '\n')
            break;

        if (!Nst_sb_push_c(&sb, (const char *)ch)) {
            Nst_sb_destroy(&sb);
            return NULL;
        }
    }

    return Nst_str_from_sb(&sb);
}

Nst_Obj *Nst_obj_typeof(Nst_Obj *ob)
{
    return Nst_inc_ref(ob->type);
}

Nst_Obj *Nst_obj_import(Nst_Obj *ob)
{
    if (ob->type != Nst_t.Str) {
        Nst_error_setf_type(
            "expected type 'Str', got '%s' instead",
            Nst_type_name(ob->type).value
        );
        return NULL;
    }

    u8 *file_name = Nst_str_value(ob);
    usize file_name_len = Nst_str_len(ob);
    bool c_import = false;

    if (Nst_str_len(ob) > 6
        && file_name[0] == '_' && file_name[1] == '_'
        && file_name[2] == 'C' && file_name[3] == '_'
        && file_name[4] == '_' && file_name[5] == ':')
    {
        c_import = true;
        file_name += 6; // skip __C__:
        file_name_len -= 6;
    }

    Nst_Obj *import_path = _Nst_get_import_path(
        (const char *)file_name,
        file_name_len);
    if (import_path == NULL) {
        Nst_error_setf_value(
            "file '%.4096s' not found",
            file_name
        );
        return NULL;
    }

    // Check if the module is in the import stack
    for (Nst_LLNode *n = Nst_state.lib_paths->head; n != NULL; n = n->next) {
        if (Nst_str_compare(import_path, n->value) == 0) {
            Nst_dec_ref(import_path);
            Nst_error_setc_import("circular import");
            return NULL;
        }
    }

    Nst_Obj *obj_map = Nst_map_get(Nst_state.lib_handles, import_path);
    if (obj_map != NULL) {
        Nst_dec_ref(import_path);
        return obj_map;
    }

    if (!Nst_llist_push(Nst_state.lib_paths, import_path, false)) {
        Nst_dec_ref(import_path);
        return NULL;
    }

    if (!c_import)
        return import_nest_lib(import_path);
    else
        return import_c_lib(import_path);
}

static bool add_to_handle_map(Nst_Obj *path, Nst_Obj *map,
                              Nst_SourceText *src_txt)
{
    if (!Nst_llist_push(Nst_state.lib_srcs, src_txt, true)) {
        Nst_dec_ref(path);
        return false;
    }
    bool res = Nst_map_set(Nst_state.lib_handles, path, map);
    Nst_dec_ref(path);
    return res;
}

static Nst_Obj *import_nest_lib(Nst_Obj *file_path)
{
    Nst_SourceText *lib_src = Nst_malloc_c(1, Nst_SourceText);
    Nst_Obj *map = NULL;

    if (lib_src == NULL)
        goto cleanup;

    Nst_source_text_init(lib_src);
    if (!Nst_run_module((const char *)Nst_str_value(file_path), lib_src)) {
        Nst_llist_push(Nst_state.lib_srcs, lib_src, true);
        goto cleanup;
    }

    map = Nst_vstack_pop(&Nst_state.es->v_stack);
    Nst_assert(map != NULL);

    if (!add_to_handle_map(file_path, map, lib_src)) {
        Nst_source_text_destroy(lib_src);
        Nst_free(lib_src);
        goto cleanup;
    }
    Nst_llist_pop(Nst_state.lib_paths);
    return map;

cleanup:
    Nst_llist_pop(Nst_state.lib_paths);
    Nst_dec_ref(file_path);
    Nst_ndec_ref(map);
    return NULL;
}

static Nst_Obj *import_c_lib(Nst_Obj *file_path)
{
    void (*lib_quit_func)();
    lib_t lib = dlopen((const char *)Nst_str_value(file_path));

    if (!lib) {
        Nst_llist_pop(Nst_state.lib_paths);
        Nst_dec_ref(file_path);
#ifdef Nst_MSVC
        Nst_error_setc_import("the file is not a valid DLL");
#else
        Nst_error_setc_import(dlerror());
#endif // !__cplusplus
        return NULL;
    }

    // Initialize library
    Nst_Declr *(*lib_init)() = (Nst_Declr *(*)())dlsym(lib, "lib_init");
    if (lib_init == NULL) {
        Nst_llist_pop(Nst_state.lib_paths);
        Nst_dec_ref(file_path);
        Nst_error_setc_import(
            "the library does not specify a 'lib_init' function");
        dlclose(lib);
        return NULL;
    }

    Nst_Declr *obj_ptrs = lib_init();

    if (obj_ptrs == NULL) {
        Nst_llist_pop(Nst_state.lib_paths);
        Nst_dec_ref(file_path);
        dlclose(lib);
        if (!Nst_error_occurred())
            Nst_error_setc_import("the module failed to initialize");
        return NULL;
    }

    // Populate the function map
    Nst_Obj *obj_map = Nst_map_new();
    if (obj_map == NULL)
        goto fail;

    for (usize i = 0; obj_ptrs[i].ptr != NULL; i++) {
        Nst_Declr obj_declr = obj_ptrs[i];
        Nst_Obj *obj;
        if (obj_declr.arg_num >= 0) {
            obj = Nst_func_new_c(
                obj_declr.arg_num,
                (Nst_NestCallable)obj_declr.ptr);
        } else
            obj = ((Nst_ConstFunc)obj_declr.ptr)();

        if (obj == NULL) {
            Nst_dec_ref(obj_map);
            goto fail;
        }

        if (!Nst_map_set_str(obj_map, obj_declr.name, obj)) {
            Nst_dec_ref(obj_map);
            Nst_dec_ref(obj);
            goto fail;
        }

        Nst_dec_ref(obj);
    }

    if (!Nst_llist_append(Nst_state.loaded_libs, lib, true)) {
        Nst_dec_ref(obj_map);
        return NULL;
    }

    if (!add_to_handle_map(file_path, obj_map, NULL)) {
        Nst_dec_ref(obj_map);
        return NULL;
    }
    Nst_llist_pop(Nst_state.lib_paths);
    return obj_map;
fail:
    lib_quit_func = (void (*)())dlsym(lib, "lib_quit");
    if (lib_quit_func)
        lib_quit_func();
    dlclose(lib);
    return NULL;
}

static Nst_Obj *search_local_directory(const char *initial_path)
{
    char *file_path;
    usize new_len = Nst_get_full_path(initial_path, &file_path, NULL);
    if (file_path == NULL)
        return NULL;

    FILE *file = Nst_fopen_unicode(file_path, "rb");
    if (file == NULL) {
        Nst_free(file_path);
        return NULL;
    }
    fclose(file);
    return Nst_str_new_allocated((u8 *)file_path, new_len);
}

static Nst_Obj *rel_path_to_abs_path_str_if_found(u8 *file_path)
{
    FILE *file = Nst_fopen_unicode((const char *)file_path, "rb");
    if (file == NULL) {
        Nst_free(file_path);
        return NULL;
    }
    fclose(file);

    u8 *abs_path;
    usize abs_path_len = Nst_get_full_path(
        (const char *)file_path,
        (char **)&abs_path,
        NULL);
    Nst_free(file_path);

    if (abs_path == NULL)
        return NULL;
    return Nst_str_new_allocated(abs_path, abs_path_len);
}

#if defined(_DEBUG) && defined(Nst_MSVC)

static Nst_Obj *search_debug_directory(const char *initial_path,
                                       usize path_len)
{
    // little hack to get the absolute path without using it explicitly
    const char *root_path = __FILE__;
    const char *obj_ops_path_suffix = "src\\obj_ops.c";
    const char *nest_files = "libs\\_nest_files\\";
    usize root_len = strlen(root_path) - strlen(obj_ops_path_suffix);
    usize nest_files_len = strlen(nest_files);
    usize full_size = root_len + nest_files_len + path_len;

    u8 *file_path = Nst_malloc_c(full_size + 1, u8);
    if (file_path == NULL)
        return NULL;

    memcpy(file_path, root_path, root_len);
    memcpy(file_path + root_len, nest_files, nest_files_len);
    memcpy(file_path + root_len + nest_files_len, initial_path, path_len);
    file_path[full_size] = '\0';

    return rel_path_to_abs_path_str_if_found(file_path);
}

#endif

static Nst_Obj *search_stdlib_directory(const char *initial_path,
                                        usize path_len)
{
#if defined(_DEBUG) && defined(Nst_MSVC)
    return search_debug_directory(initial_path, path_len);
#else
#ifdef Nst_MSVC

    u8 *appdata = getenv("LOCALAPPDATA");
    if (appdata == NULL) {
        Nst_error_failed_alloc();
        return NULL;
    }
    usize appdata_len = strlen(appdata);
    const char *nest_files = "\\Programs\\nest\\nest_libs\\";
    usize nest_files_len = strlen(nest_files);
    usize tot_len = appdata_len + nest_files_len + path_len;

    u8 *file_path = Nst_malloc_c(tot_len + 1, u8);
    if (file_path == NULL)
        return NULL;
    sprintf(file_path, "%s%s%s", appdata, nest_files, initial_path);

#else

    const char *nest_files = "/usr/lib/nest/";
    usize nest_files_len = strlen(nest_files);
    usize tot_len = nest_files_len + path_len;

    u8 *file_path = Nst_malloc_c(tot_len + 1, u8);
    if (file_path == NULL)
        return NULL;
    sprintf((char *)file_path, "%s%s", nest_files, initial_path);

#endif // !Nst_MSVC

    return rel_path_to_abs_path_str_if_found(file_path);
#endif
}

Nst_Obj *_Nst_get_import_path(const char *initial_path, usize path_len)
{
    Nst_Obj *full_path = search_local_directory(initial_path);
    if (full_path != NULL)
        return full_path;
    else if (Nst_error_occurred())
        return NULL;

    full_path = search_stdlib_directory(initial_path, path_len);

    if (Nst_error_occurred())
        return NULL;
    else if (full_path == NULL) {
        Nst_error_setf_value("file '%.4096s' not found", initial_path);
        return NULL;
    }
    return full_path;
}
