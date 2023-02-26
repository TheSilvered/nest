#ifndef NEST_RAND_H
#define NEST_RAND_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_FuncDeclr *get_func_ptrs();

NST_FUNC_SIGN(random_);
NST_FUNC_SIGN(rand_int_);
NST_FUNC_SIGN(rand_perc_);
NST_FUNC_SIGN(choice_);
NST_FUNC_SIGN(shuffle_);
NST_FUNC_SIGN(seed_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_RAND_H
