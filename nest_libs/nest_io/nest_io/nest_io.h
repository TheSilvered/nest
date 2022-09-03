#ifndef NEST_IO_H
#define NEST_IO_H

#include "../../../nest/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) Nst_FuncDeclr *get_func_ptrs();
__declspec(dllexport) NST_INIT_LIB_OBJ_FUNC;
__declspec(dllexport) void free_lib();

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
