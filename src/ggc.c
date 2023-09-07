#include <assert.h>
#include "mem.h"
#include "interpreter.h"
#include "type.h"

#define REACHABLE(ob) ((ob)->flags & Nst_FLAG_GCC_REACHABLE)
#define UNREACHABLE(ob) ((ob)->flags & Nst_FLAG_GCC_UNREACHABLE)

static inline void move_obj(Nst_GGCObj *obj, Nst_GGCList *from, Nst_GGCList *to)
{
    if (from->len == 1) {
        from->len = 0;
        from->head = NULL;
        from->tail = NULL;
    } else {
        // if it's the first object in the list
        if (obj->p_prev == NULL)
            from->head = GGC_OBJ(obj->p_next);
        else
            obj->p_prev->p_next = obj->p_next;

        // if it's the last object in the list
        if (obj->p_next == NULL)
            from->tail = GGC_OBJ(obj->p_prev);
        else
            GGC_OBJ(obj->p_next)->p_prev = obj->p_prev;

        from->len--;
    }

    if (to->len == 0)
        to->head = obj;
    else
        to->tail->p_next = OBJ(obj);
    obj->p_prev = OBJ(to->tail);
    obj->p_next = NULL;
    to->tail = obj;
    to->len++;

    obj->ggc_list = to;
}

static void move_list(Nst_GGCList *from, Nst_GGCList *to)
{
    if (from->len == 0)
        return;

    for (Nst_GGCObj *cursor = from->head;
        cursor != NULL;
        cursor = GGC_OBJ(cursor->p_next))
    {
        cursor->ggc_list = to;
    }

    if (to->head == NULL) {
        to->head = from->head;
        to->tail = from->tail;
        to->len = from->len;
        from->head = NULL;
        from->tail = NULL;
        from->len = 0;
        return;
    }

    from->head->p_prev = OBJ(to->tail);
    to->tail->p_next = OBJ(from->head);
    to->tail = from->tail;
    to->len += from->len;

    from->head = NULL;
    from->tail = NULL;
    from->len = 0;
}

static inline void remove_objs_list(Nst_GGCList *gen)
{
    for (Nst_GGCObj *ob = gen->head; ob != NULL; ob = GGC_OBJ(ob->p_next))
        ob->ggc_list = NULL;
}

static inline void call_objs_destructor(Nst_GGCList *gen)
{
    for (Nst_GGCObj *ob = gen->head; ob != NULL; ob = GGC_OBJ(ob->p_next)) {
        assert(ob->ggc_list == NULL);
        _Nst_obj_destroy(OBJ(ob));
    }
}

static inline void free_obj_memory(Nst_GGCList *gen)
{
    Nst_GGCObj *new_ob = NULL;
    for (Nst_GGCObj *ob = gen->head; ob != NULL;) {
        new_ob = GGC_OBJ(ob->p_next);
        Nst_SET_FLAG(ob, Nst_FLAG_GGC_DELETE);
        _Nst_obj_free(OBJ(ob));
        ob = new_ob;
    }
}

static inline void destroy_objects(Nst_GGCList *gen)
{
    remove_objs_list(gen);
    call_objs_destructor(gen);
    free_obj_memory(gen);
}

void Nst_ggc_collect_gen(Nst_GGCList *gen)
{
    // Unreachable values
    Nst_GGCList uv = { NULL, NULL, 0 };

    Nst_GGCObj *ob = NULL;
    Nst_GGCObj *new_ob = NULL;
    for (ob = gen->head; ob != NULL; ob = GGC_OBJ(ob->p_next))
        ob->ggc_ref_count = ob->ref_count;

    for (ob = gen->head; ob != NULL; ob = GGC_OBJ(ob->p_next))
        CONT_TYPE(ob->type)->trav(OBJ(ob));

    for (ob = gen->head; ob != NULL;) {
        new_ob = GGC_OBJ(ob->p_next);
        if (ob->ggc_ref_count == 0) {
            Nst_DEL_FLAG(ob, Nst_FLAG_GGC_REACHABLE);
            move_obj(ob, gen, &uv);
        }
        ob = new_ob;
    }

    if (gen->len == 0) {
        destroy_objects(&uv);
        return;
    }

    for (ob = gen->head; ob != NULL; ob = GGC_OBJ(ob->p_next))
        CONT_TYPE(ob->type)->trav(OBJ(ob));


    while (true) {
        Nst_GGCObj *last_traversed = gen->tail;
        for (ob = uv.head; ob != NULL;) {
            new_ob = GGC_OBJ(ob->p_next);
            if (Nst_HAS_FLAG(ob, Nst_FLAG_GGC_REACHABLE))
                move_obj(ob, &uv, gen);
            ob = new_ob;
        }

        // if no new objects were appended back to the reachable ones
        if (last_traversed->p_next == NULL)
            break;

        for (ob = GGC_OBJ(last_traversed->p_next);
             ob != NULL;
             ob = GGC_OBJ(ob->p_next))
        {
            CONT_TYPE(ob->type)->trav(OBJ(ob));
        }
    }

    destroy_objects(&uv);
}

void Nst_ggc_delete_objs(void)
{
    remove_objs_list(&Nst_state.ggc.gen1);
    remove_objs_list(&Nst_state.ggc.gen2);
    remove_objs_list(&Nst_state.ggc.gen3);
    remove_objs_list(&Nst_state.ggc.old_gen);

    call_objs_destructor(&Nst_state.ggc.gen1);
    call_objs_destructor(&Nst_state.ggc.gen2);
    call_objs_destructor(&Nst_state.ggc.gen3);
    call_objs_destructor(&Nst_state.ggc.old_gen);

    free_obj_memory(&Nst_state.ggc.gen1);
    free_obj_memory(&Nst_state.ggc.gen2);
    free_obj_memory(&Nst_state.ggc.gen3);
    free_obj_memory(&Nst_state.ggc.old_gen);
}

void Nst_ggc_init(void)
{
    Nst_GGCList gen1 = { NULL, NULL, 0 };
    Nst_GGCList gen2 = { NULL, NULL, 0 };
    Nst_GGCList gen3 = { NULL, NULL, 0 };
    Nst_GGCList old_gen = { NULL, NULL, 0 };
    Nst_state.ggc.gen1 = gen1;
    Nst_state.ggc.gen2 = gen2;
    Nst_state.ggc.gen3 = gen3;
    Nst_state.ggc.old_gen = old_gen;
    Nst_state.ggc.old_gen_pending = 0;
}

void _Nst_ggc_obj_reachable(Nst_Obj *obj)
{
    if (Nst_HAS_FLAG(obj, Nst_FLAG_GGC_IS_SUPPORTED)) {
        Nst_SET_FLAG(obj, Nst_FLAG_GGC_REACHABLE);
        GGC_OBJ(obj)->ggc_ref_count--;
    }
}

void Nst_ggc_collect(void)
{
    Nst_GarbageCollector *ggc = &Nst_state.ggc;
    usize old_gen_size = ggc->old_gen.len;
    // if the number of objects never checked in the old generation
    // is more than 25% and there are at least 10 objects
    if (old_gen_size > _Nst_OLD_GEN_MIN
        && ggc->old_gen_pending >= (i64)old_gen_size >> 2)
    {
        Nst_ggc_collect_gen(&ggc->old_gen);
        ggc->old_gen_pending = 0;
    }

    bool has_collected_gen1 = false;
    bool has_collected_gen2 = false;

    // Collect the generations if they are over their maximum value
    if (ggc->gen1.len > _Nst_GEN1_MAX) {
        Nst_ggc_collect_gen(&ggc->gen1);
        has_collected_gen1 = true;
    }

    if (ggc->gen2.len > _Nst_GEN2_MAX
        || (has_collected_gen1
            && ggc->gen1.len + ggc->gen2.len > _Nst_GEN2_MAX))
    {
        Nst_ggc_collect_gen(&ggc->gen2);
        has_collected_gen2 = true;
    }

    if (ggc->gen3.len > _Nst_GEN3_MAX
        || (has_collected_gen2
            && ggc->gen2.len + ggc->gen3.len > _Nst_GEN3_MAX))
    {
        Nst_ggc_collect_gen(&ggc->gen3);
        ggc->old_gen_pending += ggc->gen3.len;
        move_list(&ggc->gen3, &ggc->old_gen);
    }

    if (has_collected_gen2) {
        if (ggc->gen2.len + ggc->gen3.len > _Nst_GEN3_MAX) {
            ggc->old_gen_pending += ggc->gen2.len;
            move_list(&ggc->gen2, &ggc->old_gen);
        } else
            move_list(&ggc->gen2, &ggc->gen3);
    }

    if (has_collected_gen1) {
        if (ggc->gen1.len + ggc->gen2.len > _Nst_GEN2_MAX) {
            if (ggc->gen1.len + ggc->gen3.len > _Nst_GEN3_MAX) {
                ggc->old_gen_pending += ggc->gen1.len;
                move_list(&ggc->gen1, &ggc->old_gen);
            } else
                move_list(&ggc->gen1, &ggc->gen3);
        } else
            move_list(&ggc->gen1, &ggc->gen2);
    }
}

void Nst_ggc_track_obj(Nst_GGCObj *obj)
{
    assert(Nst_HAS_FLAG(obj->type, Nst_FLAG_TYPE_IS_CONTAINER));

    if (Nst_state.ggc.gen1.len == 0)
        Nst_state.ggc.gen1.head = obj;
    else {
        obj->p_prev = OBJ(Nst_state.ggc.gen1.tail);
        Nst_state.ggc.gen1.tail->p_next = OBJ(obj);
    }

    Nst_state.ggc.gen1.tail = obj;
    obj->ggc_list = &Nst_state.ggc.gen1;
    Nst_state.ggc.gen1.len++;
}
