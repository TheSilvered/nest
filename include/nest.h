/* All necessary header files */

#ifndef NEST_H
#define NEST_H

#define _NST_VERSION_BASE "beta-0.13.0"

#include <stdint.h>

#if (INTPTR_MAX == INT64_MAX) || defined(_NST_ARCH64)
  #define NST_VERSION _NST_VERSION_BASE " x64"
#elif INTPTR_MAX == INT32_MAX || defined(_NST_ARCH86)
  #define NST_VERSION _NST_VERSION_BASE " x86"
#else
  #error Failed to determine architecture, define _NST_ARCH64 or _NST_ARCH86
#endif

#include "iter.h"
#include "hash.h"
#include "obj_ops.h"
#include "tokens.h"
#include "lexer.h"
#include "parser.h"
#include "optimizer.h"
#include "encoding.h"
#include "argv_parser.h"
#include "error_internal.h"
#include "mem.h"
#include "format.h"

#endif //!NEST_H
