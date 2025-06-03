#ifndef NEST_ITUTIL_H
#define NEST_ITUTIL_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();
NstEXP void NstC lib_quit();

Nst_Obj *NstC count_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC repeat_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC chain_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC zip_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC enumerate_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC keys_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC values_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC batch_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC batch_padded_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC new_iterator_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC iter_start_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC iter_next_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC IEND_();

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_ITUTIL_H