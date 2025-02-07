# `map.h`

`Map` object interface.

## Authors

TheSilvered

---

## Macros

### `_Nst_MAP_MIN_SIZE`

**Description:**

The minimum size of a map, must be a power of two.

---

## Functions

### `Nst_map_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_map_new(void)
```

**Description:**

Creates a new map object.

**Returns:**

The new object or `NULL` on failure. The error is set.

---

### `Nst_map_copy`

**Synopsis:**

```better-c
Nst_Obj *Nst_map_copy(Nst_Obj *map)
```

**Description:**

Creates a shallow copy of a map object.

**Parameters:**

- `map`: the map to copy

**Returns:**

The copied map or `NULL` on failure. The error is set.

---

### `_Nst_map_destroy`

**Synopsis:**

```better-c
void _Nst_map_destroy(Nst_Obj *map)
```

**Description:**

The destructor for the map object.

---

### `_Nst_map_traverse`

**Synopsis:**

```better-c
void _Nst_map_traverse(Nst_Obj *map)
```

**Description:**

The traverse function for the map object.

---

### `Nst_map_len`

**Synopsis:**

```better-c
usize Nst_map_len(Nst_Obj *map)
```

**Description:**

Get the number of key-value pairs in a map.

---

### `Nst_map_cap`

**Synopsis:**

```better-c
usize Nst_map_cap(Nst_Obj *map)
```

**Description:**

Get the current capacity of a map.

---

### `Nst_map_set`

**Synopsis:**

```better-c
bool Nst_map_set(Nst_Obj *map, Nst_Obj *key, Nst_Obj *value)
```

**Description:**

Inserts or modifies a value in the map. Adds a reference to both the key and the
value.

**Parameters:**

- `map`: the map to update
- `key`: the key to insert or modify
- `value`: the value to associate with the key

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_map_get`

**Synopsis:**

```better-c
Nst_Obj *Nst_map_get(Nst_Obj *map, Nst_Obj *key)
```

**Description:**

Gets the value associated with a key.

**Parameters:**

- `map`: the map to get the value from
- `key`: the key to get

**Returns:**

The object associated with the key on success or `NULL` if the key is not
hashable or is not inside the map. No error is set.

---

### `Nst_map_drop`

**Synopsis:**

```better-c
Nst_Obj *Nst_map_drop(Nst_Obj *map, Nst_Obj *key)
```

**Description:**

Drops a key from a map and returns its value.

**Parameters:**

- `map`: the map to drop the key from
- `key`: the key to drop

**Returns:**

The object associated with the removed key on success or `NULL` if the key is
not hashable or is not inside the map. No error is set.

---

### `Nst_map_set_str`

**Synopsis:**

```better-c
bool Nst_map_set_str(Nst_Obj *map, const i8 *key, Nst_Obj *value)
```

**Description:**

Inserts or modifies a value in the map.

**Parameters:**

- `map`: the map to update
- `key`: the key to insert or modify as a C string
- `value`: the value to associate with the key

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_map_get_str`

**Synopsis:**

```better-c
Nst_Obj *Nst_map_get_str(Nst_Obj *map, const i8 *key)
```

**Description:**

Gets the value associated with a key.

**Parameters:**

- `map`: the map to get the value from
- `key`: the key to get as a C string

**Returns:**

The object associated with the key on success or `NULL` if the key is not
hashable or is not inside the map. No error is set.

---

### `Nst_map_drop_str`

**Synopsis:**

```better-c
Nst_Obj *Nst_map_drop_str(Nst_Obj *map, const i8 *key)
```

**Description:**

Drops a key from a map and returns its value.

**Parameters:**

- `map`: the map to drop the key from
- `key`: the key to drop as a C string

**Returns:**

The object associated with the removed key on success or `NULL` if the key is
not hashable or is not inside the map. No error is set.

---

### `Nst_map_next`

**Synopsis:**

```better-c
isize Nst_map_next(isize idx, Nst_Obj *map, Nst_Obj **out_key,
                   Nst_Obj **out_val)
```

**Description:**

Get the next key-value pair in the map given an index.

To get the first item pass `-1` to `idx` and to continue looping pass the
previously returned value as `idx`. The function returns `-1` when there are no
more items.

**Parameters:**

- `idx`: the previous returned index or `-1` for the first item
- `map`: the map to iterate over
- `out_key`: pointer set to the key, can be `NULL`, no reference is added to the
  key
- `out_val`: pointer set to the value, can be `NULL`, no reference is added to
  the value

**Returns:**

The index of the current pair or `-1` if the map contains no more pairs. If the
return value is `-1`, `out_key` and `out_val` are set to `NULL`.

---

### `Nst_map_prev`

**Synopsis:**

```better-c
isize Nst_map_prev(isize idx, Nst_Obj *map, Nst_Obj **out_key,
                   Nst_Obj **out_val)
```

**Description:**

Get the previous key-value pair in the map given an index.

To get the last item pass `-1` to `idx` and to continue looping pass the
previously returned value as `idx`. The function returns `-1` when there are no
more items.

**Parameters:**

- `idx`: the previous returned index or `-1` for the first item
- `map`: the map to iterate over
- `out_key`: pointer set to the key, can be `NULL`, no reference is added to the
  key
- `out_val`: pointer set to the value, can be `NULL`, no reference is added to
  the value

**Returns:**

The index of the current pair or `-1` if the map contains no more pairs. If the
return value is `-1`, `out_key` and `out_val` are set to `NULL`.
