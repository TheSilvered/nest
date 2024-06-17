#ifndef NEST_RAND_H
#define NEST_RAND_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();

Nst_Obj *NstC random_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rand_int_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rand_perc_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC choice_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC shuffle_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC seed_(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_RAND_H
