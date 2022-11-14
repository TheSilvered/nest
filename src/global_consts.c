#include "global_consts.h"

Nst_TypeObjs nst_t;
Nst_StrConsts nst_s;
Nst_Consts nst_c;

static bool nst_t_initialized = false;
static bool nst_s_initialized = false;
static bool nst_c_initialized = false;

void _nst_init_types()
{
    if ( nst_t_initialized )
        return;

    nst_t.Type   = NULL;
    nst_t.Type   = nst_new_type_obj("Type",   4);
    nst_t.Int    = nst_new_type_obj("Int",    3);
    nst_t.Real   = nst_new_type_obj("Real",   4);
    nst_t.Bool   = nst_new_type_obj("Bool",   4);
    nst_t.Null   = nst_new_type_obj("Null",   4);
    nst_t.Str    = nst_new_type_obj("Str",    3);
    nst_t.Array  = nst_new_type_obj("Array",  5);
    nst_t.Vector = nst_new_type_obj("Vector", 6);
    nst_t.Map    = nst_new_type_obj("Map",    3);
    nst_t.Func   = nst_new_type_obj("Func",   4);
    nst_t.Iter   = nst_new_type_obj("Iter",   4);
    nst_t.Byte   = nst_new_type_obj("Byte",   4);
    nst_t.IOFile = nst_new_type_obj("IOFile", 6);

    nst_t_initialized = true;
}

void _nst_del_types()
{
    if ( !nst_t_initialized )
        return;

    nst_dec_ref(nst_t.Type);
    nst_dec_ref(nst_t.Int);
    nst_dec_ref(nst_t.Real);
    nst_dec_ref(nst_t.Bool);
    nst_dec_ref(nst_t.Null);
    nst_dec_ref(nst_t.Str);
    nst_dec_ref(nst_t.Array);
    nst_dec_ref(nst_t.Vector);
    nst_dec_ref(nst_t.Map);
    nst_dec_ref(nst_t.Func);
    nst_dec_ref(nst_t.Iter);
    nst_dec_ref(nst_t.Byte);
    nst_dec_ref(nst_t.IOFile);

    nst_t_initialized = false;
}

void _nst_init_strs()
{
    if ( nst_s_initialized )
        return;

    nst_s.t_Type   = STR(_nst_copy_string(nst_t.Type));
    nst_s.t_Int    = STR(_nst_copy_string(nst_t.Int));
    nst_s.t_Real   = STR(_nst_copy_string(nst_t.Real));
    nst_s.t_Bool   = STR(_nst_copy_string(nst_t.Bool));
    nst_s.t_Null   = STR(_nst_copy_string(nst_t.Null));
    nst_s.t_Str    = STR(_nst_copy_string(nst_t.Str));
    nst_s.t_Array  = STR(_nst_copy_string(nst_t.Array));
    nst_s.t_Vector = STR(_nst_copy_string(nst_t.Vector));
    nst_s.t_Map    = STR(_nst_copy_string(nst_t.Map));
    nst_s.t_Func   = STR(_nst_copy_string(nst_t.Func));
    nst_s.t_Iter   = STR(_nst_copy_string(nst_t.Iter));
    nst_s.t_Byte   = STR(_nst_copy_string(nst_t.Byte));
    nst_s.t_IOFile = STR(_nst_copy_string(nst_t.IOFile));

    nst_s.c_true  = STR(nst_new_cstring("true",  4, false));
    nst_s.c_false = STR(nst_new_cstring("false", 5, false));
    nst_s.c_null  = STR(nst_new_cstring("null",  4, false));

    nst_s.e_SyntaxError = STR(nst_new_cstring("Syntax Error", 12, false));
    nst_s.e_MemoryError = STR(nst_new_cstring("Memory Error", 12, false));
    nst_s.e_ValueError  = STR(nst_new_cstring("Value Error",  11, false));
    nst_s.e_TypeError   = STR(nst_new_cstring("Type Error",   10, false));
    nst_s.e_CallError   = STR(nst_new_cstring("Call Error",   10, false));
    nst_s.e_MathError   = STR(nst_new_cstring("Math Error",   10, false));
    nst_s.e_ImportError = STR(nst_new_cstring("Import Error", 12, false));

    nst_s.o__vars_    = STR(nst_new_cstring("_vars_",    6, false));
    nst_s.o__globals_ = STR(nst_new_cstring("_globals_", 9, false));
    nst_s.o__args_    = STR(nst_new_cstring("_args_",    6, false));
    nst_s.o__cwd_     = STR(nst_new_cstring("_cwd_",     5, false));

    nst_s_initialized = true;
}

void _nst_del_strs()
{
    if ( !nst_s_initialized )
        return;

    nst_dec_ref(nst_s.t_Type);
    nst_dec_ref(nst_s.t_Int);
    nst_dec_ref(nst_s.t_Real);
    nst_dec_ref(nst_s.t_Bool);
    nst_dec_ref(nst_s.t_Null);
    nst_dec_ref(nst_s.t_Str);
    nst_dec_ref(nst_s.t_Array);
    nst_dec_ref(nst_s.t_Vector);
    nst_dec_ref(nst_s.t_Map);
    nst_dec_ref(nst_s.t_Func);
    nst_dec_ref(nst_s.t_Iter);
    nst_dec_ref(nst_s.t_Byte);
    nst_dec_ref(nst_s.t_IOFile);

    nst_dec_ref(nst_s.c_true);
    nst_dec_ref(nst_s.c_false);
    nst_dec_ref(nst_s.c_null);

    nst_dec_ref(nst_s.e_SyntaxError);
    nst_dec_ref(nst_s.e_MemoryError);
    nst_dec_ref(nst_s.e_ValueError);
    nst_dec_ref(nst_s.e_TypeError);
    nst_dec_ref(nst_s.e_CallError);
    nst_dec_ref(nst_s.e_MathError);
    nst_dec_ref(nst_s.e_ImportError);

    nst_dec_ref(nst_s.o__args_);
    nst_dec_ref(nst_s.o__cwd_);
    nst_dec_ref(nst_s.o__globals_);
    nst_dec_ref(nst_s.o__vars_);

    nst_s_initialized = false;
}

void _nst_init_consts()
{
    if ( nst_c_initialized )
        return;

    nst_c.b_true  = nst_new_bool(NST_TRUE);
    nst_c.b_false = nst_new_bool(NST_FALSE);
    nst_c.null = _nst_alloc_obj(sizeof(Nst_Obj), nst_t.Null, NULL);

    nst_c_initialized = true;
}

void _nst_del_consts()
{
    if ( !nst_c_initialized )
        return;

    nst_dec_ref(nst_c.b_true);
    nst_dec_ref(nst_c.b_false);
    nst_dec_ref(nst_c.null);

    nst_c_initialized = false;
}