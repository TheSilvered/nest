#ifndef NEST_CO_H
#define NEST_CO_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_DeclrList *get_func_ptrs();
EXPORT void free_lib();

typedef struct _CoroutineObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    Nst_FuncObj *func;
    Nst_MapObj *vars;
    Nst_MapObj *globals;
    Nst_Obj **stack;
    usize stack_size;
    Nst_Int idx;

    // If the function of the coroutine is called without co.call, this and the
    // size of the call stack when pausing won't match
    usize call_stack_size;
}
CoroutineObj;

enum CoroutineFlags
{
    FLAG_CO_SUSPENDED = 0b0001,
    FLAG_CO_RUNNING   = 0b0010,
    FLAG_CO_PAUSED    = 0b0100,
    FLAG_CO_ENDED     = 0b1000,

    // Assigned to the function object
    FLAG_FUNC_IS_CO   = 0b1000
};

Nst_Obj *coroutine_new(Nst_FuncObj *func);
void coroutine_traverse(CoroutineObj *co);
void coroutine_track(CoroutineObj *co);
void coroutine_destroy(CoroutineObj *co);

NST_FUNC_SIGN(create_);
NST_FUNC_SIGN(call_);
NST_FUNC_SIGN(pause_);
NST_FUNC_SIGN(get_state_);
NST_FUNC_SIGN(generator_);
NST_FUNC_SIGN(_get_co_type_obj_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_CO_H