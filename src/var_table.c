#include "lib_import.h"
#include "var_table.h"
#include "global_consts.h"
#include "mem.h"

Nst_VarTable *nst_vt_new(Nst_MapObj *global_table,
                         Nst_StrObj *cwd,
                         Nst_SeqObj *args,
                         bool        no_default)
{
    Nst_VarTable *vt = nst_malloc_c(1, Nst_VarTable);
    if ( vt == NULL )
    {
        return NULL;
    }
    Nst_MapObj *vars = MAP(nst_map_new());
    if ( vars == NULL )
    {
        nst_free(vt);
        return NULL;
    }
    vt->vars = vars;
    vt->global_table = no_default ? NULL : global_table;
    nst_map_set(vars, nst_s.o__vars_, vars);

    if ( no_default )
    {
        return vt;
    }

    if ( global_table == NULL )
    {
        nst_map_set(vars, nst_s.o__globals_, nst_c.Null_null);
    }
    else
    {
        nst_inc_ref(global_table);
        nst_map_set(vars, nst_s.o__globals_, global_table);
        if ( Nst_error_occurred() )
        {
            nst_map_drop(vars, nst_s.o__vars_);
            nst_dec_ref(vars);
            nst_free(vt);
            return NULL;
        }
        return vt;
    }

    nst_map_set(vars, nst_s.t_Type,   nst_t.Type);
    nst_map_set(vars, nst_s.t_Int,    nst_t.Int);
    nst_map_set(vars, nst_s.t_Real,   nst_t.Real);
    nst_map_set(vars, nst_s.t_Bool,   nst_t.Bool);
    nst_map_set(vars, nst_s.t_Null,   nst_t.Null);
    nst_map_set(vars, nst_s.t_Str,    nst_t.Str);
    nst_map_set(vars, nst_s.t_Array,  nst_t.Array);
    nst_map_set(vars, nst_s.t_Vector, nst_t.Vector);
    nst_map_set(vars, nst_s.t_Map,    nst_t.Map);
    nst_map_set(vars, nst_s.t_Func,   nst_t.Func);
    nst_map_set(vars, nst_s.t_Iter,   nst_t.Iter);
    nst_map_set(vars, nst_s.t_Byte,   nst_t.Byte);
    nst_map_set(vars, nst_s.t_IOFile, nst_t.IOFile);

    nst_map_set(vars, nst_s.c_true,  nst_c.Bool_true);
    nst_map_set(vars, nst_s.c_false, nst_c.Bool_false);
    nst_map_set(vars, nst_s.c_null,  nst_c.Null_null);

    nst_map_set(vars, nst_s.o__cwd_, cwd);
    nst_map_set(vars, nst_s.o__args_, args);

    if ( Nst_error_occurred() )
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

bool _nst_vt_set(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val)
{
    if ( !nst_map_set(vt->vars, name, val) )
    {
        return false;
    }
    return true;
}

void nst_vt_destroy(Nst_VarTable *vt)
{
    nst_map_drop(vt->vars, nst_s.o__vars_);
    nst_dec_ref(vt->vars);
    if ( nst_state.vt->global_table != NULL )
    {
        nst_dec_ref(nst_state.vt->global_table);
    }
    nst_free(vt);
}

Nst_VarTable *nst_vt_from_func(Nst_FuncObj *f)
{
    if ( f->mod_globals != NULL )
    {
        return nst_vt_new(f->mod_globals, NULL, NULL, false);
    }
    else if ( nst_state.vt->global_table == NULL )
    {
        return nst_vt_new(nst_state.vt->vars, NULL, NULL, false);
    }
    return nst_vt_new(nst_state.vt->global_table, NULL, NULL, false);
}
