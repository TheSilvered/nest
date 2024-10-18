#include <string.h>
#include "global_consts.h"
#include "lib_import.h"
#include "mem.h"
#include "iter.h"
#include "argv_parser.h"
#include "type.h"
#include "math.h"

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

static Nst_TypeObj *type_obj_no_err(const i8 *name, Nst_ObjDstr dstr)
{
    Nst_TypeObj *type = TYPE(Nst_raw_malloc(sizeof(Nst_TypeObj)));
    if (type == NULL)
        return NULL;

#ifdef Nst_TRACK_OBJ_INIT_POS
    type->init_line = -1;
    type->init_col = -1;
    type->init_path = NULL;
#endif

    type->ref_count = 1;
    type->p_next = NULL;
    type->hash = -1;
    type->flags = 0;
    type->p_head = NULL;
    type->p_len = 0;
    type->dstr = dstr;
    type->name = Nst_str_temp((i8 *)name, strlen(name));

    type->type = Nst_t.Type;
    Nst_ninc_ref(Nst_t.Type);
    return type;
}

static Nst_StrObj *str_obj_no_err(const i8 *value)
{
    Nst_StrObj *str = STR(Nst_raw_malloc(sizeof(Nst_StrObj)));
    if (str == NULL)
        return NULL;

#ifdef Nst_TRACK_OBJ_INIT_POS
    str->init_line = -1;
    str->init_col = -1;
    str->init_path = NULL;
#endif

    str->ref_count = 1;
    str->p_next = NULL;
    str->hash = -1;
    str->flags = 0;
    str->len = strlen(value);
    str->value = (i8 *)value;
    str->indexable_str = NULL;

    str->type = Nst_t.Str;
    Nst_inc_ref(Nst_t.Str);
    return str;
}

bool _Nst_globals_init(void)
{
    Nst_t.Type = type_obj_no_err("Type", (Nst_ObjDstr)_Nst_type_destroy);
    if (Nst_t.Type == NULL)
        return false;
    Nst_t.Type->type = Nst_t.Type;

    Nst_t.Str = type_obj_no_err("Str", (Nst_ObjDstr)_Nst_str_destroy);
    if (Nst_t.Str == NULL) {
        Nst_free(Nst_t.Type);
        return false;
    }
    Nst_t.Type->name.type = Nst_t.Str;
    Nst_t.Str->name.type  = Nst_t.Str;

    Nst_s.e_MemoryError = str_obj_no_err("Memory Error");
    if (Nst_s.e_MemoryError == NULL) {
        Nst_free(Nst_t.Type);
        Nst_free(Nst_t.Str);
        return false;
    }
    Nst_s.o_failed_alloc = str_obj_no_err("failed allocation");
    if (Nst_s.o_failed_alloc == NULL) {
        Nst_free(Nst_t.Type);
        Nst_free(Nst_t.Str);
        Nst_free(Nst_s.e_MemoryError);
        return false;
    }

    Nst_t.Int    = Nst_type_new("Int", NULL);
    Nst_t.Real   = Nst_type_new("Real", NULL);
    Nst_t.Bool   = Nst_type_new("Bool", NULL);
    Nst_t.Byte   = Nst_type_new("Byte", NULL);
    Nst_t.Null   = Nst_type_new("Null", NULL);
    Nst_t.IEnd   = Nst_type_new("IEnd", NULL);
    Nst_t.IOFile = Nst_type_new("IOFile", (Nst_ObjDstr)_Nst_iofile_destroy);
    Nst_t.Array = Nst_cont_type_new(
        "Array",
        (Nst_ObjDstr)_Nst_seq_destroy,
        (Nst_ObjTrav)_Nst_seq_traverse);
    Nst_t.Vector = Nst_cont_type_new(
        "Vector",
        (Nst_ObjDstr)_Nst_seq_destroy,
        (Nst_ObjTrav)_Nst_seq_traverse);
    Nst_t.Map = Nst_cont_type_new(
        "Map",
        (Nst_ObjDstr)_Nst_map_destroy,
        (Nst_ObjTrav)_Nst_map_traverse);
    Nst_t.Func = Nst_cont_type_new(
        "Func",
        (Nst_ObjDstr)_Nst_func_destroy,
        (Nst_ObjTrav)_Nst_func_traverse);
    Nst_t.Iter = Nst_cont_type_new(
        "Iter",
        (Nst_ObjDstr)_Nst_iter_destroy,
        (Nst_ObjTrav)_Nst_iter_traverse);

    Nst_s.c_true   = STR(Nst_str_new_c("true",  4, false));
    Nst_s.c_false  = STR(Nst_str_new_c("false", 5, false));
    Nst_s.c_null   = STR(Nst_str_new_c("null",  4, false));
    Nst_s.c_inf    = STR(Nst_str_new_c("inf",   3, false));
    Nst_s.c_nan    = STR(Nst_str_new_c("nan",   3, false));
    Nst_s.c_neginf = STR(Nst_str_new_c("-inf",  4, false));
    Nst_s.c_negnan = STR(Nst_str_new_c("-nan",  4, false));

    Nst_s.e_SyntaxError = STR(Nst_str_new_c("Syntax Error", 12, false));
    Nst_s.e_ValueError  = STR(Nst_str_new_c("Value Error",  11, false));
    Nst_s.e_TypeError   = STR(Nst_str_new_c("Type Error",   10, false));
    Nst_s.e_CallError   = STR(Nst_str_new_c("Call Error",   10, false));
    Nst_s.e_MathError   = STR(Nst_str_new_c("Math Error",   10, false));
    Nst_s.e_ImportError = STR(Nst_str_new_c("Import Error", 12, false));
    Nst_s.e_Interrupt   = STR(Nst_str_new_c("Interrupt",     9, false));

    Nst_s.o__vars_    = STR(Nst_str_new_c("_vars_",    6, false));
    Nst_s.o__globals_ = STR(Nst_str_new_c("_globals_", 9, false));
    Nst_s.o__args_    = STR(Nst_str_new_c("_args_",    6, false));

    Nst_c.Bool_true  = Nst_bool_new(true);
    Nst_c.Bool_false = Nst_bool_new(false);
    Nst_c.Null_null  = _Nst_obj_alloc(sizeof(Nst_Obj), Nst_t.Null);
    Nst_c.IEnd_iend  = _Nst_obj_alloc(sizeof(Nst_Obj), Nst_t.IEnd);
    Nst_c.Int_0      = Nst_int_new(0);
    Nst_c.Int_1      = Nst_int_new(1);
    Nst_c.Int_neg1   = Nst_int_new(-1);
    Nst_c.Real_0     = Nst_real_new(0.0);
    Nst_c.Real_1     = Nst_real_new(1.0);
    Nst_c.Real_nan   = Nst_real_new(NAN);
    Nst_c.Real_negnan= Nst_real_new(-NAN);
    Nst_c.Real_inf   = Nst_real_new(INFINITY);
    Nst_c.Real_neginf= Nst_real_new(-INFINITY);
    Nst_c.Byte_0     = Nst_byte_new(0);
    Nst_c.Byte_1     = Nst_byte_new(1);

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
    Nst_itf.range_get_val = FUNC(Nst_func_new_c(1, Nst_iter_range_get_val));
    Nst_itf.str_start     = FUNC(Nst_func_new_c(1, Nst_iter_str_start));
    Nst_itf.str_get_val   = FUNC(Nst_func_new_c(1, Nst_iter_str_get_val));
    Nst_itf.seq_start     = FUNC(Nst_func_new_c(1, Nst_iter_seq_start));
    Nst_itf.seq_get_val   = FUNC(Nst_func_new_c(1, Nst_iter_seq_get_val));
    Nst_itf.map_start     = FUNC(Nst_func_new_c(1, Nst_iter_map_start));
    Nst_itf.map_get_val   = FUNC(Nst_func_new_c(1, Nst_iter_map_get_val));

    if (Nst_error_occurred()) {
        Nst_error_clear();
        _Nst_globals_quit();
        return false;
    }
    return true;
}

void _Nst_globals_quit(void)
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
    Nst_ndec_ref(Nst_t.IEnd);

    Nst_ndec_ref(Nst_s.c_true);
    Nst_ndec_ref(Nst_s.c_false);
    Nst_ndec_ref(Nst_s.c_null);
    Nst_ndec_ref(Nst_s.c_inf);
    Nst_ndec_ref(Nst_s.c_nan);
    Nst_ndec_ref(Nst_s.c_neginf);
    Nst_ndec_ref(Nst_s.c_negnan);

    Nst_ndec_ref(Nst_s.e_SyntaxError);
    Nst_ndec_ref(Nst_s.e_MemoryError);
    Nst_ndec_ref(Nst_s.e_ValueError);
    Nst_ndec_ref(Nst_s.e_TypeError);
    Nst_ndec_ref(Nst_s.e_CallError);
    Nst_ndec_ref(Nst_s.e_MathError);
    Nst_ndec_ref(Nst_s.e_ImportError);
    Nst_ndec_ref(Nst_s.e_Interrupt);

    Nst_ndec_ref(Nst_s.o__args_);
    Nst_ndec_ref(Nst_s.o__globals_);
    Nst_ndec_ref(Nst_s.o__vars_);
    Nst_ndec_ref(Nst_s.o_failed_alloc);

    Nst_ndec_ref(Nst_c.Bool_true);
    Nst_ndec_ref(Nst_c.Bool_false);
    Nst_ndec_ref(Nst_c.Null_null);
    Nst_ndec_ref(Nst_c.IEnd_iend);
    Nst_ndec_ref(Nst_c.Int_0);
    Nst_ndec_ref(Nst_c.Int_1);
    Nst_ndec_ref(Nst_c.Int_neg1);
    Nst_ndec_ref(Nst_c.Real_0);
    Nst_ndec_ref(Nst_c.Real_1);
    Nst_ndec_ref(Nst_c.Real_nan);
    Nst_ndec_ref(Nst_c.Real_negnan);
    Nst_ndec_ref(Nst_c.Real_inf);
    Nst_ndec_ref(Nst_c.Real_neginf);
    Nst_ndec_ref(Nst_c.Byte_0);
    Nst_ndec_ref(Nst_c.Byte_1);

    Nst_ndec_ref(Nst_io.in);
    Nst_ndec_ref(Nst_io.out);
    Nst_ndec_ref(Nst_io.err);

    Nst_ndec_ref(Nst_itf.range_start);
    Nst_ndec_ref(Nst_itf.range_get_val);
    Nst_ndec_ref(Nst_itf.str_start);
    Nst_ndec_ref(Nst_itf.str_get_val);
    Nst_ndec_ref(Nst_itf.seq_start);
    Nst_ndec_ref(Nst_itf.seq_get_val);
    Nst_ndec_ref(Nst_itf.map_start);
    Nst_ndec_ref(Nst_itf.map_get_val);
}

Nst_Obj *Nst_true(void)
{
    return Nst_c.Bool_true;
}

Nst_Obj *Nst_true_ref(void)
{
    return Nst_inc_ref(Nst_c.Bool_true);
}

Nst_Obj *Nst_false(void)
{
    return Nst_c.Bool_false;
}

Nst_Obj *Nst_false_ref(void)
{
    return Nst_inc_ref(Nst_c.Bool_false);
}

Nst_Obj *Nst_null(void)
{
    return Nst_c.Null_null;
}

Nst_Obj *Nst_null_ref(void)
{
    return Nst_inc_ref(Nst_c.Null_null);
}

Nst_Obj *Nst_iend(void)
{
    return Nst_c.IEnd_iend;
}

Nst_Obj *Nst_iend_ref(void)
{
    return Nst_inc_ref(Nst_c.IEnd_iend);
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
    if (count != NULL)
        *count = Nst_string_utf8_char_len((u8 *)buf, buf_len);

    usize bytes_written = fwrite(buf, 1, buf_len, f->fp);

    if (bytes_written >= buf_len) {
        return Nst_IO_SUCCESS;
    }
    if (count != NULL) {
        isize chars_written = Nst_string_char_len(
            Nst_cp(Nst_CP_EXT_UTF8),
            (void *)buf,
            bytes_written);
        if (chars_written == -1)
            *count = 0;
        else
            *count = chars_written;
        return Nst_IO_ERROR;
    }
    return Nst_IO_ERROR;
}

static Nst_IOResult close_std_stream(Nst_IOFileObj *f)
{
    Nst_UNUSED(f);
    return Nst_IO_SUCCESS;
}

#ifdef Nst_MSVC

static bool read_characters(usize offset)
{
    DWORD len;
    BOOL result = ReadConsoleW(
        Nst_stdin.hd,
        Nst_stdin.buf + offset,
        (DWORD)(_Nst_WIN_STDIN_BUF_SIZE - offset),
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
        wchar_t ch = Nst_stdin.buf[Nst_stdin.buf_ptr];

        if (0xd800 <= ch && ch <= 0xdfff) {
            Nst_stdin.buf[0] = ch;
            if (!read_characters(1))
                return false;
        }
    }

    i32 ch_len = Nst_check_utf16_bytes(
        Nst_stdin.buf + Nst_stdin.buf_ptr,
        Nst_stdin.buf_size - Nst_stdin.buf_ptr);

    if (ch_len < 0)
        return false;

    i32 utf8_ch_len = Nst_utf16_to_utf8(
        buf->data,
        Nst_stdin.buf + Nst_stdin.buf_ptr,
        (usize)ch_len);
    buf->len += utf8_ch_len;
    Nst_stdin.buf_ptr += ch_len;
    return true;
}

#else
static bool get_ch(Nst_Buffer *buf)
{
    i8 ch_buf[5] = { 0 };

    for (i32 i = 0; i < 4; i++) {
        if (fread(ch_buf + i, 1, 1, Nst_io.in->fp) == 0)
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
#ifdef Nst_MSVC
    if (Nst_stdin.hd == NULL)
        return Nst_FILE_read(buf, buf_size, count, buf_len, f);
#else
    Nst_UNUSED(f);
#endif // !Nst_MSVC

    Nst_Buffer buffer;

    if (buf_size == 0) {
        if (!Nst_buffer_init(&buffer, count))
            return Nst_IO_ALLOC_FAILED;
    } else {
        buffer.data = buf;
        buffer.len = 0;
        buffer.cap = buf_size;
        buffer.unit_size = sizeof(u8);
    }

    for (usize i = 0; i < count; i++) {
        if (buf_size == 0) {
            if (!Nst_buffer_expand_by(&buffer, 5))
                return Nst_IO_ALLOC_FAILED;
        } else if (buf_size < buffer.len + 5)
            break;

        if (!get_ch(&buffer))
            return Nst_IO_OP_FAILED;
    }

    buffer.data[buffer.len] = '\0';

    if (buf_size == 0)
        *(i8 **)buf = buffer.data;
    if (buf_len != NULL)
        *buf_len = buffer.len;
    return Nst_IO_SUCCESS;
}
