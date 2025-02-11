# `error.h`

Error management interface.

## Authors

TheSilvered

---

## Macros

### `_Nst_EM_WRONG_ARG_NUM_FMT`

**Synopsis:**

```better-c
#define _Nst_EM_WRONG_ARG_NUM_FMT(func_arg_num, arg_num)
```

**Description:**

Correctly formats the `_Nst_EM_WRONG_ARG_NUM` error message.

---

### `Nst_set_errorf`

**Synopsis:**

```better-c
#define Nst_set_errorf(name, fmt, ...)
```

**Description:**

Alias for [`Nst_set_error`](c_api-error.md#nst_set_error) that builds a
formatted string with `fmt`.

---

### `Nst_set_syntax_errorf`

**Synopsis:**

```better-c
#define Nst_set_syntax_errorf(fmt, ...)
```

**Description:**

Alias for [`Nst_set_syntax_error`](c_api-error.md#nst_set_syntax_error) that
builds a formatted string with `fmt`.

---

### `Nst_set_memory_errorf`

**Synopsis:**

```better-c
#define Nst_set_memory_errorf(fmt, ...)
```

**Description:**

Alias for [`Nst_set_memory_error`](c_api-error.md#nst_set_memory_error) that
builds a formatted string with `fmt`.

---

### `Nst_set_type_errorf`

**Synopsis:**

```better-c
#define Nst_set_type_errorf(fmt, ...)
```

**Description:**

Alias for [`Nst_set_type_error`](c_api-error.md#nst_set_type_error) that builds
a formatted string with `fmt`.

---

### `Nst_set_value_errorf`

**Synopsis:**

```better-c
#define Nst_set_value_errorf(fmt, ...)
```

**Description:**

Alias for [`Nst_set_value_error`](c_api-error.md#nst_set_value_error) that
builds a formatted string with `fmt`.

---

### `Nst_set_math_errorf`

**Synopsis:**

```better-c
#define Nst_set_math_errorf(fmt, ...)
```

**Description:**

Alias for [`Nst_set_math_error`](c_api-error.md#nst_set_math_error) that builds
a formatted string with `fmt`.

---

### `Nst_set_call_errorf`

**Synopsis:**

```better-c
#define Nst_set_call_errorf(fmt, ...)
```

**Description:**

Alias for [`Nst_set_call_error`](c_api-error.md#nst_set_call_error) that builds
a formatted string with `fmt`.

---

### `Nst_set_import_errorf`

**Synopsis:**

```better-c
#define Nst_set_import_errorf(fmt, ...)
```

**Description:**

Alias for [`Nst_set_import_error`](c_api-error.md#nst_set_import_error) that
builds a formatted string with `fmt`.

---

## Structs

### `Nst_SourceText`

**Synopsis:**

```better-c
typedef struct _Nst_SourceText {
    bool allocated;
    i8 *text;
    i8 *path;
    i8 **lines;
    usize text_len;
    usize lines_len;
} Nst_SourceText
```

**Description:**

The structure where the source text of a Nest file is kept.

**Fields:**

- `allocated`: whether `text`, `path` and `lines` are heap allocated
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
} Nst_Pos
```

**Description:**

The structure representing a position inside a source file.

**Fields:**

- `line`: the line of the position, the first is line 0
- `col`: the column of the position, the first is 0
- `text`: the text this position refers to

---

### `Nst_Traceback`

**Synopsis:**

```better-c
typedef volatile struct _Nst_Traceback {
    bool error_occurred;
    Nst_Obj *error_name;
    Nst_Obj *error_msg;
    Nst_LList *positions;
} Nst_Traceback
```

**Description:**

The structure containing the full traceback of the error.

**Fields:**

- `error_occurred`: whether the traceback contains an error
- `error_name`: the name of the error (e.g. `Value Error`, `Type Error` etc.)
- `error_msg`: the message of the error
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

### `Nst_print_traceback`

**Synopsis:**

```better-c
void Nst_print_traceback(Nst_Traceback *tb)
```

**Description:**

Prints a formatted [`Nst_Traceback`](c_api-error.md#nst_traceback).

---

### `Nst_source_text_init`

**Synopsis:**

```better-c
void Nst_source_text_init(Nst_SourceText *src)
```

**Description:**

Initializes the fields of a [`Nst_SourceText`](c_api-error.md#nst_sourcetext)
struct.

---

### `Nst_source_text_destroy`

**Synopsis:**

```better-c
void Nst_source_text_destroy(Nst_SourceText *text)
```

**Description:**

Frees a heap allocated text source, `text` can be `NULL`. No error is set.

---

### `Nst_set_error`

**Synopsis:**

```better-c
void Nst_set_error(Nst_Obj *name, Nst_Obj *msg)
```

**Description:**

Sets the global operation error with the given name and message.

It takes a reference of both name and message.

---

### `Nst_set_syntax_error`

**Synopsis:**

```better-c
void Nst_set_syntax_error(Nst_Obj *msg)
```

**Description:**

Sets the global operation error with the given message and "Syntax Error" as the
name.

It takes a reference of the message.

---

### `Nst_set_memory_error`

**Synopsis:**

```better-c
void Nst_set_memory_error(Nst_Obj *msg)
```

**Description:**

Sets the global operation error with the given message and "Memory Error" as the
name.

It takes a reference of the message.

---

### `Nst_set_type_error`

**Synopsis:**

```better-c
void Nst_set_type_error(Nst_Obj *msg)
```

**Description:**

Sets the global operation error with the given message and "Type Error" as the
name.

It takes a reference of the message.

---

### `Nst_set_value_error`

**Synopsis:**

```better-c
void Nst_set_value_error(Nst_Obj *msg)
```

**Description:**

Sets the global operation error with the given message and "Value Error" as the
name.

It takes a reference of the message.

---

### `Nst_set_math_error`

**Synopsis:**

```better-c
void Nst_set_math_error(Nst_Obj *msg)
```

**Description:**

Sets the global operation error with the given message and "Math Error" as the
name.

It takes a reference of the message.

---

### `Nst_set_call_error`

**Synopsis:**

```better-c
void Nst_set_call_error(Nst_Obj *msg)
```

**Description:**

Sets the global operation error with the given message and "Call Error" as the
name.

It takes a reference of the message.

---

### `Nst_set_import_error`

**Synopsis:**

```better-c
void Nst_set_import_error(Nst_Obj *msg)
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
Nst_ErrorKind Nst_error_occurred(void)
```

**Returns:**

Whether an error has occurred in the current execution. Check
[`Nst_ErrorKind`](c_api-error.md#nst_errorkind) to understand better the return
value.

---

### `Nst_error_get`

**Synopsis:**

```better-c
Nst_Traceback *Nst_error_get(void)
```

**Returns:**

The traceback of the current execution or that of the interpreter if no
execution state is set.

---

### `Nst_error_clear`

**Synopsis:**

```better-c
void Nst_error_clear(void)
```

**Description:**

Clears the traceback of both the current execution and of the interpreter. If no
error has occurred nothing is done.

---

### `Nst_traceback_init`

**Synopsis:**

```better-c
bool Nst_traceback_init(Nst_Traceback *tb)
```

**Description:**

Initializes the traceback of the current
[`Nst_state`](c_api-interpreter.md#nst_state_get).

---

### `Nst_traceback_destroy`

**Synopsis:**

```better-c
void Nst_traceback_destroy(Nst_Traceback *tb)
```

**Description:**

Frees the traceback of the current
[`Nst_state`](c_api-interpreter.md#nst_state_get).

---

### `Nst_error_add_positions`

**Synopsis:**

```better-c
void Nst_error_add_positions(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end)
```

**Description:**

Adds a pair of positions to an error.

---

### `Nst_set_internal_error`

**Synopsis:**

```better-c
void Nst_set_internal_error(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                            Nst_Obj *name, Nst_Obj *msg)
```

**Description:**

Sets a [`Nst_Traceback`](c_api-error.md#nst_traceback) with the given fields.

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
void Nst_set_internal_error_c(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                              Nst_Obj *name, const i8 *msg)
```

**Description:**

Sets a [`Nst_Traceback`](c_api-error.md#nst_traceback) with the given fields and
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
void Nst_set_internal_syntax_error(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, Nst_Obj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md#nst_set_internal_error) and
uses "Syntax Error" as the name.

---

### `Nst_set_internal_memory_error`

**Synopsis:**

```better-c
void Nst_set_internal_memory_error(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, Nst_Obj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md#nst_set_internal_error) and
uses "Memory Error" as the name.

---

### `Nst_set_internal_type_error`

**Synopsis:**

```better-c
void Nst_set_internal_type_error(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                                 Nst_Obj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md#nst_set_internal_error) and
uses "Type Error" as the name.

---

### `Nst_set_internal_value_error`

**Synopsis:**

```better-c
void Nst_set_internal_value_error(Nst_Traceback *tb, Nst_Pos start,
                                  Nst_Pos end, Nst_Obj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md#nst_set_internal_error) and
uses "Value Error" as the name.

---

### `Nst_set_internal_math_error`

**Synopsis:**

```better-c
void Nst_set_internal_math_error(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                                 Nst_Obj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md#nst_set_internal_error) and
uses "Math Error" as the name.

---

### `Nst_set_internal_call_error`

**Synopsis:**

```better-c
void Nst_set_internal_call_error(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                                 Nst_Obj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md#nst_set_internal_error) and
uses "Call Error" as the name.

---

### `Nst_set_internal_import_error`

**Synopsis:**

```better-c
void Nst_set_internal_import_error(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, Nst_Obj *msg)
```

**Description:**

Similar to [`Nst_set_internal_error`](c_api-error.md#nst_set_internal_error) and
uses "Import Error" as the name.

---

### `Nst_set_internal_syntax_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_syntax_error_c(Nst_Traceback *tb, Nst_Pos start,
                                     Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to [`Nst_set_internal_error_c`](c_api-error.md#nst_set_internal_error_c)
and uses "Syntax Error" as the name.

---

### `Nst_set_internal_memory_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_memory_error_c(Nst_Traceback *tb, Nst_Pos start,
                                     Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to [`Nst_set_internal_error_c`](c_api-error.md#nst_set_internal_error_c)
and uses "Memory Error" as the name.

---

### `Nst_set_internal_type_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_type_error_c(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to [`Nst_set_internal_error_c`](c_api-error.md#nst_set_internal_error_c)
and uses "Type Error" as the name.

---

### `Nst_set_internal_value_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_value_error_c(Nst_Traceback *tb, Nst_Pos start,
                                    Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to [`Nst_set_internal_error_c`](c_api-error.md#nst_set_internal_error_c)
and uses "Value Error" as the name.

---

### `Nst_set_internal_math_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_math_error_c(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to [`Nst_set_internal_error_c`](c_api-error.md#nst_set_internal_error_c)
and uses "Math Error" as the name.

---

### `Nst_set_internal_call_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_call_error_c(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to [`Nst_set_internal_error_c`](c_api-error.md#nst_set_internal_error_c)
and uses "Call Error" as the name.

---

### `Nst_set_internal_import_error_c`

**Synopsis:**

```better-c
void Nst_set_internal_import_error_c(Nst_Traceback *tb, Nst_Pos start,
                                     Nst_Pos end, const i8 *msg)
```

**Description:**

Similar to [`Nst_set_internal_error_c`](c_api-error.md#nst_set_internal_error_c)
and uses "Import Error" as the name.

---

### `Nst_internal_failed_allocation`

**Synopsis:**

```better-c
void Nst_internal_failed_allocation(Nst_Traceback *tb, Nst_Pos start,
                                    Nst_Pos end)
```

**Description:**

Sets a memory error for a failed allocation.

---

## Enums

### `Nst_ErrorKind`

**Synopsis:**

```better-c
typedef enum _Nst_ErrorKind {
    Nst_EK_NONE = 0,
    Nst_EK_LOCAL,
    Nst_EK_GLOBAL
} Nst_ErrorKind
```

**Description:**

The kinds of errors that can occur during execution.

**Variants:**

- `Nst_EK_NONE`: no error occurred
- `Nst_EK_LOCAL`: an error has occurred in the current execution state
- `Nst_EK_GLOBAL`: an error has occurred while no execution state was set
