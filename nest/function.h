/* Nst_FuncObj interface */

#ifndef FUNCTION_H
#define FUNCTION_H

#include "nodes.h"
#include "obj.h"
#include "str.h"
#include "compiler.h"
#include "map.h"
#include "ggc.h"

#define AS_FUNC(ptr) ((Nst_FuncObj *)(ptr))
#define nst_set_global_vt(func, map) _nst_set_global_vt((Nst_FuncObj *)(func), (Nst_MapObj *)(map))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    NST_OBJ_HEAD;
    NST_GGC_SUPPORT;
    Nst_InstructionList *body;
    Nst_Obj *(*cbody)(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
    Nst_Obj **args;
    size_t arg_num;
    Nst_MapObj *mod_globals;
}
Nst_FuncObj;

// Creates a new function objects that accepts `arg_num` args
// The function's `args` must be set manually
Nst_Obj *new_func(size_t arg_num);
// Creates a new function object that is a wrapper of a C function
Nst_Obj *new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num,
                                                      Nst_Obj **args,
                                                      Nst_OpErr *err));
// Sets mod_globals for the function
void _nst_set_global_vt(Nst_FuncObj *func, Nst_MapObj *map);

// traverse function for Nst_FuncObj, needed for the GGC
void nst_traverse_func(Nst_FuncObj *func);
// destroy function for Nst_Func_Obj
void nst_destroy_func(Nst_FuncObj *func);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FUNCTION_H