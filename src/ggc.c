#include <stdlib.h>
#include "interpreter.h"

#define REACHABLE(ob) ((ob)->flags & NST_FLAG_GCC_REACHABLE)
#define UNREACHABLE(ob) ((ob)->flags & NST_FLAG_GCC_UNREACHABLE)

static inline void move_obj(Nst_GGCObj *obj, Nst_GGCList *from, Nst_GGCList *to)
{
    if ( from->size == 1 )
    {
        from->size = 0;
        from->head = NULL;
        from->tail = NULL;
    }
    else
    {
        // it's the first object in the list
        if ( obj->ggc_prev == NULL )
        {
            from->head = obj->ggc_next;
        }
        else
        {
            obj->ggc_prev->ggc_next = obj->ggc_next;
        }

        // it's the last object in the list
        if ( obj->ggc_next == NULL )
        {
            from->tail = obj->ggc_prev;
        }
        else
        {
            obj->ggc_next->ggc_prev = obj->ggc_prev;
        }

        from->size--;
    }

    if ( to->size == 0 )
    {
        to->head = obj;
    }
    else
    {
        to->tail->ggc_next = obj;
    }
    obj->ggc_prev = to->tail;
    obj->ggc_next = NULL;
    to->tail = obj;
    to->size++;

    obj->ggc_list = to;
}

static void move_list(Nst_GGCList *from, Nst_GGCList *to)
{
    if ( from->size == 0 )
    {
        return;
    }

    for ( Nst_GGCObj *cursor = from->head;
          cursor != NULL;
          cursor = cursor->ggc_next )
    {
        cursor->ggc_list = to;
    }

    if ( to->head == NULL )
    {
        to->head = from->head;
        to->tail = from->tail;
        to->size = from->size;
        from->head = NULL;
        from->tail = NULL;
        from->size = 0;
        return;
    }

    from->head->ggc_prev = to->tail;
    to->tail->ggc_next = from->head;
    to->tail = from->tail;
    to->size += from->size;

    from->head = NULL;
    from->tail = NULL;
    from->size = 0;
}

static inline void call_objs_destructor(Nst_GGCList *gen)
{
    for ( Nst_GGCObj *ob = gen->head; ob != NULL; ob = ob->ggc_next )
    {
        nst_obj_destroy(ob);
    }
}

static inline void free_obj_memory(Nst_GGCList *gen)
{
    Nst_GGCObj *new_ob = NULL;
    for ( Nst_GGCObj *ob = gen->head; ob != NULL; )
    {
        new_ob = ob->ggc_next;
        free(ob);
        ob = new_ob;
    }
}

static inline void set_unreachable(Nst_GGCList *gen)
{
    for ( Nst_GGCObj *ob = gen->head; ob != NULL; ob = ob->ggc_next )
    {
        NST_FLAG_SET(ob, NST_FLAG_GGC_UNREACHABLE);
    }
}

static inline void traverse_gen(Nst_GGCList *gen)
{
    for ( Nst_GGCObj *ob = gen->head; ob != NULL; ob = ob->ggc_next )
    {
        ob->traverse_func(OBJ(ob));
    }
}

void nst_ggc_collect_gen(Nst_GGCList *gen,
                         Nst_GGCList *other_gen1,
                         Nst_GGCList *other_gen2,
                         Nst_GGCList *other_gen3)
{
    // Unreachable values
    Nst_GGCList uv = {
        NULL,
        NULL,
        0
    };

    Nst_GGCObj *ob = NULL;
    Nst_GGCObj *new_ob = NULL;

    for ( ob = gen->head; ob != NULL; ob = ob->ggc_next )
    {
        NST_FLAG_DEL(ob, NST_FLAG_GGC_REACHABLE
                         | NST_FLAG_GGC_UNREACHABLE
                         | NST_FLAG_GGC_DELETED );
    }

    // All objects in the variable tables are reachable
    NST_FLAG_SET(nst_state.vt->vars, NST_FLAG_GGC_REACHABLE);
    _nst_map_traverse(nst_state.vt->vars);
    for ( usize i = 0, n = nst_state.f_stack->current_size; i < n; i++ )
    {
        Nst_VarTable *vt = nst_state.f_stack->stack[i].vt;
        if ( vt != NULL )
        {
            _nst_map_traverse(vt->vars);
            NST_FLAG_DEL(vt->vars, NST_FLAG_GGC_UNREACHABLE);
            NST_FLAG_SET(vt->vars, NST_FLAG_GGC_REACHABLE);
        }
    }

    // As well as the objects inside other generations
    traverse_gen(other_gen1);
    traverse_gen(other_gen2);
    traverse_gen(other_gen3);

    // The argv array is reachable too
    NST_FLAG_DEL(nst_state.argv, NST_FLAG_GGC_UNREACHABLE);
    NST_FLAG_SET(nst_state.argv, NST_FLAG_GGC_REACHABLE);
    _nst_seq_traverse(nst_state.argv);

    // And the same goes for the lib handles
    NST_FLAG_DEL(nst_state.lib_handles, NST_FLAG_GGC_UNREACHABLE);
    NST_FLAG_SET(nst_state.lib_handles, NST_FLAG_GGC_REACHABLE);
    _nst_map_traverse(nst_state.lib_handles);

    // Move unreachable objects to `unreachable_values`
    for ( ob = gen->head; ob != NULL; )
    {
        if ( NST_FLAG_HAS(ob, NST_FLAG_GGC_REACHABLE) )
        {
            ob = ob->ggc_next;
            continue;
        }
        NST_FLAG_SET(ob, NST_FLAG_GGC_UNREACHABLE);
        new_ob = ob->ggc_next;
        move_obj(ob, gen, &uv);
        ob = new_ob;
    }

    // previous unreachable_values size
    usize prev_uv_size = 0;

    // last traversed value in `gen`
    Nst_GGCObj *traversed_end = gen->head;

    do
    {
        prev_uv_size = uv.size;
        // Traverse reachable objects
        for ( ob = traversed_end; ob != NULL; )
        {
            ob->traverse_func(OBJ(ob));
            ob = ob->ggc_next;
        }

        traversed_end = gen->tail;

        if ( uv.size == 0 )
        {
            return;
        }

        // Move objects reached back into the reachable objects to be traversed
        for ( ob = uv.head; ob != NULL; )
        {
            if ( NST_FLAG_HAS(ob, NST_FLAG_GGC_REACHABLE) )
            {
                NST_FLAG_DEL(ob, NST_FLAG_GGC_UNREACHABLE);
            }
            else
            {
                ob = ob->ggc_next;
                continue;
            }

            new_ob = ob->ggc_next;
            move_obj(ob, &uv, gen);
            ob = new_ob;
        }

        if ( prev_uv_size == uv.size )
        {
            break;
        }
    }
    while ( true );

    // If an object is in uv and references
    // another object in uv too, if the memory
    // if freed, the dec_ref function will cause
    // a segfault, here i call only the destructor
    // to not free the memory right away
    call_objs_destructor(&uv);
    free_obj_memory(&uv);
}

void nst_ggc_delete_objs(Nst_GarbageCollector *ggc)
{
    set_unreachable(&ggc->gen1);
    set_unreachable(&ggc->gen2);
    set_unreachable(&ggc->gen3);
    set_unreachable(&ggc->old_gen);

    call_objs_destructor(&ggc->gen1);
    call_objs_destructor(&ggc->gen2);
    call_objs_destructor(&ggc->gen3);
    call_objs_destructor(&ggc->old_gen);

    free_obj_memory(&ggc->gen1);
    free_obj_memory(&ggc->gen2);
    free_obj_memory(&ggc->gen3);
    free_obj_memory(&ggc->old_gen);
}

void nst_ggc_collect()
{
    Nst_GarbageCollector *ggc = &nst_state.ggc;
    usize old_gen_size = ggc->old_gen.size;
    // if the number of objects never checked in the old generation
    // is more than 25% and there are at least 10 objects
    if ( old_gen_size > NST_OLD_GEN_MIN &&
         ggc->old_gen_pending >= (Nst_Int)old_gen_size >> 2 )
    {
        nst_ggc_collect_gen(&ggc->old_gen, &ggc->gen1, &ggc->gen2, &ggc->gen3);
        ggc->old_gen_pending = 0;
    }

    bool has_collected_gen1 = false;
    bool has_collected_gen2 = false;

    // Collect the generations if they are over their maximum value
    if ( ggc->gen1.size > NST_GEN1_MAX )
    {
        nst_ggc_collect_gen(&ggc->gen1, &ggc->gen2, &ggc->gen3, &ggc->old_gen);
        has_collected_gen1 = true;
    }

    if ( ggc->gen2.size > NST_GEN2_MAX ||
         (has_collected_gen1 &&
          ggc->gen1.size + ggc->gen2.size > NST_GEN2_MAX) )
    {
        nst_ggc_collect_gen(&ggc->gen2, &ggc->gen1, &ggc->gen3, &ggc->old_gen);
        has_collected_gen2 = true;
    }

    if ( ggc->gen3.size > NST_GEN3_MAX ||
         (has_collected_gen2 &&
          ggc->gen2.size + ggc->gen3.size > NST_GEN3_MAX) )
    {
        nst_ggc_collect_gen(&ggc->gen3, &ggc->gen1, &ggc->gen2, &ggc->old_gen);
        ggc->old_gen_pending += ggc->gen3.size;
        move_list(&ggc->gen3, &ggc->old_gen);
    }

    if ( has_collected_gen2 )
    {
        if ( ggc->gen2.size + ggc->gen3.size > NST_GEN3_MAX )
        {
            ggc->old_gen_pending += ggc->gen2.size;
            move_list(&ggc->gen2, &ggc->old_gen);
        }
        else
        {
            move_list(&ggc->gen2, &ggc->gen3);
        }
    }

    if ( has_collected_gen1 )
    {
        if ( ggc->gen1.size + ggc->gen2.size > NST_GEN2_MAX )
        {
            if ( ggc->gen1.size + ggc->gen3.size > NST_GEN3_MAX )
            {
                ggc->old_gen_pending += ggc->gen1.size;
                move_list(&ggc->gen1, &ggc->old_gen);
            }
            else
            {
                move_list(&ggc->gen1, &ggc->gen3);
            }
        }
        else
        {
            move_list(&ggc->gen1, &ggc->gen2);
        }
    }
}

void nst_ggc_track_obj(Nst_GGCObj *obj)
{
    Nst_GarbageCollector *ggc = &nst_state.ggc;

    if ( NST_OBJ_IS_TRACKED(obj) )
    {
        return;
    }

    if ( ggc->gen1.size == 0 )
    {
        ggc->gen1.head = obj;
        ggc->gen1.tail = obj;
        ggc->gen1.size = 1;
        obj->ggc_list = &ggc->gen1;
    }
    else
    {
        obj->ggc_prev = ggc->gen1.tail;
        obj->ggc_list = &ggc->gen1;
        ggc->gen1.tail->ggc_next = obj;
        ggc->gen1.tail = obj;
        ggc->gen1.size++;

        obj->track_func(OBJ(obj));

        if ( ggc->gen1.size > NST_GEN1_MAX )
        {
            nst_ggc_collect();
        }
    }
}
