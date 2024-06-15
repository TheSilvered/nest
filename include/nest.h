/**
 * @file nest.h
 *
 * @brief Header files and Nst_VERSION definition
 *
 * @author TheSilvered
 */

#ifndef NEST_H
#define NEST_H

/* The major version of Nest. */
#define Nst_VERSION_MAJOR 0
/* The minor version of Nest. */
#define Nst_VERSION_MINOR 15
/* The patch version of Nest. */
#define Nst_VERSION_PATCH 0

#define __Nst_TO_STR(a) __Nst_TO_STR_(a)
#define __Nst_TO_STR_(b) #b

#define _Nst_VERSION_BASE                                                     \
    "beta-" __Nst_TO_STR(Nst_VERSION_MAJOR) "."                               \
            __Nst_TO_STR(Nst_VERSION_MINOR) "."                               \
            __Nst_TO_STR(Nst_VERSION_PATCH)

#include "typedefs.h"

#ifdef _Nst_ARCH_x64
/* The current version of Nest as a string. */
#define Nst_VERSION _Nst_VERSION_BASE " x64"
#elif defined(_Nst_ARCH_x86)
/* [docs:ignore] The current version of Nest as a string. */
#define Nst_VERSION _Nst_VERSION_BASE " x86"
#else
#error Failed to determine architecture, define _Nst_ARCH_x64 or _Nst_ARCH_x86
#endif // !Nst_VERSION

#include "iter.h"
#include "hash.h"
#include "obj_ops.h"
#include "tokens.h"
#include "lexer.h"
#include "parser.h"
#include "optimizer.h"
#include "encoding.h"
#include "argv_parser.h"
#include "mem.h"
#include "format.h"
#include "file.h"

#endif // !NEST_H
