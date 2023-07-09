#ifndef NEST_IO_H
#define NEST_IO_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();
NstEXP void free_lib();

typedef struct _VirtualIOFile_data
{
    usize ptr;
    usize size;
    i8 *data;
    i8 *buf;
    usize buf_size;
    usize curr_buf_size;
}
VirtualIOFile_data;

Nst_FUNC_SIGN(open_);
Nst_FUNC_SIGN(virtual_file_);
Nst_FUNC_SIGN(close_);
Nst_FUNC_SIGN(write_);
Nst_FUNC_SIGN(write_bytes_);
Nst_FUNC_SIGN(read_);
Nst_FUNC_SIGN(read_bytes_);
Nst_FUNC_SIGN(file_size_);
Nst_FUNC_SIGN(get_fptr_);
Nst_FUNC_SIGN(move_fptr_);
Nst_FUNC_SIGN(flush_);
Nst_FUNC_SIGN(get_flags_);
Nst_FUNC_SIGN(println_);
Nst_FUNC_SIGN(_set_stdin_);
Nst_FUNC_SIGN(_set_stdout_);
Nst_FUNC_SIGN(_set_stderr_);
Nst_FUNC_SIGN(_get_stdin_);
Nst_FUNC_SIGN(_get_stdout_);
Nst_FUNC_SIGN(_get_stderr_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_IO_H
