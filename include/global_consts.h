#ifndef GLOBAL_CONSTS_H
#define GLOBAL_CONSTS_H

#include "error.h"
#include "simple_types.h"
#include "function.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// the nomenclature is [group]_[value of the strings without spaces] t_ is
// for types, c_ is for constants, e_ is for error names and o_ is for other
NstEXP typedef struct _Nst_StrConsts
{
    // Type strings
    Nst_StrObj *t_Type;
    Nst_StrObj *t_Int;
    Nst_StrObj *t_Real;
    Nst_StrObj *t_Bool;
    Nst_StrObj *t_Null;
    Nst_StrObj *t_Str;
    Nst_StrObj *t_Array;
    Nst_StrObj *t_Vector;
    Nst_StrObj *t_Map;
    Nst_StrObj *t_Func;
    Nst_StrObj *t_Iter;
    Nst_StrObj *t_Byte;
    Nst_StrObj *t_IOFile;

    // Constant strings
    Nst_StrObj *c_true;
    Nst_StrObj *c_false;
    Nst_StrObj *c_null;

    // Standard error names
    Nst_StrObj *e_SyntaxError;
    Nst_StrObj *e_MemoryError;
    Nst_StrObj *e_ValueError;
    Nst_StrObj *e_TypeError;
    Nst_StrObj *e_CallError;
    Nst_StrObj *e_MathError;
    Nst_StrObj *e_ImportError;

    // Other
    Nst_StrObj *o__vars_;
    Nst_StrObj *o__globals_;
    Nst_StrObj *o__args_;
    Nst_StrObj *o__cwd_;
    Nst_StrObj *o_failed_alloc;
}
Nst_StrConsts;

NstEXP typedef struct _Nst_TypeObjs
{
    Nst_TypeObj *Type;
    Nst_TypeObj *Int;
    Nst_TypeObj *Real;
    Nst_TypeObj *Bool;
    Nst_TypeObj *Null;
    Nst_TypeObj *Str;
    Nst_TypeObj *Array;
    Nst_TypeObj *Vector;
    Nst_TypeObj *Map;
    Nst_TypeObj *Func;
    Nst_TypeObj *Iter;
    Nst_TypeObj *Byte;
    Nst_TypeObj *IOFile;
}
Nst_TypeObjs;

NstEXP typedef struct _Nst_Consts
{
    Nst_Obj *Bool_true;
    Nst_Obj *Bool_false;
    Nst_Obj *Null_null;
    Nst_Obj *Int_0;
    Nst_Obj *Int_1;
    Nst_Obj *Int_neg1;
    Nst_Obj *Real_0;
    Nst_Obj *Real_1;
    Nst_Obj *Byte_0;
    Nst_Obj *Byte_1;
}
Nst_Consts;

NstEXP typedef struct _Nst_StdStreams
{
    Nst_IOFileObj *in;
    Nst_IOFileObj *out;
    Nst_IOFileObj *err;
}
Nst_StdStreams;

NstEXP typedef struct _Nst_IterFunctions
{
    Nst_FuncObj *range_start;
    Nst_FuncObj *range_is_done;
    Nst_FuncObj *range_get_val;

    Nst_FuncObj *seq_start;
    Nst_FuncObj *seq_is_done;
    Nst_FuncObj *seq_get_val;

    Nst_FuncObj *str_start;
    Nst_FuncObj *str_is_done;
    Nst_FuncObj *str_get_val;
}
Nst_IterFunctions;

NstEXP bool _nst_init_objects(void);
NstEXP void _nst_del_objects(void);

NstEXP Nst_Obj *nst_true(void);
NstEXP Nst_Obj *nst_false(void);
NstEXP Nst_Obj *nst_null(void);

NstEXP const Nst_TypeObjs *nst_type(void);
NstEXP const Nst_StrConsts *nst_str(void);
NstEXP const Nst_Consts *nst_const(void);
NstEXP const Nst_IterFunctions *nst_iter_func(void);
NstEXP Nst_StdStreams *nst_stdio(void);

extern Nst_TypeObjs nst_t;
extern Nst_StrConsts nst_s;
extern Nst_Consts nst_c;
extern Nst_IterFunctions nst_itf;
extern Nst_StdStreams nst_io;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GLOBAL_CONSTS_H
