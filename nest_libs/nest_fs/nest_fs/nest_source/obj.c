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

Nst_Obj *alloc_obj(size_t size, Nst_Obj *type, void (*destructor)(void *))
{
    Nst_Obj *obj = malloc(size);
    if ( obj == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    obj->ref_count = 1;
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

void init_obj(void)
{
    nst_t_type = NULL;
    nst_t_type = new_type_obj("Type",   4);
    nst_t_int  = new_type_obj("Int",    3);
    nst_t_real = new_type_obj("Real",   4);
    nst_t_bool = new_type_obj("Bool",   4);
    nst_t_null = new_type_obj("Null",   4);
    nst_t_str  = new_type_obj("Str",    3);
    nst_t_arr  = new_type_obj("Array",  5);
    nst_t_vect = new_type_obj("Vector", 6);
    nst_t_map  = new_type_obj("Map",    3);
    nst_t_func = new_type_obj("Func",   4);
    nst_t_iter = new_type_obj("Iter",   4);
    nst_t_byte = new_type_obj("Byte",   4);
    nst_t_file = new_type_obj("IOfile", 6);

    nst_true  = new_bool(NST_TRUE );
    nst_false = new_bool(NST_FALSE);
    nst_null  = alloc_obj(sizeof(Nst_Obj), nst_t_null, NULL);
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
