#ifndef FUNCTION_H
#define FUNCTION_H

#include "nodes.h"
#include "obj.h"
#include "str.h"
#include "compiler.h"

#define AS_FUNC(ptr) ((Nst_FuncObj *)(ptr))
#define AS_BFUNC(ptr) ((Nst_BcFuncObj *)(ptr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    OBJ_HEAD;
    Nst_Node *body;
    Nst_Obj *(*cbody)(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
    Nst_Obj **args;
    size_t arg_num;
}
Nst_FuncObj;

typedef struct
{
    OBJ_HEAD;
    Nst_InstructionList *body;
    Nst_Obj *(*cbody)(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
    Nst_Obj **args;
    size_t arg_num;
}
Nst_BcFuncObj;

Nst_Obj *new_func(size_t arg_num);
Nst_Obj *new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num,
                                                     Nst_Obj **args,
                                                     Nst_OpErr *err));

Nst_Obj *new_bfunc(size_t arg_num);
Nst_Obj *new_bcfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num,
                                                      Nst_Obj **args,
                                                      Nst_OpErr *err));

void destroy_func(Nst_FuncObj *func);
void destroy_bfunc(Nst_BcFuncObj *func);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FUNCTION_H