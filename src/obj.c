#include <string.h>
#include "obj.h"
#include "map.h"
#include "ggc.h"
#include "str.h"
#include "mem.h"
#include "type.h"
#include "interpreter.h"
#include "global_consts.h"

#define GGC_OBJ(obj) ((Nst_GGCObj *)(obj))

#define TYPE_HEAD                                                             \
    Nst_Obj *p_head;                                                          \
    usize p_len;                                                              \
    Nst_StrView name;                                                         \
    Nst_ObjDstr dstr

/**
 * @param p_head: the head object in the type's pool
 * @param p_len: the length of the pool
 * @param name: the name of the object as a Nest string
 * @param dstr: the destructor of the type, can be NULL
 */
NstEXP typedef struct _Nst_TypeObj {
    Nst_OBJ_HEAD;
    Nst_Obj *p_head;
    usize p_len;
    Nst_StrView name;
    Nst_ObjDstr dstr;
    Nst_ObjTrav trav;
} Nst_TypeObj;

#define TYPE(ptr) ((Nst_TypeObj *)(ptr))

Nst_Obj *Nst_type_new(const char *name, Nst_ObjDstr dstr)
{
    Nst_assert_c(Nst_encoding_check(
        Nst_encoding(Nst_EID_EXT_UTF8),
        (void *)name,
        strlen(name)) == -1);

    Nst_TypeObj *type = Nst_obj_alloc(Nst_TypeObj, Nst_t.Type);
    if (type == NULL)
        return NULL;

    type->p_head = NULL;
    type->p_len = 0;
    type->dstr = dstr;
    type->trav = NULL;
    type->name = Nst_sv_new_c(name);

    return NstOBJ(type);
}

Nst_Obj *Nst_cont_type_new(const char *name, Nst_ObjDstr dstr,
                           Nst_ObjTrav trav)
{
    Nst_assert_c(Nst_encoding_check(
        Nst_encoding(Nst_EID_EXT_UTF8),
        (void *)name,
        strlen(name)) == -1);

    Nst_TypeObj *type = Nst_obj_alloc(Nst_TypeObj, Nst_t.Type);
    if (type == NULL)
        return NULL;

    type->p_head = NULL;
    type->p_len = 0;
    type->dstr = dstr;
    type->trav = trav;
    type->name = Nst_sv_new_c(name);

    return NstOBJ(type);
}

Nst_Obj *_Nst_type_new_no_err(const char *name, Nst_ObjDstr dstr)
{
    Nst_TypeObj *type = TYPE(Nst_raw_malloc(sizeof(Nst_TypeObj)));
    if (type == NULL)
        return NULL;

#ifdef Nst_DBG_TRACK_OBJ_INIT_POS
    type->init_line = -1;
    type->init_col = -1;
    type->init_path = NULL;
#endif

    type->ref_count = 1;
    type->p_next = NULL;
    type->hash = -1;
    type->flags = 0;
    type->p_head = NULL;
    type->p_len = 0;
    type->dstr = dstr;
    type->name = Nst_sv_new_c(name);

    type->type = Nst_t.Type;
    Nst_ninc_ref(Nst_t.Type);
    return NstOBJ(type);
}

void _Nst_type_destroy(Nst_Obj *type)
{
    for (Nst_Obj *ob = TYPE(type)->p_head; ob != NULL;) {
        Nst_Obj *next_ob = ob->p_next;
        Nst_free(ob);
        ob = next_ob;
    }

    TYPE(type)->p_len = _Nst_P_LEN_MAX + 1;
}

Nst_StrView Nst_type_name(Nst_Obj *type)
{
    Nst_assert(type->type == Nst_t.Type);
    return TYPE(type)->name;
}

Nst_ObjTrav Nst_type_trav(Nst_Obj *type)
{
    Nst_assert(type->type == Nst_t.Type);
    return TYPE(type)->trav;
}

static Nst_Obj *pop_p_head(usize size, Nst_Obj *type)
{
    Nst_Obj *obj;

    if (type != NULL && TYPE(type)->p_head != NULL) {
        obj = TYPE(type)->p_head;
        TYPE(type)->p_head = obj->p_next;
        TYPE(type)->p_len--;
        // p_next is taken care of in _Nst_obj_alloc
    } else
        obj = NstOBJ(Nst_malloc(1, size));

    return obj;
}

Nst_Obj *_Nst_obj_alloc(usize size, Nst_Obj *type)
{
    Nst_assert(type->type == Nst_t.Type);

    Nst_Obj *obj = pop_p_head(size, type);
    if (obj == NULL)
        return NULL;

#ifdef Nst_DBG_TRACK_OBJ_INIT_POS
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
    if (TYPE(obj->type)->dstr != NULL)
        TYPE(obj->type)->dstr(obj);

    Nst_SET_FLAG(obj, Nst_FLAG_OBJ_DESTROYED);
}

void _Nst_obj_free(Nst_Obj *obj)
{
    Nst_assert(Nst_HAS_FLAG(obj, Nst_FLAG_OBJ_DESTROYED));

    if (Nst_HAS_FLAG(obj, Nst_FLAG_GGC_PRESERVE_MEM))
        return;

    Nst_Obj *ob_t = obj->type;

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

#if defined(Nst_DBG_DISABLE_POOLS) && !defined(Nst_MSVC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

    if (TYPE(obj->type)->p_len >= _Nst_P_LEN_MAX) {
        Nst_free(obj);

        if (obj != ob_t)
            Nst_dec_ref(ob_t);

        return;
    }

#if defined(Nst_DBG_DISABLE_POOLS) && !defined(Nst_MSVC)
#pragma GCC diagnostic pop
#endif

    obj->p_next = TYPE(obj->type)->p_head;
    TYPE(obj->type)->p_head = obj;
    TYPE(obj->type)->p_len++;

    if (obj != ob_t)
        Nst_dec_ref(ob_t);
}

void Nst_obj_traverse(Nst_Obj *obj)
{
    Nst_ObjTrav trav = TYPE(obj->type)->trav;
    if (trav != NULL)
        trav(obj);
}

Nst_Obj *Nst_inc_ref(Nst_Obj *obj)
{
    Nst_assert(obj != NULL);
    obj->ref_count++;
    return obj;
}

Nst_Obj *Nst_ninc_ref(Nst_Obj *obj)
{
    if (obj != NULL)
        return Nst_inc_ref(obj);
    return NULL;
}

void Nst_dec_ref(Nst_Obj *obj)
{
    Nst_assert(obj != NULL);
    obj->ref_count--;

    // The ref_count should nevere be below zero
    Nst_assert(obj->ref_count >= 0);

    if (obj->ref_count <= 0) {
        _Nst_obj_destroy(obj);
        _Nst_obj_free(obj);
    }
}

void Nst_ndec_ref(Nst_Obj *obj)
{
    if (obj != NULL)
        Nst_dec_ref(obj);
}
