#ifndef NEST_IO_H
#define NEST_IO_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_FuncDeclr *get_func_ptrs();
EXPORT void free_lib();

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

NST_FUNC_SIGN(open_);
NST_FUNC_SIGN(virtual_iof_);
NST_FUNC_SIGN(close_);
NST_FUNC_SIGN(write_);
NST_FUNC_SIGN(write_bytes_);
NST_FUNC_SIGN(read_);
NST_FUNC_SIGN(read_bytes_);
NST_FUNC_SIGN(file_size_);
NST_FUNC_SIGN(get_fptr_);
NST_FUNC_SIGN(move_fptr_);
NST_FUNC_SIGN(flush_);
NST_FUNC_SIGN(get_flags_);
NST_FUNC_SIGN(println_);
NST_FUNC_SIGN(_set_stdin_);
NST_FUNC_SIGN(_set_stdout_);
NST_FUNC_SIGN(_set_stderr_);
NST_FUNC_SIGN(_get_stdin_);
NST_FUNC_SIGN(_get_stdout_);
NST_FUNC_SIGN(_get_stderr_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_IO_H
