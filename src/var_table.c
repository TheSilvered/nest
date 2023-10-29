#include "lib_import.h"
#include "var_table.h"
#include "global_consts.h"
#include "mem.h"

Nst_VarTable *Nst_vt_new(Nst_MapObj *global_table, Nst_StrObj *cwd,
                         Nst_SeqObj *args, bool no_default)
{
    Nst_VarTable *vt = Nst_malloc_c(1, Nst_VarTable);
    if (vt == NULL)
        return NULL;
    Nst_MapObj *vars = MAP(Nst_map_new());
    if (vars == NULL) {
        Nst_free(vt);
        return NULL;
    }
    vt->vars = vars;
    vt->global_table = no_default ? NULL : global_table;
    Nst_map_set(vars, Nst_s.o__vars_, vars);

    if (no_default)
        return vt;

    if (global_table == NULL)
        Nst_map_set(vars, Nst_s.o__globals_, Nst_c.Null_null);
    else {
        Nst_inc_ref(global_table);
        Nst_map_set(vars, Nst_s.o__globals_, global_table);
        if (Nst_error_occurred()) {
            Nst_map_drop(vars, Nst_s.o__vars_);
            Nst_dec_ref(vars);
            Nst_free(vt);
            return NULL;
        }
        return vt;
    }

    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Type),   Nst_t.Type);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Int),    Nst_t.Int);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Real),   Nst_t.Real);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Bool),   Nst_t.Bool);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Null),   Nst_t.Null);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Str),    Nst_t.Str);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Array),  Nst_t.Array);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Vector), Nst_t.Vector);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Map),    Nst_t.Map);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Func),   Nst_t.Func);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Iter),   Nst_t.Iter);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.Byte),   Nst_t.Byte);
    Nst_map_set(vars, Nst_TYPE_STR(Nst_t.IOFile), Nst_t.IOFile);

    Nst_map_set(vars, Nst_s.c_true,  Nst_c.Bool_true);
    Nst_map_set(vars, Nst_s.c_false, Nst_c.Bool_false);
    Nst_map_set(vars, Nst_s.c_null,  Nst_c.Null_null);

    Nst_map_set(vars, Nst_s.o__cwd_, cwd);
    Nst_map_set(vars, Nst_s.o__args_, args);

    if (Nst_error_occurred()) {
        Nst_map_drop(vars, Nst_s.o__vars_);
        Nst_dec_ref(vars);
        Nst_free(vt);
        return NULL;
    }
    return vt;
}

Nst_Obj *_Nst_vt_get(Nst_VarTable *vt, Nst_Obj *name)
{
    Nst_Obj *val = _Nst_map_get(vt->vars, name);

    if (val == NULL && vt->global_table != NULL)
        val = _Nst_map_get(vt->global_table, name);

    if (val == NULL)
        Nst_RETURN_NULL;
    return val;
}

bool _Nst_vt_set(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val)
{
    return Nst_map_set(vt->vars, name, val);
}

void Nst_vt_destroy(Nst_VarTable *vt)
{
    Nst_Obj *vars = Nst_map_drop(vt->vars, Nst_s.o__vars_);
    Nst_ndec_ref(vars);
    Nst_dec_ref(vt->vars);
    if (Nst_state.vt->global_table != NULL)
        Nst_dec_ref(Nst_state.vt->global_table);
    Nst_free(vt);
}

Nst_VarTable *Nst_vt_from_func(Nst_FuncObj *f)
{
    if (f->mod_globals != NULL)
        return Nst_vt_new(f->mod_globals, NULL, NULL, false);
    else if (Nst_state.vt->global_table == NULL)
        return Nst_vt_new(Nst_state.vt->vars, NULL, NULL, false);
    return Nst_vt_new(Nst_state.vt->global_table, NULL, NULL, false);
}
