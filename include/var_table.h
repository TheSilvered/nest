/* Variable table interface using Nst_MapObjs */

#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include "map.h"
#include "sequence.h"

// Returns a value from the variable table
#define nst_vt_get(vt, name) _nst_vt_get(vt, OBJ(name))
// Sets a value in the variable table
#define nst_vt_set(vt, name, val) _nst_vt_set(vt, OBJ(name), OBJ(val))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef struct _Nst_VarTable
{
    Nst_MapObj *vars;
    Nst_MapObj *global_table;
}
Nst_VarTable;

// Creates a new variable table
NstEXP Nst_VarTable *nst_vt_new(Nst_MapObj *global_table,
                                Nst_StrObj *cwd,
                                Nst_SeqObj *args,
                                bool        no_default);
NstEXP void nst_vt_destroy(Nst_VarTable *vt);
NstEXP Nst_Obj *_nst_vt_get(Nst_VarTable *vt, Nst_Obj *name);
NstEXP bool _nst_vt_set(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val);

NstEXP Nst_VarTable *nst_vt_from_func(Nst_FuncObj *f);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VAR_TABLE_H
