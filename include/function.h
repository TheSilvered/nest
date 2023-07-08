/* Nst_FuncObj interface */

#ifndef FUNCTION_H
#define FUNCTION_H

#include "compiler.h"
#include "map.h"

#define FUNC(ptr) ((Nst_FuncObj *)(ptr))
// Sets mod_globals for the function
#define nst_func_set_vt(func, map) \
    _nst_func_set_vt((Nst_FuncObj *)(func), (Nst_MapObj *)(map))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef union _Nst_FuncBody
{
    Nst_InstList *bytecode;
    Nst_Obj *(*c_func)(usize arg_num, Nst_Obj **args);
}
Nst_FuncBody;

NstEXP typedef struct _Nst_FuncObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    Nst_FuncBody body;
    Nst_Obj **args;
    usize arg_num;
    Nst_MapObj *mod_globals;
}
Nst_FuncObj;

// Creates a new function objects that accepts `arg_num` args
// The function's `args` must be set manually
NstEXP Nst_Obj *nst_func_new(usize arg_num, Nst_InstList *bytecode);
// Creates a new function object that is a wrapper of a C function
NstEXP Nst_Obj *nst_func_new_c(usize arg_num,
                               Nst_Obj *(*cbody)(usize     arg_num,
                                                 Nst_Obj  **args));
NstEXP void _nst_func_set_vt(Nst_FuncObj *func, Nst_MapObj *map);

// traverse function for Nst_FuncObj, needed for the GGC
NstEXP void _nst_func_traverse(Nst_FuncObj *func);
NstEXP void _nst_func_track(Nst_FuncObj *func);
// destroy function for Nst_Func_Obj
NstEXP void _nst_func_destroy(Nst_FuncObj *func);

NstEXP typedef enum _Nst_FuncFlags
{
    NST_FLAG_FUNC_IS_C = 0b1
}
Nst_FuncFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FUNCTION_H
