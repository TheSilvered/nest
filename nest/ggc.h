#ifndef GGC_H
#define GGC_H

#include "obj.h"
#include "simple_types.h"

#define NST_GEN1_MAX 700
#define NST_GEN2_MAX 10
#define NST_GEN3_MAX 10
#define NST_OLD_GEN_MIN 100

#define NST_GGC_SUPPORT \
    struct Nst_GGCObj *ggc_next; \
    struct Nst_GGCObj *ggc_prev; \
    struct Nst_GGCList *ggc_list; \
    void (* traverse_func)(Nst_Obj *)

#define NST_GGC_SUPPORT_INIT(obj, trav_func) \
    do { \
        obj->ggc_prev = NULL; \
        obj->ggc_next = NULL; \
        obj->ggc_list = NULL; \
        obj->traverse_func = (void (*)(Nst_Obj *))(trav_func); \
        NST_SET_FLAG(obj, NST_FLAG_GGC_IS_SUPPORTED); \
    } while (0)

struct Nst_GGCList;

typedef struct Nst_GGCObj
{
    NST_OBJ_HEAD;
    NST_GGC_SUPPORT;
}
Nst_GGCObj;

typedef struct Nst_GGCList
{
    Nst_GGCObj *head;
    Nst_GGCObj *tail;
    size_t size;
}
Nst_GGCList;

typedef struct
{
    Nst_GGCList gen1;
    Nst_GGCList gen2;
    Nst_GGCList gen3;
    Nst_GGCList old_gen;
    Nst_Int old_gen_pending;
}
Nst_GarbageCollector;

void nst_collect_gen(Nst_GGCList *gen);
void nst_collect(void);
void nst_add_tracked_object(Nst_GGCObj *obj);
void delete_objects(Nst_GarbageCollector *ggc);

enum Nst_GGCFlags
{
    NST_FLAG_GGC_REACHABLE    = 0b10000000,
    NST_FLAG_GGC_UNREACHABLE  = 0b01000000,
    NST_FLAG_GGC_OBJ_DELETED  = 0b00100000, // used by _destroy_obj
    NST_FLAG_GGC_IS_SUPPORTED = 0b00010000
};

#endif // !GGC_H
