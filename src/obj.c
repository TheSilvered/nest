#include "obj.h"
#include "map.h"
#include "ggc.h"
#include "str.h"
#include "mem.h"

Nst_Obj *_nst_obj_alloc(usize               size,
                        struct _Nst_StrObj *type,
                        void (*destructor)(void *))
{
    Nst_Obj *obj = OBJ(nst_malloc(1, size));
    if ( obj == NULL )
    {
        return NULL;
    }

    obj->ref_count = 1;
    obj->destructor = destructor;
    obj->hash = -1;
    obj->flags = 0;

    // the type of the object is itself
    if ( type == NULL )
    {
        obj->type = TYPE(obj);
    }
    else
    {
        obj->type = type;
    }

    nst_inc_ref(obj->type);

    return obj;
}

void _nst_obj_destroy(Nst_Obj *obj)
{
    if ( !NST_FLAG_HAS(obj, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        if ( obj->destructor != NULL )
        {
            (*obj->destructor)(obj);
        }
        if ( obj != OBJ(obj->type) )
        {
            nst_dec_ref(obj->type);
        }

        nst_free(obj);
        return;
    }

    if ( NST_FLAG_HAS(obj, NST_FLAG_GGC_DELETED) )
    {
        return;
    }

    obj->ref_count = 2147483647;
    if ( obj->destructor != NULL )
    {
        (*obj->destructor)(obj);
    }
    if ( obj != OBJ(obj->type) )
    {
        nst_dec_ref(obj->type);
    }

    // if the object is being deleted by the garbage collector
    if ( NST_FLAG_HAS(obj, NST_FLAG_GGC_UNREACHABLE) )
    {
        NST_FLAG_SET(obj, NST_FLAG_GGC_DELETED);
        return;
    }

    Nst_GGCObj *ggc_obj = GGC_OBJ(obj);
    Nst_GGCList *ls = ggc_obj->ggc_list;

    if ( ls != NULL )
    {
        if ( ls->head == ggc_obj )
        {
            ls->head = ggc_obj->ggc_next;
        }
        else
        {
            ggc_obj->ggc_prev->ggc_next = ggc_obj->ggc_next;
        }

        if ( ls->tail == ggc_obj )
        {
            ls->tail = ggc_obj->ggc_prev;
        }
        else
        {
            ggc_obj->ggc_next->ggc_prev = ggc_obj->ggc_prev;
        }

        ls->size--;
    }

    nst_free(obj);
}

Nst_Obj *_nst_inc_ref(Nst_Obj *obj)
{
    obj->ref_count++;
    return obj;
}

void _nst_dec_ref(Nst_Obj *obj)
{
    obj->ref_count--;
    if ( obj->ref_count <= 0 || (obj == OBJ(obj->type) && obj->ref_count == 1) )
    {
        _nst_obj_destroy(obj);
    }
}
