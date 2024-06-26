# `iter.h`

Nst_IterObj interface.

## Authors

TheSilvered

---

## Macros

### `ITER`

**Synopsis:**

```better-c
#define ITER(ptr)
```

**Description:**

Casts `ptr` to [`Nst_IterObj *`](c_api-iter.md#nst_iterobj).

---

### `Nst_iter_start`

**Synopsis:**

```better-c
#define Nst_iter_start(iter)
```

**Description:**

Alias for [`_Nst_iter_start`](c_api-iter.md#_nst_iter_start) that casts `iter`
to [`Nst_IterObj *`](c_api-iter.md#nst_iterobj).

---

### `Nst_iter_get_val`

**Synopsis:**

```better-c
#define Nst_iter_get_val(iter)
```

**Description:**

Alias for [`_Nst_iter_get_val`](c_api-iter.md#_nst_iter_get_val) that casts
`iter` to [`Nst_IterObj *`](c_api-iter.md#nst_iterobj).

---

### `Nst_iter_new`

**Synopsis:**

```better-c
#define Nst_iter_new(start, end, value)
```

**Description:**

Alias for [`_Nst_iter_new`](c_api-iter.md#_nst_iter_new) that casts `start` and
`end` to [`Nst_FuncObj *`](c_api-function.md#nst_funcobj) and casts `value` to
[`Nst_Obj *`](c_api-obj.md#nst_obj).

---

## Structs

### `Nst_IterObj`

**Synopsis:**

```better-c
typedef struct _Nst_IterObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_FuncObj *start;
    Nst_FuncObj *get_val;
    Nst_Obj *value;
} Nst_IterObj
```

**Description:**

The structure defining a Nest iterator object.

**Fields:**

- `start`: the function that initializes the iterator
- `get_val`: the function of the iterator that gets the current value
- `value`: the value passed to the functions of the iterator

---

## Functions

### `_Nst_iter_new`

**Synopsis:**

```better-c
Nst_Obj *_Nst_iter_new(Nst_FuncObj *start, Nst_FuncObj *get_val,
                       Nst_Obj *value)
```

**Description:**

Creates a new Nest iterator object.

!!!note
    This function takes one reference of `start`, `get_val` and `value` both on
    success and on failure.

**Parameters:**

- `start`: the `start` function for the new iterator
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

### `_Nst_iter_start`

**Synopsis:**

```better-c
bool _Nst_iter_start(Nst_IterObj *iter)
```

**Description:**

Calls the `start` function of a [`Nst_IterObj`](c_api-iter.md#nst_iterobj).

**Parameters:**

- `iter`: the iterator to start

**Returns:**

`true` on success and `false` on success. The error is set.

---

### `_Nst_iter_get_val`

**Synopsis:**

```better-c
Nst_Obj *_Nst_iter_get_val(Nst_IterObj *iter)
```

**Description:**

Calls the `get_val` function of a [`Nst_IterObj`](c_api-iter.md#nst_iterobj).

**Parameters:**

- `iter`: the iterator to get the value from

**Returns:**

The resulting object on success and `NULL` on failure. The error is set.

---

### `Nst_iter_range_start`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_range_start(usize arg_num, Nst_Obj **args)
```

**Description:**

The `start` function of the range iterator.

---

### `Nst_iter_range_get_val`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_range_get_val(usize arg_num, Nst_Obj **args)
```

**Description:**

The `get_val` function of the range iterator.

---

### `Nst_iter_seq_start`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_seq_start(usize arg_num, Nst_Obj **args)
```

**Description:**

The `start` function of the sequence iterator.

---

### `Nst_iter_seq_get_val`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_seq_get_val(usize arg_num, Nst_Obj **args)
```

**Description:**

The `get_val` function of the sequence iterator.

---

### `Nst_iter_str_start`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_str_start(usize arg_num, Nst_Obj **args)
```

**Description:**

The `start` function of the string iterator.

---

### `Nst_iter_str_get_val`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_str_get_val(usize arg_num, Nst_Obj **args)
```

**Description:**

The `get_val` function of the string iterator.

---

### `Nst_iter_map_start`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_map_start(usize arg_num, Nst_Obj **args)
```

**Description:**

The `start` function of the map iterator.

---

### `Nst_iter_map_get_val`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_map_get_val(usize arg_num, Nst_Obj **args)
```

**Description:**

The `get_val` function of the map iterator.
