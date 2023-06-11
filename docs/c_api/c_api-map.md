# `map.h`

This header defines the Nest hash map object.

## Macros

### `_NST_MIN_MAP_SIZE`

**Description**:

The minimum size of the array of nodes of a map.

---

### `MAP`

**Synopsis**:

```better-c
MAP(ptr)
```

**Description**:

Casts `ptr` to `Nst_MapObj *`

---

### `nst_map_set`

**Synopsis**:

```better-c
nst_map_set(map, key, value)
```

**Description**:

Alias for [`_nst_map_set`](#_nst_map_set) that casts `map` to `Nst_MapObj *` and
casts `key` and `value` to `Nst_Obj *`.

---

### `nst_map_get`

**Synopsis**:

```better-c
nst_map_get(map, key)
```

**Description**:

Alias for [`_nst_map_get`](#_nst_map_get) that casts `map` to `Nst_MapObj *` and
casts `key` to `Nst_Obj *`.

---

### `nst_map_drop`

**Synopsis**:

```better-c
nst_map_drop(map, key)
```

**Description**:

Alias for [`_nst_map_get`](#_nst_map_drop) that casts `map` to `Nst_MapObj *`
and casts `key` to `Nst_Obj *`.

---

### `nst_map_get_next_idx`

**Synopsis**:

```better-c
nst_map_get_next_idx(curr_idx, map)
```

**Description**:

Alias for [`_nst_map_get_next_idx`](#_nst_map_get_next_idx) that casts `map` to
`Nst_MapObj *`.

---

### `nst_map_get_prev_idx`

**Synopsis**:

```better-c
nst_map_get_prev_idx(curr_idx, map)
```

**Description**:

Alias for [`_nst_map_get_prev_idx`](#_nst_map_get_prev_idx) that casts `map` to
`Nst_MapObj *`.

---

### `nst_map_set_str`

**Synopsis**:

```better-c
nst_map_set_str(map, key, value, err)
```

**Description**:

Alias for [`_nst_map_set_str`](#_nst_map_set_str) that casts `map` to
`Nst_MapObj *` and casts `value` to `Nst_Obj *`.

---

### `nst_map_get_str`

**Synopsis**:

```better-c
nst_map_get_str(map, key, err)
```

**Description**:

Alias for [`_nst_map_get_str`](#_nst_map_get_str) that casts `map` to
`Nst_MapObj *`.

---

### `nst_map_drop_str`

**Synopsis**:

```better-c
nst_map_drop_str(map, key, err)
```

**Description**:

Alias for [`_nst_map_drop_str`](#_nst_map_drop_str) that casts `map` to
`Nst_MapObj *`.

---

## Structs

### `Nst_MapNode`

**Synopsis**:

```better-c
typedef struct _Nst_MapNode
{
    i32 hash;
    Nst_Obj *key;
    Nst_Obj *value;
    i32 next_idx;
    i32 prev_idx;
}
Nst_MapNode
```

**Description**:

A single key-value pair in a map.

**Fields**:

- `hash`: the hash of the key
- `key`: the key
- `value`: the value
- `next_idx`: the next node in the map
- `prev_idx`: the previous node in the map

---

### `Nst_MapObj`

**Synopsis**:

```better-c
typedef struct _Nst_MapObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    usize size;
    usize item_count;
    usize mask;
    Nst_MapNode *nodes;
    i32 head_idx;
    i32 tail_idx;
}
Nst_MapObj
```

**Description**:

The structure for a Nest map object.

**Fields**:

- `size`: the size of the `nodes` array
- `item_count`: the number of items inside the map
- `mask`: the number of bits of the hash used to index the `nodes` array
- `nodes`: the array of key-value pairs
- `head_idx`: the index of the first node of the map
- `tail_idx`: the index of the last node of the map

---

## Functions

### `nst_map_new`

**Synopsis**:

```better-c
Nst_Obj *nst_map_new(Nst_OpErr *err)
```

**Description**:

Creates a new empty map on the heap.

**Arguments**:

- `[out] err`: the error

**Return value**:

The function returns the new map on success and `NULL` on failure.

---

### `_nst_map_set`

**Synopsis**:

```better-c
bool _nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value)
```

**Description**:

Inserts a new key-value pair in the map or updates the value of a key.

**Arguments**:

- `[inout] map`: the map to insert the value into
- `[in] key`: the key to add or update
- `[in] value`: the value associated with the key

**Return value**:

The function returns `true` if the item was inserted in the map and `false`
otherwise.

---

### `_nst_map_get`

**Synopsis**:

```better-c
Nst_Obj *_nst_map_get(Nst_MapObj *map, Nst_Obj *key)
```

**Description**:

Gets the value associated with a key.

**Arguments**:

- `[in] map`: the map to get the value from
- `[in] key`: the key of the value

**Return value**:

The function returns the associated value or `NULL` in case of error.

---

### `_nst_map_drop`

**Synopsis**:

```better-c
Nst_Obj *_nst_map_drop(Nst_MapObj *map, Nst_Obj *key)
```

**Description**:

Drops a key-value pair from a map.

**Arguments**:

- `[inout] map`: the map to drop the pair from
- `[in] key`: the key of the pair

**Return value**:

The function returns the value of the pair dropped.

---

### `_nst_map_get_next_idx`

**Synopsis**:

```better-c
i32 _nst_map_get_next_idx(i32 curr_idx, Nst_MapObj *map)
```

**Description**:

Returns the index of the node after the one at index `curr_idx`. If `curr_idx`
is `-1`, the index of the first node is returned.

**Arguments**:

- `[in] curr_idx`: the current index
- `[in] map`: the map of which the index refers to

**Return value**:

The function returns the next index or `-1` if it is the last node.

**Example**:

```better-c
// Iterating over the values of a map

for ( i32 i = nst_map_get_next_idx(-1, map);
      i != -1;
      i = nst_map_get_next_idx(i, map) ):
{
    Nst_MapNode node = map->nodes[i];
}
```

---

### `_nst_map_get_prev_idx`

**Synopsis**:

```better-c
i32 _nst_map_get_prev_idx(i32 curr_idx, Nst_MapObj *map)
```

**Description**:

Returns the index of the node before the one at index `curr_idx`. If `curr_idx`
is `-1`, the index of the last node is returned.

**Arguments**:

- `[in] curr_idx`: the current index
- `[in] map`: the map of which the index refers to

**Return value**:

The function returns the next index or `-1` if it is the first node.

---

### `_nst_map_resize`

**Synopsis**:

```better-c
bool _nst_map_resize(Nst_MapObj *map, bool force_item_reset)
```

**Description**:

Resizes the node array if necessary. `force_item_reset` forces all the items in
the map to be re-inserted

**Arguments**:

- `[inout] map`: the map to resize
- `[in] force_item_reset`: whether to re-insert the items

**Return value**:

Returns `true` on success and `false` on failure.

---

### `_nst_map_set_str`

**Synopsis**:

```better-c
bool _nst_map_set_str(Nst_MapObj *map,
                      const i8   *key,
                      Nst_Obj    *value,
                      Nst_OpErr  *err)
```

**Description**:

Sets a key-value pair creating the key from a C-style string.

**Arguments**:

- `[inout] map`: the map to add the pair to
- `[in] key`: the key
- `[in] value`: the value
- `[out] err`: the error

**Return value**:

Returns `true` on success and `false` on failure. When `false` is returned, `err`
is guaranteed to be set.

---

### `_nst_map_get_str`

**Synopsis**:

```better-c
Nst_Obj *_nst_map_get_str(Nst_MapObj *map, const i8 *key, Nst_OpErr *err)
```

**Description**:

Gets a value from a map creating the key from a C-style string.

**Arguments**:

- `[in] map`: the map to get the value from
- `[in] key`: the key associated to the value
- `[out] err`: the error

**Return value**:

The function returns `NULL` if an error occurred or if the key was not inside
the dictionary, and the value associated to the key on success.

---

### `_nst_map_drop_str`

**Synopsis**:

```better-c
Nst_Obj *_nst_map_drop_str(Nst_MapObj *map, const i8 *key, Nst_OpErr *err)
```

**Description**:

Drops a key-value pair from a map creating the key from a C-style string.

**Arguments**:

- `[inout] map`: the map to remove the pair from
- `[in] key`: the key of the pair
- `[out] err`: the error

**Return value**:

The function returns `NULL` if an error occurred or if the key was not inside
the dictionary, and the value associated to the removed key on success.

---

### Other functions

**Synopsis**:

```better-c
void _nst_map_destroy(Nst_MapObj *map)
void _nst_map_traverse(Nst_MapObj *map)
void _nst_map_track(Nst_MapObj *map)
```

**Description**:

These functions are used to manage map objects.
