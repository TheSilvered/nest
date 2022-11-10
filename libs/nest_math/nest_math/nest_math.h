#ifndef NEST_MATH_H
#define NEST_MATH_H

#include "../../../src/nest.h"

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

NST_FUNC_SIGN(floor_);
NST_FUNC_SIGN(ceil_);
NST_FUNC_SIGN(round_);
NST_FUNC_SIGN(exp_);
NST_FUNC_SIGN(log_);
NST_FUNC_SIGN(logn_);
NST_FUNC_SIGN(log2_);
NST_FUNC_SIGN(log10_);
NST_FUNC_SIGN(divmod_);
NST_FUNC_SIGN(sin_);
NST_FUNC_SIGN(cos_);
NST_FUNC_SIGN(tan_);
NST_FUNC_SIGN(asin_);
NST_FUNC_SIGN(acos_);
NST_FUNC_SIGN(atan_);
NST_FUNC_SIGN(atan2_);
NST_FUNC_SIGN(sinh_);
NST_FUNC_SIGN(cosh_);
NST_FUNC_SIGN(tanh_);
NST_FUNC_SIGN(asinh_);
NST_FUNC_SIGN(acosh_);
NST_FUNC_SIGN(atanh_);
NST_FUNC_SIGN(dist_2d_);
NST_FUNC_SIGN(dist_3d_);
NST_FUNC_SIGN(dist_nd_);
NST_FUNC_SIGN(deg_);
NST_FUNC_SIGN(rad_);
NST_FUNC_SIGN(min_);
NST_FUNC_SIGN(max_);
NST_FUNC_SIGN(min_seq_);
NST_FUNC_SIGN(max_seq_);
NST_FUNC_SIGN(sum_seq_);
NST_FUNC_SIGN(frexp_);
NST_FUNC_SIGN(ldexp_);
NST_FUNC_SIGN(map_);
NST_FUNC_SIGN(clamp_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_MATH_H