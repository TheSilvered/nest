#ifndef NEST_FS_H
#define NEST_FS_H

#include "../../../nest/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) FuncDeclr *get_func_ptrs();
__declspec(dllexport) NST_INIT_LIB_OBJ_FUNC;

Nst_Obj *isdir(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *mkdir(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *mkdirs(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *rmdir(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *rmdir_recursive(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *isfile(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *rmfile(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *copy(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *rename_file(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *list_dir(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *list_dir_recursive(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *absolute_path(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *equivalent(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_get_copy_options(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_FS_H
