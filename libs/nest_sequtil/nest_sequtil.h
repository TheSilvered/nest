#ifndef NEST_SEQUTIL_H
#define NEST_SEQUTIL_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();

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
NST_FUNC_SIGN(count_);
NST_FUNC_SIGN(lscan_);
NST_FUNC_SIGN(rscan_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SEQUTIL_H