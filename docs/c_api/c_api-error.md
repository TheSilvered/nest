# `error.h`

Error management interface.

## Authors

TheSilvered

---

## Macros

### `_Nst_WRONG_ARG_NUM`

**Synopsis:**

```better-c
#define _Nst_WRONG_ARG_NUM(func_arg_num, arg_num)
```

**Description:**

Correctly formats the error message for the wrong number of arguments.

---

## Structs

### `Nst_SourceText`

**Synopsis:**

```better-c
typedef struct _Nst_SourceText {
    bool allocated;
    char *text;
    char *path;
    char **lines;
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

### `Nst_Span`

**Synopsis:**

```better-c
typedef struct _Nst_Span {
    i32 start_line;
    i32 start_col;
    i32 end_line;
    i32 end_col;
    Nst_SourceText *text;
} Nst_Span
```

**Description:**

The structure representing a text span inside a source file.

**Fields:**

- `start_line`: the starting line, the first is line 0
- `start_col`: the starting column, the first is column 0
- `end_line`: the ending line, included in the span
- `end_col`: the ending column, included in the span
- `text`: the text this position refers to

---

### `Nst_Traceback`

**Synopsis:**

```better-c
typedef struct _Nst_Traceback {
    bool error_occurred;
    Nst_Obj *error_name;
    Nst_Obj *error_msg;
    Nst_DynArray positions;
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

### `Nst_error_set_color`

**Synopsis:**

```better-c
void Nst_error_set_color(bool color)
```

**Description:**

Sets how the error message is printed (with or without ANSI escapes).

---

### `Nst_pos_empty`

**Synopsis:**

```better-c
Nst_Pos Nst_pos_empty(void)
```

**Description:**

Creates an empty position with no valid text.

---

### `Nst_span_new`

**Synopsis:**

```better-c
Nst_Span Nst_span_new(Nst_Pos start, Nst_Pos end)
```

**Description:**

Make a new span from a start and an end posiiton.

---

### `Nst_span_from_pos`

**Synopsis:**

```better-c
Nst_Span Nst_span_from_pos(Nst_Pos pos)
```

**Description:**

Make a new span with the same start and end.

---

### `Nst_span_empty`

**Synopsis:**

```better-c
Nst_Span Nst_span_empty(void)
```

**Description:**

Creates an empty span with no valid text.

---

### `Nst_span_join`

**Synopsis:**

```better-c
Nst_Span Nst_span_join(Nst_Span span1, Nst_Span span2)
```

**Description:**

Make a span that includes both.

---

### `Nst_span_expand`

**Synopsis:**

```better-c
Nst_Span Nst_span_expand(Nst_Span span, Nst_Pos pos)
```

**Description:**

Expands a span to include the given position.

---

### `Nst_span_start`

**Synopsis:**

```better-c
Nst_Pos Nst_span_start(Nst_Span span)
```

**Description:**

Get the start position of a span.

---

### `Nst_span_end`

**Synopsis:**

```better-c
Nst_Pos Nst_span_end(Nst_Span span)
```

**Description:**

Get the end position of a span.

---

### `Nst_tb_init`

**Synopsis:**

```better-c
void Nst_tb_init(Nst_Traceback *tb)
```

**Description:**

Initializes the traceback of the current
[`Nst_state`](c_api-interpreter.md#nst_state_get).

---

### `Nst_tb_destroy`

**Synopsis:**

```better-c
void Nst_tb_destroy(Nst_Traceback *tb)
```

**Description:**

Frees the traceback of the current
[`Nst_state`](c_api-interpreter.md#nst_state_get).

---

### `Nst_tb_add_span`

**Synopsis:**

```better-c
void Nst_tb_add_span(Nst_Traceback *tb, Nst_Span span)
```

**Description:**

Adds a pair of positions to an [`Nst_Traceback`](c_api-error.md#nst_traceback).

---

### `Nst_tb_print`

**Synopsis:**

```better-c
void Nst_tb_print(Nst_Traceback *tb)
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

### `Nst_error_set`

**Synopsis:**

```better-c
void Nst_error_set(Nst_Obj *name, Nst_Obj *msg)
```

**Description:**

Sets the error with the given name and message.

It takes a reference from both `name` and `msg`.

---

### `Nst_error_set_syntax`

**Synopsis:**

```better-c
void Nst_error_set_syntax(Nst_Obj *msg)
```

**Description:**

Sets the error with the given message and "Syntax Error" as the name.

It takes a reference from `msg`.

---

### `Nst_error_set_memory`

**Synopsis:**

```better-c
void Nst_error_set_memory(Nst_Obj *msg)
```

**Description:**

Sets the error with the given message and "Memory Error" as the name.

It takes a reference from `msg`.

---

### `Nst_error_set_type`

**Synopsis:**

```better-c
void Nst_error_set_type(Nst_Obj *msg)
```

**Description:**

Sets the error with the given message and "Type Error" as the name.

It takes a reference from `msg`.

---

### `Nst_error_set_value`

**Synopsis:**

```better-c
void Nst_error_set_value(Nst_Obj *msg)
```

**Description:**

Sets the error with the given message and "Value Error" as the name.

It takes a reference from `msg`.

---

### `Nst_error_set_math`

**Synopsis:**

```better-c
void Nst_error_set_math(Nst_Obj *msg)
```

**Description:**

Sets the error with the given message and "Math Error" as the name.

It takes a reference from `msg`.

---

### `Nst_error_set_call`

**Synopsis:**

```better-c
void Nst_error_set_call(Nst_Obj *msg)
```

**Description:**

Sets the error with the given message and "Call Error" as the name.

It takes a reference from `msg`.

---

### `Nst_error_set_import`

**Synopsis:**

```better-c
void Nst_error_set_import(Nst_Obj *msg)
```

**Description:**

Sets the error with the given message and "Import Error" as the name.

It takes a reference from `msg`.

---

### `Nst_error_setc_syntax`

**Synopsis:**

```better-c
void Nst_error_setc_syntax(const char *msg)
```

**Description:**

Sets the error creating a `String` object from `msg` and using "Syntax Error" as
the name.

---

### `Nst_error_setc_memory`

**Synopsis:**

```better-c
void Nst_error_setc_memory(const char *msg)
```

**Description:**

Sets the error creating a `String` object from `msg` and using "Memory Error" as
the name.

---

### `Nst_error_setc_type`

**Synopsis:**

```better-c
void Nst_error_setc_type(const char *msg)
```

**Description:**

Sets the error creating a `String` object from `msg` and using "Type Error" as
the name.

---

### `Nst_error_setc_value`

**Synopsis:**

```better-c
void Nst_error_setc_value(const char *msg)
```

**Description:**

Sets the error creating a `String` object from `msg` and using "Value Error" as
the name.

---

### `Nst_error_setc_math`

**Synopsis:**

```better-c
void Nst_error_setc_math(const char *msg)
```

**Description:**

Sets the error creating a `String` object from `msg` and using "Math Error" as
the name.

---

### `Nst_error_setc_call`

**Synopsis:**

```better-c
void Nst_error_setc_call(const char *msg)
```

**Description:**

Sets the error creating a `String` object from `msg` and using "Call Error" as
the name.

---

### `Nst_error_setc_import`

**Synopsis:**

```better-c
void Nst_error_setc_import(const char *msg)
```

**Description:**

Sets the error creating a `String` object from `msg` and using "Import Error" as
the name.

---

### `Nst_error_setf_syntax`

**Synopsis:**

```better-c
void Nst_error_setf_syntax(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Syntax Error" as
the name

---

### `Nst_error_setf_memory`

**Synopsis:**

```better-c
void Nst_error_setf_memory(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Memory Error" as
the name

---

### `Nst_error_setf_type`

**Synopsis:**

```better-c
void Nst_error_setf_type(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Type Error" as the
name

---

### `Nst_error_setf_value`

**Synopsis:**

```better-c
void Nst_error_setf_value(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Value Error" as
the name

---

### `Nst_error_setf_math`

**Synopsis:**

```better-c
void Nst_error_setf_math(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Math Error" as the
name

---

### `Nst_error_setf_call`

**Synopsis:**

```better-c
void Nst_error_setf_call(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Call Error" as the
name

---

### `Nst_error_setf_import`

**Synopsis:**

```better-c
void Nst_error_setf_import(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Import Error" as
the name

---

### `Nst_error_failed_alloc`

**Synopsis:**

```better-c
void Nst_error_failed_alloc(void)
```

**Description:**

Sets the global operation error with a memory error of failed allocation.

---

### `Nst_error_add_span`

**Synopsis:**

```better-c
void Nst_error_add_span(Nst_Span span)
```

**Description:**

Adds a pair of positions to the current error.

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
