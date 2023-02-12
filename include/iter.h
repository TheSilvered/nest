/* Nst_IterObj interface */

#ifndef ITER_H
#define ITER_H

#include "error.h"
#include "obj.h"
#include "ggc.h"
#include "function.h"
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

typedef struct _Nst_IterObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    Nst_FuncObj *start;
    Nst_FuncObj *is_done;
    Nst_FuncObj *get_val;
    Nst_Obj *value;
}
Nst_IterObj;

Nst_Obj *nst_iter_new(Nst_FuncObj *start,
                      Nst_FuncObj *is_done,
                      Nst_FuncObj *get_val,
                      Nst_Obj     *value);
void _nst_iter_destroy(Nst_IterObj *iter);
void _nst_iter_traverse(Nst_IterObj *iter);
void _nst_iter_track(Nst_IterObj *iter);

int _nst_iter_start(Nst_IterObj *iter, Nst_OpErr *err);
int _nst_iter_is_done(Nst_IterObj *iter, Nst_OpErr *err);
Nst_Obj *_nst_iter_get_val(Nst_IterObj *iter, Nst_OpErr *err);

// Functions for the range iterator

NST_FUNC_SIGN(nst_iter_range_start);
NST_FUNC_SIGN(nst_iter_range_is_done);
NST_FUNC_SIGN(nst_iter_range_get_val);

// Functions for the sequence iterator

NST_FUNC_SIGN(nst_iter_seq_start);
NST_FUNC_SIGN(nst_iter_seq_is_done);
NST_FUNC_SIGN(nst_iter_seq_get_val);

// Functions for the string iterator

NST_FUNC_SIGN(nst_iter_str_start);
NST_FUNC_SIGN(nst_iter_str_is_done);
NST_FUNC_SIGN(nst_iter_str_get_val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // ITER_H