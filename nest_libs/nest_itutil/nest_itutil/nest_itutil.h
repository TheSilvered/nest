#ifndef NEST_ITUTIL_H
#define NEST_ITUTIL_H

#include "../../../nest/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) FuncDeclr *get_func_ptrs();
__declspec(dllexport) NST_INIT_LIB_OBJ_FUNC;

Nst_Obj *count(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *cycle(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *repeat(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *chain(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *zip(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *enumerate(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *keys(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *values(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *items(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *reversed(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_ITUTIL_H