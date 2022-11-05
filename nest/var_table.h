/* Variable table interface using Nst_MapObjs */

#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include "map.h"
#include "obj.h"
#include "str.h"
#include "sequence.h"

#define nst_get_val(vt, name) _nst_get_val(vt, (Nst_Obj *)(name))
#define nst_set_val(vt, name, val) _nst_set_val(vt, (Nst_Obj *)(name), (Nst_Obj *)(val))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_VarTable
{
    Nst_MapObj *vars;
    Nst_MapObj *global_table;
}
Nst_VarTable;

// Creates a new variable table
Nst_VarTable *nst_new_var_table(Nst_MapObj *global_table,
                                Nst_StrObj *cwd,
                                Nst_SeqObj *args);
// Returns a value from the variable table
Nst_Obj *_nst_get_val(Nst_VarTable *vt, Nst_Obj *name);
// Sets a value in the variable table
void _nst_set_val(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val);
#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VAR_TABLE_H