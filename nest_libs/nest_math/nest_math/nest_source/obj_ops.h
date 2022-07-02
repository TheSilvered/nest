#ifndef OBJ_OPS_H
#define OBJ_OPS_H

/*
Various operations for Nst_Obj.

A funtion returns NULL when a type error occurs, otherwise
'name' and 'message' are set in 'err'

Functions that are guaranteed to not return an error:
    obj_eq
    obj_concat
    obj_cast when an object is casted to a string or a boolean
*/

#include "error.h"
#include "obj.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Comparisons
Nst_Obj *obj_eq(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_ne(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_gt(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_lt(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_ge(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_le(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);

// Arithmetic operations
Nst_Obj *obj_add(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_sub(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_mul(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_div(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_pow(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_mod(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);

// Bitwise operations
Nst_Obj *obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);

// Logical operations
Nst_Obj *obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);
Nst_Obj *obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);

// Other
Nst_Obj *obj_str_cast_seq(Nst_Obj *seq_obj, LList *all_objs);
Nst_Obj *obj_str_cast_map(Nst_Obj *map_obj, LList *all_objs);
Nst_Obj *obj_cast(Nst_Obj *ob, Nst_Obj *type, OpErr *err);
Nst_Obj *obj_concat(Nst_Obj *ob1, Nst_Obj *ob2, OpErr *err);

// Local operations
Nst_Obj *obj_neg(Nst_Obj *ob, OpErr *err);
Nst_Obj *obj_len(Nst_Obj *ob, OpErr *err);
Nst_Obj *obj_bwnot(Nst_Obj *ob, OpErr *err);
Nst_Obj *obj_lgnot(Nst_Obj *ob, OpErr *err);
Nst_Obj *obj_stdout(Nst_Obj *ob, OpErr *err);
Nst_Obj *obj_stdin(Nst_Obj *ob, OpErr *err);
Nst_Obj *obj_typeof(Nst_Obj *ob, OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_OPS_H
