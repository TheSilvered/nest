#ifndef NEST_SUTIL_H
#define NEST_SUTIL_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();

Nst_Obj *NstC lfind_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rfind_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC starts_with_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC ends_with_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC trim_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC ltrim_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rtrim_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC ljust_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rjust_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cjust_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC to_title_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC to_upper_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC to_lower_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_title_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_upper_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_lower_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_alpha_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_digit_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_decimal_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_numeric_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_alnum_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_space_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_charset_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_printable_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC is_ascii_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC replace_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC decode_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC encode_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC repr_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC join_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC lsplit_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rsplit_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC bin_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC oct_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC hex_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC parse_int_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC consume_int_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC parse_real_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC consume_real_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC lremove_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC rremove_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC fmt_(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SUTIL_H