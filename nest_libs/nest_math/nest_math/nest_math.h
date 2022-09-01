#ifndef NEST_MATH_H
#define NEST_MATH_H

#include "../../../nest/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) FuncDeclr *get_func_ptrs();
__declspec(dllexport) NST_INIT_LIB_OBJ_FUNC;

Nst_Obj *floor_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *ceil_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *round_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *exp_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *log_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *logn_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *log2_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *log10_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *divmod_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *sin_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *cos_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *tan_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *asin_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *acos_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *atan_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *atan2_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *sinh_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *cosh_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *tanh_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *asinh_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *acosh_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *atanh_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *dist_2d_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *dist_3d_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *dist_nd_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *deg_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *rad_(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_MATH_H