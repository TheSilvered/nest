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
#define nst_start_iter(iter, err) _nst_start_iter(ITER(iter), err)
// Executes the _is_done_ function of `iter`, returns 0 if the _is_done_
// returned false, 1 it the function returned true and -1 on failure
#define nst_is_done_iter(iter, err) _nst_is_done_iter(ITER(iter), err)
// Executes the _advance_ function of `iter`, returns 0 on success and -1
// on failure
#define nst_advance_iter(iter, err) _nst_advance_iter(ITER(iter), err)
// Executes the _get_val_ function of `iter`, returns the object on
// success and NULL on failure
#define nst_get_val_iter(iter, err) _nst_get_val_iter(ITER(iter), err)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_IterObj
{
    NST_OBJ_HEAD;
    NST_GGC_SUPPORT;
    Nst_FuncObj *start;
    Nst_FuncObj *advance;
    Nst_FuncObj *is_done;
    Nst_FuncObj *get_val;
    Nst_Obj *value;
}
Nst_IterObj;

Nst_Obj *nst_new_iter(
    Nst_FuncObj *start,
    Nst_FuncObj *advance,
    Nst_FuncObj *is_done,
    Nst_FuncObj *get_val,
    Nst_Obj *value);
void nst_destroy_iter(Nst_IterObj *iter);
void nst_traverse_iter(Nst_IterObj *iter);
void nst_track_iter(Nst_IterObj *iter);

int _nst_start_iter(Nst_IterObj *iter, Nst_OpErr *err);
int _nst_is_done_iter(Nst_IterObj *iter, Nst_OpErr *err);
int _nst_advance_iter(Nst_IterObj *iter, Nst_OpErr *err);
Nst_Obj *_nst_get_val_iter(Nst_IterObj *iter, Nst_OpErr *err);

// Functions for the range iterator

NST_FUNC_SIGN(nst_num_iter_start);
NST_FUNC_SIGN(nst_num_iter_advance);
NST_FUNC_SIGN(nst_num_iter_is_done);
NST_FUNC_SIGN(nst_num_iter_get_val);

// Functions for the sequence iterator

NST_FUNC_SIGN(nst_seq_iter_start);
NST_FUNC_SIGN(nst_seq_iter_advance);
NST_FUNC_SIGN(nst_seq_iter_is_done);
NST_FUNC_SIGN(nst_seq_iter_get_val);

// Functions for the string iterator

NST_FUNC_SIGN(nst_str_iter_start);
NST_FUNC_SIGN(nst_str_iter_advance);
NST_FUNC_SIGN(nst_str_iter_is_done);
NST_FUNC_SIGN(nst_str_iter_get_val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // ITER_H