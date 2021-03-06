#include <stdlib.h>
#include "var_table.h"
#include "nst_types.h"

VarTable *new_var_table(VarTable *global_table, Nst_StrObj *cwd, Nst_SeqObj *args)
{
    VarTable *vt = malloc(sizeof(VarTable));
    if ( vt == NULL ) return NULL;

    vt->global_table = global_table;
    Nst_MapObj *vars = AS_MAP(new_map());
    vt->vars = vars;

    map_set_str(vars, "_vars_", (Nst_Obj *)vars);

    if ( global_table != NULL )
        return vt;

    map_set_str(vars, "Type",   nst_t_type);
    map_set_str(vars, "Int",    nst_t_int );
    map_set_str(vars, "Real",   nst_t_real);
    map_set_str(vars, "Bool",   nst_t_bool);
    map_set_str(vars, "Null",   nst_t_null);
    map_set_str(vars, "Str",    nst_t_str );
    map_set_str(vars, "Array",  nst_t_arr );
    map_set_str(vars, "Vector", nst_t_vect);
    map_set_str(vars, "Map",    nst_t_map );
    map_set_str(vars, "Func",   nst_t_func);
    map_set_str(vars, "Iter",   nst_t_iter);
    map_set_str(vars, "Byte",   nst_t_byte);
    map_set_str(vars, "IOfile", nst_t_file);

    map_set_str(vars, "true", nst_true);
    map_set_str(vars, "false", nst_false);
    map_set_str(vars, "null", nst_null);

    map_set_str(vars, "_cwd_", (Nst_Obj *)cwd);
    map_set_str(vars, "_args_", (Nst_Obj *)args);

    dec_ref(args);

    return vt;
}

Nst_Obj *get_val(VarTable *vt, Nst_Obj *name)
{
    Nst_Obj *val = map_get(vt->vars, name);

    if ( val == NULL && vt->global_table != NULL )
        val = map_get(vt->global_table->vars, name);

    if ( val == NULL )
    {
        inc_ref(nst_null);
        return nst_null;
    }
    return val;
}

void set_val(VarTable *vt, Nst_Obj *name, Nst_Obj *val)
{
    map_set(vt->vars, name, val);
}
