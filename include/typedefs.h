/* Clearer types */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) || defined(WIN32)
  #define Nst_WIN
#endif // !Nst_WIN

#if !defined(_Nst_ARCH_x64) && !defined(_Nst_ARCH_x86)
  #if INTPTR_MAX == INT64_MAX
    #define _Nst_ARCH_x64
  #elif INTPTR_MAX == INT32_MAX
    #define _Nst_ARCH_x86
  #else
    #error Failed to determine architecture, define _Nst_ARCH_x64 or _Nst_ARCH_x86
  #endif // !_Nst_ARCHxx
#endif // !_Nst_ARCHxx

#ifdef Nst_WIN
  #define NstEXP __declspec(dllexport)
#else
  #define NstEXP
#endif // !NstEXP

#ifndef Nst_NORETURN
  #if __STDC_VERSION__ >= 201112L
    #define Nst_NORETURN _Noreturn
  #elif defined(Nst_WIN)
    #define Nst_NORETURN __declspec(noreturn)
  #elif defined(__GNUC__)
    #define Nst_NORETURN __attribute__((noreturn))
  #endif // !Nst_NORETURN
#endif // !Nst_NORETURN

#if !defined(Nst_WIN_FMT) && defined(Nst_WIN)
  #define Nst_WIN_FMT _Printf_format_string_
#else
  #define Nst_WIN_FMT
#endif // !Nst_WIN_FMT

#if !defined(Nst_GNU_FMT) && defined(__GNUC__)
  #define Nst_GNU_FMT(m, n) __attribute__((format(printf,m,n)))
#else
  #define Nst_GNU_FMT(m, n)
#endif // !Nst_GNU_FMT

#ifndef Nst_WIN
#include <endian.h>
#endif // !endian.h

#define Nst_LITTLE_ENDIAN 4321
#define Nst_BIG_ENDIAN 1234

#ifndef Nst_ENDIANNESS
  #ifdef Nst_WIN
    #define Nst_ENDIANNESS Nst_LITTLE_ENDIAN
  #elif defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
        defined(__BIG_ENDIAN__) || \
        defined(__ARMEB__) || \
        defined(__THUMBEB__) || \
        defined(__AARCH64EB__) || \
        defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
    #define Nst_ENDIANNESS Nst_BIG_ENDIAN
  #elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
        defined(__LITTLE_ENDIAN__) || \
        defined(__ARMEL__) || \
        defined(__THUMBEL__) || \
        defined(__AARCH64EL__) || \
        defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
    #define Nst_ENDIANNESS Nst_LITTLE_ENDIAN
  #else
    #error Failed to determine endianness, define Nst_LITTLE_ENDIAN as 4321 (LE) or 1234 (BE)
  #endif // !Nst_ENDIANNESS
#endif // !Nst_ENDIANNESS

#ifdef Nst_WIN
  #define NstC __cdecl
#else
  #define NstC
#endif // !NstC

#ifdef Nst_UNUSED
#undef Nst_UNUSED
#endif

#define Nst_UNUSED(v) (void)(v)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef char i8;
NstEXP typedef short i16;
NstEXP typedef long  i32;
NstEXP typedef long long i64;

NstEXP typedef unsigned char  u8;
NstEXP typedef unsigned short u16;
NstEXP typedef unsigned long  u32;
NstEXP typedef unsigned long long u64;

NstEXP typedef float f32;
NstEXP typedef double f64;

NstEXP typedef size_t usize;
NstEXP typedef ptrdiff_t isize;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !TYPEDEFS_H
