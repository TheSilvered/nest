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

Nst_Obj *new_func(size_t arg_num);
Nst_Obj *new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num,
                                                      Nst_Obj **args,
                                                      Nst_OpErr *err));
void _nst_set_global_vt(Nst_FuncObj *func, Nst_MapObj *map);

void nst_traverse_func(Nst_FuncObj *func);
void nst_destroy_func(Nst_FuncObj *func);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FUNCTION_H