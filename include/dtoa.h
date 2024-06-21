/**
 * @file dtoa.h
 *
 * @brief Functions to format floating point numbers. Header for the library
 * by David M. Gay
 *
 * @author TheSilvered
 */

#ifndef DTOA_H
#define DTOA_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP i8 *NstC Nst_dtoa(f64 dd, int mode, int ndigits, int *decpt, int *sign,
                         i8 **rve);
NstEXP f64 NstC Nst_strtod(const i8 *s00, i8 **se);
NstEXP void NstC Nst_freedtoa(i8 *s);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !DTOA_H
