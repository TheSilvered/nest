/* Clearer types */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdbool.h>
#include <stddef.h>

#if defined(_WIN32) || defined(WIN32)
  #define Nst_WIN
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
  #define Nst_x64
#else
  #define Nst_x86
#endif

#ifdef Nst_WIN
  #define NstEXP __declspec(dllexport)
#else
  #define NstEXP
#endif

#ifndef Nst_NORETURN
  #if __STDC_VERSION__ >= 201112L
    #define Nst_NORETURN _Noreturn
  #elif defined(Nst_WIN)
    #define Nst_NORETURN __declspec(noreturn)
  #elif defined(__GNUC__)
    #define Nst_NORETURN __attribute__((noreturn))
  #endif
#endif

#if !defined(Nst_WIN_FMT) && defined(Nst_WIN)
  #define Nst_WIN_FMT _Printf_format_string_
#else
  #define Nst_WIN_FMT
#endif

#if !defined(Nst_GNU_FMT) && defined(__GNUC__)
  #define Nst_GNU_FMT(m, n) __attribute__((format(printf,m,n)))
#else
  #define Nst_GNU_FMT(m, n)
#endif

#ifndef Nst_WIN
#include <endian.h>
#else
#define LITTLE_ENDIAN 0
#define BIG_ENDIAN 1
#endif

#ifndef ENDIANNESS
  #ifdef Nst_WIN
    #define ENDIANNESS LITTLE_ENDIAN
  #elif defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
        defined(__BIG_ENDIAN__) || \
        defined(__ARMEB__) || \
        defined(__THUMBEB__) || \
        defined(__AARCH64EB__) || \
        defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
    #define ENDIANNESS BIG_ENDIAN
  #elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
        defined(__LITTLE_ENDIAN__) || \
        defined(__ARMEL__) || \
        defined(__THUMBEL__) || \
        defined(__AARCH64EL__) || \
        defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
    #define ENDIANNESS LITTLE_ENDIAN
  #else
    #error Failed to determine endianness, define ENDIANNESS as 0 (LE) or 1 (BE)
  #endif
#endif

#ifdef Nst_WIN
  #define NstC __cdecl
#else
  #define NstC
#endif

#ifdef __cplusplus
extern "C" {
#endif

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
#endif

#endif // !TYPEDEFS_H
