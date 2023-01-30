#ifndef NEST_JSON_H
#define NEST_JSON_H

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

NST_FUNC_SIGN(parse_s_);
NST_FUNC_SIGN(parse_f_);
NST_FUNC_SIGN(dump_s_);
NST_FUNC_SIGN(dump_f_);
NST_FUNC_SIGN(set_options_);
NST_FUNC_SIGN(get_options_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_JSON_H