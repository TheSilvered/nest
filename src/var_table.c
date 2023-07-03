#include "lib_import.h"
#include "var_table.h"
#include "global_consts.h"
#include "mem.h"

Nst_VarTable *nst_vt_new(Nst_MapObj *global_table,
                         Nst_StrObj *cwd,
                         Nst_SeqObj *args)
{
    Nst_VarTable *vt = nst_malloc_c(1, Nst_VarTable);
    if ( vt == NULL )
    {
        return NULL;
    }

    vt->global_table = global_table;
    Nst_MapObj *vars = MAP(nst_map_new());
    if ( vars == NULL )
    {
        return NULL;
    }
    vt->vars = vars;
    bool res = true;
    res = res && nst_map_set(vars, nst_s.o__vars_, vars);
    if ( global_table == NULL )
    {
        res = res && nst_map_set(vars, nst_s.o__globals_, nst_c.Null_null);
    }
    else
    {
        res = res && nst_map_set(vars, nst_s.o__globals_, global_table);
        if ( !res )
        {
            nst_dec_ref(vars);
            nst_free(vt);
            nst_failed_allocation();
            return NULL;
        }
        nst_inc_ref(global_table);
        return vt;
    }

    res = res && nst_map_set(vars, nst_s.t_Type,   nst_t.Type);
    res = res && nst_map_set(vars, nst_s.t_Int,    nst_t.Int);
    res = res && nst_map_set(vars, nst_s.t_Real,   nst_t.Real);
    res = res && nst_map_set(vars, nst_s.t_Bool,   nst_t.Bool);
    res = res && nst_map_set(vars, nst_s.t_Null,   nst_t.Null);
    res = res && nst_map_set(vars, nst_s.t_Str,    nst_t.Str);
    res = res && nst_map_set(vars, nst_s.t_Array,  nst_t.Array);
    res = res && nst_map_set(vars, nst_s.t_Vector, nst_t.Vector);
    res = res && nst_map_set(vars, nst_s.t_Map,    nst_t.Map);
    res = res && nst_map_set(vars, nst_s.t_Func,   nst_t.Func);
    res = res && nst_map_set(vars, nst_s.t_Iter,   nst_t.Iter);
    res = res && nst_map_set(vars, nst_s.t_Byte,   nst_t.Byte);
    res = res && nst_map_set(vars, nst_s.t_IOFile, nst_t.IOFile);

    res = res && nst_map_set(vars, nst_s.c_true,  nst_c.Bool_true);
    res = res && nst_map_set(vars, nst_s.c_false, nst_c.Bool_false);
    res = res && nst_map_set(vars, nst_s.c_null,  nst_c.Null_null);

    res = res && nst_map_set(vars, nst_s.o__cwd_, cwd);
    res = res && nst_map_set(vars, nst_s.o__args_, args);

    if ( !res )
    {
        nst_map_drop(vars, nst_s.o__vars_);
        nst_dec_ref(vars);
        nst_free(vt);
        nst_failed_allocation();
        return NULL;
    }
    return vt;
}

Nst_Obj *_nst_vt_get(Nst_VarTable *vt, Nst_Obj *name)
{
    Nst_Obj *val = _nst_map_get(vt->vars, name);

    if ( val == NULL && vt->global_table != NULL )
    {
        val = _nst_map_get(vt->global_table, name);
    }

    if ( val == NULL )
    {
        nst_inc_ref(nst_c.Null_null);
        return nst_c.Null_null;
    }
    return val;
}

bool _nst_vt_set(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val)
{
    if ( !nst_map_set(vt->vars, name, val) )
    {
        return false;
    }
    return true;
}
