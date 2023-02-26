#ifndef NEST_FS_H
#define NEST_FS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_FuncDeclr *get_func_ptrs();
EXPORT void free_lib();

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
NST_FUNC_SIGN(canonical_path_);
NST_FUNC_SIGN(relative_path_);
NST_FUNC_SIGN(equivalent_);
NST_FUNC_SIGN(join_);
NST_FUNC_SIGN(path_);
NST_FUNC_SIGN(filename_);
NST_FUNC_SIGN(extension_);
NST_FUNC_SIGN(_get_copy_options_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_FS_H
