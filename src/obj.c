#include "obj.h"
#include "map.h"
#include "ggc.h"
#include "str.h"
#include "mem.h"
#include "type.h"
#include "interpreter.h"

static Nst_Obj *pop_p_head(usize size, Nst_TypeObj *type)
{
    Nst_Obj *obj;

    if (type != NULL && type->p_head != NULL) {
        obj = type->p_head;
        type->p_head = obj->p_next;
        type->p_len--;
        // p_next is taken care of in _Nst_obj_alloc
    } else
        obj = OBJ(Nst_malloc(1, size));

    return obj;
}

Nst_Obj *_Nst_obj_alloc(usize size, struct _Nst_TypeObj *type)
{
    Nst_Obj *obj = pop_p_head(size, type);
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
    obj->p_next = NULL;
    obj->hash = -1;
    obj->flags = 0;

    obj->type = type;
    Nst_inc_ref(type);
    return obj;
}

void _Nst_obj_destroy(Nst_Obj *obj)
{
    if (Nst_HAS_FLAG(obj, Nst_FLAG_OBJ_DESTROYED))
        return;

    obj->ref_count = 2147483647;
    if (obj->type->dstr != NULL)
        obj->type->dstr(obj);

    Nst_SET_FLAG(obj, Nst_FLAG_OBJ_DESTROYED);
}

void _Nst_obj_free(Nst_Obj *obj)
{
    Nst_assert(Nst_HAS_FLAG(obj, Nst_FLAG_OBJ_DESTROYED));

    if (Nst_HAS_FLAG(obj, Nst_FLAG_GGC_PRESERVE_MEM))
        return;

    Nst_TypeObj *ob_t = obj->type;

    if (Nst_HAS_FLAG(obj, Nst_FLAG_GGC_IS_SUPPORTED)) {
        Nst_GGCObj *ggc_obj = GGC_OBJ(obj);
        Nst_GGCList *ls = ggc_obj->ggc_list;

        // if ls is NULL it means that the object is being deleted in a garbage
        // collection
        if (ls == NULL)
            goto free_mem;

        if (ls->head == ggc_obj)
            ls->head = GGC_OBJ(ggc_obj->p_next);
        else
            ggc_obj->p_prev->p_next = ggc_obj->p_next;

        if (ls->tail == ggc_obj)
            ls->tail = GGC_OBJ(ggc_obj->p_prev);
        else
            GGC_OBJ(ggc_obj->p_next)->p_prev = ggc_obj->p_prev;

        ls->len--;
    }

free_mem:

// silences the warning of the expression being always true when _Nst_P_LEN_MAX
// is 0 (e.g. when pools are disabled)

#if defined(Nst_DISABLE_POOLS) && !defined(Nst_MSVC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

    if (obj->type->p_len >= _Nst_P_LEN_MAX) {
        Nst_free(obj);

        if (obj != OBJ(ob_t))
            Nst_dec_ref(ob_t);

        return;
    }

#if defined(Nst_DISABLE_POOLS) && !defined(Nst_MSVC)
#pragma GCC diagnostic pop
#endif

    obj->p_next = obj->type->p_head;
    obj->type->p_head = obj;
    obj->type->p_len++;

    if (obj != OBJ(ob_t))
        Nst_dec_ref(ob_t);
}

Nst_Obj *_Nst_inc_ref(Nst_Obj *obj)
{
    obj->ref_count++;
    return obj;
}

void _Nst_dec_ref(Nst_Obj *obj)
{
    obj->ref_count--;

    // The ref_count should nevere be below zero
    Nst_assert(obj->ref_count >= 0);

    if (obj->ref_count <= 0) {
        _Nst_obj_destroy(obj);
        _Nst_obj_free(obj);
    }
}
