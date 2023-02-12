#include "global_consts.h"

Nst_TypeObjs nst_t;
Nst_StrConsts nst_s;
Nst_Consts nst_c;
Nst_StdStreams *nst_io;

static Nst_StdStreams local_nst_io;

static int close_std_stream(void *f);

void _nst_types_init()
{
    nst_t.Type   = NULL;
    nst_t.Type   = nst_type_new("Type",   4);
    nst_t.Int    = nst_type_new("Int",    3);
    nst_t.Real   = nst_type_new("Real",   4);
    nst_t.Bool   = nst_type_new("Bool",   4);
    nst_t.Null   = nst_type_new("Null",   4);
    nst_t.Str    = nst_type_new("Str",    3);
    nst_t.Array  = nst_type_new("Array",  5);
    nst_t.Vector = nst_type_new("Vector", 6);
    nst_t.Map    = nst_type_new("Map",    3);
    nst_t.Func   = nst_type_new("Func",   4);
    nst_t.Iter   = nst_type_new("Iter",   4);
    nst_t.Byte   = nst_type_new("Byte",   4);
    nst_t.IOFile = nst_type_new("IOFile", 6);
}

void _nst_types_del()
{
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
}

void _nst_strs_init()
{
    nst_s.t_Type   = STR(_nst_string_copy(nst_t.Type));
    nst_s.t_Int    = STR(_nst_string_copy(nst_t.Int));
    nst_s.t_Real   = STR(_nst_string_copy(nst_t.Real));
    nst_s.t_Bool   = STR(_nst_string_copy(nst_t.Bool));
    nst_s.t_Null   = STR(_nst_string_copy(nst_t.Null));
    nst_s.t_Str    = STR(_nst_string_copy(nst_t.Str));
    nst_s.t_Array  = STR(_nst_string_copy(nst_t.Array));
    nst_s.t_Vector = STR(_nst_string_copy(nst_t.Vector));
    nst_s.t_Map    = STR(_nst_string_copy(nst_t.Map));
    nst_s.t_Func   = STR(_nst_string_copy(nst_t.Func));
    nst_s.t_Iter   = STR(_nst_string_copy(nst_t.Iter));
    nst_s.t_Byte   = STR(_nst_string_copy(nst_t.Byte));
    nst_s.t_IOFile = STR(_nst_string_copy(nst_t.IOFile));

    nst_s.c_true  = STR(nst_string_new_c("true",  4, false));
    nst_s.c_false = STR(nst_string_new_c("false", 5, false));
    nst_s.c_null  = STR(nst_string_new_c("null",  4, false));

    nst_s.e_SyntaxError = STR(nst_string_new_c("Syntax Error", 12, false));
    nst_s.e_MemoryError = STR(nst_string_new_c("Memory Error", 12, false));
    nst_s.e_ValueError  = STR(nst_string_new_c("Value Error",  11, false));
    nst_s.e_TypeError   = STR(nst_string_new_c("Type Error",   10, false));
    nst_s.e_CallError   = STR(nst_string_new_c("Call Error",   10, false));
    nst_s.e_MathError   = STR(nst_string_new_c("Math Error",   10, false));
    nst_s.e_ImportError = STR(nst_string_new_c("Import Error", 12, false));

    nst_s.o__vars_    = STR(nst_string_new_c("_vars_",    6, false));
    nst_s.o__globals_ = STR(nst_string_new_c("_globals_", 9, false));
    nst_s.o__args_    = STR(nst_string_new_c("_args_",    6, false));
    nst_s.o__cwd_     = STR(nst_string_new_c("_cwd_",     5, false));

    nst_s.o_failed_alloc =
        STR(nst_string_new_c("failed allocation", 17, false));
}

void _nst_strs_del()
{
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
    nst_dec_ref(nst_s.o_failed_alloc);
}

void _nst_consts_init()
{
    nst_c.Bool_true  = nst_bool_new(NST_TRUE);
    nst_c.Bool_false = nst_bool_new(NST_FALSE);
    nst_c.Null_null    = _nst_obj_alloc(sizeof(Nst_Obj), nst_t.Null, NULL);
    nst_c.Int_0   = nst_int_new(0);
    nst_c.Int_1   = nst_int_new(1);
    nst_c.Int_neg1= nst_int_new(-1);
    nst_c.Real_0  = nst_real_new(0.0);
    nst_c.Real_1  = nst_real_new(1.0);
    nst_c.Byte_0  = nst_byte_new(0);
    nst_c.Byte_1  = nst_byte_new(1);
}

void _nst_consts_del()
{
    nst_dec_ref(nst_c.Bool_true);
    nst_dec_ref(nst_c.Bool_false);
    nst_dec_ref(nst_c.Null_null);
    nst_dec_ref(nst_c.Int_0);
    nst_dec_ref(nst_c.Int_1);
    nst_dec_ref(nst_c.Int_neg1);
    nst_dec_ref(nst_c.Real_0);
    nst_dec_ref(nst_c.Real_1);
    nst_dec_ref(nst_c.Byte_0);
    nst_dec_ref(nst_c.Byte_1);
}

void _nst_streams_init()
{
    nst_io = &local_nst_io;

    local_nst_io.in  = IOFILE(nst_iof_new(stdin,  false, true, false));
    local_nst_io.out = IOFILE(nst_iof_new(stdout, false, false, true));
    local_nst_io.err = IOFILE(nst_iof_new(stderr, false, false, true));

    local_nst_io.in ->close_f = close_std_stream;
    local_nst_io.out->close_f = close_std_stream;
    local_nst_io.err->close_f = close_std_stream;
}

void _nst_streams_del()
{
    nst_dec_ref(nst_io->in);
    nst_dec_ref(nst_io->out);
    nst_dec_ref(nst_io->err);
}

#if defined(_WIN32) || defined(WIN32)
#pragma warning( disable: 4100 )
#endif

static int close_std_stream(void *f) { return 0; }
