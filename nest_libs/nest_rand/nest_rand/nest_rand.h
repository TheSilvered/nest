#ifndef NEST_RAND_H
#define NEST_RAND_H

#include "../../../nest/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) Nst_FuncDeclr *get_func_ptrs();
__declspec(dllexport) NST_INIT_LIB_OBJ_FUNC;

NST_FUNC_SIGN(random_);
NST_FUNC_SIGN(rand_int_);
NST_FUNC_SIGN(rand_perc_);
NST_FUNC_SIGN(choice_);
NST_FUNC_SIGN(shuffle_);
NST_FUNC_SIGN(rand_seed_);
NST_FUNC_SIGN(_get_rand_max_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_RAND_H
