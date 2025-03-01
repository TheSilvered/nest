#include <string.h>
#include "file.h"
#include "global_consts.h"
#include "mem.h"
#include "str_builder.h"
#include "interpreter.h" // To use Nst_assert

#ifdef Nst_MSVC

#include <io.h>
#define lseek _lseek
#define fileno _fileno
#define isatty _isatty

Nst_StdIn Nst_stdin;

#else

#include <unistd.h>

#endif

typedef struct _Nst_IOFileObj {
    Nst_OBJ_HEAD;
    int fd;
    void *fp;
    Nst_Encoding *encoding;
    Nst_IOFuncSet func_set;
} Nst_IOFileObj;

#define IOFILE(ptr) ((Nst_IOFileObj *)(ptr))

static u32 io_result_ill_encoded_ch;
static usize io_result_position;
static const char *io_result_encoding_name;

Nst_Obj *Nst_iof_new(FILE *value, bool bin, bool read, bool write,
                     Nst_Encoding *encoding)
{
    Nst_IOFileObj *obj = Nst_obj_alloc(Nst_IOFileObj, Nst_t.IOFile);
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
        Nst_SET_FLAG(obj, Nst_FLAG_IOFILE_IS_BIN);
        obj->encoding = NULL;
    } else
        obj->encoding = encoding;

    if (read)
        Nst_SET_FLAG(obj, Nst_FLAG_IOFILE_CAN_READ);
    if (write)
        Nst_SET_FLAG(obj, Nst_FLAG_IOFILE_CAN_WRITE);
    if (lseek(obj->fd, 1, SEEK_SET) != -1) {
        Nst_SET_FLAG(obj, Nst_FLAG_IOFILE_CAN_SEEK);
        lseek(obj->fd, 0, SEEK_SET);
    }
    if (isatty(obj->fd))
        Nst_SET_FLAG(obj, Nst_FLAG_IOFILE_IS_TTY);

    return NstOBJ(obj);
}

Nst_Obj *Nst_iof_new_fake(void *value, bool bin, bool read, bool write,
                          bool seek, Nst_Encoding *encoding,
                          Nst_IOFuncSet func_set)
{
    Nst_IOFileObj *obj = Nst_obj_alloc(Nst_IOFileObj, Nst_t.IOFile);
    if (obj == NULL)
        return NULL;

    obj->fp = value;
    obj->fd = -1;
    obj->func_set = func_set;

    if (bin) {
        Nst_SET_FLAG(obj, Nst_FLAG_IOFILE_IS_BIN);
        obj->encoding = NULL;
    } else
        obj->encoding = encoding;

    if (read)
        Nst_SET_FLAG(obj, Nst_FLAG_IOFILE_CAN_READ);
    if (write)
        Nst_SET_FLAG(obj, Nst_FLAG_IOFILE_CAN_WRITE);
    if (seek)
        Nst_SET_FLAG(obj, Nst_FLAG_IOFILE_CAN_SEEK);

    return NstOBJ(obj);
}

Nst_IOFuncSet *Nst_iof_func_set(Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    return &IOFILE(f)->func_set;
}

int Nst_iof_fd(Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    return IOFILE(f)->fd;
}

void *Nst_iof_fp(Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    return IOFILE(f)->fp;
}

Nst_Encoding *Nst_iof_encoding(Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    return IOFILE(f)->encoding;
}

void _Nst_iofile_destroy(Nst_Obj *obj)
{
    Nst_assert(obj->type == Nst_t.IOFile);
    if (!Nst_IOF_IS_CLOSED(obj)) {
        IOFILE(obj)->func_set.flush(obj);
        IOFILE(obj)->func_set.close(obj);
    }
}

static Nst_IOResult FILE_read_get_ch(Nst_IOFileObj *f, Nst_StrBuilder *sb,
                                     bool expand_buf, usize *bytes_read)
{
    u8 ch_buf[Nst_ENCODING_MULTIBYTE_MAX_SIZE + 1] = { 0 };
    usize ch_len = 0;
    u32 ch;
    if (!expand_buf || !Nst_sb_reserve(sb, Nst_encoding_utf8.mult_max_sz + 1))
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
    Nst_io_result_set_details(
        (u32)(u8)ch_buf[0],
        *bytes_read,
        f->encoding->name);
    return Nst_IO_INVALID_DECODING;

success:
    if (!expand_buf && sb->cap - sb->len - 1 < ch_len) {
        fseek((FILE *)f->fp, (long)ch_len, SEEK_CUR);
        return Nst_IO_BUF_FULL;
    }
    *bytes_read += ch_len;

    ch = f->encoding->to_utf32(ch_buf);
    sb->len += Nst_ext_utf8_from_utf32(ch, (u8 *)(sb->value + sb->len));;
    return Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_read(u8 *buf, usize buf_size, usize count,
                           usize *buf_len, Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    if (!Nst_IOF_CAN_READ(f))
        return Nst_IO_OP_FAILED;

    usize bytes_read;
    if (Nst_IOF_IS_BIN(f)) {
        u8 *out_buf;
        if (buf_size == 0)
            out_buf = (u8 *)Nst_raw_malloc(count);
        else
            out_buf = buf;

        if (buf == NULL)
            return Nst_IO_ALLOC_FAILED;

        usize original_count = count;
        if (count > buf_size && buf_size != 0)
            count = buf_size;

        bytes_read = fread(out_buf, 1, count, (FILE *)IOFILE(f)->fp);
        if (buf_len != NULL)
            *buf_len = bytes_read;
        if (buf_size == 0)
            *(u8 **)buf = out_buf;

        if (bytes_read == count)
            return Nst_IO_EOF_REACHED;

        return original_count == count ? Nst_IO_SUCCESS : Nst_IO_BUF_FULL;
    }

    if (buf_size != 0 && !Nst_IOF_CAN_SEEK(f))
        return Nst_IO_OP_FAILED;

    // skip BOM
    if (Nst_IOF_CAN_SEEK(f)
        && ftell((FILE *)IOFILE(f)->fp) == 0
        && IOFILE(f)->encoding->bom != NULL)
    {
        u8 bom[Nst_ENCODING_BOM_MAX_SIZE];
        usize b_size = IOFILE(f)->encoding->bom_size;
        if (fread(&bom, 1, b_size, (FILE *)IOFILE(f)->fp) != b_size
            || memcmp(bom, IOFILE(f)->encoding->bom, b_size) != 0)
        {
            fseek((FILE *)IOFILE(f)->fp, 0, SEEK_SET);
        }
    }

    bool expand_buf = buf_size == 0;

    Nst_StrBuilder sb;
    if (expand_buf) {
        if (!Nst_sb_init(&sb, count + 1))
            return Nst_IO_ALLOC_FAILED;
    } else {
        sb.value = buf;
        sb.cap = buf_size;
        sb.len = 0;
    }

    bytes_read = 0;
    for (usize i = 0; i < count; i++) {
        Nst_IOResult result = FILE_read_get_ch(
            IOFILE(f),
            &sb, expand_buf,
            &bytes_read);
        if (result != Nst_IO_SUCCESS) {
            if (result < 0) {
                if (expand_buf) {
                    Nst_sb_destroy(&sb);
                    *(u8 **)buf = NULL;
                } else
                    memset(buf, 0, buf_size);

                if (buf_len != NULL)
                    *buf_len = 0;
            } else {
                if (expand_buf)
                    *(u8 **)buf = sb.value;

                if (buf_len != NULL)
                    *buf_len = sb.len;
            }
            return result;
        }
    }

    sb.value[sb.len] = '\0';

    if (expand_buf)
        *(u8 **)buf = sb.value;
    if (buf_len != NULL)
        *buf_len = sb.len;
    return Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_write(u8 *buf, usize buf_len, usize *count, Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);

    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    if (!Nst_IOF_CAN_WRITE(f))
        return Nst_IO_OP_FAILED;

    if (Nst_IOF_IS_BIN(f)) {
        usize written_bytes = fwrite(buf, 1, buf_len, IOFILE(f)->fp);
        if (count != NULL)
            *count = written_bytes;

        if (written_bytes != buf_len)
            return Nst_IO_ERROR;
        return Nst_IO_SUCCESS;
    }

    // add BOM if it can be added except for UTF-8 files
    if (Nst_IOF_CAN_SEEK(f)
        && ftell((FILE *)IOFILE(f)->fp) == 0
        && IOFILE(f)->encoding->bom != NULL
        && IOFILE(f)->encoding != &Nst_encoding_utf8
        && IOFILE(f)->encoding != &Nst_encoding_ext_utf8)
    {
        usize written_bytes = fwrite(
            IOFILE(f)->encoding->bom,
            1, IOFILE(f)->encoding->bom_size,
            (FILE *)IOFILE(f)->fp);

        if (written_bytes != IOFILE(f)->encoding->bom_size)
            return Nst_IO_ERROR;
    }

    usize chars_written = 0;
    u8 ch_buf[Nst_ENCODING_MULTIBYTE_MAX_SIZE];

    usize initial_len = buf_len;
    while (buf_len > 0) {
        i32 ch_len = Nst_check_ext_utf8_bytes((u8 *)buf, buf_len);
        u32 ch = Nst_ext_utf8_to_utf32((u8 *)buf);
        i32 ch_buf_len = IOFILE(f)->encoding->from_utf32(ch, ch_buf);
        if (ch_buf_len < 0) {
            if (count != NULL)
                *count = chars_written;
            Nst_io_result_set_details(
                ch,
                initial_len - buf_len,
                IOFILE(f)->encoding->name);
            return Nst_IO_INVALID_ENCODING;
        }
        usize written_char = fwrite(ch_buf, 1, ch_buf_len, IOFILE(f)->fp);
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

Nst_IOResult Nst_FILE_flush(Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;
    if (!Nst_IOF_CAN_WRITE(f))
        return Nst_IO_OP_FAILED;
    return fflush(IOFILE(f)->fp) == -1 ? Nst_IO_ERROR : Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_tell(Nst_Obj *f, usize *pos)
{
    Nst_assert(f->type == Nst_t.IOFile);
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;
    if (!Nst_IOF_CAN_SEEK(f))
        return Nst_IO_OP_FAILED;

#ifdef Nst_MSVC
    i64 fpi_pos = _ftelli64(IOFILE(f)->fp);
#else
    off_t fpi_pos = ftello(IOFILE(f)->fp);
#endif // !Nst_MSVC

    if (fpi_pos < 0) {
        *pos = 0;
        return Nst_IO_ERROR;
    }
    *pos = (usize)fpi_pos;
    return Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_seek(Nst_SeekWhence origin, isize offset,
                           Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;
    if (!Nst_IOF_CAN_SEEK(f))
        return Nst_IO_OP_FAILED;

#ifdef Nst_MSVC
    int result = _fseeki64(IOFILE(f)->fp, (i64)offset, origin);
#else
    int result = fseeko(IOFILE(f)->fp, (off_t)offset, origin);
#endif
    return result == -1 ? Nst_IO_ERROR : Nst_IO_SUCCESS;
}

Nst_IOResult Nst_FILE_close(Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    if (Nst_IOF_IS_CLOSED(f))
        return Nst_IO_CLOSED;

    return fclose(IOFILE(f)->fp) == -1 ? Nst_IO_ERROR : Nst_IO_SUCCESS;
}

Nst_IOResult Nst_fread(u8 *buf, usize buf_size, usize count, usize *buf_len,
                       Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    return IOFILE(f)->func_set.read(buf, buf_size, count, buf_len, f);
}

Nst_IOResult Nst_fwrite(u8 *buf, usize buf_len, usize *count, Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    return IOFILE(f)->func_set.write(buf, buf_len, count, f);
}

Nst_IOResult Nst_fflush(Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    return IOFILE(f)->func_set.flush(f);
}

Nst_IOResult Nst_ftell(Nst_Obj *f, usize *pos)
{
    Nst_assert(f->type == Nst_t.IOFile);
    return IOFILE(f)->func_set.tell(f, pos);
}

Nst_IOResult Nst_fseek(Nst_SeekWhence origin, isize offset, Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    return IOFILE(f)->func_set.seek(origin, offset, f);
}

Nst_IOResult Nst_fclose(Nst_Obj *f)
{
    Nst_assert(f->type == Nst_t.IOFile);
    Nst_IOResult result = IOFILE(f)->func_set.close(f);

    IOFILE(f)->encoding = NULL;
    IOFILE(f)->fd = -1;
    Nst_SET_FLAG(f, Nst_FLAG_IOFILE_IS_CLOSED);

    return result;
}

FILE *Nst_fopen_unicode(const char *path, const char *mode)
{
#ifdef Nst_MSVC
    wchar_t *wide_path = Nst_char_to_wchar_t(path, strlen(path));
    if (wide_path == NULL)
        return NULL;
    wchar_t *wide_mode = Nst_char_to_wchar_t(mode, strlen(mode));
    if (wide_mode == NULL) {
        Nst_free(wide_path);
        return NULL;
    }

    FILE *f = _wfopen((const wchar_t *)wide_path, (const wchar_t *)wide_mode);
    Nst_free(wide_path);
    Nst_free(wide_mode);
#else
    FILE *f = fopen(path, mode);
#endif
    return f;
}

void Nst_io_result_get_details(u32 *ill_encoded_ch, usize *position,
                               const char **encoding_name)
{
    if (ill_encoded_ch != NULL)
        *ill_encoded_ch = io_result_ill_encoded_ch;
    if (position != NULL)
        *position = io_result_position;
    if (encoding_name != NULL)
        *encoding_name = io_result_encoding_name;
}

void Nst_io_result_set_details(u32 ill_encoded_ch, usize position,
                               const char *encoding_name)
{
    io_result_ill_encoded_ch = ill_encoded_ch;
    io_result_position = position;
    io_result_encoding_name = encoding_name;
}
