#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include "obj_ops.h"
#include "nst_types.h"
#include "interpreter.h"
#include "lib_import.h"
#include "map.h"
#include "error.h"
#include "lib_import.h"

#if defined(_WIN32) || defined(WIN32)

#include <windows.h>
#define dlsym GetProcAddress

#else

#include <dlfcn.h>

#endif

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
    NST_SET_TYPE_ERROR(nst_format_error( \
        "invalid types '%s' and '%s' for '" operand "'", \
        "ss", \
        TYPE_NAME(ob1), TYPE_NAME(ob2))); \
    return NULL; \
    } while ( 0 )

#define RETURN_CAST_TYPE_ERROR do { \
    NST_SET_TYPE_ERROR(nst_format_error( \
        _NST_EM_INVALID_CASTING, \
        "ss", \
        TYPE_NAME(ob), STR(type)->value)); \
    return NULL; \
    } while ( 0 )

#define RETURN_LOCAL_OP_TYPE_ERROR(operand) do { \
    NST_SET_TYPE_ERROR(nst_format_error( \
        _NST_EM_INVALID_OPERAND_TYPE(operand), \
        "s", \
        TYPE_NAME(ob))); \
    return NULL; \
    } while ( 0 )

#define RETURN_MISSING_FUNC_ERROR(func) NST_SET_RAW_VALUE_ERROR(_NST_EM_MISSING_FUNC(func))

#define CHECK_BUFFER(buf) do { \
        if ( buf == NULL ) \
        { \
            errno = ENOMEM; \
            return NULL; \
        } \
    } while (0)

#define OBJ_INIT_FARGS \
    Nst_TypeObjs, Nst_StrConsts, Nst_Consts, Nst_StdStreams*, Nst_ExecutionState


static Nst_Obj* map_eq(Nst_MapObj* map1, Nst_MapObj* map2, Nst_LList* containers);
static Nst_Obj *seq_eq(Nst_SeqObj *seq1, Nst_SeqObj *seq2, Nst_LList *containers);
static Nst_Obj *import_nest_lib(Nst_StrObj *file_path);
static Nst_Obj *import_c_lib(Nst_StrObj *file_path, Nst_OpErr *err);
static void add_to_handle_map(Nst_StrObj *path,
                              Nst_MapObj *map,
                              Nst_SourceText *src_txt);

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
        Nst_LList *containers = nst_llist_new();
        Nst_Obj *res = seq_eq(SEQ(ob1), SEQ(ob2), containers);
        nst_llist_destroy(containers, NULL);
        return res;
    }
    else if ( ARE_TYPE(nst_t.Map) )
    {
        Nst_LList *containers = nst_llist_new();
        Nst_Obj *res = map_eq(MAP(ob1), MAP(ob2), containers);
        nst_llist_destroy(containers, NULL);
        return res;
    }
    else
    {
        NST_RETURN_FALSE;
    }
}

static Nst_Obj *seq_eq(Nst_SeqObj *seq1, Nst_SeqObj *seq2, Nst_LList *containers)
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

    nst_llist_append(containers, seq1, false);
    nst_llist_append(containers, seq2, false);

    Nst_Obj *ob1 = NULL;
    Nst_Obj *ob2 = NULL;
    Nst_Obj *result = NULL;
    for (size_t i = 0, n = seq1->len; i < n; i++)
    {
        ob1 = seq1->objs[i];
        ob2 = seq2->objs[i];

        if ( IS_SEQ(ob1) && IS_SEQ(ob2) )
        {
            result = seq_eq(SEQ(ob1), SEQ(ob2), containers);
        }
        else if ( ARE_TYPE(nst_t.Map) )
        {
            result = map_eq(MAP(ob1), MAP(ob2), containers);
        }
        else
        {
            result = nst_obj_eq(ob1, ob2, NULL);
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

static Nst_Obj* map_eq(Nst_MapObj* map1, Nst_MapObj* map2, Nst_LList* containers)
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

    nst_llist_append(containers, map1, false);
    nst_llist_append(containers, map2, false);

    Nst_Obj *key = NULL;
    Nst_Obj *ob1 = NULL;
    Nst_Obj *ob2 = NULL;
    Nst_Obj *result = NULL;
    for ( int i = nst_map_get_next_idx(-1, map1);
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
            result = seq_eq(SEQ(ob1), SEQ(ob2), containers);
        }
        else if ( ARE_TYPE(nst_t.Map) )
        {
            result = map_eq(MAP(ob1), MAP(ob2), containers);
        }
        else
        {
            result = nst_obj_eq(ob1, ob2, NULL);
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
    if ( nst_obj_eq(ob1, ob2, err) == nst_c.Bool_true )
    {
        return nst_c.Bool_true;
    }
    else
    {
        nst_dec_ref(nst_c.Bool_false);
    }

    Nst_Obj *res = nst_obj_gt(ob1, ob2, err);

    if ( err->name == nst_s.e_TypeError )
    {
        nst_dec_ref(err->name);
        nst_dec_ref(err->message);
        RETURN_STACK_OP_TYPE_ERROR(">=");
    }
    return res;
}

Nst_Obj *_nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( nst_obj_eq(ob1, ob2, err) == nst_c.Bool_true )
    {
        return nst_c.Bool_true;
    }
    else
    {
        nst_dec_ref(nst_c.Bool_false);
    }

    Nst_Obj *res = nst_obj_lt(ob1, ob2, err);

    if ( err->name == nst_s.e_TypeError )
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
        nst_vector_append(ob1, ob2);
        return nst_inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) + AS_BYTE(ob2));
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) + AS_INT(ob2));

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);

        Nst_Obj *new_obj = nst_real_new(AS_REAL(ob1) + AS_REAL(ob2));

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
            NST_SET_TYPE_ERROR(nst_format_error(
                _NST_EM_UNHASHABLE_TYPE,
                "s",
                TYPE_NAME(ob2)));
            return NULL;
        }

        nst_dec_ref(res);
        return nst_inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) - AS_BYTE(ob2));
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) - AS_INT(ob2));

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);

        Nst_Obj *new_obj = nst_real_new(AS_REAL(ob1) - AS_REAL(ob2));

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
        Nst_SeqObj *vect = SEQ(ob1);
        size_t max_ob = vect->len;

        for ( Nst_Int i = 0, n = AS_INT(ob2) - 1; i < n; i++ )
        {
            for ( size_t j = 0; j < max_ob; j++ )
            {
                nst_vector_append(vect, vect->objs[j]);
            }
        }

        if ( AS_INT(ob2) == 0 )
        {
            return nst_vector_new(0);
        }

        return nst_inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t.Byte) )
    {
        return nst_byte_new(AS_BYTE(ob1) * AS_BYTE(ob2));
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) * AS_INT(ob2));

        nst_dec_ref(ob1);
        nst_dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Real, err);
        ob2 = nst_obj_cast(ob2, nst_t.Real, err);

        Nst_Obj *new_obj = nst_real_new(AS_REAL(ob1) * AS_REAL(ob2));

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
        return nst_vector_pop(ob1, (size_t)AS_INT(ob2));
    }
    else if ( ARE_TYPE(nst_t.Byte) )
    {
        if ( AS_BYTE(ob2) == 0 )
        {
            NST_SET_RAW_MATH_ERROR(_NST_EM_DIVISION_BY_ZERO);
            return NULL;
        }

        return nst_byte_new(AS_BYTE(ob1) / AS_BYTE(ob2));
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

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) / AS_INT(ob2));

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

        Nst_Obj *new_obj = nst_real_new(AS_REAL(ob1) / AS_REAL(ob2));

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

        return nst_byte_new(res);
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

        Nst_Obj *new_obj = nst_int_new(res);

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

        return nst_real_new((Nst_Real)powl(v1, v2));
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

        return nst_byte_new(AS_BYTE(ob1) % AS_BYTE(ob2));
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

        Nst_Obj *new_obj = nst_int_new(AS_INT(ob1) % AS_INT(ob2));

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

        Nst_Obj *new_obj = nst_real_new(fmod(AS_REAL(ob1), AS_REAL(ob2)));

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
        return nst_byte_new(AS_BYTE(ob1) | AS_BYTE(ob2));
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) | AS_INT(ob2));

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
    if (ARE_TYPE(nst_t.Byte))
        return nst_byte_new(AS_BYTE(ob1) & AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2))
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) & AS_INT(ob2));

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
    if (ARE_TYPE(nst_t.Byte))
        return nst_byte_new(AS_BYTE(ob1) ^ AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2))
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) ^ AS_INT(ob2));

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
    if (ARE_TYPE(nst_t.Byte))
        return nst_byte_new(AS_BYTE(ob1) << AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2))
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) << AS_INT(ob2));

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
    if (ARE_TYPE(nst_t.Byte))
        return nst_byte_new(AS_BYTE(ob1) >> AS_BYTE(ob2));
    else if (IS_INT(ob1) && IS_INT(ob2))
    {
        ob1 = nst_obj_cast(ob1, nst_t.Int, err);
        ob2 = nst_obj_cast(ob2, nst_t.Int, err);

        Nst_Obj* new_obj = nst_int_new(AS_INT(ob1) >> AS_INT(ob2));

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

Nst_Obj* _nst_repr_str_cast(Nst_Obj* ob)
{
    Nst_TypeObj *ob_t = ob->type;

    if ( ob_t == nst_t.Str )
    {
        return nst_string_repr(ob);
    }
    else if ( ob_t == nst_t.Byte )
    {
        char *str = (char*)calloc(5, sizeof(char));
        CHECK_BUFFER(str);

        sprintf(str, "%ib", (int)AS_BYTE(ob));

        return nst_string_new_c_raw((const char *)str, true);
    }
    else
    {
        return nst_obj_cast(ob, nst_t.Str, NULL);
    }
}

Nst_Obj *_nst_obj_str_cast_seq(Nst_Obj *seq_obj, Nst_LList *all_objs)
{
    bool is_vect = seq_obj->type == nst_t.Vector;

    for ( Nst_LLNode *n = all_objs->head; n != NULL; n = n->next )
    {
        if ( seq_obj == n->value )
        {
            if ( is_vect )
            {
                return nst_string_new_c("<{.}>", 5, false);
            }
            else
            {
                return nst_string_new_c("{.}", 3, false);
            }
        }
    }

    if ( SEQ(seq_obj)->len == 0 )
    {
        if ( is_vect )
        {
            return nst_string_new_c("<{}>", 4, false);
        }
        else
        {
            return nst_string_new_c("{,}", 3, false);
        }
    }

    nst_llist_push(all_objs, seq_obj, false);

    size_t len = SEQ(seq_obj)->len;
    size_t str_len = 0;
    Nst_Obj *val = NULL;

    char *str = (char *)malloc(sizeof(char) * (is_vect ? 6 : 4));
    char *realloc_str = NULL;
    CHECK_BUFFER(str);

    if ( is_vect )
    {
        str[0] = '<';
        str[1] = '{';
        str[2] = ' ';
        str_len = 3;
    }
    else
    {
        str[0] = '{';
        str[1] = ' ';
        str_len = 2;
    }

    for ( size_t i = 0; i < len; i++ )
    {
        val = SEQ(seq_obj)->objs[i];

        if ( IS_SEQ(val) )
        {
            val = _nst_obj_str_cast_seq(val, all_objs);
        }
        else if ( val->type == nst_t.Map )
        {
            val = _nst_obj_str_cast_map(val, all_objs);
        }
        else
        {
            val = _nst_repr_str_cast(val);
        }

        realloc_str = (char *)realloc(
            str,
            str_len
             + STR(val)->len
             + (is_vect && i == len - 1 ? 4 : 3));
        CHECK_BUFFER(realloc_str);
        str = realloc_str;
        memcpy(str + str_len, STR(val)->value, STR(val)->len);
        str_len += STR(val)->len + (is_vect && i == len - 1 ? 3 : 2);
        nst_dec_ref(val);

        if ( i == len - 1 )
        {
            break;
        }

        str[str_len - 2] = ',';
        str[str_len - 1] = ' ';
    }

    if ( is_vect )
    {
        str[str_len - 3] = ' ';
        str[str_len - 2] = '}';
        str[str_len - 1] = '>';
    }
    else
    {
        str[str_len - 2] = ' ';
        str[str_len - 1] = '}';
    }
    str[str_len] = 0;

    nst_llist_pop(all_objs);

    return nst_string_new(str, str_len, true);
}

Nst_Obj *_nst_obj_str_cast_map(Nst_Obj *map_obj, Nst_LList *all_objs)
{
    for ( Nst_LLNode *n = all_objs->head; n != NULL; n = n->next )
    {
        if ( map_obj == n->value )
        {
            return nst_string_new((char*)"{.}", 3, false);
        }
    }

    if ( MAP(map_obj)->item_count == 0 )
    {
        return nst_string_new_c("{}", 2, false);
    }

    nst_llist_push(all_objs, map_obj, false);

    size_t str_len = 2;
    Nst_Obj *key = NULL;
    Nst_Obj *val = NULL;

    char *str = (char *)malloc(sizeof(char) * 4);
    char *realloc_str = NULL;
    CHECK_BUFFER(str);

    str[0] = '{';
    str[1] = ' ';

    Nst_MapObj *map = MAP(map_obj);
    int idx = -1;
    size_t tot = map->item_count;
    size_t count = 0;

    while ( count++ < tot )
    {
        idx = nst_map_get_next_idx(idx, map);
        key = map->nodes[idx].key;
        val = map->nodes[idx].value;

        // Key cannot be a vector, an array or a map
        if ( key->type == nst_t.Str )
        {
            key = nst_string_repr(STR(key));
        }
        else
        {
            key = nst_obj_cast(key, nst_t.Str, NULL);
        }

        if ( IS_SEQ(val) )
        {
            val = _nst_obj_str_cast_seq(val, all_objs);
        }
        else if ( val->type == nst_t.Map )
        {
            val = _nst_obj_str_cast_map(val, all_objs);
        }
        else
        {
            val = _nst_repr_str_cast(val);
        }

        realloc_str = (char *)realloc(str, str_len + STR(key)->len + STR(val)->len + 5);
        CHECK_BUFFER(realloc_str);
        str = realloc_str;
        memcpy(str + str_len, STR(key)->value, STR(key)->len);
        str_len += STR(key)->len + 2;
        str[str_len - 2] = ':';
        str[str_len - 1] = ' ';
        memcpy(str + str_len, STR(val)->value, STR(val)->len);
        str_len += STR(val)->len + 2;
        nst_dec_ref(key);
        nst_dec_ref(val);

        if ( count == tot )
        {
            break;
        }

        str[str_len - 2] = ',';
        str[str_len - 1] = ' ';
    }

    if ( str_len == 2 )
    {
        str_len = 3;
    }
    str[str_len - 2] = ' ';
    str[str_len - 1] = '}';
    str[str_len] = 0;

    nst_llist_pop(all_objs);

    return nst_string_new(str, str_len, true);
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
            char *buffer = (char *)malloc(MAX_INT_CHAR_COUNT * sizeof(char));
            CHECK_BUFFER(buffer);
            int len = sprintf(buffer, "%lli", AS_INT(ob));
            return nst_string_new(buffer, len, true);
        }
        else if ( ob_t == nst_t.Real )
        {
            char *buffer = (char *)malloc(MAX_REAL_CHAR_COUNT * sizeof(char));
            CHECK_BUFFER(buffer);
            int len = sprintf(buffer, "%." REAL_PRECISION "lg", AS_REAL(ob));
            for ( int i = 0; i < len; i++ )
            {
                if ( buffer[i] == '.' || buffer[i] == 'e' )
                {
                    return nst_string_new(buffer, len, true);;
                }
            }
            buffer[len++] = '.';
            buffer[len++] = '0';
            buffer[len] = '\0';
            return nst_string_new(buffer, len, true);
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
            return nst_string_copy(ob);
        }
        else if ( ob_t == nst_t.Byte )
        {
            char *str = (char *)calloc(2, sizeof(char));
            CHECK_BUFFER(str);
            str[0] = AS_BYTE(ob);

            return nst_string_new(str, 1, true);
        }
        else if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
        {
            Nst_LList *all_objs = nst_llist_new();
            Nst_Obj *str = _nst_obj_str_cast_seq(ob, all_objs);
            nst_llist_destroy(all_objs, NULL);
            return str;
        }
        else if ( ob_t == nst_t.Map )
        {
            Nst_LList *all_objs = nst_llist_new();
            Nst_Obj *str = _nst_obj_str_cast_map(ob, all_objs);
            nst_llist_destroy(all_objs, NULL);
            return str;
        }
        else if ( ob_t == nst_t.Null )
        {
            return nst_inc_ref(nst_s.c_null);
        }
        else if ( ob_t == nst_t.IOFile )
        {
            char *buffer = (char *)malloc(sizeof(char) * 14);
            CHECK_BUFFER(buffer);
            memcpy(buffer, "<IOFile --- >", 14);
            if ( NST_IOF_CAN_READ(ob) )  buffer[8] = 'r';
            if ( NST_IOF_CAN_WRITE(ob) ) buffer[9] = 'w';
            if ( NST_IOF_IS_BIN(ob) )    buffer[10]= 'b';
            return nst_string_new(buffer, 13, true);
        }
        else if ( ob_t == nst_t.Func )
        {
            char *buffer = (char *)malloc(sizeof(char) * (13 + MAX_INT_CHAR_COUNT));
            CHECK_BUFFER(buffer);
            int len;

            if ( FUNC(ob)->arg_num  == 1 )
            {
                memcpy(buffer, "<Func 1 arg>", 13);
                len = 12;
            }
            else
                len = sprintf(buffer, "<Func %zi args>", FUNC(ob)->arg_num);
            return nst_string_new(buffer, len, true);
        }
        else
        {
            char *buffer = (char *)malloc(sizeof(char) * (STR(ob->type)->len + 12 + MAX_INT_CHAR_COUNT));
            CHECK_BUFFER(buffer);
            int len = sprintf(buffer, "<%s object at 0x%p>", STR(ob->type)->value, ob);
            return nst_string_new(buffer, len, true);
        }
    }
    else if ( type == nst_t.Bool )
    {
        if ( ob_t == nst_t.Int )
            NST_RETURN_COND(AS_INT(ob) != 0);
        else if ( ob_t == nst_t.Real )
            NST_RETURN_COND(AS_REAL(ob) != 0.0);
        else if ( ob_t == nst_t.Str )
            NST_RETURN_COND(STR(ob)->len != 0);
        else if ( ob_t == nst_t.Map )
            NST_RETURN_COND(MAP(ob)->item_count != 0);
        else if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
            NST_RETURN_COND(SEQ(ob)->len != 0);
        else if ( ob_t == nst_t.Null )
            NST_RETURN_FALSE;
        else if ( ob_t == nst_t.Byte )
            NST_RETURN_COND(AS_BYTE(ob) != 0);
        else if ( ob_t == nst_t.IOFile )
            NST_RETURN_COND(!NST_IOF_IS_CLOSED(ob));
        else
            NST_RETURN_TRUE;
    }
    else if ( type == nst_t.Int )
    {
        if ( ob_t == nst_t.Real )
            return nst_int_new((Nst_Int)AS_REAL(ob));
        else if ( ob_t == nst_t.Byte )
            return nst_int_new((Nst_Int)AS_BYTE(ob));
        else if ( ob_t == nst_t.Str )
            return nst_string_parse_int(STR(ob), 0, err);
        else
            RETURN_CAST_TYPE_ERROR;
    }
    else if ( type == nst_t.Real )
    {
        if ( ob_t == nst_t.Int )
            return nst_real_new((Nst_Real)AS_INT(ob));
        else if ( ob_t == nst_t.Byte )
            return nst_real_new((Nst_Real)AS_BYTE(ob));
        else if ( ob_t == nst_t.Str )
            return nst_string_parse_real(STR(ob), err);
        else
            RETURN_CAST_TYPE_ERROR;
    }
    else if ( type == nst_t.Byte )
    {
        if ( ob_t == nst_t.Int )
            return nst_byte_new(AS_INT(ob) & 0xff);
        else if ( ob_t == nst_t.Real )
            return nst_byte_new((Nst_Int)AS_REAL(ob) & 0xff);
        else if ( ob_t == nst_t.Str )
            return nst_string_parse_byte(STR(ob), err);
        else
            RETURN_CAST_TYPE_ERROR;
    }
    else if ( type == nst_t.Iter )
    {
        if ( ob_t == nst_t.Str )
        {
            Nst_Obj *idx = nst_int_new(0);
            Nst_SeqObj *data = SEQ(nst_array_new(2));
            nst_seq_set(data, 0, idx);
            nst_seq_set(data, 1, ob);
            nst_dec_ref(idx);

            return nst_iter_new(
                FUNC(nst_func_new_c(1, nst_iter_str_start)),
                FUNC(nst_func_new_c(1, nst_iter_str_is_done)),
                FUNC(nst_func_new_c(1, nst_iter_str_get_val)),
                OBJ(data)
            );
        }
        else if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
        {
            Nst_Obj *idx = nst_int_new(0);
            Nst_SeqObj *data = SEQ(nst_array_new(2));
            nst_seq_set(data, 0, idx);
            nst_seq_set(data, 1, ob);
            nst_dec_ref(idx);

            return nst_iter_new(
                FUNC(nst_func_new_c(1, nst_iter_seq_start)),
                FUNC(nst_func_new_c(1, nst_iter_seq_is_done)),
                FUNC(nst_func_new_c(1, nst_iter_seq_get_val)),
                OBJ(data)
            );
        }
        else if ( ob_t == nst_t.Map )
        {
            Nst_Obj *start_obj   = nst_map_get_str(ob, "_start_");
            Nst_Obj *is_done_obj = nst_map_get_str(ob, "_is_done_");
            Nst_Obj *get_val_obj = nst_map_get_str(ob, "_get_val_");

            if ( start_obj == NULL )
                RETURN_MISSING_FUNC_ERROR("_start_");
            if ( is_done_obj == NULL )
                RETURN_MISSING_FUNC_ERROR("_is_done_");
            if ( get_val_obj == NULL )
                RETURN_MISSING_FUNC_ERROR("_get_val_");

            return nst_iter_new(
                FUNC(start_obj),
                FUNC(is_done_obj),
                FUNC(get_val_obj),
                nst_inc_ref(ob)
            );
        }
        else
            RETURN_CAST_TYPE_ERROR;
    }
    else if ( type == nst_t.Array || type == nst_t.Vector )
    {
        bool is_vect = type == nst_t.Vector;
        if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
        {
            size_t seq_len = SEQ(ob)->len;
            Nst_SeqObj *seq = is_vect ? SEQ(nst_vector_new(seq_len))
                                      : SEQ(nst_array_new(seq_len));

            for ( size_t i = 0; i < seq_len; i++ )
                nst_seq_set(seq, i, SEQ(ob)->objs[i]);

            return OBJ(seq);
        }
        else if ( ob_t == nst_t.Str )
        {
            size_t str_len = STR(ob)->len;
            Nst_SeqObj *seq = is_vect ? SEQ(nst_vector_new(str_len))
                                      : SEQ(nst_array_new(str_len));

            for ( size_t i = 0; i < str_len; i++ )
                seq->objs[i] = nst_string_get(ob, i);
            return OBJ(seq);
        }
        else if ( ob_t == nst_t.Iter )
        {
            Nst_SeqObj *seq = SEQ(nst_vector_new(0));
            Nst_IterObj *iter = ITER(ob);

            if ( nst_iter_start(iter, err) )
            {
                nst_dec_ref(seq);
                return NULL;
            }

            while ( true )
            {
                int is_done = nst_iter_is_done(iter, err);
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
                
                nst_vector_append(seq, result);
                nst_dec_ref(result);
            }

            if ( is_vect )
                return OBJ(seq);
            
            Nst_Obj **new_objs = (Nst_Obj **)realloc(seq->objs, seq->len * sizeof(Nst_Obj *));
            if ( new_objs != NULL )
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
            size_t seq_len = map->item_count;
            Nst_SeqObj *seq = is_vect ? SEQ(nst_vector_new(seq_len))
                                      : SEQ(nst_array_new(seq_len));

            size_t seq_i = 0;
            for ( int i = nst_map_get_next_idx(-1, map);
                  i != -1;
                  i = nst_map_get_next_idx(i, map) )
            {
                Nst_SeqObj *node_arr = SEQ(nst_array_new(2));
                node_arr->objs[0] = nst_inc_ref(map->nodes[i].key);
                node_arr->objs[1] = nst_inc_ref(map->nodes[i].value);
                seq->objs[seq_i++] = OBJ(node_arr);
            }

            return OBJ(seq);
        }
        else
            RETURN_CAST_TYPE_ERROR;
    }
    else if ( type == nst_t.Map )
    {
        if ( ob_t == nst_t.Array || ob_t == nst_t.Vector )
        {
            Nst_SeqObj *seq = SEQ(ob);
            Nst_Obj **objs = seq->objs;
            Nst_MapObj *map = MAP(nst_map_new());

            for ( size_t i = 0, n = seq->len; i < n; i++ )
            {
                if ( objs[i]->type != nst_t.Array && objs[i]->type != nst_t.Vector )
                {
                    NST_SET_TYPE_ERROR(nst_format_error(
                        _NST_EM_MAP_TO_SEQ_TYPE_ERR("index"),
                        "su",
                        TYPE_NAME(objs[i]), i));
                    nst_dec_ref(map);
                    return NULL;
                }

                if ( SEQ(objs[i])->len != 2 )
                {
                    NST_SET_TYPE_ERROR(nst_format_error(
                        _NST_EM_MAP_TO_SEQ_LEN_ERR("index"),
                        "uu",
                        SEQ(objs[i])->len, i));
                    nst_dec_ref(map);
                    return NULL;
                }

                if ( !nst_map_set(map, SEQ(objs[i])->objs[0], SEQ(objs[i])->objs[1]) )
                {
                    NST_SET_TYPE_ERROR(nst_format_error(
                        _NST_EM_MAP_TO_SEQ_HASH("index"),
                        "u", i));
                    nst_dec_ref(map);
                    return NULL;
                }
            }

            return OBJ(map);
        }
        else if ( ob_t == nst_t.Iter )
        {
            Nst_IterObj *iter = ITER(ob);
            Nst_MapObj *map = MAP(nst_map_new());

            if ( nst_iter_start(iter, err) )
            {
                nst_dec_ref(map);
                return NULL;
            }

            size_t iter_count = 1;

            while ( true )
            {
                int is_done = nst_iter_is_done(iter, err);
                if ( is_done == -1 )
                {
                    nst_dec_ref(map);
                    return NULL;
                }
                else if ( is_done ) break;

                Nst_SeqObj *result = SEQ(nst_iter_get_val(iter, err));
                if (result == NULL)
                {
                    nst_dec_ref(map);
                    return NULL;
                }

                if (result->type != nst_t.Array && result->type != nst_t.Vector)
                {
                    NST_SET_TYPE_ERROR(nst_format_error(
                        _NST_EM_MAP_TO_SEQ_TYPE_ERR("iteration"),
                        "su",
                        TYPE_NAME(result), iter_count));
                    nst_dec_ref(map);
                    return NULL;
                }

                if (result->len != 2)
                {
                    NST_SET_TYPE_ERROR(nst_format_error(
                        _NST_EM_MAP_TO_SEQ_LEN_ERR("iteration"),
                        "uu",
                        result->len, iter_count));
                    nst_dec_ref(map);
                    return NULL;
                }

                if (!nst_map_set(map, result->objs[0], result->objs[1]))
                {
                    NST_SET_TYPE_ERROR(nst_format_error(
                        _NST_EM_MAP_TO_SEQ_HASH("iteration"),
                        "u", iter_count));
                    nst_dec_ref(map);
                    return NULL;
                }

                nst_dec_ref(result);
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

    char *s1 = nst_s1->value;
    char *s2 = nst_s2->value;
    size_t len1 = nst_s1->len;
    size_t len2 = nst_s2->len;
    size_t tot_len = len1 + len2;

    char *buffer = (char *)malloc(sizeof(char) * (tot_len + 1));

    CHECK_BUFFER(buffer);

    memcpy(buffer, s1, len1);
    memcpy(buffer + len1, s2, len2);
    buffer[tot_len] = '\0';

    Nst_Obj *new_obj = nst_string_new(buffer, tot_len, true);

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

    Nst_Obj *idx = nst_int_new(0);

    Nst_SeqObj *data_seq = SEQ(nst_array_new(4));
    data_seq->objs[0] = idx;
    data_seq->objs[1] = nst_inc_ref(start);
    data_seq->objs[2] = nst_inc_ref(stop);
    data_seq->objs[3] = nst_inc_ref(step);

    Nst_Obj *iter = nst_iter_new(
        FUNC(nst_func_new_c(1, nst_iter_range_start)),
        FUNC(nst_func_new_c(1, nst_iter_range_is_done)),
        FUNC(nst_func_new_c(1, nst_iter_range_get_val)),
        OBJ(data_seq)
    );

    return iter;
}

// Local operations
Nst_Obj *_nst_obj_neg(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type == nst_t.Int )
        return nst_int_new(-AS_INT(ob));
    else if ( ob->type == nst_t.Real )
        return nst_real_new(-AS_REAL(ob));
    else
        RETURN_LOCAL_OP_TYPE_ERROR("-");
}

Nst_Obj *_nst_obj_len(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type == nst_t.Str )
        return nst_int_new(STR(ob)->len);
    else if ( ob->type == nst_t.Map )
        return nst_int_new(MAP(ob)->item_count);
    else if ( IS_SEQ(ob) )
        return nst_int_new(SEQ(ob)->len);
    else
        RETURN_LOCAL_OP_TYPE_ERROR("$");
}

Nst_Obj *_nst_obj_bwnot(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type == nst_t.Int )
        return nst_int_new(~AS_INT(ob));
    else
        RETURN_LOCAL_OP_TYPE_ERROR("~");
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
        return nst_inc_ref(ob);

    Nst_Obj *str = nst_obj_cast(ob, nst_t.Str, err);

    nst_io->out->write_f(STR(str)->value, sizeof(char), STR(str)->len, nst_io->out->value);

    nst_io->out->flush_f(nst_io->out->value);
    nst_dec_ref(str);
    return nst_inc_ref(ob);
}

static inline char get_one_char()
{
    char ch;
    size_t chars_read = nst_io->in->read_f(&ch, sizeof(char), 1, nst_io->in->value);
    if ( chars_read == 1 )
        return ch;
    else
        return 0;
}

Nst_Obj *_nst_obj_stdin(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( NST_IOF_IS_CLOSED(nst_io->in) )
        return nst_string_new_c("", 0, false);

    ob = nst_obj_cast(ob, nst_t.Str, err);
    printf("%s", STR(ob)->value);
    fflush(stdout);
    nst_dec_ref(ob);

    char *buffer = (char *)malloc(4);
    CHECK_BUFFER(buffer);

    size_t buffer_size = 4;
    size_t i = 0;
    char ch = get_one_char();

    while ( ch != '\n' )
    {
        if ( ch == '\r' )
            continue;
        else if ( ch == '\0' )
            break;

        if ( buffer_size == i + 2 )
        {
            char *new_buffer = (char *)realloc(buffer, buffer_size *= 2);
            if ( new_buffer == NULL )
            {
                free(buffer);
                errno = ENOMEM;
                return NULL;
            }
            buffer = new_buffer;
        }

        buffer[i++] = ch;
        ch = get_one_char();
    }
    buffer[i] = '\0';
    char *new_buffer = (char *)realloc(buffer, i + 1);
    if ( new_buffer == NULL )
    {
        free(buffer);
        errno = ENOMEM;
        return NULL;
    }

    return nst_string_new(new_buffer, i, true);
}

Nst_Obj *_nst_obj_import(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type != nst_t.Str )
    {
        NST_SET_TYPE_ERROR(nst_format_error(
            _NST_EM_EXPECTED_TYPE("Str"),
            "s",
            TYPE_NAME(ob)
        ));
        return NULL;
    }

    char *file_name = STR(ob)->value;
    size_t file_name_len = STR(ob)->len;
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

    Nst_StrObj *file_path = _nst_get_import_path(file_name, file_name_len);
    if ( file_path == NULL )
    {
        NST_SET_VALUE_ERROR(nst_format_error(
            _NST_EM_FILE_NOT_FOUND,
            "s",
            file_name
        ));
        return NULL;
    }

    // Check if the module is in the import stack
    for ( Nst_LLNode *n = nst_state.lib_paths->head; n != NULL; n = n->next )
    {
        if ( nst_string_compare(file_path, STR(n->value)) == 0 )
        {
            nst_llist_pop(nst_state.lib_paths);
            free(file_path);
            NST_SET_RAW_IMPORT_ERROR(_NST_EM_CIRC_IMPORT);
            return NULL;
        }
    }

    Nst_Obj *func_map = nst_map_get(nst_state.lib_handles, file_path);
    if ( func_map != NULL )
        return func_map;

    nst_llist_push(nst_state.lib_paths, file_path, false);

    if ( !c_import )
        return import_nest_lib(file_path);
    else
        return import_c_lib(file_path, err);
}

static void add_to_handle_map(Nst_StrObj     *path,
                              Nst_MapObj     *map,
                              Nst_SourceText *src_txt)
{
    nst_llist_push(nst_state.lib_srcs, src_txt, true);
    nst_map_set(nst_state.lib_handles, path, map);
}

static Nst_Obj *import_nest_lib(Nst_StrObj *file_path)
{
    Nst_SourceText *lib_src = (Nst_SourceText *)malloc(sizeof(Nst_SourceText));

    if ( lib_src == NULL )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        return NULL;
    }

    if ( nst_run_module(file_path->value, lib_src) == -1 )
    {
        nst_llist_push(nst_state.lib_srcs, lib_src, true);
        nst_llist_pop(nst_state.lib_paths);
        return NULL;
    }

    Nst_MapObj *map = MAP(nst_vstack_pop(nst_state.v_stack));

    add_to_handle_map(file_path, map, lib_src);
    nst_llist_pop(nst_state.lib_paths);
    return OBJ(map);
}

static Nst_Obj *import_c_lib(Nst_StrObj *file_path, Nst_OpErr *err)
{
#if defined(_WIN32) || defined(WIN32)
    HMODULE lib = LoadLibraryA(file_path->value);
#else
    void *lib = dlopen(file_path->value, RTLD_LAZY);
#endif

    if ( !lib )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
#if defined(_WIN32) || defined(WIN32)
        NST_SET_RAW_IMPORT_ERROR(_NST_EM_FILE_NOT_DLL);
#else
        NST_SET_RAW_IMPORT_ERROR(dlerror());
#endif
        return NULL;
    }

    void (*init_lib_obj)(OBJ_INIT_FARGS)
        = (void (*)(OBJ_INIT_FARGS))dlsym(lib, "init_lib_obj");

    if ( init_lib_obj == NULL )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        NST_SET_RAW_IMPORT_ERROR(_NST_EM_NO_LIB_FUNC("init_lib_obj"));
        return NULL;
    }

    // Link the global variables
    init_lib_obj(nst_t, nst_s, nst_c, nst_io, nst_state);

    // Initialize library
    bool (*lib_init)() = (bool (*)())dlsym(lib, "lib_init");
    if ( lib_init == NULL )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        NST_SET_RAW_IMPORT_ERROR(_NST_EM_NO_LIB_FUNC("lib_init"));
        return NULL;
    }

    if ( !lib_init() )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        errno = ENOMEM;
        return NULL;
    }

    // Get function pointers
    Nst_FuncDeclr *(*get_func_ptrs)() = (Nst_FuncDeclr *(*)())dlsym(lib, "get_func_ptrs");
    if ( get_func_ptrs == NULL )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        NST_SET_RAW_IMPORT_ERROR(_NST_EM_NO_LIB_FUNC("get_func_ptrs"));
        return NULL;
    }

    Nst_FuncDeclr *func_ptrs = get_func_ptrs();

    if ( func_ptrs == NULL )
    {
        nst_llist_pop(nst_state.lib_paths);
        nst_dec_ref(file_path);
        NST_SET_RAW_IMPORT_ERROR(_NST_EM_LIB_INIT_FAILED);
        return NULL;
    }

    // Populate the function map
    Nst_MapObj *func_map = MAP(nst_map_new());

    for ( size_t i = 0;; i++ )
    {
        Nst_FuncDeclr func = func_ptrs[i];
        if ( func.func_ptr == NULL )
            break;

        Nst_Obj *func_obj = nst_func_new_c(func.arg_num, func.func_ptr);

        nst_map_set(func_map, func.name, func_obj);
        nst_dec_ref(func.name);
        nst_dec_ref(func_obj);
    }
    free(func_ptrs);
    nst_llist_append(nst_state.loaded_libs, lib, false);

    add_to_handle_map(file_path, func_map, NULL);
    nst_llist_pop(nst_state.lib_paths);
    return OBJ(func_map);
}

Nst_StrObj *_nst_get_import_path(char *initial_path, size_t path_len)
{
    char *file_path;
    size_t new_len = nst_get_full_path(initial_path, &file_path, NULL);
    Nst_IOFile file;

    if ( file_path != NULL && (file = fopen(file_path, "r")) != NULL )
    {
        fclose(file);
        return STR(nst_string_new(file_path, new_len, true));
    }

    if ( file_path != NULL )
        free(file_path);

#if defined(_WIN32) || defined(WIN32)
 #ifdef _DEBUG

    size_t root_len = strlen(__FILE__) - 13;
    size_t nest_file_len = 17;
    const char *obj_ops_path = __FILE__;
    const char* nest_files = "libs/_nest_files/";
    size_t full_size = path_len + nest_file_len + root_len;
    file_path = (char *)malloc((full_size + 1) * sizeof(char));
    memcpy(file_path, obj_ops_path, root_len);
    memcpy(file_path + root_len, nest_files, nest_file_len);
    memcpy(file_path + root_len + nest_file_len, initial_path, path_len);
    file_path[full_size] = '\0';
 #else
    // In Windows the standard library is stored in %LOCALAPPDATA%/Programs/nest/nest_libs

    char *appdata = getenv("LOCALAPPDATA");
    if ( appdata == NULL )
        return NULL;

    size_t appdata_len = strlen(appdata);
    file_path = (char *)malloc((appdata_len + path_len + 26) * sizeof(char));
    if ( !file_path ) return NULL;
    sprintf(file_path, "%s/Programs/nest/nest_libs/%s", appdata, initial_path);
 #endif
#else

    // In UNIX the standard library is stored in /usr/lib/nest
    file_path = (char *)malloc((path_len + 15) * sizeof(char));
    if ( !file_path ) return NULL;
    sprintf(file_path, "/usr/lib/nest/%s", initial_path);

#endif

    if ( (file = fopen(file_path, "r")) == NULL )
    {
        free(file_path);
        return NULL;
    }
    fclose(file);

    char *abs_path;
    new_len = nst_get_full_path(file_path, &abs_path, NULL);
    free(file_path);
    return STR(nst_string_new(abs_path, new_len, true));
}

#if defined(_WIN32) || defined(WIN32)
#pragma warning( disable: 4100 )
#endif

Nst_Obj *_nst_obj_typeof(Nst_Obj *ob, Nst_OpErr *err)
{
    return nst_inc_ref(ob->type);
}
