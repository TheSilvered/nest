#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include "obj_ops.h"
#include "nst_types.h"
#include "interpreter.h"
#include "lib_import.h"
#include "map.h"
#include "error.h"
#include "lib_import.h"

#define MAX_INT_CHAR_COUNT 21
#define MAX_REAL_CHAR_COUNT 25
#define MAX_BYTE_CHAR_COUNT 5

#define IS_NUM(obj) ( obj->type == nst_t_int || obj->type == nst_t_real || obj->type == nst_t_byte )
#define IS_INT(obj) ( obj->type == nst_t_int || obj->type == nst_t_byte )
#define IS_SEQ(obj) ( obj->type == nst_t_arr || obj->type == nst_t_vect )
#define ARE_TYPE(nst_type) ( ob1->type == nst_type && ob2->type == nst_type )

#define RETURN_TYPE_ERROR(operand) do { \
    err->name = NST_E_TYPE_ERROR; \
    err->message = "invalid type for '" operand "'"; \
    return NULL; } while (0)

#define RETURN_MISSING_FUNC_ERROR(func) do { \
    err->name = NST_E_VALUE_ERROR; \
    err->message = "missing '" func "' for a custom iterator"; \
    return NULL; } while (0)

#define CHECK_BUFFER(buf) do { \
        if ( buf == NULL ) \
        { \
            errno = ENOMEM; \
            return NULL; \
        } \
    } while (0)

#define OBJ_INIT_FARGS \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, \
    Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_Obj *, Nst_ExecutionState *

// Comparisons
Nst_Obj *nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1 == ob2 )
        NST_RETURN_TRUE;
    else if ( ob1->type == nst_t_map || ob2->type == nst_t_map )
        NST_RETURN_FALSE;
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_int, err);
        ob2 = nst_obj_cast(ob2, nst_t_int, err);
        bool check = AS_INT(ob1) == AS_INT(ob2);
        dec_ref(ob1);
        dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_real, err);
        ob2 = nst_obj_cast(ob2, nst_t_real, err);
        bool check = AS_REAL(ob1) == AS_REAL(ob2);
        dec_ref(ob1);
        dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else if ( ARE_TYPE(nst_t_str) )
        NST_RETURN_COND(strcmp(AS_STR(ob1)->value, AS_STR(ob2)->value) == 0);
    else if ( ARE_TYPE(nst_t_bool) )
        NST_RETURN_COND(ob1 == ob2);
    else if ( IS_SEQ(ob1) && IS_SEQ(ob2) )
    {
        if ( AS_SEQ(ob1)->len != AS_SEQ(ob2)->len )
            NST_RETURN_FALSE;

        for ( size_t i = 0, n = AS_SEQ(ob1)->len; i < n; i++ )
        {
            if ( nst_obj_eq(AS_SEQ(ob1)->objs[i], AS_SEQ(ob2)->objs[i], err) == nst_false )
                NST_RETURN_FALSE;
            else
                dec_ref(nst_true);
        }
        NST_RETURN_TRUE;
    }
    else
        NST_RETURN_FALSE;
}

Nst_Obj *nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( nst_obj_eq(ob1, ob2, err) == nst_true )
    {
        dec_ref(nst_true);
        NST_RETURN_FALSE;
    }
    else
    {
        dec_ref(nst_false);
        NST_RETURN_TRUE;
    }
}

Nst_Obj *nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t_str) )
        NST_RETURN_COND(strcmp(AS_STR(ob1)->value, AS_STR(ob2)->value) > 0);
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_int, err);
        ob2 = nst_obj_cast(ob2, nst_t_int, err);
        bool check = AS_INT(ob1) > AS_INT(ob2);
        dec_ref(ob1);
        dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_real, err);
        ob2 = nst_obj_cast(ob2, nst_t_real, err);
        bool check = AS_REAL(ob1) > AS_REAL(ob2);
        dec_ref(ob1);
        dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else
        RETURN_TYPE_ERROR(">");
}

Nst_Obj *nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t_str) )
        NST_RETURN_COND(strcmp(AS_STR(ob1)->value, AS_STR(ob2)->value) < 0);
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_int, err);
        ob2 = nst_obj_cast(ob2, nst_t_int, err);
        bool check = AS_INT(ob1) < AS_INT(ob2);
        dec_ref(ob1);
        dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_real, err);
        ob2 = nst_obj_cast(ob2, nst_t_real, err);
        bool check = AS_REAL(ob1) < AS_REAL(ob2);
        dec_ref(ob1);
        dec_ref(ob2);

        NST_RETURN_COND(check);
    }
    else
        RETURN_TYPE_ERROR("<");
}

Nst_Obj *nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( nst_obj_eq(ob1, ob2, err) == nst_true )
        return nst_true;

    Nst_Obj *res = nst_obj_gt(ob1, ob2, err);

    if ( strcmp(err->name, NST_E_TYPE_ERROR) == 0 )
        err->message = "invalid type for '>='";
    return res;
}

Nst_Obj *nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( nst_obj_eq(ob1, ob2, err) == nst_true )
        return nst_true;

    Nst_Obj *res = nst_obj_lt(ob1, ob2, err);

    if ( strcmp(err->name, NST_E_TYPE_ERROR) == 0 )
    {
        err->message = "invalid type for '<='";
    }
    return res;
}

// Arithmetic operations
Nst_Obj *nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1->type == nst_t_vect )
    {
        nst_append_value_vector(ob1, ob2);
        return inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t_byte) )
        return nst_new_byte(AS_BYTE(ob1) + AS_BYTE(ob2));
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_int, err);
        ob2 = nst_obj_cast(ob2, nst_t_int, err);

        Nst_Obj *new_obj = nst_new_int(AS_INT(ob1) + AS_INT(ob2));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_real, err);
        ob2 = nst_obj_cast(ob2, nst_t_real, err);

        Nst_Obj *new_obj = nst_new_real(AS_REAL(ob1) + AS_REAL(ob2));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
        RETURN_TYPE_ERROR("+");
}

Nst_Obj *nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1->type == nst_t_vect )
    {
        return nst_rem_value_vector(ob1, ob2);
    }
    else if ( ob1->type == nst_t_map )
    {
        Nst_Obj *res = nst_map_drop(ob1, ob2);
        if ( res == NULL )
        {
            err->name = NST_E_TYPE_ERROR;
            err->message= _nst_format_type_error(UNHASHABLE_TYPE, ob2->type_name);
        }
        return res;
    }
    else if ( ARE_TYPE(nst_t_byte) )
        return nst_new_byte(AS_BYTE(ob1) - AS_BYTE(ob2));
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_int, err);
        ob2 = nst_obj_cast(ob2, nst_t_int, err);

        Nst_Obj *new_obj = nst_new_int(AS_INT(ob1) - AS_INT(ob2));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_real, err);
        ob2 = nst_obj_cast(ob2, nst_t_real, err);

        Nst_Obj *new_obj = nst_new_real(AS_REAL(ob1) - AS_REAL(ob2));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
        RETURN_TYPE_ERROR("-");
}

Nst_Obj *nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1->type == nst_t_vect && ob2->type == nst_t_int )
    {
        Nst_SeqObj *vect = AS_SEQ(ob1);
        size_t max_ob = vect->len;

        for ( Nst_Int i = 0, n = AS_INT(ob2) - 1; i < n; i++ )
            for ( size_t j = 0; j < max_ob; j++ )
                nst_append_value_vector(vect, vect->objs[j]);

        if ( AS_INT(ob2) == 0 )
            return nst_new_vector(0);

        return inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t_byte) )
        return nst_new_byte(AS_BYTE(ob1) * AS_BYTE(ob2));
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_int, err);
        ob2 = nst_obj_cast(ob2, nst_t_int, err);

        Nst_Obj *new_obj = nst_new_int(AS_INT(ob1) * AS_INT(ob2));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_real, err);
        ob2 = nst_obj_cast(ob2, nst_t_real, err);

        Nst_Obj *new_obj = nst_new_real(AS_REAL(ob1) * AS_REAL(ob2));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
        RETURN_TYPE_ERROR("*");
}

Nst_Obj *nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ob1->type == nst_t_vect && ob2->type == nst_t_int )
        return nst_pop_value_vector(ob1, (size_t)AS_INT(ob2));
    else if ( ARE_TYPE(nst_t_byte) )
    {
        if ( AS_BYTE(ob2) == 0 )
        {
            err->name = NST_E_MATH_ERROR;
            err->message = "division by zero";
            return NULL;
        }

        return nst_new_byte(AS_BYTE(ob1) / AS_BYTE(ob2));
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_int, err);
        ob2 = nst_obj_cast(ob2, nst_t_int, err);

        if ( AS_INT(ob2) == 0 )
        {
            err->name = NST_E_MATH_ERROR;
            err->message = "division by zero";
            return NULL;
        }

        Nst_Obj *new_obj = nst_new_int(AS_INT(ob1) / AS_INT(ob2));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_real, err);
        ob2 = nst_obj_cast(ob2, nst_t_real, err);

        if ( AS_REAL(ob2) == 0.0 )
        {
            err->name = NST_E_MATH_ERROR;
            err->message = "division by zero";
            return NULL;
        }

        Nst_Obj *new_obj = nst_new_real(AS_REAL(ob1) / AS_REAL(ob2));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
        RETURN_TYPE_ERROR("/");
}

Nst_Obj *nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t_byte) )
    {
        register Nst_Byte res = 1;
        register Nst_Byte num = AS_BYTE(ob1);

        for ( Nst_Byte i = 0, n = AS_BYTE(ob2); i < n; i++ )
            res *= num;

        return nst_new_byte(res);
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_int, err);
        ob2 = nst_obj_cast(ob2, nst_t_int, err);

        register Nst_Int res = 1;
        register Nst_Int num = AS_INT(ob1);

        for ( Nst_Int i = 0, n = AS_INT(ob2); i < n; i++ )
            res *= num;

        Nst_Obj *new_obj = nst_new_int(res);

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_real, err);
        ob2 = nst_obj_cast(ob2, nst_t_real, err);

        Nst_Real v1 = AS_REAL(ob1);
        Nst_Real v2 = AS_REAL(ob2);

        dec_ref(ob1);
        dec_ref(ob2);

        // any root of a negative number gives -nan as a result
        if ( v1 < 0 && floor(v2) != v2 )
        {
            err->name = NST_E_MATH_ERROR;
            err->message = "fractional power of a negative number";
            return NULL;
        }

        return nst_new_real((Nst_Real)powl(v1, v2));
    }
    else
        RETURN_TYPE_ERROR("^");
}

Nst_Obj *nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t_byte) )
    {
        if ( AS_BYTE(ob2) == 0 )
        {
            err->name = NST_E_MATH_ERROR;
            err->message = "modulo by zero";
            return NULL;
        }

        return nst_new_byte(AS_BYTE(ob1) % AS_BYTE(ob2));
    }
    else if ( IS_INT(ob1) && IS_INT(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_int, err);
        ob2 = nst_obj_cast(ob2, nst_t_int, err);

        if ( AS_INT(ob2) == 0 )
        {
            err->name = NST_E_MATH_ERROR;
            err->message = "modulo by zero";
            return NULL;
        }

        Nst_Obj *new_obj = nst_new_int(AS_INT(ob1) % AS_INT(ob2));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else if ( IS_NUM(ob1) && IS_NUM(ob2) )
    {
        ob1 = nst_obj_cast(ob1, nst_t_real, err);
        ob2 = nst_obj_cast(ob2, nst_t_real, err);

        if ( AS_REAL(ob2) == 0.0 )
        {
            err->name = NST_E_MATH_ERROR;
            err->message = "modulo by zero";
            return NULL;
        }

        Nst_Obj *new_obj = nst_new_real(fmodl(AS_REAL(ob1), AS_REAL(ob2)));

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
        RETURN_TYPE_ERROR("%");
}

// Bitwise operations
Nst_Obj *nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
        return nst_new_int(AS_INT(ob1) | AS_INT(ob2));
    else
        RETURN_TYPE_ERROR("|");
}

Nst_Obj *nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
        return nst_new_int(AS_INT(ob1) & AS_INT(ob2));
    else
        RETURN_TYPE_ERROR("&");
}

Nst_Obj *nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
        return nst_new_int(AS_INT(ob1) ^ AS_INT(ob2));
    else
        RETURN_TYPE_ERROR("^^");
}

Nst_Obj *nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
        return nst_new_int(AS_INT(ob1) << AS_INT(ob2));
    else
        RETURN_TYPE_ERROR("<<");
}

Nst_Obj *nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
        return nst_new_int(AS_INT(ob1) >> AS_INT(ob2));
    else
        RETURN_TYPE_ERROR(">>");
}

// Logical operations
Nst_Obj *nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    ob1 = nst_obj_cast(ob1, nst_t_bool, err);
    ob2 = nst_obj_cast(ob2, nst_t_bool, err);

    Nst_Bool v1 = AS_BOOL(ob1);
    Nst_Bool v2 = AS_BOOL(ob2);

    dec_ref(ob1);
    dec_ref(ob2);

    NST_RETURN_COND( v1 == NST_TRUE || v2 == NST_TRUE );
}

Nst_Obj *nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    ob1 = nst_obj_cast(ob1, nst_t_bool, err);
    ob2 = nst_obj_cast(ob2, nst_t_bool, err);

    Nst_Bool v1 = AS_BOOL(ob1);
    Nst_Bool v2 = AS_BOOL(ob2);

    dec_ref(ob1);
    dec_ref(ob2);

    NST_RETURN_COND(v1 == NST_TRUE && v2 == NST_TRUE);
}

Nst_Obj *nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    ob1 = nst_obj_cast(ob1, nst_t_bool, err);
    ob2 = nst_obj_cast(ob2, nst_t_bool, err);

    Nst_Bool v1 = AS_BOOL(ob1);
    Nst_Bool v2 = AS_BOOL(ob2);

    dec_ref(ob1);
    dec_ref(ob2);

    NST_RETURN_COND((v1 == NST_TRUE && v2 == NST_FALSE) ||
                (v1 == NST_FALSE && v2 == NST_TRUE));
}

// Other
Nst_Obj *nst_obj_str_cast_seq(Nst_Obj *seq_obj, LList *all_objs)
{
    for ( LLNode *n = all_objs->head; n != NULL; n = n->next )
    {
        if ( seq_obj == n->value )
            return nst_new_string("{.}", 3, false);
    }

    LList_push(all_objs, seq_obj, false);
    bool is_vect = seq_obj->type == nst_t_vect;

    size_t len = AS_SEQ(seq_obj)->len;
    size_t str_len = 0;
    Nst_Obj *val = NULL;

    char *str = malloc(sizeof(char) * (is_vect ? 6 : 4));
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
        val = AS_SEQ(seq_obj)->objs[i];

        if ( IS_SEQ(val) )
            val = nst_obj_str_cast_seq(val, all_objs);
        else if ( val->type == nst_t_map )
            val = nst_obj_str_cast_map(val, all_objs);
        else if ( val->type == nst_t_str )
            val = nst_repr_string(val);
        else
            val = nst_obj_cast(val, nst_t_str, NULL);

        realloc_str = realloc(
            str,
            str_len
             + AS_STR(val)->len
             + (is_vect && i == len - 1 ? 4 : 3)
        );
        CHECK_BUFFER(realloc_str);
        str = realloc_str;
        memcpy(str + str_len, AS_STR(val)->value, AS_STR(val)->len);
        str_len += AS_STR(val)->len + (is_vect && i == len - 1 ? 3 : 2);
        dec_ref(val);

        if ( i == len - 1 )
            break;

        str[str_len - 2] = ',';
        str[str_len - 1] = ' ';
    }

    if ( is_vect )
    {
        if ( str_len == 3 )
            str_len = 5;
        str[str_len - 3] = ' ';
        str[str_len - 2] = '}';
        str[str_len - 1] = '>';
    }
    else
    {
        if ( str_len == 2 )
            str_len = 3;
        str[str_len - 2] = ' ';
        str[str_len - 1] = '}';
    }
    str[str_len] = 0;

    LList_pop(all_objs);

    return nst_new_string(str, str_len, true);
}

Nst_Obj *nst_obj_str_cast_map(Nst_Obj *map_obj, LList *all_objs)
{
    for ( LLNode *n = all_objs->head; n != NULL; n = n->next )
    {
        if ( map_obj == n->value )
            return nst_new_string("{.}", 3, false);
    }

    LList_push(all_objs, map_obj, false);

    size_t str_len = 2;
    Nst_Obj *key = NULL;
    Nst_Obj *val = NULL;

    char *str = malloc(sizeof(char) * 4);
    char *realloc_str = NULL;
    CHECK_BUFFER(str);

    str[0] = '{';
    str[1] = ' ';

    Nst_MapObj *map = AS_MAP(map_obj);
    Nst_Int idx = -1;
    size_t tot = map->item_count;
    size_t count = 0;

    while ( count++ < tot )
    {
        idx = nst_map_get_next_idx(idx, map);
        key = map->nodes[idx].key;
        val = map->nodes[idx].value;

        // Key cannot be a vector, an array or a map
        if ( key->type == nst_t_str )
            key = nst_repr_string(AS_STR(key));
        else
            key = nst_obj_cast(key, nst_t_str, NULL);

        if ( IS_SEQ(val) )
            val = nst_obj_str_cast_seq(val, all_objs);
        else if ( val->type == nst_t_map )
            val = nst_obj_str_cast_map(val, all_objs);
        else if ( val->type == nst_t_str )
            val = nst_repr_string(AS_STR(val));
        else
            val = nst_obj_cast(val, nst_t_str, NULL);

        realloc_str = realloc(str, str_len + AS_STR(key)->len + AS_STR(val)->len + 5);
        CHECK_BUFFER(realloc_str);
        str = realloc_str;
        memcpy(str + str_len, AS_STR(key)->value, AS_STR(key)->len);
        str_len += AS_STR(key)->len + 2;
        str[str_len - 2] = ':';
        str[str_len - 1] = ' ';
        memcpy(str + str_len, AS_STR(val)->value, AS_STR(val)->len);
        str_len += AS_STR(val)->len + 2;
        dec_ref(key);
        dec_ref(val);

        if ( count == tot )
            break;

        str[str_len - 2] = ',';
        str[str_len - 1] = ' ';
    }

    if ( str_len == 2 ) str_len = 3;
    str[str_len - 2] = ' ';
    str[str_len - 1] = '}';
    str[str_len] = 0;

    LList_pop(all_objs);

    return nst_new_string(str, str_len, true);
}

Nst_Obj *nst_obj_cast(Nst_Obj *ob, Nst_Obj *type, Nst_OpErr *err)
{
    register Nst_Obj *ob_t = ob->type;

    if ( ob_t == type )
        return inc_ref(ob);

    if ( type == nst_t_str )
    {
        if ( ob_t == nst_t_int )
        {
            char *buffer = malloc(MAX_INT_CHAR_COUNT * sizeof(char));
            CHECK_BUFFER(buffer);
            sprintf(buffer, "%lli", AS_INT(ob));
            return nst_new_string_raw(buffer, true);
        }
        else if ( ob_t == nst_t_real )
        {
            char *buffer = malloc(MAX_REAL_CHAR_COUNT * sizeof(char));
            CHECK_BUFFER(buffer);
            sprintf(buffer, "%g", AS_REAL(ob));
            return nst_new_string_raw(buffer, true);
        }
        else if ( ob_t == nst_t_bool )
        {
            if ( AS_BOOL(ob) == NST_TRUE )
                return nst_new_string("true", 4, false);
            else
                return nst_new_string("false", 5, false);
        }
        else if ( ob_t == nst_t_type )
            return nst_copy_string(ob);
        else if ( ob_t == nst_t_byte )
        {
            char *str = calloc(2, sizeof(char));
            CHECK_BUFFER(str);

            str[0] = AS_BYTE(ob);

            return nst_new_string(str, 1, true);

        }
        else if ( ob_t == nst_t_arr || ob_t == nst_t_vect )
        {
            LList *all_objs = LList_new();
            Nst_Obj *str = nst_obj_str_cast_seq(ob, all_objs);
            LList_destroy(all_objs, NULL);
            return str;
        }
        else if ( ob_t == nst_t_map )
        {
            LList *all_objs = LList_new();
            Nst_Obj *str = nst_obj_str_cast_map(ob, all_objs);
            LList_destroy(all_objs, NULL);
            return str;
        }
        else
        {
            char *buffer = malloc(sizeof(char) * (AS_STR(ob->type)->len + 10));
            CHECK_BUFFER(buffer);

            sprintf(buffer, "<%s object>", AS_STR(ob->type)->value);

            return nst_new_string_raw(buffer, true);
        }
    }
    else if ( type == nst_t_bool )
    {
        if ( ob_t == nst_t_int )
            NST_RETURN_COND(AS_INT(ob) != 0);
        else if ( ob_t == nst_t_real )
            NST_RETURN_COND(AS_REAL(ob) != 0.0);
        else if ( ob_t == nst_t_str )
            NST_RETURN_COND(AS_STR(ob)->len != 0);
        else if ( ob_t == nst_t_map )
            NST_RETURN_COND(AS_MAP(ob)->item_count != 0);
        else if ( ob_t == nst_t_arr || ob_t == nst_t_vect )
            NST_RETURN_COND(AS_SEQ(ob)->len != 0);
        else if ( ob_t == nst_t_null )
        {
            NST_RETURN_FALSE;
        }
        else if ( ob_t == nst_t_byte )
            NST_RETURN_COND(AS_BYTE(ob) != 0);
        else
        {
            NST_RETURN_TRUE;
        }
    }
    else if ( type == nst_t_int )
    {
        if ( ob_t == nst_t_real )
            return nst_new_int((Nst_Int)AS_REAL(ob));
        else if ( ob_t == nst_t_byte )
            return nst_new_int((Nst_Int)AS_BYTE(ob));
        else if ( ob_t == nst_t_str )
            return nst_parse_int(AS_STR(ob)->value, err);
        else
            RETURN_TYPE_ERROR("::");
    }
    else if ( type == nst_t_real )
    {
        if ( ob_t == nst_t_int )
            return nst_new_real((Nst_Real)AS_INT(ob));
        else if ( ob_t == nst_t_byte )
            return nst_new_real((Nst_Real)AS_BYTE(ob));
        else if ( ob_t == nst_t_str )
            return nst_parse_real(AS_STR(ob)->value, err);
        else
            RETURN_TYPE_ERROR("::");
    }
    else if ( type == nst_t_byte )
    {
        if ( ob_t == nst_t_int )
            return nst_new_byte(AS_INT(ob) & 0xff);
        else if ( ob_t == nst_t_str )
        {
            if ( AS_STR(ob)->len != 1 )
            {
                NST_SET_VALUE_ERROR("string must be exactly one character long");
                return NULL;
            }

            return nst_new_byte(*AS_STR(ob)->value);
        }
        else
            RETURN_TYPE_ERROR("::");
    }
    else if ( type == nst_t_iter )
    {
        if ( ob_t == nst_t_str )
        {
            Nst_Obj *idx = nst_new_int(0);
            Nst_SeqObj *data = AS_SEQ(nst_new_array(2));
            nst_set_value_seq(data, 0, idx);
            nst_set_value_seq(data, 1, ob);
            dec_ref(idx);

            return new_iter(
                AS_FUNC(new_cfunc(1, nst_str_iter_start)),
                AS_FUNC(new_cfunc(1, nst_str_iter_advance)),
                AS_FUNC(new_cfunc(1, nst_str_iter_is_done)),
                AS_FUNC(new_cfunc(1, nst_str_iter_get_val)),
                (Nst_Obj *)data
            );
        }
        else if ( ob_t == nst_t_arr || ob_t == nst_t_vect )
        {
            Nst_Obj *idx = nst_new_int(0);
            Nst_SeqObj *data = AS_SEQ(nst_new_array(2));
            nst_set_value_seq(data, 0, idx);
            nst_set_value_seq(data, 1, ob);
            dec_ref(idx);

            return new_iter(
                AS_FUNC(new_cfunc(1, nst_seq_iter_start)),
                AS_FUNC(new_cfunc(1, nst_seq_iter_advance)),
                AS_FUNC(new_cfunc(1, nst_seq_iter_is_done)),
                AS_FUNC(new_cfunc(1, nst_seq_iter_get_val)),
                (Nst_Obj *)data
            );
        }
        else if ( ob_t == nst_t_map )
        {
            Nst_Obj *start_obj   = nst_map_get_str(ob, "_start_");
            Nst_Obj *advance_obj = nst_map_get_str(ob, "_advance_");
            Nst_Obj *is_done_obj = nst_map_get_str(ob, "_is_done_");
            Nst_Obj *get_val_obj = nst_map_get_str(ob, "_get_val_");

            if ( start_obj == NULL )
                RETURN_MISSING_FUNC_ERROR("_start_");
            if ( advance_obj == NULL )
                RETURN_MISSING_FUNC_ERROR("_advance_");
            if ( is_done_obj == NULL )
                RETURN_MISSING_FUNC_ERROR("_is_done_");
            if ( get_val_obj == NULL )
                RETURN_MISSING_FUNC_ERROR("_get_val_");

            return new_iter(
                AS_FUNC(start_obj),
                AS_FUNC(advance_obj),
                AS_FUNC(is_done_obj),
                AS_FUNC(get_val_obj),
                inc_ref(ob)
            );
        }
        else
            RETURN_TYPE_ERROR("::");
    }
    else if ( type == nst_t_arr || type == nst_t_vect )
    {
        bool is_vect = type == nst_t_vect;
        if ( ob_t == nst_t_arr || ob_t == nst_t_vect )
        {
            size_t seq_len = AS_SEQ(ob)->len;
            Nst_SeqObj *seq = is_vect ? AS_SEQ(nst_new_vector(seq_len))
                                      : AS_SEQ(nst_new_array(seq_len));

            for ( size_t i = 0; i < seq_len; i++ )
                nst_set_value_seq(seq, i, AS_SEQ(ob)->objs[i]);

            return (Nst_Obj *)seq;
        }
        else if ( ob_t == nst_t_str )
        {
            size_t str_len = AS_STR(ob)->len;
            Nst_SeqObj *seq = is_vect ? AS_SEQ(nst_new_vector(str_len))
                                      : AS_SEQ(nst_new_array(str_len));

            for ( size_t i = 0; i < str_len; i++ )
                seq->objs[i] = nst_string_get_idx(ob, i);
            return (Nst_Obj *)seq;
        }
        else
            RETURN_TYPE_ERROR("::");
    }
    else
        RETURN_TYPE_ERROR("::");
}

Nst_Obj *nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
{
    ob1 = nst_obj_cast(ob1, nst_t_str, err);
    ob2 = nst_obj_cast(ob2, nst_t_str, err);

    Nst_StrObj *nst_s1 = AS_STR(ob1);
    Nst_StrObj *nst_s2 = AS_STR(ob2);

    register char *s1 = nst_s1->value;
    register char *s2 = nst_s2->value;
    register size_t len1 = nst_s1->len;
    register size_t len2 = nst_s2->len;
    register size_t tot_len = len1 + len2;

    char *buffer = malloc(sizeof(char) * (tot_len + 1));

    CHECK_BUFFER(buffer);

    memcpy(buffer, s1, len1);
    memcpy(buffer + len1, s2, len2);
    buffer[tot_len] = '\0';

    Nst_Obj *new_obj = nst_new_string(buffer, tot_len, true);

    dec_ref(ob1);
    dec_ref(ob2);

    return new_obj;
}

// Local operations
Nst_Obj *nst_obj_neg(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type == nst_t_int )
        return nst_new_int(-AS_INT(ob));
    else if ( ob->type == nst_t_real )
        return nst_new_real(-AS_REAL(ob));
    else
        RETURN_TYPE_ERROR("-");
}

Nst_Obj *nst_obj_len(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type == nst_t_str )
        return nst_new_int(AS_STR(ob)->len);
    else if ( ob->type == nst_t_map )
        return nst_new_int(AS_MAP(ob)->item_count);
    else if ( IS_SEQ(ob) )
        return nst_new_int(AS_SEQ(ob)->len);
    else
        RETURN_TYPE_ERROR("$");
}

Nst_Obj *nst_obj_bwnot(Nst_Obj *ob, Nst_OpErr *err)
{
    if ( ob->type == nst_t_int )
        return nst_new_int(~AS_INT(ob));
    else
        RETURN_TYPE_ERROR("~");
}

Nst_Obj *nst_obj_lgnot(Nst_Obj *ob, Nst_OpErr *err)
{
    ob = nst_obj_cast(ob, nst_t_bool, err);
    
    if ( ob == nst_true )
    {
        dec_ref(nst_true);
        NST_RETURN_FALSE;
    }
    else
    {
        dec_ref(nst_false);
        NST_RETURN_TRUE;
    }
}

Nst_Obj *nst_obj_stdout(Nst_Obj *ob, Nst_OpErr *err)
{
    Nst_Obj *str = nst_obj_cast(ob, nst_t_str, err);
    fwrite(AS_STR(str)->value, sizeof(char), AS_STR(str)->len, stdout);
    fflush(stdout);
    dec_ref(str);
    return inc_ref(ob);
}

Nst_Obj *nst_obj_stdin(Nst_Obj *ob, Nst_OpErr *err)
{
    ob = nst_obj_cast(ob, nst_t_str, err);
    printf("%s", AS_STR(ob)->value);
    fflush(stdout);
    dec_ref(ob);

    char *buffer = malloc(4);
    CHECK_BUFFER(buffer);

    size_t buffer_size = 4;
    size_t i = 0;
    char ch = getchar();

    while ( ch != '\n' )
    {
        if ( ch == '\r' )
            continue;
        else if ( ch == '\0' )
            break;

        if ( buffer_size == i + 2 )
        {
            char *new_buffer = realloc(buffer, buffer_size *= 2);
            if ( new_buffer == NULL )
            {
                free(buffer);
                errno = ENOMEM;
                return NULL;
            }
            buffer = new_buffer;
        }

        buffer[i++] = ch;
        ch = getchar();
    }
    buffer[i] = '\0';
    char *new_buffer = realloc(buffer, i + 1);
    if ( new_buffer == NULL )
    {
        free(buffer);
        errno = ENOMEM;
        return NULL;
    }

    return nst_new_string(new_buffer, i, true);
}

Nst_Obj *nst_obj_typeof(Nst_Obj *ob, Nst_OpErr *err)
{
    return inc_ref(ob->type);
}

Nst_Obj *nst_obj_import(Nst_Obj *ob, Nst_OpErr *err)
{

    if ( ob->type != nst_t_str )
    {
        err->name = "Type Error";
        err->message = _nst_format_type_error(EXPECTED_TYPE("Str"), ob->type_name);
        return NULL;
    }

    char *file_name = AS_STR(ob)->value;
    bool c_import = false;

    if ( AS_STR(ob)->len > 6 &&
        file_name[0] == '_' && file_name[1] == '_' &&
        file_name[2] == 'C' && file_name[3] == '_' &&
        file_name[4] == '_' && file_name[5] == ':' )
    {
        c_import = true;
        file_name += 6; // skip __C__:
    }

    size_t file_name_len = strlen(file_name);
    char *file_path = file_name;
    bool file_path_allocated = false;

    // Checks if the file exists with the given path
    Nst_IOfile file;
    if ( (file = fopen(file_path, "r")) == NULL )
    {
        // Tries to open it as a file of the standard library
        char *appdata = getenv("LOCALAPPDATA");
        char *original_path = _nst_format_fnf_error(FILE_NOT_FOUND, file_name);
        if ( appdata == NULL )
        {
            err->name = "Value Error";
            err->message = original_path;
            return NULL;
        }

        size_t appdata_len = strlen(appdata);
        file_path = malloc(appdata_len + file_name_len + 26);
        file_path_allocated = true;
        if ( !file_path ) return NULL;

        char *lib_dir = "\\Programs\\nest\\nest_libs\\";
        memcpy(file_path, appdata, appdata_len);
        memcpy(file_path + appdata_len, lib_dir, 25);
        memcpy(file_path + appdata_len + 25, file_name, file_name_len + 1); // copies also \0

        if ( (file = fopen(file_path, "r")) == NULL )
        {
            err->name = "Value Error";
            err->message = original_path;
            free(file_path);
            return NULL;
        }

        free(original_path);
    }
    fclose(file);

    // Gets the full path to allow importing with different relative paths
    char *full_path = NULL;
    nst_get_full_path(file_path, &full_path, NULL);
    if ( file_path_allocated ) free(file_path);
    file_path = full_path;

    // Check if the module is in the import stack
    for ( LLNode *n = nst_state->lib_paths->head; n != NULL; n = n->next )
    {
        if ( strcmp(file_path, (const char *)(n->value)) == 0 )
        {
            err->name = "Import Error";
            err->message = "circular import";
            return NULL;
        }
    }

    // Check if the module was loaded previously
    for ( LLNode *n = nst_state->lib_handles->head; n != NULL; n = n->next )
    {
        Nst_LibHandle *handle = (Nst_LibHandle *)(n->value);
        if ( strcmp(handle->path, file_path) == 0 )
        {
            return inc_ref(handle->val);
        }
    }

    if ( !c_import )
    {
        Nst_MapObj *map = nst_run_module(file_path);

        if ( map == NULL )
        {
            nst_state->error_occurred = true;
            return NULL;
        }

        // Adds the generated map to the modules previously loaded
        Nst_LibHandle *handle = malloc(sizeof(Nst_LibHandle));
        if ( handle == NULL )
        {
            errno = ENOMEM;
            return NULL;
        }

        handle->val = map;
        handle->path = file_path;

        LList_append(nst_state->lib_handles, handle, true);
        return inc_ref(map);
    }

    HMODULE lib = LoadLibraryA(file_path);

    if ( !lib )
    {
        err->name = "Import Error";
        err->message = FILE_NOT_DLL;
        return NULL;
    }

    void (*init_lib_obj)(OBJ_INIT_FARGS)
        = (void (*)(OBJ_INIT_FARGS))GetProcAddress(lib, "init_lib_obj");

    if ( init_lib_obj == NULL )
    {
        err->name = "Import Error";
        err->message = "module does not import \"obj.h\"";
        return NULL;
    }

    // Link the global variables
    init_lib_obj(nst_t_type, nst_t_int, nst_t_real, nst_t_bool,
        nst_t_null, nst_t_str, nst_t_arr, nst_t_vect,
        nst_t_map, nst_t_func, nst_t_iter, nst_t_byte,
        nst_t_file, nst_true, nst_false, nst_null, nst_state);

    // Initialize library
    bool (*lib_init)() = (bool (*)())GetProcAddress(lib, "lib_init");
    if ( lib_init == NULL )
    {
        err->name = "Import Error";
        err->message = NO_LIB_INIT;
        return NULL;
    }

    if ( !lib_init() )
    {
        errno = ENOMEM;
        return NULL;
    }

    // Get function pointers
    FuncDeclr *(*get_func_ptrs)() = (FuncDeclr * (*)())GetProcAddress(lib, "get_func_ptrs");
    if ( get_func_ptrs == NULL )
    {
        err->name = "Import Error";
        err->message = NO_GET_FUNC_PTRS;
        return NULL;
    }

    FuncDeclr *func_ptrs = get_func_ptrs();

    if ( func_ptrs == NULL )
    {
        err->name = "Import Error";
        err->message = "module was not initialized correctly";
        return NULL;
    }

    // Populate the function map
    Nst_MapObj *func_map = AS_MAP(nst_new_map());

    for ( size_t i = 0;; i++ )
    {
        FuncDeclr func = func_ptrs[i];
        if ( func.func_ptr == NULL )
            break;

        Nst_Obj *func_obj = new_cfunc(func.arg_num, func.func_ptr);

        nst_map_set(func_map, (Nst_Obj *)func.name, func_obj);
    }

    LList_append(nst_state->loaded_libs, lib, false);

    // Add map to the loaded libaries
    Nst_LibHandle *handle = malloc(sizeof(Nst_LibHandle));
    if ( handle == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    handle->val = func_map;
    handle->path = file_path;

    LList_append(nst_state->lib_handles, handle, true);

    return inc_ref(func_map);
}
