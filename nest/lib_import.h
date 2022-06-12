#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "obj.h"
#include "error.h"
#include "str.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct FuncDeclr
{
    Nst_Obj *(*func_ptr)(size_t arg_num, Nst_Obj **args, OpErr *err);
    size_t arg_num;
    Nst_string *name;
}
FuncDeclr;

FuncDeclr *new_func_list(size_t count);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H