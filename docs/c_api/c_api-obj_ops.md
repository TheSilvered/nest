# `obj_ops.h`

Various object operations.

## Authors

TheSilvered

---

## Functions

### `Nst_obj_eq_c`

**Synopsis:**

```better-c
bool Nst_obj_eq_c(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Calls [`Nst_obj_eq`](c_api-obj_ops.md#nst_obj_eq) making the result a C bool.

---

### `Nst_obj_ne_c`

**Synopsis:**

```better-c
bool Nst_obj_ne_c(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Calls [`Nst_obj_ne`](c_api-obj_ops.md#nst_obj_ne) making the result a C bool.

---

### `Nst_obj_eq`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `==` operator. Is guaranteed to not fail.

---

### `Nst_obj_ne`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `!=` operator. Is guaranteed to not fail.

---

### `Nst_obj_gt`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `>` operator. On failure the error is set.

---

### `Nst_obj_lt`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `<` operator. On failure the error is set.

---

### `Nst_obj_ge`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `>=` operator. On failure the error is set.

---

### `Nst_obj_le`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `<=` operator. On failure the error is set.

---

### `Nst_obj_add`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `+` operator. On failure the error is set.

---

### `Nst_obj_sub`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `-` operator. On failure the error is set.

---

### `Nst_obj_mul`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `null` operator. On failure the error is set.

---

### `Nst_obj_div`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `/` operator. On failure the error is set.

---

### `Nst_obj_pow`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `^` operator. On failure the error is set.

---

### `Nst_obj_mod`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `%` operator. On failure the error is set.

---

### `Nst_obj_bwor`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `|` operator. On failure the error is set.

---

### `Nst_obj_bwand`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `&` operator. On failure the error is set.

---

### `Nst_obj_bwxor`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `^^` operator. On failure the error is set.

---

### `Nst_obj_bwls`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `<<` operator. On failure the error is set.

---

### `Nst_obj_bwrs`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `>>` operator. On failure the error is set.

---

### `Nst_obj_lgor`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `||` operator. On failure the error is set.

---

### `Nst_obj_lgand`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `&&` operator. On failure the error is set.

---

### `Nst_obj_lgxor`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `&|` operator. On failure the error is set.

---

### `Nst_obj_neg`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_neg(Nst_Obj *ob)
```

**Description:**

Implements the `-:` operator. On failure the error is set.

---

### `Nst_obj_len`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_len(Nst_Obj *ob)
```

**Description:**

Implements the `$` operator. On failure the error is set.

---

### `Nst_obj_bwnot`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_bwnot(Nst_Obj *ob)
```

**Description:**

Implements the `~` operator. On failure the error is set.

---

### `Nst_obj_lgnot`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_lgnot(Nst_Obj *ob)
```

**Description:**

Implements the `!` operator. On failure the error is set.

---

### `Nst_obj_stdout`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_stdout(Nst_Obj *ob)
```

**Description:**

Implements the `>>>` operator. On failure the error is set.

---

### `Nst_obj_stdin`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_stdin(Nst_Obj *ob)
```

**Description:**

Implements the `<<<` operator. On failure the error is set.

---

### `Nst_obj_typeof`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_typeof(Nst_Obj *ob)
```

**Description:**

Implements the `?::` operator. On failure the error is set.

---

### `Nst_obj_import`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_import(Nst_Obj *ob)
```

**Description:**

Implements the `|#|` operator. On failure the error is set.

---

### `_Nst_repr_str_cast`

**Synopsis:**

```better-c
Nst_Obj *_Nst_repr_str_cast(Nst_Obj *ob)
```

**Description:**

Casts an object to its string representation.

All objects can be casted, even custom types, but the function can still fail to
allocate memory.

**Parameters:**

- `ob`: the object to be casted

**Returns:**

The new string or `NULL` on failure. The error is set.

---

### `Nst_obj_cast`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_cast(Nst_Obj *ob, Nst_Obj *type)
```

**Description:**

Casts an object from a type to another.

Casting an object to `Bool` is guaranteed to not fail. All objects can be casted
to `Str` but the function can still fail if a memory allocation is unsuccessful.

**Parameters:**

- `ob`: the object to be casted
- `type`: the type to cast the object to

**Returns:**

The casted object or `NULL` on failure. The error is set.

---

### `Nst_obj_contains`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_contains(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Implements the `<.>` operator. On failure the error is set.

---

### `Nst_obj_concat`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2)
```

**Description:**

Concatenates two objects into a string.

Both objects are casted to `Str` before being concatenated.

**Parameters:**

- `ob1`: the first object
- `ob2`: the second object

**Returns:**

The result of the concatenation or `NULL` on failure. The error is set.

---

### `Nst_obj_range`

**Synopsis:**

```better-c
Nst_Obj *Nst_obj_range(Nst_Obj *start, Nst_Obj *stop, Nst_Obj *step)
```

**Description:**

Creates a range object given the start, stop and step.

**Parameters:**

- `start`: the starting value of the range
- `stop`: the the excluded stopping index of the range
- `step`: the step of the range

**Returns:**

The new range object or `NULL` on failure. The error is set.

---

### `_Nst_get_import_path`

**Synopsis:**

```better-c
Nst_Obj *_Nst_get_import_path(const char *initial_path, usize path_len)
```

**Description:**

Returns the absolute path for a library to import.

If the library is not found on the given path, the standard library directory is
checked.

**Parameters:**

- `initial_path`: the relative path used to import the library
- `path_len`: the length in bytes of `initial_path`

**Returns:**

The path on success and `NULL` on failure. The error is set. This function fails
if the specified library is not found.
