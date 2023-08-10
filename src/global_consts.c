#include <string.h>
#include "global_consts.h"
#include "lib_import.h"
#include "mem.h"
#include "iter.h"
#include "argv_parser.h"

Nst_TypeObjs Nst_t;
Nst_StrConsts Nst_s;
Nst_Consts Nst_c;
Nst_StdStreams Nst_io;
Nst_IterFunctions Nst_itf;

static int close_std_stream(void *f);

static Nst_StrObj *str_obj_no_err(const i8 *value, Nst_TypeObj *type)
{
    Nst_StrObj *obj = STR(Nst_raw_malloc(sizeof(Nst_StrObj)));
    if (obj == NULL)
        return NULL;

    obj->ref_count = 1;
    obj->destructor = (Nst_ObjDestructor)_Nst_string_destroy;
    obj->hash = -1;
    obj->flags = 0;
    obj->value = (i8 *)value;
    obj->len = strlen(value);

    // the type of the object is itself
    if (type == NULL)
        obj->type = TYPE(obj);
    else
        obj->type = type;

    Nst_inc_ref(obj->type);
    return obj;
}

bool _Nst_init_objects(void)
{
    Nst_t.Type = str_obj_no_err("Type", NULL);
    if (Nst_t.Type == NULL)
        return false;

    Nst_t.Str = str_obj_no_err("Str", Nst_t.Type);
    if (Nst_t.Str == NULL) {
        Nst_free(Nst_t.Type);
        return false;
    }
    Nst_s.e_MemoryError = str_obj_no_err("Memory Error", Nst_t.Str);
    if (Nst_s.e_MemoryError == NULL) {
        Nst_free(Nst_t.Type);
        Nst_free(Nst_t.Str);
        return false;
    }
    Nst_s.o_failed_alloc = str_obj_no_err("failed allocation", Nst_t.Str);
    if (Nst_s.o_failed_alloc == NULL) {
        Nst_free(Nst_t.Type);
        Nst_free(Nst_t.Str);
        Nst_free(Nst_s.e_MemoryError);
        return false;
    }

    Nst_t.Int    = Nst_type_new("Int");
    Nst_t.Real   = Nst_type_new("Real");
    Nst_t.Bool   = Nst_type_new("Bool");
    Nst_t.Null   = Nst_type_new("Null");
    Nst_t.Array  = Nst_type_new("Array");
    Nst_t.Vector = Nst_type_new("Vector");
    Nst_t.Map    = Nst_type_new("Map");
    Nst_t.Func   = Nst_type_new("Func");
    Nst_t.Iter   = Nst_type_new("Iter");
    Nst_t.Byte   = Nst_type_new("Byte");
    Nst_t.IOFile = Nst_type_new("IOFile");

    Nst_s.t_Type   = STR(_Nst_string_copy(Nst_t.Type));
    Nst_s.t_Int    = STR(_Nst_string_copy(Nst_t.Int));
    Nst_s.t_Real   = STR(_Nst_string_copy(Nst_t.Real));
    Nst_s.t_Bool   = STR(_Nst_string_copy(Nst_t.Bool));
    Nst_s.t_Null   = STR(_Nst_string_copy(Nst_t.Null));
    Nst_s.t_Str    = STR(_Nst_string_copy(Nst_t.Str));
    Nst_s.t_Array  = STR(_Nst_string_copy(Nst_t.Array));
    Nst_s.t_Vector = STR(_Nst_string_copy(Nst_t.Vector));
    Nst_s.t_Map    = STR(_Nst_string_copy(Nst_t.Map));
    Nst_s.t_Func   = STR(_Nst_string_copy(Nst_t.Func));
    Nst_s.t_Iter   = STR(_Nst_string_copy(Nst_t.Iter));
    Nst_s.t_Byte   = STR(_Nst_string_copy(Nst_t.Byte));
    Nst_s.t_IOFile = STR(_Nst_string_copy(Nst_t.IOFile));

    Nst_s.c_true  = STR(Nst_string_new_c("true",  4, false));
    Nst_s.c_false = STR(Nst_string_new_c("false", 5, false));
    Nst_s.c_null  = STR(Nst_string_new_c("null",  4, false));

    Nst_s.e_SyntaxError = STR(Nst_string_new_c("Syntax Error", 12, false));
    Nst_s.e_ValueError  = STR(Nst_string_new_c("Value Error",  11, false));
    Nst_s.e_TypeError   = STR(Nst_string_new_c("Type Error",   10, false));
    Nst_s.e_CallError   = STR(Nst_string_new_c("Call Error",   10, false));
    Nst_s.e_MathError   = STR(Nst_string_new_c("Math Error",   10, false));
    Nst_s.e_ImportError = STR(Nst_string_new_c("Import Error", 12, false));

    Nst_s.o__vars_    = STR(Nst_string_new_c("_vars_",    6, false));
    Nst_s.o__globals_ = STR(Nst_string_new_c("_globals_", 9, false));
    Nst_s.o__args_    = STR(Nst_string_new_c("_args_",    6, false));
    Nst_s.o__cwd_     = STR(Nst_string_new_c("_cwd_",     5, false));

    Nst_c.Bool_true  = Nst_bool_new(true);
    Nst_c.Bool_false = Nst_bool_new(false);
    Nst_c.Null_null  = _Nst_obj_alloc(sizeof(Nst_Obj), Nst_t.Null, NULL);
    Nst_c.Int_0    = Nst_int_new(0);
    Nst_c.Int_1    = Nst_int_new(1);
    Nst_c.Int_neg1 = Nst_int_new(-1);
    Nst_c.Real_0   = Nst_real_new(0.0);
    Nst_c.Real_1   = Nst_real_new(1.0);
    Nst_c.Byte_0   = Nst_byte_new(0);
    Nst_c.Byte_1   = Nst_byte_new(1);

    Nst_io.in  = IOFILE(Nst_iof_new(stdin,  false, true, false));
    Nst_io.out = IOFILE(Nst_iof_new(stdout, false, false, true));
    Nst_io.err = IOFILE(Nst_iof_new(stderr, false, false, true));

    Nst_io.in ->close_f = close_std_stream;
    Nst_io.out->close_f = close_std_stream;
    Nst_io.err->close_f = close_std_stream;

#ifdef Nst_WIN
    Nst_io.in->read_f = (Nst_IOFile_read_f)_Nst_windows_stdin_read;
#endif // !Nst_WIN

    Nst_itf.range_start   = FUNC(Nst_func_new_c(1, Nst_iter_range_start));
    Nst_itf.range_is_done = FUNC(Nst_func_new_c(1, Nst_iter_range_is_done));
    Nst_itf.range_get_val = FUNC(Nst_func_new_c(1, Nst_iter_range_get_val));
    Nst_itf.str_start     = FUNC(Nst_func_new_c(1, Nst_iter_str_start));
    Nst_itf.str_is_done   = FUNC(Nst_func_new_c(1, Nst_iter_str_is_done));
    Nst_itf.str_get_val   = FUNC(Nst_func_new_c(1, Nst_iter_str_get_val));
    Nst_itf.seq_start     = FUNC(Nst_func_new_c(1, Nst_iter_seq_start));
    Nst_itf.seq_is_done   = FUNC(Nst_func_new_c(1, Nst_iter_seq_is_done));
    Nst_itf.seq_get_val   = FUNC(Nst_func_new_c(1, Nst_iter_seq_get_val));
    Nst_itf.map_start     = FUNC(Nst_func_new_c(1, Nst_iter_map_start));
    Nst_itf.map_is_done   = FUNC(Nst_func_new_c(1, Nst_iter_map_is_done));
    Nst_itf.map_get_val   = FUNC(Nst_func_new_c(1, Nst_iter_map_get_val));

    if (Nst_error_occurred()) {
        Nst_error_clear();
        _Nst_del_objects();
        return false;
    }
    return true;
}

void _Nst_del_objects(void)
{
    Nst_ndec_ref(Nst_t.Type);
    Nst_ndec_ref(Nst_t.Int);
    Nst_ndec_ref(Nst_t.Real);
    Nst_ndec_ref(Nst_t.Bool);
    Nst_ndec_ref(Nst_t.Null);
    Nst_ndec_ref(Nst_t.Str);
    Nst_ndec_ref(Nst_t.Array);
    Nst_ndec_ref(Nst_t.Vector);
    Nst_ndec_ref(Nst_t.Map);
    Nst_ndec_ref(Nst_t.Func);
    Nst_ndec_ref(Nst_t.Iter);
    Nst_ndec_ref(Nst_t.Byte);
    Nst_ndec_ref(Nst_t.IOFile);

    Nst_ndec_ref(Nst_s.t_Type);
    Nst_ndec_ref(Nst_s.t_Int);
    Nst_ndec_ref(Nst_s.t_Real);
    Nst_ndec_ref(Nst_s.t_Bool);
    Nst_ndec_ref(Nst_s.t_Null);
    Nst_ndec_ref(Nst_s.t_Str);
    Nst_ndec_ref(Nst_s.t_Array);
    Nst_ndec_ref(Nst_s.t_Vector);
    Nst_ndec_ref(Nst_s.t_Map);
    Nst_ndec_ref(Nst_s.t_Func);
    Nst_ndec_ref(Nst_s.t_Iter);
    Nst_ndec_ref(Nst_s.t_Byte);
    Nst_ndec_ref(Nst_s.t_IOFile);

    Nst_ndec_ref(Nst_s.c_true);
    Nst_ndec_ref(Nst_s.c_false);
    Nst_ndec_ref(Nst_s.c_null);

    Nst_ndec_ref(Nst_s.e_SyntaxError);
    Nst_ndec_ref(Nst_s.e_MemoryError);
    Nst_ndec_ref(Nst_s.e_ValueError);
    Nst_ndec_ref(Nst_s.e_TypeError);
    Nst_ndec_ref(Nst_s.e_CallError);
    Nst_ndec_ref(Nst_s.e_MathError);
    Nst_ndec_ref(Nst_s.e_ImportError);

    Nst_ndec_ref(Nst_s.o__args_);
    Nst_ndec_ref(Nst_s.o__cwd_);
    Nst_ndec_ref(Nst_s.o__globals_);
    Nst_ndec_ref(Nst_s.o__vars_);
    Nst_ndec_ref(Nst_s.o_failed_alloc);

    Nst_ndec_ref(Nst_c.Bool_true);
    Nst_ndec_ref(Nst_c.Bool_false);
    Nst_ndec_ref(Nst_c.Null_null);
    Nst_ndec_ref(Nst_c.Int_0);
    Nst_ndec_ref(Nst_c.Int_1);
    Nst_ndec_ref(Nst_c.Int_neg1);
    Nst_ndec_ref(Nst_c.Real_0);
    Nst_ndec_ref(Nst_c.Real_1);
    Nst_ndec_ref(Nst_c.Byte_0);
    Nst_ndec_ref(Nst_c.Byte_1);

    Nst_ndec_ref(Nst_io.in);
    Nst_ndec_ref(Nst_io.out);
    Nst_ndec_ref(Nst_io.err);

    Nst_ndec_ref(Nst_itf.range_start);
    Nst_ndec_ref(Nst_itf.range_is_done);
    Nst_ndec_ref(Nst_itf.range_get_val);
    Nst_ndec_ref(Nst_itf.str_start);
    Nst_ndec_ref(Nst_itf.str_is_done);
    Nst_ndec_ref(Nst_itf.str_get_val);
    Nst_ndec_ref(Nst_itf.seq_start);
    Nst_ndec_ref(Nst_itf.seq_is_done);
    Nst_ndec_ref(Nst_itf.seq_get_val);
}

Nst_Obj *Nst_true(void)
{
    return Nst_c.Bool_true;
}

Nst_Obj *Nst_false(void)
{
    return Nst_c.Bool_false;
}

Nst_Obj *Nst_null(void)
{
    return Nst_c.Null_null;
}

const Nst_TypeObjs *Nst_type(void)
{
    return &Nst_t;
}

const Nst_StrConsts *Nst_str(void)
{
    return &Nst_s;
}

const Nst_Consts *Nst_const(void)
{
    return &Nst_c;
}

const Nst_IterFunctions *Nst_iter_func(void)
{
    return &Nst_itf;
}

Nst_StdStreams *Nst_stdio(void)
{
    return &Nst_io;
}

static int close_std_stream(void *f)
{
    Nst_UNUSED(f);
    return 0;
}
