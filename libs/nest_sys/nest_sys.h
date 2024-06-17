#ifndef NEST_SYS_H
#define NEST_SYS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();
NstEXP void NstC lib_quit();

Nst_Obj *NstC system_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC exit_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC get_env_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC set_env_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC del_env_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC get_ref_count_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC get_addr_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC get_capacity_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC hash_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC set_cwd_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC get_cwd_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _get_color_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _get_endianness_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _get_version_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _get_platform_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _raw_exit(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC _DEBUG_();

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SYS_H
