#ifndef NEST_JSON_H
#define NEST_JSON_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();

Nst_Obj *NstC load_s_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC load_f_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC dump_s_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC dump_f_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC set_option_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC get_option_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC clear_options_(usize arg_num, Nst_Obj **args);

typedef enum _JSONOptions {
    JSON_OPT_COMMENTS,
    JSON_OPT_TRAILING_COMMAS,
    JSON_OPT_NAN_AND_INF
} JSONOptions;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_JSON_H