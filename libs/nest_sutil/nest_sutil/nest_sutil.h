#ifndef NEST_SUTIL_H
#define NEST_SUTIL_H

#include "../../../nest/nest.h"

#if defined(_WIN32) || defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_FuncDeclr *get_func_ptrs();
EXPORT NST_INIT_LIB_OBJ_FUNC;

NST_FUNC_SIGN(lfind_);
NST_FUNC_SIGN(rfind_);
NST_FUNC_SIGN(trim_);
NST_FUNC_SIGN(ltrim_);
NST_FUNC_SIGN(rtrim_);
NST_FUNC_SIGN(ljust_);
NST_FUNC_SIGN(rjust_);
NST_FUNC_SIGN(to_upper_);
NST_FUNC_SIGN(to_lower_);
NST_FUNC_SIGN(is_upper_);
NST_FUNC_SIGN(is_lower_);
NST_FUNC_SIGN(is_alpha_);
NST_FUNC_SIGN(is_digit_);
NST_FUNC_SIGN(is_alnum_);
NST_FUNC_SIGN(is_charset_);
NST_FUNC_SIGN(is_printable_);
NST_FUNC_SIGN(replace_substr_);
NST_FUNC_SIGN(bytearray_to_str_);
NST_FUNC_SIGN(repr_);
NST_FUNC_SIGN(join_);
NST_FUNC_SIGN(split_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SUTIL_H