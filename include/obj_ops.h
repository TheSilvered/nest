/* Object operations */

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

#include "map.h"
#include "sequence.h"

// operator ==
#define nst_obj_eq(ob1, ob2) _nst_obj_eq(OBJ(ob1), OBJ(ob2))
// operator !=
#define nst_obj_ne(ob1, ob2) _nst_obj_ne(OBJ(ob1), OBJ(ob2))
// operator >
#define nst_obj_gt(ob1, ob2) _nst_obj_gt(OBJ(ob1), OBJ(ob2))
// operator <
#define nst_obj_lt(ob1, ob2) _nst_obj_lt(OBJ(ob1), OBJ(ob2))
// operator >=
#define nst_obj_ge(ob1, ob2) _nst_obj_ge(OBJ(ob1), OBJ(ob2))
// operator <=
#define nst_obj_le(ob1, ob2) _nst_obj_le(OBJ(ob1), OBJ(ob2))
// operator +
#define nst_obj_add(ob1, ob2) _nst_obj_add(OBJ(ob1), OBJ(ob2))
// operator -
#define nst_obj_sub(ob1, ob2) _nst_obj_sub(OBJ(ob1), OBJ(ob2))
// operator *
#define nst_obj_mul(ob1, ob2) _nst_obj_mul(OBJ(ob1), OBJ(ob2))
// operator /
#define nst_obj_div(ob1, ob2) _nst_obj_div(OBJ(ob1), OBJ(ob2))
// operator ^
#define nst_obj_pow(ob1, ob2) _nst_obj_pow(OBJ(ob1), OBJ(ob2))
// operator %
#define nst_obj_mod(ob1, ob2) _nst_obj_mod(OBJ(ob1), OBJ(ob2))
// operator |
#define nst_obj_bwor(ob1, ob2) _nst_obj_bwor(OBJ(ob1), OBJ(ob2))
// operator &
#define nst_obj_bwand(ob1, ob2) _nst_obj_bwand(OBJ(ob1), OBJ(ob2))
// operator ^^
#define nst_obj_bwxor(ob1, ob2) _nst_obj_bwxor(OBJ(ob1), OBJ(ob2))
// operator <<
#define nst_obj_bwls(ob1, ob2) _nst_obj_bwls(OBJ(ob1), OBJ(ob2))
// operator >>
#define nst_obj_bwrs(ob1, ob2) _nst_obj_bwrs(OBJ(ob1), OBJ(ob2))
// operator ||
#define nst_obj_lgor(ob1, ob2) _nst_obj_lgor(OBJ(ob1), OBJ(ob2))
// operator &&
#define nst_obj_lgand(ob1, ob2) _nst_obj_lgand(OBJ(ob1), OBJ(ob2))
// operator &|
#define nst_obj_lgxor(ob1, ob2) _nst_obj_lgxor(OBJ(ob1), OBJ(ob2))
// operator ::
#define nst_obj_cast(ob, type) _nst_obj_cast(OBJ(ob), TYPE(type))
// operator ><
#define nst_obj_concat(ob1, ob2) _nst_obj_concat(OBJ(ob1), OBJ(ob2))
// operator ->
#define nst_obj_range(ob1, ob2, ob3) \
    _nst_obj_range(OBJ(ob1), OBJ(ob2), OBJ(ob3))
// operator -:
#define nst_obj_neg(ob) _nst_obj_neg(OBJ(ob))
// operator $
#define nst_obj_len(ob) _nst_obj_len(OBJ(ob))
// operator ~
#define nst_obj_bwnot(ob) _nst_obj_bwnot(OBJ(ob))
// operator !
#define nst_obj_lgnot(ob) _nst_obj_lgnot(OBJ(ob))
// operator >>>
#define nst_obj_stdout(ob) _nst_obj_stdout(OBJ(ob))
// operator <<<
#define nst_obj_stdin(ob) _nst_obj_stdin(OBJ(ob))
// operator ?::
#define nst_obj_typeof(ob) _nst_obj_typeof(OBJ(ob))
// operator |#|
#define nst_obj_import(ob) _nst_obj_import(OBJ(ob))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Comparisons

NstEXP Nst_Obj *_nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2);

// Arithmetic operations

NstEXP Nst_Obj *_nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2);

// Bitwise operations

NstEXP Nst_Obj *_nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2);

// Logical operations

NstEXP Nst_Obj *_nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2);

// Other

// Casts an object to a string that does not contain unprintable characters
// and that usually a valid literal for the value
NstEXP Nst_Obj *_nst_repr_str_cast(Nst_Obj *ob);
// Internal cast from sequence to Str
NstEXP Nst_Obj *_nst_obj_str_cast_seq(Nst_SeqObj *seq_obj,
                                      Nst_LList  *all_objs);
// Internal cast from Map to Str
NstEXP Nst_Obj *_nst_obj_str_cast_map(Nst_MapObj *map_obj,
                                      Nst_LList  *all_objs);
NstEXP Nst_Obj *_nst_obj_cast(Nst_Obj *ob, Nst_TypeObj *type);
NstEXP Nst_Obj *_nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2);
NstEXP Nst_Obj *_nst_obj_range(Nst_Obj   *start,
                               Nst_Obj   *stop,
                               Nst_Obj   *step);

// Local operations

NstEXP Nst_Obj *_nst_obj_neg(Nst_Obj *ob);
NstEXP Nst_Obj *_nst_obj_len(Nst_Obj *ob);
NstEXP Nst_Obj *_nst_obj_bwnot(Nst_Obj *ob);
NstEXP Nst_Obj *_nst_obj_lgnot(Nst_Obj *ob);
NstEXP Nst_Obj *_nst_obj_stdout(Nst_Obj *ob);
NstEXP Nst_Obj *_nst_obj_stdin(Nst_Obj *ob);
NstEXP Nst_Obj *_nst_obj_typeof(Nst_Obj *ob);
NstEXP Nst_Obj *_nst_obj_import(Nst_Obj *ob);

// returns the path of the library to import or NULL if it cannot be found
// the path is heap allocated so it will require to be freed
NstEXP Nst_StrObj *_nst_get_import_path(i8 *initial_path, usize path_len);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_OPS_H
