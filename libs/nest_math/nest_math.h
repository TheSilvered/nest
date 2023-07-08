#ifndef NEST_MATH_H
#define NEST_MATH_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();

NST_FUNC_SIGN(floor_);
NST_FUNC_SIGN(ceil_);
NST_FUNC_SIGN(round_);
NST_FUNC_SIGN(exp_);
NST_FUNC_SIGN(ln_);
NST_FUNC_SIGN(log_);
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
NST_FUNC_SIGN(sum_);
NST_FUNC_SIGN(frexp_);
NST_FUNC_SIGN(ldexp_);
NST_FUNC_SIGN(map_);
NST_FUNC_SIGN(clamp_);
NST_FUNC_SIGN(gcd_);
NST_FUNC_SIGN(lcm_);
NST_FUNC_SIGN(abs_);
NST_FUNC_SIGN(hypot_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_MATH_H