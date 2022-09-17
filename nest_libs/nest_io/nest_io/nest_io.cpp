#include <cstdio>
#include <cstring>
#include <cerrno>
#include "nest_io.h"
#include "../../../nest/obj_ops.h"

#define FUNC_COUNT 13

#define SET_FILE_CLOSED_ERROR \
    err->name = NST_E_VALUE_ERROR; \
    err->message = (char *)"file given was closed"

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;
static Nst_Obj *stdin_obj;
static Nst_Obj *stdout_obj;
static Nst_Obj *stderr_obj;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(open_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(close_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(write_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(write_bytes_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(read_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(read_bytes_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(file_size_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(move_fptr_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_fptr_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(flush_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_stdin_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_stdout_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_stderr_, 0);

    stdin_obj = nst_new_file(stdin, false, true, false);
    stdout_obj = nst_new_file(stdout, false, false, true);
    stderr_obj = nst_new_file(stderr, false, false, true);

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

void free_lib()
{
    nst_dec_ref(stdin_obj);
    nst_dec_ref(stdout_obj);
    nst_dec_ref(stderr_obj);
}

NST_FUNC_SIGN(open_)
{
    Nst_StrObj *file_name_str;
    Nst_StrObj *file_mode_str;

    if ( !nst_extract_arg_values("ss", arg_num, args, err, &file_name_str, &file_mode_str) )
        return nullptr;

    char *file_name = file_name_str->value;
    char *file_mode = file_mode_str->value;

    bool is_bin = false;
    bool can_read = false;
    bool can_write = false;

    size_t file_mode_len = file_mode_str->len;

    if ( file_mode_len < 1 || file_mode_len > 3 )
    {
        NST_SET_VALUE_ERROR("file mode is not valid");
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
        NST_SET_VALUE_ERROR("file mode is not valid");
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
            NST_SET_VALUE_ERROR("file mode is not valid");
            return nullptr;
        }
    }
    else if ( file_mode_len == 3 )
    {
        is_bin = true;
        can_read = true;
        can_write = true;

        if ( (file_mode[1] != 'b' && file_mode[2] != '+') ||
             (file_mode[1] != '+' && file_mode[2] != 'b') )
        {
            NST_SET_VALUE_ERROR("file mode is not valid");
            return nullptr;
        }
    }

    Nst_IOfile file_ptr = fopen(file_name, file_mode);
    if ( file_ptr == nullptr )
        return nst_inc_ref(nst_null);

    return nst_new_file(file_ptr, is_bin, can_read, can_write);
}

NST_FUNC_SIGN(close_)
{
    Nst_IOFileObj *f;

    if ( !nst_extract_arg_values("F", arg_num, args, err, &f) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    fclose(f->value);
    f->value = nullptr;
    f->flags &= NST_FLAG_IOFILE_IS_CLOSED;
    
    return nst_inc_ref(nst_null);
}

NST_FUNC_SIGN(write_)
{
    Nst_Obj *value_to_write = args[1];
    Nst_IOFileObj *f;

    if ( !nst_extract_arg_values("F", 1, args, err, &f) )
        return nullptr;

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !NST_IOF_CAN_WRITE(f) )
    {
        NST_SET_VALUE_ERROR("the file does not support writing");
        return nullptr;
    }
    else if ( NST_IOF_IS_BIN(f) )
    {
        NST_SET_VALUE_ERROR("the file is binary, try using 'write_bytes'");
        return nullptr;
    }

    // casting to a string never returns an error
    Nst_Obj *str_to_write = nst_obj_cast(value_to_write, nst_t_str, nullptr);
    Nst_StrObj *str = AS_STR(str_to_write);
    fwrite(str->value, sizeof(char), str->len, f->value);

    nst_dec_ref(str_to_write);
    return nst_inc_ref(nst_null);
}

NST_FUNC_SIGN(write_bytes_)
{
    Nst_IOFileObj *f;
    Nst_SeqObj *seq;

    if ( !nst_extract_arg_values("FA", arg_num, args, err, &f, &seq) )
        return nullptr;

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !NST_IOF_CAN_WRITE(f) )
    {
        NST_SET_VALUE_ERROR("the file does not support writing");
        return nullptr;
    }
    else if ( !NST_IOF_IS_BIN(f) )
    {
        NST_SET_VALUE_ERROR("the file is not binary, try using 'write'");
        return nullptr;
    }

    size_t seq_len = seq->len;
    Nst_Obj **objs = seq->objs;
    char *bytes = new char[seq_len + 1];

    for ( size_t i = 0; i < seq_len; i++ )
    {
        if ( objs[i]->type != nst_t_byte )
        {
            delete[] bytes;
            NST_SET_TYPE_ERROR("expected 'Byte'");
            return nullptr;
        }

        bytes[i] = AS_BYTE(objs[i]);
    }

    fwrite(bytes, sizeof(char), seq_len, f->value);

    delete[] bytes;
    return nst_inc_ref(nst_null);
}

NST_FUNC_SIGN(read_)
{
    Nst_IOFileObj *f;
    Nst_Int bytes_to_read;

    if ( !nst_extract_arg_values("Fi", arg_num, args, err, &f, &bytes_to_read) )
        return nullptr;

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !NST_IOF_CAN_READ(f) )
    {
        NST_SET_VALUE_ERROR("the file does not support reading");
        return nullptr;
    }
    else if ( NST_IOF_IS_BIN(f) )
    {
        NST_SET_VALUE_ERROR("the file is binary, try using 'read_bytes'");
        return nullptr;
    }

    long start = ftell(f->value);
    fseek(f->value, 0, SEEK_END);
    long end = ftell(f->value);
    fseek(f->value, start, SEEK_SET); // pointer back to what it was before

    Nst_Int max_size = (Nst_Int)(end - start);
    if ( bytes_to_read < 0 || bytes_to_read > max_size )
        bytes_to_read = max_size;

    char *buffer = new char[(unsigned int)(bytes_to_read + 1)];
    size_t read_bytes = fread(buffer, sizeof(char), (size_t)bytes_to_read, f->value);
    buffer[read_bytes] = 0;

    return nst_new_string(buffer, read_bytes, true);
}

NST_FUNC_SIGN(read_bytes_)
{
    Nst_IOFileObj *f;
    Nst_Int bytes_to_read;

    if ( !nst_extract_arg_values("Fi", arg_num, args, err, &f, &bytes_to_read) )
        return nullptr;

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    else if ( !NST_IOF_CAN_READ(f) )
    {
        NST_SET_VALUE_ERROR("the file does not support reading");
        return nullptr;
    }
    else if ( !NST_IOF_IS_BIN(f) )
    {
        NST_SET_VALUE_ERROR("the file is not binary, try using 'read'");
        return nullptr;
    }

    long start = ftell(f->value);
    fseek(f->value, 0, SEEK_END);
    long end = ftell(f->value);
    fseek(f->value, start, SEEK_SET); // pointer back to what it was before

    Nst_Int max_size = (Nst_Int)(end - start);
    if ( bytes_to_read < 0 || bytes_to_read > max_size )
        bytes_to_read = max_size;

    char *buffer = new char[(unsigned int)bytes_to_read];
    size_t read_bytes = fread(buffer, sizeof(char), (size_t)bytes_to_read, f->value);

    Nst_SeqObj *bytes_array = AS_SEQ(nst_new_array(read_bytes));

    for ( size_t i = 0; i < read_bytes; i++ )
        bytes_array->objs[i] = nst_new_byte(buffer[i]);

    delete[] buffer;
    return (Nst_Obj *)bytes_array;
}

NST_FUNC_SIGN(file_size_)
{
    Nst_IOFileObj *f;

    if ( !nst_extract_arg_values("F", arg_num, args, err, &f) )
        return nullptr;

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    long start = ftell(f->value);
    fseek(f->value, 0, SEEK_END);
    long end = ftell(f->value);
    fseek(f->value, start, SEEK_SET);

    return nst_new_int(end);
}

NST_FUNC_SIGN(get_fptr_)
{
    Nst_IOFileObj *f;

    if ( !nst_extract_arg_values("F", arg_num, args, err, &f) )
        return nullptr;

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    return nst_new_int(ftell(f->value));
}

NST_FUNC_SIGN(move_fptr_)
{
    Nst_IOFileObj *f;
    Nst_Int start;
    Nst_Int offset;

    if ( !nst_extract_arg_values("Fii", arg_num, args, err, &f, &start, &offset) )
        return nullptr;

    if ( NST_IOF_IS_CLOSED(f) )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( start < 0 || start > 2 )
    {
        NST_SET_VALUE_ERROR("invalid origin for seek_fptr");
        return nullptr;
    }

    fseek(f->value, (long)offset, (int)start);

    return nst_inc_ref(nst_null);
}

NST_FUNC_SIGN(flush_)
{
    Nst_IOFileObj *f;

    if ( !nst_extract_arg_values("F", arg_num, args, err, &f) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    fflush(f->value);
    return nst_inc_ref(nst_null);
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
