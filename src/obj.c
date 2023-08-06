#include "obj.h"
#include "map.h"
#include "ggc.h"
#include "str.h"
#include "mem.h"

#ifdef Nst_TRACK_OBJ_INIT_POS
#include "interpreter.h"
#endif

Nst_Obj *_Nst_obj_alloc(usize size, struct _Nst_StrObj *type,
                        void (*destructor)(void *))
{
    Nst_Obj *obj = OBJ(Nst_malloc(1, size));
    if (obj == NULL)
        return NULL;

#ifdef Nst_TRACK_OBJ_INIT_POS
    Nst_Inst *inst = Nst_current_inst();
    if (inst == NULL) {
        obj->init_line = -1;
        obj->init_col = -1;
        obj->init_path = NULL;
    } else {
        obj->init_line = inst->start.line;
        obj->init_col = inst->start.col;
        obj->init_path = inst->start.text->path;
    }
#endif

    obj->ref_count = 1;
    obj->destructor = destructor;
    obj->hash = -1;
    obj->flags = 0;

    // if the type of the object is itself
    if (type == NULL)
        obj->type = TYPE(obj);
    else
        obj->type = type;

    Nst_inc_ref(obj->type);
    return obj;
}

void _Nst_obj_destroy(Nst_Obj *obj)
{
    if (!Nst_FLAG_HAS(obj, Nst_FLAG_GGC_IS_SUPPORTED)) {
        if (obj->destructor != NULL)
            (*obj->destructor)(obj);
        if (obj != OBJ(obj->type))
            Nst_dec_ref(obj->type);

        Nst_free(obj);
        return;
    }

    if (Nst_FLAG_HAS(obj, Nst_FLAG_GGC_DELETED))
        return;

    // If the object is being freed during a collection and it is referenced
    // by other objects in the same generation this prevents the destructor
    // from being called more than once.
    obj->ref_count = 2147483647;
    if (obj->destructor != NULL)
        (*obj->destructor)(obj);
    if (obj != OBJ(obj->type))
        Nst_dec_ref(obj->type);

    // if the object is being deleted by the garbage collector
    if (Nst_FLAG_HAS(obj, Nst_FLAG_GGC_UNREACHABLE)) {
        Nst_FLAG_SET(obj, Nst_FLAG_GGC_DELETED);
        return;
    }

    Nst_GGCObj *ggc_obj = GGC_OBJ(obj);
    Nst_GGCList *ls = ggc_obj->ggc_list;

    if (ls != NULL) {
        if (ls->head == ggc_obj)
            ls->head = ggc_obj->ggc_next;
        else
            ggc_obj->ggc_prev->ggc_next = ggc_obj->ggc_next;

        if (ls->tail == ggc_obj)
            ls->tail = ggc_obj->ggc_prev;
        else
            ggc_obj->ggc_next->ggc_prev = ggc_obj->ggc_prev;

        ls->len--;
    }

    Nst_free(obj);
}

Nst_Obj *_Nst_inc_ref(Nst_Obj *obj)
{
    obj->ref_count++;
    return obj;
}

void _Nst_dec_ref(Nst_Obj *obj)
{
    obj->ref_count--;
    if (obj->ref_count <= 0 || (obj == OBJ(obj->type) && obj->ref_count == 1))
        _Nst_obj_destroy(obj);
}
