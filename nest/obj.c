#include <stdlib.h>
#include <errno.h>
#include "obj.h"
#include "map.h"
#include "nst_types.h"
#include "ggc.h"

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

Nst_Obj *nst_alloc_obj(size_t size, Nst_Obj *type, void (*destructor)(void *))
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
    obj->flags = 0;

    // the type of the type object is itself
    if ( type == NULL )
        obj->type = obj;
    else
        obj->type = type;

    nst_inc_ref(obj->type);

    return obj;
}

void _nst_destroy_obj(Nst_Obj *obj)
{
    if ( obj->flags & NST_FLAG_GGC_IS_SUPPORTED )
    {
        // the object has already been deleted
        if ( NST_HAS_FLAG(obj, NST_FLAG_GGC_OBJ_DELETED) )
            return;

        obj->ref_count = 2147483647;
        if ( obj->destructor != NULL )
            (*obj->destructor)(obj);
        if ( obj != obj->type )
            nst_dec_ref(obj->type);

        // The object is being deleted by the garbage collector
        if ( NST_HAS_FLAG(obj, NST_FLAG_GGC_UNREACHABLE) )
            NST_SET_FLAG(obj, NST_FLAG_GGC_OBJ_DELETED);
        else
        {
            Nst_GGCObj *ggc_obj = (Nst_GGCObj *)obj;
            Nst_GGCList *ls = ggc_obj->ggc_list;

            if ( ls != NULL )
            {
                if ( ls->head == ggc_obj )
                    ls->head = ggc_obj->ggc_next;
                else
                    ggc_obj->ggc_prev = ggc_obj->ggc_next;

                if ( ls->tail == ggc_obj )
                    ls->tail = ggc_obj->ggc_prev;
                else
                    ggc_obj->ggc_prev = ggc_obj->ggc_next;

                ls->size--;
            }

            free(obj);
        }
    }
    else
    {
        if ( obj->destructor != NULL )
            (*obj->destructor)(obj);
        if ( obj != obj->type )
            nst_dec_ref(obj->type);

        free(obj);
    }
}

void _nst_init_obj(void)
{
    nst_t_type = NULL;
    nst_t_type = nst_new_type_obj("Type",   4);
    nst_t_int  = nst_new_type_obj("Int",    3);
    nst_t_real = nst_new_type_obj("Real",   4);
    nst_t_bool = nst_new_type_obj("Bool",   4);
    nst_t_null = nst_new_type_obj("Null",   4);
    nst_t_str  = nst_new_type_obj("Str",    3);
    nst_t_arr  = nst_new_type_obj("Array",  5);
    nst_t_vect = nst_new_type_obj("Vector", 6);
    nst_t_map  = nst_new_type_obj("Map",    3);
    nst_t_func = nst_new_type_obj("Func",   4);
    nst_t_iter = nst_new_type_obj("Iter",   4);
    nst_t_byte = nst_new_type_obj("Byte",   4);
    nst_t_file = nst_new_type_obj("IOfile", 6);

    nst_true  = nst_new_bool(NST_TRUE );
    nst_false = nst_new_bool(NST_FALSE);
    nst_null  = nst_alloc_obj(sizeof(Nst_Obj), nst_t_null, NULL);
}

void _nst_del_obj(void)
{
    nst_destroy_obj(nst_true);
    nst_destroy_obj(nst_false);
    nst_destroy_obj(nst_null);

    nst_destroy_obj(nst_t_int);
    nst_destroy_obj(nst_t_real);
    nst_destroy_obj(nst_t_bool);
    nst_destroy_obj(nst_t_null);
    nst_destroy_obj(nst_t_str);
    nst_destroy_obj(nst_t_arr);
    nst_destroy_obj(nst_t_vect);
    nst_destroy_obj(nst_t_map);
    nst_destroy_obj(nst_t_func);
    nst_destroy_obj(nst_t_iter);
    nst_destroy_obj(nst_t_byte);
    nst_destroy_obj(nst_t_file);
    nst_destroy_obj(nst_t_type);
}
