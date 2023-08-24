# `iter.h`

Nst_IterObj interface.

## Authors

TheSilvered

## Macros

### `ITER`

**Synopsis:**

```better-c
ITER(ptr)
```

**Description:**

Casts `ptr` to [`Nst_IterObj *`](c_api-iter.md/#nst_iterobj).

---

### `Nst_iter_start`

**Synopsis:**

```better-c
Nst_iter_start(iter)
```

**Description:**

Alias for [`_Nst_iter_start`](c_api-iter.md/#_nst_iter_start) that casts iter to
[`Nst_IterObj *`](c_api-iter.md/#nst_iterobj).

---

### `Nst_iter_is_done`

**Synopsis:**

```better-c
Nst_iter_is_done(iter)
```

**Description:**

Alias for [`_Nst_iter_is_done`](c_api-iter.md/#_nst_iter_is_done) that casts
iter to [`Nst_IterObj *`](c_api-iter.md/#nst_iterobj).

---

### `Nst_iter_get_val`

**Synopsis:**

```better-c
Nst_iter_get_val(iter)
```

**Description:**

Alias for [`_Nst_iter_get_val`](c_api-iter.md/#_nst_iter_get_val) that casts
iter to [`Nst_IterObj *`](c_api-iter.md/#nst_iterobj).

---

## Structs

### `Nst_IterObj`

**Synopsis:**

```better-c
typedef struct _Nst_IterObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_FuncObj *start;
    Nst_FuncObj *is_done;
    Nst_FuncObj *get_val;
    Nst_Obj *value;
} struct _Nst_IterObj
```

**Description:**

The structure defining a Nest iterator object.

**Fields:**

- `start`: the function that initializes the iterator
- `is_done`: the function of the iterator that checks if it has finished
- `get_val`: the function of the iterator that gets the current value
- `value`: the value passed to the functions of the iterator

---

## Functions

### `Nst_iter_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_new(Nst_FuncObj *start, Nst_FuncObj *is_done,
                      Nst_FuncObj *get_val, Nst_Obj *value)
```

**Description:**

Creates a new Nest iterator object.

!!!note
    This function takes one reference of `start`, `is_done`, `get_val` and value
    both on success and on failure.

**Parameters:**

- `start`: the `start` function for the new iterator
- `is_done`: the `is_done` function for the new iterator
- `get_val`: the `get_val` function for the new iterator
- `value`: the `value` for the new iterator

**Returns:**

The new object or `NULL` on failure. The error is set.

---

### `_Nst_iter_destroy`

**Synopsis:**

```better-c
void _Nst_iter_destroy(Nst_IterObj *iter)
```

**Description:**

Destructor for Nest iter objects.

---

### `_Nst_iter_traverse`

**Synopsis:**

```better-c
void _Nst_iter_traverse(Nst_IterObj *iter)
```

**Description:**

Traverse function for Nest iter objects.

---

### `_Nst_iter_track`

**Synopsis:**

```better-c
void _Nst_iter_track(Nst_IterObj *iter)
```

**Description:**

Track function for Nest iter objects.

---

### `_Nst_iter_start`

**Synopsis:**

```better-c
i32 _Nst_iter_start(Nst_IterObj *iter)
```

**Description:**

Calls the `start` function of a [`Nst_IterObj`](c_api-iter.md/#nst_iterobj).

**Parameters:**

- `iter`: the iterator to start

**Returns:**

`-1` on failure and `0` on success. The error may be set internally and must not
be set by the caller.

---

### `_Nst_iter_is_done`

**Synopsis:**

```better-c
i32 _Nst_iter_is_done(Nst_IterObj *iter)
```

**Description:**

Calls the `is_done` function of a [`Nst_IterObj`](c_api-iter.md/#nst_iterobj).

**Parameters:**

- `iter`: the iterator to check for completion

**Returns:**

`-1` on failure, `1` if the iterator is done, `0` if it can still iterate. The
error may be set internally and must not be set by the caller.

---

### `_Nst_iter_get_val`

**Synopsis:**

```better-c
Nst_Obj *_Nst_iter_get_val(Nst_IterObj *iter)
```

**Description:**

Calls the `get_val` function of a [`Nst_IterObj`](c_api-iter.md/#nst_iterobj).

**Parameters:**

- `iter`: the iterator to get the value from

**Returns:**

The resulting object on success and `NULL` on failure. The error may be set
internally and must not be set by the caller.

---

### `Nst_iter_range_start`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_range_start)
```

**Description:**

The `start` function of the range iterator.

---

### `Nst_iter_range_is_done`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_range_is_done)
```

**Description:**

The `is_done` function of the range iterator.

---

### `Nst_iter_range_get_val`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_range_get_val)
```

**Description:**

The `get_val` function of the range iterator.

---

### `Nst_iter_seq_start`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_seq_start)
```

**Description:**

The `start` function of the sequence iterator.

---

### `Nst_iter_seq_is_done`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_seq_is_done)
```

**Description:**

The `is_done` function of the sequence iterator.

---

### `Nst_iter_seq_get_val`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_seq_get_val)
```

**Description:**

The `get_val` function of the sequence iterator.

---

### `Nst_iter_str_start`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_str_start)
```

**Description:**

The `start` function of the string iterator.

---

### `Nst_iter_str_is_done`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_str_is_done)
```

**Description:**

The `is_done` function of the string iterator.

---

### `Nst_iter_str_get_val`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_str_get_val)
```

**Description:**

The `get_val` function of the string iterator.

---

### `Nst_iter_map_start`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_map_start)
```

**Description:**

The `start` function of the map iterator.

---

### `Nst_iter_map_is_done`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_map_is_done)
```

**Description:**

The `is_done` function of the map iterator.

---

### `Nst_iter_map_get_val`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(Nst_iter_map_get_val)
```

**Description:**

The `get_val` function of the map iterator.

