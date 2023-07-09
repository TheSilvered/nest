#ifndef NEST_JSON_H
#define NEST_JSON_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();

Nst_FUNC_SIGN(load_s_);
Nst_FUNC_SIGN(load_f_);
Nst_FUNC_SIGN(dump_s_);
Nst_FUNC_SIGN(dump_f_);
Nst_FUNC_SIGN(set_options_);
Nst_FUNC_SIGN(get_options_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_JSON_H