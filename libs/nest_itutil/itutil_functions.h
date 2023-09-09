#ifndef ITUTIL_FUNCTIONS_H
#define ITUTIL_FUNCTIONS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

Nst_FUNC_SIGN(count_start);
Nst_FUNC_SIGN(count_get_val);

Nst_FUNC_SIGN(cycle_str_start);
Nst_FUNC_SIGN(cycle_str_get_val);

Nst_FUNC_SIGN(cycle_seq_start);
Nst_FUNC_SIGN(cycle_seq_get_val);

Nst_FUNC_SIGN(cycle_iter_start);
Nst_FUNC_SIGN(cycle_iter_get_val);

Nst_FUNC_SIGN(repeat_start);
Nst_FUNC_SIGN(repeat_get_val);

Nst_FUNC_SIGN(chain_start);
Nst_FUNC_SIGN(chain_get_val);

Nst_FUNC_SIGN(zip_start);
Nst_FUNC_SIGN(zip_get_val);

Nst_FUNC_SIGN(zipn_start);
Nst_FUNC_SIGN(zipn_get_val);

Nst_FUNC_SIGN(enumerate_start);
Nst_FUNC_SIGN(enumerate_get_val);

Nst_FUNC_SIGN(keys_get_val);
Nst_FUNC_SIGN(values_get_val);

Nst_FUNC_SIGN(reversed_start);
Nst_FUNC_SIGN(reversed_get_val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ITUTIL_FUNCTIONS_H
