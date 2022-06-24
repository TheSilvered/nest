#ifndef NEST_IO_H
#define NEST_IO_H

#include "nest_source/nest_include.h"

#ifdef NESTIO_EXPORTS
#define NEST_IO_API __declspec(dllexport)
#else
#define NEST_IO_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NEST_IO_API bool lib_init();
NEST_IO_API FuncDeclr *get_func_ptrs();
NEST_IO_API INIT_LIB_OBJ_FUNC;

Nst_Obj *open(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *close(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *write(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *write_bytes(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *read(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *read_bytes(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *file_size(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *move_fptr(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *get_fptr(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_get_stdin(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_get_stdout(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_get_stderr(size_t arg_num, Nst_Obj **args, OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_IO_H
