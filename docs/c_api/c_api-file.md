# `file.h`

[`Nst_IOFileObj`](c_api-file.md#nst_iofileobj) interface.

## Authors

TheSilvered

---

## Macros

### `IOFILE`

**Synopsis:**

```better-c
#define IOFILE(ptr)
```

**Description:**

Casts ptr to a [`Nst_IOFileObj *`](c_api-file.md#nst_iofileobj).

---

### `Nst_IOF_IS_CLOSED`

**Synopsis:**

```better-c
#define Nst_IOF_IS_CLOSED(f)
```

**Description:**

Checks if `f` is closed.

---

### `Nst_IOF_IS_BIN`

**Synopsis:**

```better-c
#define Nst_IOF_IS_BIN(f)
```

**Description:**

Checks if `f` was opened in binary mode.

---

### `Nst_IOF_IS_TTY`

**Synopsis:**

```better-c
#define Nst_IOF_IS_TTY(f)
```

**Description:**

Checks if `f` is a TTY.

---

### `Nst_IOF_CAN_WRITE`

**Synopsis:**

```better-c
#define Nst_IOF_CAN_WRITE(f)
```

**Description:**

Checks if `f` can be written.

---

### `Nst_IOF_CAN_READ`

**Synopsis:**

```better-c
#define Nst_IOF_CAN_READ(f)
```

**Description:**

Checks if `f` can be read.

---

### `Nst_IOF_CAN_SEEK`

**Synopsis:**

```better-c
#define Nst_IOF_CAN_SEEK(f)
```

**Description:**

Checks if `f` can be seeked.

---

## Structs

### `Nst_IOFuncSet`

**Synopsis:**

```better-c
typedef struct _Nst_IOFuncSet {
    Nst_IOFile_read_f read;
    Nst_IOFile_write_f write;
    Nst_IOFile_flush_f flush;
    Nst_IOFile_tell_f tell;
    Nst_IOFile_seek_f seek;
    Nst_IOFile_close_f close;
} Nst_IOFuncSet
```

**Description:**

A structure representing the functions necessary to operate a Nest file object.

---

### `Nst_StdIn`

**Synopsis:**

```better-c
typedef struct _Nst_StdIn {
    HANDLE hd;
    wchar_t buf[1024];
    FILE *fp;
    i32 buf_size;
    i32 buf_ptr;
} Nst_StdIn
```

**Description:**

**WINDOWS ONLY** A structure representing the standard input file on Windows.

---

### `Nst_IOFileObj`

**Synopsis:**

```better-c
typedef struct _Nst_IOFileObj {
    Nst_OBJ_HEAD;
    void *fp;
    int fd;
    Nst_CP *encoding;
    Nst_IOFuncSet func_set;
} Nst_IOFileObj
```

**Description:**

A structure representing a Nest IO file object.

**Fields:**

- `fp`: the pointer to the file, it may not be a `FILE *`
- `fd`: the file descriptor, `-1` if not supported
- `encoding`: the encoding the file was opened in, `NULL` when opened in binary
  mode
- `func_set`: the functions used to operate the file

---

## Type aliases

### `Nst_IOFile_read_f`

**Synopsis:**

```better-c
typedef Nst_IOResult (*Nst_IOFile_read_f)(i8 *buf, usize buf_size, usize count,
                                          usize *buf_len, Nst_IOFileObj *f)
```

**Description:**

The type that represents a read function of a Nest file object.

This function shall read from the given file object count characters or bytes
when in binary mode. `buf` shall be interpreted as
[`i8 **`](c_api_index.md#type-definitions) instead of
[`i8 *`](c_api_index.md#type-definitions) and a new buffer shall be allocated
with [`Nst_malloc`](c_api-mem.md#nst_malloc) or similar functions. The buffer
shall contain UTF8-encoded text. When buf_len is not `NULL` the function shall
fill it with the number of characters written (or bytes if it is in binary
mode).

**Returns:**

This function shall return any [`Nst_IOResult`](c_api-file.md#nst_ioresult)
variant except for [`Nst_IO_INVALID_ENCODING`](c_api-file.md#nst_ioresult) as
follows:

- [`Nst_IO_BUF_FULL`](c_api-file.md#nst_ioresult) when `buf` is not allocated
  and cannot store all requested characters or bytes.
- [`Nst_IO_EOF_REACHED`](c_api-file.md#nst_ioresult) when the end of the file
  has been reached.
- [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) when everything works
  correctly.
- [`Nst_IO_ALLOC_FAILED`](c_api-file.md#nst_ioresult) when the buffer fails to
  be allocated.
- [`Nst_IO_INVALID_DECODING`](c_api-file.md#nst_ioresult) when the text read
  cannot be decoded. This variant cannot be returned if the file is in binary
  mode. When it is returned
  [`Nst_io_result_set_details`](c_api-file.md#nst_io_result_set_details) must be
  called.
- [`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult) if the file does not support
  reading.
- [`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult) if the file is closed.
- [`Nst_IO_ERROR`](c_api-file.md#nst_ioresult) for any other error that might
  occur.

---

### `Nst_IOFile_write_f`

**Synopsis:**

```better-c
typedef Nst_IOResult (*Nst_IOFile_write_f)(i8 *buf, usize buf_len,
                                           usize *count, Nst_IOFileObj *f)
```

**Description:**

The type that represents a write function of a Nest file object.

This function shall write the contents of buf to a file. If count is not `NULL`
it is filled with the number of characters written (or the number of bytes if
the file is in binary mode). `buf` shall contain UTF-8 text that allows invalid
characters under U+10FFFF.

**Returns:**

This function shall not return [`Nst_IO_BUF_FULL`](c_api-file.md#nst_ioresult),
[`Nst_IO_EOF_REACHED`](c_api-file.md#nst_ioresult) and
[`Nst_IO_INVALID_DECODING`](c_api-file.md#nst_ioresult) variants of
[`Nst_IOResult`](c_api-file.md#nst_ioresult). The other ones shall be returned
as follows:

- [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) when the function successfully
  writes the characters to the file.
- [`Nst_IO_ALLOC_FAILED`](c_api-file.md#nst_ioresult) if a memory allocation
  fails.
- [`Nst_IO_INVALID_ENCODING`](c_api-file.md#nst_ioresult) if a character cannot
  be encoded in the encoding the file is opened in. This variant can only be
  returned when the file is not binary. When it is returned
  [`Nst_io_result_set_details`](c_api-file.md#nst_io_result_set_details) must be
  called.
- [`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult) if the file does not support
  writing.
- [`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult) if the file is closed.
- [`Nst_IO_ERROR`](c_api-file.md#nst_ioresult) for any other error that might
  occur.

---

### `Nst_IOFile_flush_f`

**Synopsis:**

```better-c
typedef Nst_IOResult (*Nst_IOFile_flush_f)(Nst_IOFileObj *f)
```

**Description:**

The type that represents a flush function of a Nest file object.

This function shall write any buffered bytes to the file.

**Returns:**

This function shall return only either
[`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult),
[`Nst_IO_ERROR`](c_api-file.md#nst_ioresult),
[`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult),
[`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) or
[`Nst_IO_ALLOC_FAILED`](c_api-file.md#nst_ioresult) as follows:

- [`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult) when the file is closed.
- [`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult) if the file does not support
  writing.
- [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) if the function exits
  successfully.
- [`Nst_IO_ALLOC_FAILED`](c_api-file.md#nst_ioresult) if a memory allocation
  fails.
- [`Nst_IO_ERROR`](c_api-file.md#nst_ioresult) for any other error.

---

### `Nst_IOFile_tell_f`

**Synopsis:**

```better-c
typedef Nst_IOResult (*Nst_IOFile_tell_f)(Nst_IOFileObj *f, usize *pos)
```

**Description:**

The type that represents a tell function of a Nest file object.

This function shall fill pos with the current position in bytes from the start
of the file of the file-position indicator.

**Returns:**

This function shall return only either
[`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult),
[`Nst_IO_ERROR`](c_api-file.md#nst_ioresult),
[`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult) or
[`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) as follows:

- [`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult) when the file is closed.
- [`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult) if the file does not support
  seeking.
- [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) if the function exits
  successfully.
- [`Nst_IO_ERROR`](c_api-file.md#nst_ioresult) for any other error.

---

### `Nst_IOFile_seek_f`

**Synopsis:**

```better-c
typedef Nst_IOResult (*Nst_IOFile_seek_f)(Nst_SeekWhence origin, isize offset,
                                          Nst_IOFileObj *f)
```

**Description:**

The type that represents a seek function of a Nest file object.

This function shall move the file-position indicator by an offset starting from
origin. [`Nst_SEEK_SET`](c_api-file.md#nst_seekwhence) is the start of the file,
[`Nst_SEEK_CUR`](c_api-file.md#nst_seekwhence) is the current position of the
file-position indicator and [`Nst_SEEK_END`](c_api-file.md#nst_seekwhence) is
the end of the file.

**Returns:**

This function shall return only either
[`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult),
[`Nst_IO_ERROR`](c_api-file.md#nst_ioresult),
[`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult) or
[`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) as follows:

- [`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult) when the file is closed.
- [`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult) if the file does not support
  seeking.
- [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) if the function exits
  successfully.
- [`Nst_IO_ERROR`](c_api-file.md#nst_ioresult) for any other error.

---

### `Nst_IOFile_close_f`

**Synopsis:**

```better-c
typedef Nst_IOResult (*Nst_IOFile_close_f)(Nst_IOFileObj *f)
```

**Description:**

The type that represents a close function of a Nest file object.

This function shall close the given file and free any allocated memory.

**Returns:**

This function shall return only either
[`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult),
[`Nst_IO_ERROR`](c_api-file.md#nst_ioresult) or
[`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) as follows:

- [`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult) when the file was already
  closed.
- [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) if the function exits
  successfully.
- [`Nst_IO_ERROR`](c_api-file.md#nst_ioresult) for any other error.

---

## Functions

### `Nst_iof_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_iof_new(FILE *value, bool bin, bool read, bool write,
                     Nst_CP *encoding)
```

**Description:**

Creates a new [`Nst_IOFileObj`](c_api-file.md#nst_iofileobj) from a C file
pointer.

**Parameters:**

- `value`: the value of the new object
- `bin`: if the file is in binary mode
- `read`: whether the file can be read
- `write`: whether the file can be written
- `encoding`: the encoding of the opened file, ignored when bin is true

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_iof_new_fake`

**Synopsis:**

```better-c
Nst_Obj *Nst_iof_new_fake(void *value, bool bin, bool read, bool write,
                          bool seek, Nst_CP *encoding, Nst_IOFuncSet func_set)
```

**Description:**

Creates a new [`Nst_IOFileObj`](c_api-file.md#nst_iofileobj) that is not a C
file pointer.

**Parameters:**

- `value`: the value of the new object
- `bin`: if the file is in binary mode
- `read`: whether the file can be read
- `write`: whether the file can be written
- `seek`: whether the file can be sought
- `encoding`: the encoding of the opened file, ignored when `bin` is `true`
- `func_set`: custom functions used to read the file

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `_Nst_iofile_destroy`

**Synopsis:**

```better-c
void _Nst_iofile_destroy(Nst_IOFileObj *obj)
```

**Description:**

Destructor of a [`Nst_IOFileObj`](c_api-file.md#nst_iofileobj).

---

### `Nst_fread`

**Synopsis:**

```better-c
Nst_IOResult Nst_fread(i8 *buf, usize buf_size, usize count, usize *buf_len,
                       Nst_IOFileObj *f)
```

**Description:**

Calls the read function of the file, see
[`Nst_IOFile_read_f`](c_api-file.md#nst_iofile_read_f).

---

### `Nst_fwrite`

**Synopsis:**

```better-c
Nst_IOResult Nst_fwrite(i8 *buf, usize buf_len, usize *count, Nst_IOFileObj *f)
```

**Description:**

Calls the write function of the file, see
[`Nst_IOFile_write_f`](c_api-file.md#nst_iofile_write_f).

---

### `Nst_fflush`

**Synopsis:**

```better-c
Nst_IOResult Nst_fflush(Nst_IOFileObj *f)
```

**Description:**

Calls the flush function of the file, see
[`Nst_IOFile_flush_f`](c_api-file.md#nst_iofile_flush_f).

---

### `Nst_ftell`

**Synopsis:**

```better-c
Nst_IOResult Nst_ftell(Nst_IOFileObj *f, usize *pos)
```

**Description:**

Calls the tell function of the file, see
[`Nst_IOFile_tell_f`](c_api-file.md#nst_iofile_tell_f).

---

### `Nst_fseek`

**Synopsis:**

```better-c
Nst_IOResult Nst_fseek(Nst_SeekWhence origin, isize offset, Nst_IOFileObj *f)
```

**Description:**

Calls the seek function of the file, see
[`Nst_IOFile_seek_f`](c_api-file.md#nst_iofile_seek_f).

---

### `Nst_fclose`

**Synopsis:**

```better-c
Nst_IOResult Nst_fclose(Nst_IOFileObj *f)
```

**Description:**

Calls the close function of the file, see
[`Nst_IOFile_close_f`](c_api-file.md#nst_iofile_close_f).

---

### `Nst_FILE_read`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_read(i8 *buf, usize buf_size, usize count,
                           usize *buf_len, Nst_IOFileObj *f)
```

**Description:**

Read function for standard C file descriptors.

---

### `Nst_FILE_write`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_write(i8 *buf, usize buf_len, usize *count,
                            Nst_IOFileObj *f)
```

**Description:**

Write function for standard C file descriptors.

---

### `Nst_FILE_flush`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_flush(Nst_IOFileObj *f)
```

**Description:**

Flush function for standard C file descriptors.

---

### `Nst_FILE_tell`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_tell(Nst_IOFileObj *f, usize *pos)
```

**Description:**

Tell function for standard C file descriptors.

---

### `Nst_FILE_close`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_close(Nst_IOFileObj *f)
```

**Description:**

Close function for standard C file descriptors.

---

### `Nst_io_result_get_details`

**Synopsis:**

```better-c
void Nst_io_result_get_details(u32 *ill_encoded_ch, usize *position,
                               const i8 **encoding_name)
```

**Description:**

Gets the details of the [`Nst_IOResult`](c_api-file.md#nst_ioresult) returned by
the functions.

This function can only be called when the returned
[`Nst_IOResult`](c_api-file.md#nst_ioresult) is either
[`Nst_IO_INVALID_ENCODING`](c_api-file.md#nst_ioresult) or
[`Nst_IO_INVALID_DECODING`](c_api-file.md#nst_ioresult). If the result is the
former `ill_encoded_ch` will be the code point that could not be encoded,
otherwise if the result is the latter `ill_encoded_ch` will represent the byte
that could not be decoded. Similarly `encoding_name` is the encoding that failed
to encode the code point for
[`Nst_IO_INVALID_ENCODING`](c_api-file.md#nst_ioresult) and the name of the one
that failed to decode the byte for
[`Nst_IO_INVALID_DECODING`](c_api-file.md#nst_ioresult).

**Parameters:**

- `ill_encoded_ch`: variable filled with the character that failed to encode or
  decode, may be `NULL`
- `position`: variable filled with the position in the file of the encoding
  error, may be `NULL`
- `encoding_name`: variable filled with the name of the encoding, may be `NULL`

---

### `Nst_io_result_set_details`

**Synopsis:**

```better-c
void Nst_io_result_set_details(u32 ill_encoded_ch, usize position,
                               const i8 *encoding_name)
```

**Description:**

Sets the values returned with
[`Nst_io_result_get_details`](c_api-file.md#nst_io_result_get_details).

---

### `Nst_fopen_unicode`

**Synopsis:**

```better-c
FILE *Nst_fopen_unicode(i8 *path, const i8 *mode)
```

**Description:**

Opens a file given a path that can contain unicode characters in UTF-8.

**Parameters:**

- `path`: the path to the file
- `mode`: the mode to open the file with

**Returns:**

The file pointer on success and `NULL` on failure. The error is set only if a
`Memory Error` occurs.

---

## Enums

### `Nst_IOResult`

**Synopsis:**

```better-c
typedef enum _Nst_IOResult {
    Nst_IO_BUF_FULL = 2,
    Nst_IO_EOF_REACHED = 1,
    Nst_IO_SUCCESS = 0,
    Nst_IO_ALLOC_FAILED = -1,
    Nst_IO_INVALID_ENCODING = -2,
    Nst_IO_INVALID_DECODING = -3,
    Nst_IO_OP_FAILED = -4,
    Nst_IO_CLOSED = -5,
    Nst_IO_ERROR = -6
} Nst_IOResult
```

**Description:**

Enumeration of the possible IO return values.

---

### `Nst_SeekWhence`

**Synopsis:**

```better-c
typedef enum _Nst_SeekWhence {
    Nst_SEEK_SET = 0,
    Nst_SEEK_CUR = 1,
    Nst_SEEK_END = 2
} Nst_SeekWhence
```

**Description:**

Enumeration of the possible origins for seek file functions.

---

### `Nst_IOFileFlag`

**Synopsis:**

```better-c
typedef enum _Nst_IOFileFlag {
    Nst_FLAG_IOFILE_IS_CLOSED = Nst_FLAG(1),
    Nst_FLAG_IOFILE_IS_BIN    = Nst_FLAG(2),
    Nst_FLAG_IOFILE_CAN_WRITE = Nst_FLAG(3),
    Nst_FLAG_IOFILE_CAN_READ  = Nst_FLAG(4),
    Nst_FLAG_IOFILE_CAN_SEEK  = Nst_FLAG(5),
    Nst_FLAG_IOFILE_IS_TTY    = Nst_FLAG(6)
} Nst_IOFileFlag
```

**Description:**

The flags of a IO file.
