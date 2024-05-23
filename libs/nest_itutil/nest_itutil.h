#ifndef NEST_ITUTIL_H
#define NEST_ITUTIL_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();
NstEXP void NstC free_lib();

Nst_FUNC_SIGN(count_);
Nst_FUNC_SIGN(cycle_);
Nst_FUNC_SIGN(repeat_);
Nst_FUNC_SIGN(chain_);
Nst_FUNC_SIGN(zip_);
Nst_FUNC_SIGN(enumerate_);
Nst_FUNC_SIGN(keys_);
Nst_FUNC_SIGN(values_);
Nst_FUNC_SIGN(reversed_);
Nst_FUNC_SIGN(batch_);
Nst_FUNC_SIGN(batch_padded_);
Nst_FUNC_SIGN(new_iterator_);
Nst_FUNC_SIGN(iter_start_);
Nst_FUNC_SIGN(iter_get_val_);
Nst_FUNC_SIGN(_get_iend_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_ITUTIL_H