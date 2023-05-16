# `simple_types.h`

This header defines the Nest int, real, byte and file objects.

## Macros

### `NST_TRUE`

**Description**:

The value `true` for [`Nst_Bool`](#nst_int-nst_real-nst_bool-nst_byte).

---

### `NST_FALSE`

**Description**:

The value `false` for [`Nst_Bool`](#nst_int-nst_real-nst_bool-nst_byte).

---

### `AS_INT`

**Synopsis**:

```better-c
AS_INT(ptr)
```

**Description**:

Casts `ptr` to `Nst_IntObj *` and gets its value field.

---

### `AS_REAL`

**Synopsis**:

```better-c
AS_REAL(ptr)
```

**Description**:

Casts `ptr` to `Nst_RealObj *` and gets its value field.

---

### `AS_BYTE`

**Synopsis**:

```better-c
AS_BYTE(ptr)
```

**Description**:

Casts `ptr` to `Nst_ByteObj *` and gets its value field.

---

### `AS_BOOL`

**Synopsis**:

```better-c
AS_BOOL(ptr)
```

**Description**:

Casts `ptr` to `Nst_BoolObj *` and gets its value field.

---

### `IOFILE`

**Synopsis**:

```better-c
IOFILE(ptr)
```

**Description**:

Casts `ptr` to `Nst_IOFile *`

---

### `NST_IOF_IS_CLOSED`

**Synopsis**:

```better-c
NST_IOF_IS_CLOSED(f)
```

**Description**:

Whether `f` has the `NST_FLAG_IOFILE_IS_CLOSED` flag set.

---

### `NST_IOF_IS_BIN`

**Synopsis**:

```better-c
NST_IOF_IS_BIN(f)
```

**Description**:

Whether `f` has the `NST_FLAG_IOFILE_IS_BIN` flag set.

---

### `NST_IOF_CAN_WRITE`

**Synopsis**:

```better-c
NST_IOF_CAN_WRITE(f)
```

**Description**:

Whether `f` has the `NST_FLAG_IOFILE_CAN_WRITE` flag set.

---

### `NST_IOF_CAN_READ`

**Synopsis**:

```better-c
NST_IOF_CAN_READ(f)
```

**Description**:

Whether `f` has the `NST_FLAG_IOFILE_CAN_READ` flag set.

---

## Structs

### `Nst_IntObj`

**Synopsis**:

```better-c
typedef struct _Nst_IntObj
{
    NST_OBJ_HEAD;
    Nst_Int value;
}
Nst_IntObj
```

**Description**:

The structure defining a Nest int object.

---

### `Nst_RealObj`

**Synopsis**:

```better-c
typedef struct _Nst_RealObj
{
    NST_OBJ_HEAD;
    Nst_Real value;
}
Nst_RealObj
```

**Description**:

The structure defining a Nest real object.

---

### `Nst_BoolObj`

**Synopsis**:

```better-c
typedef struct _Nst_BoolObj
{
    NST_OBJ_HEAD;
    Nst_Bool value;
}
Nst_BoolObj
```

**Description**:

The structure defining a Nest bool object.

---

### `Nst_ByteObj`

**Synopsis**:

```better-c
typedef struct _Nst_ByteObj
{
    NST_OBJ_HEAD;
    Nst_Byte value;
}
Nst_ByteObj
```

**Description**:

The structure defining a Nest byte object.

---

### `Nst_IOFileObj`

**Synopsis**:

```better-c
typedef struct _Nst_IOFileObj
{
    NST_OBJ_HEAD;
    Nst_IOFile value;
    Nst_IOFile_read_f  read_f;
    Nst_IOFile_write_f write_f;
    Nst_IOFile_flush_f flush_f;
    Nst_IOFile_tell_f  tell_f;
    Nst_IOFile_seek_f  seek_f;
    Nst_IOFile_close_f close_f;
}
Nst_IOFileObj
```

**Description**:

The structure defining a Nest file object.

**Fields**:

- `value`: the file pointer
- `read_f`: the function to read from the file, does not check for the 
  `NST_FLAG_IOFILE_CAN_READ` flag
- `write_f`: the function to write to the file, does not check for the 
  `NST_FLAG_IOFILE_CAN_WRITE` flag
- `flush_f`: the function to flush the file buffer
- `tell_f`: the function to get the file cursor position
- `seek_f`: the function to set the file cursor position
- `close_f`: the function to close the file, does not set the
  `NST_FLAG_IOFILE_IS_CLOSED` flag

---

## Type aliases

### `Nst_Int`, `Nst_Real`, `Nst_Bool`, `Nst_Byte`

**Synopsis**:

```better-c
typedef i64 Nst_Int
typedef f64 Nst_Real
typedef i8 Nst_Bool
typedef u8 Nst_Byte
```

---

### `Nst_IOFile`

**Synopsis**:

```better-c
typedef FILE *Nst_IOFile
```

---

### `Nst_IOFile_read_f`, `Nst_IOFile_write_f`, `Nst_IOFile_flush_f`, `Nst_IOFile_tell_f`, `Nst_IOFile_seek_f`, `Nst_IOFile_close_f`

**Synopsis**:

```better-c
typedef usize (*Nst_IOFile_read_f)(void  *buf,
                                   usize  size,
                                   usize  count,
                                   void  *f_value)

typedef usize (*Nst_IOFile_write_f)(void  *buf,
                                    usize  size,
                                    usize  count,
                                    void  *f_value)

typedef i32 (*Nst_IOFile_flush_f)(void *f_value)
typedef i32 (*Nst_IOFile_tell_f)(void *f_value)
typedef i32 (*Nst_IOFile_seek_f)(void *f_value, i32 offset, i32 origin)
typedef i32 (*Nst_IOFile_close_f)(void *f_value)
```

## Functions

### `nst_int_new`

**Synopsis**:

```better-c
Nst_Obj *nst_int_new(Nst_Int value, Nst_OpErr *err)
```

**Description**:

Creates a new Nest int object.

**Arguments**:

- `[in] value`: the value of the object
- `[out] err`: the error

**Return value**:

The function returns the new object on success and `NULL` on failure.

---

### `nst_real_new`

**Synopsis**:

```better-c
Nst_Obj *nst_real_new(Nst_Real value, Nst_OpErr *err)
```

**Description**:

Creates a new Nest real object.

**Arguments**:

- `[in] value`: the value of the object
- `[out] err`: the error

**Return value**:

The function returns the new object on success and `NULL` on failure.

---

### `nst_bool_new`

**Synopsis**:

```better-c
Nst_Obj *nst_bool_new(Nst_Bool value, Nst_OpErr *err)
```

**Description**:

Creates a new Nest bool object. It should never be called since the `true` and
`false` objects are always the same through the lifetime of the program; use
`nst_true()` and `nst_false()` to get the boolean values instead.

**Arguments**:

- `[in] value`: the value of the object
- `[out] err`: the error

**Return value**:

The function returns the new object on success and `NULL` on failure.

---

### `nst_byte_new`

**Synopsis**:

```better-c
Nst_Obj *nst_byte_new(Nst_Byte value, Nst_OpErr *err)
```

**Description**:

Creates a new Nest byte object.

**Arguments**:

- `[in] value`: the value of the object
- `[out] err`: the error

**Return value**:

The function returns the new object on success and `NULL` on failure.

---

### `nst_iof_new`

**Synopsis**:

```better-c
Nst_Obj *nst_iof_new(Nst_IOFile value,
                     bool       bin,
                     bool       read,
                     bool       write,
                     Nst_OpErr *err)
```

**Description**:

Creates a new Nest file object.

**Arguments**:

- `[in] value`: the file pointer of the file object
- `[in] bin`: whether the file was opened in binary mode
- `[in] read`: whether the file supports reading
- `[in] write`: whether the file supports writing
- `[out] err`: the error

**Return value**:

The function returns the new object or `NULL` on failure.

---

### `nst_iof_new_fake`

**Synopsis**:

```better-c
Nst_Obj *nst_iof_new_fake(void *value,
                          bool bin, bool read, bool write,
                          Nst_IOFile_read_f  read_f,
                          Nst_IOFile_write_f write_f,
                          Nst_IOFile_flush_f flush_f,
                          Nst_IOFile_tell_f  tell_f,
                          Nst_IOFile_seek_f  seek_f,
                          Nst_IOFile_close_f close_f,
                          Nst_OpErr *err)
```

**Description**:

Creates a new object that emulates a file with custom functions. See
`libs/nest_io/nest_io.cpp` for an example.

**Arguments**:

- `[in] value`: the file pointer of the file object
- `[in] bin`: whether the file was opened in binary mode
- `[in] read`: whether the file supports reading
- `[in] write`: whether the file supports writing
- `[in] read_f`: the function used to read from the file
- `[in] write_f`: the function used to write to the file
- `[in] flush_f`: the function used to flush the file buffer
- `[in] tell_f`: the function used to get the cursor position of the file
- `[in] seek_f`: the function used to set the cursor position of the file
- `[in] close_f`: the function used to close the file
- `[out] err`: the error

**Return value**:

The function returns the new object or `NULL` on failure.

---

### `_nst_iofile_destroy`

**Synopsis**:

```better-c
void _nst_iofile_destroy(Nst_IOFileObj *obj)
```

**Description**:

The function used to destroy a file object.

---

### `nst_fread`

**Synopsis**:

```better-c
usize nst_fread(void          *buf,
                usize          size,
                usize          count,
                Nst_IOFileObj *f)
```

**Description**:

Reads from a file object. It checks the `NST_FLAG_IOFILE_IS_CLOSED` and
`NST_FLAG_IOFILE_CAN_READ` flags.

**Arguments**:

- `[inout] buf`: the buffer where to store the contents read
- `[in] size`: the size of one element in bytes
- `[in] count`: the maximum number of elements to read
- `[inout] f`: the file to read from

**Return value**:

The function returns the number of elements read that can be less than `couunt`
or `0` when the cursor is at the end of the file and `-1` on failure.

---

### `nst_fwrite`

**Synopsis**:

```better-c
usize nst_fwrite(void          *buf,
                 usize          size,
                 usize          count,
                 Nst_IOFileObj *f)
```

**Description**:

Writes to a file object. It checks the `NST_FLAG_IOFILE_IS_CLOSED` and
`NST_FLAG_IOFILE_CAN_WRITE` flags.

**Arguments**:

- `[inout] buf`: the buffer where to read the contentes from
- `[in] size`: the size of one element in bytes
- `[in] count`: the number of elements to write
- `[inout] f`: the file to write to

**Return value**:

The function returns the number of elements written or `-1` on failure.

---

### `nst_fflush`

**Synopsis**:

```better-c
i32 nst_fflush(Nst_IOFileObj *f)
```

**Description**:

Flushes the contentes of a file, checks for the `NST_FLAG_IOFILE_IS_CLOSED` flag.

**Arguments**:

- `[inout] f`: the file to flush

**Return value**:

The function returns `0` on success and `EOF` on failure.

---

### `nst_ftell`

**Synopsis**:

```better-c
i32 nst_ftell(Nst_IOFileObj *f)
```

**Arguments**:

- `[in] f`: the file to get the cursor position of

**Return value**:

The function returns the current position of the cursor of the file or `-1` on
failure. Checks for the `NST_FLAG_IOFILE_IS_CLOSED` flag.

---

### `nst_fseek`

**Synopsis**:

```better-c
i32 nst_fseek(Nst_IOFileObj *f, i32 offset, i32 origin)
```

**Description**:

Moves the file cursor, checks for the `NST_FLAG_IOFILE_IS_CLOSED` flag.

**Arguments**:

- `[inout] f`: the file to move the cursor of
- `[in] offset`: the number of bytes to move from `origin`
- `[in] origin`: the starting position of `offset`, it can be one of `SEEK_END`,
  `SEEK_CUR` and `SEEK_SET`

**Return value**:

The function returns `0` on success and `-1` on failure.

---

### `nst_fclose`

**Synopsis**:

```better-c
i32 nst_fclose(Nst_IOFileObj *f)
```

**Description**:

Closes a file setting the `NST_FLAG_IOFILE_IS_CLOSED` flag.

**Arguments**:

- `[inout] f`: the file to close

**Return value**:

The function returns `0` on success and `EOF` on failure.

---

## Enums

### `Nst_IOFileFlag`

**Synopsis**:

```better-c
typedef enum _Nst_IOFileFlag
{
    NST_FLAG_IOFILE_IS_CLOSED = 0b0001,
    NST_FLAG_IOFILE_IS_BIN    = 0b0010,
    NST_FLAG_IOFILE_CAN_WRITE = 0b0100,
    NST_FLAG_IOFILE_CAN_READ  = 0b1000
}
Nst_IOFileFlag
```

**Description**:

The flags used to manage file objects.
