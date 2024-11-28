#ifndef NEST_CODECS_H
#define NEST_CODECS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();

Nst_Obj *NstC from_cp_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC to_cp_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cp_is_valid_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC encoding_info_(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_CODECS_H
