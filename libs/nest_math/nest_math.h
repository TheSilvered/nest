#ifndef NEST_MATH_H
#define NEST_MATH_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();

Nst_Obj *NstC floor_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC ceil_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC round_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC exp_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC ln_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC log_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC log2_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC log10_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC divmod_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC sin_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cos_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC tan_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC asin_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC acos_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC atan_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC atan2_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC sinh_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cosh_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC tanh_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC asinh_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC acosh_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC atanh_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC dist_2d_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC dist_3d_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC dist_nd_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC deg_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rad_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC min_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC max_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC sum_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC frexp_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC ldexp_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC map_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC clamp_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC gcd_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC lcm_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC abs_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC hypot_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_nan_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_inf_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC INF_();
Nst_Obj *NstC NAN_();

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_MATH_H