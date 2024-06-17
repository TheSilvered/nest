#include <string.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include "obj_ops.h"
#include "iter.h"
#include "interpreter.h"
#include "lib_import.h"
#include "map.h"
#include "error.h"
#include "lib_import.h"
#include "format.h"
#include "hash.h"

#ifdef Nst_WIN

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
#define REAL_PRECISION "16"

#define IS_NUM(obj)                                                           \
    (obj->type == Nst_t.Int                                                   \
     || obj->type == Nst_t.Real                                               \
     || obj->type == Nst_t.Byte)
#define IS_INT(obj) (obj->type == Nst_t.Int || obj->type == Nst_t.Byte)
#define IS_SEQ(obj) (obj->type == Nst_t.Array || obj->type == Nst_t.Vector)
#define ARE_TYPE(type_obj) (ob1->type == type_obj && ob2->type == type_obj)

#define RETURN_STACK_OP_TYPE_ERROR(operand) do {                              \
    Nst_set_type_error(Nst_sprintf(                                           \
        "invalid types '%s' and '%s' for '" operand "'",                      \
        TYPE_NAME(ob1), TYPE_NAME(ob2)));                                     \
    return NULL;                                                              \
    } while (0)

#define RETURN_CAST_TYPE_ERROR(type) do {                                     \
    Nst_set_type_error(Nst_sprintf(                                           \
        _Nst_EM_INVALID_CASTING,                                              \
        TYPE_NAME(ob), Nst_TYPE_STR(type)->value));                           \
    return NULL;                                                              \
    } while (0)

#define RETURN_LOCAL_OP_TYPE_ERROR(operand) do {                              \
    Nst_set_type_error(Nst_sprintf(                                           \
        _Nst_EM_INVALID_OPERAND_TYPE(operand),                                \
        TYPE_NAME(ob)));                                                      \
    return NULL;                                                              \
    } while (0)

#define CHECK_BUFFER(buf) do {                                                \
        if (buf == NULL)                                                      \
            return NULL;                                                      \
    } while (0)

static Nst_Obj *seq_eq(Nst_SeqObj *seq1, Nst_SeqObj *seq2,
                       Nst_LList *containers);
static Nst_Obj *map_eq(Nst_MapObj *map1, Nst_MapObj *map2,
                       Nst_LList *containers);
static Nst_Obj *import_nest_lib(Nst_StrObj *file_path);
static Nst_Obj *import_c_lib(Nst_StrObj *file_path);
static bool add_to_handle_map(Nst_StrObj *path, Nst_MapObj *map,
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
Nst_Obj *_Nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1 == ob2)
        Nst_RETURN_TRUE;
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        Nst_RETURN_BOOL(v1 == v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);
        if (isnan(v1) || isnan(v2))
            Nst_RETURN_FALSE;
        Nst_RETURN_BOOL(fabs(v1 - v2) < REAL_EPSILON);
    } else if (ARE_TYPE(Nst_t.Str)) {
        Nst_RETURN_BOOL(STR(ob1)->len == STR(ob2)->len &&
            Nst_string_compare(STR(ob1), STR(ob2)) == 0);
    } else if (ARE_TYPE(Nst_t.Bool))
        Nst_RETURN_BOOL(ob1 == ob2);
    else if (IS_SEQ(ob1) && IS_SEQ(ob2)) {
        Nst_LList *containers = Nst_llist_new();
        if (containers == NULL) {
            Nst_error_clear();
            Nst_RETURN_FALSE;
        }
        Nst_Obj *res = seq_eq(SEQ(ob1), SEQ(ob2), containers);
        Nst_llist_destroy(containers, NULL);
        return res;
    } else if (ARE_TYPE(Nst_t.Map)) {
        Nst_LList *containers = Nst_llist_new();
        if (containers == NULL)
            return NULL;
        Nst_Obj *res = map_eq(MAP(ob1), MAP(ob2), containers);
        Nst_llist_destroy(containers, NULL);
        return res;
    } else
        Nst_RETURN_FALSE;
}

static Nst_Obj *seq_eq(Nst_SeqObj *seq1, Nst_SeqObj *seq2,
                       Nst_LList  *containers)
{
    if (seq1->len != seq2->len)
        Nst_RETURN_FALSE;

    for (Nst_LLNode *n = containers->head; n != NULL; n = n->next) {
        if (n->value == seq1 && n->next->value == seq2)
            Nst_RETURN_TRUE;
        if (n->value == seq1 || n->value == seq2)
            Nst_RETURN_FALSE;
    }

    if (!Nst_llist_append(containers, seq1, false)) {
        Nst_error_clear();
        Nst_RETURN_FALSE;
    }

    if (!Nst_llist_append(containers, seq2, false)) {
        Nst_error_clear();
        Nst_RETURN_FALSE;
    }

    Nst_Obj *ob1 = NULL;
    Nst_Obj *ob2 = NULL;
    Nst_Obj *result = NULL;
    for (usize i = 0, n = seq1->len; i < n; i++) {
        ob1 = seq1->objs[i];
        ob2 = seq2->objs[i];

        if (IS_SEQ(ob1) && IS_SEQ(ob2))
            result = seq_eq(SEQ(ob1), SEQ(ob2), containers);
        else if (ARE_TYPE(Nst_t.Map))
            result = map_eq(MAP(ob1), MAP(ob2), containers);
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

    Nst_RETURN_TRUE;
}

static Nst_Obj *map_eq(Nst_MapObj *map1, Nst_MapObj *map2,
                       Nst_LList *containers)
{
    if (map1->len != map2->len)
        Nst_RETURN_FALSE;

    for (Nst_LLNode *n = containers->head; n != NULL; n = n->next) {
        if (n->value == map1 && n->next->value == map2)
            Nst_RETURN_TRUE;
        if (n->value == map1 || n->value == map2)
            Nst_RETURN_FALSE;
    }

    if (!Nst_llist_append(containers, map1, false)) {
        Nst_error_clear();
        Nst_RETURN_FALSE;
    }

    if (!Nst_llist_append(containers, map2, false)) {
        Nst_error_clear();
        Nst_RETURN_FALSE;
    }

    Nst_Obj *key = NULL;
    Nst_Obj *ob1 = NULL;
    Nst_Obj *ob2 = NULL;
    Nst_Obj *result = NULL;
    for (i32 i = Nst_map_get_next_idx(-1, map1);
         i != -1;
         i = Nst_map_get_next_idx(i, map1))
    {
        key = map1->nodes[i].key;
        ob1 = map1->nodes[i].value;

        ob2 = _Nst_map_get(map2, key);
        if (ob2 == NULL)
            Nst_RETURN_FALSE;
        else
            Nst_dec_ref(ob2);

        if (IS_SEQ(ob1) && IS_SEQ(ob2))
            result = seq_eq(SEQ(ob1), SEQ(ob2), containers);
        else if (ARE_TYPE(Nst_t.Map))
            result = map_eq(MAP(ob1), MAP(ob2), containers);
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

    Nst_RETURN_TRUE;
}

Nst_Obj *_Nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (Nst_obj_eq(ob1, ob2) == Nst_c.Bool_true) {
        Nst_dec_ref(Nst_c.Bool_true);
        Nst_RETURN_FALSE;
    } else {
        Nst_dec_ref(Nst_c.Bool_false);
        Nst_RETURN_TRUE;
    }
}

Nst_Obj *_Nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Str))
        Nst_RETURN_BOOL(Nst_string_compare(STR(ob1), STR(ob2)) > 0);
    else if (ARE_TYPE(Nst_t.Byte))
        Nst_RETURN_BOOL(AS_BYTE(ob1) > AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        Nst_RETURN_BOOL(v1 > v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);
        if (isnan(v1) || isnan(v2))
            Nst_RETURN_FALSE;
        Nst_RETURN_BOOL(v1 > v2 && !(fabs(v1 - v2) < REAL_EPSILON));
    } else
        RETURN_STACK_OP_TYPE_ERROR(">");
}

Nst_Obj *_Nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Str))
        Nst_RETURN_BOOL(Nst_string_compare(STR(ob1), STR(ob2)) < 0);
    else if (ARE_TYPE(Nst_t.Byte))
        Nst_RETURN_BOOL(AS_BYTE(ob1) < AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        Nst_RETURN_BOOL(v1 < v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);
        if (isnan(v1) || isnan(v2))
            Nst_RETURN_FALSE;
        Nst_RETURN_BOOL(v1 < v2 && !(fabs(v1 - v2) < REAL_EPSILON));
    } else
        RETURN_STACK_OP_TYPE_ERROR("<");
}

Nst_Obj *_Nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2)
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

Nst_Obj *_Nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2)
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
Nst_Obj *_Nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Vector) {
        if (!Nst_vector_append(ob1, ob2))
            return NULL;
        return Nst_inc_ref(ob1);
    } else if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(AS_BYTE(ob1) + AS_BYTE(ob2));
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

Nst_Obj *_Nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Vector) {
        Nst_vector_remove(ob1, ob2);
        return Nst_inc_ref(ob1);
    } else if (ob1->type == Nst_t.Map) {
        Nst_Obj *res = Nst_map_drop(ob1, ob2);
        if (res == NULL && ob2->hash == -1) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_UNHASHABLE_TYPE,
                TYPE_NAME(ob2)));
            return NULL;
        }

        Nst_ndec_ref(res);
        return Nst_inc_ref(ob1);
    } else if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(AS_BYTE(ob1) - AS_BYTE(ob2));
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

Nst_Obj *_Nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Vector && ob2->type == Nst_t.Int) {
        if (AS_INT(ob2) == 0)
            return Nst_vector_new(0);

        Nst_SeqObj *vect = SEQ(ob1);
        usize v_len = vect->len;
        usize new_size = (usize)AS_INT(ob2) * v_len;
        Nst_Obj **new_objs = Nst_realloc_c(vect->objs, new_size, Nst_Obj *, 0);
        if (new_objs == NULL)
            return NULL;
        vect->objs = new_objs;
        vect->cap = new_size;
        vect->len = new_size;
        for (usize i = v_len; i < new_size; i++)
            new_objs[i] = Nst_inc_ref(new_objs[i % v_len]);

        return Nst_inc_ref(ob1);
    } else if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(AS_BYTE(ob1) * AS_BYTE(ob2));
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

Nst_Obj *_Nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Vector && ob2->type == Nst_t.Int)
        return Nst_vector_pop(ob1, (usize)AS_INT(ob2));
    else if (ARE_TYPE(Nst_t.Byte)) {
        if (AS_BYTE(ob2) == 0) {
            Nst_set_math_error_c(_Nst_EM_DIVISION_BY_ZERO);
            return NULL;
        }

        return Nst_byte_new(AS_BYTE(ob1) / AS_BYTE(ob2));
    } else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);

        if (v2 == 0) {
            Nst_set_math_error_c(_Nst_EM_DIVISION_BY_ZERO);
            return NULL;
        }

        return Nst_int_new(v1 / v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);

        if (v2 == 0.0) {
            Nst_set_math_error_c(_Nst_EM_DIVISION_BY_ZERO);
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

Nst_Obj *_Nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte)) {
        u8 base = AS_BYTE(ob1);
        u8 exp = AS_BYTE(ob2);
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
            Nst_set_math_error_c(_Nst_EM_COMPLEX_POW);
            return NULL;
        }

        return Nst_real_new((f64)powl(v1, v2));
    } else
        RETURN_STACK_OP_TYPE_ERROR("^");
}

Nst_Obj *_Nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte)) {
        if (AS_BYTE(ob2) == 0) {
            Nst_set_math_error_c(_Nst_EM_MODULO_BY_ZERO);
            return NULL;
        }

        return Nst_byte_new(AS_BYTE(ob1) % AS_BYTE(ob2));
    } else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);

        if (v2 == 0) {
            Nst_set_math_error_c(_Nst_EM_MODULO_BY_ZERO);
            return NULL;
        }

        return Nst_int_new(v1 % v2);
    } else if (IS_NUM(ob1) && IS_NUM(ob2)) {
        f64 v1 = Nst_number_to_f64(ob1);
        f64 v2 = Nst_number_to_f64(ob2);

        if (v2 == 0.0) {
            Nst_set_math_error_c(_Nst_EM_MODULO_BY_ZERO);
            return NULL;
        }

        return Nst_real_new(fmod(v1, v2));
    } else
        RETURN_STACK_OP_TYPE_ERROR("%%");
}

// Bitwise operations
Nst_Obj *_Nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(AS_BYTE(ob1) | AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 | v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("|");
}

Nst_Obj *_Nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(AS_BYTE(ob1) & AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 & v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("&");
}

Nst_Obj *_Nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(AS_BYTE(ob1) ^ AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 ^ v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("^^");
}

Nst_Obj *_Nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(AS_BYTE(ob1) << AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 << v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR("<<");
}

Nst_Obj *_Nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ARE_TYPE(Nst_t.Byte))
        return Nst_byte_new(AS_BYTE(ob1) >> AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2)) {
        i64 v1 = Nst_number_to_i64(ob1);
        i64 v2 = Nst_number_to_i64(ob2);
        return Nst_int_new(v1 >> v2);
    } else
        RETURN_STACK_OP_TYPE_ERROR(">>");
}

// Logical operations
Nst_Obj *_Nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (Nst_obj_to_bool(ob1))
        return Nst_inc_ref(ob1);
    else
        return Nst_inc_ref(ob2);
}

Nst_Obj *_Nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (!Nst_obj_to_bool(ob1))
        return Nst_inc_ref(ob1);
    else
        return Nst_inc_ref(ob2);
}

Nst_Obj *_Nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2)
{
    bool v1 = Nst_obj_to_bool(ob1);
    bool v2 = Nst_obj_to_bool(ob2);

    Nst_RETURN_BOOL((v1 && !v2) || (!v1  && v2));
}

// Other

Nst_Obj* _Nst_repr_str_cast(Nst_Obj* ob)
{
    Nst_TypeObj *ob_t = ob->type;

    if (ob_t == Nst_t.Str)
        return Nst_string_repr(ob);
    else if (ob_t == Nst_t.Byte) {
        i8 *str = Nst_calloc_c(5, i8, NULL);
        CHECK_BUFFER(str);

        sprintf(str, "%lib", (i32)AS_BYTE(ob));
        return Nst_string_new_c_raw((const i8 *)str, true);
    } else
        return Nst_obj_cast(ob, Nst_t.Str);
}

Nst_Obj *_Nst_obj_str_cast_seq(Nst_SeqObj *seq_obj, Nst_LList *all_objs)
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
            return Nst_string_new_c_raw(recursive, false);
    }

    if (seq_obj->len == 0)
        return Nst_string_new_c_raw(empty, false);

    if (!Nst_llist_push(all_objs, seq_obj, false))
        return NULL;

    usize seq_len = seq_obj->len;

    Nst_Buffer buf;
    if (!Nst_buffer_init(&buf, 6))
        return NULL;

    Nst_buffer_append_c_str(&buf, open);

    for (usize i = 0; i < seq_len; i++) {
        Nst_Obj *ob = seq_obj->objs[i];
        Nst_StrObj *ob_str;

        if (IS_SEQ(ob))
            ob_str = STR(_Nst_obj_str_cast_seq(SEQ(ob), all_objs));
        else if (ob->type == Nst_t.Map)
            ob_str = STR(_Nst_obj_str_cast_map(MAP(ob), all_objs));
        else
            ob_str = STR(_Nst_repr_str_cast(ob));
        if (ob_str == NULL) {
            Nst_buffer_destroy(&buf);
            return NULL;
        }

        if (!Nst_buffer_append(&buf, ob_str)) {
            Nst_buffer_destroy(&buf);
            return NULL;
        }

        Nst_dec_ref(ob_str);

        if (i == seq_len - 1)
            break;

        if (!Nst_buffer_append_c_str(&buf, ", ")) {
            Nst_buffer_destroy(&buf);
            return NULL;
        }
    }

    if (!Nst_buffer_append_c_str(&buf, close)) {
        Nst_buffer_destroy(&buf);
        return NULL;
    }

    Nst_llist_pop(all_objs);
    return OBJ(Nst_buffer_to_string(&buf));
}

Nst_Obj *_Nst_obj_str_cast_map(Nst_MapObj *map_obj, Nst_LList *all_objs)
{
    for (Nst_LLNode *n = all_objs->head; n != NULL; n = n->next) {
        if (map_obj == n->value)
            return Nst_string_new_c("{.}", 3, false);
    }

    if (MAP(map_obj)->len == 0)
        return Nst_string_new_c("{}", 2, false);

    if (!Nst_llist_push(all_objs, map_obj, false))
        return NULL;

    Nst_Buffer buf;
    if (!Nst_buffer_init(&buf, 7))
        return NULL;
    Nst_buffer_append_c_str(&buf, "{");

    for (i32 idx = Nst_map_get_next_idx(-1, map_obj);
         idx != -1;
         idx = Nst_map_get_next_idx(idx, map_obj))
    {
        Nst_Obj *key = map_obj->nodes[idx].key;
        Nst_Obj *val = map_obj->nodes[idx].value;

        // Key cannot be a vector, an array or a map
        Nst_StrObj *key_str = STR(_Nst_repr_str_cast(key));
        Nst_StrObj *val_str;

        if (key_str == NULL) {
            Nst_buffer_destroy(&buf);
            return NULL;
        }

        if (IS_SEQ(val))
            val_str = STR(_Nst_obj_str_cast_seq(SEQ(val), all_objs));
        else if (val->type == Nst_t.Map)
            val_str = STR(_Nst_obj_str_cast_map(MAP(val), all_objs));
        else
            val_str = STR(_Nst_repr_str_cast(val));

        if (key_str == NULL) {
            Nst_dec_ref(key_str);
            Nst_buffer_destroy(&buf);
            return NULL;
        }

        usize expantion_amount = key_str->len + val_str->len + 4;
        if (!Nst_buffer_expand_by(&buf, expantion_amount)) {
            Nst_buffer_destroy(&buf);
            return NULL;
        }

        Nst_buffer_append(&buf, key_str);
        Nst_buffer_append_c_str(&buf, ": ");
        Nst_buffer_append(&buf, val_str);
        Nst_buffer_append_c_str(&buf, ", ");

        Nst_dec_ref(key_str);
        Nst_dec_ref(val_str);
    }

    buf.len -= 2;
    Nst_buffer_append_c_str(&buf, "}");
    Nst_llist_pop(all_objs);

    return OBJ(Nst_buffer_to_string(&buf));
}

#ifndef Nst_WIN
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

static Nst_Obj *obj_to_str(Nst_Obj *ob)
{
    Nst_TypeObj *ob_t = ob->type;

    if (ob_t == Nst_t.Int) {
        i8 *buffer = Nst_malloc_c(MAX_INT_CHAR_COUNT, i8);
        CHECK_BUFFER(buffer);
        i32 len = sprintf(buffer, "%lli", AS_INT(ob));
        return Nst_string_new_allocated(buffer, len);
    } else if (ob_t == Nst_t.Real) {
        f64 val = AS_REAL(ob);
        if (isinf(val)) {
            if (*(i64 *)&val & 0x8000000000000000)
                return Nst_inc_ref(Nst_s.c_neginf);
            else
                return Nst_inc_ref(Nst_s.c_inf);
        }
        if (isnan(val)) {
            if (*(i64 *)&val & 0x8000000000000000)
                return Nst_inc_ref(Nst_s.c_negnan);
            else
                return Nst_inc_ref(Nst_s.c_nan);
        }

        i8 *buffer = Nst_malloc_c(MAX_REAL_CHAR_COUNT, i8);
        CHECK_BUFFER(buffer);
        i32 len = sprintf(buffer, "%." REAL_PRECISION "lg", val);
        // this is temporary, Nest_fmt will work better
        for (i32 i = 0; i < len; i++) {
            if (buffer[i] == '.' || buffer[i] == 'e')
                return Nst_string_new_allocated(buffer, len);
        }
        buffer[len++] = '.';
        buffer[len++] = '0';
        buffer[len] = '\0';
        return Nst_string_new_allocated(buffer, len);
    } else if (ob_t == Nst_t.Bool) {
        if (ob == Nst_c.Bool_true)
            return Nst_inc_ref(Nst_s.c_true);
        else
            return Nst_inc_ref(Nst_s.c_false);
    } else if (ob_t == Nst_t.Type) {
        return Nst_inc_ref(Nst_TYPE_STR(ob));
    } else if (ob_t == Nst_t.Byte) {
        u8 value = AS_BYTE(ob);
        i8 *str = Nst_calloc_c(3, i8, NULL);
        CHECK_BUFFER(str);
        if (value <= 0x7f) {
            str[0] = AS_BYTE(ob);
            return Nst_string_new_allocated(str, 1);
        }
        str[0] = 0b11000000 | (value >> 6);
        str[1] = 0b10000000 | (value & 0x3f);
        return Nst_string_new_allocated(str, 2);
    } else if (ob_t == Nst_t.Array || ob_t == Nst_t.Vector) {
        Nst_LList *all_objs = Nst_llist_new();
        if (all_objs == NULL)
            return NULL;
        Nst_Obj *str = _Nst_obj_str_cast_seq(SEQ(ob), all_objs);
        Nst_llist_destroy(all_objs, NULL);
        return str;
    } else if (ob_t == Nst_t.Map) {
        Nst_LList *all_objs = Nst_llist_new();
        if (all_objs == NULL)
            return NULL;
        Nst_Obj *str = _Nst_obj_str_cast_map(MAP(ob), all_objs);
        Nst_llist_destroy(all_objs, NULL);
        return str;
    } else if (ob_t == Nst_t.Null)
        return Nst_inc_ref(Nst_s.c_null);
    else if (ob_t == Nst_t.IOFile) {
        const i8 *empty_val = "<IOFile[-----]>";
        i8 *buffer = (i8 *)Nst_calloc(
            1, sizeof(i8) * strlen(empty_val),
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
        return Nst_string_new_allocated(buffer, strlen(empty_val));
    } else if (ob_t == Nst_t.Func) {
        i8 *buffer = Nst_malloc_c(13 + MAX_INT_CHAR_COUNT, i8);
        CHECK_BUFFER(buffer);
        usize arg_num = FUNC(ob)->arg_num;
        i32 len = sprintf(
            buffer,
            "<Func %zi arg%s>",
            arg_num,
            arg_num == 1 ? "" : "s");
        return Nst_string_new_allocated(buffer, len);
    } else {
        i8 *buffer = Nst_malloc_c(
            Nst_TYPE_STR(ob->type)->len + 16 + (i32)sizeof(usize) * 2,
            i8);
        CHECK_BUFFER(buffer);
        i32 len = sprintf(
            buffer,
            "<%s object at 0x%0*zX>",
            TYPE_NAME(ob),
            (int)sizeof(usize) * 2,
            (usize)ob);
        return Nst_string_new_allocated(buffer, len);
    }
}

static Nst_Obj *obj_to_bool(Nst_Obj *ob)
{
    Nst_TypeObj *ob_t = ob->type;
    if (ob_t == Nst_t.Int)
        Nst_RETURN_BOOL(AS_INT(ob) != 0);
    else if (ob_t == Nst_t.Real)
        Nst_RETURN_BOOL(AS_REAL(ob) != 0.0);
    else if (ob_t == Nst_t.Str)
        Nst_RETURN_BOOL(STR(ob)->len != 0);
    else if (ob_t == Nst_t.Map)
        Nst_RETURN_BOOL(MAP(ob)->len != 0);
    else if (ob_t == Nst_t.Array || ob_t == Nst_t.Vector)
        Nst_RETURN_BOOL(SEQ(ob)->len != 0);
    else if (ob_t == Nst_t.Null)
        Nst_RETURN_FALSE;
    else if (ob_t == Nst_t.Byte)
        Nst_RETURN_BOOL(AS_BYTE(ob) != 0);
    else if (ob_t == Nst_t.IOFile)
        Nst_RETURN_BOOL(!Nst_IOF_IS_CLOSED(ob));
    else
        Nst_RETURN_TRUE;
}

static Nst_Obj *obj_to_byte(Nst_Obj *ob)
{
    Nst_TypeObj *ob_t = ob->type;

    if (ob_t == Nst_t.Int) {
        f64 val = AS_REAL(ob);
        if (isnan(val)) {
            Nst_set_value_error_c(_Nst_EM_NAN_TO_BYTE);
            return NULL;
        } else if (isinf(val)) {
            Nst_set_value_error_c(_Nst_EM_INF_TO_BYTE);
            return NULL;
        }
            return Nst_byte_new(AS_INT(ob) & 0xff);
    }
    else if (ob_t == Nst_t.Real)
        return Nst_byte_new((i64)AS_REAL(ob) & 0xff);
    else if (ob_t == Nst_t.Str)
        return Nst_string_parse_byte(STR(ob));
    RETURN_CAST_TYPE_ERROR(Nst_t.Byte);
}

static Nst_Obj *obj_to_int(Nst_Obj *ob)
{
    Nst_TypeObj *ob_t = ob->type;

    if (ob_t == Nst_t.Real) {
        f64 val = AS_REAL(ob);
        if (isnan(val)) {
            Nst_set_value_error_c(_Nst_EM_NAN_TO_INT);
            return NULL;
        } else if (isinf(val)) {
            Nst_set_value_error_c(_Nst_EM_INF_TO_INT);
            return NULL;
        }
        return Nst_int_new((i64)val);
    } else if (ob_t == Nst_t.Byte)
        return Nst_int_new((i64)AS_BYTE(ob));
    else if (ob_t == Nst_t.Str)
        return Nst_string_parse_int(STR(ob), 0);
    RETURN_CAST_TYPE_ERROR(Nst_t.Int);
}

static Nst_Obj *obj_to_real(Nst_Obj *ob)
{
    Nst_TypeObj *ob_t = ob->type;

    if (ob_t == Nst_t.Int)
        return Nst_real_new((f64)AS_INT(ob));
    else if (ob_t == Nst_t.Byte)
        return Nst_real_new((f64)AS_BYTE(ob));
    else if (ob_t == Nst_t.Str)
        return Nst_string_parse_real(STR(ob));
    RETURN_CAST_TYPE_ERROR(Nst_t.Real);
}

static Nst_Obj *seq_to_seq(Nst_Obj *ob, bool is_vect)
{
    usize seq_len = SEQ(ob)->len;
    Nst_SeqObj *seq = is_vect ? SEQ(Nst_vector_new(seq_len))
                              : SEQ(Nst_array_new(seq_len));
    if (seq == NULL)
        return NULL;
    for (usize i = 0; i < seq_len; i++)
        Nst_seq_set(seq, i, SEQ(ob)->objs[i]);

    return OBJ(seq);
}

static Nst_Obj *str_to_seq(Nst_Obj *ob, bool is_vect)
{
    usize str_len = STR(ob)->true_len;
    Nst_SeqObj *seq = is_vect ? SEQ(Nst_vector_new(str_len))
                              : SEQ(Nst_array_new(str_len));
    if (seq == NULL)
        return NULL;

    Nst_Obj *ch;
    isize idx = 0;
    isize i = 0;

    while (Nst_string_next_ch(ob, &idx, &ch))
        seq->objs[i++] = ch;

    if (idx == -1) {
        seq->len = i;
        Nst_dec_ref(seq);
        return NULL;
    }

    return OBJ(seq);
}

static Nst_Obj *iter_to_seq(Nst_Obj *ob, bool is_vect)
{
    Nst_SeqObj *seq = SEQ(Nst_vector_new(0));
    Nst_IterObj *iter = ITER(ob);
    if (seq == NULL)
        return NULL;

    if (!Nst_iter_start(iter)) {
        Nst_dec_ref(seq);
        return NULL;
    }

    while (true) {
        Nst_Obj *result = Nst_iter_get_val(iter);
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
        return OBJ(seq);
    Nst_Obj **new_objs = Nst_realloc_c(
        seq->objs,
        seq->len,
        Nst_Obj *,
        seq->cap);
    if (new_objs != seq->objs) {
        seq->objs = new_objs;
        seq->cap = seq->len;
    }

    seq->type = TYPE(Nst_inc_ref(Nst_t.Array));
    Nst_dec_ref(Nst_t.Vector);
    return OBJ(seq);
}

static Nst_Obj *map_to_seq(Nst_Obj *ob, bool is_vect)
{
    Nst_MapObj *map = MAP(ob);
    usize seq_len = map->len;
    Nst_SeqObj *seq = is_vect ? SEQ(Nst_vector_new(seq_len))
                              : SEQ(Nst_array_new(seq_len));

    usize seq_i = 0;
    for (i32 i = Nst_map_get_next_idx(-1, map);
         i != -1;
         i = Nst_map_get_next_idx(i, map))
    {
        Nst_Obj *node_arr = Nst_array_create_c(
            "OO",
            map->nodes[i].key,
            map->nodes[i].value);
        if (node_arr == NULL) {
            seq->len = seq_i;
            Nst_dec_ref(seq);
            return NULL;
        }
        seq->objs[seq_i++] = node_arr;
    }

    return OBJ(seq);
}

static Nst_Obj *obj_to_seq(Nst_Obj *ob, bool is_vect)
{
    Nst_TypeObj *ob_t = ob->type;

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

static Nst_Obj *str_to_iter(Nst_Obj *ob)
{
    Nst_Obj *data = Nst_array_create_c("iO", 0, ob);
    if (data == NULL)
        return NULL;

    Nst_inc_ref(Nst_itf.str_start);
    Nst_inc_ref(Nst_itf.str_get_val);
    return Nst_iter_new(
        Nst_itf.str_start,
        Nst_itf.str_get_val,
        OBJ(data));
}

static Nst_Obj *seq_to_iter(Nst_Obj *ob)
{
    Nst_Obj *data = Nst_array_create_c("iO", 0, ob);
    if (data == NULL)
        return NULL;

    Nst_inc_ref(Nst_itf.seq_start);
    Nst_inc_ref(Nst_itf.seq_get_val);
    return Nst_iter_new(
        Nst_itf.seq_start,
        Nst_itf.seq_get_val,
        OBJ(data));
}

static Nst_Obj *map_to_iter(Nst_Obj *ob)
{
    Nst_Obj *data = Nst_array_create_c("iO", 0, ob);
    if (data == NULL)
        return NULL;

    Nst_inc_ref(Nst_itf.map_start);
    Nst_inc_ref(Nst_itf.map_get_val);
    return Nst_iter_new(
        Nst_itf.map_start,
        Nst_itf.map_get_val,
        OBJ(data));
}

static Nst_Obj *obj_to_iter(Nst_Obj *ob)
{
    Nst_TypeObj *ob_t = ob->type;

    if (ob_t == Nst_t.Str)
        return str_to_iter(ob);
    else if (ob_t == Nst_t.Array || ob_t == Nst_t.Vector)
        return seq_to_iter(ob);
    else if (ob_t == Nst_t.Map)
        return map_to_iter(ob);
    else
        RETURN_CAST_TYPE_ERROR(Nst_t.Iter);
}

static Nst_Obj *seq_to_map(Nst_Obj *ob)
{
    Nst_SeqObj *seq = SEQ(ob);
    Nst_Obj **objs = seq->objs;
    Nst_MapObj *map = MAP(Nst_map_new());
    if (map == NULL)
        return NULL;

    for (usize i = 0, n = seq->len; i < n; i++) {
        if (objs[i]->type != Nst_t.Array && objs[i]->type != Nst_t.Vector) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_MAP_TO_SEQ_TYPE_ERR("index"),
                TYPE_NAME(objs[i]), i));
            Nst_dec_ref(map);
            return NULL;
        }

        if (SEQ(objs[i])->len != 2) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_MAP_TO_SEQ_LEN_ERR("index"),
                SEQ(objs[i])->len, i));
            Nst_dec_ref(map);
            return NULL;
        }

        i32 hash = Nst_obj_hash(SEQ(objs[i])->objs[0]);

        if (hash == -1) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_MAP_TO_SEQ_HASH("index"), i));
            Nst_dec_ref(map);
            return NULL;
        }

        if (!Nst_map_set(map, SEQ(objs[i])->objs[0], SEQ(objs[i])->objs[1])) {
            Nst_dec_ref(map);
            return NULL;
        }
    }

    return OBJ(map);
}

static Nst_Obj *iter_to_map(Nst_Obj *ob)
{
    Nst_IterObj *iter = ITER(ob);
    Nst_MapObj *map = MAP(Nst_map_new());
    if (map == NULL)
        return NULL;

    if (!Nst_iter_start(iter)) {
        Nst_dec_ref(map);
        return NULL;
    }

    usize iter_count = 1;

    while (true) {
        Nst_SeqObj *result = SEQ(Nst_iter_get_val(iter));
        if (result == NULL) {
            Nst_dec_ref(map);
            return NULL;
        } else if (OBJ(result) == Nst_c.IEnd_iend) {
            Nst_dec_ref(result);
            break;
        }

        if (result->type != Nst_t.Array && result->type != Nst_t.Vector) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_MAP_TO_SEQ_TYPE_ERR("iteration"),
                TYPE_NAME(result), iter_count));
            Nst_dec_ref(map);
            return NULL;
        }

        if (result->len != 2) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_MAP_TO_SEQ_LEN_ERR("iteration"),
                result->len, iter_count));
            Nst_dec_ref(map);
            return NULL;
        }

        i32 hash = Nst_obj_hash(result->objs[0]);

        if (hash == -1) {
            Nst_set_type_error(Nst_sprintf(
                _Nst_EM_MAP_TO_SEQ_HASH("iteration"), iter_count));
            Nst_dec_ref(map);
            return NULL;
        }

        if (!Nst_map_set(map, result->objs[0], result->objs[1])) {
            Nst_dec_ref(map);
            return NULL;
        }
        Nst_dec_ref(result);
        iter_count++;
    }

    return OBJ(map);
}

static Nst_Obj *obj_to_map(Nst_Obj *ob)
{
    Nst_TypeObj *ob_t = ob->type;

    if (ob_t == Nst_t.Array || ob_t == Nst_t.Vector)
        return seq_to_map(ob);
    else if (ob_t == Nst_t.Iter)
        return iter_to_map(ob);
    RETURN_CAST_TYPE_ERROR(Nst_t.Map);
}

Nst_Obj *_Nst_obj_cast(Nst_Obj *ob, Nst_TypeObj *type)
{
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

Nst_Obj *_Nst_obj_contains(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if (ob1->type == Nst_t.Array || ob1->type == Nst_t.Vector) {
        Nst_Obj **objs = SEQ(ob1)->objs;
        for (usize i = 0, n = SEQ(ob1)->len; i < n; i++) {
            if (Nst_obj_eq_c(objs[i], ob2))
                Nst_RETURN_TRUE;
        }
        Nst_RETURN_FALSE;
    } else if (ob1->type == Nst_t.Map) {
        if (Nst_obj_hash(ob2) == -1)
            Nst_RETURN_FALSE;

        Nst_Obj *item = Nst_map_get(ob1, ob2);
        if (item == NULL)
            Nst_RETURN_FALSE;
        else {
            Nst_dec_ref(item);
            Nst_RETURN_TRUE;
        }
    } else if (ob1->type == Nst_t.Str && ob2->type == Nst_t.Str) {
        i8 *res = Nst_string_find(
            STR(ob1)->value, STR(ob1)->len,
            STR(ob2)->value, STR(ob2)->len);
        Nst_RETURN_BOOL(res != NULL);
    } else
        RETURN_STACK_OP_TYPE_ERROR("<.>");
}

Nst_Obj *_Nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2)
{
    ob1 = Nst_obj_cast(ob1, Nst_t.Str);
    ob2 = Nst_obj_cast(ob2, Nst_t.Str);

    Nst_StrObj *nst_s1 = STR(ob1);
    Nst_StrObj *nst_s2 = STR(ob2);

    i8 *s1 = nst_s1->value;
    i8 *s2 = nst_s2->value;
    usize len1 = nst_s1->len;
    usize len2 = nst_s2->len;
    usize tot_len = len1 + len2;

    i8 *buffer = Nst_malloc_c(tot_len + 1, i8);

    CHECK_BUFFER(buffer);

    memcpy(buffer, s1, len1);
    memcpy(buffer + len1, s2, len2);
    buffer[tot_len] = '\0';

    Nst_Obj *new_obj = Nst_string_new_len(
        buffer,
        tot_len,
        nst_s1->true_len + nst_s2->true_len,
        true);

    if (new_obj == NULL)
        Nst_free(buffer);

    Nst_dec_ref(ob1);
    Nst_dec_ref(ob2);

    return new_obj;
}

Nst_Obj *_Nst_obj_range(Nst_Obj *start, Nst_Obj *stop, Nst_Obj *step)
{
    if (start->type != Nst_t.Int) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_INVALID_OPERAND_TYPE("->"),
            TYPE_NAME(start)));
        return NULL;
    }

    if (stop->type != Nst_t.Int) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_INVALID_OPERAND_TYPE("->"),
            TYPE_NAME(stop)));
        return NULL;
    }

    if (step->type != Nst_t.Int) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_INVALID_OPERAND_TYPE("->"),
            TYPE_NAME(step)));
        return NULL;
    }

    if (AS_INT(step) == 0) {
        Nst_set_value_error_c(_Nst_EM_RANGE_STEP_ZERO);
        return NULL;
    }

    Nst_Obj *data_seq = Nst_array_create_c("iOOO", 0, start, stop, step);
    if (data_seq == NULL)
        return NULL;

    Nst_inc_ref(Nst_itf.range_start);
    Nst_inc_ref(Nst_itf.range_get_val);
    return Nst_iter_new(
        Nst_itf.range_start,
        Nst_itf.range_get_val,
        data_seq);
}

// Local operations
Nst_Obj *_Nst_obj_neg(Nst_Obj *ob)
{
    if (ob->type == Nst_t.Byte)
        return Nst_byte_new(-AS_BYTE(ob));
    else if (ob->type == Nst_t.Int)
        return Nst_int_new(-AS_INT(ob));
    else if (ob->type == Nst_t.Real)
        return Nst_real_new(-AS_REAL(ob));
    else
        RETURN_LOCAL_OP_TYPE_ERROR("-:");
}

Nst_Obj *_Nst_obj_len(Nst_Obj *ob)
{
    if (ob->type == Nst_t.Str)
        return Nst_int_new(STR(ob)->true_len);
    else if (ob->type == Nst_t.Map)
        return Nst_int_new(MAP(ob)->len);
    else if (IS_SEQ(ob))
        return Nst_int_new(SEQ(ob)->len);
    else if (ob->type == Nst_t.Func)
        return Nst_int_new(FUNC(ob)->arg_num);
    else
        RETURN_LOCAL_OP_TYPE_ERROR("$");
}

Nst_Obj *_Nst_obj_bwnot(Nst_Obj *ob)
{
    if (ob->type == Nst_t.Byte)
        return Nst_byte_new(~AS_BYTE(ob));
    else if (ob->type == Nst_t.Int)
        return Nst_int_new(~AS_INT(ob));
    else
        RETURN_LOCAL_OP_TYPE_ERROR("~");
}

Nst_Obj *_Nst_obj_lgnot(Nst_Obj *ob)
{
    if (Nst_obj_to_bool(ob))
        Nst_RETURN_FALSE;
    else
        Nst_RETURN_TRUE;
}

Nst_Obj *_Nst_obj_stdout(Nst_Obj *ob)
{
    if (Nst_IOF_IS_CLOSED(Nst_io.out))
        return Nst_inc_ref(ob);

    Nst_Obj *str = Nst_obj_cast(ob, Nst_t.Str);

    Nst_IOResult result = Nst_fwrite(
        STR(str)->value,
        STR(str)->len,
        NULL,
        Nst_io.out);

    Nst_dec_ref(str);

    if (result != Nst_IO_SUCCESS) {
        switch (result) {
        case Nst_IO_CLOSED:
            Nst_set_value_errorf(_Nst_EM_FILE_CLOSED, "@@io._get_stdout");
            break;
        case Nst_IO_OP_FAILED:
            Nst_set_value_error_c(_Nst_EM_WRITE_FAILED);
            break;
        case Nst_IO_ERROR:
            Nst_set_value_error_c(_Nst_EM_CALL_FAILED("Nst_write"));
            break;
        case Nst_IO_INVALID_DECODING: {
            u32 ch;
            const i8 *name;
            Nst_io_result_get_details(&ch, NULL, &name);
            Nst_set_value_errorf(_Nst_EM_INVALID_DECODING, (int)ch, name);
            break;
        }
        case Nst_IO_INVALID_ENCODING: {
            u32 ch;
            const i8 *name;
            Nst_io_result_get_details(&ch, NULL, &name);
            Nst_set_value_errorf(_Nst_EM_INVALID_ENCODING, (u8)ch, name);
            break;
        }
        default:
            Nst_set_call_error_c(_Nst_EM_CALL_FAILED("_Nst_obj_stdout"));
        }
        return NULL;
    }

    return Nst_inc_ref(ob);
}

static inline i8 get_one_char(i8 *ch)
{
    ch[0] = 0; ch[1] = 0; ch[2] = 0; ch[3] = 0; ch[4] = 0;

    Nst_IOResult result = Nst_fread(ch, 5, 1, NULL, Nst_io.in);
    if (result == Nst_IO_SUCCESS)
        return true;
    else
        return false;
}

Nst_Obj *_Nst_obj_stdin(Nst_Obj *ob)
{
    if (Nst_IOF_IS_CLOSED(Nst_io.in))
        return Nst_string_new_c("", 0, false);

    ob = Nst_obj_cast(ob, Nst_t.Str);
    Nst_fwrite(STR(ob)->value, STR(ob)->len, NULL, Nst_io.out);
    Nst_fflush(Nst_io.out);
    Nst_dec_ref(ob);

    Nst_Buffer buf;
    if (!Nst_buffer_init(&buf, 4))
        return NULL;

    i8 ch[5];
    while (get_one_char(ch)) {
        if (ch[0] == '\r')
            continue;
        if (ch[0] == '\n')
            break;

        if (!Nst_buffer_append_c_str(&buf, (const i8 *)ch)) {
            Nst_buffer_destroy(&buf);
            return NULL;
        }
    }

    return OBJ(Nst_buffer_to_string(&buf));
}

Nst_Obj *_Nst_obj_typeof(Nst_Obj *ob)
{
    return Nst_inc_ref(ob->type);
}

Nst_Obj *_Nst_obj_import(Nst_Obj *ob)
{
    if (ob->type != Nst_t.Str) {
        Nst_set_type_error(Nst_sprintf(
            _Nst_EM_EXPECTED_TYPE("Str"),
            TYPE_NAME(ob)
        ));
        return NULL;
    }

    i8 *file_name = STR(ob)->value;
    usize file_name_len = STR(ob)->len;
    bool c_import = false;

    if (STR(ob)->len > 6
        && file_name[0] == '_' && file_name[1] == '_'
        && file_name[2] == 'C' && file_name[3] == '_'
        && file_name[4] == '_' && file_name[5] == ':')
    {
        c_import = true;
        file_name += 6; // skip __C__:
        file_name_len -= 6;
    }

    Nst_StrObj *import_path = _Nst_get_import_path(file_name, file_name_len);
    if (import_path == NULL) {
        Nst_set_value_error(Nst_sprintf(
            _Nst_EM_FILE_NOT_FOUND,
            file_name
        ));
        return NULL;
    }

    // Check if the module is in the import stack
    for (Nst_LLNode *n = Nst_state.lib_paths->head; n != NULL; n = n->next) {
        if (Nst_string_compare(import_path, STR(n->value)) == 0) {
            Nst_dec_ref(import_path);
            Nst_set_import_error_c(_Nst_EM_CIRC_IMPORT);
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

static bool add_to_handle_map(Nst_StrObj *path, Nst_MapObj *map,
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

static Nst_Obj *import_nest_lib(Nst_StrObj *file_path)
{
    Nst_SourceText *lib_src = Nst_malloc_c(1, Nst_SourceText);
    Nst_MapObj *map = NULL;

    if (lib_src == NULL)
        goto cleanup;

    if (!Nst_run_module(file_path->value, lib_src)) {
        Nst_llist_push(Nst_state.lib_srcs, lib_src, true);
        goto cleanup;
    }

    map = MAP(Nst_vstack_pop(&Nst_state.es->v_stack));
    Nst_assert(map != NULL);

    if (!add_to_handle_map(file_path, map, lib_src)) {
        Nst_source_text_destroy(lib_src);
        Nst_free(lib_src);
        goto cleanup;
    }
    Nst_llist_pop(Nst_state.lib_paths);
    return OBJ(map);

cleanup:
    Nst_llist_pop(Nst_state.lib_paths);
    Nst_dec_ref(file_path);
    Nst_ndec_ref(map);
    return NULL;
}

static Nst_Obj *import_c_lib(Nst_StrObj *file_path)
{
    void (*lib_quit_func)();
    lib_t lib = dlopen(file_path->value);

    if (!lib) {
        Nst_llist_pop(Nst_state.lib_paths);
        Nst_dec_ref(file_path);
#ifdef Nst_WIN
        Nst_set_import_error_c(_Nst_EM_FILE_NOT_DLL);
#else
        Nst_set_import_error_c(dlerror());
#endif // !__cplusplus
        return NULL;
    }

    // Initialize library
    Nst_Declr *(*lib_init)() = (Nst_Declr *(*)())dlsym(lib, "lib_init");
    if (lib_init == NULL) {
        Nst_llist_pop(Nst_state.lib_paths);
        Nst_dec_ref(file_path);
        Nst_set_import_error_c(_Nst_EM_NO_LIB_FUNC("lib_init"));
        dlclose(lib);
        return NULL;
    }

    Nst_Declr *obj_ptrs = lib_init();

    if (obj_ptrs == NULL) {
        Nst_llist_pop(Nst_state.lib_paths);
        Nst_dec_ref(file_path);
        dlclose(lib);
        if (!Nst_error_occurred())
            Nst_set_import_error_c(_Nst_EM_LIB_INIT_FAILED);
        return NULL;
    }

    // Populate the function map
    Nst_MapObj *obj_map = MAP(Nst_map_new());
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
    return OBJ(obj_map);
fail:
    lib_quit_func = (void (*)())dlsym(lib, "lib_quit");
    if (lib_quit_func)
        lib_quit_func();
    dlclose(lib);
    return NULL;
}

static Nst_Obj *search_local_directory(i8 *initial_path)
{
    i8 *file_path;
    usize new_len = Nst_get_full_path(initial_path, &file_path, NULL);
    if (file_path == NULL)
        return NULL;

    FILE *file = Nst_fopen_unicode(file_path, "rb");
    if (file == NULL) {
        Nst_free(file_path);
        return NULL;
    }
    fclose(file);
    return Nst_string_new_allocated(file_path, new_len);
}

static Nst_Obj *rel_path_to_abs_path_str_if_found(i8 *file_path)
{
    FILE *file = Nst_fopen_unicode(file_path, "rb");
    if (file == NULL) {
        Nst_free(file_path);
        return NULL;
    }
    fclose(file);

    i8 *abs_path;
    usize abs_path_len = Nst_get_full_path(file_path, &abs_path, NULL);
    Nst_free(file_path);

    if (abs_path == NULL)
        return NULL;
    return Nst_string_new_allocated(abs_path, abs_path_len);
}

#if defined(_DEBUG) && defined(Nst_WIN)

static Nst_Obj *search_debug_directory(i8 *initial_path, usize path_len)
{
    // little hack to get the absolute path without using it explicitly
    const i8 *root_path = __FILE__;
    const i8 *obj_ops_path_suffix = "src\\obj_ops.c";
    const i8 *nest_files = "libs\\_nest_files\\";
    usize root_len = strlen(root_path) - strlen(obj_ops_path_suffix);
    usize nest_files_len = strlen(nest_files);
    usize full_size = root_len + nest_files_len + path_len;

    i8 *file_path = Nst_malloc_c(full_size + 1, i8);
    if (file_path == NULL)
        return NULL;

    memcpy(file_path, root_path, root_len);
    memcpy(file_path + root_len, nest_files, nest_files_len);
    memcpy(file_path + root_len + nest_files_len, initial_path, path_len);
    file_path[full_size] = '\0';

    return rel_path_to_abs_path_str_if_found(file_path);
}

#endif

static Nst_Obj *search_stdlib_directory(i8 *initial_path, usize path_len)
{
#if defined(_DEBUG) && defined(Nst_WIN)
    return search_debug_directory(initial_path, path_len);
#else
#ifdef Nst_WIN

    i8 *appdata = getenv("LOCALAPPDATA");
    if (appdata == NULL) {
        Nst_failed_allocation();
        return NULL;
    }
    usize appdata_len = strlen(appdata);
    const i8 *nest_files = "\\Programs\\nest\\nest_libs\\";
    usize nest_files_len = strlen(nest_files);
    usize tot_len = appdata_len + nest_files_len + path_len;

    i8 *file_path = Nst_malloc_c(tot_len + 1, i8);
    if (file_path == NULL)
        return NULL;
    sprintf(file_path, "%s%s%s", appdata, nest_files, initial_path);

#else

    const i8 *nest_files = "/usr/lib/nest/";
    usize nest_files_len = strlen(nest_files);
    usize tot_len = nest_files_len + path_len;

    i8 *file_path = Nst_malloc_c(tot_len + 1, i8);
    if (file_path == NULL)
        return NULL;
    sprintf(file_path, "%s%s", nest_files, initial_path);

#endif // !Nst_WIN

    return rel_path_to_abs_path_str_if_found(file_path);
#endif
}

Nst_StrObj *_Nst_get_import_path(i8 *initial_path, usize path_len)
{
    Nst_Obj *full_path = search_local_directory(initial_path);
    if (full_path != NULL)
        return STR(full_path);
    else if (Nst_error_occurred())
        return NULL;

    full_path = search_stdlib_directory(initial_path, path_len);

    if (Nst_error_occurred())
        return NULL;
    else if (full_path == NULL) {
        Nst_set_value_errorf(_Nst_EM_FILE_NOT_FOUND, initial_path);
        return NULL;
    }
    return STR(full_path);
}
