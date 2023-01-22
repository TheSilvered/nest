/* Nst_FuncObj interface */

#ifndef FUNCTION_H
#define FUNCTION_H

#include "nodes.h"
#include "obj.h"
#include "str.h"
#include "compiler.h"
#include "map.h"
#include "ggc.h"

#define FUNC(ptr) ((Nst_FuncObj *)(ptr))
// Sets mod_globals for the function
#define nst_set_vt_func(func, map) \
    _nst_set_vt_func((Nst_FuncObj *)(func), (Nst_MapObj *)(map))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef union _FuncBody
{
    Nst_InstructionList *bytecode;
    Nst_Obj *(*c_func)(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
}
FuncBody;

typedef struct _Nst_FuncObj
{
    NST_OBJ_HEAD;
    NST_GGC_SUPPORT;
    FuncBody body;
    Nst_Obj **args;
    size_t arg_num;
    Nst_MapObj *mod_globals;
}
Nst_FuncObj;

// Creates a new function objects that accepts `arg_num` args
// The function's `args` must be set manually
Nst_Obj *nst_new_func(size_t arg_num, Nst_InstructionList *bytecode);
// Creates a new function object that is a wrapper of a C function
Nst_Obj *nst_new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t     arg_num,
                                                         Nst_Obj  **args,
                                                         Nst_OpErr *err));
void _nst_set_vt_func(Nst_FuncObj *func, Nst_MapObj *map);

// traverse function for Nst_FuncObj, needed for the GGC
void nst_traverse_func(Nst_FuncObj *func);
void nst_track_func(Nst_FuncObj *func);
// destroy function for Nst_Func_Obj
void nst_destroy_func(Nst_FuncObj *func);

enum Nst_FuncFlags
{
    NST_FLAG_FUNC_IS_C = 0b1
};

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FUNCTION_H