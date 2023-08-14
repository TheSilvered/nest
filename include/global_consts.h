/**
 * @file global_consts.h
 *
 * @brief Constant objects used in the program
 *
 * @author TheSilvered
 */

#ifndef GLOBAL_CONSTS_H
#define GLOBAL_CONSTS_H

#include "error.h"
#include "simple_types.h"
#include "function.h"
#include "file.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * String constants
 *
 * @brief Fields starting with t_ are the name of types, the ones starting with
 * c_ are the name of constants, the one starting with e_ are the name of
 * errors and the one starting with o_ are others.
 */
NstEXP typedef struct _Nst_StrConsts {
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
} Nst_StrConsts;

/* Type constants */
NstEXP typedef struct _Nst_TypeObjs {
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
} Nst_TypeObjs;

/**
 * Other constants
 *
 * @brief Each constants is preceded by the name of its type.
 */
NstEXP typedef struct _Nst_Consts {
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
} Nst_Consts;

/* Standard IO streams, these are not constant and can change at run-time. */
NstEXP typedef struct _Nst_StdStreams {
    Nst_IOFileObj *in;
    Nst_IOFileObj *out;
    Nst_IOFileObj *err;
} Nst_StdStreams;

/* The functions of the built-in iterators */
NstEXP typedef struct _Nst_IterFunctions {
    Nst_FuncObj *range_start;
    Nst_FuncObj *range_is_done;
    Nst_FuncObj *range_get_val;

    Nst_FuncObj *seq_start;
    Nst_FuncObj *seq_is_done;
    Nst_FuncObj *seq_get_val;

    Nst_FuncObj *str_start;
    Nst_FuncObj *str_is_done;
    Nst_FuncObj *str_get_val;

    Nst_FuncObj *map_start;
    Nst_FuncObj *map_is_done;
    Nst_FuncObj *map_get_val;
} Nst_IterFunctions;

/**
 * Initializes all the global constants and IO streams.
 *
 * @return true on success and false on failure. No error is set.
 */
NstEXP bool NstC _Nst_init_objects(void);
/* Deletes all the global constants and IO streams. */
NstEXP void NstC _Nst_del_objects(void);

/* Returns the true object constant. */
NstEXP Nst_Obj *NstC Nst_true(void);
/* Returns the false object constant. */
NstEXP Nst_Obj *NstC Nst_false(void);
/* Returns the null object constant. */
NstEXP Nst_Obj *NstC Nst_null(void);

/* Returns a Nst_TypeObjs struct containing all the types. */
NstEXP const Nst_TypeObjs *NstC Nst_type(void);
/* Returns a Nst_StrConsts struct containing all the string constants. */
NstEXP const Nst_StrConsts *NstC Nst_str(void);
/* Returns a Nst_Consts struct containing all the object constants. */
NstEXP const Nst_Consts *NstC Nst_const(void);
/* Returns a Nst_IterFunctions struct containing all the function constants. */
NstEXP const Nst_IterFunctions *NstC Nst_iter_func(void);
/* Returns a Nst_StdStreams struct containing all the standard IO streams. */
NstEXP Nst_StdStreams *NstC Nst_stdio(void);

extern Nst_TypeObjs Nst_t;
extern Nst_StrConsts Nst_s;
extern Nst_Consts Nst_c;
extern Nst_IterFunctions Nst_itf;
extern Nst_StdStreams Nst_io;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GLOBAL_CONSTS_H
