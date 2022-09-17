#ifndef GGC_H
#define GGC_H

#include "obj.h"

#define NST_GGC_SUPPORT \
    Nst_Obj *ggc_next; \
    Nst_Obj *ggc_prev; \
    Nst_GarbageCollector *ggc

typedef struct
{
    Nst_Obj *gen1_head;
    Nst_Obj *gen1_tail;
    size_t gen1_size;
    Nst_Obj *gen2_head;
    Nst_Obj *gen2_tail;
    size_t gen2_size;
    Nst_Obj *gen3_head;
    Nst_Obj *gen4_tail;
    size_t gen3_size;
}
Nst_GarbageCollector;

#endif // !GGC_H
