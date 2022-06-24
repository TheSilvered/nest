#ifndef NEST_FS_H
#define NEST_FS_H

#include "nest_source/nest_include.h"

#ifdef NESTFS_EXPORTS
#define NEST_FS_API __declspec(dllexport)
#else
#define NEST_FS_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NEST_FS_API bool lib_init();
NEST_FS_API FuncDeclr *get_func_ptrs();
NEST_FS_API INIT_LIB_OBJ_FUNC;

Nst_Obj *isdir(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *mkdir(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rmdir(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *isfile(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rmfile(size_t arg_num, Nst_Obj **args, OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_FS_H
