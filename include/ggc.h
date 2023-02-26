/* Generational Garbage Collector (GGC) */

#ifndef GGC_H
#define GGC_H

#include "simple_types.h"

#define NST_GEN1_MAX 700
#define NST_GEN2_MAX 10
#define NST_GEN3_MAX 10
#define NST_OLD_GEN_MIN 100

#define GGC_OBJ(obj) ((Nst_GGCObj *)(obj))

#define NST_OBJ_IS_TRACKED(obj) (GGC_OBJ(obj)->ggc_list != NULL)

// To add after NST_OBJ_HEAD and before any arguments, adds support for the GGC
// to the object
#define NST_GGC_HEAD \
    struct _Nst_GGCObj *ggc_next; \
    struct _Nst_GGCObj *ggc_prev; \
    struct _Nst_GGCList *ggc_list; \
    void (* traverse_func)(Nst_Obj *); \
    void (* track_func)(Nst_Obj *)

// To use when creating a new object that supports the GGC
#define NST_GGC_OBJ_INIT(obj, trav_func, track_function) \
    do { \
        obj->ggc_prev = NULL; \
        obj->ggc_next = NULL; \
        obj->ggc_list = NULL; \
        obj->traverse_func = (void (*)(Nst_Obj *))(trav_func); \
        obj->track_func = (void (*)(Nst_Obj *))(track_function); \
        NST_FLAG_SET(obj, NST_FLAG_GGC_IS_SUPPORTED); \
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

struct _Nst_GGCList;

EXPORT typedef struct _Nst_GGCObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
}
Nst_GGCObj;

EXPORT typedef struct _Nst_GGCList
{
    Nst_GGCObj *head;
    Nst_GGCObj *tail;
    usize size;
}
Nst_GGCList;

EXPORT typedef struct _Nst_GarbageCollector
{
    Nst_GGCList gen1;
    Nst_GGCList gen2;
    Nst_GGCList gen3;
    Nst_GGCList old_gen;
    Nst_Int old_gen_pending;
}
Nst_GarbageCollector;

// Collects the object of a generation
EXPORT
void nst_ggc_collect_gen(Nst_GGCList *gen,
                         Nst_GGCList *other_gen1,
                         Nst_GGCList *other_gen2,
                         Nst_GGCList *other_gen3);
// Runs a collection, does not guaratee to collect all generations
EXPORT void nst_ggc_collect(void);
// Adds an object to the tracked objects by the garbage collector
EXPORT void nst_ggc_track_obj(Nst_GGCObj *obj);
// Deletes the objects still present in the GGC at program end
// This function should never be called
EXPORT void nst_ggc_delete_objs(Nst_GarbageCollector *ggc);

EXPORT typedef enum _Nst_GGCFlag
{
    NST_FLAG_GGC_REACHABLE    = 0b10000000,
    NST_FLAG_GGC_UNREACHABLE  = 0b01000000,
    NST_FLAG_GGC_DELETED      = 0b00100000,
    NST_FLAG_GGC_IS_SUPPORTED = 0b00010000
}
Nst_GGCFlag;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GGC_H
