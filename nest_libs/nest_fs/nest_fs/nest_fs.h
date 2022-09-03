#ifndef NEST_FS_H
#define NEST_FS_H

#include "../../../nest/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) Nst_FuncDeclr *get_func_ptrs();
__declspec(dllexport) NST_INIT_LIB_OBJ_FUNC;

NST_FUNC_SIGN(isdir_);
NST_FUNC_SIGN(mkdir_);
NST_FUNC_SIGN(mkdirs_);
NST_FUNC_SIGN(rmdir_);
NST_FUNC_SIGN(rmdir_recursive_);
NST_FUNC_SIGN(isfile_);
NST_FUNC_SIGN(rmfile_);
NST_FUNC_SIGN(copy_);
NST_FUNC_SIGN(rename_);
NST_FUNC_SIGN(list_dir_);
NST_FUNC_SIGN(list_dir_recursive_);
NST_FUNC_SIGN(absolute_path_);
NST_FUNC_SIGN(equivalent_);
NST_FUNC_SIGN(_get_copy_options_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_FS_H
