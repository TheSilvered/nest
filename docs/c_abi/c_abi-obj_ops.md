# `obj_ops.h`

This header contains the functions that implement the operators in Nest.

## Macros

### Binary operator aliases

**Synopsis**:

```better-c
nst_obj_eq(ob1, ob2, err)
nst_obj_ne(ob1, ob2, err)
nst_obj_gt(ob1, ob2, err)
nst_obj_lt(ob1, ob2, err)
nst_obj_ge(ob1, ob2, err)
nst_obj_le(ob1, ob2, err)
nst_obj_add(ob1, ob2, err)
nst_obj_sub(ob1, ob2, err)
nst_obj_mul(ob1, ob2, err)
nst_obj_div(ob1, ob2, err)
nst_obj_pow(ob1, ob2, err)
nst_obj_mod(ob1, ob2, err)
nst_obj_bwor(ob1, ob2, err)
nst_obj_bwand(ob1, ob2, err)
nst_obj_bwxor(ob1, ob2, err)
nst_obj_bwls(ob1, ob2, err)
nst_obj_bwrs(ob1, ob2, err)
nst_obj_lgor(ob1, ob2, err)
nst_obj_lgand(ob1, ob2, err)
nst_obj_lgxor(ob1, ob2, err)
nst_obj_concat(ob1, ob2, err)
```

**Description**:

Aliases for the function of the same name that starts with an underscore, that
cast `ob1` and `ob2` to `Nst_Obj *`.

---

### Unary operator aliases

```better-c
nst_obj_neg(ob, err)
nst_obj_len(ob, err)
nst_obj_bwnot(ob, err)
nst_obj_lgnot(ob, err)
nst_obj_stdout(ob, err)
nst_obj_stdin(ob, err)
nst_obj_typeof(ob, err)
nst_obj_import(ob, err)
```

**Description**:

Aliases for the function of the same name that starts with an underscore, that
cast `ob` to `Nst_Obj *`.

---

### `nst_obj_cast`

```better-c
nst_obj_cast(ob, type, err)
```

**Description**:

Alias for [`_nst_obj_cast`](#_nst_obj_cast) that casts `ob` to `Nst_Obj *` and
`type` to `Nst_TypeObj *`.

---

### `nst_obj_range`

```better-c
nst_obj_range(ob1, ob2, ob3, err)
```

**Description**:

Alias for [`_nst_obj_range`](#_nst_obj_range) that casts `ob1`, `ob2` and `ob3`
to `Nst_Obj *`.

---

## Functions

### Binary operator function

**Synopsis**:

```better-c
// Comparisons

EXPORT Nst_Obj *_nst_obj_eq(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_ne(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_gt(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_lt(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_ge(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_le(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)

// Arithmetic operations

EXPORT Nst_Obj *_nst_obj_add(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_sub(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_mul(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_div(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_pow(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_mod(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)

// Bitwise operations

EXPORT Nst_Obj *_nst_obj_bwor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_bwand(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_bwxor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_bwls(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_bwrs(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)

// Logical operations

EXPORT Nst_Obj *_nst_obj_lgor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_lgand(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_lgxor(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)

// Other

EXPORT Nst_Obj *_nst_obj_concat(Nst_Obj *ob1, Nst_Obj *ob2, Nst_OpErr *err)
```

**Description**:

All of these functions execute a certain Nest binary operator:

- `eq`: `==`
- `ne`: `!=`
- `gt`: `>`
- `lt`: `<`
- `ge`: `>=`
- `le`: `<=`
- `add`: `+`
- `sub`: `-`
- `mul`: `*`
- `div`: `/`
- `pow`: `^`
- `mod`: `%`
- `bwor`: `|`
- `bwand`: `&`
- `bwxor`: `^^`
- `bwls`: `<<`
- `bwrs`: `>>`
- `lgor`: `||`
- `lgand`: `&&`
- `lgxor`: `&|`

**Arguments**:

- `[in] ob1`: the first object of the operator
- `[in] ob2`: the second object of the operator
- `[out] err`: the second object of the operator

**Return value**:

The function returns the result of the operation or `NULL` if an error occurs.

---

### Unary operator functions

**Synopsis**:

```better-c
EXPORT Nst_Obj *_nst_obj_neg(Nst_Obj *ob, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_len(Nst_Obj *ob, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_bwnot(Nst_Obj *ob, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_lgnot(Nst_Obj *ob, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_stdout(Nst_Obj *ob, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_stdin(Nst_Obj *ob, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_typeof(Nst_Obj *ob, Nst_OpErr *err)
EXPORT Nst_Obj *_nst_obj_import(Nst_Obj *ob, Nst_OpErr *err)
```

**Description**:

All of these functions execute a certain Nest unary operator:

- `neg`: `-:`
- `len`: `$`
- `bwnot`: `~`
- `lgnot`: `!`
- `stdout`: `>>>`
- `stdin`: `<<<`
- `typeof`: `?::`
- `import`: `|#|`

**Arguments**:

- `[in] ob1`: the object to operate on
- `[out] err`: the second object of the operator

**Return value**:

The function returns the result of the operation or `NULL` if an error occurs.

---

### `_nst_obj_range`

**Synopsis**:

```better-c
Nst_Obj *_nst_obj_range(Nst_Obj   *start,
                        Nst_Obj   *stop,
                        Nst_Obj   *step,
                        Nst_OpErr *err)
```

**Description**:

Implements the built-in range operator `->`.

**Arguments**:

- `[in] start`: the number to start from
- `[in] stop`: the number to stop at, not inclusive
- `[in] step`: the size of the step
- `[out] err`: the error

**Return value**:

The function returns the iterator or `NULL` if an error occurs.

---

### `_nst_obj_cast`

**Synopsis**:

```better-c
Nst_Obj *_nst_obj_cast(Nst_Obj *ob, Nst_TypeObj *type, Nst_OpErr *err)
```

**Description**:

Casts `ob` to `type`. The cast to `Bool` is always guaranteed to succeed, the
one to `Str` accepts any type of object but could fail because of memory
allocations.

Casts to the same type of the object return the object itself increasing the
reference count.

**Arguments**:

- `[in] ob`: the object to cast to another type
- `[in] type`: the type to cast the object to
- `[out] err`: the error

**Return value**:

The function returns the casted object or `NULL` if an error occurs.

---

### `_nst_repr_str_cast`

**Synopsis**:

```better-c
Nst_Obj *_nst_repr_str_cast(Nst_Obj *ob, Nst_OpErr *err)
```

**Description**:

Casts `ob` to a printable representation of its value. The only times this is
different from `nst_obj_cast(ob, nst_type()->Str, err)` is with strings and
bytes.

**Arguments**:

- `[in] ob`: the object to cast to a string
- `[out] err`: the error

**Return value**:

The function returns the new string or `NULL` on failure.

---

### `_nst_obj_str_cast_seq`

**Synopsis**:

```better-c
Nst_Obj *_nst_obj_str_cast_seq(Nst_SeqObj *seq_obj,
                               Nst_LList  *all_objs,
                               Nst_OpErr  *err)
```

**Description**:

Casts a sequence to a string.

**Arguments**:

- `[in] seq_obj`: the sequence to cast
- `[inout] all_objs`: the "container stack" that lists all the containers that
  contain the one being casted, to prevent infinite recursion
- `[out] err`: the error

**Return value**:

The function returns the new string or `NULL` if an error occurred.

---

### `_nst_obj_str_cast_map`

**Synopsis**:

```better-c
Nst_Obj *_nst_obj_str_cast_map(Nst_MapObj *map_obj,
                               Nst_LList  *all_objs,
                               Nst_OpErr  *err)
```

**Description**:

Casts a map to a string.

**Arguments**:

- `[in] map_obj`: the map to cast
- `[inout] all_objs`: the "container stack" that lists all the containers that
  contain the one being casted, to prevent infinite recursion
- `[out] err`: the error

**Return value**:

The function returns the new string or `NULL` if an error occurred.

---

### `_nst_get_import_path`

**Synopsis**:

```better-c
Nst_StrObj *_nst_get_import_path(i8        *initial_path,
                                 usize      path_len,
                                 Nst_OpErr *err)
```

**Description**:

Creates the absolute path to a file for importing, if a local file has the same
name as one from the standard library, the local one is imported.

**Arguments**:

- `[in] initial_path`: the relative path to the file or the name of the file in
  the standard library
- `[in] path_len`: the length of `initial_path`
- `[out] err`: the error

**Return value**:

The function returns the absolute path to the module being imported or `NULL` if
the file is not found or an error occurs.
