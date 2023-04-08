#include <string.h>
#include "global_consts.h"
#include "lib_import.h"
#include "mem.h"

Nst_TypeObjs nst_t;
Nst_StrConsts nst_s;
Nst_Consts nst_c;
Nst_StdStreams *nst_io;

static Nst_StdStreams local_nst_io;

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

    nst_io = &local_nst_io;

    local_nst_io.in  = IOFILE(nst_iof_new(stdin,  false, true, false, &err));
    local_nst_io.out = IOFILE(nst_iof_new(stdout, false, false, true, &err));
    local_nst_io.err = IOFILE(nst_iof_new(stderr, false, false, true, &err));

    local_nst_io.in ->close_f = close_std_stream;
    local_nst_io.out->close_f = close_std_stream;
    local_nst_io.err->close_f = close_std_stream;

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
    if ( nst_t.Type )   nst_dec_ref(nst_t.Type);
    if ( nst_t.Int )    nst_dec_ref(nst_t.Int);
    if ( nst_t.Real )   nst_dec_ref(nst_t.Real);
    if ( nst_t.Bool )   nst_dec_ref(nst_t.Bool);
    if ( nst_t.Null )   nst_dec_ref(nst_t.Null);
    if ( nst_t.Str )    nst_dec_ref(nst_t.Str);
    if ( nst_t.Array )  nst_dec_ref(nst_t.Array);
    if ( nst_t.Vector ) nst_dec_ref(nst_t.Vector);
    if ( nst_t.Map )    nst_dec_ref(nst_t.Map);
    if ( nst_t.Func )   nst_dec_ref(nst_t.Func);
    if ( nst_t.Iter )   nst_dec_ref(nst_t.Iter);
    if ( nst_t.Byte )   nst_dec_ref(nst_t.Byte);
    if ( nst_t.IOFile ) nst_dec_ref(nst_t.IOFile);

    if ( nst_s.t_Type )   nst_dec_ref(nst_s.t_Type);
    if ( nst_s.t_Int )    nst_dec_ref(nst_s.t_Int);
    if ( nst_s.t_Real )   nst_dec_ref(nst_s.t_Real);
    if ( nst_s.t_Bool )   nst_dec_ref(nst_s.t_Bool);
    if ( nst_s.t_Null )   nst_dec_ref(nst_s.t_Null);
    if ( nst_s.t_Str )    nst_dec_ref(nst_s.t_Str);
    if ( nst_s.t_Array )  nst_dec_ref(nst_s.t_Array);
    if ( nst_s.t_Vector ) nst_dec_ref(nst_s.t_Vector);
    if ( nst_s.t_Map )    nst_dec_ref(nst_s.t_Map);
    if ( nst_s.t_Func )   nst_dec_ref(nst_s.t_Func);
    if ( nst_s.t_Iter )   nst_dec_ref(nst_s.t_Iter);
    if ( nst_s.t_Byte )   nst_dec_ref(nst_s.t_Byte);
    if ( nst_s.t_IOFile ) nst_dec_ref(nst_s.t_IOFile);

    if ( nst_s.c_true )  nst_dec_ref(nst_s.c_true);
    if ( nst_s.c_false ) nst_dec_ref(nst_s.c_false);
    if ( nst_s.c_null )  nst_dec_ref(nst_s.c_null);

    if ( nst_s.e_SyntaxError ) nst_dec_ref(nst_s.e_SyntaxError);
    if ( nst_s.e_MemoryError ) nst_dec_ref(nst_s.e_MemoryError);
    if ( nst_s.e_ValueError )  nst_dec_ref(nst_s.e_ValueError);
    if ( nst_s.e_TypeError )   nst_dec_ref(nst_s.e_TypeError);
    if ( nst_s.e_CallError )   nst_dec_ref(nst_s.e_CallError);
    if ( nst_s.e_MathError )   nst_dec_ref(nst_s.e_MathError);
    if ( nst_s.e_ImportError ) nst_dec_ref(nst_s.e_ImportError);

    if ( nst_s.o__args_ )       nst_dec_ref(nst_s.o__args_);
    if ( nst_s.o__cwd_ )        nst_dec_ref(nst_s.o__cwd_);
    if ( nst_s.o__globals_ )    nst_dec_ref(nst_s.o__globals_);
    if ( nst_s.o__vars_ )       nst_dec_ref(nst_s.o__vars_);
    if ( nst_s.o_failed_alloc ) nst_dec_ref(nst_s.o_failed_alloc);

    if ( nst_c.Bool_true )  nst_dec_ref(nst_c.Bool_true);
    if ( nst_c.Bool_false ) nst_dec_ref(nst_c.Bool_false);
    if ( nst_c.Null_null )  nst_dec_ref(nst_c.Null_null);
    if ( nst_c.Int_0 )      nst_dec_ref(nst_c.Int_0);
    if ( nst_c.Int_1 )      nst_dec_ref(nst_c.Int_1);
    if ( nst_c.Int_neg1 )   nst_dec_ref(nst_c.Int_neg1);
    if ( nst_c.Real_0 )     nst_dec_ref(nst_c.Real_0);
    if ( nst_c.Real_1 )     nst_dec_ref(nst_c.Real_1);
    if ( nst_c.Byte_0 )     nst_dec_ref(nst_c.Byte_0);
    if ( nst_c.Byte_1 )     nst_dec_ref(nst_c.Byte_1);

    if ( nst_io->in )  nst_dec_ref(nst_io->in);
    if ( nst_io->out ) nst_dec_ref(nst_io->out);
    if ( nst_io->err ) nst_dec_ref(nst_io->err);
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

Nst_StdStreams *nst_stdio()
{
    return nst_io;
}

#ifdef WINDOWS
#pragma warning( disable: 4100 )
#endif

static i32 close_std_stream(void *f) { return 0; }
