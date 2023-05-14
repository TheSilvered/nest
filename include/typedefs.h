/* Clearer types */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdbool.h>
#include <stddef.h>

#if defined(_WIN32) || defined(WIN32)
#define WINDOWS
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
#define ARCH_x64
#else
#define ARCH_x86
#endif

#ifdef WINDOWS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifndef NORETURN
#if __STDC_VERSION__ >= 201112L
#define NORETURN _Noreturn
#elif defined(WINDOWS)
#define NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#define NORETURN __attribute__((noreturn))
#endif
#endif

#if !defined(WIN_FMT) && defined(WINDOWS)
#define WIN_FMT _Printf_format_string_
#else
#define WIN_FMT
#endif

#if !defined(GNU_FMT) && defined(__GNUC__)
#define GNU_FMT(m, n) __attribute__((format(printf,m,n)))
#else
#define GNU_FMT(m, n)
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT typedef char i8;
EXPORT typedef short i16;
EXPORT typedef long  i32;
EXPORT typedef long long i64;

EXPORT typedef unsigned char  u8;
EXPORT typedef unsigned short u16;
EXPORT typedef unsigned long  u32;
EXPORT typedef unsigned long long u64;

EXPORT typedef float f32;
EXPORT typedef double f64;

EXPORT typedef size_t usize;
EXPORT typedef ptrdiff_t isize;

#ifdef __cplusplus
}
#endif

#endif // !TYPEDEFS_H
