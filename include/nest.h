/* All necessary header files */

#ifndef NEST_H
#define NEST_H

#define _NST_VERSION_BASE "beta-0.12.1"

#if defined(_WIN32) || defined(WIN32)
  #ifdef _WIN64
    #define NST_VERSION _NST_VERSION_BASE " x64"
  #else
    #define NST_VERSION _NST_VERSION_BASE " x86"
  #endif
#elif defined(__GNUC__)
  #if defined(__x86_64__) || defined(__ppc64__)
    #define NST_VERSION _NST_VERSION_BASE " x64"
  #else
    #define NST_VERSION _NST_VERSION_BASE " x86"
  #endif
#else
  #define NST_VERSION _NST_VERSION_BASE " x??"
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
