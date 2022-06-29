#ifndef NEST_SUTIL_H
#define NEST_SUTIL_H

#include "nest_source/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) FuncDeclr *get_func_ptrs();
__declspec(dllexport) INIT_LIB_OBJ_FUNC;

Nst_Obj *nst_lfind(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rfind(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *trim(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *ltrim(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rtrim(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *ljust(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rjust(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *to_upper(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *to_lower(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *is_upper(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *is_lower(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *is_alpha(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *is_digit(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *is_alnum(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *is_charset(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *is_printable(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *replace_substr(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *bytearray_to_str(size_t arg_num, Nst_Obj **args, OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SUTIL_H