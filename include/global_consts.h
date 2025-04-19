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
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * String constants.
 *
 * @brief Fields starting with `c_` are the name of language constants (e.g.
 * true), the one starting with `e_` are the name of errors and the ones
 * starting with `o_` are others.
 */
NstEXP typedef struct _Nst_StrConsts {
    // Built-in type names

    Nst_ObjRef *t_Type;
    Nst_ObjRef *t_Int;
    Nst_ObjRef *t_Real;
    Nst_ObjRef *t_Bool;
    Nst_ObjRef *t_Null;
    Nst_ObjRef *t_Str;
    Nst_ObjRef *t_Array;
    Nst_ObjRef *t_Vector;
    Nst_ObjRef *t_Map;
    Nst_ObjRef *t_Func;
    Nst_ObjRef *t_Iter;
    Nst_ObjRef *t_Byte;
    Nst_ObjRef *t_IOFile;
    Nst_ObjRef *t_IEnd;

    // Constant strings

    Nst_ObjRef *c_true;
    Nst_ObjRef *c_false;
    Nst_ObjRef *c_null;
    Nst_ObjRef *c_inf;
    Nst_ObjRef *c_nan;
    Nst_ObjRef *c_neginf;
    Nst_ObjRef *c_negnan;

    // Standard error names

    Nst_ObjRef *e_SyntaxError;
    Nst_ObjRef *e_MemoryError;
    Nst_ObjRef *e_ValueError;
    Nst_ObjRef *e_TypeError;
    Nst_ObjRef *e_CallError;
    Nst_ObjRef *e_MathError;
    Nst_ObjRef *e_ImportError;
    Nst_ObjRef *e_Interrupt;

    // Other

    Nst_ObjRef *o__vars_;
    Nst_ObjRef *o__globals_;
    Nst_ObjRef *o__args_;
    Nst_ObjRef *o_failed_alloc;
} Nst_StrConsts;

/* Type constants. */
NstEXP typedef struct _Nst_TypeObjs {
    Nst_ObjRef *Type;
    Nst_ObjRef *Int;
    Nst_ObjRef *Real;
    Nst_ObjRef *Bool;
    Nst_ObjRef *Null;
    Nst_ObjRef *Str;
    Nst_ObjRef *Array;
    Nst_ObjRef *Vector;
    Nst_ObjRef *Map;
    Nst_ObjRef *Func;
    Nst_ObjRef *Iter;
    Nst_ObjRef *Byte;
    Nst_ObjRef *IOFile;
    Nst_ObjRef *IEnd;
} Nst_TypeObjs;

/**
 * Other constants.
 *
 * @brief Each constants is preceded by the name of its type.
 */
NstEXP typedef struct _Nst_Consts {
    Nst_ObjRef *Bool_true;
    Nst_ObjRef *Bool_false;
    Nst_ObjRef *Null_null;
    Nst_ObjRef *IEnd_iend;
    Nst_ObjRef *Int_0;
    Nst_ObjRef *Int_1;
    Nst_ObjRef *Int_neg1;
    Nst_ObjRef *Real_0;
    Nst_ObjRef *Real_1;
    Nst_ObjRef *Real_nan;
    Nst_ObjRef *Real_negnan;
    Nst_ObjRef *Real_inf;
    Nst_ObjRef *Real_neginf;
    Nst_ObjRef *Byte_0;
    Nst_ObjRef *Byte_1;
} Nst_Consts;

/**
 * Standard IO streams.
 *
 * @brief Note: these are not constant and can change at run-time.
 */
NstEXP typedef struct _Nst_StdStreams {
    Nst_ObjRef *in;
    Nst_ObjRef *out;
    Nst_ObjRef *err;
} Nst_StdStreams;

typedef struct _Nst_IterFunctions {
    Nst_ObjRef *range_start;
    Nst_ObjRef *range_next;

    Nst_ObjRef *seq_start;
    Nst_ObjRef *seq_next;

    Nst_ObjRef *str_start;
    Nst_ObjRef *str_next;

    Nst_ObjRef *map_start;
    Nst_ObjRef *map_next;
} Nst_IterFunctions;

/**
 * Initializes all the global constants and IO streams.
 *
 * @return `true` on success and `false` on failure. No error is set.
 */
NstEXP bool NstC _Nst_globals_init(void);
/* Deletes all the global constants and IO streams. */
NstEXP void NstC _Nst_globals_quit(void);

/* Returns the true object constant. */
NstEXP Nst_Obj *NstC Nst_true(void);
/* Returns a new reference to the true object constant. */
NstEXP Nst_ObjRef *NstC Nst_true_ref(void);

/* Returns the false object constant. */
NstEXP Nst_Obj *NstC Nst_false(void);
/* Returns a new reference to the false object constant. */
NstEXP Nst_ObjRef *NstC Nst_false_ref(void);

/* Returns the null object constant. */
NstEXP Nst_Obj *NstC Nst_null(void);
/* Returns a new reference to the null object constant. */
NstEXP Nst_ObjRef *NstC Nst_null_ref(void);

/* Returns the iend object constant. */
NstEXP Nst_Obj *NstC Nst_iend(void);
/* Returns a new reference to the iend object constant. */
NstEXP Nst_ObjRef *NstC Nst_iend_ref(void);

/* [docs:link Nst_type()->Bool Nst_type] */

/* Returns a `Nst_TypeObjs` struct containing all the types. */
NstEXP const Nst_TypeObjs *NstC Nst_type(void);
/* Returns a `Nst_StrConsts` struct containing all the string constants. */
NstEXP const Nst_StrConsts *NstC Nst_str(void);
/* Returns a `Nst_Consts` struct containing all the object constants. */
NstEXP const Nst_Consts *NstC Nst_const(void);
/* Returns a `Nst_StdStreams` struct containing all the standard IO streams. */
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
