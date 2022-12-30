# Input/output library (`stdio.nest` - `io`)

## Functions

### `[path: Str, mode: Str] @open`

Opens a file returning an `IOFile` object or a `null` object if the file was not
found.

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

### `[binary: Bool] @virtual_iof`

Creates a virtual `IOFile` object that works like a normal file but is not an
actual file.  
`binary` specifies if the file should use `write` and `read` or `write_bytes` ad
`read_bytes`.

### `[file: IOFile] @close`

Closes a file. After a file is closed, you will not be able to do any other
operation with it.

### `[file: IOFile, content: Any] @write`

Writes to a file opened in `w`, `a`, `r+`, `w+` or `a+`. `content` is casted
to a string before being written.

```text
|#| 'stdio.nest' = io

'a.txt' 'w' @io.open = f
f { 1, 2, 3 } @io.write
f @io.close
```

Now the file `a.txt` contains the string `{ 1, 2, 3 }`

### `[file: IOFile, content: Array|Vector] @write_bytes`

Writes to a binary file opened in `wb`, `ab`, `rb+`, `wb+` or `ab+`.  
The second argument is an array or vector containing only `Byte` objects.  
To create such vector from a string, use the
[`str_to_bytearray`](string_utilities_library.md#string-str-str_to_bytearray)
function in `stdsutil.nest`.

### `[file: IOFile, size: Int] @read`

Reads a number of bytes from a file opened in `r`, `r+`, `w+` or `a+` and returns
a `Str` object.  
Any negative integer for `size` reads the whole file.

### `[file: IOFile, size: Int] @read_bytes`

Reads a number of bytes from a file opened in `rb`, `rb+`, `wb+` or `ab+` and
returns an `Array` object. To convert the array to a string, use the
[`bytearray_to_str`](string_utilities_library.md#sequence-arrayvector-bytearray_to_str)
function in `stdsutil.nest`.  
Any negative integer for `size` reads the whole file.

### `[file: IOFile] @file_size`

Returns the number of bytes that the file contains.

### `[file: IOFile, starting_position: Int, offset: Int] @move_fptr`

Moves the file pointer from `starting_position` that can be set with
`FROM_START`, `FROM_SET`, `FROM_CUR` and `FROM_END` by a number of bytes
specified by the `offset`.  
`offset` can also be negative.

### `[file: IOFile] @get_fptr`

Returns the position in bytes of the file pointer.

### `[file: IOFile] @flush`

Flushes the output buffer of a file.

### `[file: IOFile] @get_flags`

Returns a 3-characted string where the first character is `r` if the file can
be read and `-` otherwise, the second one is `w` if the file can be written and
`-` otherwise and the last one `b` if the file is opened in binary mode and
`-` if it is opened normally.

```text
|#| 'stdio.nest' = io

'a.txt' 'w' @io.open = f1
>>> (f1 @io.get_flags '\n' ><) --> '-w-'
f1 @io.close

'a.txt' 'r+' @io.open = f2
>>> (f2 @io.get_flags '\n' ><) --> 'rw-'
f2 @io.close

'a.txt' 'rb' @io.open = f3
>>> (f3 @io.get_flags '\n' ><) --> 'r-b'
f3 @io.close
```

### `[file: IOFile] @_set_stdin`

Changes the standard input stream to `file`.  
`file` must support reading and must not be already closed.

The [`STDIN`](#stdin) constant will **not** reflect any changes and will always
point to the original input stream unless changed manually.

### `[file: IOFile] @_set_stdout`

Changes the standard output stream to `file`.  
`file` must support writing and must not be already closed.

The [`STDOUT`](#stdout) constant will **not** reflect any changes and will
always point to the original output stream unless changed manually.

### `[file: IOFile] @_set_stderr`

Changes the standard error stream to `file`.  
`file` must support writing and must not be already closed.

The [`STDERR`](#stderr) constant will **not** reflect any changes and will
always point to the original error stream unless changed manually.

### `[file: IOFile] @_get_stdin`

Returns the current input stream. If `_set_stdin` is never called this will
return the same object that `STDIN` points to.

### `[file: IOFile] @_get_stdout`

Returns the current output stream. If `_set_stdout` is never called this will
return the same object that `STDOUT` points to.

### `[file: IOFile] @_get_stderr`

Returns the current error stream. If `_set_stderr` is never called this will
return the same object that `STDERR` points to.

## Constants

### `STDIN`

File object of the standard input stream, changing this constant does not change
the stream used by `<<<`, use [`_set_stdin`](#file-iofile-_set_stdin) instead.

### `STDOUT`

File object of the standard output stream, changing this constant does not
change the stream used by `>>>`, use [`_set_stdout`](#file-iofile-_set_stdout)
instead.

### `STDERR`

File object of the standard error stream, changing this constant does not change
the actual error stream, use [`_set_stderr`](#file-iofile-_set_stderr) instead.

### `FROM_START`

Used for `move_fptr`, puts the file pointer at the start of the file. The same
as `SEEK_SET` in C.

### `FROM_SET`

The same as `FROM_START` with a more familiar name.

### `FROM_CUR`

Used for `move_fptr`, does not move the file pointer. The same as `SEEK_CUR` in
C.

### `FROM_END`

Used for `move_fptr`, puts the file pointer at the end of the file. The same as
`SEEK_SET` in C.
