#ifndef NEST_CODECS_H
#define NEST_CODECS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();

Nst_FUNC_SIGN(get_len_);
Nst_FUNC_SIGN(get_at_);
Nst_FUNC_SIGN(to_iter_);
Nst_FUNC_SIGN(from_cp_);
Nst_FUNC_SIGN(to_cp_);
Nst_FUNC_SIGN(cp_is_valid_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_CODECS_H
