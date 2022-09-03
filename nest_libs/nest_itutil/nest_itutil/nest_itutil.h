#ifndef NEST_ITUTIL_H
#define NEST_ITUTIL_H

#include "../../../nest/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) Nst_FuncDeclr *get_func_ptrs();
__declspec(dllexport) NST_INIT_LIB_OBJ_FUNC;

NST_FUNC_SIGN(count_);
NST_FUNC_SIGN(cycle_);
NST_FUNC_SIGN(repeat_);
NST_FUNC_SIGN(chain_);
NST_FUNC_SIGN(zip_);
NST_FUNC_SIGN(enumerate_);
NST_FUNC_SIGN(keys_);
NST_FUNC_SIGN(values_);
NST_FUNC_SIGN(items_);
NST_FUNC_SIGN(reversed_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_ITUTIL_H