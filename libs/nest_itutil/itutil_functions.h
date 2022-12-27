#ifndef ITUTIL_FUNCTIONS_H
#define ITUTIL_FUNCTIONS_H

#include "nest.h"

NST_FUNC_SIGN(count_start);
NST_FUNC_SIGN(count_advance);
NST_FUNC_SIGN(count_is_done);
NST_FUNC_SIGN(count_get_val);

NST_FUNC_SIGN(cycle_start);
NST_FUNC_SIGN(cycle_advance);
NST_FUNC_SIGN(cycle_is_done);
NST_FUNC_SIGN(cycle_get_val);

NST_FUNC_SIGN(repeat_start);
NST_FUNC_SIGN(repeat_advance);
NST_FUNC_SIGN(repeat_is_done);
NST_FUNC_SIGN(repeat_get_val);

NST_FUNC_SIGN(chain_start);
NST_FUNC_SIGN(chain_advance);
NST_FUNC_SIGN(chain_is_done);
NST_FUNC_SIGN(chain_get_val);

NST_FUNC_SIGN(zip_start);
NST_FUNC_SIGN(zip_advance);
NST_FUNC_SIGN(zip_is_done);
NST_FUNC_SIGN(zip_get_val);

NST_FUNC_SIGN(zipn_start);
NST_FUNC_SIGN(zipn_advance);
NST_FUNC_SIGN(zipn_is_done);
NST_FUNC_SIGN(zipn_get_val);

NST_FUNC_SIGN(enumerate_start);
NST_FUNC_SIGN(enumerate_advance);
NST_FUNC_SIGN(enumerate_is_done);
NST_FUNC_SIGN(enumerate_get_val);

NST_FUNC_SIGN(kvi_start);
NST_FUNC_SIGN(kvi_advance);
NST_FUNC_SIGN(kvi_is_done);
NST_FUNC_SIGN(keys_get_val);
NST_FUNC_SIGN(values_get_val);
NST_FUNC_SIGN(items_get_val);

NST_FUNC_SIGN(reversed_start);
NST_FUNC_SIGN(reversed_advance);
NST_FUNC_SIGN(reversed_is_done);
NST_FUNC_SIGN(reversed_get_val);

#endif // !ITUTIL_FUNCTIONS_H
