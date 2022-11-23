#ifndef NEST_CO_H
#define NEST_CO_H

#include "../../../src/nest.h"

#if defined(_WIN32) || defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_FuncDeclr *get_func_ptrs();
EXPORT NST_INIT_LIB_OBJ_FUNC;
EXPORT void free_lib();

typedef struct _CorutineObj
{
    NST_OBJ_HEAD;
    NST_GGC_SUPPORT;
    Nst_FuncObj *func;
    Nst_MapObj *vars;
    Nst_MapObj *globals;
    Nst_Obj **stack;
    size_t stack_size;
    Nst_Int idx;

    // If the function of the coroutine is called without co.call, this and the
    // size of the call stack when pausing won't match
    size_t call_stack_size;
}
CorutineObj;

enum CorutineFlags
{
    FLAG_CO_SUSPENDED = 0b0001,
    FLAG_CO_RUNNING   = 0b0010,
    FLAG_CO_PAUSED    = 0b0100,
    FLAG_CO_ENDED     = 0b1000,

    // Assigned to the function object
    FLAG_FUNC_IS_CO   = 0b1000
};

Nst_Obj *new_coroutine(Nst_FuncObj *func);
void traverse_coroutine(CorutineObj *co);
void track_coroutine(CorutineObj *co);
void destroy_coroutine(CorutineObj *co);

NST_FUNC_SIGN(create_);
NST_FUNC_SIGN(call_);
NST_FUNC_SIGN(pause_);
NST_FUNC_SIGN(get_state_);
NST_FUNC_SIGN(_get_co_type_obj_);

// Here you can put your function signatures
// They must always be `NST_FUNC_SIGN(func_name);`
// replace func_name with your function's name

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_CO_H