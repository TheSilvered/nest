#ifndef ITUTIL_FUNCTIONS_H
#define ITUTIL_FUNCTIONS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _ItutilFunctions {
    Nst_Obj *count_start;
    Nst_Obj *count_next;
    Nst_Obj *cycle_str_start;
    Nst_Obj *cycle_str_next;
    Nst_Obj *cycle_seq_start;
    Nst_Obj *cycle_seq_next;
    Nst_Obj *cycle_iter_start;
    Nst_Obj *cycle_iter_next;
    Nst_Obj *repeat_start;
    Nst_Obj *repeat_next;
    Nst_Obj *chain_start;
    Nst_Obj *chain_next;
    Nst_Obj *zip_start;
    Nst_Obj *zip_next;
    Nst_Obj *zipn_start;
    Nst_Obj *zipn_next;
    Nst_Obj *enumerate_start;
    Nst_Obj *enumerate_next;
    Nst_Obj *keys_next;
    Nst_Obj *values_next;
    Nst_Obj *batch_start;
    Nst_Obj *batch_next;
    Nst_Obj *batch_padded_next;
} ItutilFunctions;

extern ItutilFunctions itutil_functions;

bool init_itutil_functions();
void free_itutil_functions();

Nst_Obj *NstC count_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC count_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC cycle_str_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_str_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC cycle_seq_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_seq_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC cycle_iter_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_iter_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC repeat_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC repeat_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC chain_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC chain_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC zip_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC zip_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC zipn_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC zipn_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC enumerate_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC enumerate_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC keys_next(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC values_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC batch_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC batch_next(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC batch_padded_next(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ITUTIL_FUNCTIONS_H
