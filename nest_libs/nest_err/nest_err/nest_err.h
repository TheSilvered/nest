#ifndef NEST_ERR_H
#define NEST_ERR_H

#include "../../../nest/nest_include.h"

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

NST_FUNC_SIGN(try_);
NST_FUNC_SIGN(raise_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_ERR_H