#ifndef VEC_LIB_H
#define VEC_LIB_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Vec2Obj
{
    NST_OBJ_HEAD;
    f64 x, y;
}
Vec2Obj;

EXPORT bool lib_init();
EXPORT Nst_DeclrList *get_func_ptrs();
EXPORT void free_lib();

Nst_Obj *vec2_new(f64 x, f64 y, Nst_OpErr *err);

NST_FUNC_SIGN(vec2_);
NST_FUNC_SIGN(vec2_to_str_);
NST_FUNC_SIGN(x_);
NST_FUNC_SIGN(y_);
NST_FUNC_SIGN(len_);
NST_FUNC_SIGN(dot_);
NST_FUNC_SIGN(cross_);
NST_FUNC_SIGN(normalized_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VEC_LIB_H
