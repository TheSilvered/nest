#include <cstdio>
#include <cstring>
#include <cerrno>
#include "nest_io.h"
#include "nest_source/obj_ops.h"

#define FUNC_COUNT 9

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
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == NULL )
        return false;

    // Set each function to an index in func_list_
    // func_list_[0] = {
    //     func_ptr, -> the function pointer
    //     1, -> the number of arguments the function takes
    //     new_string_raw("func_name", false) -> the string containing the name
    // }                                        of the funtion inside Nest

    func_list_[0] = { open_file,  2, new_string_raw("open", false) };
    func_list_[1] = { close_file, 1, new_string_raw("close", false) };
    func_list_[2] = { write_to_file,  2, new_string_raw("write", false) };
    func_list_[3] = { read_from_file, 2, new_string_raw("read",  false) };
    func_list_[4] = { get_file_size, 1, new_string_raw("file_size", false) };
    func_list_[5] = { move_file_pointer, 2, new_string_raw("seek_fptr", false) };
    func_list_[5] = { get_file_pointer, 2, new_string_raw("get_fptr", false) };
    func_list_[6] = { get_stdin,  0, new_string_raw("_get_stdin",  false) };
    func_list_[7] = { get_stdout, 0, new_string_raw("_get_stdout", false) };
    func_list_[8] = { get_stderr, 0, new_string_raw("_get_stderr", false) };

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : NULL;
}

Nst_Obj *open_file(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *file_name_obj = args[0];
    Nst_Obj *file_mode_obj = args[1];

    if ( file_name_obj->type != nst_t_str )
    {
        SET_TYPE_ERROR("file name must be a string");
        return NULL;
    }

    if ( file_mode_obj->type != nst_t_str )
    {
        SET_TYPE_ERROR("file mode must be a string");
        return NULL;
    }

    char *file_name = AS_STR(file_name_obj)->value;
    char *file_mode = AS_STR(file_mode_obj)->value;

    size_t file_mode_len = AS_STR(file_mode_obj)->len;

    if ( file_mode_len == 1 )
    {
        if ( *file_mode != 'r' && *file_mode != 'w' && *file_mode != 'a' )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return NULL;
        }
    }
    else if ( file_mode_len == 2 )
    {
        if ( *file_mode != 'r' && *file_mode != 'w' && *file_mode != 'a' )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return NULL;
        }

        if ( file_mode[1] != 'b' && file_mode[1] != '+' )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return NULL;
        }
    }
    else if ( file_mode_len == 3 )
    {
        if ( *file_mode != 'r' && *file_mode != 'w' && *file_mode != 'a' )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return NULL;
        }

        if ( (file_mode[1] != 'b' && file_mode[2] != '+') ||
             (file_mode[1] != '+' && file_mode[2] != 'b') )
        {
            SET_VALUE_ERROR("file mode is not valid");
            return NULL;
        }
    }
    else
    {
        SET_VALUE_ERROR("file mode is not valid");
        return NULL;
    }

    Nst_iofile *file_ptr = fopen(file_name, file_mode);
    if ( file_ptr == NULL )
    {
        inc_ref(nst_null);
        return nst_null;
    }

    return make_obj(file_ptr, nst_t_file, NULL);
}

Nst_Obj *close_file(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *file = args[0];

    if ( file->type != nst_t_file )
    {
        SET_TYPE_ERROR("expected 'IOfile'");
        return NULL;
    }

    if ( AS_FILE(file) == NULL )
    {
        SET_FILE_CLOSED_ERROR;
        return NULL;
    }

    fclose((Nst_iofile *)(file->value));
    file->value = NULL;
    
    inc_ref(nst_null);
    return nst_null;
}

Nst_Obj *write_to_file(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *file = args[0];
    Nst_Obj *value_to_write = args[1];

    if ( file->type != nst_t_file )
    {
        SET_TYPE_ERROR("expected 'IOfile'");
        return NULL;
    }

    if ( (Nst_iofile *)(file->value) == NULL )
    {
        SET_FILE_CLOSED_ERROR;
        return NULL;
    }

    // casting to a string never returns an error
    Nst_Obj *str_to_write = obj_cast(value_to_write, nst_t_str, NULL);
    Nst_string *str = AS_STR(str_to_write);

    fwrite(str->value, sizeof(char), str->len, (Nst_iofile *)(file->value));

    dec_ref(str_to_write);

    inc_ref(nst_null);
    return nst_null;
}

Nst_Obj *read_from_file(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *file = args[0];
    Nst_Obj *bytes_to_read_obj = args[1];

    if ( file->type != nst_t_file )
    {
        SET_TYPE_ERROR("expected 'IOfile'");
        return NULL;
    }

    if ( (Nst_iofile *)(file->value) == NULL )
    {
        SET_FILE_CLOSED_ERROR;
        return NULL;
    }

    Nst_iofile *file_ptr = (Nst_iofile *)(file->value);

    if ( bytes_to_read_obj->type != nst_t_int )
    {
        SET_TYPE_ERROR("expected 'Int'");
        return NULL;
    }

    Nst_int bytes_to_read = AS_INT(bytes_to_read_obj);

    long start = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_END);
    long end = ftell(file_ptr);
    fseek(file_ptr, start, SEEK_SET); // pointer back to what it was before

    Nst_int max_size = (Nst_int)(end - start);
    if ( bytes_to_read < 0 || bytes_to_read > max_size )
        bytes_to_read = max_size;

    Nst_sequence *bytes_array = new_array_empty(bytes_to_read);
    char *buffer = new char[bytes_to_read];
    fread(buffer, sizeof(char), bytes_to_read, file_ptr);

    for ( size_t i = 0; i < (size_t)bytes_to_read; i++ )
        bytes_array->objs[i] = new_byte_obj(buffer[i]);

    delete[] buffer;
    return make_obj(bytes_array, nst_t_arr, (void (*)(void *))destroy_seq);
}

Nst_Obj *get_file_size(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *file = args[0];

    if ( file->type != nst_t_file )
    {
        SET_TYPE_ERROR("expected 'IOfile'");
        return NULL;
    }

    if ( (Nst_iofile *)(file->value) == NULL )
    {
        SET_FILE_CLOSED_ERROR;
        return NULL;
    }

    Nst_iofile *file_ptr = (Nst_iofile *)(file->value);

    long start = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_END);
    long end = ftell(file_ptr);
    fseek(file_ptr, start, SEEK_SET);

    return new_int_obj(end);
}

Nst_Obj *get_file_pointer(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *file = args[0];

    if ( file->type != nst_t_file )
    {
        SET_TYPE_ERROR("expected 'IOfile'");
        return NULL;
    }

    if ( (Nst_iofile *)(file->value) == NULL )
    {
        SET_FILE_CLOSED_ERROR;
        return NULL;
    }

    return new_int_obj(ftell((Nst_iofile *)(file->value)));
}

Nst_Obj *move_file_pointer(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *file = args[0];
    Nst_Obj *start_obj = args[1];
    Nst_Obj *offset_obj = args[2];

    if ( file->type != nst_t_file )
    {
        SET_TYPE_ERROR("expected 'IOfile'");
        return NULL;
    }

    if ( (Nst_iofile *)(file->value) == NULL )
    {
        SET_FILE_CLOSED_ERROR;
        return NULL;
    }

    if ( file->type != nst_t_file )
    {
        SET_TYPE_ERROR("expected 'IOfile'");
        return NULL;
    }

    if ( start_obj->type != nst_t_int || offset_obj->type != nst_t_int )
    {
        SET_TYPE_ERROR("expected 'Int'");
        return NULL;
    }

    Nst_iofile *file_ptr = (Nst_iofile *)(file->value);
    int start = (int)AS_INT(start_obj);
    long offset = (long)AS_INT(offset_obj);

    if ( start < 0 || start > 2 )
    {
        SET_VALUE_ERROR("invalid origin for seek_fptr");
        return NULL;
    }

    fseek(file_ptr, offset, start);

    inc_ref(nst_null);
    return nst_null;
}

Nst_Obj *get_stdin(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return make_obj(stdin, nst_t_file, NULL);
}

Nst_Obj *get_stdout(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return make_obj(stdout, nst_t_file, NULL);
}

Nst_Obj *get_stderr(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return make_obj(stderr, nst_t_file, NULL);
}
