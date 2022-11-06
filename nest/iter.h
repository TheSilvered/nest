/* Nst_IterObj interface */

#ifndef ITER_H
#define ITER_H

#include "error.h"
#include "obj.h"
#include "ggc.h"
#include "function.h"
#include "lib_import.h"

#define ITER(ptr) ((Nst_IterObj *)(ptr))

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