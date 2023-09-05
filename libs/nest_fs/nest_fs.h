#ifndef NEST_FS_H
#define NEST_FS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();
NstEXP void NstC free_lib();

Nst_FUNC_SIGN(is_dir_);
Nst_FUNC_SIGN(is_file_);
Nst_FUNC_SIGN(is_symlink_);
Nst_FUNC_SIGN(is_socket_);
Nst_FUNC_SIGN(is_block_device_);
Nst_FUNC_SIGN(is_char_device_);
Nst_FUNC_SIGN(is_named_pipe_);
Nst_FUNC_SIGN(make_dir_);
Nst_FUNC_SIGN(make_dirs_);
Nst_FUNC_SIGN(remove_dir_);
Nst_FUNC_SIGN(remove_dirs_);
Nst_FUNC_SIGN(remove_file_);
Nst_FUNC_SIGN(make_dir_symlink_);
Nst_FUNC_SIGN(make_file_symlink_);
Nst_FUNC_SIGN(read_symlink_);
Nst_FUNC_SIGN(make_hard_link_);
Nst_FUNC_SIGN(exists_);
Nst_FUNC_SIGN(copy_);
Nst_FUNC_SIGN(rename_);
Nst_FUNC_SIGN(list_dir_);
Nst_FUNC_SIGN(list_dirs_);
Nst_FUNC_SIGN(absolute_path_);
Nst_FUNC_SIGN(canonical_path_);
Nst_FUNC_SIGN(relative_path_);
Nst_FUNC_SIGN(equivalent_);
Nst_FUNC_SIGN(join_);
Nst_FUNC_SIGN(normalize_);
Nst_FUNC_SIGN(parent_path_);
Nst_FUNC_SIGN(filename_);
Nst_FUNC_SIGN(extension_);
Nst_FUNC_SIGN(_get_copy_options_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_FS_H
