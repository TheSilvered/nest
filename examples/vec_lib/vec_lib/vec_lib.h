#ifndef VEC_LIB_H
#define VEC_LIB_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();
NstEXP void NstC lib_quit();

typedef struct _Vec2Obj {
    Nst_OBJ_HEAD;
    f64 x, y;
} Vec2Obj;

Nst_Obj *vec2_new(f64 x, f64 y);

Nst_Obj *NstC vec2_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC vec2_to_str_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC x_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC y_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC len_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC dot_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cross_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC normalized_(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VEC_LIB_H
