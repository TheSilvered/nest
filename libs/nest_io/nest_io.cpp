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
    Nst_FUNCDECLR(seek_, 3),
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

static usize get_file_size(Nst_Obj *f)
{
    usize start, end;
    Nst_ftell(f, &start);
    Nst_fseek(Nst_SEEK_END, 0, f);
    Nst_ftell(f, &end);
    Nst_fseek(Nst_SEEK_SET, start, f);
    return end;
}

static Nst_IOResult virtual_file_read(i8 *buf, usize buf_size, usize count,
                                      usize *buf_len, Nst_Obj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    // virtual files always support reading

    VirtualFile *vf = (VirtualFile *)Nst_iof_fp(f);
    i8 *out_buf = buf_size == 0 ? NULL : buf;

    if (Nst_IOF_IS_BIN(f)) {
        // limit count to stay within the boundaries of the file
        if (count > buf_size && buf_size != 0)
            count = buf_size;
        if (vf->ptr + count > vf->data.len)
            count = vf->data.len - vf->ptr;

        if (out_buf == NULL) {
            out_buf = (i8 *)Nst_raw_malloc(count);
            if (out_buf == NULL)
                return Nst_IO_ALLOC_FAILED;
        }

        memcpy(out_buf, (i8 *)vf->data.data + vf->ptr, count);
        vf->ptr += count;

        if (buf_size == 0)
            *(i8 **)buf = out_buf;
        if (buf_len != NULL)
            *buf_len = count;

        if (vf->ptr == vf->data.len)
            return Nst_IO_EOF_REACHED;
        return Nst_IO_SUCCESS;
    }

    if (vf->ptr >= vf->data.len)
        count = 0;

    // count the bytes until the desired number of characters is reached, the
    // end of the file is reached or the buffer size limit is reached
    usize byte_count = 0;
    while (count > 0) {
        if (vf->data.len == vf->ptr + byte_count)
            break;

        i32 ch_size = Nst_check_ext_utf8_bytes(
            (u8 *)vf->data.data + vf->ptr + byte_count,
            vf->data.len - vf->ptr - byte_count);
        if (ch_size == -1) {
            Nst_io_result_set_details(
                (u32)*((i8 *)vf->data.data + vf->ptr + byte_count),
                vf->ptr + byte_count,
                Nst_encoding(Nst_EID_EXT_UTF8)->name);
            return Nst_IO_ALLOC_FAILED;
        }
        if (buf_size != 0 && byte_count + ch_size >= buf_size)
            break;
        byte_count += ch_size;
        count--;
    }

    if (out_buf == NULL) {
        out_buf = Nst_malloc_c(byte_count + 1, i8);
        if (out_buf == NULL)
            return Nst_IO_ALLOC_FAILED;
    }

    // the virtual file stores text in extUTF8, just copy the contents
    memcpy(out_buf, (i8 *)vf->data.data + vf->ptr, byte_count);
    out_buf[byte_count] = '\0';
    vf->ptr += byte_count;

    if (buf_size == 0)
        *(i8 **)buf = out_buf;
    if (buf_len != NULL)
        *buf_len = byte_count;

    if (vf->ptr == vf->data.len)
        return Nst_IO_EOF_REACHED;
    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_file_write(i8 *buf, usize buf_len, usize *count,
                                       Nst_Obj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    // virtual files always support writing

    VirtualFile *vf = (VirtualFile *)Nst_iof_fp(f);

    if (count != NULL && Nst_IOF_IS_BIN(f))
        *count = buf_len;
    else if (count != NULL) {
        usize char_count = 0;
        usize buf_len_cpy = buf_len;
        i8 *buf_cpy = buf;
        while (buf_len_cpy) {
            i32 ch_size = Nst_check_ext_utf8_bytes((u8 *)buf_cpy, buf_len_cpy);
            // buf is expected to be properly encoded but check nevertheless
            if (ch_size < 0)
                return Nst_IO_ERROR;
            buf_len_cpy -= ch_size;
            buf_cpy += ch_size;
        }
        *count = char_count;
    }

    isize space_needed = (isize)vf->ptr + (isize)buf_len - (isize)vf->data.len;
    if (space_needed > 0 && !Nst_sbuffer_expand_by(&vf->data, space_needed))
        return Nst_IO_ALLOC_FAILED;

    memcpy((u8 *)vf->data.data + vf->ptr, buf, buf_len);
    vf->ptr += buf_len;
    if (vf->ptr > vf->data.len)
        vf->data.len = vf->ptr;

    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_file_flush(Nst_Obj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_file_tell(Nst_Obj *f, usize *pos)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    *pos = ((VirtualFile *)Nst_iof_fp(f))->ptr;
    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_file_seek(Nst_SeekWhence origin, isize offset,
                                      Nst_Obj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    VirtualFile *vf = (VirtualFile *)Nst_iof_fp(f);

    isize new_pos;
    if (origin == Nst_SEEK_CUR)
        new_pos = vf->ptr + offset;
    else if (origin == Nst_SEEK_SET)
        new_pos = offset;
    else
        new_pos = (i32)vf->data.len - offset;

    // clamp the position into a valid value
    if (new_pos < 0)
        new_pos = 0;
    else if ((usize)new_pos > vf->data.len)
        new_pos = vf->data.len;

    vf->ptr = new_pos;

    return Nst_IO_SUCCESS;
}

static Nst_IOResult virtual_file_close(Nst_Obj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    VirtualFile *vf = (VirtualFile *)Nst_iof_fp(f);
    Nst_sbuffer_destroy(&vf->data);
    Nst_free(vf);

    return Nst_IO_SUCCESS;
}

static Nst_IOFuncSet vf_funcs = {
    .read = virtual_file_read,
    .write = virtual_file_write,
    .flush = virtual_file_flush,
    .tell = virtual_file_tell,
    .seek = virtual_file_seek,
    .close = virtual_file_close
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

    Nst_Encoding *encoding;
    if (is_bin) {
        if (encoding_obj != Nst_null()) {
            Nst_set_value_error_c(
                "encoding is not supported when the file is opened in binary"
                " mode");
            return nullptr;
        }
        encoding = NULL;
    } else {
        Nst_EncodingID cpid = Nst_DEF_VAL(
            encoding_obj,
            Nst_encoding_from_name(STR(encoding_obj)->value),
            Nst_EID_UTF8);
        if (cpid == Nst_EID_UNKNOWN) {
            Nst_set_value_errorf(
                "invalid encoding '%.100s'",
                STR(encoding_obj)->value);
            return nullptr;
        }

        cpid = Nst_encoding_to_single_byte(cpid);

        encoding = Nst_encoding(cpid);
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

    VirtualFile *vf = Nst_malloc_c(1, VirtualFile);
    if (vf == nullptr)
        return nullptr;

    if (!Nst_sbuffer_init(&vf->data, sizeof(i8), usize(buf_size))) {
        Nst_free(vf);
        return nullptr;
    }
    vf->ptr = 0;

    return Nst_iof_new_fake(
        (void *)vf,
        bin, true, true, true,
        Nst_encoding(Nst_EID_UTF8),
        vf_funcs);
}

Nst_Obj *NstC close_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;

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
    Nst_Obj *f;

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
    Nst_Obj *f;
    Nst_Obj *seq;

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

    usize seq_len = Nst_seq_len(seq);
    Nst_Obj **objs = _Nst_seq_objs(seq);
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
    Nst_Obj *f;
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
    Nst_Obj *f;
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

    Nst_Obj *bytes_array = Nst_array_new(buf_len);

    for (usize i = 0; i < buf_len; i++)
        Nst_seq_setnf(bytes_array, i, Nst_byte_new(buf[i]));

    Nst_free(buf);
    return OBJ(bytes_array);
}

Nst_Obj *NstC file_size_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;

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

Nst_Obj *NstC seek_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;
    Nst_Obj *start_obj;
    Nst_Obj *offset_obj;

    if (!Nst_extract_args(
            "F ?i ?i",
            arg_num, args,
            &f, &start_obj, &offset_obj))
    {
        return nullptr;
    }

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    if (!Nst_IOF_CAN_SEEK(f)) {
        Nst_set_value_error_c("the file cannot be seeked");
        return nullptr;
    }

    i64 start = Nst_DEF_VAL(start_obj, AS_INT(start_obj), 1);
    i64 offset = Nst_DEF_VAL(offset_obj, AS_INT(offset_obj), 0);

    if (start < 0 || start > 2) {
        Nst_set_value_errorf("invalid origin '%lli'", start);
        return nullptr;
    }

    isize size = -1;
    isize end_pos = 0;

    if (start == SEEK_END) {
        size = get_file_size(f);
        end_pos = size + isize(offset);
    } else if (start == SEEK_SET)
        end_pos = isize(offset);
    else {
        if (Nst_ftell(f, (usize *)&end_pos) != Nst_IO_ERROR)
            end_pos += isize(offset);
        else
            end_pos = 0;
    }

    if (offset != 0) {
        if (size == -1)
            size = get_file_size(f);
        if (end_pos < 0 || end_pos > (isize)size) {
            Nst_set_value_error_c(
                "the file-position indicator goes outside the file");
            return nullptr;
        }
    }

    if (Nst_fseek((Nst_SeekWhence)start, (isize)offset, f) == Nst_IO_ERROR) {
        Nst_set_call_error_c(
            "failed to change the position of the file-position indicator");
    }

    return Nst_int_new(end_pos);
}

Nst_Obj *NstC flush_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;

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
    Nst_Obj *f;

    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;

    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }

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
    Nst_Obj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    Nst_RETURN_BOOL(Nst_IOF_CAN_READ(f));
}

Nst_Obj *NstC can_write_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    Nst_RETURN_BOOL(Nst_IOF_CAN_WRITE(f));
}

Nst_Obj *NstC can_seek_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    Nst_RETURN_BOOL(Nst_IOF_CAN_SEEK(f));
}

Nst_Obj *NstC is_bin_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    Nst_RETURN_BOOL(Nst_IOF_IS_BIN(f));
}

Nst_Obj *NstC is_a_tty_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    Nst_RETURN_BOOL(Nst_IOF_IS_TTY(f));
}

Nst_Obj *NstC descriptor_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    return Nst_int_new(Nst_iof_fd(f));
}

Nst_Obj *NstC encoding_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;
    if (!Nst_extract_args("F", arg_num, args, &f))
        return nullptr;
    if (Nst_IOF_IS_CLOSED(f)) {
        SET_FILE_CLOSED_ERROR;
        return nullptr;
    }
    Nst_Encoding *encoding = Nst_iof_encoding(f);
    if (encoding == NULL) {
        if (Nst_IOF_IS_BIN(f))
            Nst_set_type_error_c("cannot get the encoding of a binary file");
        else
            Nst_set_value_error_c("failed to get the encoding of the file");
        return nullptr;
    }
    return Nst_str_new_c_raw(encoding->name, false);
}

Nst_Obj *NstC println_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *obj;
    bool flush;
    Nst_Obj *file_obj;
    if (!Nst_extract_args("o y ?F", arg_num, args, &obj, &flush, &file_obj))
        return nullptr;

    Nst_Obj *file = Nst_DEF_VAL(file_obj, file_obj, Nst_stdio()->out);

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
    Nst_Obj *f;

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
    Nst_stdio()->in = Nst_inc_ref(f);
    stdin_obj = Nst_inc_ref(f);
    Nst_RETURN_NULL;
}

Nst_Obj *NstC _set_stdout_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;

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
    Nst_stdio()->out = Nst_inc_ref(f);
    stdout_obj = Nst_inc_ref(f);
    Nst_RETURN_NULL;
}

Nst_Obj *NstC _set_stderr_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *f;

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
    Nst_stdio()->err = Nst_inc_ref(f);
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
