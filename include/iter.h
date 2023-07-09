/* Nst_IterObj interface */

#ifndef ITER_H
#define ITER_H

#include "lib_import.h"

#define ITER(ptr) ((Nst_IterObj *)(ptr))
// Executes the _start_ function of `iter`, returns 0 on success and -1
// on failure
#define Nst_iter_start(iter) _Nst_iter_start(ITER(iter))
// Executes the _is_done_ function of `iter`, returns 0 if the _is_done_
// returned false, 1 it the function returned true and -1 on failure
#define Nst_iter_is_done(iter) _Nst_iter_is_done(ITER(iter))
// Executes the _get_val_ function of `iter`, returns the object on
// success and NULL on failure
#define Nst_iter_get_val(iter) _Nst_iter_get_val(ITER(iter))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef struct _Nst_IterObj
{
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_FuncObj *start;
    Nst_FuncObj *is_done;
    Nst_FuncObj *get_val;
    Nst_Obj *value;
}
Nst_IterObj;

NstEXP Nst_Obj *NstC Nst_iter_new(Nst_FuncObj *start, Nst_FuncObj *is_done,
                                  Nst_FuncObj *get_val, Nst_Obj *value);
NstEXP void NstC _Nst_iter_destroy(Nst_IterObj *iter);
NstEXP void NstC _Nst_iter_traverse(Nst_IterObj *iter);
NstEXP void NstC _Nst_iter_track(Nst_IterObj *iter);

NstEXP i32 NstC _Nst_iter_start(Nst_IterObj *iter);
NstEXP i32 NstC _Nst_iter_is_done(Nst_IterObj *iter);
NstEXP Nst_Obj *NstC _Nst_iter_get_val(Nst_IterObj *iter);

// Functions for the range iterator

NstEXP Nst_FUNC_SIGN(Nst_iter_range_start);
NstEXP Nst_FUNC_SIGN(Nst_iter_range_is_done);
NstEXP Nst_FUNC_SIGN(Nst_iter_range_get_val);

// Functions for the sequence iterator

NstEXP Nst_FUNC_SIGN(Nst_iter_seq_start);
NstEXP Nst_FUNC_SIGN(Nst_iter_seq_is_done);
NstEXP Nst_FUNC_SIGN(Nst_iter_seq_get_val);

// Functions for the string iterator

NstEXP Nst_FUNC_SIGN(Nst_iter_str_start);
NstEXP Nst_FUNC_SIGN(Nst_iter_str_is_done);
NstEXP Nst_FUNC_SIGN(Nst_iter_str_get_val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // ITER_H
