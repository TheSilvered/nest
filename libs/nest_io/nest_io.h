#ifndef NEST_IO_H
#define NEST_IO_H

#include "nest.h"

#if defined(_WIN32) || defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_FuncDeclr *get_func_ptrs();
EXPORT NST_INIT_LIB_OBJ_FUNC;
EXPORT void free_lib();

NST_FUNC_SIGN(open_);
NST_FUNC_SIGN(close_);
NST_FUNC_SIGN(write_);
NST_FUNC_SIGN(write_bytes_);
NST_FUNC_SIGN(read_);
NST_FUNC_SIGN(read_bytes_);
NST_FUNC_SIGN(file_size_);
NST_FUNC_SIGN(get_fptr_);
NST_FUNC_SIGN(move_fptr_);
NST_FUNC_SIGN(flush_);
NST_FUNC_SIGN(_get_stdin_);
NST_FUNC_SIGN(_get_stdout_);
NST_FUNC_SIGN(_get_stderr_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_IO_H
