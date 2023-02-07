#ifndef NEST_UTF8_H
#define NEST_UTF8_H

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

NST_FUNC_SIGN(is_valid_);
NST_FUNC_SIGN(get_len_);
NST_FUNC_SIGN(get_at_);
NST_FUNC_SIGN(to_iter_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_UTF8_H
