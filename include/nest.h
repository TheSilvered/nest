/* All necessary header files */

#ifndef NEST_H
#define NEST_H

#define _NST_VERSION_BASE "beta-0.11.2"

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
  #define NST_VERSION _NST_VERSION_BASE
#endif

#include "error.h"
#include "hash.h"
#include "interpreter.h"
#include "lib_import.h"
#include "llist.h"
#include "nst_types.h"
#include "obj_ops.h"
#include "global_consts.h"
#include "tokens.h"
#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "optimizer.h"
#include "encoding.h"

#endif //!NEST_H
