#ifndef NEST_SEQUTIL_H
#define NEST_SEQUTIL_H

#include "../../../nest/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) Nst_FuncDeclr *get_func_ptrs();
__declspec(dllexport) NST_INIT_LIB_OBJ_FUNC;

NST_FUNC_SIGN(map_);
NST_FUNC_SIGN(insert_at_);
NST_FUNC_SIGN(remove_at_);
NST_FUNC_SIGN(slice_);
NST_FUNC_SIGN(merge_);
NST_FUNC_SIGN(sort_);
NST_FUNC_SIGN(empty_);
NST_FUNC_SIGN(any_);
NST_FUNC_SIGN(all_);
NST_FUNC_SIGN(from_iter_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SEQUTIL_H