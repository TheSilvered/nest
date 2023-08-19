#include <string.h>
#include "file.h"
#include "global_consts.h"
#include "mem.h"

#ifdef Nst_WIN

#include <io.h>
#define lseek _lseek
#define fileno _fileno
#define isatty _isatty

Nst_StdIn Nst_stdin;

#else

#include <unistd.h>

#endif

static u32 io_result_ill_encoded_ch;
static usize io_result_position;
static const i8 *io_result_encoding_name;

Nst_Obj *Nst_iof_new(FILE *value, bool bin, bool read, bool write,
                     Nst_CP *encoding)
{
    Nst_IOFileObj *obj = Nst_obj_alloc(
        Nst_IOFileObj,
        Nst_t.IOFile,
        _Nst_iofile_destroy);
    if (obj == NULL)
        return NULL;

    obj->fp = (void *)value;
    obj->fd = fileno(value);
    obj->func_set.read  = Nst_FILE_read;
    obj->func_set.write = Nst_FILE_write;
    obj->func_set.flush = Nst_FILE_flush;
    obj->func_set.tell  = Nst_FILE_tell;
    obj->func_set.seek  = Nst_FILE_seek;
    obj->func_set.close = Nst_FILE_close;

    if (bin) {
        Nst_FLAG_SET(obj, Nst_FLAG_IOFILE_IS_BIN);
        obj->encoding = NULL;
    } else
        obj->encoding = encoding;

    if (read)
        Nst_FLAG_SET(obj, Nst_FLAG_IOFILE_CAN_READ);
    if (write)
        Nst_FLAG_SET(obj, Nst_FLAG_IOFILE_CAN_WRITE);
    if (lseek(obj->fd, 1, SEEK_SET) != -1) {
        Nst_FLAG_SET(obj, Nst_FLAG_IOFILE_CAN_SEEK);
        lseek(obj->fd, 0, SEEK_SET);
    }
    if (isatty(obj->fd))
        Nst_FLAG_SET(obj, Nst_FLAG_IOFILE_IS_TTY);

    return OBJ(obj);
}

Nst_Obj *Nst_iof_new_fake(void *value, bool bin, bool read, bool write,
                          bool seek, Nst_CP *encoding, Nst_IOFuncSet func_set)
{
    Nst_IOFileObj *obj = Nst_obj_alloc(
        Nst_IOFileObj,
        Nst_t.IOFile,
        _Nst_iofile_destroy);
    if (obj == NULL)
        return NULL;

    obj->fp = value;
    obj->fd = -1;
    obj->func_set = func_set;

    if (bin) {
        Nst_FLAG_SET(obj, Nst_FLAG_IOFILE_IS_BIN);
        obj->encoding = NULL;
    } else
        obj->encoding = encoding;

    if (read)
        Nst_FLAG_SET(obj, Nst_FLAG_IOFILE_CAN_READ);
    if (write)
        Nst_FLAG_SET(obj, Nst_FLAG_IOFILE_CAN_WRITE);
    if (seek)
        Nst_FLAG_SET(obj, Nst_FLAG_IOFILE_CAN_SEEK);

    return OBJ(obj);
}

void _Nst_iofile_destroy(Nst_IOFileObj *obj)
{
    if (!Nst_IOF_IS_CLOSED(obj)) {
        obj->func_set.flush(obj);
        obj->func_set.close(obj);
    }
}

static Nst_IOResult FILE_read_get_ch(Nst_IOFileObj *f, Nst_Buffer *buf,
                                     bool expand_buf, usize *bytes_read)
{
    i8 ch_buf[Nst_CP_MULTIBYTE_MAX_SIZE + 1] = { 0 };
    usize ch_len = 0;
    u32 ch;
    if (!expand_buf || !Nst_buffer_expand_by(buf, Nst_cp_utf8.mult_max_sz + 1))
        return Nst_IO_ALLOC_FAILED;

    for (usize i = 0; i < f->encoding->mult_max_sz; i++) {
        if (fread(ch_buf + i, 1, 1, f->fp) == 0) {
            if (i == 0 && feof(f->fp))
                return Nst_IO_EOF_REACHED;
            else if (feof(f->fp)) {
                *bytes_read += i;
                break;
            } else
                return Nst_IO_ERROR;
        }
        if (f->encoding->check_bytes(ch_buf, i + 1) > 0) {
            ch_len = i + 1;
            goto success;
        }
    }
    Nst_io_result_set_details((u32)ch_buf[0], *bytes_read, f->encoding->name);
    return Nst_IO_INVALID_DECODING;

success:
    if (!expand_buf && buf->cap - buf->len - 1 < ch_len) {
        fseek((FILE *)f->fp, (long)ch_len, SEEK_CUR);
        return Nst_IO_BUF_FULL;
    }
    *bytes_read += ch_len;

    ch = f->encoding->to_utf32(ch_buf);
    Nst_ext_utf8_from_utf32(ch, (u8 *)(buf->data + buf->len));
    buf->len += ch_len;
    return Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_read(i8 *buf, usize buf_size, usize count,
                           usize *buf_len, Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    if (!Nst_IOF_CAN_READ(f))
        return Nst_IO_OP_FAILED;

    usize bytes_read;
    if (Nst_IOF_IS_BIN(f)) {
        i8 *out_buf;
        if (buf_size == 0)
            out_buf = (i8 *)Nst_raw_malloc(count);
        else
            out_buf = buf;

        if (buf == NULL)
            return Nst_IO_ALLOC_FAILED;

        usize original_count = count;
        if (count > buf_size && buf_size != 0)
            count = buf_size;

        bytes_read = fread(out_buf, 1, count, (FILE *)f->fp);
        if (buf_len != NULL)
            *buf_len = bytes_read;
        if (buf_size == 0)
            *(i8 **)buf = out_buf;

        if (bytes_read == count)
            return Nst_IO_EOF_REACHED;

        return original_count == count ? Nst_IO_SUCCESS : Nst_IO_BUF_FULL;
    }

    if (buf_size != 0 && !Nst_IOF_CAN_SEEK(f))
        return Nst_IO_OP_FAILED;

    // skip BOM
    if (Nst_IOF_CAN_SEEK(f)
        && ftell((FILE *)f->fp) == 0
        && f->encoding->bom != NULL)
    {
        i8 bom[Nst_CP_BOM_MAX_SIZE];
        usize b_size = f->encoding->bom_size;
        if (fread(&bom, 1, b_size, (FILE *)f->fp) != b_size
            || memcmp(bom, f->encoding->bom, b_size) != 0)
        {
            fseek((FILE *)f->fp, 0, SEEK_SET);
        }
    }

    bool expand_buf = buf_size == 0;
    Nst_Buffer buffer;
    if (expand_buf) {
        if (!Nst_buffer_init(&buffer, count + 1))
            return Nst_IO_ALLOC_FAILED;
    } else {
        buffer.data = buf;
        buffer.cap = buf_size;
        buffer.len = 0;
    }

    bytes_read = 0;
    for (usize i = 0; i < count; i++) {
        Nst_IOResult result = FILE_read_get_ch(
            f,
            &buffer, expand_buf,
            &bytes_read);
        if (result != Nst_IO_SUCCESS) {
            if (result < 0) {
                if (expand_buf) {
                    Nst_buffer_destroy(&buffer);
                    *(i8 **)buf = NULL;
                } else
                    memset(buf, 0, buf_size);

                if (buf_len != NULL)
                    *buf_len = 0;
            } else {
                if (expand_buf)
                    *(i8 **)buf = buffer.data;

                if (buf_len != NULL)
                    *buf_len = buffer.len;
            }
            return result;
        }
    }

    buffer.data[buffer.len] = '\0';

    if (expand_buf)
        *(i8 **)buf = buffer.data;
    if (buf_len != NULL)
        *buf_len = buffer.len;
    return Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_write(i8 *buf, usize buf_len, usize *count,
                            Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    if (!Nst_IOF_CAN_WRITE(f))
        return Nst_IO_OP_FAILED;

    if (Nst_IOF_IS_BIN(f)) {
        usize written_bytes = fwrite(buf, 1, buf_len, f->fp);
        if (count != NULL)
            *count = written_bytes;

        if (written_bytes != buf_len)
            return Nst_IO_ERROR;
        return Nst_IO_SUCCESS;
    }

    // add BOM if it can be added except for UTF-8 files
    if (Nst_IOF_CAN_SEEK(f)
        && ftell((FILE *)f->fp) == 0
        && f->encoding->bom != NULL
        && f->encoding != &Nst_cp_utf8
        && f->encoding != &Nst_cp_ext_utf8)
    {
        usize written_bytes = fwrite(
            f->encoding->bom,
            1, f->encoding->bom_size,
            (FILE *)f->fp);

        if (written_bytes != f->encoding->bom_size)
            return Nst_IO_ERROR;
    }

    usize chars_written = 0;
    i8 ch_buf[Nst_CP_MULTIBYTE_MAX_SIZE];

    usize initial_len = buf_len;
    while (buf_len > 0) {
        i32 ch_len = Nst_check_ext_utf8_bytes((u8 *)buf, buf_len);
        u32 ch = Nst_ext_utf8_to_utf32((u8 *)buf);
        i32 ch_buf_len = f->encoding->from_utf32(ch, ch_buf);
        if (ch_buf_len < 0) {
            if (count != NULL)
                *count = chars_written;
            Nst_io_result_set_details(
                ch,
                initial_len - buf_len,
                f->encoding->name);
            return Nst_IO_INVALID_ENCODING;
        }
        usize written_char = fwrite(ch_buf, 1, ch_buf_len, f->fp);
        if (written_char != (usize)ch_buf_len) {
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

Nst_IOResult Nst_FILE_flush(Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;
    if (!Nst_IOF_CAN_WRITE(f))
        return Nst_IO_OP_FAILED;
    return fflush(f->fp) == -1 ? Nst_IO_ERROR : Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_tell(Nst_IOFileObj *f, usize *pos)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;
    if (!Nst_IOF_CAN_SEEK(f))
        return Nst_IO_OP_FAILED;

#ifdef Nst_WIN
    i64 fpi_pos = _ftelli64(f->fp);
#else
    off_t fpi_pos = ftello(f->fp);
#endif // !Nst_WIN

    if (fpi_pos == -1) {
        *pos = 0;
        return Nst_IO_ERROR;
    }
    *pos = (usize)fpi_pos;
    return Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_seek(Nst_SeekWhence origin, isize offset,
                           Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;
    if (!Nst_IOF_CAN_SEEK(f))
        return Nst_IO_OP_FAILED;

#ifdef Nst_WIN
    int result = _fseeki64(f->fp, (i64)offset, origin);
#else
    int result = fseeko(f->fp, (off_t)offset, origin);
#endif
    return result == -1 ? Nst_IO_ERROR : Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_close(Nst_IOFileObj *f)
{
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    return fclose(f->fp) == -1 ? Nst_IO_ERROR : Nst_IO_SUCCESS;
}

Nst_IOResult Nst_fread(i8 *buf, usize buf_size, usize count, usize *buf_len,
                       Nst_IOFileObj *f)
{
    return f->func_set.read(buf, buf_size, count, buf_len, f);
}

Nst_IOResult Nst_fwrite(i8 *buf, usize buf_len, usize *count, Nst_IOFileObj *f)
{
    return f->func_set.write(buf, buf_len, count, f);
}

Nst_IOResult Nst_fflush(Nst_IOFileObj *f)
{
    return f->func_set.flush(f);
}

Nst_IOResult Nst_ftell(Nst_IOFileObj *f, usize *pos)
{
    return f->func_set.tell(f, pos);
}

Nst_IOResult Nst_fseek(Nst_SeekWhence origin, isize offset, Nst_IOFileObj *f)
{
    return f->func_set.seek(origin, offset, f);
}

Nst_IOResult Nst_fclose(Nst_IOFileObj *f)
{
    Nst_IOResult result = f->func_set.close(f);

    f->encoding = NULL;
    f->fd = -1;
    Nst_FLAG_SET(f, Nst_FLAG_IOFILE_IS_CLOSED);

    return result;
}

void Nst_io_result_get_details(u32 *ill_encoded_ch, usize *position,
                               const i8 **encoding_name)
{
    if (ill_encoded_ch != NULL)
        *ill_encoded_ch = io_result_ill_encoded_ch;
    if (position != NULL)
        *position = io_result_position;
    if (encoding_name != NULL)
        *encoding_name = io_result_encoding_name;
}

void Nst_io_result_set_details(u32 ill_encoded_ch, usize position,
                               const i8 *encoding_name)
{
    io_result_ill_encoded_ch = ill_encoded_ch;
    io_result_position = position;
    io_result_encoding_name = encoding_name;
}
