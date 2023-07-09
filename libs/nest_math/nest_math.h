#ifndef NEST_MATH_H
#define NEST_MATH_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();

Nst_FUNC_SIGN(floor_);
Nst_FUNC_SIGN(ceil_);
Nst_FUNC_SIGN(round_);
Nst_FUNC_SIGN(exp_);
Nst_FUNC_SIGN(ln_);
Nst_FUNC_SIGN(log_);
Nst_FUNC_SIGN(log2_);
Nst_FUNC_SIGN(log10_);
Nst_FUNC_SIGN(divmod_);
Nst_FUNC_SIGN(sin_);
Nst_FUNC_SIGN(cos_);
Nst_FUNC_SIGN(tan_);
Nst_FUNC_SIGN(asin_);
Nst_FUNC_SIGN(acos_);
Nst_FUNC_SIGN(atan_);
Nst_FUNC_SIGN(atan2_);
Nst_FUNC_SIGN(sinh_);
Nst_FUNC_SIGN(cosh_);
Nst_FUNC_SIGN(tanh_);
Nst_FUNC_SIGN(asinh_);
Nst_FUNC_SIGN(acosh_);
Nst_FUNC_SIGN(atanh_);
Nst_FUNC_SIGN(dist_2d_);
Nst_FUNC_SIGN(dist_3d_);
Nst_FUNC_SIGN(dist_nd_);
Nst_FUNC_SIGN(deg_);
Nst_FUNC_SIGN(rad_);
Nst_FUNC_SIGN(min_);
Nst_FUNC_SIGN(max_);
Nst_FUNC_SIGN(sum_);
Nst_FUNC_SIGN(frexp_);
Nst_FUNC_SIGN(ldexp_);
Nst_FUNC_SIGN(map_);
Nst_FUNC_SIGN(clamp_);
Nst_FUNC_SIGN(gcd_);
Nst_FUNC_SIGN(lcm_);
Nst_FUNC_SIGN(abs_);
Nst_FUNC_SIGN(hypot_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_MATH_H