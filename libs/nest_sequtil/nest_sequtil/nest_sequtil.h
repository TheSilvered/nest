#ifndef NEST_SEQUTIL_H
#define NEST_SEQUTIL_H

#include "../../../nest/nest.h"

#if defined(_WIN32) || defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_FuncDeclr *get_func_ptrs();
EXPORT NST_INIT_LIB_OBJ_FUNC;

NST_FUNC_SIGN(map_);
NST_FUNC_SIGN(insert_at_);
NST_FUNC_SIGN(remove_at_);
NST_FUNC_SIGN(slice_);
NST_FUNC_SIGN(merge_);
NST_FUNC_SIGN(sort_);
NST_FUNC_SIGN(empty_);
NST_FUNC_SIGN(filter_);
NST_FUNC_SIGN(contains_);
NST_FUNC_SIGN(any_);
NST_FUNC_SIGN(all_);
NST_FUNC_SIGN(from_iter_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SEQUTIL_H