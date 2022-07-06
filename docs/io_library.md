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

### `[file: IOFile] @close`

Closes a file.

### `[file: IOFile, content: Str] @write`

Writes to a file opened in `w`, `a`, `r+`, `w+` or `a+`.

### `[file: IOFile, content: Array|Vector] @write_bytes`

Writes to a binary file opened in `wb`, `ab`, `rb+`, `wb+` or `ab+`.  
The second argument is an array or vector containing only `Byte` objects.

### `[file: IOFile, size: Int] @read`

Reads a number of bytes from a file opened in `r`, `r+`, `w+` or `a+` and returns
a `Str` object.  
Any negative integer for `size` reads the whole file.

### `[file: IOFile, size: Int] @read_bytes`

Reads a number of bytes from a file opened in `r`, `r+`, `w+` or `a+` and returns
an `Array` object. To convert the array to a string, use the function
`bytearray_to_str` in `stdsutil.nest`.  
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

### `[file: IOFile] @at_eof`

Returns `true` if the file pointer is at the end of the file, `false` otherwise.

## Constants

### `STDIN`

File pointer to the standard input.

### `STDOUT`

File pointer to the standard output.

### `STDERR`

File pointer to the standard error.

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
