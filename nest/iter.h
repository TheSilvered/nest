#ifndef ITER_H
#define ITER_H

#include "error.h"
#include "obj.h"
#include "function.h"
#include "lib_import.h"

#define AS_ITER(ptr) ((Nst_IterObj *)(ptr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    OBJ_HEAD;
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

NST_FUNC_SIGN(nst_num_iter_start);
NST_FUNC_SIGN(nst_num_iter_advance);
NST_FUNC_SIGN(nst_num_iter_is_done);
NST_FUNC_SIGN(nst_num_iter_get_val);

NST_FUNC_SIGN(nst_seq_iter_start);
NST_FUNC_SIGN(nst_seq_iter_advance);
NST_FUNC_SIGN(nst_seq_iter_is_done);
NST_FUNC_SIGN(nst_seq_iter_get_val);

NST_FUNC_SIGN(nst_str_iter_start);
NST_FUNC_SIGN(nst_str_iter_advance);
NST_FUNC_SIGN(nst_str_iter_is_done);
NST_FUNC_SIGN(nst_str_iter_get_val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // ITER_H