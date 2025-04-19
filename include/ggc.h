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
#define _Nst_GEN2_MAX 100
/* The maximum number of objects inside the third generation. */
#define _Nst_GEN3_MAX 10
/* The minimum size of the old generation needed to collect it. */
#define _Nst_OLD_GEN_MIN 100

/**
 * The macro to add support to the GGC to an object structure.
 *
 * @brief It must be placed after `Nst_OBJ_HEAD` and before any other fields.
 */
#define Nst_GGC_HEAD                                                          \
    isize ggc_ref_count;                                                      \
    Nst_Obj *p_prev;                                                          \
    struct _Nst_GGCList *ggc_list

/**
 * @brief Initializes the fields of a `Nst_GGCObj`. Should be called before
 * initializing other fields of the object.
 */
#define Nst_GGC_OBJ_INIT(obj) do {                                            \
    obj->p_prev = NULL;                                                       \
    obj->ggc_list = NULL;                                                     \
    obj->ggc_ref_count = 0;                                                   \
    Nst_SET_FLAG(obj, Nst_FLAG_GGC_IS_SUPPORTED);                             \
    Nst_ggc_track_obj((Nst_GGCObj *)(obj));                                   \
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

struct _Nst_GGCList;

/**
 * The struct representing a garbage collector object.
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
 * @param allow_tracking: whether the tracking of new objects is allowed
 */
NstEXP typedef struct _Nst_GarbageCollector {
    Nst_GGCList gen1;
    Nst_GGCList gen2;
    Nst_GGCList gen3;
    Nst_GGCList old_gen;
    i64 old_gen_pending;
} Nst_GarbageCollector;

/* Collects the object of a generation */
NstEXP void NstC Nst_ggc_collect_gen(Nst_GGCList *gen);
/* Runs a general collection, that collects generations as needed. */
NstEXP void NstC Nst_ggc_collect(void);
/* Adds an object to the tracked objects by the garbage collector. */
NstEXP void NstC Nst_ggc_track_obj(Nst_GGCObj *obj);
/* Sets an `Nst_Obj` as reachable for the garbage collector. */
NstEXP void NstC Nst_ggc_obj_reachable(Nst_Obj *obj);

void _Nst_ggc_quit(void);
void _Nst_ggc_init(void);


/* The flags of a garbage collector object. */
NstEXP typedef enum _Nst_GGCFlags {
    Nst_FLAG_GGC_REACHABLE    = Nst_FLAG(32),
    Nst_FLAG_GGC_PRESERVE_MEM = Nst_FLAG(31),
    Nst_FLAG_GGC_IS_SUPPORTED = Nst_FLAG(30)
} Nst_GGCFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GGC_H
