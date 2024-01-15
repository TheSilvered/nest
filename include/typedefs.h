/**
 * @file typedefs.h
 *
 * @brief Clearer C types and useful platform-agnostic macros
 *
 * @author TheSilvered
 */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

// DO NOT ENABLE, used for documentation purpuses. If you need to define these
// macros, define them elsewhere in this file.
#if 0
/**
 * If defined enables tracking of the position in the program where objects are
 * allocated. This macro should be defined in `typedefs.h` when compiling.
 *
 * @brief Note: this macro works only when the program is compiled in debug
 * mode.
 */
#define Nst_TRACK_OBJ_INIT_POS
/**
 * If defined disables object pools and instead frees the memory of each
 * object. This macro should be defined in `typedefs.h` when compiling.
 *
 * @brief Note: this macro works only when the program is compiled in debug
 * mode.
 */
#define Nst_DISABLE_POOLS
/**
 * If defined enables allocation counting and declares the
 * `Nst_log_alloc_count` function. This macro should be defined in `typedefs.h`
 * when compiling.
 *
 * @brief Note: this macro works only when the program is compiled in debug
 * mode.
 */
#define Nst_COUNT_ALLOC
/**
 * On Windows, instead of calling abort, __debugbreak is called instead when an
 * assetion fails.
 */
#define Nst_BREAKPOINT_ON_ASSERTION_FAIL
#endif // !0

// #define Nst_TRACK_OBJ_INIT_POS
#define Nst_DISABLE_POOLS
#define Nst_COUNT_ALLOC
#define Nst_BREAKPOINT_ON_ASSERTION_FAIL

#if defined(_WIN32) || defined(WIN32)
/* Defined when compiling on MS Windows. */
#define Nst_WIN
#elif !defined(__GNUC__) || defined(__clang__)
#error Use MSVC or GCC to compile.
#else
#define Nst_BUILD_GGC_VER(maj, min, patch) (maj * 10000 + min * 100 + patch)
#define Nst_GCC_VER                                                           \
    Nst_BUILD_GGC_VER(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#endif

#if !defined(_Nst_ARCH_x64) && !defined(_Nst_ARCH_x86)
#if INTPTR_MAX == INT64_MAX
/* Defined when compiling on 64-bit architectures. */
#define _Nst_ARCH_x64
#elif INTPTR_MAX == INT32_MAX
/* Defined when compiling on 32-bit architectures. */
#define _Nst_ARCH_x86
#else
#error Failed to determine architecture, define _Nst_ARCH_x64 or _Nst_ARCH_x86
#endif // !_Nst_ARCHxx
#endif // !_Nst_ARCHxx

#ifdef Nst_WIN
/* Exports a symbol in a dynamic library. */
#define NstEXP __declspec(dllexport)
#else
/* [docs:ignore] Exports a symbol in a dynamic library. */
#define NstEXP
#endif // !NstEXP

#ifndef Nst_NORETURN
#if __STDC_VERSION__ >= 201112L
/* Marks a function that does not finish. */
#define Nst_NORETURN _Noreturn
#elif defined(Nst_WIN)
/* [docs:ignore] Marks a function that does not finish. */
#define Nst_NORETURN __declspec(noreturn)
#elif defined(__GNUC__) && Nst_GCC_VER > Nst_BUILD_GGC_VER(2, 5, 0)
/* [docs:ignore] Marks a function that does not finish. */
#define Nst_NORETURN __attribute__((noreturn))
#else
/* [docs:ignore] Marks a function that does not finish. */
#define Nst_NORETURN
#endif // !Nst_NORETURN
#endif // !Nst_NORETURN

#if !defined(Nst_WIN_FMT) && defined(Nst_WIN)
/* Marks an argument as a printf format string on MSVC. */
#define Nst_WIN_FMT _Printf_format_string_
#else
/* [docs:ignore] Marks an argument as a printf format string on MSVC. */
#define Nst_WIN_FMT
#endif // !Nst_WIN_FMT

#if !defined(Nst_GNU_FMT) && defined(__GNUC__)
/* Marks an argument as a printf format string on GCC. */
#define Nst_GNU_FMT(m, n) __attribute__((format(printf,m,n)))
#else
/* [docs:ignore]  Marks an argument as a printf format string on GCC. */
#define Nst_GNU_FMT(m, n)
#endif // !Nst_GNU_FMT

#ifndef Nst_WIN
#include <endian.h>
#endif // !endian.h

/* Represents little-endian systems. Always defined. */
#define Nst_LITTLE_ENDIAN 4321
/* Represents big-endian systems. Always defined. */
#define Nst_BIG_ENDIAN 1234

#ifndef Nst_BYTEORDER
#ifdef Nst_WIN
/**
 * @brief The endianness of the system, either `Nst_LITTLE_ENDIAN` or
 * `Nst_BIG_ENDIAN`.
 */
#define Nst_BYTEORDER Nst_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN ||                \
      defined(__BIG_ENDIAN__) || defined(__ARMEB__) ||                        \
      defined(__THUMBEB__) || defined(__AARCH64EB__) ||                       \
      defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
/** [docs:ignore]
 * @brief The endianness of the system, either `Nst_LITTLE_ENDIAN` or
 * `Nst_BIG_ENDIAN`.
 */
#define Nst_BYTEORDER Nst_BIG_ENDIAN
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN ||             \
      defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) ||                     \
      defined(__THUMBEL__) || defined(__AARCH64EL__) ||                       \
      defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
/** [docs:ignore]
 * @brief The endianness of the system, either `Nst_LITTLE_ENDIAN` or
 * `Nst_BIG_ENDIAN`.
 */
#define Nst_BYTEORDER Nst_LITTLE_ENDIAN
#else
#error Failed to determine endianness, define Nst_BYTEORDER as 4321 (LE) or 1234 (BE)
#endif // !Nst_BYTEORDER
#endif // !Nst_BYTEORDER

#ifdef Nst_WIN
/* Marks a function for for the standard C declaration (`__cdecl`). */
#define NstC __cdecl
#else
/** [docs:ignore]
 * @brief Marks a function for for the standard C declaration (`__cdecl`).
 */
#define NstC
#endif // !NstC

#ifdef Nst_UNUSED
#undef Nst_UNUSED
#endif

/**
 * @brief Marks the argument of a function as unused, without rasing any
 * compiler warnings.
 */
#define Nst_UNUSED(v) (void)(v)

#ifndef _DEBUG
#ifdef Nst_TRACK_OBJ_INIT_POS
#undef Nst_TRACK_OBJ_INIT_POS
#endif // !Nst_TRACK_OBJ_INIT_POS

#ifdef Nst_DISABLE_POOLS
#undef Nst_DISABLE_POOLS
#endif // !Nst_DISABLE_POOLS

#ifdef Nst_COUNT_ALLOC
#undef Nst_COUNT_ALLOC
#endif // !Nst_COUNT_ALLOC

#ifdef Nst_BREAKPOINT_ON_ASSERTION_FAIL
#undef Nst_BREAKPOINT_ON_ASSERTION_FAIL
#endif // !Nst_BREAKPOINT_ON_ASSERTION_FAIL
#endif

#if defined(_DEBUG) && !defined(Nst_BREAKPOINT_ON_ASSERTION_FAIL)
/**
 * @brief Aborts with an error message when an expression is false. The error
 * specifies the expression, the path and line of both the C and Nest file.
 */
#define Nst_assert(expr)                                                      \
    (void)(                                                                   \
        !!(expr)                                                              \
    ||                                                                        \
        ((void)((Nst_current_inst() && fprintf(                               \
            stderr,                                                           \
            "Assertion failed: %s (C - %s:%i, Nest - %s:%li)\n",              \
            #expr,                                                            \
            __FILE__,                                                         \
            __LINE__,                                                         \
            Nst_current_inst()->start.text->path,                             \
            Nst_current_inst()->start.line) >= 0                              \
        ) || fprintf(                                                         \
            stderr,                                                           \
            "Assertion failed: %s (C - %s:%i, Nest - <unknown>)\n",           \
            #expr,                                                            \
            __FILE__,                                                         \
            __LINE__)),                                                       \
        (abort(), 0))                                                         \
    )

/**
 * @brief Aborts with an error message when an expression is false. The error
 * specifies the expression and the path and line of the C file.
 */
#define Nst_assert_c(expr)                                                    \
    (void)(                                                                   \
        !!(expr)                                                              \
    ||                                                                        \
        ((void)fprintf(                                                       \
            stderr,                                                           \
            "Assertion failed: %s (C - %s:%i)\n",                             \
            #expr,                                                            \
            __FILE__,                                                         \
            __LINE__),                                                        \
        (abort(), 0))                                                         \
    )

#elif defined(Nst_WIN) && defined(Nst_BREAKPOINT_ON_ASSERTION_FAIL)

/** [docs:ignore]
 * @brief Aborts with an error message when an expression is false. The error
 * specifies the expression, the path and line of both the C and Nest file.
 */
#define Nst_assert(expr)                                                      \
    (void)(                                                                   \
        !!(expr)                                                              \
    ||                                                                        \
        ((void)((Nst_current_inst() && fprintf(                               \
            stderr,                                                           \
            "Assertion failed: %s (C - %s:%i, Nest - %s:%li)\n",              \
            #expr,                                                            \
            __FILE__,                                                         \
            __LINE__,                                                         \
            Nst_current_inst()->start.text->path,                             \
            Nst_current_inst()->start.line) >= 0                              \
        ) || fprintf(                                                         \
            stderr,                                                           \
            "Assertion failed: %s (C - %s:%i, Nest - <unknown>)\n",           \
            #expr,                                                            \
            __FILE__,                                                         \
            __LINE__)),                                                       \
        (__debugbreak(), 0))                                                  \
    )

/** [docs:ignore]
 * @brief Aborts with an error message when an expression is false. The error
 * specifies the expression and the path and line of the C file.
 */
#define Nst_assert_c(expr)                                                    \
    (void)(                                                                   \
        !!(expr)                                                              \
    ||                                                                        \
        ((void)fprintf(                                                       \
            stderr,                                                           \
            "Assertion failed: %s (C - %s:%i)\n",                             \
            #expr,                                                            \
            __FILE__,                                                         \
            __LINE__),                                                        \
        (__debugbreak(), 0))                                                  \
    )

#else

/** [docs:ignore]
 * @brief Aborts with an error message when an expression is false. The error
 * specifies the expression, the path and line of both the C and Nest file.
 */
#define Nst_assert(expr)

/** [docs:ignore]
 * @brief Aborts with an error message when an expression is false. The error
 * specifies the expression and the path and line of the C file.
 */
#define Nst_assert_c(expr)

#endif // !_DEBUG

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* [docs:link i8 <c_api_index.md#type-definitions>] */
/* [docs:link i16 <c_api_index.md#type-definitions>] */
/* [docs:link i32 <c_api_index.md#type-definitions>] */
/* [docs:link i64 <c_api_index.md#type-definitions>] */
/* [docs:link u8 <c_api_index.md#type-definitions>] */
/* [docs:link u16 <c_api_index.md#type-definitions>] */
/* [docs:link u32 <c_api_index.md#type-definitions>] */
/* [docs:link u64 <c_api_index.md#type-definitions>] */
/* [docs:link f32 <c_api_index.md#type-definitions>] */
/* [docs:link f64 <c_api_index.md#type-definitions>] */
/* [docs:link usize <c_api_index.md#type-definitions>] */
/* [docs:link isize <c_api_index.md#type-definitions>] */

/* `char` alias. */
NstEXP typedef char i8;
/* `short` alias. */
NstEXP typedef short i16;
/* `long int` alias. */
NstEXP typedef long i32;
/* `long long int` alias. */
NstEXP typedef long long i64;

/* `unsigned char` alias. */
NstEXP typedef unsigned char u8;
/* `unsigned short` alias. */
NstEXP typedef unsigned short u16;
/* `unsigned long` alias. */
NstEXP typedef unsigned long u32;
/* `unsigned long long` alias. */
NstEXP typedef unsigned long long u64;

/* `float` alias. */
NstEXP typedef float f32;
/* `double` alias. */
NstEXP typedef double f64;

/* `size_t` alias. */
NstEXP typedef size_t usize;
/* `ptrdiff_t` alias. */
NstEXP typedef ptrdiff_t isize;

typedef struct _Nst_Obj Nst_Obj;

/* The signature of a C function callable by Nest. */
NstEXP typedef Nst_Obj *(*Nst_NestCallable)(usize, Nst_Obj **);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !TYPEDEFS_H
