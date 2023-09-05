#ifndef NEST_SEQUTIL_H
#define NEST_SEQUTIL_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();

Nst_FUNC_SIGN(map_);
Nst_FUNC_SIGN(insert_at_);
Nst_FUNC_SIGN(remove_at_);
Nst_FUNC_SIGN(slice_);
Nst_FUNC_SIGN(merge_);
Nst_FUNC_SIGN(sort_);
Nst_FUNC_SIGN(empty_);
Nst_FUNC_SIGN(filter_);
Nst_FUNC_SIGN(contains_);
Nst_FUNC_SIGN(any_);
Nst_FUNC_SIGN(all_);
Nst_FUNC_SIGN(count_);
Nst_FUNC_SIGN(lscan_);
Nst_FUNC_SIGN(rscan_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SEQUTIL_H