#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "obj_ops.h"
#include "nst_types.h"

#define MAX_INT_CHAR_COUNT 21
#define MAX_REAL_CHAR_COUNT 25
#define MAX_BYTE_CHAR_COUNT 5

#define IS_NUMBER(obj) ( obj->type == nst_t_int || obj->type == nst_t_real )
#define IS_SEQ(obj) ( obj->type == nst_t_arr || obj->type == nst_t_vect )
#define ARE_TYPE(nst_type) ( ob1->type == nst_type && ob2->type == nst_type )

#define RETURN_TYPE_ERROR(operand) \
    err->name = "Type Error"; \
    err->message = "invalid type for '" operand "'"; \
    return NULL

#define RETURN_TRUE inc_ref(nst_true); return nst_true
#define RETURN_FALSE inc_ref(nst_false); return nst_false
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
    register Nst_Obj *v1 = ob1->value;
    register Nst_Obj *v2 = ob2->value;

    if ( ob1 == ob2 )
    {
        RETURN_TRUE;
    }
    else if ( ob1->type == nst_t_map || ob2->type == nst_t_map )
    {
        RETURN_FALSE;
    }
    else if ( ARE_TYPE(nst_t_int) ) {
        RETURN_COND(*AS_INT(v1) == *AS_INT(v2));
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);
        bool check = *AS_REAL(v1) == *AS_REAL(v2);
        dec_ref(ob1);
        dec_ref(ob2);

        if ( check )
        {
            RETURN_TRUE;
        }
        else
        {
            RETURN_FALSE;
        }
    }
    else if ( ob1->type == nst_t_str && ob2->type == nst_t_str )
    {
        Nst_string s1 = *AS_STR(v1);
        Nst_string s2 = *AS_STR(v2);

        RETURN_COND(strcmp(s1.value, s2.value) == 0);
    }
    else if ( ob1->type == nst_t_bool && ob2->type == nst_t_bool )
    {
        RETURN_COND(v1 == v2);
    }
    else if ( IS_SEQ(ob1) && IS_SEQ(ob2) )
    {
        if ( AS_SEQ(v1)->len != AS_SEQ(v2)->len )
        {
            RETURN_FALSE;
        }

        for ( size_t i = 0, n = AS_SEQ(v1)->len; i < n; i++ )
        {
            if ( obj_eq(AS_SEQ(v1)->objs[i], AS_SEQ(v2)->objs[i], err) == nst_false )
            {
                RETURN_FALSE;
            }
        }
        RETURN_TRUE;
    }
    else
    {
        RETURN_FALSE;
    }
}

Nst_Obj *obj_ne(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    return obj_lgnot(obj_eq(ob1, ob2, err), err);
}

Nst_Obj *obj_gt(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    register Nst_Obj *v1 = ob1->value;
    register Nst_Obj *v2 = ob2->value;

    if ( ARE_TYPE(nst_t_str) )
    {
        Nst_string s1 = *AS_STR(v1);
        Nst_string s2 = *AS_STR(v2);

        RETURN_COND(strcmp(s1.value, s2.value) > 0);
    }
    else if ( ARE_TYPE(nst_t_int) )
    {
        RETURN_COND(*AS_INT(v1) > *AS_INT(v2));
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);
        bool check = *AS_REAL(v1) > *AS_REAL(v2);
        dec_ref(ob1);
        dec_ref(ob2);

        RETURN_COND(check);
    }
    else
    {
        RETURN_TYPE_ERROR(">");
    }
}

Nst_Obj *obj_lt(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    register Nst_Obj *v1 = ob1->value;
    register Nst_Obj *v2 = ob2->value;

    if ( ARE_TYPE(nst_t_str) )
    {
        Nst_string s1 = *AS_STR(v1);
        Nst_string s2 = *AS_STR(v2);

        RETURN_COND(strcmp(s1.value, s2.value) < 0);
    }
    else if ( ARE_TYPE(nst_t_int) )
    {
        RETURN_COND(*AS_INT(v1) < *AS_INT(v2));
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);
        bool check = *AS_REAL(v1) < *AS_REAL(v2);
        dec_ref(ob1);
        dec_ref(ob2);

        RETURN_COND(check);
    }
    else
    {
        RETURN_TYPE_ERROR("<");
    }
}

Nst_Obj *obj_ge(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( obj_eq(ob1, ob2, err) == nst_true )
    {
        RETURN_TRUE;
    }

    Nst_Obj *res = obj_gt(ob1, ob2, err);

    if ( strcmp(err->name, "Type Error") == 0 )
    {
        err->message = "invalid type for '>='";
    }
    return res;
}

Nst_Obj *obj_le(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( obj_eq(ob1, ob2, err) == nst_true )
    {
        RETURN_TRUE;
    }

    Nst_Obj *res = obj_lt(ob1, ob2, err);

    if ( strcmp(err->name, "Type Error") == 0 )
    {
        err->message = "invalid type for '<='";
    }
    return res;
}

// Arithmetic operations
Nst_Obj *obj_add(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(*AS_INT(ob1->value) + *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        Nst_Obj *new_obj = make_obj(
            new_real(*AS_REAL(ob1->value) + *AS_REAL(ob2->value)),
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
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(*AS_INT(ob1->value) - *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        Nst_Obj *new_obj = make_obj(
            new_real(*AS_REAL(ob1->value) - *AS_REAL(ob2->value)),
            nst_t_real, free
        );

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_TYPE_ERROR("-");
    }
}

Nst_Obj *obj_mul(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(*AS_INT(ob1->value) * *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        Nst_Obj *new_obj = make_obj(
            new_real(*AS_REAL(ob1->value) * *AS_REAL(ob2->value)),
            nst_t_real, free
        );

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_TYPE_ERROR("*");
    }
}

Nst_Obj *obj_div(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        if ( *AS_INT(ob2->value) == 0 )
        {
            err->name = "Math Error";
            err->message = "division by zero";
            return NULL;
        }

        return make_obj(
            new_int(*AS_INT(ob1->value) / *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        if ( *AS_REAL(ob2->value) == 0.0 )
        {
            err->name = "Math Error";
            err->message = "division by zero";
            return NULL;
        }

        Nst_Obj *new_obj = make_obj(
            new_real(*AS_REAL(ob1->value) / *AS_REAL(ob2->value)),
            nst_t_real, free
        );

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_TYPE_ERROR("/");
    }
}

Nst_Obj *obj_pow(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int((Nst_int)powl(
                (Nst_real)*AS_INT(ob1->value),
                (Nst_real)*AS_INT(ob2->value)
            )),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        Nst_real v1 = *AS_REAL(ob1->value);
        Nst_real v2 = *AS_REAL(ob2->value);

        dec_ref(ob1);
        dec_ref(ob2);

        // any root of a negative number gives -nan as a result
        if ( v1 < 0 && floor(v2) != v2 )
        {
            err->name = "Math Error";
            err->message = "fractional power of a negative number";
            return NULL;
        }

        return make_obj(
            new_real((Nst_real)powl(v1, v2)),
            nst_t_real, free
        );
    }
    else
    {
        RETURN_TYPE_ERROR("^");
    }
}

Nst_Obj *obj_mod(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        if ( *AS_INT(ob2->value) == 0 )
        {
            err->name = "Math Error";
            err->message = "modulo by zero";
            return NULL;
        }

        return make_obj(
            new_int(*AS_INT(ob1->value) % *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else if ( IS_NUMBER(ob1) && IS_NUMBER(ob2) )
    {
        ob1 = obj_cast(ob1, nst_t_real, err);
        ob2 = obj_cast(ob2, nst_t_real, err);

        if ( *AS_REAL(ob2->value) == 0.0 )
        {
            err->name = "Math Error";
            err->message = "modulo by zero";
            return NULL;
        }

        Nst_Obj *new_obj = make_obj(
            new_real(fmodl(*AS_REAL(ob1->value), *AS_REAL(ob2->value))),
            nst_t_real, free
        );

        dec_ref(ob1);
        dec_ref(ob2);

        return new_obj;
    }
    else
    {
        RETURN_TYPE_ERROR("/");
    }
}

// Bitwise operations
Nst_Obj *obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(*AS_INT(ob1->value) | *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else
    {
        RETURN_TYPE_ERROR("|");
    }
}

Nst_Obj *obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(*AS_INT(ob1->value) & *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else
    {
        RETURN_TYPE_ERROR("&");
    }
}

Nst_Obj *obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(*AS_INT(ob1->value) ^ *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else
    {
        RETURN_TYPE_ERROR("^^");
    }
}

Nst_Obj *obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(*AS_INT(ob1->value) << *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else
    {
        RETURN_TYPE_ERROR("<<");
    }
}

Nst_Obj *obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    if ( ARE_TYPE(nst_t_int) )
    {
        return make_obj(
            new_int(*AS_INT(ob1->value) >> *AS_INT(ob2->value)),
            nst_t_int, free
        );
    }
    else
    {
        RETURN_TYPE_ERROR(">>");
    }
}

// Logical operations
Nst_Obj *obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    ob1 = obj_cast(ob1, nst_t_bool, err);
    ob2 = obj_cast(ob2, nst_t_bool, err);

    Nst_bool v1 = *AS_BOOL(ob1->value);
    Nst_bool v2 = *AS_BOOL(ob2->value);

    dec_ref(ob1);
    dec_ref(ob2);

    RETURN_COND( v1 == NST_TRUE || v2 == NST_TRUE );
}

Nst_Obj *obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    ob1 = obj_cast(ob1, nst_t_bool, err);
    ob2 = obj_cast(ob2, nst_t_bool, err);

    Nst_bool v1 = *AS_BOOL(ob1->value);
    Nst_bool v2 = *AS_BOOL(ob2->value);

    dec_ref(ob1);
    dec_ref(ob2);

    RETURN_COND(v1 == NST_TRUE && v2 == NST_TRUE);
}

Nst_Obj *obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    ob1 = obj_cast(ob1, nst_t_bool, err);
    ob2 = obj_cast(ob2, nst_t_bool, err);

    Nst_bool v1 = *AS_BOOL(ob1->value);
    Nst_bool v2 = *AS_BOOL(ob2->value);

    dec_ref(ob1);
    dec_ref(ob2);

    RETURN_COND((v1 == NST_TRUE && v2 == NST_FALSE) ||
                (v1 == NST_FALSE && v2 == NST_TRUE));
}

// Other
Nst_Obj *obj_cast(Nst_Obj *ob, Nst_Obj *type, OpErr *err)
{
    register Nst_Obj *ob_t = ob->type;
    register void *ob_val = ob->value;

    if ( ob_t == type )
    {
        inc_ref(ob);
        return ob;
    }

    if ( type == nst_t_str )
    {
        if ( ob_t == nst_t_int )
        {
            char *buffer = malloc(MAX_INT_CHAR_COUNT * sizeof(char));
            if ( buffer == NULL )
            {
                errno = ENOMEM;
                return NULL;
            }
            sprintf(buffer, "%lli", *AS_INT(ob_val));
            return make_obj(
                new_string_raw(buffer, true),
                nst_t_str, destroy_string
            );
        }
        else if ( ob_t == nst_t_real )
        {
            char *buffer = malloc(MAX_REAL_CHAR_COUNT * sizeof(char));
            if ( buffer == NULL )
            {
                errno = ENOMEM;
                return NULL;
            }
            sprintf(buffer, "%g", *AS_REAL(ob_val));
            return make_obj(
                new_string_raw(buffer, true),
                nst_t_str, destroy_string
            );
        }
        else if ( ob_t == nst_t_bool )
        {
            if ( *AS_BOOL(ob_val) == NST_TRUE )
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
            if ( *AS_BYTE(ob_val) >= '!' && *AS_BYTE(ob_val) <= '~' )
            {
                char str[2] = { *AS_BYTE(ob_val), 0 };

                return make_obj(
                    new_string(str, 1, false),
                    nst_t_str, destroy_string
                );
            }
            
            char str[MAX_BYTE_CHAR_COUNT] = { 0 };
            sprintf(str, "\\%i", *AS_BYTE(ob_val));

            return make_obj(
                new_string_raw(str, false),
                nst_t_str, destroy_string
            );
        }
        else
        {
            char *buffer = malloc(sizeof(char) * (AS_STR(ob_val)->len + 10));
            if ( buffer == NULL )
            {
                errno = ENOMEM;
                return NULL;
            }

            sprintf(buffer, "<%s object>", AS_STR(ob_val)->value);

            return make_obj(
                new_string_raw(buffer, true),
                nst_t_str, destroy_string
            );
        }
    }
    else if ( type == nst_t_bool )
    {
        if ( ob_t == nst_t_int )
            RETURN_COND(*AS_INT(ob_val) != 0);
        else if ( ob_t == nst_t_real )
            RETURN_COND(*AS_REAL(ob_val) != 0.0);
        else if ( ob_t == nst_t_str )
            RETURN_COND(AS_STR(ob_val)->len != 0);
        else if ( ob_t == nst_t_map )
            RETURN_COND(AS_MAP(ob_val)->item_count != 0);
        else if ( ob_t == nst_t_arr || ob_t == nst_t_vect )
            RETURN_COND(AS_SEQ(ob_val)->len != 0);
        else if ( ob_t == nst_t_null )
        {
            RETURN_FALSE;
        }
        else if ( ob_t == nst_t_byte )
            RETURN_COND(*AS_BYTE(ob_val) != 0);
        else
        {
            RETURN_TRUE;
        }
    }
    else if ( type == nst_t_int )
    {
        if ( ob_t == nst_t_real )
            return make_obj(
                new_int((Nst_int)*AS_REAL(ob_val)),
                nst_t_int, free
            );
        else if ( ob_t == nst_t_byte )
            return make_obj(
                new_int((Nst_int)*AS_INT(ob_val)),
                nst_t_int, free
            );
        else
        {
            RETURN_TYPE_ERROR("::");
        }
    }
    else if ( type == nst_t_real )
    {
        if ( ob_t == nst_t_int )
            return make_obj(
                new_real((Nst_real)*AS_INT(ob_val)),
                nst_t_real, free
            );
        else if ( ob_t == nst_t_byte )
            return make_obj(
                new_real((Nst_real)*AS_BYTE(ob_val)),
                nst_t_real, free
            );
        else
        {
            RETURN_TYPE_ERROR("::");
        }
    }
    else
    {
        RETURN_TYPE_ERROR("::");
    }
}

Nst_Obj *obj_concat(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err)
{
    ob1 = obj_cast(ob1, nst_t_str, err);
    ob2 = obj_cast(ob2, nst_t_str, err);

    Nst_string *nst_s1 = ob1->value;
    Nst_string *nst_s2 = ob2->value;

    register char *s1 = AS_STR(nst_s1)->value;
    register char *s2 = AS_STR(nst_s2)->value;
    register size_t len1 = AS_STR(nst_s1)->len;
    register size_t len2 = AS_STR(nst_s2)->len;
    register size_t tot_len = len1 + len2;

    char *buffer = malloc(sizeof(char) * (tot_len + 1));

    if ( buffer == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

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
Nst_Obj *obj_len(Nst_Obj *ob, OpErr *err)
{
    if ( ob->type == nst_t_str )
        return make_obj(
            new_int(AS_STR(ob->value)->len),
            nst_t_int, free
        );
    else if ( ob->type == nst_t_map )
        return make_obj(
            new_int(AS_MAP(ob->value)->item_count),
            nst_t_int, free
        );
    else if ( IS_SEQ(ob) )
        return make_obj(
            new_int(AS_SEQ(ob->value)->len),
            nst_t_int, free
        );
    else
    {
        RETURN_TYPE_ERROR("$");
    }
}

Nst_Obj *obj_bwnot(Nst_Obj *ob, OpErr *err)
{
    if ( ob->type == nst_t_int )
        return make_obj(
            new_int(~*AS_INT(ob->value)),
            nst_t_int, free
        );
    else
    {
        RETURN_TYPE_ERROR("~");
    }
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
    Nst_string *text = AS_STR(str->value);
    printf("%s", text->value);
    dec_ref(str);
    inc_ref(ob);
    return ob;
}

Nst_Obj *obj_stdin(Nst_Obj *ob, OpErr *err)
{
    ob = obj_cast(ob, nst_t_str, err);
    Nst_string *text = AS_STR(ob->value);
    printf("%s", text->value);
    dec_ref(ob);

    char *buffer = malloc(4);
    if ( buffer == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

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
        new_string(buffer, i, true),
        nst_t_str, destroy_string
    );
}

Nst_Obj *obj_typeof(Nst_Obj *ob, OpErr *err)
{
    inc_ref(ob->type);
    return ob->type;
}
