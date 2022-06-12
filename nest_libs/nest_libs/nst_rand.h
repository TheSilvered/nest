#pragma once

#include "../../nest/nest_include.h"
#include "../../nest/str.h"
#include "../../nest/simple_types.h"

#ifdef NST_RAND_EXPORTS
#define NST_RAND_API __declspec(dllexport)
#else
#define NST_RAND_API __declspec(dllimport)
#endif

extern "C" NST_RAND_API bool lib_init();
extern "C" NST_RAND_API FuncDeclr *get_func_ptrs();
extern "C" NST_RAND_API Nst_Obj *randint(size_t arg_num, Nst_Obj **args, OpErr *err);
