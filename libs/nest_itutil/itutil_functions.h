#ifndef ITUTIL_FUNCTIONS_H
#define ITUTIL_FUNCTIONS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _ItutilFunctions {
    Nst_FuncObj *count_start;
    Nst_FuncObj *count_get_val;
    Nst_FuncObj *cycle_str_start;
    Nst_FuncObj *cycle_str_get_val;
    Nst_FuncObj *cycle_seq_start;
    Nst_FuncObj *cycle_seq_get_val;
    Nst_FuncObj *cycle_iter_start;
    Nst_FuncObj *cycle_iter_get_val;
    Nst_FuncObj *repeat_start;
    Nst_FuncObj *repeat_get_val;
    Nst_FuncObj *chain_start;
    Nst_FuncObj *chain_get_val;
    Nst_FuncObj *zip_start;
    Nst_FuncObj *zip_get_val;
    Nst_FuncObj *zipn_start;
    Nst_FuncObj *zipn_get_val;
    Nst_FuncObj *enumerate_start;
    Nst_FuncObj *enumerate_get_val;
    Nst_FuncObj *keys_get_val;
    Nst_FuncObj *values_get_val;
    Nst_FuncObj *batch_start;
    Nst_FuncObj *batch_get_val;
    Nst_FuncObj *batch_padded_get_val;
} ItutilFunctions;

extern ItutilFunctions itutil_functions;

bool init_itutil_functions();
void free_itutil_functions();

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

Nst_FUNC_SIGN(batch_start);
Nst_FUNC_SIGN(batch_get_val);
Nst_FUNC_SIGN(batch_padded_get_val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ITUTIL_FUNCTIONS_H
