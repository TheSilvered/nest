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

typedef struct Nst_VarTable
{
    Nst_MapObj *vars;
    struct Nst_VarTable *global_table;
}
Nst_VarTable;

Nst_VarTable *nst_new_var_table(Nst_VarTable *global_table,
                                Nst_StrObj *cwd,
                                Nst_SeqObj *args);
Nst_Obj *_nst_get_val(Nst_VarTable *vt, Nst_Obj *name);
void _nst_set_val(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val);
#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VAR_TABLE_H