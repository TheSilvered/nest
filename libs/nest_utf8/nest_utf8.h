#ifndef NEST_UTF8_H
#define NEST_UTF8_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();

NST_FUNC_SIGN(is_valid_);
NST_FUNC_SIGN(get_len_);
NST_FUNC_SIGN(get_at_);
NST_FUNC_SIGN(to_iter_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_UTF8_H
