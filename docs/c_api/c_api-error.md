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

Correctly formats the error message for using the wrong number of arguments.

---

## Structs

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
    Nst_ObjRef *error_name;
    Nst_ObjRef *error_msg;
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

Set how the error message is printed (with or without ANSI color escapes).

---

### `Nst_pos_empty`

**Synopsis:**

```better-c
Nst_Pos Nst_pos_empty(void)
```

**Description:**

Create an empty position with no valid text.

---

### `Nst_span_new`

**Synopsis:**

```better-c
Nst_Span Nst_span_new(Nst_Pos start, Nst_Pos end)
```

**Description:**

Make a new span from a start and an end position.

---

### `Nst_span_from_pos`

**Synopsis:**

```better-c
Nst_Span Nst_span_from_pos(Nst_Pos pos)
```

**Description:**

Make a new span with the same start and end positions.

---

### `Nst_span_empty`

**Synopsis:**

```better-c
Nst_Span Nst_span_empty(void)
```

**Description:**

Create an empty span with no valid text.

---

### `Nst_span_join`

**Synopsis:**

```better-c
Nst_Span Nst_span_join(Nst_Span span1, Nst_Span span2)
```

**Description:**

Make a span that includes both `span1` and `span2`.

---

### `Nst_span_expand`

**Synopsis:**

```better-c
Nst_Span Nst_span_expand(Nst_Span span, Nst_Pos pos)
```

**Description:**

Expand a span to include the given position.

---

### `Nst_span_start`

**Synopsis:**

```better-c
Nst_Pos Nst_span_start(Nst_Span span)
```

**Returns:**

The start position of a span.

---

### `Nst_span_end`

**Synopsis:**

```better-c
Nst_Pos Nst_span_end(Nst_Span span)
```

**Returns:**

The end position of a span.

---

### `Nst_error_print`

**Synopsis:**

```better-c
void Nst_error_print(void)
```

**Description:**

Print the error traceback.

---

### `Nst_error_set`

**Synopsis:**

```better-c
void Nst_error_set(Nst_ObjRef *name, Nst_ObjRef *msg)
```

**Description:**

Set the error with the given name and message.

It takes one reference from both `name` and `msg`.

---

### `Nst_error_set_syntax`

**Synopsis:**

```better-c
void Nst_error_set_syntax(Nst_ObjRef *msg)
```

**Description:**

Set the error with the given message and "Syntax Error" as the name.

It takes one reference from `msg`.

---

### `Nst_error_set_memory`

**Synopsis:**

```better-c
void Nst_error_set_memory(Nst_ObjRef *msg)
```

**Description:**

Set the error with the given message and "Memory Error" as the name.

It takes one reference from `msg`.

---

### `Nst_error_set_type`

**Synopsis:**

```better-c
void Nst_error_set_type(Nst_ObjRef *msg)
```

**Description:**

Set the error with the given message and "Type Error" as the name.

It takes one reference from `msg`.

---

### `Nst_error_set_value`

**Synopsis:**

```better-c
void Nst_error_set_value(Nst_ObjRef *msg)
```

**Description:**

Set the error with the given message and "Value Error" as the name.

It takes one reference from `msg`.

---

### `Nst_error_set_math`

**Synopsis:**

```better-c
void Nst_error_set_math(Nst_ObjRef *msg)
```

**Description:**

Set the error with the given message and "Math Error" as the name.

It takes one reference from `msg`.

---

### `Nst_error_set_call`

**Synopsis:**

```better-c
void Nst_error_set_call(Nst_ObjRef *msg)
```

**Description:**

Set the error with the given message and "Call Error" as the name.

It takes one reference from `msg`.

---

### `Nst_error_set_import`

**Synopsis:**

```better-c
void Nst_error_set_import(Nst_ObjRef *msg)
```

**Description:**

Set the error with the given message and "Import Error" as the name.

It takes one reference from `msg`.

---

### `Nst_error_setc_syntax`

**Synopsis:**

```better-c
void Nst_error_setc_syntax(const char *msg)
```

**Description:**

Set the error creating a `String` object from `msg` and using "Syntax Error" as
the name.

---

### `Nst_error_setc_memory`

**Synopsis:**

```better-c
void Nst_error_setc_memory(const char *msg)
```

**Description:**

Set the error creating a `String` object from `msg` and using "Memory Error" as
the name.

---

### `Nst_error_setc_type`

**Synopsis:**

```better-c
void Nst_error_setc_type(const char *msg)
```

**Description:**

Set the error creating a `String` object from `msg` and using "Type Error" as
the name.

---

### `Nst_error_setc_value`

**Synopsis:**

```better-c
void Nst_error_setc_value(const char *msg)
```

**Description:**

Set the error creating a `String` object from `msg` and using "Value Error" as
the name.

---

### `Nst_error_setc_math`

**Synopsis:**

```better-c
void Nst_error_setc_math(const char *msg)
```

**Description:**

Set the error creating a `String` object from `msg` and using "Math Error" as
the name.

---

### `Nst_error_setc_call`

**Synopsis:**

```better-c
void Nst_error_setc_call(const char *msg)
```

**Description:**

Set the error creating a `String` object from `msg` and using "Call Error" as
the name.

---

### `Nst_error_setc_import`

**Synopsis:**

```better-c
void Nst_error_setc_import(const char *msg)
```

**Description:**

Set the error creating a `String` object from `msg` and using "Import Error" as
the name.

---

### `Nst_error_setf_syntax`

**Synopsis:**

```better-c
void Nst_error_setf_syntax(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Syntax Error" as
the name.

---

### `Nst_error_setf_memory`

**Synopsis:**

```better-c
void Nst_error_setf_memory(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Memory Error" as
the name.

---

### `Nst_error_setf_type`

**Synopsis:**

```better-c
void Nst_error_setf_type(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Type Error" as the
name.

---

### `Nst_error_setf_value`

**Synopsis:**

```better-c
void Nst_error_setf_value(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Value Error" as
the name.

---

### `Nst_error_setf_math`

**Synopsis:**

```better-c
void Nst_error_setf_math(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Math Error" as the
name.

---

### `Nst_error_setf_call`

**Synopsis:**

```better-c
void Nst_error_setf_call(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Call Error" as the
name.

---

### `Nst_error_setf_import`

**Synopsis:**

```better-c
void Nst_error_setf_import(const char *fmt, ...)
```

**Description:**

Set the error creating a formatted `String` object and using "Import Error" as
the name.

---

### `Nst_error_failed_alloc`

**Synopsis:**

```better-c
void Nst_error_failed_alloc(void)
```

**Description:**

Set the global operation error with a memory error of failed allocation.

---

### `Nst_error_add_span`

**Synopsis:**

```better-c
void Nst_error_add_span(Nst_Span span)
```

**Description:**

Add a pair of positions to the error.

---

### `Nst_error_occurred`

**Synopsis:**

```better-c
bool Nst_error_occurred(void)
```

**Returns:**

Whether an error has occurred.

---

### `Nst_error_get`

**Synopsis:**

```better-c
Nst_Traceback *Nst_error_get(void)
```

**Returns:**

The traceback.

---

### `Nst_error_clear`

**Synopsis:**

```better-c
void Nst_error_clear(void)
```

**Description:**

Clear the traceback. If no error has occurred nothing is done.
