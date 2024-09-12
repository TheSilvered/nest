#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include "nest_io.h"

#define SET_FILE_CLOSED_ERROR \
    Nst_set_value_error_c("the given file given was previously closed")

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(open_, 4),
    Nst_FUNCDECLR(virtual_file_, 2),
    Nst_FUNCDECLR(close_, 1),
    Nst_FUNCDECLR(write_, 2),
    Nst_FUNCDECLR(write_bytes_, 2),
    Nst_FUNCDECLR(read_, 2),
    Nst_FUNCDECLR(read_bytes_, 2),
    Nst_FUNCDECLR(file_size_, 1),
    Nst_FUNCDECLR(move_fpi_, 3),
    Nst_FUNCDECLR(get_fpi_, 1),
    Nst_FUNCDECLR(flush_, 1),
    Nst_FUNCDECLR(get_flags_, 1),
    Nst_FUNCDECLR(can_read_, 1),
    Nst_FUNCDECLR(can_write_, 1),
    Nst_FUNCDECLR(can_seek_, 1),
    Nst_FUNCDECLR(is_bin_, 1),
    Nst_FUNCDECLR(is_a_tty_, 1),
    Nst_FUNCDECLR(descriptor_, 1),
    Nst_FUNCDECLR(encoding_, 1),
    Nst_FUNCDECLR(println_, 3),
    Nst_FUNCDECLR(_set_stdin_, 1),
    Nst_FUNCDECLR(_set_stdout_, 1),
    Nst_FUNCDECLR(_set_stderr_, 1),
    Nst_FUNCDECLR(_get_stdin_, 0),
    Nst_FUNCDECLR(_get_stdout_, 0),
    Nst_FUNCDECLR(_get_stderr_, 0),
    Nst_DECLR_END
};
static Nst_Obj *stdin_obj;
static Nst_Obj *stdout_obj;
static Nst_Obj *stderr_obj;

Nst_Declr *lib_init()
{
    stdin_obj  = Nst_inc_ref(Nst_stdio()->in);
    stdout_obj = Nst_inc_ref(Nst_stdio()->out);
    stderr_obj = Nst_inc_ref(Nst_stdio()->err);

    return obj_list_;
}

void lib_quit()
{
    Nst_dec_ref(stdin_obj);
    Nst_dec_ref(stderr_obj);
    Nst_dec_ref(stdout_obj);
}

static usize get_file_size(Nst_IOFileObj *f)
{
    usize start, end;
    Nst_ftell(f, &start);
    Nst_fseek(Nst_SEEK_END, 0, f);
    Nst_ftell(f, &end);
    Nst_fseek(Nst_SEEK_SET, start, f);
    return end;
}

static Nst_IOResult virtual_iof_read(i8 *buf, usize buf_size, usize count,
                                     usize *buf_len, Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    VirtualIOFile_data *vf = (VirtualIOFile_data *)f->fp;

    if (vf->ptr >= vf->data.len)
        count = 0;

    if (Nst_IOF_IS_BIN(f)) {
        if (count > buf_size && buf_size != 0)
            count = buf_size;

        i8 *out_buf;
        if (buf_size == 0)
            out_buf = (i8 *)Nst_raw_malloc(count);
        else
            out_buf = buf;

        if (vf->ptr + count > vf->data.len)
            count = vf->data.len - (usize)vf->ptr;

        memcpy(buf, vf->data.data + vf->ptr, count);
        vf->ptr += count;

        if (buf_size == 0)
            *(i8 **)buf = out_buf;
        if (buf_len != NULL)
            *buf_len = count;

        if (vf->ptr == vf->data.len)
            return Nst_IO_EOF_REACHED;
        return Nst_IO_SUCCESS;
    }

    Nst_Buffer buffer;
    if (buf_size == 0) {
        if (!Nst_buffer_init(&buffer, count + 1))
            return Nst_IO_ALLOC_FAILED;
    } else {
        buffer.data = buf;
        buffer.cap = buf_size;
        buffer.len = 0;
    }

    for (usize i = 0; i < count; i++) {
        if (vf->data.len - vf->ptr == 0) {
            if (buf_size == 0)
                *(i8 **)buf = buffer.data;
            if (buf_len != NULL)
                *buf_len = buffer.len;
            return Nst_IO_EOF_REACHED;
        }

        i32 ch_size = Nst_check_ext_utf8_bytes(
            (u8 *)vf->data.data,
            vf->data.len - vf->ptr);

        if (ch_size == -1) {
            if (buf_size == 0)
                Nst_buffer_destroy(&buffer);
            return Nst_IO_INVALID_DECODING;
        }
        if (buf_size == 0 && !Nst_buffer_expand_by(&buffer, (usize)ch_size)) {
            Nst_buffer_destroy(&buffer);
            return Nst_IO_ALLOC_FAILED;
        }

        if (buffer.cap - buffer.len - 1 < (usize)ch_size) {
            if (buf_size == 0)
                *(i8 **)buf = buffer.data;
            if (buf_len != NULL)
                *buf_len = buffer.len;
            return Nst_IO_BUF_FULL;
        }
        memcpy(buffer.data + buffer.len, vf->data.data + vf->ptr, ch_size);
        buffer.len += ch_size;
        vf->ptr += ch_size;
    }
    if (buf_size == 0)
        *(i8 **)buf = buffer.data;
    if (buf_len != NULL)
        *buf_len = buffer.len;
    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_iof_write(i8 *buf, usize buf_len, usize *count,
                                      Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    VirtualIOFile_data *vf = (VirtualIOFile_data *)f->fp;

    if (vf->ptr > vf->data.len)
        return Nst_IO_ERROR;

    Nst_StrObj temp = Nst_str_temp(buf, buf_len);
    if (!Nst_buffer_append(&vf->data, &temp)) {
        return Nst_IO_ALLOC_FAILED;
    }
    vf->ptr += buf_len;

    if (count != NULL) {
        if (Nst_IOF_IS_BIN(f))
            *count = buf_len;
        else {
            usize char_count = 0;
            while (buf_len) {
                i32 ch_size = Nst_check_ext_utf8_bytes((u8 *)buf, buf_len);
                buf_len -= ch_size;
                buf += ch_size;
            }
            *count = char_count;
        }
    }
    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_iof_flush(Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_iof_tell(Nst_IOFileObj *f, usize *pos)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    *pos = ((VirtualIOFile_data *)f->fp)->ptr;
    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_iof_seek(Nst_SeekWhence origin, isize offset,
                                     Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    VirtualIOFile_data *vf = (VirtualIOFile_data *)f->fp;

    isize new_pos;
    if (origin == Nst_SEEK_CUR)
        new_pos = vf->ptr + offset;
    else if (origin == Nst_SEEK_SET)
        new_pos = offset;
    else
        new_pos = (i32)vf->data.len - offset;

    if (new_pos < 0)
        return Nst_IO_OP_FAILED;

    vf->ptr = new_pos;

    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_iof_close(Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    VirtualIOFile_data *vf = (VirtualIOFile_data *)f->fp;
    Nst_buffer_destroy(&vf->data);
    Nst_free(vf);

    return Nst_IO_SUCCESS;
}

static Nst_IOFuncSet vf_funcs = {
    .read = virtual_iof_read,
    .write = virtual_iof_write,
    .flush = virtual_iof_flush,
    .tell = virtual_iof_tell,
    .seek = virtual_iof_seek,
    .close = virtual_iof_close
};

Nst_Obj *NstC open_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *file_name_str;
    Nst_Obj *file_mode_str;
    Nst_Obj *encoding_obj;
    Nst_Obj *buf_size;
    if (!Nst_extract_args(
            "s ?s ?s ?i",
            arg_num, args,
            &file_name_str, &file_mode_str, &encoding_obj, &buf_size))
    {
        return nullptr;
    }

    i8 *file_name = file_name_str->value;
    i8 *file_mode = Nst_DEF_VAL(
        file_mode_str,
        STR(file_mode_str)->value,
        (i8 *)"r");
    usize file_mode_len = Nst_DEF_VAL(
        file_mode_str,
        STR(file_mode_str)->len, 1);

    bool is_bin = false;
    bool can_read = false;
    bool can_write = false;

    if (file_mode_len < 1 || file_mode_len > 3) {
        Nst_set_value_error_c("the file mode is not valid");
        return nullptr;
    }

    char bin_mode[4] = { 0, 'b', 0, 0 };

    switch (*file_mode) {
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

    if (file_mode_len == 2) {
        switch (file_mode[1]) {
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
    } else if (file_mode_len == 3) {
        is_bin = true;
        can_read = true;
        can_write = true;
        bin_mode[2] = '+';

        if (!(file_mode[1] == 'b' && file_mode[2] == '+')
            && !(file_mode[1] == '+' && file_mode[2] == 'b'))
        {
            Nst_set_value_error_c("the file mode is not valid");
            return nullptr;
        }
    }

    Nst_CP *encoding;
    if (is_bin) {
        if (encoding_obj != Nst_null()) {
            Nst_set_value_error_c(
                "encoding is not supported when the file is opened in binary"
                " mode");
            return nullptr;
        }
        encoding = NULL;
    } else {
        Nst_CPID cpid = Nst_DEF_VAL(
            encoding_obj,
            Nst_encoding_from_name(STR(encoding_obj)->value),
            Nst_CP_UTF8);
        if (cpid == Nst_CP_UNKNOWN) {
            Nst_set_value_errorf(
                "invalid encoding '%.100s'",
                STR(encoding_obj)->value);
            return nullptr;
        }

        cpid = Nst_single_byte_cp(cpid);

        encoding = Nst_cp(cpid);
    }

    FILE *file_ptr = Nst_fopen_unicode(file_name, bin_mode);

    if (file_ptr == nullptr) {
        if (!Nst_error_occurred()) {
            Nst_set_value_errorf("file '%.4096s' not found", file_name);
        }
        return nullptr;
    }

    if (buf_size != Nst_null()) {
        setvbuf(file_ptr, nullptr, _IOFBF, (usize)AS_INT(buf_size));
    }

    return Nst_iof_new(file_ptr, is_bin, can_read, can_write, encoding);
}

Nst_Obj *NstC virtual_file_(usize arg_num, Nst_Obj **args)
{
    bool bin;
    Nst_Obj *buf_size_obj;

    if (!Nst_extract_args("y ?i", arg_num, args, &bin, &buf_size_obj))
        return nullptr;

    i64 buf_size = Nst_DEF_VAL(buf_size_obj, AS_INT(buf_size_obj), 128);

    VirtualIOFile_data *f = Nst_malloc_c(1, VirtualIOFile_data);
    if (f == nullptr)
        return nullptr;

    if (!Nst_buffer_init(&f->data, usize(buf_size))) {
        Nst_free(f);
        return nullptr;
    }
    f->ptr = 0;

    return Nst_iof_new_fake(
        (void *)f,
        bin, true, true, true,
        Nst_cp(Nst_CP_UTF8),
        vf_funcs);
}

Nst_Obj *NstC close_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;

    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    Nst_fclose(f);

    Nst_RETURN_NULL;
}

Nst_Obj *NstC write_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *value_to_write;
    Nst_IOFileObj *f;

    if (!Nst_extract_args("F o", arg_num, args, &f, &value_to_write))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    } else if (!Nst_IOF_CAN_WRITE(f)) {
        Nst_set_value_error_c("the file does not support writing");
        return nullptr;
    } else if (Nst_IOF_IS_BIN(f)) {
        Nst_set_value_error_c("the file is binary, try using 'write_bytes'");
        return nullptr;
    }

    Nst_Obj *str_to_write = Nst_obj_cast(value_to_write, Nst_type()->Str);
    Nst_StrObj *str = STR(str_to_write);
    usize count;
    Nst_IOResult result = Nst_fwrite(str->value, str->len, &count, f);
    Nst_dec_ref(str_to_write);

    if (result == Nst_IO_INVALID_ENCODING) {
        u32 failed_ch;
        usize failed_pos;
        const i8 *name;
        Nst_io_result_get_details(&failed_ch, &failed_pos, &name);
        Nst_set_value_errorf(
            "could not encode U+%0*X at %zi for %s encoding",
            failed_ch > 0xffff ? 6 : 4,
            (int)failed_ch,
            failed_pos,
            name);
        return nullptr;
    } else if (result == Nst_IO_ERROR) {
        Nst_set_call_error_c("failed to write the entire string");
        return nullptr;
    } else if (result == Nst_IO_ALLOC_FAILED) {
        Nst_failed_allocation();
        return nullptr;
    }

    return Nst_int_new(count);
}

Nst_Obj *NstC write_bytes_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    Nst_SeqObj *seq;

    if (!Nst_extract_args("F A.B", arg_num, args, &f, &seq))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    } else if (!Nst_IOF_CAN_WRITE(f)) {
        Nst_set_value_error_c("the file does not support writing");
        return nullptr;
    } else if (!Nst_IOF_IS_BIN(f)) {
        Nst_set_value_error_c("the file is not binary, try using 'write'");
        return nullptr;
    }

    usize seq_len = seq->len;
    Nst_Obj **objs = seq->objs;
    i8 *bytes = Nst_malloc_c(seq_len + 1, i8);
    if (bytes == nullptr)
        return nullptr;

    for (usize i = 0; i < seq_len; i++)
        bytes[i] = AS_BYTE(objs[i]);

    usize count;
    Nst_IOResult result = Nst_fwrite(bytes, seq_len, &count, f);
    Nst_free(bytes);
    if (result == Nst_IO_ERROR) {
        Nst_set_call_error_c("failed to write all bytes");
        return nullptr;
    } else if (result == Nst_IO_ALLOC_FAILED) {
        Nst_failed_allocation();
        return nullptr;
    }
    return Nst_int_new(count);
}

Nst_Obj *NstC read_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    Nst_Obj *bytes_to_read_obj;

    if (!Nst_extract_args("F ?i", arg_num, args, &f, &bytes_to_read_obj))
        return nullptr;
    i64 bytes_to_read = Nst_DEF_VAL(
        bytes_to_read_obj,
        AS_INT(bytes_to_read_obj),
        -1);

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    } else if (!Nst_IOF_CAN_READ(f)) {
        Nst_set_value_error_c("the file does not support reading");
        return nullptr;
    } else if (Nst_IOF_IS_BIN(f)) {
        Nst_set_value_error_c("the file is binary, try using 'read_bytes'");
        return nullptr;
    }

    if (!Nst_IOF_CAN_SEEK(f) && bytes_to_read < 0) {
        Nst_set_value_error_c("the file must be seekable to read it entierly");
        return nullptr;
    } else if (Nst_IOF_CAN_SEEK(f)) {
        usize start;
        Nst_ftell(f, &start);
        usize end = get_file_size(f);

        i64 max_size = (i64)(end - start);
        if (bytes_to_read < 0 || bytes_to_read > max_size)
            bytes_to_read = max_size;
    }

    i8 *buf;
    usize buf_len;
    Nst_IOResult result = Nst_fread(
        (i8 *)&buf, 0,
        usize(bytes_to_read), &buf_len,
        f);

    if (result == Nst_IO_ALLOC_FAILED) {
        Nst_failed_allocation();
        return nullptr;
    } else if (result == Nst_IO_INVALID_DECODING) {
        u32 failed_ch;
        usize failed_pos;
        const i8 *name;
        Nst_io_result_get_details(&failed_ch, &failed_pos, &name);
        Nst_set_value_errorf(
            "could not decode byte %#x at position %zi for %s encoding",
            (int)failed_ch,
            failed_pos,
            name);
        return nullptr;
    } else if (result == Nst_IO_ERROR) {
        Nst_set_call_error_c("failed to read the file");
        return nullptr;
    }

    return Nst_str_new(buf, buf_len, true);
}

Nst_Obj *NstC read_bytes_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    Nst_Obj *bytes_to_read_obj;

    if (!Nst_extract_args("F ?i", arg_num, args, &f, &bytes_to_read_obj))
        return nullptr;
    i64 bytes_to_read = Nst_DEF_VAL(
        bytes_to_read_obj,
        AS_INT(bytes_to_read_obj),
        -1);

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    } else if (!Nst_IOF_CAN_READ(f)) {
        Nst_set_value_error_c("the file does not support reading");
        return nullptr;
    } else if (!Nst_IOF_IS_BIN(f)) {
        Nst_set_value_error_c("the file is not binary, try using 'read'");
        return nullptr;
    }

    if (!Nst_IOF_CAN_SEEK(f) && bytes_to_read < 0) {
        Nst_set_value_error_c("the file must be seekable to read it entierly");
        return nullptr;
    } else if (Nst_IOF_CAN_SEEK(f)) {
        usize start;
        Nst_ftell(f, &start);
        usize end = get_file_size(f);

        i64 max_size = (i64)(end - start);
        if (bytes_to_read < 0 || bytes_to_read > max_size)
            bytes_to_read = max_size;
    }

    i8 *buf;
    usize buf_len;
    Nst_IOResult result = Nst_fread(
        (i8 *)&buf, 0,
        usize(bytes_to_read), &buf_len,
        f);

    if (result == Nst_IO_ALLOC_FAILED) {
        Nst_failed_allocation();
        return nullptr;
    } else if (result == Nst_IO_ERROR) {
        Nst_set_call_error_c("failed to read the file");
        return nullptr;
    }

    Nst_SeqObj *bytes_array = SEQ(Nst_array_new(buf_len));

    for (usize i = 0; i < buf_len; i++)
        bytes_array->objs[i] = Nst_byte_new(buf[i]);

    Nst_free(buf);
    return OBJ(bytes_array);
}

Nst_Obj *NstC file_size_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;

    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    } else if (!Nst_IOF_CAN_SEEK(f)) {
        Nst_set_value_error_c("the file cannot be seeked");
        return nullptr;
    }

    return Nst_int_new(get_file_size(f));
}

Nst_Obj *NstC get_fpi_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;

    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    if (!Nst_IOF_CAN_SEEK(f)) {
        Nst_set_value_error_c("the file cannot be seeked");
        return nullptr;
    }

    usize pos;
    if (Nst_ftell(f, &pos) == Nst_IO_ERROR) {
        Nst_set_call_error_c(
            "failed to get the position of the file-position indicator");
        return nullptr;
    }

    return Nst_int_new((i64)pos);
}


Nst_Obj *NstC move_fpi_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    i64 start;
    i64 offset;

    if (!Nst_extract_args("F i i", arg_num, args, &f, &start, &offset))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    if (!Nst_IOF_CAN_SEEK(f)) {
        Nst_set_value_error_c("the file cannot be seeked");
        return nullptr;
    }

    if (start < 0 || start > 2) {
        Nst_set_value_errorf("invalid origin '%lli'", start);
        return nullptr;
    }

    usize size = get_file_size(f);
    isize end_pos = 0;

    if (start == SEEK_END)
        end_pos = size + isize(offset);
    else if (start == SEEK_SET)
        end_pos = isize(offset);
    else {
        if (Nst_ftell(f, (usize *)&end_pos) != Nst_IO_ERROR)
            end_pos += isize(offset);
        else
            end_pos = 0;
    }

    if (end_pos < 0 || end_pos > (isize)size) {
        Nst_set_value_error_c(
            "the file-position indicator goes outside the file");
        return nullptr;
    }

    if (Nst_fseek((Nst_SeekWhence)start, (isize)offset, f) == Nst_IO_ERROR) {
        Nst_set_call_error_c(
            "failed to change the position of the file-position indicator");
    }

    Nst_RETURN_NULL;
}

Nst_Obj *NstC flush_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;

    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    } else if (!Nst_IOF_CAN_WRITE(f)) {
        Nst_set_value_error_c("the file cannot be written");
        return nullptr;
    }

    Nst_IOResult result = Nst_fflush(f);
    if (result == Nst_IO_ERROR) {
        Nst_set_memory_error_c("failed to flush the file");
        return nullptr;
    } else if (result == Nst_IO_ALLOC_FAILED) {
        Nst_failed_allocation();
        return nullptr;
    }

    Nst_RETURN_NULL;
}

Nst_Obj *NstC get_flags_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;

    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;

    i8 *flags = Nst_malloc_c(6, i8);
    if (flags == nullptr)
        return nullptr;

    flags[0] = Nst_IOF_CAN_READ(f)  ? 'r' : '-';
    flags[1] = Nst_IOF_CAN_WRITE(f) ? 'w' : '-';
    flags[2] = Nst_IOF_IS_BIN(f)    ? 'b' : '-';
    flags[3] = Nst_IOF_CAN_SEEK(f)  ? 's' : '-';
    flags[4] = Nst_IOF_IS_TTY(f)    ? 't' : '-';
    flags[5] = 0;

    return Nst_str_new(flags, 5, true);
}

Nst_Obj *NstC can_read_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    Nst_RETURN_BOOL(Nst_IOF_CAN_READ(f));
}

Nst_Obj *NstC can_write_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    Nst_RETURN_BOOL(Nst_IOF_CAN_WRITE(f));
}

Nst_Obj *NstC can_seek_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    Nst_RETURN_BOOL(Nst_IOF_CAN_SEEK(f));
}

Nst_Obj *NstC is_bin_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    Nst_RETURN_BOOL(Nst_IOF_IS_BIN(f));
}

Nst_Obj *NstC is_a_tty_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    Nst_RETURN_BOOL(Nst_IOF_IS_TTY(f));
}

Nst_Obj *NstC descriptor_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    return Nst_int_new(f->fd);
}

Nst_Obj *NstC encoding_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    if (f->encoding == NULL) {
        if (Nst_IOF_IS_CLOSED(f))
            SET_FILE_CLOSED_ERROR;
        else if (Nst_IOF_IS_BIN(f))
            Nst_set_type_error_c("cannot get the encoding of a binary file");
        else
            Nst_set_value_error_c("failed to get the encoding of the file");
        return nullptr;
    }
    return Nst_str_new_c_raw(f->encoding->name, false);
}

Nst_Obj *NstC println_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *obj;
    bool flush;
    Nst_Obj *file_obj;
    if (!Nst_extract_args("o y ?F", arg_num, args, &obj, &flush, &file_obj))
        return nullptr;

    Nst_IOFileObj *file = Nst_DEF_VAL(
        file_obj,
        IOFILE(file_obj),
        Nst_stdio()->out);

    if (Nst_IOF_IS_CLOSED(file)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    } else if (!Nst_IOF_CAN_WRITE(file)) {
        Nst_set_value_error_c("the file cannot be written");
        return nullptr;
    }

    Nst_StrObj *s_obj = STR(Nst_obj_cast(obj, Nst_type()->Str));
    Nst_IOResult res = Nst_fwrite(s_obj->value, s_obj->len, NULL, file);
    Nst_fprintln(file, "");

    if (res == Nst_IO_SUCCESS and flush)
        res = Nst_fflush(file);

    if (res == Nst_IO_ALLOC_FAILED) {
        Nst_failed_allocation();
        return nullptr;
    } else if (res == Nst_IO_INVALID_ENCODING) {
        u32 failed_ch;
        usize failed_pos;
        const i8 *name;
        Nst_io_result_get_details(&failed_ch, &failed_pos, &name);
        Nst_set_value_errorf(
            "could not encode U+%0*X at %zi for %s encoding",
            failed_ch > 0xffff ? 6 : 4,
            (int)failed_ch,
            failed_pos,
            name);
        return nullptr;
    } else if (res == Nst_IO_ERROR) {
        Nst_set_call_error_c("failed to write to the file");
        return nullptr;
    }

    Nst_dec_ref(s_obj);
    Nst_RETURN_NULL;
}

Nst_Obj *NstC _set_stdin_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;

    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if (!Nst_IOF_CAN_READ(f)) {
        Nst_set_value_error_c("the file must support reading");
        return nullptr;
    }

    if (f == Nst_stdio()->in)
        Nst_RETURN_NULL;

    Nst_dec_ref(Nst_stdio()->in);
    Nst_dec_ref(stdin_obj);
    Nst_stdio()->in = IOFILE(Nst_inc_ref(f));
    stdin_obj = Nst_inc_ref(f);
    Nst_RETURN_NULL;
}

Nst_Obj *NstC _set_stdout_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;

    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if (!Nst_IOF_CAN_WRITE(f)) {
        Nst_set_value_error_c("the file must support writing");
        return nullptr;
    }

    if (f == Nst_stdio()->out)
        Nst_RETURN_NULL;

    Nst_dec_ref(Nst_stdio()->out);
    Nst_dec_ref(stdout_obj);
    Nst_stdio()->out = IOFILE(Nst_inc_ref(f));
    stdout_obj = Nst_inc_ref(f);
    Nst_RETURN_NULL;
}

Nst_Obj *NstC _set_stderr_(usize arg_num, Nst_Obj **args)
{
    Nst_IOFileObj *f;

    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

    if (!Nst_IOF_CAN_WRITE(f)) {
        Nst_set_value_error_c("the file must support writing");
        return nullptr;
    }

    if (f == Nst_stdio()->err)
        Nst_RETURN_NULL;

    Nst_dec_ref(Nst_stdio()->err);
    Nst_dec_ref(stderr_obj);
    Nst_stdio()->err = IOFILE(Nst_inc_ref(f));
    stderr_obj = Nst_inc_ref(f);
    Nst_RETURN_NULL;
}

Nst_Obj *NstC _get_stdin_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(stdin_obj);
}

Nst_Obj *NstC _get_stdout_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(stdout_obj);
}

Nst_Obj *NstC _get_stderr_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(stderr_obj);
}
