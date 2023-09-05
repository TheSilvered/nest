#ifndef VEC_LIB_H
#define VEC_LIB_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Vec2Obj
{
    Nst_OBJ_HEAD;
    f64 x, y;
}
Vec2Obj;

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();
NstEXP void NstC free_lib();

Nst_Obj *vec2_new(f64 x, f64 y);

Nst_FUNC_SIGN(vec2_);
Nst_FUNC_SIGN(vec2_to_str_);
Nst_FUNC_SIGN(x_);
Nst_FUNC_SIGN(y_);
Nst_FUNC_SIGN(len_);
Nst_FUNC_SIGN(dot_);
Nst_FUNC_SIGN(cross_);
Nst_FUNC_SIGN(normalized_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VEC_LIB_H
