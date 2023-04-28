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

#ifdef WINDOWS

#include <windows.h>
#define dlsym GetProcAddress
#define dlopen LoadLibraryA
#define dlclose FreeLibrary
typedef HMODULE lib_t;

#else

#include <dlfcn.h>
typedef void * lib_t;
#define dlopen(lib) dlopen(lib, RTLD_LAZY)

#endif
#include "mem.h"
#define MAX_INT_CHAR_COUNT 21

// precision    1  +     16    +  1  +    2     +        3               + 1
//           (sign) (precision) (dot) (e+ or e-) (over e+308 becomes inf) (\0)
#define MAX_REAL_CHAR_COUNT 24
#define MAX_BYTE_CHAR_COUNT 5
#define REAL_EPSILON 9.9e-15
#define REAL_PRECISION "16"

#define IS_NUM(obj) \
    ( obj->type == nst_t.Int || \
      obj->type == nst_t.Real || \
      obj->type == nst_t.Byte )
#define IS_INT(obj) ( obj->type == nst_t.Int || obj->type == nst_t.Byte )
#define IS_SEQ(obj) ( obj->type == nst_t.Array || obj->type == nst_t.Vector )
#define ARE_TYPE(nst_type) ( ob1->type == nst_type && ob2->type == nst_type )

#define RETURN_STACK_OP_TYPE_ERROR(operand) do { \
    NST_SET_TYPE_ERROR(nst_sprintf( \
        "invalid types '%s' and '%s' for '" operand "'", \
        TYPE_NAME(ob1), TYPE_NAME(ob2))); \
    return NULL; \
    } while ( 0 )

#define RETURN_CAST_TYPE_ERROR do { \
    NST_SET_TYPE_ERROR(nst_sprintf( \
        _NST_EM_INVALID_CASTING, \
        TYPE_NAME(ob), STR(type)->value)); \
    return NULL; \
    } while ( 0 )

#define RETURN_LOCAL_OP_TYPE_ERROR(operand) do { \
    NST_SET_TYPE_ERROR(nst_sprintf( \
        _NST_EM_INVALID_OPERAND_TYPE(operand), \
        TYPE_NAME(ob))); \
    return NULL; \
    } while ( 0 )

#define RETURN_MISSING_FUNC_ERROR(func) NST_SET_RAW_VALUE_ERROR(_NST_EM_MISSING_FUNC(func))

#define CHECK_BUFFER(buf) do { \
        if ( buf == NULL ) \
        { \
            return NULL; \
        } \
    } while (0)

#define OBJ_INIT_FARGS \
    Nst_TypeObjs, Nst_StrConsts, Nst_Consts, Nst_StdStreams*, Nst_ExecutionState


static Nst_Obj *map_eq(Nst_MapObj *map1,
                       Nst_MapObj *map2,
                       Nst_LList *containers,
                       Nst_OpErr *err);
static Nst_Obj *seq_eq(Nst_SeqObj *seq1,
                       Nst_SeqObj *seq2,
                       Nst_LList *containers,
                       Nst_OpErr *err);
static Nst_Obj *import_nest_lib(Nst_StrObj *file_path, Nst_OpErr *err);
static Nst_Obj *import_c_lib(Nst_StrObj *file_path, Nst_OpErr *err);
static void add_to_handle_map(Nst_StrObj *path,
                              Nst_MapObj *map,
                              Nst_SourceText *src_txt,
                              Nst_OpErr *err);

// Comparisons
Nst_Obj *_nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1 == ob2 )
    {
        NST_RETURN_TRUE;
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);
        bool check = AS_INT(ob1) == AS_INT(ob2);
        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);
        bool check = fabsl(AS_REAL(ob1) - AS_REAL(ob2)) < REAL_EPSILON;
        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else if ( ARE_TYPE(nst_t.Str) )
    {
        NST_RETURN_COND(STR(ob1)->len == STR(ob2)->len &&
            nst_string_compare(STR(ob1), STR(ob2)) == 0);
    }
    else if ( ARE_TYPE(nst_t.Bool) )
    {
        NST_RETURN_COND(ob1 == ob2);
    }
    else if ( IS_SEQ(ob1) && IS_SEQ(ob2) )
    {
        Nst_LList *containers = nst_llist_new(err);
        if ( containers == NULL )
        {
            return NULL;
        }
        Nst_Obj *res = seq_eq(SEQ(ob1), SEQ(ob2), containers, err);
        nst_llist_destroy(containers, NULL);
        return res;
    }
    else if ( ARE_TYPE(nst_t.Map) )
    {
        Nst_LList *containers = nst_llist_new(err);
        if ( containers == NULL )
        {
            return NULL;
        }
        Nst_Obj *res = map_eq(MAP(ob1), MAP(ob2), containers, err);
        nst_llist_destroy(containers, NULL);
        return res;
    }
    else
    {
        NST_RETURN_FALSE;
    }
}

static Nst_Obj *seq_eq(Nst_SeqObj *seq1,
                       Nst_SeqObj *seq2,
                       Nst_LList *containers,
                       Nst_OpErr *err)
{
    if (seq1->len != seq2->len)
        NST_RETURN_FALSE;

    for ( Nst_LLNode *n = containers->head; n != NULL; n = n->next )
    {
        if ( n->value == seq1 || n->value == seq2 )
        {
            NST_RETURN_FALSE;
        }
    }

    nst_llist_append(containers, seq1, false, err);
    if ( err->name != NULL )
    {
        return NULL;
    }
    nst_llist_append(containers, seq2, false, err);
    if ( err->name != NULL )
    {
        return NULL;
    }

    Nst_Obj *ob1 = NULL;
    Nst_Obj *ob2 = NULL;
    Nst_Obj *result = NULL;
    for ( usize i = 0, n = seq1->len; i < n; i++ )
    {
        ob1 = seq1->objs[i];
        ob2 = seq2->objs[i];

        if ( IS_SEQ(ob1) && IS_SEQ(ob2) )
        {
            result = seq_eq(SEQ(ob1), SEQ(ob2), containers, err);
        }
        else if ( ARE_TYPE(nst_t.Map) )
        {
            result = map_eq(MAP(ob1), MAP(ob2), containers, err);
        }
        else
        {
            result = nst_obj_eq(ob1, ob2, NULL);
        }

        if ( result == NULL )
        {
            return NULL;
        }

        if ( result == nst_c.Bool_false )
        {
            return nst_c.Bool_false;
        }
        else
        {
            nst_dec_ref(nst_c.Bool_true);
        }
    }

    nst_llist_pop(containers); // pops seq1
    nst_llist_pop(containers); // pops seq2

    NST_RETURN_TRUE;
}

static Nst_Obj *map_eq(Nst_MapObj *map1,
                       Nst_MapObj *map2,
                       Nst_LList *containers,
                       Nst_OpErr *err)
{
    if ( map1->item_count != map2->item_count )
    {
        NST_RETURN_FALSE;
    }

    for ( Nst_LLNode *n = containers->head; n != NULL; n = n->next )
    {
        if ( n->value == map1 || n->value == map2 )
        {
            NST_RETURN_FALSE;
        }
    }

    nst_llist_append(containers, map1, false, err);
    nst_llist_append(containers, map2, false, err);
    if ( err->name != NULL )
    {
        return NULL;
    }

    Nst_Obj *key = NULL;
    Nst_Obj *ob1 = NULL;
    Nst_Obj *ob2 = NULL;
    Nst_Obj *result = NULL;
    for ( i32 i = nst_map_get_next_idx(-1, map1);
          i != -1;
          i = nst_map_get_next_idx(i, map1) )
    {
        key = map1->nodes[i].key;
        ob1 = map1->nodes[i].value;

        ob2 = _nst_map_get(map2, key);
        if ( ob2 == NULL )
        {
            NST_RETURN_FALSE;
        }
        else
        {
            nst_dec_ref(ob2);
        }

        if ( IS_SEQ(ob1) && IS_SEQ(ob2) )
        {
            result = seq_eq(SEQ(ob1), SEQ(ob2), containers, err);
        }
        else if ( ARE_TYPE(nst_t.Map) )
        {
            result = map_eq(MAP(ob1), MAP(ob2), containers, err);
        }
        else
        {
            result = nst_obj_eq(ob1, ob2, err);
        }

        if ( result == nst_c.Bool_false )
        {
            return nst_c.Bool_false;
        }
        else if ( result == NULL )
        {
            return NULL;
        }
        else
        {
            nst_dec_ref(nst_c.Bool_true);
        }
    }

    nst_llist_pop(containers); // pops seq1
    nst_llist_pop(containers); // pops seq2

    NST_RETURN_TRUE;
}

Nst_Obj *_nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( nst_obj_eq(ob1, ob2, err) == nst_c.Bool_true )
    {
        nst_dec_ref(nst_c.Bool_true);
        NST_RETURN_FALSE;
    }
    else
    {
        nst_dec_ref(nst_c.Bool_false);
        NST_RETURN_TRUE;
    }
}

Nst_Obj *_nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t.Str) )
    {
        NST_RETURN_COND(nst_string_compare(STR(ob1), STR(ob2)) > 0);
    }
    else if ( ARE_TYPE(nst_t.Byte) )
    {
        NST_RETURN_COND(AS_BYTE(ob1) > AS_BYTE(ob2));
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);
        bool check = AS_INT(ob1) > AS_INT(ob2);
        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);
        bool eq_check = fabsl(AS_REAL(ob1) - AS_REAL(ob2)) < REAL_EPSILON;
        bool check = AS_REAL(ob1) > AS_REAL(ob2);
        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        NST_RETURN_COND(check && !eq_check);
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR(">");
    }
}

Nst_Obj *_nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t.Str) )
    {
        NST_RETURN_COND(nst_string_compare(STR(ob1), STR(ob2)) < 0);
    }
    else if ( ARE_TYPE(nst_t.Byte))
    {
        NST_RETURN_COND(AS_BYTE(ob1) < AS_BYTE(ob2));
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);
        bool check = AS_INT(ob1) < AS_INT(ob2);
        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);
        bool eq_check = fabsl(AS_REAL(ob1) - AS_REAL(ob2)) < REAL_EPSILON;
        bool check = AS_REAL(ob1) < AS_REAL(ob2);
        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        NST_RETURN_COND(check && !eq_check);
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("<");
    }
}

Nst_Obj *_nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    Nst_Obj *res = nst_obj_eq(ob1, ob2, err);
    if ( res == nst_c.Bool_true )
    {
        return nst_c.Bool_true;
    }
    else if ( res == NULL )
    {
        return NULL;
    }
    else
    {
        nst_dec_ref(nst_c.Bool_false);
    }

    res = nst_obj_gt(ob1, ob2, err);

    if ( err != NULL && err->name == nst_s.e_TypeError )
    {
        nst_dec_ref(err->name);
        nst_dec_ref(err->message);
        RETURN_STACK_OP_TYPE_ERROR(">=");
    }
    return res;
}

Nst_Obj *_nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    Nst_Obj *res = nst_obj_eq(ob1, ob2, err);
    if ( res == nst_c.Bool_true )
    {
        return nst_c.Bool_true;
    }
    else if ( res == NULL )
    {
        return NULL;
    }
    else
    {
        nst_dec_ref(nst_c.Bool_false);
    }

    res = nst_obj_lt(ob1, ob2, err);

    if ( err != NULL && err->name == nst_s.e_TypeError )
    {
        nst_dec_ref(err->name);
        nst_dec_ref(err->message);
        RETURN_STACK_OP_TYPE_ERROR("<=");
    }
    return res;
}

// Arithmetic operations
Nst_Obj *_nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1->type == nst_t.Vector )
    {
        nst_vector_append(ob1, ob2, err);
        if ( NST_ERROR_OCCURRED )
        {
            return NULL;
        }
        return nst_inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) + AS_BYTE(ob2), err);
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) + AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);

        Nst_Obj *new_obj = nst_real_new(AS_REAL(ob1) + AS_REAL(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("+");
    }
}

Nst_Obj *_nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1->type == nst_t.Vector )
    {
        nst_dec_ref(nst_vector_remove(ob1, ob2));
        return nst_inc_ref(ob1);
    }
    else if ( ob1->type == nst_t.Map )
    {
        Nst_Obj *res = nst_map_drop(ob1, ob2);
        if ( res == NULL )
        {
            NST_SET_TYPE_ERROR(nst_sprintf(
                _NST_EM_UNHASHABLE_TYPE,
                TYPE_NAME(ob2)));
            return NULL;
        }

        nst_dec_ref(res);
        return nst_inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) - AS_BYTE(ob2), err);
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) - AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);

        Nst_Obj *new_obj = nst_real_new(AS_REAL(ob1) - AS_REAL(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("-");
    }
}

Nst_Obj *_nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1->type == nst_t.Vector && ob2->type == nst_t.Int )
    {
        if ( AS_INT(ob2) == 0 )
        {
            return nst_vector_new(0, err);
        }

        Nst_SeqObj *vect = SEQ(ob1);
        usize v_len = vect->len;
        usize new_size = (usize)AS_INT(ob2) * v_len;
        Nst_Obj **new_objs = nst_realloc(vect->objs, new_size, sizeof(Nst_Obj *), 0, err);
        if ( new_objs == NULL )
        {
            return NULL;
        }
        vect->objs = new_objs;
        vect->size = new_size;
        vect->len = new_size;
        for ( usize i = v_len; i < new_size; i++ )
        {
            new_objs[i] = nst_inc_ref(new_objs[i % v_len]);
        }

        return nst_inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) * AS_BYTE(ob2), err);
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) * AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);

        Nst_Obj *new_obj = nst_real_new(AS_REAL(ob1) * AS_REAL(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("*");
    }
}

Nst_Obj *_nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1->type == nst_t.Vector && ob2->type == nst_t.Int )
    {
        return nst_vector_pop(ob1, (usize)AS_INT(ob2));
    }
    else if ( ARE_TYPE(nst_t.Byte) )
    {
        if ( AS_BYTE(ob2) == 0 )
        {
            NST_SET_RAW_MATH_ERROR(_NST_EM_DIVISION_BY_ZERO);
            return NULL;
        }

        return nst_byte_new(AS_BYTE(ob1) / AS_BYTE(ob2), err);
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        if ( AS_INT(ob2) == 0 )
        {
            NST_SET_RAW_MATH_ERROR(_NST_EM_DIVISION_BY_ZERO);
            return NULL;
        }

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) / AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);

        if ( AS_REAL(ob2) == 0.0 )
        {
            NST_SET_RAW_MATH_ERROR(_NST_EM_DIVISION_BY_ZERO);
            return NULL;
        }

        Nst_Obj *new_obj = nst_real_new(AS_REAL(ob1) / AS_REAL(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("/");
    }
}

Nst_Obj *_nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t.Byte) )
    {
        Nst_Byte res = 1;
        Nst_Byte num = AS_BYTE(ob1);

        for ( Nst_Byte i = 0, n = AS_BYTE(ob2); i < n; i++ )
        {
            res *= num;
        }

        return nst_byte_new(res, err);
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Int res = 1;
        Nst_Int num = AS_INT(ob1);

        for ( Nst_Int i = 0, n = AS_INT(ob2); i < n; i++ )
        {
            res *= num;
        }

        Nst_Obj *new_obj = nst_int_new(res, err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);

        Nst_Real v1 = AS_REAL(ob1);
        Nst_Real v2 = AS_REAL(ob2);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        // any root of a negative number gives -nan as a result
        if ( v1 < 0 && floorl(v2) != v2 )
        {
            NST_SET_RAW_MATH_ERROR(_NST_EM_COMPLEX_POW);
            return NULL;
        }

        return nst_real_new((Nst_Real)powl(v1, v2), err);
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("^");
    }
}

Nst_Obj *_nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t.Byte) )
    {
        if ( AS_BYTE(ob2) == 0 )
        {
            NST_SET_RAW_MATH_ERROR(_NST_EM_MODULO_BY_ZERO);
            return NULL;
        }

        return nst_byte_new(AS_BYTE(ob1) % AS_BYTE(ob2), err);
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        if ( AS_INT(ob2) == 0 )
        {
            NST_SET_RAW_MATH_ERROR(_NST_EM_MODULO_BY_ZERO);
            return NULL;
        }

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) % AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);

        if ( AS_REAL(ob2) == 0.0 )
        {
            NST_SET_RAW_MATH_ERROR(_NST_EM_MODULO_BY_ZERO);
            return NULL;
        }

        Nst_Obj *new_obj = nst_real_new(fmod(AS_REAL(ob1), AS_REAL(ob2)), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("%");
    }
}

// Bitwise operations
Nst_Obj *_nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) | AS_BYTE(ob2), err);
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) | AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("|");
    }
}

Nst_Obj *_nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) & AS_BYTE(ob2), err);
    }
    else if (IS_INT(ob1) && IS_INT(ob2))
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) & AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("&");
    }
}

Nst_Obj *_nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) ^ AS_BYTE(ob2), err);
    }
    else if (IS_INT(ob1) && IS_INT(ob2))
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) ^ AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("^^");
    }
}

Nst_Obj *_nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) << AS_BYTE(ob2), err);
    }
    else if (IS_INT(ob1) && IS_INT(ob2))
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) << AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR("<<");
    }
}

Nst_Obj *_nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) >> AS_BYTE(ob2), err);
    }
    else if (IS_INT(ob1) && IS_INT(ob2))
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) >> AS_INT(ob2), err);

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_STACK_OP_TYPE_ERROR(">>");
    }
}

// Logical operations
Nst_Obj *_nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    Nst_Obj *b = nst_obj_cast(ob1, nst_t.Bool, err);
    Nst_Bool v1 = AS_BOOL(b);
    nst_dec_ref(b);

    if ( v1 )
    {
        return nst_inc_ref(ob1);
    }
    else
    {
        return nst_inc_ref(ob2);
    }
}

Nst_Obj *_nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    Nst_Obj *b = nst_obj_cast(ob1, nst_t.Bool, err);
    Nst_Bool v1 = AS_BOOL(b);
    nst_dec_ref(b);

    if ( !v1 )
    {
        return nst_inc_ref(ob1);
    }
    else
    {
        return nst_inc_ref(ob2);
    }
}

Nst_Obj *_nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    ob1 = nst_obj_cast(ob1, nst_t.Bool, err);
    ob2 = nst_obj_cast(ob2, nst_t.Bool, err);

    Nst_Bool v1 = AS_BOOL(ob1);
    Nst_Bool v2 = AS_BOOL(ob2);

    nst_dec_ref(ob1);
    nst_dec_ref(ob2);

    NST_RETURN_COND((v1 == NST_TRUE && v2 == NST_FALSE) ||
                    (v1 == NST_FALSE && v2 == NST_TRUE));
}

// Other

Nst_Obj* _nst_repr_str_cast(Nst_Obj* ob, Nst_OpErr *err)
{
    Nst_TypeObj *ob_t = ob->type;

    if ( ob_t == nst_t.Str )
    {
        return nst_string_repr(ob, err);
    }
    else if ( ob_t == nst_t.Byte )
    {
        i8 *str = (i8*)nst_calloc(5, sizeof(i8), NULL, err);
        CHECK_BUFFER(str);

        sprintf(str, "%lib", (i32)AS_BYTE(ob));
        return nst_string_new_c_raw((const i8 *)str, true, err);
    }
    else
    {
        return nst_obj_cast(ob, nst_t.Str, NULL);
    }
}

Nst_Obj *_nst_obj_str_cast_seq(Nst_SeqObj *seq_obj,
                               Nst_LList *all_objs,
                               Nst_OpErr *err)
{
    bool is_vect = seq_obj->type == nst_t.Vector;

    for ( Nst_LLNode *n = all_objs->head; n != NULL; n = n->next )
    {
        if ( seq_obj == n->value )
        {
            if ( is_vect )
            {
                return nst_string_new_c("<{.}>", 5, false, err);
            }
            else
            {
                return nst_string_new_c("{.}", 3, false, err);
            }
        }
    }

    if ( seq_obj->len == 0 )
    {
        if ( is_vect )
        {
            return nst_string_new_c("<{}>", 4, false, err);
        }
        else
        {
            return nst_string_new_c("{,}", 3, false, err);
        }
    }

    nst_llist_push(all_objs, seq_obj, false, err);
    if ( NST_ERROR_OCCURRED )
    {
        return NULL;
    }

    usize seq_len = seq_obj->len;

    Nst_Buffer buf;
    if ( !nst_buffer_init(&buf, 6, err) )
    {
        return NULL;
    }

    if ( is_vect )
    {
        nst_buffer_append_c_str(&buf, "<{ ", NULL);
    }
    else
    {
        nst_buffer_append_c_str(&buf, "{ ", NULL);
    }

    for ( usize i = 0; i < seq_len; i++ )
    {
        Nst_Obj *ob = seq_obj->objs[i];
        Nst_StrObj *ob_str;

        if ( IS_SEQ(ob) )
        {
            ob_str = STR(_nst_obj_str_cast_seq(SEQ(ob), all_objs, err));
        }
        else if ( ob->type == nst_t.Map )
        {
            ob_str = STR(_nst_obj_str_cast_map(MAP(ob), all_objs, err));
        }
        else
        {
            ob_str = STR(_nst_repr_str_cast(ob, err));
        }
        if ( NST_ERROR_OCCURRED )
        {
            nst_buffer_destroy(&buf);
            return NULL;
        }

        if ( !nst_buffer_append(&buf, ob_str, err) )
        {
            nst_buffer_destroy(&buf);
            return NULL;
        }

        if ( i == seq_len - 1 )
        {
            break;
        }

        if ( !nst_buffer_append_c_str(&buf, ", ", err) )
        {
            nst_buffer_destroy(&buf);
            return NULL;
        }
    }

    if ( is_vect )
    {
        nst_buffer_append_c_str(&buf, " }>", err);
    }
    else
    {
        nst_buffer_append_c_str(&buf, " }", err);
    }

    if ( NST_ERROR_OCCURRED )
    {
        nst_buffer_destroy(&buf);
        return NULL;
    }

    nst_llist_pop(all_objs);
    return OBJ(nst_buffer_to_string(&buf, err));
}

Nst_Obj *_nst_obj_str_cast_map(Nst_MapObj *map_obj,
                               Nst_LList  *all_objs,
                               Nst_OpErr  *err)
{
    for ( Nst_LLNode *n = all_objs->head; n != NULL; n = n->next )
    {
        if ( map_obj == n->value )
        {
            return nst_string_new_c("{.}", 3, false, err);
        }
    }

    if ( MAP(map_obj)->item_count == 0 )
    {
        return nst_string_new_c("{}", 2, false, err);
    }

    nst_llist_push(all_objs, map_obj, false, err);
    if ( NST_ERROR_OCCURRED )
    {
        return NULL;
    }

    Nst_Buffer buf;
    if ( !nst_buffer_init(&buf, 4, err) )
    {
        return NULL;
    }
    nst_buffer_append_c_str(&buf, "{ ", NULL);

    for ( i32 idx = nst_map_get_next_idx(-1, map_obj);
          idx != -1;
          idx = nst_map_get_next_idx(idx, map_obj) )
    {
        Nst_Obj *key = map_obj->nodes[idx].key;
        Nst_Obj *val = map_obj->nodes[idx].value;

        // Key cannot be a vector, an array or a map
        Nst_StrObj *key_str = STR(_nst_repr_str_cast(key, err));
        Nst_StrObj *val_str;

        if ( IS_SEQ(val) )
        {
            val_str = STR(_nst_obj_str_cast_seq(SEQ(val), all_objs, err));
        }
        else if ( val->type == nst_t.Map )
        {
            val_str = STR(_nst_obj_str_cast_map(MAP(val), all_objs, err));
        }
        else
        {
            val_str = STR(_nst_repr_str_cast(val, err));
        }

        if ( NST_ERROR_OCCURRED )
        {
            if ( key_str ) nst_dec_ref(key_str);
            if ( val_str ) nst_dec_ref(val_str);
            nst_buffer_destroy(&buf);
            return NULL;
        }

        usize expantion_amount = key_str->len + val_str->len + 4;
        if ( !nst_buffer_expand_by(&buf, expantion_amount, err) )
        {
            nst_buffer_destroy(&buf);
            return NULL;
        }

        nst_buffer_append(&buf, key_str, NULL);
        nst_buffer_append_c_str(&buf, ": ", NULL);
        nst_buffer_append(&buf, val_str, NULL);
        nst_buffer_append_c_str(&buf, ", ", NULL);

        nst_dec_ref(key_str);
        nst_dec_ref(val_str);
    }

    buf.len -= 2;
    nst_buffer_append_c_str(&buf, " }", NULL);
    nst_llist_pop(all_objs);

    return OBJ(nst_buffer_to_string(&buf, err));
}

Nst_Obj *_nst_obj_cast(Nst_Obj *ob, Nst_TypeObj *type, Nst_OpErr *err)
{
    Nst_TypeObj *ob_t = ob->type;

    if ( ob_t == type )
    {
        return nst_inc_ref(ob);
    }

    if ( type == nst_t.Str )
    {
        if ( ob_t == nst_t.Int )
        {
            i8 *buffer = (i8 *)nst_malloc(MAX_INT_CHAR_COUNT, sizeof(i8), err);
            CHECK_BUFFER(buffer);
            i32 len = sprintf(buffer, "%lli", AS_INT(ob));
            NST_RETURN_NEW_STR(buffer, len);
        }
        else if ( ob_t == nst_t.Real )
        {
            i8 *buffer = (i8 *)nst_malloc(MAX_REAL_CHAR_COUNT, sizeof(i8), err);
            CHECK_BUFFER(buffer);
            i32 len = sprintf(buffer, "%." REAL_PRECISION "lg", AS_REAL(ob));
            for ( i32 i = 0; i < len; i++ )
            {
                if ( buffer[i] == '.' || buffer[i] == 'e' )
                {
                    NST_RETURN_NEW_STR(buffer, len);
                }
            }
            buffer[len++] = '.';
            buffer[len++] = '0';
            buffer[len] = '\0';
            NST_RETURN_NEW_STR(buffer, len);
        }
        else if ( ob_t == nst_t.Bool )
        {
            if ( ob == nst_c.Bool_true )
            {
                return nst_inc_ref(nst_s.c_true);
            }
            else
            {
                return nst_inc_ref(nst_s.c_false);
            }
        }
        else if ( ob_t == nst_t.Type )
        {
            if ( ob == OBJ(nst_t.Null) )
            {
                return nst_inc_ref(nst_s.t_Null);
            }
            if ( ob == OBJ(nst_t.Int) )
            {
                return nst_inc_ref(nst_s.t_Int);
            }
            if ( ob == OBJ(nst_t.Real) )
            {
                return nst_inc_ref(nst_s.t_Real);
            }
            if ( ob == OBJ(nst_t.Bool) )
            {
                return nst_inc_ref(nst_s.t_Bool);
            }
            if ( ob == OBJ(nst_t.Str) )
            {
                return nst_inc_ref(nst_s.t_Str);
            }
            if ( ob == OBJ(nst_t.Map) )
            {
                return nst_inc_ref(nst_s.t_Map);
            }
            if ( ob == OBJ(nst_t.Vector) )
            {
                return nst_inc_ref(nst_s.t_Vector);
            }
            if ( ob == OBJ(nst_t.Array) )
            {
                return nst_inc_ref(nst_s.t_Array);
            }
            if ( ob == OBJ(nst_t.Byte) )
            {
                return nst_inc_ref(nst_s.t_Byte);
            }
            if ( ob == OBJ(nst_t.Type) )
            {
                return nst_inc_ref(nst_s.t_Type);
            }
            if ( ob == OBJ(nst_t.Iter) )
            {
                return nst_inc_ref(nst_s.t_Iter);
            }
            if ( ob == OBJ(nst_t.Func) )
            {
                return nst_inc_ref(nst_s.t_Func);
            }
            if ( ob == OBJ(nst_t.IOFile) )
            {
                return nst_inc_ref(nst_s.t_IOFile);
            }

            // for custom types defined in external libraries
            return nst_string_copy(ob, err);
        }
        else if ( ob_t == nst_t.Byte )
        {
            i8 *str = (i8 *)nst_calloc(2, sizeof(i8), NULL, err);
            CHECK_BUFFER(str);
            str[0] = AS_BYTE(ob);
            NST_RETURN_NEW_STR(str, 1);
        }
        else if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
        {
            Nst_LList *all_objs = nst_llist_new(err);
            if ( all_objs == NULL )
            {
                return NULL;
            }
            Nst_Obj *str = _nst_obj_str_cast_seq(SEQ(ob), all_objs, err);
            nst_llist_destroy(all_objs, NULL);
            return str;
        }
        else if ( ob_t == nst_t.Map )
        {
            Nst_LList *all_objs = nst_llist_new(err);
            if ( all_objs == NULL )
            {
                return NULL;
            }
            Nst_Obj *str = _nst_obj_str_cast_map(MAP(ob), all_objs, err);
            nst_llist_destroy(all_objs, NULL);
            return str;
        }
        else if ( ob_t == nst_t.Null )
        {
            return nst_inc_ref(nst_s.c_null);
        }
        else if ( ob_t == nst_t.IOFile )
        {
            i8 *buffer = (i8 *)nst_malloc(14, sizeof(i8), err);
            CHECK_BUFFER(buffer);
            memcpy(buffer, "<IOFile --- >", 14);
            if ( NST_IOF_CAN_READ(ob) )  buffer[8] = 'r';
            if ( NST_IOF_CAN_WRITE(ob) ) buffer[9] = 'w';
            if ( NST_IOF_IS_BIN(ob) )    buffer[10]= 'b';
            NST_RETURN_NEW_STR(buffer, 13);
        }
        else if ( ob_t == nst_t.Func )
        {
            i8 *buffer = (i8 *)nst_malloc(13 + MAX_INT_CHAR_COUNT, sizeof(i8), err);
            CHECK_BUFFER(buffer);
            i32 len;

            if ( FUNC(ob)->arg_num  == 1 )
            {
                memcpy(buffer, "<Func 1 arg>", 13);
                len = 12;
            }
            else
                len = sprintf(buffer, "<Func %zi args>", FUNC(ob)->arg_num);
            NST_RETURN_NEW_STR(buffer, len);
        }
        else
        {
            i8 *buffer = (i8 *)nst_malloc(
                STR(ob->type)->len + 16 + (i32)sizeof(usize) * 2,
                sizeof(i8),
                err);
            CHECK_BUFFER(buffer);
            i32 len = sprintf(
                buffer,
                "<%s object at 0x%0*zX>",
                STR(ob->type)->value,
                (int)sizeof(usize) * 2,
                (usize)ob);
            NST_RETURN_NEW_STR(buffer, len);
        }
    }
    else if ( type == nst_t.Bool )
    {
        if ( ob_t == nst_t.Int )
        {
            NST_RETURN_COND(AS_INT(ob) != 0);
        }
        else if ( ob_t == nst_t.Real )
        {
            NST_RETURN_COND(AS_REAL(ob) != 0.0);
        }
        else if ( ob_t == nst_t.Str )
        {
            NST_RETURN_COND(STR(ob)->len != 0);
        }
        else if ( ob_t == nst_t.Map )
        {
            NST_RETURN_COND(MAP(ob)->item_count != 0);
        }
        else if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
        {
            NST_RETURN_COND(SEQ(ob)->len != 0);
        }
        else if ( ob_t == nst_t.Null )
        {
            NST_RETURN_FALSE;
        }
        else if ( ob_t == nst_t.Byte )
        {
            NST_RETURN_COND(AS_BYTE(ob) != 0);
        }
        else if ( ob_t == nst_t.IOFile )
        {
            NST_RETURN_COND(!NST_IOF_IS_CLOSED(ob));
        }
        else
        {
            NST_RETURN_TRUE;
        }
    }
    else if ( type == nst_t.Int )
    {
        if ( ob_t == nst_t.Real )
        {
            return nst_int_new((Nst_Int)AS_REAL(ob), err);
        }
        else if ( ob_t == nst_t.Byte )
        {
            return nst_int_new((Nst_Int)AS_BYTE(ob), err);
        }
        else if ( ob_t == nst_t.Str )
        {
            return nst_string_parse_int(STR(ob), 0, err);
        }
        else
        {
            RETURN_CAST_TYPE_ERROR;
        }
    }
    else if ( type == nst_t.Real )
    {
        if ( ob_t == nst_t.Int )
        {
            return nst_real_new((Nst_Real)AS_INT(ob), err);
        }
        else if ( ob_t == nst_t.Byte )
        {
            return nst_real_new((Nst_Real)AS_BYTE(ob), err);
        }
        else if ( ob_t == nst_t.Str )
        {
            return nst_string_parse_real(STR(ob), err);
        }
        else
        {
            RETURN_CAST_TYPE_ERROR;
        }
    }
    else if ( type == nst_t.Byte )
    {
        if ( ob_t == nst_t.Int )
        {
            return nst_byte_new(AS_INT(ob) & 0xff, err);
        }
        else if ( ob_t == nst_t.Real )
        {
            return nst_byte_new((Nst_Int)AS_REAL(ob) & 0xff, err);
        }
        else if ( ob_t == nst_t.Str )
        {
            return nst_string_parse_byte(STR(ob), err);
        }
        else
        {
            RETURN_CAST_TYPE_ERROR;
        }
    }
    else if ( type == nst_t.Iter )
    {
        if ( ob_t == nst_t.Str )
        {
            Nst_Obj *idx = nst_int_new(0, err);
            Nst_SeqObj *data = SEQ(nst_array_new(2, err));
            nst_seq_set(data, 0, idx);
            nst_seq_set(data, 1, ob);
            nst_dec_ref(idx);

            if ( NST_ERROR_OCCURRED )
            {
                if ( idx  ) nst_dec_ref(idx);
                if ( data ) nst_dec_ref(data);
                return NULL;
            }

            Nst_Obj *start_obj   = nst_func_new_c(1, nst_iter_str_start, err);
            Nst_Obj *is_done_obj = nst_func_new_c(1, nst_iter_str_is_done, err);
            Nst_Obj *get_val_obj = nst_func_new_c(1, nst_iter_str_get_val, err);

            if ( NST_ERROR_OCCURRED )
            {
                if ( start_obj   ) nst_dec_ref(start_obj);
                if ( is_done_obj ) nst_dec_ref(is_done_obj);
                if ( get_val_obj ) nst_dec_ref(get_val_obj);
                nst_dec_ref(idx);
                nst_dec_ref(data);
                return NULL;
            }

            return nst_iter_new(
                FUNC(start_obj),
                FUNC(is_done_obj),
                FUNC(get_val_obj),
                OBJ(data),
                err);
        }
        else if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
        {
            Nst_Obj *idx = nst_int_new(0, err);
            Nst_SeqObj *data = SEQ(nst_array_new(2, err));
            nst_seq_set(data, 0, idx);
            nst_seq_set(data, 1, ob);
            nst_dec_ref(idx);

            if ( NST_ERROR_OCCURRED )
            {
                if ( idx  ) nst_dec_ref(idx);
                if ( data ) nst_dec_ref(data);
                return NULL;
            }

            Nst_Obj *start_obj   = nst_func_new_c(1, nst_iter_seq_start, err);
            Nst_Obj *is_done_obj = nst_func_new_c(1, nst_iter_seq_is_done, err);
            Nst_Obj *get_val_obj = nst_func_new_c(1, nst_iter_seq_get_val, err);

            if ( NST_ERROR_OCCURRED )
            {
                if ( start_obj   ) nst_dec_ref(start_obj);
                if ( is_done_obj ) nst_dec_ref(is_done_obj);
                if ( get_val_obj ) nst_dec_ref(get_val_obj);
                nst_dec_ref(idx);
                nst_dec_ref(data);
                return NULL;
            }

            return nst_iter_new(
                FUNC(start_obj),
                FUNC(is_done_obj),
                FUNC(get_val_obj),
                OBJ(data),
                err);
        }
        else if ( ob_t == nst_t.Map )
        {
            Nst_Obj *start_obj   = nst_map_get_str(ob, "_start_", err);
            Nst_Obj *is_done_obj = nst_map_get_str(ob, "_is_done_", err);
            Nst_Obj *get_val_obj = nst_map_get_str(ob, "_get_val_", err);

            if ( NST_ERROR_OCCURRED )
            {
                if ( start_obj ) nst_dec_ref(start_obj);
                if ( is_done_obj ) nst_dec_ref(is_done_obj);
                if ( get_val_obj ) nst_dec_ref(get_val_obj);
                return NULL;
            }

            if ( start_obj == NULL )
            {
                RETURN_MISSING_FUNC_ERROR("_start_");
            }
            if ( is_done_obj == NULL )
            {
                RETURN_MISSING_FUNC_ERROR("_is_done_");
            }
            if ( get_val_obj == NULL )
            {
                RETURN_MISSING_FUNC_ERROR("_get_val_");
            }

            return nst_iter_new(
                FUNC(start_obj),
                FUNC(is_done_obj),
                FUNC(get_val_obj),
                nst_inc_ref(ob),
                err);
        }
        else
        {
            RETURN_CAST_TYPE_ERROR;
        }
    }
    else if ( type == nst_t.Array || type == nst_t.Vector )
    {
        bool is_vect = type == nst_t.Vector;
        if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
        {
            usize seq_len = SEQ(ob)->len;
            Nst_SeqObj *seq = is_vect ? SEQ(nst_vector_new(seq_len, err))
                                      : SEQ(nst_array_new(seq_len, err));
            if ( seq == NULL )
            {
                return NULL;
            }
            for ( usize i = 0; i < seq_len; i++ )
            {
                nst_seq_set(seq, i, SEQ(ob)->objs[i]);
            }

            return OBJ(seq);
        }
        else if ( ob_t == nst_t.Str )
        {
            usize str_len = STR(ob)->len;
            Nst_SeqObj *seq = is_vect ? SEQ(nst_vector_new(str_len, err))
                                      : SEQ(nst_array_new(str_len, err));
            if ( seq == NULL )
            {
                return NULL;
            }
            for ( usize i = 0; i < str_len; i++ )
            {
                seq->objs[i] = nst_string_get(ob, i, err);
                if ( seq->objs[i] == NULL )
                {
                    for ( usize j = 0; j < i; j++ )
                    {
                        nst_dec_ref(seq->objs[i]);
                        nst_free(seq->objs);
                        nst_free(seq);
                        return NULL;
                    }
                }
            }
            return OBJ(seq);
        }
        else if ( ob_t == nst_t.Iter )
        {
            Nst_SeqObj *seq = SEQ(nst_vector_new(0, err));
            Nst_IterObj *iter = ITER(ob);
            if ( seq == NULL )
            {
                return NULL;
            }

            if ( nst_iter_start(iter, err) )
            {
                nst_dec_ref(seq);
                return NULL;
            }

            while ( true )
            {
                i32 is_done = nst_iter_is_done(iter, err);
                if ( is_done == -1 )
                {
                    nst_dec_ref(seq);
                    return NULL;
                }
                else if ( is_done )
                    break;

                Nst_Obj *result = nst_iter_get_val(iter, err);
                if (result == NULL)
                {
                    nst_dec_ref(seq);
                    return NULL;
                }
                nst_vector_append(seq, result, err);
                if ( NST_ERROR_OCCURRED )
                {
                    nst_dec_ref(seq);
                    return NULL;
                }
                nst_dec_ref(result);
            }

            if ( is_vect )
            {
                return OBJ(seq);
            }
            Nst_Obj **new_objs = (Nst_Obj **)nst_realloc(
                seq->objs,
                seq->len,
                sizeof(Nst_Obj *),
                seq->size,
                err);
            if ( new_objs != seq->objs )
            {
                seq->objs = new_objs;
                seq->size = seq->len;
            }

            seq->type = STR(nst_inc_ref(nst_t.Array));
            nst_dec_ref(nst_t.Vector);
            return OBJ(seq);
        }
        else if ( ob_t == nst_t.Map )
        {
            Nst_MapObj *map = MAP(ob);
            usize seq_len = map->item_count;
            Nst_SeqObj *seq = is_vect ? SEQ(nst_vector_new(seq_len, err))
                                      : SEQ(nst_array_new(seq_len, err));

            usize seq_i = 0;
            for ( i32 i = nst_map_get_next_idx(-1, map);
                  i != -1;
                  i = nst_map_get_next_idx(i, map) )
            {
                Nst_SeqObj *node_arr = SEQ(nst_array_new(2, err));
                if ( node_arr == NULL )
                {
                    return NULL;
                }
                node_arr->objs[0] = nst_inc_ref(map->nodes[i].key);
                node_arr->objs[1] = nst_inc_ref(map->nodes[i].value);
                seq->objs[seq_i++] = OBJ(node_arr);
            }

            return OBJ(seq);
        }
        else
        {
            RETURN_CAST_TYPE_ERROR;
        }
    }
    else if ( type == nst_t.Map )
    {
        if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
        {
            Nst_SeqObj *seq = SEQ(ob);
            Nst_Obj **objs = seq->objs;
            Nst_MapObj *map = MAP(nst_map_new(err));
            if ( map == NULL )
            {
                return NULL;
            }

            for ( usize i = 0, n = seq->len; i < n; i++ )
            {
                if ( objs[i]->type != nst_t.Array && objs[i]->type != nst_t.Vector )
                {
                    NST_SET_TYPE_ERROR(nst_sprintf(
                        _NST_EM_MAP_TO_SEQ_TYPE_ERR("index"),
                        TYPE_NAME(objs[i]), i));
                    nst_dec_ref(map);
                    return NULL;
                }

                if ( SEQ(objs[i])->len != 2 )
                {
                    NST_SET_TYPE_ERROR(nst_sprintf(
                        _NST_EM_MAP_TO_SEQ_LEN_ERR("index"),
                        SEQ(objs[i])->len, i));
                    nst_dec_ref(map);
                    return NULL;
                }

                if ( !nst_map_set(map, SEQ(objs[i])->objs[0], SEQ(objs[i])->objs[1]) )
                {
                    NST_SET_TYPE_ERROR(nst_sprintf(
                        _NST_EM_MAP_TO_SEQ_HASH("index"), i));
                    nst_dec_ref(map);
                    return NULL;
                }
            }

            return OBJ(map);
        }
        else if ( ob_t == nst_t.Iter )
        {
            Nst_IterObj *iter = ITER(ob);
            Nst_MapObj *map = MAP(nst_map_new(err));
            if ( map == NULL )
            {
                return NULL;
            }

            if ( nst_iter_start(iter, err) )
            {
                nst_dec_ref(map);
                return NULL;
            }

            usize iter_count = 1;

            while ( true )
            {
                i32 is_done = nst_iter_is_done(iter, err);
                if ( is_done == -1 )
                {
                    nst_dec_ref(map);
                    return NULL;
                }
                else if ( is_done )
                {
                    break;
                }

                Nst_SeqObj *result = SEQ(nst_iter_get_val(iter, err));
                if ( result == NULL )
                {
                    nst_dec_ref(map);
                    return NULL;
                }

                if ( result->type != nst_t.Array && result->type != nst_t.Vector )
                {
                    NST_SET_TYPE_ERROR(nst_sprintf(
                        _NST_EM_MAP_TO_SEQ_TYPE_ERR("iteration"),
                        TYPE_NAME(result), iter_count));
                    nst_dec_ref(map);
                    return NULL;
                }

                if ( result->len != 2 )
                {
                    NST_SET_TYPE_ERROR(nst_sprintf(
                        _NST_EM_MAP_TO_SEQ_LEN_ERR("iteration"),
                        result->len, iter_count));
                    nst_dec_ref(map);
                    return NULL;
                }

                if ( !nst_map_set(map, result->objs[0], result->objs[1]) )
                {
                    NST_SET_TYPE_ERROR(nst_sprintf(
                        _NST_EM_MAP_TO_SEQ_HASH("iteration"),
                        iter_count));
                    nst_dec_ref(map);
                    return NULL;
                }
                nst_dec_ref(result);
                if ( NST_ERROR_OCCURRED )
                {
                    nst_dec_ref(map);
                    return NULL;
                }
                iter_count++;
            }

            return OBJ(map);
        }
        else
            RETURN_CAST_TYPE_ERROR;
    }
    else
        RETURN_CAST_TYPE_ERROR;
}

Nst_Obj *_nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    ob1 = nst_obj_cast(ob1, nst_t.Str, err);
    ob2 = nst_obj_cast(ob2, nst_t.Str, err);

    Nst_StrObj *nst_s1 = STR(ob1);
    Nst_StrObj *nst_s2 = STR(ob2);

    i8 *s1 = nst_s1->value;
    i8 *s2 = nst_s2->value;
    usize len1 = nst_s1->len;
    usize len2 = nst_s2->len;
    usize tot_len = len1 + len2;

    i8 *buffer = (i8 *)nst_malloc(tot_len + 1, sizeof(i8), err);

    CHECK_BUFFER(buffer);

    memcpy(buffer, s1, len1);
    memcpy(buffer + len1, s2, len2);
    buffer[tot_len] = '\0';

    Nst_Obj *new_obj = nst_string_new(buffer, tot_len, true, err);

    nst_dec_ref(ob1);
    nst_dec_ref(ob2);

    return new_obj;
}

Nst_Obj *_nst_obj_range(Nst_Obj *start, Nst_Obj *stop, Nst_Obj *step, Nst_OpErr *err)
{
    if ( AS_INT(step) == 0 )
    {
        NST_SET_RAW_VALUE_ERROR(_NST_EM_RANGE_STEP_ZERO);
        return NULL;
    }

    Nst_Obj *idx = nst_int_new(0, err);
    Nst_SeqObj *data_seq = SEQ(nst_array_new(4, err));

    if ( idx == NULL || data_seq == NULL )
    {
        if ( idx ) nst_dec_ref(idx);
        if ( data_seq ) nst_dec_ref(data_seq);
        return NULL;
    }

    data_seq->objs[0] = idx;
    data_seq->objs[1] = nst_inc_ref(start);
    data_seq->objs[2] = nst_inc_ref(stop);
    data_seq->objs[3] = nst_inc_ref(step);

    Nst_Obj *start_func = nst_func_new_c(1, nst_iter_range_start, err);
    Nst_Obj *is_done_func = nst_func_new_c(1, nst_iter_range_is_done, err);
    Nst_Obj *get_val_func = nst_func_new_c(1, nst_iter_range_get_val, err);

    if ( NST_ERROR_OCCURRED )
    {
        if ( start_func ) nst_dec_ref(start_func);
        if ( is_done_func ) nst_dec_ref(is_done_func);
        if ( get_val_func ) nst_dec_ref(get_val_func);
        return NULL;
    }

    return nst_iter_new(
        FUNC(start_func),
        FUNC(is_done_func),
        FUNC(get_val_func),
        OBJ(data_seq),
        err);
}

// Local operations
Nst_Obj *_nst_obj_neg(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type == nst_t.Byte )
    {
        return nst_byte_new(-AS_BYTE(ob), err);
    }
    else if ( ob->type == nst_t.Int )
    {
        return nst_int_new(-AS_INT(ob), err);
    }
    else if ( ob->type == nst_t.Real )
    {
        return nst_real_new(-AS_REAL(ob), err);
    }
    else
        RETURN_LOCAL_OP_TYPE_ERROR("-:");
}

Nst_Obj *_nst_obj_len(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type == nst_t.Str )
    {
        return nst_int_new(STR(ob)->len, err);
    }
    else if ( ob->type == nst_t.Map )
    {
        return nst_int_new(MAP(ob)->item_count, err);
    }
    else if ( IS_SEQ(ob) )
    {
        return nst_int_new(SEQ(ob)->len, err);
    }
    else if ( ob->type == nst_t.Func )
    {
        return nst_int_new(FUNC(ob)->arg_num, err);
    }
    else
    {
        RETURN_LOCAL_OP_TYPE_ERROR("$");
    }
}

Nst_Obj *_nst_obj_bwnot(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type == nst_t.Byte )
    {
        return nst_byte_new(~AS_BYTE(ob), err);
    }
    else if ( ob->type == nst_t.Int )
    {
        return nst_int_new(~AS_INT(ob), err);
    }
    else
    {
        RETURN_LOCAL_OP_TYPE_ERROR("~");
    }
}

Nst_Obj *_nst_obj_lgnot(Nst_Obj *ob, Nst_OpErr *err)
{
    ob = nst_obj_cast(ob, nst_t.Bool, err);

    if ( ob == nst_c.Bool_true )
    {
        nst_dec_ref(nst_c.Bool_true);
        NST_RETURN_FALSE;
    }
    else
    {
        nst_dec_ref(nst_c.Bool_false);
        NST_RETURN_TRUE;
    }
}

Nst_Obj *_nst_obj_stdout(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( NST_IOF_IS_CLOSED(nst_io->out) )
    {
        return nst_inc_ref(ob);
    }

    Nst_Obj *str = nst_obj_cast(ob, nst_t.Str, err);

    nst_fwrite(STR(str)->value, sizeof(i8), STR(str)->len, nst_io->out);

    nst_dec_ref(str);
    return nst_inc_ref(ob);
}

static inline i8 get_one_char()
{
    i8 ch;
    usize chars_read = nst_io->in->read_f(&ch, sizeof(i8), 1, nst_io->in->value);
    if ( chars_read == 1 )
        return ch;
    else
        return 0;
}

Nst_Obj *_nst_obj_stdin(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( NST_IOF_IS_CLOSED(nst_io->in) )
    {
        return nst_string_new_c("", 0, false, err);
    }

    ob = nst_obj_cast(ob, nst_t.Str, err);
    nst_print(STR(ob)->value);
    nst_fflush(nst_io->out);
    nst_dec_ref(ob);

    Nst_Buffer buf;
    if ( !nst_buffer_init(&buf, 4, err) )
    {
        return NULL;
    }

    for ( i8 ch = get_one_char(); ch && ch != '\n'; ch = get_one_char() )
    {
        if ( ch == '\r' )
        {
            continue;
        }

        if ( !nst_buffer_append_char(&buf, ch, err) )
        {
            nst_buffer_destroy(&buf);
            return NULL;
        }
    }

    return OBJ(nst_buffer_to_string(&buf, err));
}

Nst_Obj *_nst_obj_import(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type != nst_t.Str )
    {
        NST_SET_TYPE_ERROR(nst_sprintf(
            _NST_EM_EXPECTED_TYPE("Str"),
            TYPE_NAME(ob)
        ));
        return NULL;
    }

    i8 *file_name = STR(ob)->value;
    usize file_name_len = STR(ob)->len;
    bool c_import = false;

    if ( STR(ob)->len > 6 &&
        file_name[0] == '_' && file_name[1] == '_' &&
        file_name[2] == 'C' && file_name[3] == '_' &&
        file_name[4] == '_' && file_name[5] == ':' )
    {
        c_import = true;
        file_name += 6; // skip __C__:
        file_name_len -= 6;
    }

    Nst_StrObj *import_path = _nst_get_import_path(file_name, file_name_len, err);
    if ( import_path == NULL )
    {
        NST_SET_VALUE_ERROR(nst_sprintf(
            _NST_EM_FILE_NOT_FOUND,
            file_name
        ));
        return NULL;
    }

    // Check if the module is in the import stack
    for ( Nst_LLNode *n = nst_state.lib_paths->head; n != NULL; n = n->next )
    {
        if ( nst_string_compare(import_path, STR(n->value)) == 0 )
        {
            nst_dec_ref(import_path);
            NST_SET_RAW_IMPORT_ERROR(_NST_EM_CIRC_IMPORT);
            return NULL;
        }
    }

    Nst_Obj *obj_map = nst_map_get(nst_state.lib_handles, import_path);
    if ( obj_map != NULL )
    {
        nst_dec_ref(import_path);
        return obj_map;
    }

    nst_llist_push(nst_state.lib_paths, import_path, false, err);
    if ( NST_ERROR_OCCURRED )
    {
        nst_dec_ref(import_path);
        return NULL;
    }

    if ( !c_import )
        return import_nest_lib(import_path, err);
    else
        return import_c_lib(import_path, err);
}

static void add_to_handle_map(Nst_StrObj     *path,
                              Nst_MapObj     *map,
                              Nst_SourceText *src_txt,
                              Nst_OpErr      *err)
{
    nst_llist_push(nst_state.lib_srcs, src_txt, true, err);
    if ( NST_ERROR_OCCURRED )
    {
        nst_dec_ref(path);
        return;
    }
    if ( !nst_map_set(nst_state.lib_handles, path, map) )
    {
        NST_FAILED_ALLOCATION;
    }
    nst_dec_ref(path);
}

static Nst_Obj *import_nest_lib(Nst_StrObj *file_path, Nst_OpErr *err)
{
    Nst_SourceText *lib_src =
        (Nst_SourceText *)nst_malloc(1, sizeof(Nst_SourceText), err);

    if ( lib_src == NULL )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        return NULL;
    }

    if ( nst_run_module(file_path->value, lib_src) == -1 )
    {
        nst_llist_push(nst_state.lib_srcs, lib_src, true, err);
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        return NULL;
    }

    Nst_MapObj *map = MAP(nst_vstack_pop(nst_state.v_stack));

    add_to_handle_map(file_path, map, lib_src, err);
    nst_llist_pop(nst_state.lib_paths);
    return OBJ(map);
}

static Nst_Obj *import_c_lib(Nst_StrObj *file_path, Nst_OpErr *err)
{
    void (*free_lib_func)();
    lib_t lib = dlopen(file_path->value);

    if ( !lib )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
#ifdef WINDOWS
        NST_SET_RAW_IMPORT_ERROR(_NST_EM_FILE_NOT_DLL);
#else
        NST_SET_RAW_IMPORT_ERROR(dlerror());
#endif
        return NULL;
    }

    // Initialize library
    bool (*lib_init)() = (bool (*)())dlsym(lib, "lib_init");
    if ( lib_init == NULL )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        NST_SET_RAW_IMPORT_ERROR(_NST_EM_NO_LIB_FUNC("lib_init"));
        dlclose(lib);
        return NULL;
    }

    if ( !lib_init() )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        dlclose(lib);
        return NULL;
    }

    // Get function pointers
    Nst_DeclrList *(*get_func_ptrs)() =
        (Nst_DeclrList *(*)())dlsym(lib, "get_func_ptrs");
    if ( get_func_ptrs == NULL )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        NST_SET_RAW_IMPORT_ERROR(_NST_EM_NO_LIB_FUNC("get_func_ptrs"));
        goto fail;
    }

    Nst_DeclrList *obj_ptrs = get_func_ptrs();

    if ( obj_ptrs == NULL )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        NST_SET_RAW_IMPORT_ERROR(_NST_EM_LIB_INIT_FAILED);
        goto fail;
    }

    // Populate the function map
    Nst_MapObj *obj_map = MAP(nst_map_new(err));
    if ( obj_map == NULL )
    {
        goto fail;
    }

    for ( usize i = 0, n = obj_ptrs->obj_count; i < n; i++ )
    {
        Nst_ObjDeclr obj_declr = obj_ptrs->objs[i];
        Nst_Obj *obj;
        if ( obj_declr.arg_num >= 0 )
        {
            obj = nst_func_new_c(obj_declr.arg_num, obj_declr.ptr, err);
        }
        else
        {
            obj = nst_inc_ref(obj_declr.ptr);
        }

        if ( obj == NULL )
        {
            nst_dec_ref(obj_map);
            goto fail;
        }

        if ( !nst_map_set(obj_map, obj_declr.name, obj) )
        {
            NST_FAILED_ALLOCATION;
            nst_dec_ref(obj_map);
            nst_dec_ref(obj_declr.name);
            nst_dec_ref(obj);
            goto fail;
        }

        nst_dec_ref(obj_declr.name);
        nst_dec_ref(obj);
    }
    nst_llist_append(nst_state.loaded_libs, lib, true, err);

    add_to_handle_map(file_path, obj_map, NULL, err);
    nst_llist_pop(nst_state.lib_paths);
    return OBJ(obj_map);
fail:
    free_lib_func = (void (*)())dlsym(lib, "free_lib");
    if ( free_lib_func )
    {
        free_lib_func();
    }
    dlclose(lib);
    return NULL;
}

Nst_StrObj *_nst_get_import_path(i8 *initial_path, usize path_len, Nst_OpErr *err)
{
    i8 *file_path;
    usize new_len = nst_get_full_path(initial_path, &file_path, NULL, NULL);
    Nst_IOFile file;

    if ( file_path != NULL && (file = fopen(file_path, "r")) != NULL )
    {
        fclose(file);
        Nst_Obj *path_str = nst_string_new(file_path, new_len, true, err);
        if ( path_str == NULL )
        {
            nst_free(file_path);
        }
        return STR(path_str);
    }

    if ( file_path != NULL )
    {
        nst_free(file_path);
    }

#ifdef WINDOWS
 #ifdef _DEBUG

    usize root_len = strlen(__FILE__) - 13;
    usize nest_file_len = 17;
    const i8 *obj_ops_path = __FILE__;
    const i8 *nest_files = "libs/_nest_files/";
    usize full_size = path_len + nest_file_len + root_len;
    file_path = (i8 *)nst_malloc((full_size + 1), sizeof(i8), err);
    memcpy(file_path, obj_ops_path, root_len);
    memcpy(file_path + root_len, nest_files, nest_file_len);
    memcpy(file_path + root_len + nest_file_len, initial_path, path_len);
    file_path[full_size] = '\0';
 #else
    // In Windows the standard library is stored in %LOCALAPPDATA%/Programs/nest/nest_libs

    i8 *appdata = getenv("LOCALAPPDATA");
    if ( appdata == NULL )
    {
        return NULL;
    }

    usize appdata_len = strlen(appdata);
    file_path = (i8 *)nst_malloc(appdata_len + path_len + 26, sizeof(i8), err);
    if ( !file_path )
    {
        return NULL;
    }
    sprintf(file_path, "%s/Programs/nest/nest_libs/%s", appdata, initial_path);
 #endif
#else

    // In UNIX the standard library is stored in /usr/lib/nest
    file_path = (i8 *)nst_malloc(path_len + 15, sizeof(i8), err);
    if ( !file_path )
    {
        return NULL;
    }
    sprintf(file_path, "/usr/lib/nest/%s", initial_path);

#endif

    if ( (file = fopen(file_path, "r")) == NULL )
    {
        nst_free(file_path);
        return NULL;
    }
    fclose(file);

    i8 *abs_path;
    new_len = nst_get_full_path(file_path, &abs_path, NULL, err);
    nst_free(file_path);
    if ( abs_path == NULL )
    {
        return NULL;
    }
    return STR(nst_string_new(abs_path, new_len, true, err));
}

#ifdef WINDOWS
#pragma warning( disable: 4100 )
#endif

Nst_Obj *_nst_obj_typeof(Nst_Obj *ob, Nst_OpErr *err)
{
    return nst_inc_ref(ob->type);
}
