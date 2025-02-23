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

    Nst_Obj *t_Type;
    Nst_Obj *t_Int;
    Nst_Obj *t_Real;
    Nst_Obj *t_Bool;
    Nst_Obj *t_Null;
    Nst_Obj *t_Str;
    Nst_Obj *t_Array;
    Nst_Obj *t_Vector;
    Nst_Obj *t_Map;
    Nst_Obj *t_Func;
    Nst_Obj *t_Iter;
    Nst_Obj *t_Byte;
    Nst_Obj *t_IOFile;
    Nst_Obj *t_IEnd;

    // Constant strings

    Nst_Obj *c_true;
    Nst_Obj *c_false;
    Nst_Obj *c_null;
    Nst_Obj *c_inf;
    Nst_Obj *c_nan;
    Nst_Obj *c_neginf;
    Nst_Obj *c_negnan;

    // Standard error names

    Nst_Obj *e_SyntaxError;
    Nst_Obj *e_MemoryError;
    Nst_Obj *e_ValueError;
    Nst_Obj *e_TypeError;
    Nst_Obj *e_CallError;
    Nst_Obj *e_MathError;
    Nst_Obj *e_ImportError;
    Nst_Obj *e_Interrupt;

    // Other

    Nst_Obj *o__vars_;
    Nst_Obj *o__globals_;
    Nst_Obj *o__args_;
    Nst_Obj *o_failed_alloc;
} Nst_StrConsts;

/* Type constants. */
NstEXP typedef struct _Nst_TypeObjs {
    Nst_Obj *Type;
    Nst_Obj *Int;
    Nst_Obj *Real;
    Nst_Obj *Bool;
    Nst_Obj *Null;
    Nst_Obj *Str;
    Nst_Obj *Array;
    Nst_Obj *Vector;
    Nst_Obj *Map;
    Nst_Obj *Func;
    Nst_Obj *Iter;
    Nst_Obj *Byte;
    Nst_Obj *IOFile;
    Nst_Obj *IEnd;
} Nst_TypeObjs;

/**
 * Other constants.
 *
 * @brief Each constants is preceded by the name of its type.
 */
NstEXP typedef struct _Nst_Consts {
    Nst_Obj *Bool_true;
    Nst_Obj *Bool_false;
    Nst_Obj *Null_null;
    Nst_Obj *IEnd_iend;
    Nst_Obj *Int_0;
    Nst_Obj *Int_1;
    Nst_Obj *Int_neg1;
    Nst_Obj *Real_0;
    Nst_Obj *Real_1;
    Nst_Obj *Real_nan;
    Nst_Obj *Real_negnan;
    Nst_Obj *Real_inf;
    Nst_Obj *Real_neginf;
    Nst_Obj *Byte_0;
    Nst_Obj *Byte_1;
} Nst_Consts;

/**
 * Standard IO streams.
 *
 * @brief Note: these are not constant and can change at run-time.
 */
NstEXP typedef struct _Nst_StdStreams {
    Nst_Obj *in;
    Nst_Obj *out;
    Nst_Obj *err;
} Nst_StdStreams;

typedef struct _Nst_IterFunctions {
    Nst_Obj *range_start;
    Nst_Obj *range_next;

    Nst_Obj *seq_start;
    Nst_Obj *seq_next;

    Nst_Obj *str_start;
    Nst_Obj *str_next;

    Nst_Obj *map_start;
    Nst_Obj *map_next;
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
NstEXP Nst_Obj *NstC Nst_true_ref(void);

/* Returns the false object constant. */
NstEXP Nst_Obj *NstC Nst_false(void);
/* Returns a new reference to the false object constant. */
NstEXP Nst_Obj *NstC Nst_false_ref(void);

/* Returns the null object constant. */
NstEXP Nst_Obj *NstC Nst_null(void);
/* Returns a new reference to the null object constant. */
NstEXP Nst_Obj *NstC Nst_null_ref(void);

/* Returns the iend object constant. */
NstEXP Nst_Obj *NstC Nst_iend(void);
/* Returns a new reference to the iend object constant. */
NstEXP Nst_Obj *NstC Nst_iend_ref(void);

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
