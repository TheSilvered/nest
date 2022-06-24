#include <stdlib.h>
#include "var_table.h"
#include "nst_types.h"

VarTable *new_var_table(VarTable *global_table, Nst_string *path)
{
    VarTable *vt = malloc(sizeof(VarTable));
    if ( vt == NULL ) return NULL;

    vt->global_table = global_table;
    vt->vars = new_map();

    if ( global_table != NULL )
        return vt;

    Nst_Obj *key_type = make_obj(new_string_raw("Type",   false), nst_t_str, destroy_string);
    Nst_Obj *key_int  = make_obj(new_string_raw("Int",    false), nst_t_str, destroy_string);
    Nst_Obj *key_real = make_obj(new_string_raw("Real",   false), nst_t_str, destroy_string);
    Nst_Obj *key_bool = make_obj(new_string_raw("Bool",   false), nst_t_str, destroy_string);
    Nst_Obj *key_null = make_obj(new_string_raw("Null",   false), nst_t_str, destroy_string);
    Nst_Obj *key_str  = make_obj(new_string_raw("Str",    false), nst_t_str, destroy_string);
    Nst_Obj *key_arr  = make_obj(new_string_raw("Array",  false), nst_t_str, destroy_string);
    Nst_Obj *key_vect = make_obj(new_string_raw("Vector", false), nst_t_str, destroy_string);
    Nst_Obj *key_map  = make_obj(new_string_raw("Map",    false), nst_t_str, destroy_string);
    Nst_Obj *key_func = make_obj(new_string_raw("Func",   false), nst_t_str, destroy_string);
    Nst_Obj *key_iter = make_obj(new_string_raw("Iter",   false), nst_t_str, destroy_string);
    Nst_Obj *key_byte = make_obj(new_string_raw("Byte",   false), nst_t_str, destroy_string);
    Nst_Obj *key_file = make_obj(new_string_raw("IOfile", false), nst_t_str, destroy_string);

    Nst_Obj *key_const_true  = make_obj(new_string_raw("true",  false), nst_t_str, destroy_string);
    Nst_Obj *key_const_false = make_obj(new_string_raw("false", false), nst_t_str, destroy_string);
    Nst_Obj *key_const_null  = make_obj(new_string_raw("null",  false), nst_t_str, destroy_string);

    Nst_Obj *key_path = make_obj(new_string_raw("_cwd_", false), nst_t_str, destroy_string);

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
    map_set(vt->vars, key_path, new_str_obj(path));

    dec_ref(key_type);
    dec_ref(key_int);
    dec_ref(key_real);
    dec_ref(key_bool);
    dec_ref(key_null);
    dec_ref(key_str);
    dec_ref(key_arr);
    dec_ref(key_vect);
    dec_ref(key_map);
    dec_ref(key_func);
    dec_ref(key_iter);
    dec_ref(key_byte);
    dec_ref(key_file);
    dec_ref(key_const_true);
    dec_ref(key_const_false);
    dec_ref(key_const_null);
    dec_ref(key_path);

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

Nst_Obj *set_argv(VarTable *vt, int argc, char **argv)
{
    Nst_sequence *cmd_args = new_array_empty(argc - 1);

    for ( int i = 1; i < argc; i++ )
        set_value_seq(cmd_args, i - 1, new_str_obj(new_string_raw(argv[i], false)));

    Nst_Obj *argv_obj = new_arr_obj(cmd_args);
    Nst_Obj *argv_key = new_str_obj(new_string("_args_", 6, false));

    map_set(vt->vars, argv_key, argv_obj);

    dec_ref(argv_key);
    return argv_obj;
}
