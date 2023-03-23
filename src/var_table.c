#include "lib_import.h"
#include "var_table.h"
#include "global_consts.h"
#include "mem.h"

Nst_VarTable *nst_vt_new(Nst_MapObj *global_table,
                         Nst_StrObj *cwd,
                         Nst_SeqObj *args,
                         Nst_OpErr  *err)
{
    Nst_VarTable *vt = (Nst_VarTable *)nst_malloc(1, sizeof(Nst_VarTable), err);
    if ( vt == NULL )
    {
        return NULL;
    }

    vt->global_table = global_table;
    Nst_MapObj *vars = MAP(nst_map_new(err));
    if ( vars == NULL )
    {
        return NULL;
    }
    vt->vars = vars;

    nst_map_set(vars, nst_s.o__vars_, vars, err);
    if ( global_table == NULL )
    {
        nst_map_set(vars, nst_s.o__globals_, nst_c.Null_null, err);
    }
    else
    {
        nst_map_set(vars, nst_s.o__globals_, global_table, err);
        if ( NST_ERROR_OCCURRED )
        {
            nst_dec_ref(vars);
            nst_free(vt);
            return NULL;
        }
        nst_inc_ref(global_table);
        return vt;
    }

    nst_map_set(vars, nst_s.t_Type,   nst_t.Type,   err);
    nst_map_set(vars, nst_s.t_Int,    nst_t.Int,    err);
    nst_map_set(vars, nst_s.t_Real,   nst_t.Real,   err);
    nst_map_set(vars, nst_s.t_Bool,   nst_t.Bool,   err);
    nst_map_set(vars, nst_s.t_Null,   nst_t.Null,   err);
    nst_map_set(vars, nst_s.t_Str,    nst_t.Str,    err);
    nst_map_set(vars, nst_s.t_Array,  nst_t.Array,  err);
    nst_map_set(vars, nst_s.t_Vector, nst_t.Vector, err);
    nst_map_set(vars, nst_s.t_Map,    nst_t.Map,    err);
    nst_map_set(vars, nst_s.t_Func,   nst_t.Func,   err);
    nst_map_set(vars, nst_s.t_Iter,   nst_t.Iter,   err);
    nst_map_set(vars, nst_s.t_Byte,   nst_t.Byte,   err);
    nst_map_set(vars, nst_s.t_IOFile, nst_t.IOFile, err);

    nst_map_set(vars, nst_s.c_true,  nst_c.Bool_true, err);
    nst_map_set(vars, nst_s.c_false, nst_c.Bool_false, err);
    nst_map_set(vars, nst_s.c_null,  nst_c.Null_null, err);

    nst_map_set(vars, nst_s.o__cwd_, cwd, err);
    nst_map_set(vars, nst_s.o__args_, args, err);

    if ( NST_ERROR_OCCURRED )
    {
        nst_map_drop(vars, nst_s.o__vars_);
        nst_dec_ref(vars);
        nst_free(vt);
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

void _nst_vt_set(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val, Nst_OpErr *err)
{
    nst_map_set(vt->vars, name, val, err);
}
