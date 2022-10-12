#ifndef NEST_SYS_H
#define NEST_SYS_H

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

NST_FUNC_SIGN(system_);
NST_FUNC_SIGN(exit_);
NST_FUNC_SIGN(getenv_);
NST_FUNC_SIGN(get_ref_count_);
NST_FUNC_SIGN(get_addr_);
NST_FUNC_SIGN(hash_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SYS_H