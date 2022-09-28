#include "ggc.h"
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
            from->head = obj->ggc_next;
        else
            obj->ggc_prev->ggc_next = obj->ggc_next;

        // it's the last object in the list
        if ( obj->ggc_next == NULL )
            from->tail = obj->ggc_prev;
        else
            obj->ggc_next->ggc_prev = obj->ggc_prev;

        from->size--;
    }

    if ( to->size == 0 )
        to->head = obj;
    obj->ggc_prev = to->tail;
    obj->ggc_next = NULL;
    to->tail = obj;
    to->size++;

    obj->ggc_list = to;
}

static void move_list(Nst_GGCList *from, Nst_GGCList *to)
{
    if ( from->size == 0 )
        return;

    for ( Nst_GGCObj *cursor = from->head;
          cursor != NULL;
          cursor = cursor->ggc_next )
        cursor->ggc_list = to;

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
    to->tail = from->tail;
    to->size += from->size;

    from->head = NULL;
    from->tail = NULL;
    from->size = 0;
}

void nst_collect_gen(Nst_GGCList *gen)
{
    // Unreachable values
    Nst_GGCList uv = {
        NULL,
        NULL,
        0
    };

    for ( Nst_GGCObj *ob = gen->head;
            ob->ggc_next != NULL;
            ob = ob->ggc_next )
    {
        NST_UNSET_FLAG(ob, NST_FLAG_GGC_REACHABLE
                         | NST_FLAG_GGC_UNREACHABLE 
                         | NST_FLAG_GGC_OBJ_DELETED );
    }

    // All objects in the variable tables are reachable
    NST_UNSET_FLAG((*nst_state.vt)->vars, NST_FLAG_GGC_UNREACHABLE);
    NST_SET_FLAG((*nst_state.vt)->vars, NST_FLAG_GGC_REACHABLE);
    nst_traverse_map((*nst_state.vt)->vars);
    for ( size_t i = 0, n = nst_state.f_stack->current_size; i < n; i++ )
    {
        Nst_VarTable *vt = nst_state.f_stack->stack[i].vt;
        if ( vt != NULL )
        {
            nst_traverse_map(vt->vars);
            NST_UNSET_FLAG(vt->vars, NST_FLAG_GGC_UNREACHABLE);
            NST_SET_FLAG(vt->vars, NST_FLAG_GGC_REACHABLE);
        }
    }

    NST_UNSET_FLAG(nst_state.argv, NST_FLAG_GGC_UNREACHABLE);
    NST_SET_FLAG(nst_state.argv, NST_FLAG_GGC_REACHABLE);
    nst_traverse_seq(nst_state.argv);

    register Nst_GGCObj *ob = NULL;
    register Nst_GGCObj *new_ob = NULL;

    // Move unreachable objects to `unreachable_values`
    for ( ob = gen->head; ob != NULL; )
    {
        if ( NST_HAS_FLAG(ob, NST_FLAG_GGC_REACHABLE) )
        {
            ob = ob->ggc_next;
            continue;
        }
        NST_SET_FLAG(ob, NST_FLAG_GGC_UNREACHABLE);
        new_ob = ob->ggc_next;
        move_obj(ob, gen, &uv);
        ob = new_ob;
    }

    // previous unreachable_values size
    register size_t prev_uv_size = 0;

    // last traversed value in `gen`
    register Nst_GGCObj *traversed_end = gen->head;

    do
    {
        prev_uv_size = uv.size;
        // Traverse reachable objects
        for ( ob = traversed_end; ob != NULL; )
        {
            ob->traverse_func((Nst_Obj *)ob);
            ob = ob->ggc_next;
        }

        traversed_end = gen->tail;

        if ( uv.size == 0 )
            return;
        if ( prev_uv_size == uv.size )
            break;

        // Move objects reached back into the reachable objects to be traversed
        for ( ob = uv.head; ob != NULL; )
        {
            if ( NST_HAS_FLAG(ob, NST_FLAG_GGC_REACHABLE) )
                NST_UNSET_FLAG(ob, NST_FLAG_GGC_UNREACHABLE);
            else
            {
                ob = ob->ggc_next;
                continue;
            }

            new_ob = ob->ggc_next;
            move_obj(ob, &uv, gen);
            ob = new_ob;
        }
    }
    while ( true );

    // Delete the objects
    for ( ob = uv.head; ob != NULL; ob = ob->ggc_next )
        // does not actually destroy the object, only calls the destructor
        // this happens because of the NST_FLAG_GGC_UNREACHABLE flag
        nst_destroy_obj(ob);

    // Free the memory
    for ( ob = uv.head; ob != NULL; )
    {
        new_ob = ob->ggc_next;
        free(ob);
        ob = new_ob;
    }
}

void nst_collect()
{
    register Nst_GarbageCollector *ggc = nst_state.ggc;
    register size_t old_gen_size = ggc->old_gen.size;
    // if the number of objects never checked in the old generation
    // is more than 25% and there are at least 10 objects
    if ( old_gen_size > NST_OLD_GEN_MIN &&
         old_gen_size - ggc->old_gen_pending > old_gen_size >> 2 )
    {
        printf("Collected old_gen\n");
        nst_collect_gen(&ggc->old_gen);
        ggc->old_gen_pending = 0;
    }

    bool has_collected_gen1 = false;
    bool has_collected_gen2 = false;
    bool has_collected_gen3 = false;

    // Collect the generations if they are over their maximum value
    if ( ggc->gen1.size > NST_GEN1_MAX )
    {
        printf("Collected gen1\n");
        nst_collect_gen(&ggc->gen1);
        has_collected_gen1 = true;
    }

    if ( ggc->gen2.size > NST_GEN2_MAX ||
         (has_collected_gen1 &&
          ggc->gen1.size + ggc->gen2.size > NST_GEN2_MAX) )
    {
        printf("Collected gen2\n");
        nst_collect_gen(&ggc->gen2);
        has_collected_gen2 = true;
    }

    if ( ggc->gen3.size > NST_GEN3_MAX ||
         (has_collected_gen2 && 
          ggc->gen2.size + ggc->gen3.size > NST_GEN3_MAX) )
    {
        printf("Collected gen3\n");
        nst_collect_gen(&ggc->gen2);
        has_collected_gen3 = true;
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
            move_list(&ggc->gen2, &ggc->gen3);
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
                move_list(&ggc->gen1, &ggc->gen3);
        }
        else
            move_list(&ggc->gen1, &ggc->gen2);
    }
}

void nst_add_tracked_object(Nst_GGCObj *obj)
{
    //printf("Added object %p of type %s\n", obj, TYPE_NAME(obj));
    register Nst_GarbageCollector *ggc = nst_state.ggc;
    printf("%zi, %zi, %zi, %zi\n", ggc->gen1.size, ggc->gen2.size, ggc->gen3.size, ggc->old_gen.size);

    if ( ggc->gen1.size == 0 )
    {
        ggc->gen1.head = obj;
        ggc->gen1.tail = obj;
        ggc->gen1.size = 1;
    }
    else
    {
        obj->ggc_prev = ggc->gen1.tail;
        ggc->gen1.tail->ggc_next = obj;
        ggc->gen1.tail = obj;
        ggc->gen1.size += 1;

        if ( ggc->gen1.size > NST_GEN1_MAX )
            nst_collect();
    }
}
