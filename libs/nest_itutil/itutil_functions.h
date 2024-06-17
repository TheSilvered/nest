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

Nst_Obj *NstC count_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC count_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC cycle_str_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_str_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC cycle_seq_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_seq_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC cycle_iter_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_iter_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC repeat_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC repeat_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC chain_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC chain_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC zip_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC zip_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC zipn_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC zipn_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC enumerate_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC enumerate_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC keys_get_val(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC values_get_val(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC batch_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC batch_get_val(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC batch_padded_get_val(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ITUTIL_FUNCTIONS_H
