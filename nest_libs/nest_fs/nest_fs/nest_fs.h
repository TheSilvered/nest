#ifndef NEST_FS_H
#define NEST_FS_H

#include "nest_source/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) FuncDeclr *get_func_ptrs();
__declspec(dllexport) INIT_LIB_OBJ_FUNC;

Nst_Obj *isdir(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *mkdir(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rmdir(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *isfile(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rmfile(size_t arg_num, Nst_Obj **args, OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_FS_H
