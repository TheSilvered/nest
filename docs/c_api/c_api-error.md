# `error.h`

Error management interface.

## Authors

TheSilvered

## Macros

### `Nst_set_error`

**Synopsis:**

```better-c
Nst_set_error(name, msg)
```

**Description:**

Alias for [`_Nst_set_error`](c_api-error.md/#_nst_set_error) that casts `name`
and `msg` to [`Nst_StrObj *`](c_api-str.md/#nst_strobj).

---

### `Nst_set_syntax_error`

**Synopsis:**

```better-c
Nst_set_syntax_error(msg)
```

**Description:**

Alias for [`_Nst_set_syntax_error`](c_api-error.md/#_nst_set_syntax_error) that
casts `msg` to [`Nst_StrObj *`](c_api-str.md/#nst_strobj).

---

### `Nst_set_memory_error`

**Synopsis:**

```better-c
Nst_set_memory_error(msg)
```

**Description:**

Alias for [`_Nst_set_memory_error`](c_api-error.md/#_nst_set_memory_error) that
casts `msg` to [`Nst_StrObj *`](c_api-str.md/#nst_strobj).

---

### `Nst_set_type_error`

**Synopsis:**

```better-c
Nst_set_type_error(msg)
```

**Description:**

Alias for [`_Nst_set_type_error`](c_api-error.md/#_nst_set_type_error) that
casts `msg` to [`Nst_StrObj *`](c_api-str.md/#nst_strobj).

---

### `Nst_set_value_error`

**Synopsis:**

```better-c
Nst_set_value_error(msg)
```

**Description:**

Alias for [`_Nst_set_value_error`](c_api-error.md/#_nst_set_value_error) that
casts `msg` to [`Nst_StrObj *`](c_api-str.md/#nst_strobj).

---

### `Nst_set_math_error`

**Synopsis:**

```better-c
Nst_set_math_error(msg)
```

**Description:**

Alias for [`_Nst_set_math_error`](c_api-error.md/#_nst_set_math_error) that
casts `msg` to [`Nst_StrObj *`](c_api-str.md/#nst_strobj).

---

### `Nst_set_call_error`

**Synopsis:**

```better-c
Nst_set_call_error(msg)
```

**Description:**

Alias for [`_Nst_set_call_error`](c_api-error.md/#_nst_set_call_error) that
casts `msg` to [`Nst_StrObj *`](c_api-str.md/#nst_strobj).

---

### `Nst_set_import_error`

**Synopsis:**

```better-c
Nst_set_import_error(msg)
```

**Description:**

Alias for [`_Nst_set_import_error`](c_api-error.md/#_nst_set_import_error) that
casts `msg` to [`Nst_StrObj *`](c_api-str.md/#nst_strobj).

---

### `Nst_set_errorf`

**Synopsis:**

```better-c
Nst_set_errorf(name, fmt, ...)
```

**Description:**

Alias for [`_Nst_set_error`](c_api-error.md/#_nst_set_error) that casts `name`
to [`Nst_StrObj *`](c_api-str.md/#nst_strobj) and builds a formatted string with
`fmt`.

---

### `Nst_set_syntax_errorf`

**Synopsis:**

```better-c
Nst_set_syntax_errorf(fmt, ...)
```

**Description:**

Alias for [`_Nst_set_syntax_error`](c_api-error.md/#_nst_set_syntax_error) that
builds a formatted string with `fmt`.

---

### `Nst_set_memory_errorf`

**Synopsis:**

```better-c
Nst_set_memory_errorf(fmt, ...)
```

**Description:**

Alias for [`_Nst_set_memory_error`](c_api-error.md/#_nst_set_memory_error) that
builds a formatted string with `fmt`.

---

### `Nst_set_type_errorf`

**Synopsis:**

```better-c
Nst_set_type_errorf(fmt, ...)
```

**Description:**

Alias for [`_Nst_set_type_error`](c_api-error.md/#_nst_set_type_error) that
builds a formatted string with `fmt`.

---

### `Nst_set_value_errorf`

**Synopsis:**

```better-c
Nst_set_value_errorf(fmt, ...)
```

**Description:**

Alias for [`_Nst_set_value_error`](c_api-error.md/#_nst_set_value_error) that
builds a formatted string with `fmt`.

---

### `Nst_set_math_errorf`

**Synopsis:**

```better-c
Nst_set_math_errorf(fmt, ...)
```

**Description:**

Alias for [`_Nst_set_math_error`](c_api-error.md/#_nst_set_math_error) that
builds a formatted string with `fmt`.

---

### `Nst_set_call_errorf`

**Synopsis:**

```better-c
Nst_set_call_errorf(fmt, ...)
```

**Description:**

Alias for [`_Nst_set_call_error`](c_api-error.md/#_nst_set_call_error) that
builds a formatted string with `fmt`.

---

### `Nst_set_import_errorf`

**Synopsis:**

```better-c
Nst_set_import_errorf(fmt, ...)
```

**Description:**

Alias for [`_Nst_set_import_error`](c_api-error.md/#_nst_set_import_error) that
builds a formatted string with `fmt`.

---

## Structs

### `Nst_SourceText`

**Synopsis:**

```better-c
typedef struct _Nst_SourceText {
    i8 *text;
    i8 *path;
    i8 **lines;
    usize text_len;
    usize lines_len;
} struct _Nst_SourceText
```

**Description:**

The structure where the source text of a Nest file is kept.

**Fields:**

- `text`: the UTF-8 text of the file
- `path`: the path of the file
- `lines`: the beginning of each line of the file
- `len`: the length in bytes of `text`
- `lines_len`: the number of lines in the file

---

### `Nst_Pos`

**Synopsis:**

```better-c
typedef struct _Nst_Pos {
    i32 line;
    i32 col;
    Nst_SourceText *text;
} struct _Nst_Pos
```

**Description:**

The structure representing a position inside a source file.

**Fields:**

- `line`: the line of the position, the first is line 0
- `col`: the column of the position, the first is 0
- `text`: the text this position refers to

---

### `Nst_Error`

**Synopsis:**

```better-c
typedef struct _Nst_Error {
    bool occurred;
    Nst_Pos start;
    Nst_Pos end;
    Nst_StrObj *name;
    Nst_StrObj *message;
} struct _Nst_Error
```

**Description:**

The structure representing an error with a determined position.

**Fields:**

- `occurred`: whether the struct contains a valid error
- `start`: the start position of the error
- `end`: the end position of the error
- `name`: the name of the error (e.g. `Value Error`, `Type Error` etc.)
- `message`: the message of the error

---

### `Nst_OpErr`

**Synopsis:**

```better-c
typedef struct _Nst_OpErr {
    Nst_StrObj *name;
    Nst_StrObj *message;
} struct _Nst_OpErr
```

**Description:**

The structure representing an error occurred during an operation and that does
not yet have a position.

**Fields:**

- `name`: the name of the error (e.g. `Value Error`, `Type Error` etc.)
- `message`: the message of the error

---

### `Nst_Traceback`

**Synopsis:**

```better-c
typedef struct _Nst_Traceback {
    Nst_Error error;
    Nst_LList *positions;
} struct _Nst_Traceback
```

**Description:**

The structure containing the full traceback of the error.

**Fields:**

- `error`: the error of the traceback
- `positions`: the list of positions that led to the error

---

## Functions

### `Nst_set_color`

**Synopsis:**

```better-c
void Nst_set_color(bool color)
```

**Description:**

Sets how the error message is printed (with or without ANSI escapes).

---

### `Nst_copy_pos`

**Synopsis:**

```better-c
Nst_Pos Nst_copy_pos(Nst_Pos pos)
```

**Description:**

Forces a copy of the position.

---

### `Nst_no_pos`

**Synopsis:**

```better-c
Nst_Pos Nst_no_pos(void)
```

**Description:**

Creates an empty position, with no valid text.

---

### `Nst_print_error`

**Synopsis:**

```better-c
void Nst_print_error(Nst_Error err)
```

**Description:**

Prints a formatted [`Nst_Error`](c_api-error.md/#nst_error).

---

### `Nst_print_traceback`

**Synopsis:**

```better-c
void Nst_print_traceback(Nst_Traceback tb)
```

**Description:**

Prints a formatted [`Nst_Traceback`](c_api-error.md/#nst_traceback).

---

### `Nst_free_src_text`

**Synopsis:**

```better-c
void Nst_free_src_text(Nst_SourceText *text)
```

**Description:**

Frees a heap allocated text source, `text` can be `NULL`. No error is set.

---

### `_Nst_set_error`

**Synopsis:**

```better-c
void _Nst_set_error(Nst_StrObj *name, Nst_StrObj *msg)
```

**Description:**

Sets the global operation error with the given name and message.

It takes a reference of both name and message.

---

### `_Nst_set_syntax_error`

**Synopsis:**

```better-c
void _Nst_set_syntax_error(Nst_StrObj *msg)
```

**Description:**

Sets the global operation error with the given message and "Syntax Error" as the
name.

It takes a reference of the message.

---

### `_Nst_set_memory_error`

**Synopsis:**

```better-c
void _Nst_set_memory_error(Nst_StrObj *msg)
```

**Description:**

Sets the global operation error with the given message and "Memory Error" as the
name.

It takes a reference of the message.

---

### `_Nst_set_type_error`

**Synopsis:**

```better-c
void _Nst_set_type_error(Nst_StrObj *msg)
```

**Description:**

Sets the global operation error with the given message and "Type Error" as the
name.

It takes a reference of the message.

---

### `_Nst_set_value_error`

**Synopsis:**

```better-c
void _Nst_set_value_error(Nst_StrObj *msg)
```

**Description:**

Sets the global operation error with the given message and "Value Error" as the
name.

It takes a reference of the message.

---

### `_Nst_set_math_error`

**Synopsis:**

```better-c
void _Nst_set_math_error(Nst_StrObj *msg)
```

**Description:**

Sets the global operation error with the given message and "Math Error" as the
name.

It takes a reference of the message.

---

### `_Nst_set_call_error`

**Synopsis:**

```better-c
void _Nst_set_call_error(Nst_StrObj *msg)
```

**Description:**

Sets the global operation error with the given message and "Call Error" as the
name.

It takes a reference of the message.

---

### `_Nst_set_import_error`

**Synopsis:**

```better-c
void _Nst_set_import_error(Nst_StrObj *msg)
```

**Description:**

Sets the global operation error with the given message and "Import Error" as the
name.

It takes a reference of the message.

---

### `Nst_set_syntax_error_c`

**Synopsis:**

```better-c
void Nst_set_syntax_error_c(const i8 *msg)
```

**Description:**

Sets the global operation error creating a string object from the given message
and using "Syntax Error" as the name.

---

### `Nst_set_memory_error_c`

**Synopsis:**

```better-c
void Nst_set_memory_error_c(const i8 *msg)
```

**Description:**

Sets the global operation error creating a string object from the given message
and using "Memory Error" as the name.

---

### `Nst_set_type_error_c`

**Synopsis:**

```better-c
void Nst_set_type_error_c(const i8 *msg)
```

**Description:**

Sets the global operation error creating a string object from the given message
and using "Type Error" as the name.

---

### `Nst_set_value_error_c`

**Synopsis:**

```better-c
void Nst_set_value_error_c(const i8 *msg)
```

**Description:**

Sets the global operation error creating a string object from the given message
and using "Value Error" as the name.

---

### `Nst_set_math_error_c`

**Synopsis:**

```better-c
void Nst_set_math_error_c(const i8 *msg)
```

**Description:**

Sets the global operation error creating a string object from the given message
and using "Math Error" as the name.

---

### `Nst_set_call_error_c`

**Synopsis:**

```better-c
void Nst_set_call_error_c(const i8 *msg)
```

**Description:**

Sets the global operation error creating a string object from the given message
and using "Call Error" as the name.

---

### `Nst_set_import_error_c`

**Synopsis:**

```better-c
void Nst_set_import_error_c(const i8 *msg)
```

**Description:**

Sets the global operation error creating a string object from the given message
and using "Import Error" as the name.

---

### `Nst_failed_allocation`

**Synopsis:**

```better-c
void Nst_failed_allocation(void)
```

**Description:**

Sets the global operation error with a memory error of failed allocation.

---

### `Nst_error_occurred`

**Synopsis:**

```better-c
bool Nst_error_occurred(void)
```

**Description:**

Returns whether the global operation error is set.

---

### `Nst_error_get`

**Synopsis:**

```better-c
Nst_OpErr *Nst_error_get(void)
```

**Description:**

Returns the global operation error, it does not create a copy.

---

### `Nst_error_clear`

**Synopsis:**

```better-c
void Nst_error_clear(void)
```

**Description:**

Clears the global operation error, even if it is not set.

---

### `Nst_traceback_init`

**Synopsis:**

```better-c
bool Nst_traceback_init(void)
```

**Description:**

Initializes the traceback of the current
[`Nst_state`](c_api-interpreter.md/#nst_get_state).

---

### `Nst_traceback_delete`

**Synopsis:**

```better-c
void Nst_traceback_delete(void)
```

**Description:**

Frees the traceback of the current
[`Nst_state`](c_api-interpreter.md/#nst_get_state).

---

### `Nst_set_internal_error`

**Synopsis:**

```better-c
void Nst_set_internal_error(Nst_Error *error, Nst_Pos start, Nst_Pos end,
                            Nst_StrObj *name, Nst_StrObj *msg)
```

**Description:**

Sets a [`Nst_Error`](c_api-error.md/#nst_error) with the given fields.

`name` will increment the reference count but `msg` will not.

**Parameters:**

- `error`: the error to fill
- `start`: the start position
- `end`: the end position
- `msg`: the message of the error

---

### `Nst_set_internal_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_error_c(Nst_Error *error, Nst_Pos start, Nst_Pos end,
                              Nst_StrObj *name, const i8 *msg)
```

**Description:**

Sets a [`Nst_Error`](c_api-error.md/#nst_error) with the given fields and
creating the message from a C string.

`name` will increment the reference count.

**Parameters:**

- `error`: the error to fill
- `start`: the start position
- `end`: the end position
- `msg`: the message of the error

---

### `Nst_set_internal_syntax_error`

**Synopsis:**

```better-c
void Nst_set_internal_syntax_error(Nst_Error *error, Nst_Pos start,
                                   Nst_Pos end, Nst_StrObj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md/#nst_set_internal_error)
and uses "Syntax Error" as the name.

---

### `Nst_set_internal_memory_error`

**Synopsis:**

```better-c
void Nst_set_internal_memory_error(Nst_Error *error, Nst_Pos start,
                                   Nst_Pos end, Nst_StrObj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md/#nst_set_internal_error)
and uses "Memory Error" as the name.

---

### `Nst_set_internal_type_error`

**Synopsis:**

```better-c
void Nst_set_internal_type_error(Nst_Error *error, Nst_Pos start, Nst_Pos end,
                                 Nst_StrObj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md/#nst_set_internal_error)
and uses "Type Error" as the name.

---

### `Nst_set_internal_value_error`

**Synopsis:**

```better-c
void Nst_set_internal_value_error(Nst_Error *error, Nst_Pos start, Nst_Pos end,
                                  Nst_StrObj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md/#nst_set_internal_error)
and uses "Value Error" as the name.

---

### `Nst_set_internal_math_error`

**Synopsis:**

```better-c
void Nst_set_internal_math_error(Nst_Error *error, Nst_Pos start, Nst_Pos end,
                                 Nst_StrObj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md/#nst_set_internal_error)
and uses "Math Error" as the name.

---

### `Nst_set_internal_call_error`

**Synopsis:**

```better-c
void Nst_set_internal_call_error(Nst_Error *error, Nst_Pos start, Nst_Pos end,
                                 Nst_StrObj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md/#nst_set_internal_error)
and uses "Call Error" as the name.

---

### `Nst_set_internal_import_error`

**Synopsis:**

```better-c
void Nst_set_internal_import_error(Nst_Error *error, Nst_Pos start,
                                   Nst_Pos end, Nst_StrObj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md/#nst_set_internal_error)
and uses "Import Error" as the name.

---

### `Nst_set_internal_syntax_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_syntax_error_c(Nst_Error *error, Nst_Pos start,
                                     Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to
[`Nst_set_internal_error_c`](c_api-error.md/#nst_set_internal_error_c) and uses
"Syntax Error" as the name.

---

### `Nst_set_internal_memory_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_memory_error_c(Nst_Error *error, Nst_Pos start,
                                     Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to
[`Nst_set_internal_error_c`](c_api-error.md/#nst_set_internal_error_c) and uses
"Memory Error" as the name.

---

### `Nst_set_internal_type_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_type_error_c(Nst_Error *error, Nst_Pos start,
                                   Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to
[`Nst_set_internal_error_c`](c_api-error.md/#nst_set_internal_error_c) and uses
"Type Error" as the name.

---

### `Nst_set_internal_value_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_value_error_c(Nst_Error *error, Nst_Pos start,
                                    Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to
[`Nst_set_internal_error_c`](c_api-error.md/#nst_set_internal_error_c) and uses
"Value Error" as the name.

---

### `Nst_set_internal_math_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_math_error_c(Nst_Error *error, Nst_Pos start,
                                   Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to
[`Nst_set_internal_error_c`](c_api-error.md/#nst_set_internal_error_c) and uses
"Math Error" as the name.

---

### `Nst_set_internal_call_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_call_error_c(Nst_Error *error, Nst_Pos start,
                                   Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to
[`Nst_set_internal_error_c`](c_api-error.md/#nst_set_internal_error_c) and uses
"Call Error" as the name.

---

### `Nst_set_internal_import_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_import_error_c(Nst_Error *error, Nst_Pos start,
                                     Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to
[`Nst_set_internal_error_c`](c_api-error.md/#nst_set_internal_error_c) and uses
"Import Error" as the name.

---

### `Nst_internal_failed_allocation`

**Synopsis:**

```better-c
void Nst_internal_failed_allocation(Nst_Error *error, Nst_Pos start,
                                    Nst_Pos end)
```

**Description:**

Sets a memory error for a failed allocation.

---

### `Nst_set_internal_error_from_op_err`

**Synopsis:**

```better-c
void Nst_set_internal_error_from_op_err(Nst_Error *error, Nst_Pos start,
                                        Nst_Pos end)
```

**Description:**

Sets a [`Nst_Error`](c_api-error.md/#nst_error) from the global error and clears
it.

