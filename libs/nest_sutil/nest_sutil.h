#ifndef NEST_SUTIL_H
#define NEST_SUTIL_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool lib_init();
NstEXP Nst_DeclrList *get_func_ptrs();

Nst_FUNC_SIGN(lfind_);
Nst_FUNC_SIGN(rfind_);
Nst_FUNC_SIGN(starts_with_);
Nst_FUNC_SIGN(ends_with_);
Nst_FUNC_SIGN(trim_);
Nst_FUNC_SIGN(ltrim_);
Nst_FUNC_SIGN(rtrim_);
Nst_FUNC_SIGN(ljust_);
Nst_FUNC_SIGN(rjust_);
Nst_FUNC_SIGN(center_);
Nst_FUNC_SIGN(to_title_);
Nst_FUNC_SIGN(to_upper_);
Nst_FUNC_SIGN(to_lower_);
Nst_FUNC_SIGN(is_title_);
Nst_FUNC_SIGN(is_upper_);
Nst_FUNC_SIGN(is_lower_);
Nst_FUNC_SIGN(is_alpha_);
Nst_FUNC_SIGN(is_digit_);
Nst_FUNC_SIGN(is_alnum_);
Nst_FUNC_SIGN(is_charset_);
Nst_FUNC_SIGN(is_printable_);
Nst_FUNC_SIGN(replace_substr_);
Nst_FUNC_SIGN(bytearray_to_str_);
Nst_FUNC_SIGN(str_to_bytearray_);
Nst_FUNC_SIGN(repr_);
Nst_FUNC_SIGN(join_);
Nst_FUNC_SIGN(split_);
Nst_FUNC_SIGN(bin_);
Nst_FUNC_SIGN(oct_);
Nst_FUNC_SIGN(hex_);
Nst_FUNC_SIGN(parse_int_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SUTIL_H