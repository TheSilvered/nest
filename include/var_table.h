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

    EXPORT typedef struct _Nst_VarTable
{
    Nst_MapObj *vars;
    Nst_MapObj *global_table;
}
Nst_VarTable;

// Creates a new variable table
EXPORT
Nst_VarTable *nst_vt_new(Nst_MapObj *global_table,
                         Nst_StrObj *cwd,
                         Nst_SeqObj *args);
EXPORT Nst_Obj *_nst_vt_get(Nst_VarTable *vt, Nst_Obj *name);
EXPORT void _nst_vt_set(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val);
#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VAR_TABLE_H