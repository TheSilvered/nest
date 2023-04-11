# `error_internal.h`

The header containing the macros used internally when dealing with `Nst_Error`.

## Macros

### `_NST_SET_ERROR`

**Synopsis**:

```better-c
_NST_SET_ERROR(error, e_start, e_end, e_name, msg)
```

**Description**:

Sets the error with the given name and message, increasing the reference
count of the name but not of the message.

**Arguments**:

- `error`: the pointer to the error (of type `Nst_Error *`)
- `e_start`: a `Nst_Pos` of the start of the error
- `e_end`: a `Nst_Pos` of the end of the error
- `e_name`: a Nest string object used as the name, its reference count
  is automatically increased
- `msg`: a Nest string object used for the message, its reference count
  must be increased manually

---

### `_NST_SET_RAW_ERROR`

**Synopsis**:

```better-c
_NST_SET_ERROR(error, e_start, e_end, e_name, msg)
```

**Description**:

Sets the error with the given name and creating a new string object for
the message.

**Arguments**:

- `error`: the pointer to the error (of type `Nst_Error *`)
- `e_start`: a `Nst_Pos` of the start of the error
- `e_end`: a `Nst_Pos` of the end of the error
- `e_name`: a Nest string object used as the name, its reference count
  is automatically increased
- `msg`: a NUL-ended `i8 *` that is used to create the message

---

### `_NST_SET_[error_name]_ERROR` macros

**Synopsis**:

```better-c
_NST_SET_[error_name]_ERROR(error, e_start, e_end, msg)
```

**Description**:

The available macros are:

- `_NST_SET_SYNTAX_ERROR`
- `_NST_SET_MEMORY_ERROR`
- `_NST_SET_TYPE_ERROR`
- `_NST_SET_VALUE_ERROR`
- `_NST_SET_MATH_ERROR`
- `_NST_SET_CALL_ERROR`
- `_NST_SET_IMPORT_ERROR`
- `_NST_SET_IMPORT_ERROR`

These macros expand using `_NST_SET_ERROR` passing as `e_name` the
name corresponding to what `[error_name]` corresponds to.

**Arguments**:

- `error`: the pointer to the error (of type `Nst_Error *`)
- `e_start`: a `Nst_Pos` of the start of the error
- `e_end`: a `Nst_Pos` of the end of the error
- `msg`: a Nest string object used for the message, its reference count
  must be increased manually

---

### `_NST_SET_RAW_[error_name]_ERROR` macros

**Synopsis**:

```better-c
_NST_SET_RAW_[error_name]_ERROR(error, e_start, e_end, msg)
```

**Description**:

The available macros are:

- `_NST_SET_SYNTAX_ERROR`
- `_NST_SET_MEMORY_ERROR`
- `_NST_SET_TYPE_ERROR`
- `_NST_SET_VALUE_ERROR`
- `_NST_SET_MATH_ERROR`
- `_NST_SET_CALL_ERROR`
- `_NST_SET_IMPORT_ERROR`
- `_NST_SET_IMPORT_ERROR`

These macros expand using `_NST_SET_RAW_ERROR` passing as `e_name` the
name corresponding to what `[error_name]` corresponds to.

**Arguments**:

- `error`: the pointer to the error (of type `Nst_Error *`)
- `e_start`: a `Nst_Pos` of the start of the error
- `e_end`: a `Nst_Pos` of the end of the error
- `msg`: a NUL-ended `i8 *` that is used to create the message

---

### `_NST_FAILED_ALLOCATION`

**Synopsis**:

```better-c
_NST_FAILED_ALLOCATION(error, e_start, e_end)
```

**Description**:

Sets `error` as a memory error with the message reporting a failed
memory allocation.

---

### `_NST_SET_ERROR_FROM_OP_ERR`

**Synopsis**:

```better-c
_NST_SET_ERROR_FROM_OP_ERR(error, op_err, e_start, e_end)
```

**Description**:

Sets the name and message of the error taking the ones from `op_err`. After this
macro the contents of `op_err` are cleared.

**Arguments**:

- `error`: the pointer to the error (of type `Nst_Error *`)
- `op_err`: the `Nst_OpErr *` from which to take the name and the message
- `e_start`: a `Nst_Pos` of the start of the error
- `e_end`: a `Nst_Pos` of the end of the error

