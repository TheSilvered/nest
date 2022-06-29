#ifndef NEST_RAND_H
#define NEST_RAND_H

#include "nest_source/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) FuncDeclr *get_func_ptrs();
__declspec(dllexport) INIT_LIB_OBJ_FUNC;

Nst_Obj *random(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rand_int(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rand_perc(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *choice(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *shuffle(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *rand_seed(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_get_rand_max(size_t arg_num, Nst_Obj **args, OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_RAND_H
