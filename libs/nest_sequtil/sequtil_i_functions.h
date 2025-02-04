#ifndef SEQUTIL_I_FUNCTIONS_H
#define SEQUTIL_I_FUNCTIONS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif

Nst_Obj *NstC map_i_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC map_i_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC slice_i_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC slice_i_seq_next(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC slice_i_str_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC filter_i_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC filter_i_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC reverse_i_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC reverse_i_next(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif

#endif // !SEQUTIL_I_FUNCTIONS_H
