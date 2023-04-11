# `error.h`

The header that contains the functions and structures used for error handling,
tracking and reporting in Nest.

## Macros

### `_NST_EM_*` macros

**Description**:

All the macros starting with `_NST_EM` are error messages used internally by the
interpreter.

---

## Structs

### `Nst_SourceText`

**Synopsis**:

```better-c
typedef struct _Nst_SourceText
{
    i8 *text;
    i8 *path;
    i8 **lines;
    usize len;
    usize line_count;
}
Nst_SourceText;
```

**Description**:

A struct that holds the source text of a loaded file, that could be either the
main file or any modules imported afterwards.

**Fields**:

- `text` the text itself
- `path` the path of the file
- `lines` an array of pointers to the start of each line of the file
- `len` the length of `text`
- `line_count` the number of lines present in the file

---

### `Nst_Pos`

**Synopsis**:

```better-c
typedef struct _Nst_Pos
{
    i32 line;
    i32 col;
    Nst_SourceText *text;
}
Nst_Pos;
```

**Description**:

A struct that keeps a position inside a certain file.

**Fields**:

- `line` the line of the position, starts from `0`
- `col` the column of the position, also starts from `0`

---

### `Nst_Error`

**Synopsis**:

```better-c
typedef struct _Nst_Error
{
    bool occurred;
    Nst_Pos start;
    Nst_Pos end;
    Nst_StrObj *name;
    Nst_StrObj *message;
}
Nst_Error;
```

**Description**:

The internal struct for errors.

**Fields**:

- `occurred` whether the struct contains a valid error
- `start` the start position of the error
- `end` the end position of the error, inclusive
- `name` the name of the error
- `message` the message of the error

---

### `Nst_OpError`

**Synopsis**:

```better-c
typedef struct _Nst_OpErr
{
    Nst_StrObj *name;
    Nst_StrObj *message;
}
Nst_OpErr;
```

**Description**:

The error used by C functions.

**Fields**:

- `name` the name of the error
- `message` the message of the error

---

### `Nst_Traceback`

**Synopsis**:

```better-c
typedef struct _Nst_Traceback
{
    Nst_Error error;
    Nst_LList *positions;
}
Nst_Traceback;
```

**Description**:

The trace of positions that let to the error.

**Fields**:

- `error` the error that occurred
- `positions` the list of positions structured like \[start1, end1, start2,
  end2, ...]

---

## Functions

### `nst_set_color`

**Synopsis**:

```better-c
void nst_set_color(bool color)
```

**Description**:

Sets whether the error output should be printed with ANSI color escapes.

**Arguments**:

- `[in] color` if set to `true`, color is used otherwise it is not

---

### `nst_copy_pos`

**Synopsis**:

```better-c
Nst_Pos nst_copy_pos(Nst_Pos pos)
```

**Description**:

Copies a position on the stack.

**Arguments**:

- `[in] pos` the position to be copied

**Return value**:

Returns the copied position.

---

### `nst_no_pos`

**Synopsis**:

```better-c
Nst_Pos nst_no_pos()
```

**Description**:

Creates an empty position.

**Return value**:

The position created.

---

### `nst_print_error`

**Synopsis**:

```better-c
void nst_print_error(Nst_Error err)
```

**Description**:

Prints an error to standard error formatting it and then frees it.

**Arguments**:

- `[in] err` the error to be printed

---

### `nst_print_traceback`

**Synopsis**:

```better-c
void nst_print_traceback(Nst_Traceback tb)
```

**Description**:

Prints an traceback to standard error formatting it and then frees it.

**Arguments**:

- `[in] tb` the traceback to be printed

---

### `nst_free_src_text`

**Synopsis**:

```better-c
void nst_free_src_text(Nst_SourceText *text)
```

**Description**:

Frees a heap-allocated text source.

**Arguments**:

- `[in] text` the text to be freed
