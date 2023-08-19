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

static Nst_IOResult write_std_stream(i8 *buf, usize buf_len, usize *count,
                                    Nst_IOFileObj *f);
static Nst_IOResult close_std_stream(Nst_IOFileObj *f);

static Nst_IOResult read_std_stream(i8 *buf, usize buf_size, usize count,
                                    usize *buf_len, Nst_IOFileObj *f);

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

    Nst_io.in  = IOFILE(Nst_iof_new(stdin,  false, true, false, NULL));
    Nst_io.out = IOFILE(Nst_iof_new(stdout, false, false, true, NULL));
    Nst_io.err = IOFILE(Nst_iof_new(stderr, false, false, true, NULL));

    Nst_io.in->func_set.read = read_std_stream;

    Nst_io.out->func_set.write = write_std_stream;
    Nst_io.err->func_set.write = write_std_stream;

    Nst_io.in ->func_set.close = close_std_stream;
    Nst_io.out->func_set.close = close_std_stream;
    Nst_io.err->func_set.close = close_std_stream;


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

static Nst_IOResult write_std_stream(i8 *buf, usize buf_len, usize *count,
                                     Nst_IOFileObj *f)
{
    usize chars_written = 0;

    while (buf_len > 0) {
        i32 ch_len = Nst_check_ext_utf8_bytes((u8 *)buf, buf_len);
        usize written_char = fwrite(buf, 1, ch_len, f->fp);
        if (written_char != (usize)ch_len) {
            if (count != NULL)
                *count = chars_written;
            return Nst_IO_ERROR;
        }
        chars_written++;
        buf += ch_len;
        buf_len -= ch_len;
    }
    if (count != NULL)
        *count = chars_written;
    return Nst_IO_SUCCESS;
}

static Nst_IOResult close_std_stream(Nst_IOFileObj *f)
{
    Nst_UNUSED(f);
    return Nst_IO_SUCCESS;
}

#ifdef Nst_WIN

static bool read_characters(usize offset)
{
    DWORD len;
    BOOL result = ReadConsoleW(
        Nst_stdin.hd,
        Nst_stdin.buf + offset,
        (DWORD)(1024 - offset),
        &len,
        NULL);
    if (!result)
        return false;

    Nst_stdin.buf_size = (usize)len;
    Nst_stdin.buf_ptr = (i32)offset;
    return true;
}

static bool get_ch(Nst_Buffer *buf)
{
    if (Nst_stdin.buf_ptr >= Nst_stdin.buf_size) {
        if (!read_characters(0))
            return false;
    } else if (Nst_stdin.buf_ptr + 1 == Nst_stdin.buf_size) {
        // fix surrogate pairs getting cut-off
        Nst_stdin.buf[0] = Nst_stdin.buf[Nst_stdin.buf_ptr];
        if (!read_characters(1))
            return false;
    }

    i32 ch_len = Nst_check_utf16_bytes(
        Nst_stdin.buf + Nst_stdin.buf_ptr,
        Nst_stdin.buf_size - Nst_stdin.buf_ptr);

    if (ch_len < 0)
        return false;

    Nst_utf16_to_utf8(
        buf->data,
        Nst_stdin.buf + Nst_stdin.buf_ptr,
        (usize)ch_len);
    buf->len += ch_len;
    return true;
}

#else
static bool get_ch(Nst_Buffer *buf)
{
    i8 ch_buf[5] = { 0 };

    for (i32 i = 0; i < 4; i++) {
        if (fread(buf + i, 1, 1, Nst_io.in->fp) == 0)
            return false;

        if (Nst_check_ext_utf8_bytes((u8 *)ch_buf, i + 1) > 0) {
            goto success;
        }
    }
    return false;

success:
    Nst_buffer_append_c_str(buf, ch_buf);
    return true;
}
#endif

static Nst_IOResult read_std_stream(i8 *buf, usize buf_size, usize count,
                                    usize *buf_len, Nst_IOFileObj *f)
{
#ifdef Nst_WIN
    if (Nst_stdin.hd == NULL)
        return Nst_FILE_read(buf, buf_size, count, buf_len, f);
#else
    Nst_UNUSED(f);
#endif // !Nst_WIN

    bool expand_buf = buf_size == 0;
    Nst_Buffer buffer;
    if (expand_buf) {
        if (!Nst_buffer_init(&buffer, count))
            return Nst_IO_ALLOC_FAILED;
    } else {
        buffer.data = buf;
        buffer.cap = buf_size;
        buffer.len = 0;
    }

    for (usize i = 0; i < count; i++) {
        if (expand_buf && !Nst_buffer_expand_by(&buffer, 5))
            return Nst_IO_ALLOC_FAILED;

        if (!get_ch(&buffer))
            return Nst_IO_OP_FAILED;
    }

    buffer.data[buffer.len] = '\0';

    if (expand_buf)
        *(i8 **)buf = buffer.data;
    if (buf_len != NULL)
        *buf_len = buffer.len;
    return Nst_IO_SUCCESS;
}
