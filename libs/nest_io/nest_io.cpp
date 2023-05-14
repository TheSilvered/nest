#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include "nest_io.h"

#define FUNC_COUNT 19

#define SET_FILE_CLOSED_ERROR \
    NST_SET_RAW_VALUE_ERROR("the given file given was previously closed")

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_Obj *stdin_obj;
static Nst_Obj *stdout_obj;
static Nst_Obj *stderr_obj;

bool lib_init()
{
    usize idx = 0;
    Nst_OpErr err = { nullptr, nullptr };

    func_list_[idx++] = NST_MAKE_FUNCDECLR(open_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(virtual_file_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(close_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(write_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(write_bytes_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(read_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(read_bytes_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(file_size_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(move_fptr_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_fptr_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(flush_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_flags_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(println_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_set_stdin_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_set_stdout_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_set_stderr_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_stdin_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_stdout_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_stderr_, 0);

#if __LINE__ - FUNC_COUNT != 25
#error
#endif

    stdin_obj  = nst_iof_new(stdin,  false, true, false, &err);
    stdout_obj = nst_iof_new(stdout, false, false, true, &err);
    stderr_obj = nst_iof_new(stderr, false, false, true, &err);

    lib_init_ = err.name == nullptr;
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    nst_dec_ref(stdin_obj);
    nst_dec_ref(stderr_obj);
    nst_dec_ref(stdout_obj);
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
        i8 *new_data = (i8 *)nst_realloc(
            f->data,
            (usize)f->ptr + tot_bytes,
            sizeof(i8),
            0, nullptr);
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

static i32 virtual_iof_flush_f(VirtualIOFile_data *f)
{
    if ( f == nullptr )
    {
        return EOF;
    }

    usize byte_count = f->curr_buf_size;
    i8 *new_data = (i8 *)nst_realloc(
        f->data,
        (usize)f->ptr + byte_count,
        sizeof(i8),
        0, nullptr);
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

static i32 virtual_iof_seek_f(VirtualIOFile_data *f, i32 offset, i32 start)
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

static i32 virtual_iof_close_f(VirtualIOFile_data *f)
{
    if ( f == nullptr )
    {
        return -1;
    }

    nst_free(f->data);
    nst_free(f->buf);
    nst_free(f);
    return 0;
}

NST_FUNC_SIGN(open_)
{
    Nst_StrObj *file_name_str;
    Nst_Obj *file_mode_str;
    NST_DEF_EXTRACT("s?s", &file_name_str, &file_mode_str);

    i8 *file_name = file_name_str->value;
    i8 *file_mode = NST_DEF_VAL(file_mode_str, STR(file_mode_str)->value, (i8 *)"r");
    usize file_mode_len = NST_DEF_VAL(file_mode_str, STR(file_mode_str)->len, 1);

    bool is_bin = false;
    bool can_read = false;
    bool can_write = false;

    if ( file_mode_len < 1 || file_mode_len > 3 )
    {
        NST_SET_RAW_VALUE_ERROR("the file mode is not valid");
        return nullptr;
    }

    switch ( *file_mode )
    {
    case 'r':
        can_read = true;
        break;
    case 'a':
    case 'w':
        can_write = true;
        break;
    default:
        NST_SET_RAW_VALUE_ERROR("the file mode is not valid");
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
            break;
        default:
            NST_SET_RAW_VALUE_ERROR("the file mode is not valid");
            return nullptr;
        }
    }
    else if ( file_mode_len == 3 )
    {
        is_bin = true;
        can_read = true;
        can_write = true;

        if ( !(file_mode[1] == 'b' && file_mode[2] == '+') &&
             !(file_mode[1] == '+' && file_mode[2] == 'b') )
        {
            NST_SET_RAW_VALUE_ERROR("the file mode is not valid");
            return nullptr;
        }
    }

    Nst_IOFile file_ptr = fopen(file_name, file_mode);
    if ( file_ptr == nullptr )
    {
        NST_SET_VALUE_ERROR(nst_sprintf("file '%.4096s' not found", file_name));
        return nullptr;
    }

    return nst_iof_new(file_ptr, is_bin, can_read, can_write, nullptr);
}

NST_FUNC_SIGN(virtual_file_)
{
    Nst_Obj *bin_obj;
    Nst_Obj *buf_size_obj;

    NST_DEF_EXTRACT("?b?i", &bin_obj, &buf_size_obj);

    Nst_Bool bin = NST_DEF_VAL(bin_obj, AS_BOOL(bin_obj), false);
    Nst_Int buf_size = NST_DEF_VAL(buf_size_obj, AS_INT(buf_size_obj), 128);

    VirtualIOFile_data *f =
        (VirtualIOFile_data *)nst_malloc(1, sizeof(VirtualIOFile_data), err);
    if ( f == nullptr )
    {
        return nullptr;
    }

    f->data = (i8 *)nst_malloc(1, sizeof(i8), err);
    f->size = 0;
    f->ptr = 0;
    f->buf = (i8 *)nst_malloc((usize)buf_size, sizeof(i8), err);
    f->buf_size = (usize)buf_size;
    f->curr_buf_size = 0;

    if ( f->data == nullptr || f->buf == nullptr )
    {
        nst_free(f->data);
        nst_free(f->buf);
        nst_free(f);
        return nullptr;
    }

    return nst_iof_new_fake((void *)f, bin, true, true,
                             (Nst_IOFile_read_f)virtual_iof_read_f,
                             (Nst_IOFile_write_f)virtual_iof_write_f,
                             (Nst_IOFile_flush_f)virtual_iof_flush_f,
                             (Nst_IOFile_tell_f)virtual_iof_tell_f,
                             (Nst_IOFile_seek_f)virtual_iof_seek_f,
                             (Nst_IOFile_close_f)virtual_iof_close_f, err);
}

NST_FUNC_SIGN(close_)
{
    Nst_IOFileObj *f;

    NST_DEF_EXTRACT("F", &f);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    nst_fclose(f);

    NST_RETURN_NULL;
}

NST_FUNC_SIGN(write_)
{
    Nst_Obj *value_to_write;
    Nst_IOFileObj *f;

    NST_DEF_EXTRACT("Fo", &f, &value_to_write);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !NST_IOF_CAN_WRITE(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file does not support writing");
        return nullptr;
    }
    else if ( NST_IOF_IS_BIN(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file is binary, try using 'write_bytes'");
        return nullptr;
    }

    // casting to a string never returns an error
    Nst_Obj *str_to_write = nst_obj_cast(value_to_write, nst_type()->Str, nullptr);
    Nst_StrObj *str = STR(str_to_write);
    usize res = nst_fwrite(str->value, sizeof(i8), str->len, f);

    nst_dec_ref(str_to_write);
    return nst_int_new(res, err);
}

NST_FUNC_SIGN(write_bytes_)
{
    Nst_IOFileObj *f;
    Nst_SeqObj *seq;

    NST_DEF_EXTRACT("FA.B", &f, &seq);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !NST_IOF_CAN_WRITE(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file does not support writing");
        return nullptr;
    }
    else if ( !NST_IOF_IS_BIN(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file is not binary, try using 'write'");
        return nullptr;
    }

    usize seq_len = seq->len;
    Nst_Obj **objs = seq->objs;
    i8 *bytes = (i8 *)nst_malloc((seq_len + 1), sizeof(i8), err);
    if ( bytes == nullptr )
    {
        return nullptr;
    }

    for ( usize i = 0; i < seq_len; i++ )
    {
        bytes[i] = AS_BYTE(objs[i]);
    }

    usize res = nst_fwrite(bytes, sizeof(i8), seq_len, f);

    nst_free(bytes);
    return nst_int_new(res, err);
}

NST_FUNC_SIGN(read_)
{
    Nst_IOFileObj *f;
    Nst_Obj *bytes_to_read_obj;

    NST_DEF_EXTRACT("F?i", &f, &bytes_to_read_obj);
    Nst_Int bytes_to_read = NST_DEF_VAL(
        bytes_to_read_obj,
        AS_INT(bytes_to_read_obj),
        -1);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !NST_IOF_CAN_READ(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file does not support reading");
        return nullptr;
    }
    else if ( NST_IOF_IS_BIN(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file is binary, try using 'read_bytes'");
        return nullptr;
    }

    i32 start = nst_ftell(f);
    i32 end = get_file_size(f);

    Nst_Int max_size = (Nst_Int)(end - start);
    if ( bytes_to_read < 0 || bytes_to_read > max_size )
    {
        bytes_to_read = max_size;
    }

    i8 *buffer = (i8 *)nst_malloc((usize)bytes_to_read + 1, sizeof(i8), err);
    if ( buffer == nullptr )
    {
        NST_FAILED_ALLOCATION;
        return nullptr;
    }

    usize read_bytes = nst_fread(
        buffer,
        sizeof(i8),
        (usize)bytes_to_read,
        f);
    buffer[read_bytes] = 0;

    return nst_string_new(buffer, read_bytes, true, err);
}

NST_FUNC_SIGN(read_bytes_)
{
    Nst_IOFileObj *f;
    Nst_Obj *bytes_to_read_obj;

    NST_DEF_EXTRACT("F?i", &f, &bytes_to_read_obj);
    Nst_Int bytes_to_read = NST_DEF_VAL(
        bytes_to_read_obj,
        AS_INT(bytes_to_read_obj),
        -1);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !NST_IOF_CAN_READ(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file does not support reading");
        return nullptr;
    }
    else if ( !NST_IOF_IS_BIN(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file is not binary, try using 'read'");
        return nullptr;
    }

    i32 start = nst_ftell(f);
    i32 end = get_file_size(f);

    Nst_Int max_size = (Nst_Int)(end - start);
    if ( bytes_to_read < 0 || bytes_to_read > max_size )
    {
        bytes_to_read = max_size;
    }

    i8 *buffer = (i8 *)nst_malloc((usize)bytes_to_read, sizeof(i8), err);
    if ( buffer == nullptr )
    {
        NST_FAILED_ALLOCATION;
        return nullptr;
    }

    usize read_bytes = nst_fread(
        buffer,
        sizeof(i8),
        (usize)bytes_to_read,
        f);

    Nst_SeqObj *bytes_array = SEQ(nst_array_new(read_bytes, err));

    for ( usize i = 0; i < read_bytes; i++ )
    {
        bytes_array->objs[i] = nst_byte_new(buffer[i], err);
    }

    nst_free(buffer);
    return OBJ(bytes_array);
}

NST_FUNC_SIGN(file_size_)
{
    Nst_IOFileObj *f;

    NST_DEF_EXTRACT("F", &f);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    return nst_int_new(get_file_size(f), err);
}

NST_FUNC_SIGN(get_fptr_)
{
    Nst_IOFileObj *f;

    NST_DEF_EXTRACT("F", &f);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    return nst_int_new(nst_ftell(f), err);
}

NST_FUNC_SIGN(move_fptr_)
{
    Nst_IOFileObj *f;
    Nst_Int start;
    Nst_Int offset;

    NST_DEF_EXTRACT("Fii", &f, &start, &offset);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( start < 0 || start > 2 )
    {
        NST_SET_RAW_VALUE_ERROR("invalid origin for seek_fptr");
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
        end_pos = nst_ftell(f) + i32(offset);
    }

    if ( end_pos < 0 || end_pos > size )
    {
        NST_SET_RAW_VALUE_ERROR("the file pointer goes outside the file");
        return nullptr;
    }

    nst_fseek(f, (i32)offset, (i32)start);

    NST_RETURN_NULL;
}

NST_FUNC_SIGN(flush_)
{
    Nst_IOFileObj *f;

    NST_DEF_EXTRACT("F", &f);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    i32 res = nst_fflush(f);
    if ( res == EOF )
    {
        NST_SET_RAW_MEMORY_ERROR("failed to flush the file");
        return nullptr;
    }
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(get_flags_)
{
    Nst_IOFileObj *f;

    NST_DEF_EXTRACT("F", &f);

    i8 *flags = (i8 *)nst_malloc(4, sizeof(i8), err);
    if (flags == nullptr)
    {
        return nullptr;
    }

    flags[0] = NST_IOF_CAN_READ(f)  ? 'r' : '-';
    flags[1] = NST_IOF_CAN_WRITE(f) ? 'w' : '-';
    flags[2] = NST_IOF_IS_BIN(f)    ? 'b' : '-';
    flags[3] = 0;

    return nst_string_new(flags,  3, true, err);
}

NST_FUNC_SIGN(println_)
{
    Nst_Obj *obj;
    Nst_Obj *flush;
    Nst_Obj *file_obj;
    NST_DEF_EXTRACT("o?b?F", &obj, &flush, &file_obj);
    Nst_IOFileObj *file = NST_DEF_VAL(file_obj, IOFILE(file_obj), nst_stdio()->out);

    if ( NST_IOF_IS_CLOSED(file) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    Nst_StrObj *s_obj = STR(nst_obj_cast(obj, nst_type()->Str, nullptr));
    nst_fprintln(file, (const i8 *)s_obj->value);

    if ( nst_obj_cast(flush, nst_type()->Bool, nullptr) == nst_true() )
    {
        nst_fflush(file);
        nst_dec_ref(nst_true());
    }
    else
    {
        nst_dec_ref(nst_false());
    }
    nst_dec_ref(s_obj);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(_set_stdin_)
{
    Nst_IOFileObj *f;

    NST_DEF_EXTRACT("F", &f);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( !NST_IOF_CAN_READ(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file must support reading");
        return nullptr;
    }

    if ( f == nst_stdio()->in )
    {
        NST_RETURN_NULL;
    }

    nst_dec_ref(nst_stdio()->in);
    nst_dec_ref(stdin_obj);
    nst_stdio()->in = IOFILE(nst_inc_ref(f));
    stdin_obj  = nst_inc_ref(f);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(_set_stdout_)
{
    Nst_IOFileObj *f;

    NST_DEF_EXTRACT("F", &f);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( !NST_IOF_CAN_WRITE(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file must support writing");
        return nullptr;
    }

    if ( f == nst_stdio()->out )
    {
        NST_RETURN_NULL;
    }

    nst_dec_ref(nst_stdio()->out);
    nst_dec_ref(stdout_obj);
    nst_stdio()->out = IOFILE(nst_inc_ref(f));
    stdout_obj  = nst_inc_ref(f);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(_set_stderr_)
{
    Nst_IOFileObj *f;

    NST_DEF_EXTRACT("F", &f);

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( !NST_IOF_CAN_WRITE(f) )
    {
        NST_SET_RAW_VALUE_ERROR("the file must support writing");
        return nullptr;
    }

    if ( f == nst_stdio()->err )
    {
        NST_RETURN_NULL;
    }

    nst_dec_ref(nst_stdio()->err);
    nst_dec_ref(stderr_obj);
    nst_stdio()->err = IOFILE(nst_inc_ref(f));
    stderr_obj  = nst_inc_ref(f);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(_get_stdin_)
{
    return nst_inc_ref(stdin_obj);
}

NST_FUNC_SIGN(_get_stdout_)
{
    return nst_inc_ref(stdout_obj);
}

NST_FUNC_SIGN(_get_stderr_)
{
    return nst_inc_ref(stderr_obj);
}
