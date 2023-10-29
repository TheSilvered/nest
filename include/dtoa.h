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

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#ifdef ENABLE_NST_FMT

Nst_dtoa
Nst_strtod
Nst_freedtoa

#endif ENABLE_NST_FMT

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !DTOA_H
