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
 * Initializes the fields of a `Nst_GGCObj`. Should be called after having
 * initialized all the other fields of the object.
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

/* The struct representing a garbage collector object. */
NstEXP typedef struct _Nst_GGCObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
} Nst_GGCObj;

/**
 * The structure representing a generation of the garbage collector.
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
