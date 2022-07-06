#ifndef FUNCTION_H
#define FUNCTION_H

#include "nodes.h"
#include "obj.h"
#include "str.h"

#define AS_FUNC(ptr) ((Nst_func *)(ptr->value))
#define AS_FUNC_V(ptr) ((Nst_func *)(ptr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    Node *body;
    Nst_Obj *(*cbody)(size_t arg_num, Nst_Obj **args, OpErr *err);
    Nst_Obj **args;
    size_t arg_num;
}
Nst_func;

Nst_Obj *new_func_obj(Nst_func *func);
Nst_func *new_func(size_t arg_num);
Nst_func *new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num, Nst_Obj **args, OpErr *err));
void destroy_func(Nst_func *func);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FUNCTION_H