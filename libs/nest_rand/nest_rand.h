#ifndef NEST_RAND_H
#define NEST_RAND_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();

Nst_FUNC_SIGN(random_);
Nst_FUNC_SIGN(rand_int_);
Nst_FUNC_SIGN(rand_perc_);
Nst_FUNC_SIGN(choice_);
Nst_FUNC_SIGN(shuffle_);
Nst_FUNC_SIGN(seed_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_RAND_H
