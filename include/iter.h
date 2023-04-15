/* Nst_IterObj interface */

#ifndef ITER_H
#define ITER_H

#include "lib_import.h"

#define ITER(ptr) ((Nst_IterObj *)(ptr))
// Executes the _start_ function of `iter`, returns 0 on success and -1
// on failure
#define nst_iter_start(iter, err) _nst_iter_start(ITER(iter), err)
// Executes the _is_done_ function of `iter`, returns 0 if the _is_done_
// returned false, 1 it the function returned true and -1 on failure
#define nst_iter_is_done(iter, err) _nst_iter_is_done(ITER(iter), err)
// Executes the _get_val_ function of `iter`, returns the object on
// success and NULL on failure
#define nst_iter_get_val(iter, err) _nst_iter_get_val(ITER(iter), err)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef struct _Nst_IterObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    Nst_FuncObj *start;
    Nst_FuncObj *is_done;
    Nst_FuncObj *get_val;
    Nst_Obj *value;
}
Nst_IterObj;

EXPORT
Nst_Obj *nst_iter_new(Nst_FuncObj *start,
                      Nst_FuncObj *is_done,
                      Nst_FuncObj *get_val,
                      Nst_Obj     *value,
                      Nst_OpErr   *err);
EXPORT void _nst_iter_destroy(Nst_IterObj *iter);
EXPORT void _nst_iter_traverse(Nst_IterObj *iter);
EXPORT void _nst_iter_track(Nst_IterObj *iter);

EXPORT i32 _nst_iter_start(Nst_IterObj *iter, Nst_OpErr *err);
EXPORT i32 _nst_iter_is_done(Nst_IterObj *iter, Nst_OpErr *err);
EXPORT Nst_Obj *_nst_iter_get_val(Nst_IterObj *iter, Nst_OpErr *err);

// Functions for the range iterator

EXPORT NST_FUNC_SIGN(nst_iter_range_start);
EXPORT NST_FUNC_SIGN(nst_iter_range_is_done);
EXPORT NST_FUNC_SIGN(nst_iter_range_get_val);

// Functions for the sequence iterator

EXPORT NST_FUNC_SIGN(nst_iter_seq_start);
EXPORT NST_FUNC_SIGN(nst_iter_seq_is_done);
EXPORT NST_FUNC_SIGN(nst_iter_seq_get_val);

// Functions for the string iterator

EXPORT NST_FUNC_SIGN(nst_iter_str_start);
EXPORT NST_FUNC_SIGN(nst_iter_str_is_done);
EXPORT NST_FUNC_SIGN(nst_iter_str_get_val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // ITER_H
