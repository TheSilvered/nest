# `iter.h`

This header contains

## Macros

### `ITER`

**Synopsis**:

```better-c
ITER(ptr)
```

**Description**:

Casts `ptr` to a [`Nst_IterObj *`](#nst_iterobj).

---

### `nst_iter_start`

**Synopsis**:

```better-c
nst_iter_start(iter, err)
```

**Description**:

Alias for [`_nst_iter_start`](#_nst_iter_start) that casts `iter` to a
[`Nst_IterObj *`](#nst_iterobj) implicitly.

---

### `nst_iter_is_done`

**Synopsis**:

```better-c
nst_iter_is_done(iter, err)
```

**Description**:

Alias for [`_nst_iter_is_done`](#_nst_iter_is_done) that casts `iter` to a
[`Nst_IterObj *`](#nst_iterobj) implicitly.

---

### `nst_iter_get_val`

**Synopsis**:

```better-c
nst_iter_get_val(iter, err)
```

**Description**:

Alias for [`_nst_iter_get_val`](#_nst_iter_get_val) that casts `iter` to a
[`Nst_IterObj *`](#nst_iterobj) implicitly.

---

## Structs

### `Nst_IterObj`

**Synopsis**:

```better-c
typedef struct _Nst_IterObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    Nst_FuncObj *start;
    Nst_FuncObj *is_done;
    Nst_FuncObj *get_val;
    Nst_Obj *value;
}
Nst_IterObj
```

**Description**:

The structure that defines a Nest iterator object.

**Fields**:

- `start`: the function called when the iterator starts
- `is_done`: the function called before each iteration to check if it has ended
- `get_val`: the function called before each iteration to get the value to be
  assigned to the variable
- `value`: the object passed to `start`, `is_done` and `get_val`

---

## Functions

### `nst_iter_new`

**Synopsis**:

```better-c
Nst_Obj *nst_iter_new(Nst_FuncObj *start,
                      Nst_FuncObj *is_done,
                      Nst_FuncObj *get_val,
                      Nst_Obj     *value,
                      Nst_OpErr   *err)
```

**Description**:

Creates a new iterator. The references to the arguments must be increased
manually.

**Arguments**:

- `[in] start`: the `_start_` function of the iterator
- `[in] is_done`: the `_is_done_` function of the iterator
- `[in] get_val`: the `_get_val_` function of the iterator
- `[in] value`: the `value` of the iterator
- `[out] err`: the error

**Return value**:

The function returns the new iterator or `NULL` in case of failure.

---

### `_nst_iter_start`

**Synopsis**:

```better-c
i32 _nst_iter_start(Nst_IterObj *iter, Nst_OpErr *err)
```

**Description**:

Executes the `_start_` function of `iter`.

**Arguments**:

- `iter`: the iter of which function to use
- `err`: the error

**Return value**:

The function returns `0` on success and `-1` on failure.

---

### `_nst_iter_is_done`

**Synopsis**:

```better-c
i32 _nst_iter_is_done(Nst_IterObj *iter, Nst_OpErr *err)
```

**Description**:

Executes the `_is_done_` function of `iter`.

**Arguments**:

- `iter`: the iter of which function to use
- `err`: the error

**Return value**:

The function returns `0` if `_is_done_` returned false, `1` it the function
returned true and `-1` on failure.

---

### `_nst_iter_get_val`

**Synopsis**:

```better-c
Nst_Obj * _nst_iter_get_val(Nst_IterObj *iter, Nst_OpErr *err)
```

**Description**:

Executes the `_get_val_` function of `iter`.

**Arguments**:

- `iter`: the iter of which function to use
- `err`: the error

**Return value**:

The function returns the object returned by `_get_val_` on success and `NULL`
on failure.

---

### Other functions

**Synopsis**:

```better-c
// Functions for the iter object

void _nst_iter_destroy(Nst_IterObj *iter);
void _nst_iter_traverse(Nst_IterObj *iter);
void _nst_iter_track(Nst_IterObj *iter);

// Functions for the range iterator

NST_FUNC_SIGN(nst_iter_range_start);
NST_FUNC_SIGN(nst_iter_range_is_done);
NST_FUNC_SIGN(nst_iter_range_get_val);

// Functions for the sequence iterator

NST_FUNC_SIGN(nst_iter_seq_start);
NST_FUNC_SIGN(nst_iter_seq_is_done);
NST_FUNC_SIGN(nst_iter_seq_get_val);

// Functions for the string iterator

NST_FUNC_SIGN(nst_iter_str_start);
NST_FUNC_SIGN(nst_iter_str_is_done);
NST_FUNC_SIGN(nst_iter_str_get_val);
```

**Description**:

These functions are used internally by Nest to create the built-in iterators or
to manage `Iter` objects.
