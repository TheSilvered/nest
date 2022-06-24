#ifndef LIB_TEST_H
#define LIB_TEST_H

#include "nest_source/nest_include.h"

#ifdef LIBTEST_EXPORTS
#define LIB_TEST_API __declspec(dllexport)
#else
#define LIB_TEST_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

LIB_TEST_API bool lib_init();
LIB_TEST_API FuncDeclr *get_func_ptrs();
LIB_TEST_API INIT_LIB_OBJ_FUNC;

// Here you can put your function signatures
// They must always be `Nst_Obj *func_name(size_t arg_num, Nst_Obj **args, OpErr *err);`
// replace func_name with your function's name

// To compile remove precompiled headers in Project->Properties->C/C++->
// ->Precompiled headers and set "Precompiled header" to "Not Using Precompiled
// Headers"

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_TEST_H