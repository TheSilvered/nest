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

    // the type of the type object is itself
    if ( type == NULL )
        type = obj;

    obj->type = type;
    obj->type_name = AS_STR(type->value)->value;

    inc_ref(obj->type);

    return obj;
}

void inc_ref(Nst_Obj *obj)
{
    obj->ref_count++;
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
    nst_t_type = make_obj(new_string("type",   4, false), NULL,       destroy_string);
    nst_t_int  = make_obj(new_string("int",    3, false), nst_t_type, destroy_string);
    nst_t_real = make_obj(new_string("real",   4, false), nst_t_type, destroy_string);
    nst_t_bool = make_obj(new_string("bool",   4, false), nst_t_type, destroy_string);
    nst_t_null = make_obj(new_string("null",   4, false), nst_t_type, destroy_string);
    nst_t_str  = make_obj(new_string("str",    3, false), nst_t_type, destroy_string);
    nst_t_arr  = make_obj(new_string("array",  5, false), nst_t_type, destroy_string);
    nst_t_vect = make_obj(new_string("vector", 6, false), nst_t_type, destroy_string);
    nst_t_map  = make_obj(new_string("map",    3, false), nst_t_type, destroy_string);
    nst_t_func = make_obj(new_string("func",   4, false), nst_t_type, destroy_string);
    nst_t_iter = make_obj(new_string("iter",   4, false), nst_t_type, destroy_string);
    nst_t_byte = make_obj(new_string("byte",   4, false), nst_t_type, destroy_string);
    nst_t_file = make_obj(new_string("iofile", 6, false), nst_t_type, destroy_string);

    nst_true  = make_obj(new_bool(NST_TRUE ), nst_t_bool, free);
    nst_false = make_obj(new_bool(NST_FALSE), nst_t_bool, free);
    nst_null = make_obj(NULL, nst_t_null, NULL);

    // There must always be a reference to these objects
    inc_ref(nst_t_type);
    inc_ref(nst_t_int );
    inc_ref(nst_t_real);
    inc_ref(nst_t_bool);
    inc_ref(nst_t_null);
    inc_ref(nst_t_str );
    inc_ref(nst_t_arr );
    inc_ref(nst_t_vect);
    inc_ref(nst_t_map );
    inc_ref(nst_t_func);
    inc_ref(nst_t_iter);
    inc_ref(nst_t_byte);
    inc_ref(nst_t_file);
    inc_ref(nst_true  );
    inc_ref(nst_false );
    inc_ref(nst_null  );
}
