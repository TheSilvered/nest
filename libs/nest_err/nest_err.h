#ifndef NEST_ERR_H
#define NEST_ERR_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();

Nst_Obj *NstC try_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC SYNTAX_ERROR_();
Nst_Obj *NstC VALUE_ERROR_();
Nst_Obj *NstC TYPE_ERROR_();
Nst_Obj *NstC CALL_ERROR_();
Nst_Obj *NstC MEMORY_ERROR_();
Nst_Obj *NstC MATH_ERROR_();
Nst_Obj *NstC IMPORT_ERROR_();

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_ERR_H