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

#define nst_obj_eq(ob1, ob2, err) _nst_obj_eq((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_ne(ob1, ob2, err) _nst_obj_ne((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_gt(ob1, ob2, err) _nst_obj_gt((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_lt(ob1, ob2, err) _nst_obj_lt((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_ge(ob1, ob2, err) _nst_obj_ge((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_le(ob1, ob2, err) _nst_obj_le((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_add(ob1, ob2, err) _nst_obj_add((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_sub(ob1, ob2, err) _nst_obj_sub((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_mul(ob1, ob2, err) _nst_obj_mul((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_div(ob1, ob2, err) _nst_obj_div((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_pow(ob1, ob2, err) _nst_obj_pow((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_mod(ob1, ob2, err) _nst_obj_mod((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_bwor(ob1, ob2, err) _nst_obj_bwor((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_bwand(ob1, ob2, err) _nst_obj_bwand((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_bwxor(ob1, ob2, err) _nst_obj_bwxor((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_bwls(ob1, ob2, err) _nst_obj_bwls((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_bwrs(ob1, ob2, err) _nst_obj_bwrs((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_lgor(ob1, ob2, err) _nst_obj_lgor((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_lgand(ob1, ob2, err) _nst_obj_lgand((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_lgxor(ob1, ob2, err) _nst_obj_lgxor((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_cast(ob, type, err) _nst_obj_cast((Nst_Obj *)(ob), (Nst_Obj *)(type), err)
#define nst_obj_concat(ob1, ob2, err) _nst_obj_concat((Nst_Obj *)(ob1), (Nst_Obj *)(ob2), err)
#define nst_obj_neg(ob, err) _nst_obj_neg((Nst_Obj *)(ob), err)
#define nst_obj_len(ob, err) _nst_obj_len((Nst_Obj *)(ob), err)
#define nst_obj_bwnot(ob, err) _nst_obj_bwnot((Nst_Obj *)(ob), err)
#define nst_obj_lgnot(ob, err) _nst_obj_lgnot((Nst_Obj *)(ob), err)
#define nst_obj_stdout(ob, err) _nst_obj_stdout((Nst_Obj *)(ob), err)
#define nst_obj_stdin(ob, err) _nst_obj_stdin((Nst_Obj *)(ob), err)
#define nst_obj_typeof(ob, err) _nst_obj_typeof((Nst_Obj *)(ob), err)
#define nst_obj_import(ob, err) _nst_obj_import((Nst_Obj *)(ob), err)

#include "error.h"
#include "obj.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Comparisons
Nst_Obj *_nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);

// Arithmetic operations
Nst_Obj *_nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);

// Bitwise operations
Nst_Obj *_nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);

// Logical operations
Nst_Obj *_nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);
Nst_Obj *_nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);

// Other
Nst_Obj *_nst_obj_str_cast_seq(Nst_Obj *seq_obj, LList *all_objs);
Nst_Obj *_nst_obj_str_cast_map(Nst_Obj *map_obj, LList *all_objs);
Nst_Obj *_nst_obj_cast(Nst_Obj *ob, Nst_Obj *type, Nst_OpErr *err);
Nst_Obj *_nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err);

// Local operations
Nst_Obj *_nst_obj_neg(Nst_Obj *ob, Nst_OpErr *err);
Nst_Obj *_nst_obj_len(Nst_Obj *ob, Nst_OpErr *err);
Nst_Obj *_nst_obj_bwnot(Nst_Obj *ob, Nst_OpErr *err);
Nst_Obj *_nst_obj_lgnot(Nst_Obj *ob, Nst_OpErr *err);
Nst_Obj *_nst_obj_stdout(Nst_Obj *ob, Nst_OpErr *err);
Nst_Obj *_nst_obj_stdin(Nst_Obj *ob, Nst_OpErr *err);
Nst_Obj *_nst_obj_typeof(Nst_Obj *ob, Nst_OpErr *err);
Nst_Obj *_nst_obj_import(Nst_Obj *ob, Nst_OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_OPS_H
