# `file.h`

`IOFile` object interface.

## Authors

TheSilvered

---

## Macros

### `Nst_IOF_IS_CLOSED`

**Synopsis:**

```better-c
#define Nst_IOF_IS_CLOSED(f)
```

**Description:**

Check if `f` is closed.

---

### `Nst_IOF_IS_BIN`

**Synopsis:**

```better-c
#define Nst_IOF_IS_BIN(f)
```

**Description:**

Check if `f` was opened in binary mode.

---

### `Nst_IOF_IS_TTY`

**Synopsis:**

```better-c
#define Nst_IOF_IS_TTY(f)
```

**Description:**

Check if `f` is a TTY.

---

### `Nst_IOF_CAN_WRITE`

**Synopsis:**

```better-c
#define Nst_IOF_CAN_WRITE(f)
```

**Description:**

Check if `f` can be written.

---

### `Nst_IOF_CAN_READ`

**Synopsis:**

```better-c
#define Nst_IOF_CAN_READ(f)
```

**Description:**

Check if `f` can be read.

---

### `Nst_IOF_CAN_SEEK`

**Synopsis:**

```better-c
#define Nst_IOF_CAN_SEEK(f)
```

**Description:**

Check if `f` can be seeked.

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
    wchar_t buf[_Nst_WIN_STDIN_BUF_SIZE];
    FILE *fp;
    i32 buf_size;
    i32 buf_ptr;
} Nst_StdIn
```

**Description:**

**WINDOWS ONLY** A structure representing the standard input file on Windows.

---

## Type aliases

### `Nst_IOFile_read_f`

**Synopsis:**

```better-c
typedef Nst_IOResult (*Nst_IOFile_read_f)(u8 *buf, usize buf_size, usize count,
                                          usize *buf_len, Nst_Obj *f)
```

**Description:**

The type that represents a read function of a Nest file object.

This function shall read from the given file object `count` characters (or
`count` bytes when in binary mode) starting from the file position indicator.

**Parameters:**

- `buf`: the buffer where the read text is written. If `buf_size` is `0` this
  parameter should be interpreted as [`u8 **`](c_api_index.md#type-definitions)
  and a malloc'd buffer of the right size shall be put in it. When the file is
  opened in normal mode the contents of the buffer must be in `extUTF8` encoding
  and must terminate with a NUL character.
- `buf_size`: the size of `buf` in bytes, if set to `0` the buffer will be
  allocated instead
- `count`: the number of characters to read when opened in normal mode or the
  number of bytes to read when opened in binary mode, a valid value can be
  expected only when the function returns
  [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) or
  [`Nst_IO_EOF_REACHED`](c_api-file.md#nst_ioresult)
- `buf_len`: this is an out parameter set to the length in bytes of the data
  written in `buf` ignoring the NUL character, it may be `NULL` to not recieve
  the information read when the file is opened in normal mode and to the number
  of bytes read when opened in binary mode
- `f`: the file to read

**Returns:**

This function shall return one of the following
[`Nst_IOResult`](c_api-file.md#nst_ioresult) variants:

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
  called to communicate the appropriate information.
- [`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult) if the file does not support
  reading.
- [`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult) if the file is closed.
- [`Nst_IO_ERROR`](c_api-file.md#nst_ioresult) for any other error that might
  occur.

---

### `Nst_IOFile_write_f`

**Synopsis:**

```better-c
typedef Nst_IOResult (*Nst_IOFile_write_f)(u8 *buf, usize buf_len,
                                           usize *count, Nst_Obj *f)
```

**Description:**

The type that represents a write function of a Nest file object.

This function shall write the contents of buf to a file starting from the file
position indicator and overwriting any previous content. If count is not `NULL`
it is filled with the number of characters written (or the number of bytes if
the file is in binary mode). `buf` shall contain UTF-8 text that allows invalid
characters under U+10FFFF.

**Parameters:**

- `buf`: the content to write to the file
- `buf_len`: the length in bytes of `buf`
- `count`: an out parameter set to the number of characters written when the
  file is opened in normal mode or to the number of bytes written when it is in
  binary mode, it may be `NULL` to not recieve the information,a valid value can
  be expected only when the function returns
  [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult)
- `f`: the file to write to

**Returns:**

This function shall return one of the following
[`Nst_IOResult`](c_api-file.md#nst_ioresult) variants:

- [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult) when the function successfully
  writes the characters to the file.
- [`Nst_IO_ALLOC_FAILED`](c_api-file.md#nst_ioresult) if a memory allocation
  fails.
- [`Nst_IO_INVALID_ENCODING`](c_api-file.md#nst_ioresult) if a character cannot
  be encoded in the encoding the file is opened in. This variant can only be
  returned when the file is not binary. When it is returned
  [`Nst_io_result_set_details`](c_api-file.md#nst_io_result_set_details) must be
  called to communicate the appropriate information.
- [`Nst_IO_OP_FAILED`](c_api-file.md#nst_ioresult) if the file does not support
  writing.
- [`Nst_IO_CLOSED`](c_api-file.md#nst_ioresult) if the file is closed.
- [`Nst_IO_ERROR`](c_api-file.md#nst_ioresult) for any other error that might
  occur.

---

### `Nst_IOFile_flush_f`

**Synopsis:**

```better-c
typedef Nst_IOResult (*Nst_IOFile_flush_f)(Nst_Obj *f)
```

**Description:**

The type that represents a flush function of a Nest file object.

This function shall write any buffered bytes to the file.

**Parameters:**

- `f`: the file to flush

**Returns:**

This function shall return one of the following
[`Nst_IOResult`](c_api-file.md#nst_ioresult) variants:

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
typedef Nst_IOResult (*Nst_IOFile_tell_f)(Nst_Obj *f, usize *pos)
```

**Description:**

The type that represents a tell function of a Nest file object.

This function shall get the current position in bytes from the start of the file
of the file-position indicator.

**Parameters:**

- `f`: the file to get the position from
- `pos`: the pointer filled with the retrived position, a valid value can be
  expected only when the function returns
  [`Nst_IO_SUCCESS`](c_api-file.md#nst_ioresult)

**Returns:**

This function shall return one of the following
[`Nst_IOResult`](c_api-file.md#nst_ioresult) variants:

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
                                          Nst_Obj *f)
```

**Description:**

The type that represents a seek function of a Nest file object.

This function shall move the file-position indicator.
[`Nst_SEEK_SET`](c_api-file.md#nst_seekwhence) is the start of the file,
[`Nst_SEEK_CUR`](c_api-file.md#nst_seekwhence) is the current position of the
file-position indicator and [`Nst_SEEK_END`](c_api-file.md#nst_seekwhence) is
the end of the file.

**Parameters:**

- `origin`: where to calculate the offset from,
  [`Nst_SEEK_SET`](c_api-file.md#nst_seekwhence) is the start of the file,
  [`Nst_SEEK_CUR`](c_api-file.md#nst_seekwhence) is the current position of the
  indicator and [`Nst_SEEK_END`](c_api-file.md#nst_seekwhence) is the end of the
  file
- `offset`: an offset in bytes from `origin` to move the indicator
- `f`: the file to move the indicator of

**Returns:**

This function shall return one of the following
[`Nst_IOResult`](c_api-file.md#nst_ioresult) variants:

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
typedef Nst_IOResult (*Nst_IOFile_close_f)(Nst_Obj *f)
```

**Description:**

The type that represents a close function of a Nest file object.

This function shall close the given file and free any allocated memory.

**Parameters:**

- `f`: the file to close

**Returns:**

This function shall return one of the following
[`Nst_IOResult`](c_api-file.md#nst_ioresult) variants:

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
Nst_ObjRef *Nst_iof_new(FILE *value, bool bin, bool read, bool write,
                        Nst_Encoding *encoding)
```

**Description:**

Create a new `IOFile` object from a C file pointer.

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
Nst_ObjRef *Nst_iof_new_fake(void *value, bool bin, bool read, bool write,
                             bool seek, Nst_Encoding *encoding,
                             Nst_IOFuncSet func_set)
```

**Description:**

Create a new `IOFile` object that is not a C file pointer.

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

### `Nst_iof_func_set`

**Synopsis:**

```better-c
Nst_IOFuncSet *Nst_iof_func_set(Nst_Obj *f)
```

**Returns:**

The [`Nst_IOFuncSet`](c_api-file.md#nst_iofuncset) of a file.

---

### `Nst_iof_fd`

**Synopsis:**

```better-c
int Nst_iof_fd(Nst_Obj *f)
```

**Returns:**

The file descriptor of a file. If it's negative the file is not a real file on
disk.

---

### `Nst_iof_fp`

**Synopsis:**

```better-c
void *Nst_iof_fp(Nst_Obj *f)
```

**Returns:**

A pointer to the internal data of a file. If the descriptor returned by
[`Nst_iof_fd`](c_api-file.md#nst_iof_fd) is positive this is of type `FILE *`.

---

### `Nst_iof_encoding`

**Synopsis:**

```better-c
Nst_Encoding *Nst_iof_encoding(Nst_Obj *f)
```

**Returns:**

The encoding of a file.

---

### `Nst_fread`

**Synopsis:**

```better-c
Nst_IOResult Nst_fread(u8 *buf, usize buf_size, usize count, usize *buf_len,
                       Nst_Obj *f)
```

**Description:**

Call the read function of the file, see
[`Nst_IOFile_read_f`](c_api-file.md#nst_iofile_read_f).

---

### `Nst_fwrite`

**Synopsis:**

```better-c
Nst_IOResult Nst_fwrite(u8 *buf, usize buf_len, usize *count, Nst_Obj *f)
```

**Description:**

Call the write function of the file, see
[`Nst_IOFile_write_f`](c_api-file.md#nst_iofile_write_f).

---

### `Nst_fflush`

**Synopsis:**

```better-c
Nst_IOResult Nst_fflush(Nst_Obj *f)
```

**Description:**

Call the flush function of the file, see
[`Nst_IOFile_flush_f`](c_api-file.md#nst_iofile_flush_f).

---

### `Nst_ftell`

**Synopsis:**

```better-c
Nst_IOResult Nst_ftell(Nst_Obj *f, usize *pos)
```

**Description:**

Call the tell function of the file, see
[`Nst_IOFile_tell_f`](c_api-file.md#nst_iofile_tell_f).

---

### `Nst_fseek`

**Synopsis:**

```better-c
Nst_IOResult Nst_fseek(Nst_SeekWhence origin, isize offset, Nst_Obj *f)
```

**Description:**

Call the seek function of the file, see
[`Nst_IOFile_seek_f`](c_api-file.md#nst_iofile_seek_f).

---

### `Nst_fclose`

**Synopsis:**

```better-c
Nst_IOResult Nst_fclose(Nst_Obj *f)
```

**Description:**

Call the close function of the file, see
[`Nst_IOFile_close_f`](c_api-file.md#nst_iofile_close_f).

---

### `Nst_FILE_read`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_read(u8 *buf, usize buf_size, usize count,
                           usize *buf_len, Nst_Obj *f)
```

**Description:**

Read function for standard C file descriptors.

---

### `Nst_FILE_write`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_write(u8 *buf, usize buf_len, usize *count, Nst_Obj *f)
```

**Description:**

Write function for standard C file descriptors.

---

### `Nst_FILE_flush`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_flush(Nst_Obj *f)
```

**Description:**

Flush function for standard C file descriptors.

---

### `Nst_FILE_tell`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_tell(Nst_Obj *f, usize *pos)
```

**Description:**

Tell function for standard C file descriptors.

---

### `Nst_FILE_close`

**Synopsis:**

```better-c
Nst_IOResult Nst_FILE_close(Nst_Obj *f)
```

**Description:**

Close function for standard C file descriptors.

---

### `Nst_io_result_get_details`

**Synopsis:**

```better-c
void Nst_io_result_get_details(u32 *ill_encoded_ch, usize *position,
                               const char **encoding_name)
```

**Description:**

Get the details of the [`Nst_IOResult`](c_api-file.md#nst_ioresult) returned by
the functions.

This function can only be called when the returned
[`Nst_IOResult`](c_api-file.md#nst_ioresult) is either
[`Nst_IO_INVALID_ENCODING`](c_api-file.md#nst_ioresult) or
[`Nst_IO_INVALID_DECODING`](c_api-file.md#nst_ioresult). If the result is the
former `ill_encoded_ch` will be the code point that could not be encoded,
otherwise, if the result is the latter, `ill_encoded_ch` will represent the byte
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
                               const char *encoding_name)
```

**Description:**

Set the values returned with
[`Nst_io_result_get_details`](c_api-file.md#nst_io_result_get_details).

---

### `Nst_fopen_unicode`

**Synopsis:**

```better-c
FILE *Nst_fopen_unicode(const char *path, const char *mode)
```

**Description:**

Open a file given a path that can contain unicode characters in UTF-8.

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
