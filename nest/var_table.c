#include <stdlib.h>
#include "var_table.h"

Nst_VarTable *nst_new_var_table(Nst_MapObj *global_table,
                                Nst_StrObj *cwd,
                                Nst_SeqObj *args)
{
    Nst_VarTable *vt = (Nst_VarTable *)malloc(sizeof(Nst_VarTable));
    if ( vt == NULL ) return NULL;

    vt->global_table = global_table;
    Nst_MapObj *vars = AS_MAP(nst_new_map());
    vt->vars = vars;

    nst_map_set_str(vars, "_vars_", vars);
    if ( global_table == NULL )
        nst_map_set_str(vars, "_globals_", nst_null);
    else
        nst_map_set_str(vars, "_globals_", global_table);

    if ( global_table != NULL )
        return vt;

    _nst_map_set_str(vars, "Type",   nst_t_type);
    _nst_map_set_str(vars, "Int",    nst_t_int );
    _nst_map_set_str(vars, "Real",   nst_t_real);
    _nst_map_set_str(vars, "Bool",   nst_t_bool);
    _nst_map_set_str(vars, "Null",   nst_t_null);
    _nst_map_set_str(vars, "Str",    nst_t_str );
    _nst_map_set_str(vars, "Array",  nst_t_arr );
    _nst_map_set_str(vars, "Vector", nst_t_vect);
    _nst_map_set_str(vars, "Map",    nst_t_map );
    _nst_map_set_str(vars, "Func",   nst_t_func);
    _nst_map_set_str(vars, "Iter",   nst_t_iter);
    _nst_map_set_str(vars, "Byte",   nst_t_byte);
    _nst_map_set_str(vars, "IOFile", nst_t_file);

    _nst_map_set_str(vars, "true", nst_true);
    _nst_map_set_str(vars, "false", nst_false);
    _nst_map_set_str(vars, "null", nst_null);

    _nst_map_set_str(vars, "_cwd_", (Nst_Obj *)cwd);
    _nst_map_set_str(vars, "_args_", (Nst_Obj *)args);

    return vt;
}

Nst_Obj *_nst_get_val(Nst_VarTable *vt, Nst_Obj *name)
{
    Nst_Obj *val = _nst_map_get(vt->vars, name);

    if ( val == NULL && vt->global_table != NULL )
        val = _nst_map_get(vt->global_table, name);

    if ( val == NULL )
    {
        nst_inc_ref(nst_null);
        return nst_null;
    }
    return val;
}

void _nst_set_val(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val)
{
    nst_map_set(vt->vars, name, val);
}
