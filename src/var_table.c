#include "nest.h"

bool Nst_vt_init(Nst_VarTable *vt, Nst_Obj *global_table, Nst_Obj *args,
                 bool no_default)
{
    Nst_Obj *vars = Nst_map_new();
    if (vars == NULL)
        return false;
    vt->vars = vars;
    vt->global_table = no_default ? NULL : global_table;
    Nst_map_set(vars, Nst_s.o__vars_, vars);

#ifdef _DEBUG
#ifdef _Nst_ARCH_x64
    Nst_Obj *debug_str = Nst_str_new_c("x64");
#else
    Nst_Obj *debug_str = Nst_str_new_c("x86");
#endif // !_Nst_ARCH_x64
    Nst_map_set_str(vars, "_debug_", Nst_c.Bool_true);
    Nst_map_set_str(vars, "_debug_arch_", debug_str);
    Nst_dec_ref(debug_str);
#endif // !_DEBUG

    if (no_default)
        return true;

    if (global_table == NULL)
        Nst_map_set(vars, Nst_s.o__globals_, Nst_c.Null_null);
    else {
        Nst_assert(global_table->type == Nst_t.Map);
        Nst_inc_ref(global_table);
        Nst_map_set(vars, Nst_s.o__globals_, global_table);
        if (Nst_error_occurred()) {
            Nst_map_drop(vars, Nst_s.o__vars_);
            Nst_dec_ref(vars);
            return false;
        }
        return true;
    }

    Nst_map_set(vars, Nst_s.t_Type,   Nst_t.Type);
    Nst_map_set(vars, Nst_s.t_Int,    Nst_t.Int);
    Nst_map_set(vars, Nst_s.t_Real,   Nst_t.Real);
    Nst_map_set(vars, Nst_s.t_Bool,   Nst_t.Bool);
    Nst_map_set(vars, Nst_s.t_Null,   Nst_t.Null);
    Nst_map_set(vars, Nst_s.t_Str,    Nst_t.Str);
    Nst_map_set(vars, Nst_s.t_Array,  Nst_t.Array);
    Nst_map_set(vars, Nst_s.t_Vector, Nst_t.Vector);
    Nst_map_set(vars, Nst_s.t_Map,    Nst_t.Map);
    Nst_map_set(vars, Nst_s.t_Func,   Nst_t.Func);
    Nst_map_set(vars, Nst_s.t_Iter,   Nst_t.Iter);
    Nst_map_set(vars, Nst_s.t_Byte,   Nst_t.Byte);
    Nst_map_set(vars, Nst_s.t_IOFile, Nst_t.IOFile);

    Nst_map_set(vars, Nst_s.c_true,  Nst_c.Bool_true);
    Nst_map_set(vars, Nst_s.c_false, Nst_c.Bool_false);
    Nst_map_set(vars, Nst_s.c_null,  Nst_c.Null_null);

    Nst_map_set(vars, Nst_s.o__args_, args);

    if (Nst_error_occurred()) {
        Nst_map_drop(vars, Nst_s.o__vars_);
        Nst_dec_ref(vars);
        return false;
    }
    return true;
}

Nst_ObjRef *Nst_vt_get(Nst_VarTable vt, Nst_Obj *name)
{
    Nst_Obj *val = Nst_map_get(vt.vars, name);

    if (val == NULL && vt.global_table != NULL)
        val = Nst_map_get(vt.global_table, name);

    if (val == NULL)
        return Nst_null_ref();
    return val;
}

bool Nst_vt_set(Nst_VarTable vt, Nst_Obj *name, Nst_Obj *val)
{
    return Nst_map_set(vt.vars, name, val);
}

void Nst_vt_destroy(Nst_VarTable *vt)
{
    if (vt->vars != NULL) {
        Nst_Obj *vars = Nst_map_get(vt->vars, Nst_s.o__vars_);
        if (vars == vt->vars) {
            Nst_map_drop(vt->vars, Nst_s.o__vars_);
            Nst_ndec_ref(vars);
        }
        Nst_dec_ref(vt->vars);
    }
    Nst_ndec_ref(vt->global_table);
    vt->vars = NULL;
    vt->global_table = NULL;
}
