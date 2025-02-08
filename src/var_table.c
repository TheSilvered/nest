#include "lib_import.h"
#include "var_table.h"
#include "global_consts.h"
#include "mem.h"

Nst_VarTable *Nst_vt_new(Nst_Obj *global_table, Nst_Obj *args,
                         bool no_default)
{
    Nst_VarTable *vt = Nst_malloc_c(1, Nst_VarTable);
    if (vt == NULL)
        return NULL;
    Nst_Obj *vars = Nst_map_new();
    if (vars == NULL) {
        Nst_free(vt);
        return NULL;
    }
    vt->vars = vars;
    vt->global_table = no_default ? NULL : global_table;
    Nst_map_set(vars, OBJ(Nst_s.o__vars_), vars);

#ifdef _DEBUG
#ifdef _Nst_ARCH_x64
    Nst_Obj *debug_str = Nst_str_new_c_raw("x64", false);
#else
    Nst_Obj *debug_str = Nst_str_new_c_raw("x86", false);
#endif // !_Nst_ARCH_x64
    Nst_map_set_str(vars, "_debug_", Nst_c.Bool_true);
    Nst_map_set_str(vars, "_debug_arch_", debug_str);
    Nst_dec_ref(debug_str);
#endif // !_DEBUG

    if (no_default)
        return vt;

    if (global_table == NULL)
        Nst_map_set(vars, OBJ(Nst_s.o__globals_), Nst_c.Null_null);
    else {
        Nst_assert(global_table->type == Nst_t.Map);
        Nst_inc_ref(global_table);
        Nst_map_set(vars, OBJ(Nst_s.o__globals_), global_table);
        if (Nst_error_occurred()) {
            Nst_map_drop(vars, OBJ(Nst_s.o__vars_));
            Nst_dec_ref(vars);
            Nst_free(vt);
            return NULL;
        }
        return vt;
    }

    Nst_map_set(vars, Nst_s.t_Type,   OBJ(Nst_t.Type));
    Nst_map_set(vars, Nst_s.t_Int,    OBJ(Nst_t.Int));
    Nst_map_set(vars, Nst_s.t_Real,   OBJ(Nst_t.Real));
    Nst_map_set(vars, Nst_s.t_Bool,   OBJ(Nst_t.Bool));
    Nst_map_set(vars, Nst_s.t_Null,   OBJ(Nst_t.Null));
    Nst_map_set(vars, Nst_s.t_Str,    OBJ(Nst_t.Str));
    Nst_map_set(vars, Nst_s.t_Array,  OBJ(Nst_t.Array));
    Nst_map_set(vars, Nst_s.t_Vector, OBJ(Nst_t.Vector));
    Nst_map_set(vars, Nst_s.t_Map,    OBJ(Nst_t.Map));
    Nst_map_set(vars, Nst_s.t_Func,   OBJ(Nst_t.Func));
    Nst_map_set(vars, Nst_s.t_Iter,   OBJ(Nst_t.Iter));
    Nst_map_set(vars, Nst_s.t_Byte,   OBJ(Nst_t.Byte));
    Nst_map_set(vars, Nst_s.t_IOFile, OBJ(Nst_t.IOFile));

    Nst_map_set(vars, Nst_s.c_true,  Nst_c.Bool_true);
    Nst_map_set(vars, Nst_s.c_false, Nst_c.Bool_false);
    Nst_map_set(vars, Nst_s.c_null,  Nst_c.Null_null);

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
    Nst_Obj *val = Nst_map_get(vt->vars, name);

    if (val == NULL && vt->global_table != NULL)
        val = Nst_map_get(vt->global_table, name);

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
    Nst_Obj *vars = Nst_map_drop(vt->vars, OBJ(Nst_s.o__vars_));
    Nst_ndec_ref(vars);
    Nst_dec_ref(vt->vars);
    Nst_ndec_ref(vt->global_table);
    Nst_free(vt);
}

Nst_VarTable *Nst_vt_from_func(Nst_Obj *f)
{
    Nst_Obj *func_globals = Nst_func_mod_globals(f);
    if (func_globals != NULL)
        return Nst_vt_new(func_globals, NULL, false);
    else if (Nst_state.es->vt->global_table == NULL)
        return Nst_vt_new(Nst_state.es->vt->vars, NULL, false);
    return Nst_vt_new(Nst_state.es->vt->global_table, NULL, false);
}
