#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include "map.h"
#include "obj.h"

typedef struct VarTable
{
    Nst_map *vars;
    struct VarTable *global_table;
}
VarTable;

VarTable *new_var_table(VarTable *global_table);
Nst_Obj *get_val(VarTable *vt, Nst_Obj *name);
void set_val(VarTable *vt, Nst_Obj *name, Nst_Obj *val);

#endif // !VAR_TABLE_H