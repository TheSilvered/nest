#ifndef NEST_ERR_H
#define NEST_ERR_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();

Nst_FUNC_SIGN(try_);
Nst_FUNC_SIGN(_get_err_names_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_ERR_H