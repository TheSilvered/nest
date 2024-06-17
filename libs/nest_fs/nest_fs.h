#ifndef NEST_FS_H
#define NEST_FS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();
NstEXP void NstC lib_quit();

Nst_Obj *NstC is_dir_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_file_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_symlink_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_socket_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_block_device_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_char_device_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_named_pipe_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC make_dir_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC make_dirs_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC remove_dir_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC remove_dirs_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC remove_file_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC make_dir_symlink_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC make_file_symlink_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC read_symlink_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC make_hard_link_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC exists_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC copy_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rename_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC list_dir_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC list_dirs_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC absolute_path_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC canonical_path_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC relative_path_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC equivalent_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC join_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC normalize_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC parent_path_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC filename_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC extension_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC CPO_();

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_FS_H
