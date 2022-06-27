#include <stdlib.h>
#include <errno.h>
#include "obj.h"
#include "map.h"
#include "nst_types.h"

Nst_Obj *nst_t_type;
Nst_Obj *nst_t_int;
Nst_Obj *nst_t_real;
Nst_Obj *nst_t_bool;
Nst_Obj *nst_t_null;
Nst_Obj *nst_t_str;
Nst_Obj *nst_t_arr;
Nst_Obj *nst_t_vect;
Nst_Obj *nst_t_map;
Nst_Obj *nst_t_func;
Nst_Obj *nst_t_iter;
Nst_Obj *nst_t_byte;
Nst_Obj *nst_t_file;

Nst_Obj *nst_true;
Nst_Obj *nst_false;
Nst_Obj *nst_null;

Nst_Obj *make_obj(void *value, Nst_Obj *type, void (*destructor)(void *))
{
    Nst_Obj *obj = malloc(sizeof(Nst_Obj));
    if ( obj == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    obj->ref_count = 1;
    obj->value = value;
    obj->destructor = destructor;
    obj->hash = -1;

    // the type of the type object is itself
    if ( type == NULL )
        type = obj;

    obj->type = type;
    obj->type_name = AS_STR(type)->value;

    inc_ref(obj->type);

    return obj;
}

Nst_Obj *make_obj_free(void *value, Nst_Obj *type)
{
    return make_obj(value, type, free);
}

Nst_Obj *inc_ref(Nst_Obj *obj)
{
    obj->ref_count++;
    return obj;
}

void dec_ref(Nst_Obj *obj)
{
    obj->ref_count--;
    if ( obj->ref_count <= 0 )
        destroy_obj(obj);
}

void destroy_obj(Nst_Obj *obj)
{
    if ( obj->value != NULL && obj->destructor != NULL )
        (*obj->destructor)(obj->value);
    dec_ref(obj->type);
    free(obj);
}

void init_obj(void)
{
    nst_t_type = make_obj(new_string("Type",   4, false), NULL,       destroy_string);
    nst_t_int  = make_obj(new_string("Int",    3, false), nst_t_type, destroy_string);
    nst_t_real = make_obj(new_string("Real",   4, false), nst_t_type, destroy_string);
    nst_t_bool = make_obj(new_string("Bool",   4, false), nst_t_type, destroy_string);
    nst_t_null = make_obj(new_string("Null",   4, false), nst_t_type, destroy_string);
    nst_t_str  = make_obj(new_string("Str",    3, false), nst_t_type, destroy_string);
    nst_t_arr  = make_obj(new_string("Array",  5, false), nst_t_type, destroy_string);
    nst_t_vect = make_obj(new_string("Vector", 6, false), nst_t_type, destroy_string);
    nst_t_map  = make_obj(new_string("Map",    3, false), nst_t_type, destroy_string);
    nst_t_func = make_obj(new_string("Func",   4, false), nst_t_type, destroy_string);
    nst_t_iter = make_obj(new_string("Iter",   4, false), nst_t_type, destroy_string);
    nst_t_byte = make_obj(new_string("Byte",   4, false), nst_t_type, destroy_string);
    nst_t_file = make_obj(new_string("IOfile", 6, false), nst_t_type, destroy_string);

    nst_true  = make_obj(new_bool(NST_TRUE ), nst_t_bool, free);
    nst_false = make_obj(new_bool(NST_FALSE), nst_t_bool, free);
    nst_null  = make_obj(NULL, nst_t_null, NULL);
}

void del_obj(void)
{
    dec_ref(nst_t_type);
    dec_ref(nst_t_int);
    dec_ref(nst_t_real);
    dec_ref(nst_t_bool);
    dec_ref(nst_t_null);
    dec_ref(nst_t_str);
    dec_ref(nst_t_arr);
    dec_ref(nst_t_vect);
    dec_ref(nst_t_map);
    dec_ref(nst_t_func);
    dec_ref(nst_t_iter);
    dec_ref(nst_t_byte);
    dec_ref(nst_t_file);
    dec_ref(nst_true);
    dec_ref(nst_false);
    dec_ref(nst_null);
}
