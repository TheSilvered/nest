#ifndef NEST_ERR_H
#define NEST_ERR_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_FuncDeclr *get_func_ptrs();

NST_FUNC_SIGN(try_);
NST_FUNC_SIGN(_get_err_names_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_ERR_H