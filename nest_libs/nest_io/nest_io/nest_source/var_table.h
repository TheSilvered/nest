#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include "map.h"
#include "obj.h"
#include "nst_types.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct VarTable
{
    Nst_map *vars;
    struct VarTable *global_table;
}
VarTable;

VarTable *new_var_table(VarTable *global_table, Nst_string *path);
Nst_Obj *get_val(VarTable *vt, Nst_Obj *name);
void set_val(VarTable *vt, Nst_Obj *name, Nst_Obj *val);
Nst_Obj *set_argv(VarTable *vt, int argc, char **argv);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VAR_TABLE_H