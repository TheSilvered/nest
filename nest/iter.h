#ifndef ITER_H
#define ITER_H

#include "error.h"
#include "obj.h"

#define AS_ITER(ptr) ((Nst_iter *)(ptr->value))
#define AS_ITER_V(ptr) ((Nst_iter *)(ptr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct Nst_iter
{
    Nst_Obj *start;
    Nst_Obj *advance;
    Nst_Obj *is_done;
    Nst_Obj *get_val;
    Nst_Obj *value;
}
Nst_iter;

Nst_Obj *new_iter_obj(Nst_iter *iter);
Nst_iter *new_iter(Nst_Obj *start,
                   Nst_Obj *advance,
                   Nst_Obj *is_done,
                   Nst_Obj *get_val,
                   Nst_Obj *value);
void destroy_iter(Nst_iter *iter);

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