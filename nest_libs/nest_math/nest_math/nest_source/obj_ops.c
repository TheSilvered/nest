#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include "obj_ops.h"
#include "nst_types.h"
#include "map.h"
#include "error.h"

#define MAX_INT_CHAR_COUNT 21
#define MAX_REAL_CHAR_COUNT 25
#define MAX_BYTE_CHAR_COUNT 5

#define IS_NUMBER(obj) ( obj->type == nst_t_int || obj->type == nst_t_real )
#define IS_SEQ(obj) ( obj->type == nst_t_arr || obj->type == nst_t_vect )
#define ARE_TYPE(nst_type) ( ob1->type == nst_type && ob2->type == nst_type )

#define RETURN_TYPE_ERROR(operand) do { \
    err->name = TYPE_ERROR; \
    err->message = "invalid type for '" operand "'"; \
    return NULL; } while (0)

#define CHECK_BUFFER(buf) do { \
        if ( buf == NULL ) \
        { \
            errno = ENOMEM; \
            return NULL; \
        } \
    } while (0)

#define RETURN_TRUE return inc_ref(nst_true)
#define RETURN_FALSE return inc_ref(nst_false)
#define RETURN_COND(cond) do { \
    if ( cond ) \
    { \
        RETURN_TRUE; \
    } \
    else \
    { \
        RETURN_FALSE; \
    } } while (0)

// Comparisons
Nst_Obj *obj_eq(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    register void *v1 = ob1->value;
    register void *v2 = ob2->value;

    if ( ob1 == ob2 )
        RETURN_TRUE;
    else if ( ob1->type == nst_t_map || ob2->type == nst_t_map )
        RETURN_FALSE;
    else if ( ARE_TYPE(nst_t_int) )
        RETURN_COND(AS_INT_V(v1) == AS_INT_V(v2));
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);
        bool check = AS_BOOL_V(v1) == AS_BOOL_V(v2);
        dec_ref(ob1);
        dec_ref(ob2);

        if ( check )
            RETURN_TRUE;
        else
            RETURN_FALSE;
    }
    else if ( ob1->type == nst_t_str && ob2->type == nst_t_str )
    {
        Nst_string s1 = *AS_STR_V(v1);
        Nst_string s2 = *AS_STR_V(v2);

        RETURN_COND(strcmp(s1.value, s2.value) == 0);
    }
    else if ( ob1->type == nst_t_bool && ob2->type == nst_t_bool )
        RETURN_COND(v1 == v2);
    else if ( IS_SEQ(ob1) && IS_SEQ(ob2) )
    {
        if ( AS_SEQ_V(v1)->len != AS_SEQ_V(v2)->len )
            RETURN_FALSE;

        for ( size_t i = 0, n = AS_SEQ_V(v1)->len; i < n; i++ )
        {
            if ( obj_eq(AS_SEQ_V(v1)->objs[i], AS_SEQ_V(v2)->objs[i], err) == nst_false )
                RETURN_FALSE;
        }
        RETURN_TRUE;
    }
    else
        RETURN_FALSE;
}

Nst_Obj *obj_ne(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    return obj_lgnot(obj_eq(ob1, ob2, err), err);
}

Nst_Obj *obj_gt(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    register void *v1 = ob1->value;
    register void *v2 = ob2->value;

    if ( ARE_TYPE(nst_t_str) )
    {
        Nst_string s1 = *AS_STR_V(v1);
        Nst_string s2 = *AS_STR_V(v2);

        RETURN_COND(strcmp(s1.value, s2.value) > 0);
    }
    else if ( ARE_TYPE(nst_t_int) )
        RETURN_COND(AS_INT_V(v1) > AS_INT_V(v2));
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);
        bool check = AS_REAL_V(v1) > AS_REAL_V(v2);
        dec_ref(ob1);
        dec_ref(ob2);

        RETURN_COND(check);
    }
    else
        RETURN_TYPE_ERROR(">");
}

Nst_Obj *obj_lt(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    register void *v1 = ob1->value;
    register void *v2 = ob2->value;

    if ( ARE_TYPE(nst_t_str) )
    {
        Nst_string s1 = *AS_STR_V(v1);
        Nst_string s2 = *AS_STR_V(v2);

        RETURN_COND(strcmp(s1.value, s2.value) < 0);
    }
    else if ( ARE_TYPE(nst_t_int) )
        RETURN_COND(AS_INT_V(v1) < AS_INT_V(v2));
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);
        bool check = AS_REAL_V(v1) < AS_REAL_V(v2);
        dec_ref(ob1);
        dec_ref(ob2);

        RETURN_COND(check);
    }
    else
        RETURN_TYPE_ERROR("<");
}

Nst_Obj *obj_ge(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( obj_eq(ob1, ob2, err) == nst_true )
        RETURN_TRUE;

    Nst_Obj *res = obj_gt(ob1, ob2, err);

    if ( strcmp(err->name, TYPE_ERROR) == 0 )
        err->message = "invalid type for '>='";
    return res;
}

Nst_Obj *obj_le(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( obj_eq(ob1, ob2, err) == nst_true )
        RETURN_TRUE;

    Nst_Obj *res = obj_lt(ob1, ob2, err);

    if ( strcmp(err->name, TYPE_ERROR) == 0 )
    {
        err->message = "invalid type for '<='";
    }
    return res;
}

// Arithmetic operations
Nst_Obj *obj_add(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ob1->type == nst_t_vect )
    {
        append_value_vector(ob1->value, ob2);
        return inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(AS_INT(ob1) + AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        Nst_Obj *new_obj = make_obj(
            new_real(AS_REAL(ob1) + AS_REAL(ob2)),
            nst_t_real, free
        );
        
        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
    {
        printf("%s\n", ob1->type_name);
        RETURN_TYPE_ERROR("+");
    }
}

Nst_Obj *obj_sub(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ob1->type == nst_t_vect )
    {
        return rem_value_vector(ob1->value, ob2);
    }
    else if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(AS_INT(ob1) - AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        Nst_Obj *new_obj = make_obj(
            new_real(AS_REAL(ob1) - AS_REAL(ob2)),
            nst_t_real, free
        );

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
        RETURN_TYPE_ERROR("-");
}

Nst_Obj *obj_mul(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ob1->type == nst_t_vect && ob2->type == nst_t_int )
    {
        register Nst_sequence *vect = ob1->value;
        size_t max_ob = vect->len;

        for ( Nst_int i = 0, n = AS_INT(ob2) - 1; i < n; i++ )
        {
            for ( size_t j = 0; j < max_ob; j++ )
            {
                append_value_vector(vect, vect->objs[j]);
            }
        }

        if ( AS_INT(ob2) == 0 )
        {
            return make_obj(new_vector_empty(0), nst_t_vect, destroy_seq);
        }

        return inc_ref(ob1);
    }
    else if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(AS_INT(ob1) * AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        Nst_Obj *new_obj = make_obj(
            new_real(AS_REAL(ob1) * AS_REAL(ob2)),
            nst_t_real, free
        );

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
        RETURN_TYPE_ERROR("*");
}

Nst_Obj *obj_div(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ob1->type == nst_t_vect && ob2->type == nst_t_int )
    {
        return pop_value_vector(ob1->value, (size_t)AS_INT(ob2));
    }
    else if ( ARE_TYPE(nst_t_int) )
    {
        if ( AS_INT(ob2) == 0 )
        {
            err->name = MATH_ERROR;
            err->message = "division by zero";
            return NULL;
        }

        return make_obj(
            new_int(AS_INT(ob1) / AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        if ( AS_REAL(ob2) == 0.0 )
        {
            err->name = MATH_ERROR;
            err->message = "division by zero";
            return NULL;
        }

        Nst_Obj *new_obj = make_obj(
            new_real(AS_REAL(ob1) / AS_REAL(ob2)),
            nst_t_real, free
        );

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
        RETURN_TYPE_ERROR("/");
}

Nst_Obj *obj_pow(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int((Nst_int)powl(
                (Nst_real)AS_INT(ob1),
                (Nst_real)AS_INT(ob2)
            )),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        Nst_real v1 = AS_REAL(ob1);
        Nst_real v2 = AS_REAL(ob2);

        dec_ref(ob1);
        dec_ref(ob2);

        // any root of a negative number gives -nan as a result
        if ( v1 < 0 && floor(v2) != v2 )
        {
            err->name = MATH_ERROR;
            err->message = "fractional power of a negative number";
            return NULL;
        }

        return make_obj(
            new_real((Nst_real)powl(v1, v2)),
            nst_t_real, free
        );
    }
    else
        RETURN_TYPE_ERROR("^");
}

Nst_Obj *obj_mod(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        if ( AS_INT(ob2) == 0 )
        {
            err->name = MATH_ERROR;
            err->message = "modulo by zero";
            return NULL;
        }

        return make_obj(
            new_int(AS_INT(ob1) % AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        if ( AS_REAL(ob2) == 0.0 )
        {
            err->name = MATH_ERROR;
            err->message = "modulo by zero";
            return NULL;
        }

        Nst_Obj *new_obj = make_obj(
            new_real(fmodl(AS_REAL(ob1), AS_REAL(ob2))),
            nst_t_real, free
        );

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
        RETURN_TYPE_ERROR("%");
}

// Bitwise operations
Nst_Obj *obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(AS_INT(ob1) | AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else
        RETURN_TYPE_ERROR("|");
}

Nst_Obj *obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(AS_INT(ob1) & AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else
        RETURN_TYPE_ERROR("&");
}

Nst_Obj *obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(AS_INT(ob1) ^ AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else
        RETURN_TYPE_ERROR("^^");
}

Nst_Obj *obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(AS_INT(ob1) << AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else
        RETURN_TYPE_ERROR("<<");
}

Nst_Obj *obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(AS_INT(ob1) >> AS_INT(ob2)),
            nst_t_int, free
        );
    }
    else
        RETURN_TYPE_ERROR(">>");
}

// Logical operations
Nst_Obj *obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    ob1 = obj_cast(ob1, nst_t_bool, err);
    ob2 = obj_cast(ob2, nst_t_bool, err);

    Nst_bool v1 = AS_BOOL(ob1);
    Nst_bool v2 = AS_BOOL(ob2);

    dec_ref(ob1);
    dec_ref(ob2);

    RETURN_COND( v1 == NST_TRUE || v2 == NST_TRUE );
}

Nst_Obj *obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    ob1 = obj_cast(ob1, nst_t_bool, err);
    ob2 = obj_cast(ob2, nst_t_bool, err);

    Nst_bool v1 = AS_BOOL(ob1);
    Nst_bool v2 = AS_BOOL(ob2);

    dec_ref(ob1);
    dec_ref(ob2);

    RETURN_COND(v1 == NST_TRUE && v2 == NST_TRUE);
}

Nst_Obj *obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    ob1 = obj_cast(ob1, nst_t_bool, err);
    ob2 = obj_cast(ob2, nst_t_bool, err);

    Nst_bool v1 = AS_BOOL(ob1);
    Nst_bool v2 = AS_BOOL(ob2);

    dec_ref(ob1);
    dec_ref(ob2);

    RETURN_COND((v1 == NST_TRUE && v2 == NST_FALSE) ||
                (v1 == NST_FALSE && v2 == NST_TRUE));
}

// Other
Nst_Obj *obj_str_cast_seq(Nst_Obj *seq_obj, LList *all_objs)
{
    for ( LLNode *n = all_objs->head; n != NULL; n = n->next )
    {
        if ( seq_obj == n->value )
            return new_str_obj(new_string("{.}", 3, false));
    }

    LList_push(all_objs, seq_obj, false);
    bool is_vect = seq_obj->type == nst_t_vect;

    size_t len = AS_SEQ(seq_obj)->len;
    size_t str_len = 0;
    Nst_Obj *val_obj = NULL;
    Nst_string *val = NULL;

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
        val_obj = AS_SEQ(seq_obj)->objs[i];

        if ( IS_SEQ(val_obj) )
        {
            val_obj = obj_str_cast_seq(val_obj, all_objs);
            val = AS_STR(val_obj);
        }
        else if ( val_obj->type == nst_t_map )
        {
            val_obj = obj_str_cast_map(val_obj, all_objs);
            val = AS_STR(val_obj);
        }
        else if ( val_obj->type == nst_t_str )
        {
            val = repr_string(AS_STR(val_obj));
            val_obj = NULL;
        }
        else
        {
            val_obj = obj_cast(val_obj, nst_t_str, NULL);
            val = AS_STR(val_obj);
        }

        realloc_str = realloc(
            str,
            str_len
             + val->len
             + (is_vect && i == len - 1 ? 4 : 3)
        );
        CHECK_BUFFER(realloc_str);
        str = realloc_str;
        memcpy(str + str_len, val->value, val->len);
        str_len += val->len + (is_vect && i == len - 1 ? 3 : 2);
        if ( val_obj != NULL )
        {
            dec_ref(val_obj);
            val_obj = NULL;
        }

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

    return new_str_obj(new_string(str, str_len, true));
}

Nst_Obj *obj_str_cast_map(Nst_Obj *map_obj, LList *all_objs)
{
    for ( LLNode *n = all_objs->head; n != NULL; n = n->next )
    {
        if ( map_obj == n->value )
            return new_str_obj(new_string("{.}", 3, false));
    }

    LList_push(all_objs, map_obj, false);

    size_t str_len = 2;
    Nst_Obj *key_obj = NULL;
    Nst_string *key = NULL;
    Nst_Obj *val_obj = NULL;
    Nst_string *val = NULL;

    char *str = malloc(sizeof(char) * 4);
    char *realloc_str = NULL;
    CHECK_BUFFER(str);

    str[0] = '{';
    str[1] = ' ';

    Nst_map *map = AS_MAP(map_obj);
    size_t idx = -1;
    size_t tot = map->item_count;
    size_t count = 0;

    while ( count++ < tot )
    {
        idx = get_next_idx(idx, map);
        Nst_Obj *key_obj = map->nodes[idx].key;
        Nst_Obj *val_obj = map->nodes[idx].value;

        // Key cannot be a vector, an array or a map
        if ( key_obj->type == nst_t_str )
        {
            key = repr_string(AS_STR(key_obj));
            key_obj = NULL;
        }
        else
        {
            key_obj = obj_cast(key_obj, nst_t_str, NULL);
            key = AS_STR(key_obj);
        }

        if ( IS_SEQ(val_obj) )
        {
            val_obj = obj_str_cast_seq(val_obj, all_objs);
            val = AS_STR(val_obj);
        }
        else if ( val_obj->type == nst_t_map )
        {
            val_obj = obj_str_cast_map(val_obj, all_objs);
            val = AS_STR(val_obj);
        }
        else if ( val_obj->type == nst_t_str )
        {
            val = repr_string(AS_STR(val_obj));
            val_obj = NULL;
        }
        else
        {
            val_obj = obj_cast(val_obj, nst_t_str, NULL);
            val = AS_STR(val_obj);
        }

        realloc_str = realloc(str, str_len + key->len + val->len + 5);
        CHECK_BUFFER(realloc_str);
        str = realloc_str;
        memcpy(str + str_len, key->value, key->len);
        str_len += key->len + 2;
        str[str_len - 2] = ':';
        str[str_len - 1] = ' ';
        memcpy(str + str_len, val->value, val->len);
        str_len += val->len + 2;

        if ( key_obj != NULL )
        {
            dec_ref(key_obj);
            key_obj = NULL;
        }
        if ( val_obj != NULL )
        {
            dec_ref(val_obj);
            val_obj = NULL;
        }

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

    return new_str_obj(new_string(str, str_len, true));
}

Nst_Obj *obj_cast(Nst_Obj *ob, Nst_Obj *type, OpErr *err)
{
    register Nst_Obj *ob_t = ob->type;
    register void *ob_val = ob->value;

    if ( ob_t == type )
        return inc_ref(ob);

    if ( type == nst_t_str )
    {
        if ( ob_t == nst_t_int )
        {
            char *buffer = malloc(MAX_INT_CHAR_COUNT * sizeof(char));
            CHECK_BUFFER(buffer);
            sprintf(buffer, "%lli", AS_INT_V(ob_val));
            return make_obj(
                new_string_raw(buffer, true),
                nst_t_str, destroy_string
            );
        }
        else if ( ob_t == nst_t_real )
        {
            char *buffer = malloc(MAX_REAL_CHAR_COUNT * sizeof(char));
            CHECK_BUFFER(buffer);
            sprintf(buffer, "%g", AS_REAL_V(ob_val));
            return make_obj(
                new_string_raw(buffer, true),
                nst_t_str, destroy_string
            );
        }
        else if ( ob_t == nst_t_bool )
        {
            if ( AS_BOOL_V(ob_val) == NST_TRUE )
                return make_obj(
                    new_string("true", 4, false),
                    nst_t_str, destroy_string
                );
            else
                return make_obj(
                    new_string("false", 5, false),
                    nst_t_str, destroy_string
                );
        }
        else if ( ob_t == nst_t_type )
        {
            return make_obj(
                copy_string(ob_val),
                nst_t_str, destroy_string
            );
        }
        else if ( ob_t == nst_t_byte )
        {
            char *str = calloc(2, sizeof(char));
            CHECK_BUFFER(str);

            str[0] = AS_BYTE_V(ob_val);

            return make_obj(
                new_string(str, 1, true),
                nst_t_str, destroy_string
            );

        }
        else if ( ob_t == nst_t_arr || ob_t == nst_t_vect )
        {
            LList *all_objs = LList_new();
            Nst_Obj *str = obj_str_cast_seq(ob, all_objs);
            LList_destroy(all_objs, NULL);
            return str;
        }
        else if ( ob_t == nst_t_map )
        {
            LList *all_objs = LList_new();
            Nst_Obj *str = obj_str_cast_map(ob, all_objs);
            LList_destroy(all_objs, NULL);
            return str;
        }
        else
        {
            char *buffer = malloc(sizeof(char) * (AS_STR(ob->type)->len + 10));
            CHECK_BUFFER(buffer);

            sprintf(buffer, "<%s object>", AS_STR(ob->type)->value);

            return make_obj(
                new_string_raw(buffer, true),
                nst_t_str, destroy_string
            );
        }
    }
    else if ( type == nst_t_bool )
    {
        if ( ob_t == nst_t_int )
            RETURN_COND(AS_INT_V(ob_val) != 0);
        else if ( ob_t == nst_t_real )
            RETURN_COND(AS_REAL_V(ob_val) != 0.0);
        else if ( ob_t == nst_t_str )
            RETURN_COND(AS_STR_V(ob_val)->len != 0);
        else if ( ob_t == nst_t_map )
            RETURN_COND(AS_MAP_V(ob_val)->item_count != 0);
        else if ( ob_t == nst_t_arr || ob_t == nst_t_vect )
            RETURN_COND(AS_SEQ_V(ob_val)->len != 0);
        else if ( ob_t == nst_t_null )
        {
            RETURN_FALSE;
        }
        else if ( ob_t == nst_t_byte )
            RETURN_COND(AS_BYTE_V(ob_val) != 0);
        else
        {
            RETURN_TRUE;
        }
    }
    else if ( type == nst_t_int )
    {
        if ( ob_t == nst_t_real )
            return make_obj(
                new_int((Nst_int)AS_REAL_V(ob_val)),
                nst_t_int, free
            );
        else if ( ob_t == nst_t_byte )
            return make_obj(
                new_int((Nst_int)AS_INT_V(ob_val)),
                nst_t_int, free
            );
        else if ( ob_t == nst_t_str )
        {
            Nst_int *val = parse_int(AS_STR_V(ob_val)->value, err);
            if ( val == NULL )
                return NULL;
            return make_obj_free(val, nst_t_int);
        }
        else
            RETURN_TYPE_ERROR("::");
    }
    else if ( type == nst_t_real )
    {
        if ( ob_t == nst_t_int )
            return make_obj(
                new_real((Nst_real)AS_INT_V(ob_val)),
                nst_t_real, free
            );
        else if ( ob_t == nst_t_byte )
            return make_obj(
                new_real((Nst_real)AS_BYTE_V(ob_val)),
                nst_t_real, free
            );
        else if ( ob_t == nst_t_str )
        {
            Nst_real *val = parse_real(AS_STR_V(ob_val)->value, err);
            if ( val == NULL )
                return NULL;
            return make_obj_free(val, nst_t_real);
        }
        else
            RETURN_TYPE_ERROR("::");
    }
    else if ( type == nst_t_byte )
    {
        if ( ob_t == nst_t_int )
        {
            return new_byte_obj(AS_INT_V(ob_val) & 0xff);
        }
        else
            RETURN_TYPE_ERROR("::");
    }
    else if ( type == nst_t_iter )
    {
        if ( ob_t == nst_t_str )
        {
            Nst_Obj *idx = make_obj(new_int(0), nst_t_int, free);
            Nst_sequence *data_seq = new_array_empty(2);
            set_value_seq(data_seq, 0, idx);
            set_value_seq(data_seq, 1, ob);
            Nst_Obj *data_obj = make_obj(data_seq, nst_t_arr, destroy_seq);
            dec_ref(idx);

            Nst_func *start_func   = new_cfunc(1, str_iter_start);
            Nst_func *advance_func = new_cfunc(1, str_iter_advance);
            Nst_func *is_done_func = new_cfunc(1, str_iter_is_done);
            Nst_func *get_val_func = new_cfunc(1, str_iter_get_val);

            Nst_Obj *start_obj   = make_obj(start_func,   nst_t_func, destroy_func);
            Nst_Obj *advance_obj = make_obj(advance_func, nst_t_func, destroy_func);
            Nst_Obj *is_done_obj = make_obj(is_done_func, nst_t_func, destroy_func);
            Nst_Obj *get_val_obj = make_obj(get_val_func, nst_t_func, destroy_func);

            Nst_iter *iter = new_iter(
                start_obj,
                advance_obj,
                is_done_obj,
                get_val_obj,
                data_obj
            );

            return make_obj(iter, nst_t_iter, destroy_iter);
        }
        else if ( ob_t == nst_t_arr || ob_t == nst_t_vect )
        {
            Nst_Obj *idx = make_obj(new_int(0), nst_t_int, free);
            Nst_sequence *data_seq = new_array_empty(2);
            set_value_seq(data_seq, 0, idx);
            set_value_seq(data_seq, 1, ob);
            Nst_Obj *data_obj = make_obj(data_seq, nst_t_arr, destroy_seq);
            dec_ref(idx);

            Nst_func *start_func   = new_cfunc(1, seq_iter_start);
            Nst_func *advance_func = new_cfunc(1, seq_iter_advance);
            Nst_func *is_done_func = new_cfunc(1, seq_iter_is_done);
            Nst_func *get_val_func = new_cfunc(1, seq_iter_get_val);

            Nst_Obj *start_obj   = make_obj(start_func,   nst_t_func, destroy_func);
            Nst_Obj *advance_obj = make_obj(advance_func, nst_t_func, destroy_func);
            Nst_Obj *is_done_obj = make_obj(is_done_func, nst_t_func, destroy_func);
            Nst_Obj *get_val_obj = make_obj(get_val_func, nst_t_func, destroy_func);

            Nst_iter *iter = new_iter(
                start_obj,
                advance_obj,
                is_done_obj,
                get_val_obj,
                data_obj
            );

            return make_obj(iter, nst_t_iter, destroy_iter);
        }
        else if ( ob_t == nst_t_map )
        {
            Nst_Obj *start_key   = make_obj(new_string("_start_",   7, false), nst_t_str, destroy_string);
            Nst_Obj *advance_key = make_obj(new_string("_advance_", 9, false), nst_t_str, destroy_string);
            Nst_Obj *is_done_key = make_obj(new_string("_is_done_", 9, false), nst_t_str, destroy_string);
            Nst_Obj *get_val_key = make_obj(new_string("_get_val_", 9, false), nst_t_str, destroy_string);

            Nst_Obj *start_obj   = map_get(ob_val, start_key);
            Nst_Obj *advance_obj = map_get(ob_val, advance_key);
            Nst_Obj *is_done_obj = map_get(ob_val, is_done_key);
            Nst_Obj *get_val_obj = map_get(ob_val, get_val_key);

            dec_ref(start_key);
            dec_ref(advance_key);
            dec_ref(is_done_key);
            dec_ref(get_val_key);

            Nst_iter *iter = new_iter(
                start_obj,
                advance_obj,
                is_done_obj,
                get_val_obj,
                ob
            );

            inc_ref(ob);

            return make_obj(iter, nst_t_iter, destroy_iter);
        }
        else
            RETURN_TYPE_ERROR("::");
    }
    else if ( type == nst_t_arr || type == nst_t_vect )
    {
        bool is_vect = type == nst_t_vect;
        if ( ob_t == nst_t_arr || ob_t == nst_t_vect )
        {
            size_t seq_len = AS_SEQ_V(ob_val)->len;
            Nst_sequence *seq = is_vect ? new_vector_empty(seq_len)
                                        : new_array_empty(seq_len);

            for ( size_t i = 0; i < seq_len; i++ )
                set_value_seq(seq, i, AS_SEQ_V(ob_val)->objs[i]);

            return is_vect ? new_vect_obj(seq) : new_arr_obj(seq);
        }
        else if ( ob_t == nst_t_str )
        {
            size_t str_len = AS_STR_V(ob_val)->len;
            Nst_sequence *seq = is_vect ? new_vector_empty(str_len)
                                        : new_array_empty(str_len);

            for ( size_t i = 0; i < str_len; i++ )
            {
                char *ch = calloc(2, sizeof(char));
                CHECK_BUFFER(ch);

                ch[0] = AS_STR_V(ob_val)->value[i];
                seq->objs[i] = new_str_obj(new_string(ch, 1, true));
            }
            return is_vect ? new_vect_obj(seq) : new_arr_obj(seq);
        }
        else
            RETURN_TYPE_ERROR("::");
    }
    else
        RETURN_TYPE_ERROR("::");
}

Nst_Obj *obj_concat(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    ob1 = obj_cast(ob1, nst_t_str, err);
    ob2 = obj_cast(ob2, nst_t_str, err);

    Nst_string *nst_s1 = ob1->value;
    Nst_string *nst_s2 = ob2->value;

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

    Nst_Obj *new_obj = make_obj(
        new_string(buffer, tot_len, true),
        nst_t_str, destroy_string
    );

    dec_ref(ob1);
    dec_ref(ob2);

    return new_obj;
}

// Local operations
Nst_Obj *obj_neg(Nst_Obj *ob, OpErr *err)
{
    if ( ob->type == nst_t_int )
        return make_obj_free(new_int(-AS_INT(ob)), nst_t_int);
    else if ( ob->type == nst_t_real )
        return make_obj_free(new_real(-AS_REAL(ob)), nst_t_real);
    else
        RETURN_TYPE_ERROR("-");
}

Nst_Obj *obj_len(Nst_Obj *ob, OpErr *err)
{
    if ( ob->type == nst_t_str )
        return make_obj(
            new_int(AS_STR(ob)->len),
            nst_t_int, free
        );
    else if ( ob->type == nst_t_map )
        return make_obj(
            new_int(AS_MAP(ob)->item_count),
            nst_t_int, free
        );
    else if ( IS_SEQ(ob) )
        return make_obj(
            new_int(AS_SEQ(ob)->len),
            nst_t_int, free
        );
    else
        RETURN_TYPE_ERROR("$");
}

Nst_Obj *obj_bwnot(Nst_Obj *ob, OpErr *err)
{
    if ( ob->type == nst_t_int )
        return make_obj(
            new_int(~AS_INT(ob)),
            nst_t_int, free
        );
    else
        RETURN_TYPE_ERROR("~");
}

Nst_Obj *obj_lgnot(Nst_Obj *ob, OpErr *err)
{
    ob = obj_cast(ob, nst_t_bool, err);
    
    if ( ob == nst_true )
    {
        dec_ref(nst_true);
        RETURN_FALSE;
    }
    else
    {
        dec_ref(nst_false);
        RETURN_TRUE;
    }
}

Nst_Obj *obj_stdout(Nst_Obj *ob, OpErr *err)
{
    Nst_Obj *str = obj_cast(ob, nst_t_str, err);
    Nst_string *text = AS_STR(str);
    fwrite(text->value, sizeof(char), text->len, stdout);
    fflush(stdout);
    dec_ref(str);
    return inc_ref(ob);
}

Nst_Obj *obj_stdin(Nst_Obj *ob, OpErr *err)
{
    ob = obj_cast(ob, nst_t_str, err);
    Nst_string *text = AS_STR(ob);
    printf("%s", text->value);
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

    return make_obj(
        new_string(new_buffer, i, true),
        nst_t_str, destroy_string
    );
}

Nst_Obj *obj_typeof(Nst_Obj *ob, OpErr *err)
{
    return inc_ref(ob->type);
}
