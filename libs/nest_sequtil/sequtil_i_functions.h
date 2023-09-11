#ifndef SEQUTIL_I_FUNCTIONS_H
#define SEQUTIL_I_FUNCTIONS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif

Nst_FUNC_SIGN(map_i_start);
Nst_FUNC_SIGN(map_i_get_val);

Nst_FUNC_SIGN(slice_i_start);
Nst_FUNC_SIGN(slice_i_seq_get_val);
Nst_FUNC_SIGN(slice_i_str_get_val);

Nst_FUNC_SIGN(filter_i_start);
Nst_FUNC_SIGN(filter_i_get_val);

Nst_FUNC_SIGN(lscan_i_start);
Nst_FUNC_SIGN(lscan_i_get_val);

Nst_FUNC_SIGN(rscan_i_start);
Nst_FUNC_SIGN(rscan_i_get_val);

#ifdef __cplusplus
}
#endif

#endif // !SEQUTIL_I_FUNCTIONS_H
