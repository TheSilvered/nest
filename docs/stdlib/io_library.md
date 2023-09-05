# Input-output library

## Importing

```nest
|#| 'stdio.nest' = io
```

## Functions

### `@can_read`

**Synopsis:**

`[file: IOFile] @can_read -> Bool`

**Returns:**

`true` if the file can be read and `false` otherwise.

---

### `@can_seek`

**Synopsis:**

`[file: IOFile] @can_seek -> Bool`

**Returns:**

`true` if the file can be saught and `false` otherwise.

---

### `@can_write`

**Synopsis:**

`[file: IOFile] @can_write -> Bool`

**Returns:**

`true` if the file can be written and `false` otherwise.

---

### `@close`

**Synopsis:**

`[file: IOFile] @close -> null`

**Description:**

Closes a file. After a file is closed, you will not be able to do any other
operation with it. If the file was already closed an error is thrown.
When the program ends any file that is still open gets closed.

**Arguments:**

- `file`: the file to close

---

### `@descriptor`

**Synopsis:**

`[file: IOFile] @descriptor -> Int`

**Returns:**

The file descriptor of a given file. If the file is closed or does not have a
descriptor `-1` is returned and no error is thrown.

---

### `@encoding`

**Synopsis:**

`[file: IOFile] @encoding -> Str`

**Returns:**

The encoding of the file as a string. If the file is closed or in binary mode
this function fails.

---

### `@file_size`

**Synopsis:**

`[file: IOFile] @file_size -> Int`

**Returns:**

The size of the file in bytes.

---

### `@flush`

**Synopsis:**

`[file: IOFile] @flush -> null`

**Description:**

Flushes the output buffer of a file.

---

### `@get_flags`

**Synopsis:**

`[file: IOFile] @get_flags -> Str`

**Returns:**

A 5-character string where the first character is `r` if the file can be read
and `-` otherwise, the second one is `w` if the file can be written and
`-` otherwise, the third one `b` if the file is opened in binary mode and
`-` if it is opened normally, the fourth is `s` if the file is seekable and
`-` otherwise and the last one is `t` if the file is a TTY and `-` otherwise.

**Example:**

```nest
|#| 'stdio.nest' = io

'a.txt' 'w' @io.open = f1
>>> (f1 @io.get_flags '\n' ><) --> '-w-s-'
f1 @io.close

'a.txt' 'r+' @io.open = f2
>>> (f2 @io.get_flags '\n' ><) --> 'rw-s-'
f2 @io.close

'a.txt' 'rb' @io.open = f3
>>> (f3 @io.get_flags '\n' ><) --> 'r-bs-'
f3 @io.close
```

---

### `@get_fpi`

**Synopsis:**

`[file: IOFile] @get_fpi -> Int`

**Description:**

Returns the position in bytes of the file pointer.

---

### `@is_a_tty`

**Synopsis:**

`[file: IOFile] @is_a_tty -> Bool`

**Returns:**

`true` if the file is a TTY and `false` otherwise.

---

### `@is_bin`

**Synopsis:**

`[file: IOFile] @is_bin -> Bool`

**Returns:**

`true` if the file was opened in binary mode and `false` otherwise.

---

### `@move_fpi`

**Synopsis:**

`[file: IOFile, starting_position: Int, offset: Int] @move_fpi -> null`

**Description:**

Moves the file pointer from `starting_position` that can be set with
`FROM_START`, `FROM_SET`, `FROM_CUR` and `FROM_END` by a number of bytes
specified by the `offset`.
`offset` can also be negative.

**Arguments:**

- `file`: the file of which the file pointer should be moved
- `starting_position`: the position from which the offset is applied
- `offset`: the offset in bytes from the starting position

---

### `@open`

**Synopsis:**

`[path: Str, mode: Str?, encoding: Str?, buf_size: Int?] @open -> IOFile`

**Description:**

Opens a file. If the mode is `null` it is set to `r`. If encoding is `null` it
is set to `utf8`. If `buf_size` is `null` it is set to 512 bytes.

The file modes are:

| Mode           | Description                                   |
| -------------- | --------------------------------------------- |
| `w`            | write                                         |
| `wb`           | write bytes                                   |
| `r`            | read                                          |
| `rb`           | read bytes                                    |
| `a`            | append                                        |
| `ab`           | append bytes                                  |
| `r+`           | read and write, keeping the contents          |
| `rb+` or `r+b` | read and write bytes, keeping the contents    |
| `w+`           | read and write, destroying the contents       |
| `wb+` or `w+b` | read and write bytes, destroying the contents |
| `a+`           | read and append, keeping the contents         |
| `ab+` or `a+b` | read and append bytes, keeping the contents   |

The encodings are:

| Encoding   | Aliases                                            |
| ---------- | -------------------------------------------------- |
| `ascii`    | `us-ascii`                                         |
| `cp1250`   | `cp-1250`, `windows1250`, `windows-1250`           |
| `cp1251`   | `cp-1251`, `windows1251`, `windows-1251`           |
| `cp1252`   | `cp-1252`, `windows1252`, `windows-1252`           |
| `cp1253`   | `cp-1253`, `windows1253`, `windows-1253`           |
| `cp1254`   | `cp-1254`, `windows1254`, `windows-1254`           |
| `cp1255`   | `cp-1255`, `windows1255`, `windows-1255`           |
| `cp1256`   | `cp-1256`, `windows1256`, `windows-1256`           |
| `cp1257`   | `cp-1257`, `windows1257`, `windows-1257`           |
| `cp1258`   | `cp-1258`, `windows1258`, `windows-1258`           |
| `latin-1`  | `latin1`, `l1`, `latin`, `iso-8859-1`, `iso8859-1` |
| `utf8`     | `utf-8`                                            |
| `ext-utf8` | `ext-utf-8`, `extutf8`, `extutf-8`                 |
| `utf16le`  | `utf-16le`, `utf16`, `utf-16`                      |
| `utf16be`  | `utf-16be`                                         |
| `utf32le`  | `utf-32le`, `utf32`, `utf-32`                      |
| `utf32be`  | `utf-32be`                                         |

The name of the encoding is case insensitive. Underscores (`_`), hyphens (`-`)
and spaces (` `) are interchangeable. This means that any of the following is
recognized as UTF-8.

`utf8`, `utf-8`, `utf_8`, `utf 8`, `Utf8`, `Utf-8`,
`Utf_8`, `Utf 8`, `uTf8`, `uTf-8`, `uTf_8`, `uTf 8`, `UTf8`, `UTf-8`, `UTf_8`,
`UTf 8`, `utF8`, `utF-8`, `utF_8`, `utF 8`, `UtF8`, `UtF-8`, `UtF_8`, `UtF 8`,
`uTF8`, `uTF-8`, `uTF_8`, `uTF 8`, `UTF8`, `UTF-8`, `UTF_8`, `UTF 8`.

**Arguments:**

- `path`: the path of the file to open
- `mode`: the mode in which it should be opened
- `encoding`: the encoding used to open the file, if `mode` is binary this
  argument must be `null`
- `buf_size`: the size of the buffer of the file

**Returns:**

An `IOFile` object or `null` if the file was not found.

---

### `@println`

**Synopsis:**

`[object: Any, flush: Bool?, file: IOFile?] @println -> null`

**Description:**

Will print `object` like `>>>` followed by a newline.
`flush` specifies if the file must be flushed and is `false` by default.
`file` is the file where the object should be printed, stdout by default. If it
is closed an error will be thrown.

**Arguments:**

- `object`: the object to be printed
- `flush`: whether the file should be flushed
- `file`: the file to write to

---

### `@read`

**Synopsis:**

`[file: IOFile, size: Int?] @read -> Str`

**Description:**

Reads a number of characters from a file opened in `r`, `r+`, `w+` or `a+` and
returns a `Str` object. If `size` is negative or `null` the whole file is read.
If the file is not seekable and the whole file is trying to be read, an error
is thrown.

**Arguments:**

- `file`: the file to be read
- `size`: the number or characters to read

**Returns:**

The content that it read as a string.

---

### `@read_bytes`

**Synopsis:**

`[file: IOFile, size: Int?] @read_bytes -> Array.Byte`

**Description:**

Reads a number of bytes from a file opened in `rb`, `rb+`, `wb+` or `ab+` and
returns an `Array` object. To convert the array to a string, use the
[`bytearray_to_str`](string_utilities_library.md#bytearray_to_str) function in
`stdsutil.nest`. If `size` is negative or `null` the whole file is read.

**Arguments:**

- `file`: the file to be read
- `size`: the number of bytes to read

**Returns:**

The content that it read as an array of `Byte` objects.

---

### `@virtual_file`

**Synopsis:**

`[binary: Bool?, buffer_size: Int?] @virtual_file -> IOFile`

Creates a virtual `IOFile` object that works like a normal file but is not an
actual file.

**Arguments:**
- `binary`: specifies if the file should use `write` and `read` or `write_bytes`
  and `read_bytes`. If set to `null` it is interpreted as false.
- `buffer_size` specifies the initial size of the file in bytes

**Returns:**

The newly created file.

---

### `@write`

**Synopsis:**

`[file: IOFile, content: Any] @write -> Int`

**Description:**

Writes to a file opened in `w`, `a`, `r+`, `w+` or `a+`. `content` is casted
to a string before being written. If the file is closed an error is thrown.

**Arguments:**

- `file`: the file to be written
- `content`: the contents to be written

**Returns:**

The number of characters written.

**Example:**

```nest
|#| 'stdio.nest' = io

'example.txt' 'w' @io.open = f
f { 1, 2, 3 } @io.write
f @io.close
'example.txt' @io.open = f
f @io.read @io.println --> '{ 1, 2, 3 }'
f @io.close
```

---

### `@write_bytes`

**Synopsis:**

`[file: IOFile, content: Array|Vector.Byte] @write_bytes -> Int`

**Description:**

Writes to a binary file opened in `wb`, `ab`, `rb+`, `wb+` or `ab+`.
The second argument is an array or vector containing only `Byte` objects.
To create such vector from a string, use the
[`str_to_bytearray`](string_utilities_library.md#str_to_bytearray)
function in `stdsutil.nest`.

**Arguments:**

- `file`: the file to be written
- `content`: the sequence of bytes to write

**Returns:**

The number of bytes written.

---

### `@_get_stdin`

**Synopsis:**

`[] @_get_stdin -> IOFile`

**Returns:**

The current input stream. If `_set_stdin` is never called this will return the
same object that `STDIN` points to.

---

### `@_get_stderr`

**Synopsis:**

`[] @_get_stdin -> IOFile`

**Returns:**

The current error stream. If `_set_stderr` is never called this will return the
same object that `STDERR` points to.

---

### `@_get_stdout`

**Synopsis:**

`[] @_get_stdin -> IOFile`

**Returns:**

The current output stream. If `_set_stdout` is never called this will return
the same object that `STDOUT` points to.

---

### `@_set_stdin`

**Synopsis:**

`[file: IOFile] @_set_stdin -> null`

**Description:**

Changes the standard input stream to `file`.
`file` must support reading and must not be already closed.

The [`STDIN`](#stdin) constant will **not** reflect any changes and will always
point to the original input stream unless changed manually.

---

### `@_set_stderr`

**Synopsis:**

`[file: IOFile] @_set_stderr -> null`

**Description:**

Changes the standard error stream to `file`.
`file` must support writing and must not be already closed.

The [`STDERR`](#stderr) constant will **not** reflect any changes and will
always point to the original error stream unless changed manually.

---

### `@_set_stdout`

**Synopsis:**

`[file: IOFile] @_set_stdout -> null`

**Description:**

Changes the standard output stream to `file`.
`file` must support writing and must not be already closed.

The [`STDOUT`](#stdout) constant will **not** reflect any changes and will
always point to the original output stream unless changed manually.

---

## Constants

### `FROM_START`

Used for `move_fptr`, puts the file pointer at the start of the file. The same
as `SEEK_SET` in C.

---

### `FROM_SET`

The same as `FROM_START` with a more familiar name.

---

### `FROM_CUR`

Used for `move_fptr`, does not move the file pointer. The same as `SEEK_CUR` in
C.

---

### `FROM_END`

Used for `move_fptr`, puts the file pointer at the end of the file. The same as
`SEEK_SET` in C.

---

### `STDIN`

File object of the standard input stream, changing this constant does not change
the stream used by `<<<`, use [`_set_stdin`](#_set_stdin) instead.

---

### `STDERR`

File object of the standard error stream, changing this constant does not change
the actual error stream, use [`_set_stderr`](#_set_stderr) instead.

---

### `STDOUT`

File object of the standard output stream, changing this constant does not
change the stream used by `>>>`, use [`_set_stdout`](#_set_stdout)
instead.
