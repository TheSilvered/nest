# Input/output library (`stdio.nest` - `io`)

## Functions

### `[Str, Str] @open`

Opens a file returning an `IOFile` object or a `null` object if the file was not
found.  
The first argument is the path to the file, the second is the mode to open the
file.

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

### `[IOFile] @close`

Closes a file.

### `[IOFile, Str] @write`

Writes to a file opened in `w`, `a`, `r+`, `w+` or `a+`.  
The first argument is the file, the second are the contents to write.

### `[IOFile, Array|Vector] @write_bytes`

Writes to a binary file opened in `wb`, `ab`, `rb+`, `wb+` or `ab+`.  
The first argument is the file, the second argument is an array or vector
containing only `Byte` objects.

### `[IOFile, Int] @read`

Reads a number of bytes from a file opened in `r`, `r+`, `w+` or `a+` and returns
a `Str` object.  
The first argument is the file, the second is the number of bytes to read, any
negative integer reads the whole file.

### `[IOFile, Array|Vector] @read_bytes`

Reads a number of bytes from a file opened in `r`, `r+`, `w+` or `a+` and returns
an `Array` object. To convert the array to a string, use the function
`bytearray_to_str` in `stdsutil.nest`.  
The first argument is the file, the second is the number of bytes to read, any
negative integer reads the whole file.

### `[IOFile] @file_size`

Returns the number of bytes the file contains.

### `[IOFile, Int, Int] @move_fptr`

Moves the file pointer. The first argument is the file itself, the second is the
starting position and the last is the offset from the start.  
The starting position can be set with `FROM_START`, `FROM_SET`, `FROM_CUR` and
`FROM_END`.

### `[IOFile] @get_fptr`

Returns the position of the file pointer.

### `[IOFile] @flush`

Flushes the buffer of a file.

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
