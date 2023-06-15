#include <string.h>
#include "global_consts.h"
#include "lib_import.h"
#include "mem.h"
#include "iter.h"
#include "argv_parser.h"

#define safe_dec_ref(obj) do { if ( obj ) nst_dec_ref(obj); } while ( 0 )

Nst_TypeObjs nst_t;
Nst_StrConsts nst_s;
Nst_Consts nst_c;
Nst_StdStreams nst_io;
Nst_IterFunctions nst_itf;

static i32 close_std_stream(void *f);

static Nst_StrObj *str_obj_no_err(const i8 *value, Nst_TypeObj *type)
{
    Nst_StrObj *obj = STR(nst_raw_malloc(sizeof(Nst_StrObj)));
    if ( obj == NULL )
    {
        return NULL;
    }

    obj->ref_count = 1;
    obj->destructor = (Nst_ObjDestructor)_nst_string_destroy;
    obj->hash = -1;
    obj->flags = 0;
    obj->value = (i8 *)value;
    obj->len = strlen(value);

    // the type of the object is itself
    if ( type == NULL )
    {
        obj->type = TYPE(obj);
    }
    else
    {
        obj->type = type;
    }

    nst_inc_ref(obj->type);

    return obj;
}

bool _nst_init_objects()
{
    Nst_OpErr err = { NULL, NULL };
    nst_t.Type = str_obj_no_err("Type", NULL);
    if ( nst_t.Type == NULL )
    {
        return false;
    }
    nst_t.Str = str_obj_no_err("Str", nst_t.Type);
    if ( nst_t.Str == NULL )
    {
        nst_free(nst_t.Type);
        return false;
    }
    nst_s.e_MemoryError = str_obj_no_err("Memory Error", nst_t.Str);
    if ( nst_s.e_MemoryError == NULL )
    {
        nst_free(nst_t.Type);
        nst_free(nst_t.Str);
        return false;
    }
    nst_s.o_failed_alloc = str_obj_no_err("failed allocation", nst_t.Str);
    if ( nst_s.o_failed_alloc == NULL )
    {
        nst_free(nst_t.Type);
        nst_free(nst_t.Str);
        nst_free(nst_s.e_MemoryError);
        return false;
    }

    nst_t.Int    = nst_type_new("Int",    3, &err);
    nst_t.Real   = nst_type_new("Real",   4, &err);
    nst_t.Bool   = nst_type_new("Bool",   4, &err);
    nst_t.Null   = nst_type_new("Null",   4, &err);
    nst_t.Array  = nst_type_new("Array",  5, &err);
    nst_t.Vector = nst_type_new("Vector", 6, &err);
    nst_t.Map    = nst_type_new("Map",    3, &err);
    nst_t.Func   = nst_type_new("Func",   4, &err);
    nst_t.Iter   = nst_type_new("Iter",   4, &err);
    nst_t.Byte   = nst_type_new("Byte",   4, &err);
    nst_t.IOFile = nst_type_new("IOFile", 6, &err);

    nst_s.t_Type   = STR(_nst_string_copy(nst_t.Type,   &err));
    nst_s.t_Int    = STR(_nst_string_copy(nst_t.Int,    &err));
    nst_s.t_Real   = STR(_nst_string_copy(nst_t.Real,   &err));
    nst_s.t_Bool   = STR(_nst_string_copy(nst_t.Bool,   &err));
    nst_s.t_Null   = STR(_nst_string_copy(nst_t.Null,   &err));
    nst_s.t_Str    = STR(_nst_string_copy(nst_t.Str,    &err));
    nst_s.t_Array  = STR(_nst_string_copy(nst_t.Array,  &err));
    nst_s.t_Vector = STR(_nst_string_copy(nst_t.Vector, &err));
    nst_s.t_Map    = STR(_nst_string_copy(nst_t.Map,    &err));
    nst_s.t_Func   = STR(_nst_string_copy(nst_t.Func,   &err));
    nst_s.t_Iter   = STR(_nst_string_copy(nst_t.Iter,   &err));
    nst_s.t_Byte   = STR(_nst_string_copy(nst_t.Byte,   &err));
    nst_s.t_IOFile = STR(_nst_string_copy(nst_t.IOFile, &err));

    nst_s.c_true  = STR(nst_string_new_c("true",  4, false, &err));
    nst_s.c_false = STR(nst_string_new_c("false", 5, false, &err));
    nst_s.c_null  = STR(nst_string_new_c("null",  4, false, &err));

    nst_s.e_SyntaxError = STR(nst_string_new_c("Syntax Error", 12, false, &err));
    nst_s.e_ValueError  = STR(nst_string_new_c("Value Error",  11, false, &err));
    nst_s.e_TypeError   = STR(nst_string_new_c("Type Error",   10, false, &err));
    nst_s.e_CallError   = STR(nst_string_new_c("Call Error",   10, false, &err));
    nst_s.e_MathError   = STR(nst_string_new_c("Math Error",   10, false, &err));
    nst_s.e_ImportError = STR(nst_string_new_c("Import Error", 12, false, &err));

    nst_s.o__vars_    = STR(nst_string_new_c("_vars_",    6, false, &err));
    nst_s.o__globals_ = STR(nst_string_new_c("_globals_", 9, false, &err));
    nst_s.o__args_    = STR(nst_string_new_c("_args_",    6, false, &err));
    nst_s.o__cwd_     = STR(nst_string_new_c("_cwd_",     5, false, &err));

    nst_c.Bool_true  = nst_bool_new(NST_TRUE, &err);
    nst_c.Bool_false = nst_bool_new(NST_FALSE, &err);
    nst_c.Null_null  = _nst_obj_alloc(sizeof(Nst_Obj), nst_t.Null, NULL, &err);
    nst_c.Int_0    = nst_int_new(0, &err);
    nst_c.Int_1    = nst_int_new(1, &err);
    nst_c.Int_neg1 = nst_int_new(-1, &err);
    nst_c.Real_0   = nst_real_new(0.0, &err);
    nst_c.Real_1   = nst_real_new(1.0, &err);
    nst_c.Byte_0   = nst_byte_new(0, &err);
    nst_c.Byte_1   = nst_byte_new(1, &err);

    nst_io.in  = IOFILE(nst_iof_new(stdin,  false, true, false, &err));
    nst_io.out = IOFILE(nst_iof_new(stdout, false, false, true, &err));
    nst_io.err = IOFILE(nst_iof_new(stderr, false, false, true, &err));

    nst_io.in ->close_f = close_std_stream;
    nst_io.out->close_f = close_std_stream;
    nst_io.err->close_f = close_std_stream;

#ifdef WINDOWS
    nst_io.in->read_f = (Nst_IOFile_read_f)_nst_windows_stdin_read;
#endif

    nst_itf.range_start   = FUNC(nst_func_new_c(1, nst_iter_range_start,   &err));
    nst_itf.range_is_done = FUNC(nst_func_new_c(1, nst_iter_range_is_done, &err));
    nst_itf.range_get_val = FUNC(nst_func_new_c(1, nst_iter_range_get_val, &err));
    nst_itf.str_start     = FUNC(nst_func_new_c(1, nst_iter_str_start,     &err));
    nst_itf.str_is_done   = FUNC(nst_func_new_c(1, nst_iter_str_is_done,   &err));
    nst_itf.str_get_val   = FUNC(nst_func_new_c(1, nst_iter_str_get_val,   &err));
    nst_itf.seq_start     = FUNC(nst_func_new_c(1, nst_iter_seq_start,     &err));
    nst_itf.seq_is_done   = FUNC(nst_func_new_c(1, nst_iter_seq_is_done,   &err));
    nst_itf.seq_get_val   = FUNC(nst_func_new_c(1, nst_iter_seq_get_val,   &err));

    if ( err.name != NULL )
    {
        nst_dec_ref(err.name);
        nst_dec_ref(err.message);
        _nst_del_objects();
        return false;
    }
    return true;
}

void _nst_del_objects()
{
    safe_dec_ref(nst_t.Type);
    safe_dec_ref(nst_t.Int);
    safe_dec_ref(nst_t.Real);
    safe_dec_ref(nst_t.Bool);
    safe_dec_ref(nst_t.Null);
    safe_dec_ref(nst_t.Str);
    safe_dec_ref(nst_t.Array);
    safe_dec_ref(nst_t.Vector);
    safe_dec_ref(nst_t.Map);
    safe_dec_ref(nst_t.Func);
    safe_dec_ref(nst_t.Iter);
    safe_dec_ref(nst_t.Byte);
    safe_dec_ref(nst_t.IOFile);

    safe_dec_ref(nst_s.t_Type);
    safe_dec_ref(nst_s.t_Int);
    safe_dec_ref(nst_s.t_Real);
    safe_dec_ref(nst_s.t_Bool);
    safe_dec_ref(nst_s.t_Null);
    safe_dec_ref(nst_s.t_Str);
    safe_dec_ref(nst_s.t_Array);
    safe_dec_ref(nst_s.t_Vector);
    safe_dec_ref(nst_s.t_Map);
    safe_dec_ref(nst_s.t_Func);
    safe_dec_ref(nst_s.t_Iter);
    safe_dec_ref(nst_s.t_Byte);
    safe_dec_ref(nst_s.t_IOFile);

    safe_dec_ref(nst_s.c_true);
    safe_dec_ref(nst_s.c_false);
    safe_dec_ref(nst_s.c_null);

    safe_dec_ref(nst_s.e_SyntaxError);
    safe_dec_ref(nst_s.e_MemoryError);
    safe_dec_ref(nst_s.e_ValueError);
    safe_dec_ref(nst_s.e_TypeError);
    safe_dec_ref(nst_s.e_CallError);
    safe_dec_ref(nst_s.e_MathError);
    safe_dec_ref(nst_s.e_ImportError);

    safe_dec_ref(nst_s.o__args_);
    safe_dec_ref(nst_s.o__cwd_);
    safe_dec_ref(nst_s.o__globals_);
    safe_dec_ref(nst_s.o__vars_);
    safe_dec_ref(nst_s.o_failed_alloc);

    safe_dec_ref(nst_c.Bool_true);
    safe_dec_ref(nst_c.Bool_false);
    safe_dec_ref(nst_c.Null_null);
    safe_dec_ref(nst_c.Int_0);
    safe_dec_ref(nst_c.Int_1);
    safe_dec_ref(nst_c.Int_neg1);
    safe_dec_ref(nst_c.Real_0);
    safe_dec_ref(nst_c.Real_1);
    safe_dec_ref(nst_c.Byte_0);
    safe_dec_ref(nst_c.Byte_1);

    safe_dec_ref(nst_io.in);
    safe_dec_ref(nst_io.out);
    safe_dec_ref(nst_io.err);

    safe_dec_ref(nst_itf.range_start);
    safe_dec_ref(nst_itf.range_is_done);
    safe_dec_ref(nst_itf.range_get_val);
    safe_dec_ref(nst_itf.str_start);
    safe_dec_ref(nst_itf.str_is_done);
    safe_dec_ref(nst_itf.str_get_val);
    safe_dec_ref(nst_itf.seq_start);
    safe_dec_ref(nst_itf.seq_is_done);
    safe_dec_ref(nst_itf.seq_get_val);
}

Nst_Obj *nst_true()
{
    return nst_c.Bool_true;
}

Nst_Obj *nst_false()
{
    return nst_c.Bool_false;
}

Nst_Obj *nst_null()
{
    return nst_c.Null_null;
}

const Nst_TypeObjs *nst_type()
{
    return &nst_t;
}

const Nst_StrConsts *nst_str()
{
    return &nst_s;
}

const Nst_Consts *nst_const()
{
    return &nst_c;
}

const Nst_IterFunctions *nst_iter_func(void)
{
    return &nst_itf;
}

Nst_StdStreams *nst_stdio()
{
    return &nst_io;
}

#ifdef WINDOWS
#pragma warning( disable: 4100 )
#endif

static i32 close_std_stream(void *f) { return 0; }
