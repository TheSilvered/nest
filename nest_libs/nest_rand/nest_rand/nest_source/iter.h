#ifndef ITER_H
#define ITER_H

#include "error.h"
#include "obj.h"
#include "function.h"

#define AS_ITER(ptr) ((Nst_IterObj *)(ptr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    OBJ_HEAD;
    Nst_FuncObj *start;
    Nst_FuncObj *advance;
    Nst_FuncObj *is_done;
    Nst_FuncObj *get_val;
    Nst_Obj *value;
}
Nst_IterObj;

Nst_Obj *new_iter(
    Nst_FuncObj *start,
    Nst_FuncObj *advance,
    Nst_FuncObj *is_done,
    Nst_FuncObj *get_val,
    Nst_Obj *value);
void destroy_iter(Nst_IterObj *iter);

Nst_Obj *num_iter_start(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *num_iter_advance(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *num_iter_is_done(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *num_iter_get_val(size_t arg_num, Nst_Obj **args, OpErr *err);

Nst_Obj *seq_iter_start(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *seq_iter_advance(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *seq_iter_is_done(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *seq_iter_get_val(size_t arg_num, Nst_Obj **args, OpErr *err);

Nst_Obj *str_iter_start(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *str_iter_advance(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *str_iter_is_done(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *str_iter_get_val(size_t arg_num, Nst_Obj **args, OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // ITER_H