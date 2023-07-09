#ifndef ITUTIL_FUNCTIONS_H
#define ITUTIL_FUNCTIONS_H

#include "nest.h"

Nst_FUNC_SIGN(count_start);
Nst_FUNC_SIGN(count_is_done);
Nst_FUNC_SIGN(count_get_val);

Nst_FUNC_SIGN(cycle_start);
Nst_FUNC_SIGN(cycle_is_done);
Nst_FUNC_SIGN(cycle_get_val);

Nst_FUNC_SIGN(repeat_start);
Nst_FUNC_SIGN(repeat_is_done);
Nst_FUNC_SIGN(repeat_get_val);

Nst_FUNC_SIGN(chain_start);
Nst_FUNC_SIGN(chain_is_done);
Nst_FUNC_SIGN(chain_get_val);

Nst_FUNC_SIGN(zip_start);
Nst_FUNC_SIGN(zip_is_done);
Nst_FUNC_SIGN(zip_get_val);

Nst_FUNC_SIGN(zipn_start);
Nst_FUNC_SIGN(zipn_is_done);
Nst_FUNC_SIGN(zipn_get_val);

Nst_FUNC_SIGN(enumerate_start);
Nst_FUNC_SIGN(enumerate_is_done);
Nst_FUNC_SIGN(enumerate_get_val);

Nst_FUNC_SIGN(kvi_start);
Nst_FUNC_SIGN(kvi_is_done);
Nst_FUNC_SIGN(keys_get_val);
Nst_FUNC_SIGN(values_get_val);
Nst_FUNC_SIGN(items_get_val);

Nst_FUNC_SIGN(reversed_start);
Nst_FUNC_SIGN(reversed_is_done);
Nst_FUNC_SIGN(reversed_get_val);

#endif // !ITUTIL_FUNCTIONS_H
