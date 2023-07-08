#ifndef NEST_JSON_H
#define NEST_JSON_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();

NST_FUNC_SIGN(load_s_);
NST_FUNC_SIGN(load_f_);
NST_FUNC_SIGN(dump_s_);
NST_FUNC_SIGN(dump_f_);
NST_FUNC_SIGN(set_options_);
NST_FUNC_SIGN(get_options_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_JSON_H