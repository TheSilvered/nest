#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include "nest_io.h"

#define FUNC_COUNT 19

#define SET_FILE_CLOSED_ERROR \
    Nst_set_value_error_c("the given file given was previously closed")

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_Obj *stdin_obj;
static Nst_Obj *stdout_obj;
static Nst_Obj *stderr_obj;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(open_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(virtual_file_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(close_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(write_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(write_bytes_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(read_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(read_bytes_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(file_size_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(move_fptr_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_fptr_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(flush_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_flags_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(println_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_set_stdin_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_set_stdout_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_set_stderr_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_get_stdin_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_get_stdout_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_get_stderr_, 0);

#if __LINE__ - FUNC_COUNT != 24
#error
#endif

    stdin_obj  = Nst_iof_new(stdin,  false, true, false);
    stdout_obj = Nst_iof_new(stdout, false, false, true);
    stderr_obj = Nst_iof_new(stderr, false, false, true);

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    Nst_dec_ref(stdin_obj);
    Nst_dec_ref(stderr_obj);
    Nst_dec_ref(stdout_obj);
}

static i32 get_file_size(Nst_IOFileObj *f)
{
    i32 start = f->tell_f(f->value);
    f->seek_f(f->value, 0, SEEK_END);
    i32 end = f->tell_f(f->value);
    f->seek_f(f->value, start, SEEK_SET);
    return end;
}

static usize virtual_iof_read_f(void               *buf,
                                usize               e_size,
                                usize               e_count,
                                VirtualIOFile_data *f)
{
    if ( f == nullptr )
    {
        return 0;
    }

    usize byte_count = e_size * e_count;
    if ( byte_count == 0 || f->ptr >= f->size )
    {
        return 0;
    }

    if ( f->ptr + byte_count > f->size )
    {
        byte_count = f->size - (usize)f->ptr;
        byte_count -= byte_count % e_size;
    }

    memcpy(buf, f->data + f->ptr, byte_count);
    f->ptr += (i32)byte_count;
    return byte_count / e_size;
}

static usize virtual_iof_write_f(void               *buf,
                                 usize               e_size,
                                 usize               e_count,
                                 VirtualIOFile_data *f)
{
    if ( f == nullptr )
    {
        return 0;
    }

    usize byte_count = e_size * e_count;
    if ( byte_count == 0 || f->ptr > f->size )
    {
        return 0;
    }

    if ( f->curr_buf_size + byte_count <= f->buf_size )
    {
        memcpy(f->buf + f->curr_buf_size, buf, byte_count);
        f->curr_buf_size += byte_count;
    }
    else
    {
        usize tot_bytes = byte_count + f->curr_buf_size;
        i8 *new_data = Nst_realloc_c(
            f->data,
            (usize)f->ptr + tot_bytes,
            i8,
            0);
        if ( new_data == NULL )
        {
            return 0;
        }
        f->size = (usize)f->ptr + tot_bytes;
        f->data = new_data;
        memcpy(f->data + f->ptr, f->buf, f->curr_buf_size);
        memcpy(f->data + f->ptr + f->curr_buf_size, buf, byte_count);
        f->ptr += tot_bytes;
        f->curr_buf_size = 0;
    }

    return e_count;
}

static int virtual_iof_flush_f(VirtualIOFile_data *f)
{
    if ( f == nullptr )
    {
        return EOF;
    }

    usize byte_count = f->curr_buf_size;
    i8 *new_data = Nst_realloc_c(
        f->data,
        (usize)f->ptr + byte_count,
        i8,
        0);
    if ( new_data == NULL )
    {
        return EOF;
    }
    f->size = (usize)f->ptr + byte_count;
    f->data = new_data;
    memcpy(f->data + f->ptr, f->buf, byte_count);
    f->ptr += byte_count;
    f->curr_buf_size = 0;
    return 0;
}

static i32 virtual_iof_tell_f(VirtualIOFile_data *f)
{
    if ( f == nullptr )
    {
        return -1;
    }

    return (i32)f->ptr;
}

static int virtual_iof_seek_f(VirtualIOFile_data *f, i32 offset, int start)
{
    if ( f == nullptr )
    {
        return -1;
    }

    isize new_pos;
    if ( start == SEEK_CUR )
    {
        new_pos = f->ptr + offset;
    }
    else if ( start == SEEK_SET)
    {
        new_pos = offset;
    }
    else
    {
        new_pos = (i32)f->size - offset;
    }

    if ( new_pos < 0 )
    {
        return -1;
    }

    f->ptr = new_pos;

    return 0;
}

static int virtual_iof_close_f(VirtualIOFile_data *f)
{
    if ( f == nullptr )
    {
        return -1;
    }

    Nst_free(f->data);
    Nst_free(f->buf);
    Nst_free(f);
    return 0;
}

Nst_FUNC_SIGN(open_)
{
    Nst_StrObj *file_name_str;
    Nst_Obj *file_mode_str;
    Nst_DEF_EXTRACT("s?s", &file_name_str, &file_mode_str);

    i8 *file_name = file_name_str->value;
    i8 *file_mode = Nst_DEF_VAL(file_mode_str, STR(file_mode_str)->value, (i8 *)"r");
    usize file_mode_len = Nst_DEF_VAL(file_mode_str, STR(file_mode_str)->len, 1);

    bool is_bin = false;
    bool can_read = false;
    bool can_write = false;

    if ( file_mode_len < 1 || file_mode_len > 3 )
    {
        Nst_set_value_error_c("the file mode is not valid");
        return nullptr;
    }

#ifdef Nst_WIN
    wchar_t bin_mode[4] = { 0, 'b', 0, 0 };
#else
    char bin_mode[4] = { 0, 'b', 0, 0 };
#endif

    switch ( *file_mode )
    {
    case 'r':
        can_read = true;
        bin_mode[0] = 'r';
        break;
    case 'a':
        can_write = true;
        bin_mode[0] = 'a';
        break;
    case 'w':
        can_write = true;
        bin_mode[0] = 'w';
        break;
    default:
        Nst_set_value_error_c("the file mode is not valid");
        return nullptr;
    }

    if ( file_mode_len == 2 )
    {
        switch ( file_mode[1] )
        {
        case 'b':
            is_bin = true;
            break;
        case '+':
            can_read = true;
            can_write = true;
            bin_mode[2] = '+';
            break;
        default:
            Nst_set_value_error_c("the file mode is not valid");
            return nullptr;
        }
    }
    else if ( file_mode_len == 3 )
    {
        is_bin = true;
        can_read = true;
        can_write = true;
        bin_mode[2] = '+';

        if ( !(file_mode[1] == 'b' && file_mode[2] == '+') &&
             !(file_mode[1] == '+' && file_mode[2] == 'b') )
        {
            Nst_set_value_error_c("the file mode is not valid");
            return nullptr;
        }
    }

#ifdef Nst_WIN
    wchar_t *wide_filename = Nst_char_to_wchar_t(file_name, file_name_str->len);
    if ( wide_filename == nullptr )
    {
        return nullptr;
    }

    Nst_IOFile file_ptr = _wfopen(wide_filename, bin_mode);
    Nst_free(wide_filename);
#else
    Nst_IOFile file_ptr = fopen(file_name, bin_mode);
#endif
    if ( file_ptr == nullptr )
    {
        Nst_set_value_error(Nst_sprintf("file '%.4096s' not found", file_name));
        return nullptr;
    }

    return Nst_iof_new(file_ptr, is_bin, can_read, can_write);
}

Nst_FUNC_SIGN(virtual_file_)
{
    bool bin;
    Nst_Obj *buf_size_obj;

    Nst_DEF_EXTRACT("o_b?i", &bin, &buf_size_obj);

    i64 buf_size = Nst_DEF_VAL(buf_size_obj, AS_INT(buf_size_obj), 128);

    VirtualIOFile_data *f = Nst_malloc_c(1, VirtualIOFile_data);
    if ( f == nullptr )
    {
        return nullptr;
    }

    f->data = Nst_malloc_c(1, i8);
    f->size = 0;
    f->ptr = 0;
    f->buf = Nst_malloc_c((usize)buf_size, i8);
    f->buf_size = (usize)buf_size;
    f->curr_buf_size = 0;

    if ( f->data == nullptr || f->buf == nullptr )
    {
        Nst_free(f->data);
        Nst_free(f->buf);
        Nst_free(f);
        return nullptr;
    }

    return Nst_iof_new_fake((void *)f, bin, true, true,
                             (Nst_IOFile_read_f)virtual_iof_read_f,
                             (Nst_IOFile_write_f)virtual_iof_write_f,
                             (Nst_IOFile_flush_f)virtual_iof_flush_f,
                             (Nst_IOFile_tell_f)virtual_iof_tell_f,
                             (Nst_IOFile_seek_f)virtual_iof_seek_f,
                             (Nst_IOFile_close_f)virtual_iof_close_f);
}

Nst_FUNC_SIGN(close_)
{
    Nst_IOFileObj *f;

    Nst_DEF_EXTRACT("F", &f);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    Nst_fclose(f);

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(write_)
{
    Nst_Obj *value_to_write;
    Nst_IOFileObj *f;

    Nst_DEF_EXTRACT("Fo", &f, &value_to_write);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !Nst_IOF_CAN_WRITE(f) )
    {
        Nst_set_value_error_c("the file does not support writing");
        return nullptr;
    }
    else if ( Nst_IOF_IS_BIN(f) )
    {
        Nst_set_value_error_c("the file is binary, try using 'write_bytes'");
        return nullptr;
    }

    // casting to a string never returns an error
    Nst_Obj *str_to_write = Nst_obj_cast(value_to_write, Nst_type()->Str);
    Nst_StrObj *str = STR(str_to_write);
    usize res = Nst_fwrite(str->value, sizeof(i8), str->len, f);

    Nst_dec_ref(str_to_write);
    return Nst_int_new(res);
}

Nst_FUNC_SIGN(write_bytes_)
{
    Nst_IOFileObj *f;
    Nst_SeqObj *seq;

    Nst_DEF_EXTRACT("FA.B", &f, &seq);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !Nst_IOF_CAN_WRITE(f) )
    {
        Nst_set_value_error_c("the file does not support writing");
        return nullptr;
    }
    else if ( !Nst_IOF_IS_BIN(f) )
    {
        Nst_set_value_error_c("the file is not binary, try using 'write'");
        return nullptr;
    }

    usize seq_len = seq->len;
    Nst_Obj **objs = seq->objs;
    i8 *bytes = Nst_malloc_c(seq_len + 1, i8);
    if ( bytes == nullptr )
    {
        return nullptr;
    }

    for ( usize i = 0; i < seq_len; i++ )
    {
        bytes[i] = AS_BYTE(objs[i]);
    }

    usize res = Nst_fwrite(bytes, sizeof(i8), seq_len, f);

    Nst_free(bytes);
    return Nst_int_new(res);
}

Nst_FUNC_SIGN(read_)
{
    Nst_IOFileObj *f;
    Nst_Obj *bytes_to_read_obj;

    Nst_DEF_EXTRACT("F?i", &f, &bytes_to_read_obj);
    i64 bytes_to_read = Nst_DEF_VAL(
        bytes_to_read_obj,
        AS_INT(bytes_to_read_obj),
        -1);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !Nst_IOF_CAN_READ(f) )
    {
        Nst_set_value_error_c("the file does not support reading");
        return nullptr;
    }
    else if ( Nst_IOF_IS_BIN(f) )
    {
        Nst_set_value_error_c("the file is binary, try using 'read_bytes'");
        return nullptr;
    }

    i32 start = Nst_ftell(f);
    i32 end = get_file_size(f);

    i64 max_size = (i64)(end - start);
    if ( bytes_to_read < 0 || bytes_to_read > max_size )
    {
        bytes_to_read = max_size;
    }

    i8 *buffer = Nst_malloc_c((usize)bytes_to_read + 1, i8);
    if ( buffer == nullptr )
    {
        Nst_failed_allocation();
        return nullptr;
    }

    usize read_bytes = Nst_fread(
        buffer,
        sizeof(i8),
        (usize)bytes_to_read,
        f);
    buffer[read_bytes] = 0;

    return Nst_string_new(buffer, read_bytes, true);
}

Nst_FUNC_SIGN(read_bytes_)
{
    Nst_IOFileObj *f;
    Nst_Obj *bytes_to_read_obj;

    Nst_DEF_EXTRACT("F?i", &f, &bytes_to_read_obj);
    i64 bytes_to_read = Nst_DEF_VAL(
        bytes_to_read_obj,
        AS_INT(bytes_to_read_obj),
        -1);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !Nst_IOF_CAN_READ(f) )
    {
        Nst_set_value_error_c("the file does not support reading");
        return nullptr;
    }
    else if ( !Nst_IOF_IS_BIN(f) )
    {
        Nst_set_value_error_c("the file is not binary, try using 'read'");
        return nullptr;
    }

    i32 start = Nst_ftell(f);
    i32 end = get_file_size(f);

    i64 max_size = (i64)(end - start);
    if ( bytes_to_read < 0 || bytes_to_read > max_size )
    {
        bytes_to_read = max_size;
    }

    i8 *buffer = Nst_malloc_c((usize)bytes_to_read, i8);
    if ( buffer == nullptr )
    {
        Nst_failed_allocation();
        return nullptr;
    }

    usize read_bytes = Nst_fread(
        buffer,
        sizeof(i8),
        (usize)bytes_to_read,
        f);

    Nst_SeqObj *bytes_array = SEQ(Nst_array_new(read_bytes));

    for ( usize i = 0; i < read_bytes; i++ )
    {
        bytes_array->objs[i] = Nst_byte_new(buffer[i]);
    }

    Nst_free(buffer);
    return OBJ(bytes_array);
}

Nst_FUNC_SIGN(file_size_)
{
    Nst_IOFileObj *f;

    Nst_DEF_EXTRACT("F", &f);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    return Nst_int_new(get_file_size(f));
}

Nst_FUNC_SIGN(get_fptr_)
{
    Nst_IOFileObj *f;

    Nst_DEF_EXTRACT("F", &f);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    return Nst_int_new(Nst_ftell(f));
}

Nst_FUNC_SIGN(move_fptr_)
{
    Nst_IOFileObj *f;
    i64 start;
    i64 offset;

    Nst_DEF_EXTRACT("Fii", &f, &start, &offset);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( start < 0 || start > 2 )
    {
        Nst_set_value_error_c("invalid origin for seek_fptr");
        return nullptr;
    }

    i32 size = get_file_size(f);
    i32 end_pos = 0;

    if ( start == SEEK_END )
    {
        end_pos = i32(size + offset);
    }
    else if ( start == SEEK_SET )
    {
        end_pos = i32(offset);
    }
    else
    {
        end_pos = Nst_ftell(f) + i32(offset);
    }

    if ( end_pos < 0 || end_pos > size )
    {
        Nst_set_value_error_c("the file pointer goes outside the file");
        return nullptr;
    }

    Nst_fseek(f, (i32)offset, (i32)start);

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(flush_)
{
    Nst_IOFileObj *f;

    Nst_DEF_EXTRACT("F", &f);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    i32 res = Nst_fflush(f);
    if ( res == EOF )
    {
        Nst_set_memory_error_c("failed to flush the file");
        return nullptr;
    }
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(get_flags_)
{
    Nst_IOFileObj *f;

    Nst_DEF_EXTRACT("F", &f);

    i8 *flags = Nst_malloc_c(4, i8);
    if (flags == nullptr)
    {
        return nullptr;
    }

    flags[0] = Nst_IOF_CAN_READ(f)  ? 'r' : '-';
    flags[1] = Nst_IOF_CAN_WRITE(f) ? 'w' : '-';
    flags[2] = Nst_IOF_IS_BIN(f)    ? 'b' : '-';
    flags[3] = 0;

    return Nst_string_new(flags,  3, true);
}

Nst_FUNC_SIGN(println_)
{
    Nst_Obj *obj;
    bool flush;
    Nst_Obj *file_obj;
    Nst_DEF_EXTRACT("oo_b?F", &obj, &flush, &file_obj);
    Nst_IOFileObj *file = Nst_DEF_VAL(file_obj, IOFILE(file_obj), Nst_stdio()->out);

    if ( Nst_IOF_IS_CLOSED(file) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    Nst_StrObj *s_obj = STR(Nst_obj_cast(obj, Nst_type()->Str));
    Nst_fprintln(file, (const i8 *)s_obj->value);

    if ( flush )
    {
        Nst_fflush(file);
    }
    Nst_dec_ref(s_obj);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(_set_stdin_)
{
    Nst_IOFileObj *f;

    Nst_DEF_EXTRACT("F", &f);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( !Nst_IOF_CAN_READ(f) )
    {
        Nst_set_value_error_c("the file must support reading");
        return nullptr;
    }

    if ( f == Nst_stdio()->in )
    {
        Nst_RETURN_NULL;
    }

    Nst_dec_ref(Nst_stdio()->in);
    Nst_dec_ref(stdin_obj);
    Nst_stdio()->in = IOFILE(Nst_inc_ref(f));
    stdin_obj  = Nst_inc_ref(f);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(_set_stdout_)
{
    Nst_IOFileObj *f;

    Nst_DEF_EXTRACT("F", &f);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( !Nst_IOF_CAN_WRITE(f) )
    {
        Nst_set_value_error_c("the file must support writing");
        return nullptr;
    }

    if ( f == Nst_stdio()->out )
    {
        Nst_RETURN_NULL;
    }

    Nst_dec_ref(Nst_stdio()->out);
    Nst_dec_ref(stdout_obj);
    Nst_stdio()->out = IOFILE(Nst_inc_ref(f));
    stdout_obj  = Nst_inc_ref(f);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(_set_stderr_)
{
    Nst_IOFileObj *f;

    Nst_DEF_EXTRACT("F", &f);

    if ( Nst_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( !Nst_IOF_CAN_WRITE(f) )
    {
        Nst_set_value_error_c("the file must support writing");
        return nullptr;
    }

    if ( f == Nst_stdio()->err )
    {
        Nst_RETURN_NULL;
    }

    Nst_dec_ref(Nst_stdio()->err);
    Nst_dec_ref(stderr_obj);
    Nst_stdio()->err = IOFILE(Nst_inc_ref(f));
    stderr_obj  = Nst_inc_ref(f);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(_get_stdin_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(stdin_obj);
}

Nst_FUNC_SIGN(_get_stdout_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(stdout_obj);
}

Nst_FUNC_SIGN(_get_stderr_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(stderr_obj);
}
