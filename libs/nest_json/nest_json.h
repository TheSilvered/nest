#ifndef NEST_JSON_H
#define NEST_JSON_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();

Nst_FUNC_SIGN(load_s_);
Nst_FUNC_SIGN(load_f_);
Nst_FUNC_SIGN(dump_s_);
Nst_FUNC_SIGN(dump_f_);
Nst_FUNC_SIGN(set_option_);
Nst_FUNC_SIGN(get_option_);
Nst_FUNC_SIGN(clear_options_);

typedef enum _JSONOptions {
    JSON_OPT_COMMENTS,
    JSON_OPT_TRAILING_COMMAS,
    JSON_OPT_NAN_AND_INF
} JSONOptions;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_JSON_H