#ifndef NEST_CO_H
#define NEST_CO_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();
NstEXP void NstC lib_quit();

typedef struct _CoroutineObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_Obj *func;
    Nst_VarTable *vt;
    Nst_Obj **stack;
    usize stack_size;
    i64 idx;
    usize call_stack_size;
} CoroutineObj;

typedef struct _CoroutineCallStack {
    CoroutineObj **stack;
    usize len;
    usize cap;
} CoroutineCallStack;

enum _CoroutineFlags {
    FLAG_CO_SUSPENDED = Nst_FLAG(1),
    FLAG_CO_RUNNING   = Nst_FLAG(2),
    FLAG_CO_PAUSED    = Nst_FLAG(3),
    FLAG_CO_ENDED     = Nst_FLAG(4),

    // Assigned to the function object
    FLAG_FUNC_IS_CO   = Nst_FLAG(4)
} CoroutineFlags;

Nst_Obj *coroutine_new(Nst_Obj *func);
void coroutine_traverse(CoroutineObj *co);
void coroutine_destroy(CoroutineObj *co);

Nst_Obj *NstC create_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC call_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC yield_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC get_state_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC generator_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _get_co_type_obj_(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_CO_H
