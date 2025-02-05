# `iter.h`

Nst_IterObj interface.

## Authors

TheSilvered

---

## Functions

### `Nst_iter_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_new(Nst_Obj *start, Nst_Obj *next, Nst_Obj *value)
```

**Description:**

Creates a new Nest iterator object.

!!!note
    This function takes one reference of `start`, `next` and `value` both on
    success and on failure.

**Parameters:**

- `start`: the `start` function for the new iterator
- `next`: the `next` function for the new iterator
- `value`: the `value` for the new iterator

**Returns:**

The new object or `NULL` on failure. The error is set.

---

### `_Nst_iter_destroy`

**Synopsis:**

```better-c
void _Nst_iter_destroy(Nst_Obj *iter)
```

**Description:**

Destructor for Nest iter objects.

---

### `_Nst_iter_traverse`

**Synopsis:**

```better-c
void _Nst_iter_traverse(Nst_Obj *iter)
```

**Description:**

Traverse function for Nest iter objects.

---

### `Nst_iter_start`

**Synopsis:**

```better-c
bool Nst_iter_start(Nst_Obj *iter)
```

**Description:**

Calls the `start` function of an `Iter` object.

**Parameters:**

- `iter`: the iterator to start

**Returns:**

`true` on success and `false` on success. The error is set.

---

### `Nst_iter_next`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_next(Nst_Obj *iter)
```

**Description:**

Calls the `next` function of an `Iter` object.

**Parameters:**

- `iter`: the iterator to get the value from

**Returns:**

The resulting object on success and `NULL` on failure. The error is set.

---

### `Nst_iter_start_func`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_start_func(Nst_Obj *iter)
```

**Description:**

Get the `start` function of an iterator. No reference is added.

---

### `Nst_iter_next_func`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_next_func(Nst_Obj *iter)
```

**Description:**

Get the `next` function of an iterator. No reference is added.

---

### `Nst_iter_value`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_value(Nst_Obj *iter)
```

**Description:**

Get the `value` passed to `start` and `next` of an iterator. No reference is
added.

---

### `Nst_iter_range_start`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_range_start(usize arg_num, Nst_Obj **args)
```

**Description:**

The `start` function of the range iterator.

---

### `Nst_iter_range_next`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_range_next(usize arg_num, Nst_Obj **args)
```

**Description:**

The `next` function of the range iterator.

---

### `Nst_iter_seq_start`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_seq_start(usize arg_num, Nst_Obj **args)
```

**Description:**

The `start` function of the sequence iterator.

---

### `Nst_iter_seq_next`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_seq_next(usize arg_num, Nst_Obj **args)
```

**Description:**

The `next` function of the sequence iterator.

---

### `Nst_iter_str_start`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_str_start(usize arg_num, Nst_Obj **args)
```

**Description:**

The `start` function of the string iterator.

---

### `Nst_iter_str_next`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_str_next(usize arg_num, Nst_Obj **args)
```

**Description:**

The `next` function of the string iterator.

---

### `Nst_iter_map_start`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_map_start(usize arg_num, Nst_Obj **args)
```

**Description:**

The `start` function of the map iterator.

---

### `Nst_iter_map_next`

**Synopsis:**

```better-c
Nst_Obj *Nst_iter_map_next(usize arg_num, Nst_Obj **args)
```

**Description:**

The `next` function of the map iterator.
