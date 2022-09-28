#ifndef GGC_H
#define GGC_H

#include "obj.h"
#include "simple_types.h"

#define NST_GGC_SUPPORT \
    struct Nst_GGCObject *ggc_next; \
    struct Nst_GGCObject *ggc_prev; \
    void (* traverse_func)(Nst_Obj *)

#define NST_GEN1_MAX 700
#define NST_GEN2_MAX 10
#define NST_GEN3_MAX 10
#define NST_OLD_GEN_MIN 10

typedef struct Nst_GGCObject
{
    NST_OBJ_HEAD;
    NST_GGC_SUPPORT;
}
Nst_GGCObject;

typedef struct
{
    Nst_GGCObject *head;
    Nst_GGCObject *tail;
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
void nst_collect();
void nst_add_tracked_object(Nst_GGCObject *obj);

enum Nst_GGCFlags
{
    NST_FLAG_GGC_REACHABLE    = 0b10000000,
    NST_FLAG_GGC_UNREACHABLE  = 0b01000000,
    NST_FLAG_GGC_OBJ_DELETED  = 0b00100000, // used by _destroy_obj
    NST_FLAG_GGC_IS_SUPPORTED = 0b00010000
};

#endif // !GGC_H
