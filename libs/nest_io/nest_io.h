#ifndef NEST_IO_H
#define NEST_IO_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();
NstEXP void NstC lib_quit();

typedef struct _VirtualFile {
    Nst_DynArray data;
    usize ptr;
} VirtualFile;

Nst_Obj *NstC open_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC virtual_file_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC close_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC write_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC write_bytes_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC read_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC read_bytes_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC file_size_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC seek_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC flush_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC get_flags_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC can_read_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC can_write_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC can_seek_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_bin_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_a_tty_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC descriptor_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC encoding_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC println_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _set_stdin_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _set_stdout_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _set_stderr_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _get_stdin_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _get_stdout_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _get_stderr_(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_IO_H
