#include <stdlib.h>
#include "var_table.h"
#include "nst_types.h"

VarTable *new_var_table(VarTable *global_table)
{
    VarTable *vt = malloc(sizeof(VarTable));
    if ( vt == NULL ) return NULL;

    vt->global_table = global_table;

    if ( vt->global_table != NULL )
        return vt;

    Nst_Obj *key_type = make_obj(new_string_raw("type",   false), nst_t_str, destroy_string);
    Nst_Obj *key_int  = make_obj(new_string_raw("int",    false), nst_t_str, destroy_string);
    Nst_Obj *key_real = make_obj(new_string_raw("real",   false), nst_t_str, destroy_string);
    Nst_Obj *key_bool = make_obj(new_string_raw("bool",   false), nst_t_str, destroy_string);
    Nst_Obj *key_null = make_obj(new_string_raw("null",   false), nst_t_str, destroy_string);
    Nst_Obj *key_str  = make_obj(new_string_raw("str",    false), nst_t_str, destroy_string);
    Nst_Obj *key_arr  = make_obj(new_string_raw("array",  false), nst_t_str, destroy_string);
    Nst_Obj *key_vect = make_obj(new_string_raw("vector", false), nst_t_str, destroy_string);
    Nst_Obj *key_map  = make_obj(new_string_raw("map",    false), nst_t_str, destroy_string);
    Nst_Obj *key_func = make_obj(new_string_raw("func",   false), nst_t_str, destroy_string);
    Nst_Obj *key_iter = make_obj(new_string_raw("iter",   false), nst_t_str, destroy_string);
    Nst_Obj *key_byte = make_obj(new_string_raw("byte",   false), nst_t_str, destroy_string);
    Nst_Obj *key_file = make_obj(new_string_raw("iofile", false), nst_t_str, destroy_string);

    Nst_Obj *key_const_true  = make_obj(new_string_raw("TRUE",  false), nst_t_str, destroy_string);
    Nst_Obj *key_const_false = make_obj(new_string_raw("FALSE", false), nst_t_str, destroy_string);
    Nst_Obj *key_const_null  = make_obj(new_string_raw("NULL",  false), nst_t_str, destroy_string);

    map_set(vt->vars, key_type, nst_t_type);
    map_set(vt->vars, key_int , nst_t_int );
    map_set(vt->vars, key_real, nst_t_real);
    map_set(vt->vars, key_bool, nst_t_bool);
    map_set(vt->vars, key_null, nst_t_null);
    map_set(vt->vars, key_str , nst_t_str );
    map_set(vt->vars, key_arr , nst_t_arr );
    map_set(vt->vars, key_vect, nst_t_vect);
    map_set(vt->vars, key_map , nst_t_map );
    map_set(vt->vars, key_func, nst_t_func);
    map_set(vt->vars, key_iter, nst_t_iter);
    map_set(vt->vars, key_byte, nst_t_byte);
    map_set(vt->vars, key_file, nst_t_file);
    map_set(vt->vars, key_const_true,  nst_true);
    map_set(vt->vars, key_const_false, nst_false);
    map_set(vt->vars, key_const_null,  nst_null);

    // ensures that these never get freed
    inc_ref(key_type);
    inc_ref(key_int);
    inc_ref(key_real);
    inc_ref(key_bool);
    inc_ref(key_null);
    inc_ref(key_str);
    inc_ref(key_arr);
    inc_ref(key_vect);
    inc_ref(key_map);
    inc_ref(key_func);
    inc_ref(key_iter);
    inc_ref(key_byte);
    inc_ref(key_file);
    inc_ref(key_const_true);
    inc_ref(key_const_false);
    inc_ref(key_const_null);

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
