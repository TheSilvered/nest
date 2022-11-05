#include <stdlib.h>
#include "var_table.h"
#include "global_consts.h"

Nst_VarTable *nst_new_var_table(Nst_MapObj *global_table,
                                Nst_StrObj *cwd,
                                Nst_SeqObj *args)
{
    Nst_VarTable *vt = (Nst_VarTable *)malloc(sizeof(Nst_VarTable));
    if ( vt == NULL ) return NULL;

    vt->global_table = global_table;
    Nst_MapObj *vars = MAP(nst_new_map());
    vt->vars = vars;

    nst_map_set(vars, nst_s.o__vars_, vars);
    if ( global_table == NULL )
        nst_map_set_str(vars, nst_s.o__globals_, nst_c.null);
    else
        nst_map_set_str(vars, nst_s.o__globals_, global_table);

    if ( global_table != NULL )
        return vt;

    nst_map_set(vars, nst_s.t_Type,   nst_t.Type);
    nst_map_set(vars, nst_s.t_Int,    nst_t.Int );
    nst_map_set(vars, nst_s.t_Real,   nst_t.Real);
    nst_map_set(vars, nst_s.t_Bool,   nst_t.Bool);
    nst_map_set(vars, nst_s.t_Null,   nst_t.Null);
    nst_map_set(vars, nst_s.t_Str,    nst_t.Str );
    nst_map_set(vars, nst_s.t_Array,  nst_t.Array );
    nst_map_set(vars, nst_s.t_Vector, nst_t.Vector);
    nst_map_set(vars, nst_s.t_Map,    nst_t.Map );
    nst_map_set(vars, nst_s.t_Func,   nst_t.Func);
    nst_map_set(vars, nst_s.t_Iter,   nst_t.Iter);
    nst_map_set(vars, nst_s.t_Byte,   nst_t.Byte);
    nst_map_set(vars, nst_s.t_IOFile, nst_t.IOFile);

    nst_map_set(vars, nst_s.c_true,  nst_c.b_true);
    nst_map_set(vars, nst_s.c_false, nst_c.b_false);
    nst_map_set(vars, nst_s.c_null,  nst_c.null);

    nst_map_set(vars, nst_s.o__cwd_, (Nst_Obj *)cwd);
    nst_map_set(vars, nst_s.o__args_, (Nst_Obj *)args);

    return vt;
}

Nst_Obj *_nst_get_val(Nst_VarTable *vt, Nst_Obj *name)
{
    Nst_Obj *val = _nst_map_get(vt->vars, name);

    if ( val == NULL && vt->global_table != NULL )
        val = _nst_map_get(vt->global_table, name);

    if ( val == NULL )
    {
        nst_inc_ref(nst_c.null);
        return nst_c.null;
    }
    return val;
}

void _nst_set_val(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val)
{
    nst_map_set(vt->vars, name, val);
}
