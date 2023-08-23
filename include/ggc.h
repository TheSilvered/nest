/**
 * @file ggc.h
 *
 * @brief Generational Garbage Collector (GGC)
 *
 * @author TheSilvered
 */

#ifndef GGC_H
#define GGC_H

#include "simple_types.h"

/* The maximum number of objects inside the first generation. */
#define _Nst_GEN1_MAX 700
/* The maximum number of objects inside the second generation. */
#define _Nst_GEN2_MAX 10
/* The maximum number of objects inside the third generation. */
#define _Nst_GEN3_MAX 10
/* The minimum size of the old generation needed to collect it. */
#define _Nst_OLD_GEN_MIN 100

/* Casts obj to `Nst_GGCObj *`. */
#define GGC_OBJ(obj) ((Nst_GGCObj *)(obj))

/* Checks whether a `Nst_GGCObj` is tracked by the garbage collector. */
#define Nst_OBJ_IS_TRACKED(obj) (GGC_OBJ(obj)->ggc_list != NULL)

/**
 * The macro to add support to the GGC to an object structure.
 *
 * @brief It must be placed after `Nst_OBJ_HEAD` and before any other fields.
 */
#define Nst_GGC_HEAD                                                          \
    struct _Nst_GGCObj *ggc_next;                                             \
    struct _Nst_GGCObj *ggc_prev;                                             \
    struct _Nst_GGCList *ggc_list;                                            \
    void (* traverse_func)(Nst_Obj *);                                        \
    void (* track_func)(Nst_Obj *)

/* Initializes the fields of a `Nst_GGCObj`. */
#define Nst_GGC_OBJ_INIT(obj, trav_func, track_function) do {                 \
    obj->ggc_prev = NULL;                                                     \
    obj->ggc_next = NULL;                                                     \
    obj->ggc_list = NULL;                                                     \
    obj->traverse_func = (void (*)(Nst_Obj *))(trav_func);                    \
    obj->track_func = (void (*)(Nst_Obj *))(track_function);                  \
    Nst_FLAG_SET(obj, Nst_FLAG_GGC_IS_SUPPORTED);                             \
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

struct _Nst_GGCList;

/**
 * The struct representing a garbage collector object.
 *
 * @param ggc_next: the next object in the generation it belongs to
 * @param ggc_prev: the previous object in the generation it belongs to
 * @param ggc_list: the genreration it belongs to
 * @param traverse_func: the function that sets as reachable all the objects
 * contained in the object
 * @param track_func: the function that tracks all the trackable objects that
 * the object contains when it itself is tracked
 */
NstEXP typedef struct _Nst_GGCObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
} Nst_GGCObj;

/**
 * The structure representing a garbage collector generation.
 *
 * @param head: the first object in the generation
 * @param tail: the last object in the generation
 * @param len: the total number of objects in the generation
 */
NstEXP typedef struct _Nst_GGCList {
    Nst_GGCObj *head;
    Nst_GGCObj *tail;
    usize len;
} Nst_GGCList;

/**
 * The structure representing the garbage collector.
 *
 * @param gen1: the first generation
 * @param gen2: the second generation
 * @param gen3: the third generation
 * @param old_gen: the old generation
 * @param old_gen_pending: the number of objects in the old generation that
 * have been added since its last collection
 */
NstEXP typedef struct _Nst_GarbageCollector {
    Nst_GGCList gen1;
    Nst_GGCList gen2;
    Nst_GGCList gen3;
    Nst_GGCList old_gen;
    i64 old_gen_pending;
} Nst_GarbageCollector;

/* Collects the object of a generation */
NstEXP void NstC Nst_ggc_collect_gen(Nst_GGCList *gen, Nst_GGCList *other_gen1,
                                     Nst_GGCList *other_gen2,
                                     Nst_GGCList *other_gen3);
/* Runs a general collection, that collects generations as needed. */
NstEXP void NstC Nst_ggc_collect(void);
/* Adds an object to the tracked objects by the garbage collector. */
NstEXP void NstC Nst_ggc_track_obj(Nst_GGCObj *obj);
/* Deletes all objects still present in the garbage collector. */
NstEXP void NstC Nst_ggc_delete_objs(void);
/* Initializes the garbage collector of `Nst_state`. */
NstEXP void NstC Nst_ggc_init(void);

/* The flags of a garbage collector object. */
NstEXP typedef enum _Nst_GGCFlags {
    Nst_FLAG_GGC_REACHABLE    = 0x80000000,
    Nst_FLAG_GGC_UNREACHABLE  = 0x40000000,
    Nst_FLAG_GGC_DELETED      = 0x20000000,
    Nst_FLAG_GGC_IS_SUPPORTED = 0x10000000
} Nst_GGCFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GGC_H
