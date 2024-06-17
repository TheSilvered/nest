#ifndef NEST_SEQUTIL_H
#define NEST_SEQUTIL_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();

Nst_Obj *NstC map_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC map_i_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC insert_at_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC remove_at_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC slice_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC slice_i_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC merge_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC extend_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC sort_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC empty_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC filter_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC filter_i_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC contains_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC any_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC all_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC count_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC lscan_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rscan_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC copy_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC deep_copy_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC enum_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC reverse_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC reverse_i_(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SEQUTIL_H