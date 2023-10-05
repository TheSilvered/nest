#ifndef NEST_SUTIL_H
#define NEST_SUTIL_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();

Nst_FUNC_SIGN(lfind_);
Nst_FUNC_SIGN(rfind_);
Nst_FUNC_SIGN(starts_with_);
Nst_FUNC_SIGN(ends_with_);
Nst_FUNC_SIGN(trim_);
Nst_FUNC_SIGN(ltrim_);
Nst_FUNC_SIGN(rtrim_);
Nst_FUNC_SIGN(justify_);
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
Nst_FUNC_SIGN(replace_);
Nst_FUNC_SIGN(decode_);
Nst_FUNC_SIGN(encode_);
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