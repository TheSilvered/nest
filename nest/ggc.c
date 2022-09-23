#include "ggc.h"
#include "interpreter.h"

#define REACHABLE(ob) ((ob)->flags & NST_FLAG_GCC_REACHABLE)
#define UNREACHABLE(ob) ((ob)->flags & NST_FLAG_GCC_UNREACHABLE)

inline void move_obj(Nst_GGCObject *obj, Nst_GGCList *from, Nst_GGCList *to)
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
}

void collect_gen(Nst_GGCList *gen)
{
    // Unreachable values
    Nst_GGCList uv = {
        NULL,
        NULL,
        0
    };

    // Clear flags
    for ( Nst_GGCObject *ob = gen->head;
          ob->ggc_next != NULL;
          ob = ob->ggc_next )
    {
        NST_UNSET_FLAG(ob, NST_FLAG_GGC_REACHABLE
                           | NST_FLAG_GGC_UNREACHABLE 
                           | NST_FLAG_GGC_OBJ_DELETED );
    }

    // All objects in the variable tables are reachable
    nst_traverse_map((*nst_state.vt)->vars);
    for ( size_t i = 0, n = nst_state.f_stack->current_size; i < n; i++ )
    {
        Nst_VarTable *vt = nst_state.f_stack->stack[i].vt;
        if ( vt != NULL )
            nst_traverse_map(vt->vars);
    }

    // previous unreachable_values size 
    register size_t prev_uv_size = 0;

    // last traversed value in `gen`
    register Nst_GGCObject *traversed_end = NULL;

    register Nst_GGCObject *ob = NULL;
    register Nst_GGCObject *new_ob = NULL;

    do
    {
        prev_uv_size = uv.size;
        // Move unreachable objects to `unreachable_values`
        for ( ob = traversed_end ? traversed_end : gen->head; ob != NULL; )
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

        traversed_end = gen->tail;

        if ( prev_uv_size == uv.size )
            break;

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
