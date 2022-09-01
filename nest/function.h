#ifndef FUNCTION_H
#define FUNCTION_H

#include "nodes.h"
#include "obj.h"
#include "str.h"
#include "compiler.h"

#define AS_FUNC(ptr) ((Nst_FuncObj *)(ptr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    OBJ_HEAD;
    Nst_InstructionList *body;
    Nst_Obj *(*cbody)(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
    Nst_Obj **args;
    size_t arg_num;
}
Nst_FuncObj;

Nst_Obj *new_func(size_t arg_num);
Nst_Obj *new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num,
                                                      Nst_Obj **args,
                                                      Nst_OpErr *err));

void destroy_func(Nst_FuncObj *func);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FUNCTION_H