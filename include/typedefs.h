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
#include <limits.h>
#include <inttypes.h>

// DO NOT ENABLE, used for documentation purpuses. If you need to define these
// macros, define them elsewhere in this file or in the compiler.
#if 0
/**
 * If defined enables tracking of the position in the program where objects are
 * allocated. This macro should be defined in `typedefs.h` when compiling.
 *
 * @brief Note: this macro works only when the program is compiled in debug
 * mode.
 */
#define Nst_DBG_TRACK_OBJ_INIT_POS
/**
 * If defined disables object pools and instead frees the memory of each
 * object. This macro should be defined in `typedefs.h` when compiling.
 *
 * @brief Note: this macro works only when the program is compiled in debug
 * mode.
 */
#define Nst_DBG_DISABLE_POOLS
/**
 * If defined enables allocation counting and declares the
 * `Nst_log_alloc_count` function. This macro should be defined in `typedefs.h`
 * when compiling.
 *
 * @brief Note: this macro works only when the program is compiled in debug
 * mode.
 */
#define Nst_DBG_COUNT_ALLOC
/**
 * @brief Used in `Nst_assert` and `Nst_assert_c`, in debug mode is `abort()`
 * by default.
 */
#define Nst_DBG_ASSERT_CALLBACK
/**
 * @brief If defined dynamic libraries are not closed when calling `Nst_quit`
 * to allow the checking of stack traces in memory allocations after the
 * library is closed.
 */
#define Nst_DBG_KEEP_DYN_LIBS
#endif // !0

// #define Nst_DBG_TRACK_OBJ_INIT_POS
#define Nst_DBG_DISABLE_POOLS
#define Nst_DBG_COUNT_ALLOC
// #define Nst_DBG_KEEP_DYN_LIBS

#if defined(_WIN32) || defined(WIN32)

/* Defined when compiling with MSVC. */
#define Nst_MSVC

#elif defined(__clang__)

/* Defined when compiling with Clang. */
#define Nst_CLANG

#elif defined(__GNUC__)

/* Defined when compiling with GCC. */
#define Nst_GCC

#define BUILD_GGC_VER(maj, min, patch) (maj * 10000 + min * 100 + patch)
#define GCC_VER                                                           \
    BUILD_GGC_VER(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)

#else
#error Use MSVC, GCC or clang to compile.
#endif

#ifdef Nst_MSVC
#define Nst_DBG_ASSERT_CALLBACK DebugBreak()
#endif // !Nst_MSVC

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

#ifdef Nst_MSVC
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
#elif defined(Nst_MSVC)
/* [docs:ignore] Marks a function that does not finish. */
#define Nst_NORETURN __declspec(noreturn)
#elif (defined(Nst_GCC) && GCC_VER > BUILD_GGC_VER(2, 5, 0)) || defined(Nst_CLANG)
/* [docs:ignore] Marks a function that does not finish. */
#define Nst_NORETURN __attribute__((noreturn))
#else
/* [docs:ignore] Marks a function that does not finish. */
#define Nst_NORETURN
#endif // !Nst_NORETURN
#endif // !Nst_NORETURN

#ifdef Nst_GCC
#undef GGC_VER
#undef BUILD_GGC_VER
#endif // !Nst_GCC

#if !defined(Nst_WIN_FMT) && defined(Nst_MSVC)
/* Marks an argument as a printf format string on MSVC. */
#define Nst_WIN_FMT _Printf_format_string_
#else
/* [docs:ignore] Marks an argument as a printf format string on MSVC. */
#define Nst_WIN_FMT
#endif // !Nst_WIN_FMT

#if !defined(Nst_NIX_FMT) && (defined(Nst_GCC) || defined(Nst_CLANG))
/* Marks an argument as a printf format string on GCC or CLANG. */
#define Nst_NIX_FMT(m, n) __attribute__((format(printf,m,n)))
#else
/** [docs:ignore]
 * Marks an argument as a printf format string on GCC or CLANG.
 */
#define Nst_NIX_FMT(m, n)
#endif // !Nst_NIX_FMT

#ifndef Nst_MSVC
#include <endian.h>
#endif // !endian.h

/* Represents little-endian systems, check against Nst_BYTEORDER. */
#define Nst_LITTLE_ENDIAN 4321
/* Represents big-endian systems, check against Nst_BYTEORDER. */
#define Nst_BIG_ENDIAN 1234

#ifndef Nst_BYTEORDER
#ifdef Nst_MSVC
/**
 * The endianness of the system, either `Nst_LITTLE_ENDIAN` or
 * `Nst_BIG_ENDIAN`.
 */
#define Nst_BYTEORDER Nst_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN ||                \
      defined(__BIG_ENDIAN__) || defined(__ARMEB__) ||                        \
      defined(__THUMBEB__) || defined(__AARCH64EB__) ||                       \
      defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
/** [docs:ignore]
 * The endianness of the system, either `Nst_LITTLE_ENDIAN` or
 * `Nst_BIG_ENDIAN`.
 */
#define Nst_BYTEORDER Nst_BIG_ENDIAN
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN ||             \
      defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) ||                     \
      defined(__THUMBEL__) || defined(__AARCH64EL__) ||                       \
      defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
/** [docs:ignore]
 * The endianness of the system, either `Nst_LITTLE_ENDIAN` or
 * `Nst_BIG_ENDIAN`.
 */
#define Nst_BYTEORDER Nst_LITTLE_ENDIAN
#else
#error Failed to determine endianness, define Nst_BYTEORDER as Nst_LITTLE_ENDIAN or Nst_BIG_ENDIAN
#endif // !Nst_BYTEORDER
#endif // !Nst_BYTEORDER

#ifdef Nst_MSVC
/* Marks a function for for the standard C declaration (`__cdecl`). */
#define NstC __cdecl
#else
/** [docs:ignore]
 * Marks a function for for the standard C declaration (`__cdecl`).
 */
#define NstC
#endif // !NstC

#ifdef Nst_UNUSED
#undef Nst_UNUSED
#endif

/**
 * Marks the argument of a function as unused, without rasing any compiler
 * warnings.
 */
#define Nst_UNUSED(v) (void)(v)

#ifndef _DEBUG

#ifdef Nst_DBG_TRACK_OBJ_INIT_POS
#undef Nst_DBG_TRACK_OBJ_INIT_POS
#endif // !Nst_DBG_TRACK_OBJ_INIT_POS

#ifdef Nst_DBG_DISABLE_POOLS
#undef Nst_DBG_DISABLE_POOLS
#endif // !Nst_DBG_DISABLE_POOLS

#ifdef Nst_DBG_COUNT_ALLOC
#undef Nst_DBG_COUNT_ALLOC
#endif // !Nst_DBG_COUNT_ALLOC

#ifdef Nst_DBG_ASSERT_CALLBACK
#undef Nst_DBG_ASSERT_CALLBACK
#endif // !Nst_DBG_ASSERT_CALLBACK

#ifdef Nst_DBG_KEEP_DYN_LIBS
#undef Nst_DBG_KEEP_DYN_LIBS
#endif // !Nst_DBG_KEEP_DYN_LIBS

#else

#ifndef Nst_DBG_ASSERT_CALLBACK
#define Nst_DBG_ASSERT_CALLBACK abort()
#endif // !Nst_DBG_ASSERT_CALLBACK

#endif // !_DEBUG

#if defined(_DEBUG)
/**
 * Evaluates `Nst_DBG_ASSERT_CALLBACK` and prints an error message when
 * `expr` is `false`. The error specifies the expression that is false, the
 * path and line number of both the C and Nest file where the assertion failed.
 */
#define Nst_assert(expr)                                                      \
    (void)(                                                                   \
        !!(expr)                                                              \
    ||                                                                        \
        ((void)((Nst_state_span().text != NULL && fprintf(                    \
            stderr,                                                           \
            "Assertion failed: %s (C - %s:%i, Nest - %s:%" PRIi32 ")\n",      \
            #expr,                                                            \
            __FILE__,                                                         \
            __LINE__,                                                         \
            Nst_state_span().text->path,                                      \
            Nst_state_span().start_line + 1) >= 0                             \
        ) || fprintf(                                                         \
            stderr,                                                           \
            "Assertion failed: %s (C - %s:%i, Nest - <unknown>)\n",           \
            #expr,                                                            \
            __FILE__,                                                         \
            __LINE__)),                                                       \
        (Nst_DBG_ASSERT_CALLBACK, 0))                                         \
    )

/**
 * Evaluates `Nst_DBG_ASSERT_CALLBACK` and prints an error message when
 * `expr` is `false`. The error specifies the expression that is false, the
 * path and line number of the C file where.
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
        (Nst_DBG_ASSERT_CALLBACK, 0))                                         \
    )

#else

/** [docs:ignore]
 * Evaluates `Nst_DBG_ASSERT_CALLBACK` and prints an error message when
 * `expr` is `false`. The error specifies the expression that is false, the
 * path and line number of both the C and Nest file where the assertion failed.
 */
#define Nst_assert(expr)

/** [docs:ignore]
 * Evaluates `Nst_DBG_ASSERT_CALLBACK` and prints an error message when
 * `expr` is `false`. The error specifies the expression that is false, the
 * path and line number of the C file where.
 */
#define Nst_assert_c(expr)

#endif // !_DEBUG

#ifdef Nst_MSVC
/* Marks an execution path as unreachable. */
#define Nst_UNREACHABLE __assume(0)
/* Marks a condition as likely to be true. */
#define Nst_LIKELY(expr) (expr)
/* Marks a condition as likely to be false. */
#define Nst_UNLIKELY(expr) (expr)
#else
/* [docs:ignore] Marks an execution path as unreachable. */
#define Nst_UNREACHABLE __builtin_unreachable()
/* [docs:ignore] Marks a condition as likely to be true. */
#define Nst_LIKELY(expr)  __builtin_expect(!!(expr), 1)
/* [docs:ignore] Marks a condition as likely to be false. */
#define Nst_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#endif // !Nst_MSVC

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

/* 8-bit signed integer. */
NstEXP typedef int8_t i8;
/* 16-bit signed integer. */
NstEXP typedef int16_t i16;
/* 32-bit signed integer. */
NstEXP typedef int32_t i32;
/* 64-bit signed integer. */
NstEXP typedef int64_t i64;

/* 8-bit unsigned integer. */
NstEXP typedef uint8_t u8;
/* 16-bit unsigned integer. */
NstEXP typedef uint16_t u16;
/* `unsigned int` alias. */
NstEXP typedef unsigned int uint;
/* 32-bit unsigned integer. */
NstEXP typedef uint32_t u32;
/* 64-bit unsigned integer. */
NstEXP typedef uint64_t u64;

/* `float` alias. */
NstEXP typedef float f32;
/* `double` alias. */
NstEXP typedef double f64;

/* `size_t` alias. */
NstEXP typedef size_t usize;
/* `ptrdiff_t` alias. */
NstEXP typedef ptrdiff_t isize;

/**
 * The structure representing a basic Nest object.
 *
 * @param ref_count: the reference count of the object
 * @param type: the type of the object
 * @param p_next: the next object in the type's pool
 * @param hash: the hash of the object, `-1` if it has not yet been hashed or
 * is not hashable
 * @param flags: the flags of the object
 * @param init_line: **this field only exists when `Nst_DBG_TRACK_OBJ_INIT_POS`
 * is defined** - the line of the instruction that initialized the object
 * @param init_col: **this field only exists when `Nst_DBG_TRACK_OBJ_INIT_POS`
 * is defined** - the column of the instruction that initialized the object
 * @param init_path: **this field only exists when `Nst_DBG_TRACK_OBJ_INIT_POS`
 * is defined** - the path to the file where the object was initialized
 */
NstEXP typedef struct _Nst_Obj {
    struct _Nst_Obj *type;
    struct _Nst_Obj *p_next;
    isize ref_count;
    i32 hash;
    u32 flags;
#ifdef Nst_DBG_TRACK_OBJ_INIT_POS
    i32 init_line;
    i32 init_col;
    char *init_path;
#endif // !Nst_DBG_TRACK_OBJ_INIT_POS
} Nst_Obj;

/**
 * Using `Nst_ObjRef *` instead of `Nst_Obj *` signals that an object reference
 * is being passed or owned. Depending on context it has different meanings:
 *! When used as the type of a function argument it signals that a reference is
 * taken from the argument itself.
 *! When used as the type of the return value of a function it signals that the
 * function returns a new reference to the object.
 *! When used in a struct it signals that the struct owns a reference to the
 * object.
 */
NstEXP typedef Nst_Obj Nst_ObjRef;

/* The signature of a C function callable by Nest. */
NstEXP typedef Nst_ObjRef *(*Nst_NestCallable)(usize, Nst_Obj **);
/* The signature of a generic destructor. */
NstEXP typedef void (*Nst_Destructor)(void *);

#if UCHAR_MAX != 255
#error sizeof(char) must be equal to 1
#endif // !UCHAR_MAX

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !TYPEDEFS_H
