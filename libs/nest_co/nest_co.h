#ifndef NEST_CO_H
#define NEST_CO_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();
NstEXP void NstC free_lib();

typedef struct _CoroutineObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_FuncObj *func;
    Nst_MapObj *vars;
    Nst_MapObj *globals;
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

Nst_Obj *coroutine_new(Nst_FuncObj *func);
void coroutine_traverse(CoroutineObj *co);
void coroutine_destroy(CoroutineObj *co);

Nst_FUNC_SIGN(create_);
Nst_FUNC_SIGN(call_);
Nst_FUNC_SIGN(pause_);
Nst_FUNC_SIGN(get_state_);
Nst_FUNC_SIGN(generator_);
Nst_FUNC_SIGN(_get_co_type_obj_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_CO_H
