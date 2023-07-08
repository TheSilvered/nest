#ifndef NEST_ITUTIL_H
#define NEST_ITUTIL_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();

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
NST_FUNC_SIGN(iter_start_);
NST_FUNC_SIGN(iter_is_done_);
NST_FUNC_SIGN(iter_get_val_);
NST_FUNC_SIGN(iter_advance_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_ITUTIL_H