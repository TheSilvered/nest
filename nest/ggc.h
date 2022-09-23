#ifndef GGC_H
#define GGC_H

#include "obj.h"

#define NST_GGC_SUPPORT \
    struct Nst_GGCObject *ggc_next; \
    struct Nst_GGCObject *ggc_prev; \
    void (* traverse_func)(Nst_Obj *)

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
}
Nst_GarbageCollector;

void collect_gen(Nst_GGCList *gen);

enum Nst_GGCFlags
{
    NST_FLAG_GGC_REACHABLE    = 0b10000000,
    NST_FLAG_GGC_UNREACHABLE  = 0b01000000,
    NST_FLAG_GGC_OBJ_DELETED  = 0b00100000, // used by _destroy_obj
    NST_FLAG_GGC_IS_SUPPORTED = 0b00010000
};

#endif // !GGC_H
