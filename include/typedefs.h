/* Clearer types */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdbool.h>
#include <stddef.h>

#if defined(_WIN32) || defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
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
