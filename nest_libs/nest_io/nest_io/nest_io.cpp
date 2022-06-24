#include <cstdio>
#include <cstring>
#include <cerrno>
#include "nest_io.h"
#include "nest_source/obj_ops.h"

#define FUNC_COUNT 12

#define SET_TYPE_ERROR(msg) \
    err->name = (char *)"Type Error"; \
    err->message = (char *)msg

#define SET_VALUE_ERROR(msg) \
    err->name = (char *)"Value Error"; \
    err->message = (char *)msg

#define SET_FILE_CLOSED_ERROR \
    err->name = (char *)"Value Error"; \
    err->message = (char *)"file given was closed"

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = MAKE_FUNCDECLR(open, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(close, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(write, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(write_bytes, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(read, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(read_bytes, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(file_size, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(move_fptr, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(get_fptr, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(_get_stdin, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_get_stdout, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_get_stderr, 0);

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

Nst_Obj *open(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_string *file_name_str;
    Nst_string *file_mode_str;

    if ( !extract_arg_values("ss", arg_num, args, err, &file_name_str, &file_mode_str) )
        return nullptr;

    char *file_name = file_name_str->value;
    char *file_mode = file_mode_str->value;

    size_t file_mode_len = file_mode_str->len;

    if ( file_mode_len == 1 )
    {
        if ( *file_mode != 'r' && *file_mode != 'w' && *file_mode != 'a' )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return nullptr;
        }
    }
    else if ( file_mode_len == 2 )
    {
        if ( *file_mode != 'r' && *file_mode != 'w' && *file_mode != 'a' )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return nullptr;
        }

        if ( file_mode[1] != 'b' && file_mode[1] != '+' )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return nullptr;
        }
    }
    else if ( file_mode_len == 3 )
    {
        if ( *file_mode != 'r' && *file_mode != 'w' && *file_mode != 'a' )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return nullptr;
        }

        if ( (file_mode[1] != 'b' && file_mode[2] != '+') ||
             (file_mode[1] != '+' && file_mode[2] != 'b') )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return nullptr;
        }
    }
    else
    {
        SET_VALUE_ERROR("file mode is not valid");
        return nullptr;
    }

    Nst_iofile *file_ptr = fopen(file_name, file_mode);
    if ( file_ptr == nullptr )
    {
        inc_ref(nst_null);
        return nst_null;
    }

    return make_obj(file_ptr, nst_t_file, nullptr);
}

Nst_Obj *close(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_iofile *f;

    if ( !extract_arg_values("F", arg_num, args, err, &f) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    fclose(f);
    args[0]->value = nullptr;
    
    inc_ref(nst_null);
    return nst_null;
}

Nst_Obj *write(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *value_to_write = args[1];
    Nst_iofile *f;

    if ( !extract_arg_values("F", 1, args, err, &f) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    // casting to a string never returns an error
    Nst_Obj *str_to_write = obj_cast(value_to_write, nst_t_str, nullptr);
    Nst_string *str = AS_STR(str_to_write);
    fwrite(str->value, sizeof(char), str->len, f);

    dec_ref(str_to_write);
    inc_ref(nst_null);
    return nst_null;
}

Nst_Obj *write_bytes(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_iofile *f;
    Nst_sequence *seq;

    if ( !extract_arg_values("Fa", arg_num, args, err, &f, &seq) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
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
            SET_TYPE_ERROR("expected 'Byte'");
            return nullptr;
        }

        bytes[i] = AS_BYTE(objs[i]);
    }

    fwrite(bytes, sizeof(char), seq_len, f);

    delete[] bytes;
    inc_ref(nst_null);
    return nst_null;
}

Nst_Obj *read(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_iofile *f;
    Nst_int bytes_to_read;

    if ( !extract_arg_values("Fi", arg_num, args, err, &f, &bytes_to_read) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    long start = ftell(f);
    fseek(f, 0, SEEK_END);
    long end = ftell(f);
    fseek(f, start, SEEK_SET); // pointer back to what it was before

    Nst_int max_size = (Nst_int)(end - start);
    if ( bytes_to_read < 0 || bytes_to_read > max_size )
        bytes_to_read = max_size;

    char *buffer = new char[bytes_to_read + 1];
    size_t read_bytes = fread(buffer, sizeof(char), bytes_to_read, f);
    buffer[read_bytes] = 0;

    return new_str_obj(new_string(buffer, read_bytes, true));
}

Nst_Obj *read_bytes(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_iofile *f;
    Nst_int bytes_to_read;

    if ( !extract_arg_values("Fi", arg_num, args, err, &f, &bytes_to_read) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    long start = ftell(f);
    fseek(f, 0, SEEK_END);
    long end = ftell(f);
    fseek(f, start, SEEK_SET); // pointer back to what it was before

    Nst_int max_size = (Nst_int)(end - start);
    if ( bytes_to_read < 0 || bytes_to_read > max_size )
        bytes_to_read = max_size;

    char *buffer = new char[bytes_to_read];
    size_t read_bytes = fread(buffer, sizeof(char), bytes_to_read, f);

    Nst_sequence *bytes_array = new_array_empty(read_bytes);

    for ( size_t i = 0; i < read_bytes; i++ )
        bytes_array->objs[i] = new_byte_obj(buffer[i]);

    delete[] buffer;
    return make_obj(bytes_array, nst_t_arr, (void (*)(void *))destroy_seq);
}

Nst_Obj *file_size(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_iofile *f;

    if ( !extract_arg_values("F", arg_num, args, err, &f) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    long start = ftell(f);
    fseek(f, 0, SEEK_END);
    long end = ftell(f);
    fseek(f, start, SEEK_SET);

    return new_int_obj(end);
}

Nst_Obj *get_fptr(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_iofile *f;

    if ( !extract_arg_values("F", arg_num, args, err, &f) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    return new_int_obj(ftell(f));
}

Nst_Obj *move_fptr(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_iofile *f;
    Nst_int start;
    Nst_int offset;

    if ( !extract_arg_values("Fii", arg_num, args, err, &f, &start, &offset) )
        return nullptr;

    if ( f == nullptr )
    {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if ( start < 0 || start > 2 )
    {
        SET_VALUE_ERROR("invalid origin for seek_fptr");
        return nullptr;
    }

    fseek(f, (long)offset, (int)start);

    inc_ref(nst_null);
    return nst_null;
}

Nst_Obj *_get_stdin(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return make_obj(stdin, nst_t_file, nullptr);
}

Nst_Obj *_get_stdout(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return make_obj(stdout, nst_t_file, nullptr);
}

Nst_Obj *_get_stderr(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return make_obj(stderr, nst_t_file, nullptr);
}
