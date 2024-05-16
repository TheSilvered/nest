#ifndef NEST_SYS_H
#define NEST_SYS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();
NstEXP void NstC free_lib();

Nst_FUNC_SIGN(system_);
Nst_FUNC_SIGN(exit_);
Nst_FUNC_SIGN(get_env_);
Nst_FUNC_SIGN(set_env_);
Nst_FUNC_SIGN(del_env_);
Nst_FUNC_SIGN(get_ref_count_);
Nst_FUNC_SIGN(get_addr_);
Nst_FUNC_SIGN(hash_);
Nst_FUNC_SIGN(_get_color_);
Nst_FUNC_SIGN(_set_cwd_);
Nst_FUNC_SIGN(_get_cwd_);
Nst_FUNC_SIGN(_get_endianness_);
Nst_FUNC_SIGN(_get_version_);
Nst_FUNC_SIGN(_get_platform_);
Nst_FUNC_SIGN(_raw_exit);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_SYS_H
