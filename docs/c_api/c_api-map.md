# `map.h`

[`Nst_MapObj`](c_api-map.md#nst_mapobj) interface.

## Authors

TheSilvered

## Macros

### `_Nst_MAP_MIN_SIZE`

**Description:**

The minimum size of a map, must be a power of two.

---

### `MAP`

**Synopsis:**

```better-c
MAP(ptr)
```

**Description:**

Casts `ptr` to [`Nst_MapObj *`](c_api-map.md#nst_mapobj).

---

### `Nst_map_set`

**Synopsis:**

```better-c
Nst_map_set(map, key, value)
```

**Description:**

Alias for [`_Nst_map_set`](c_api-map.md#_nst_map_set) which casts `map` to
[`Nst_MapObj *`](c_api-map.md#nst_mapobj), and `key` and value to
[`Nst_Obj *`](c_api-obj.md#nst_obj).

---

### `Nst_map_get`

**Synopsis:**

```better-c
Nst_map_get(map, key)
```

**Description:**

Alias for [`_Nst_map_get`](c_api-map.md#_nst_map_get) that casts `map` to
[`Nst_MapObj *`](c_api-map.md#nst_mapobj) and `key` to
[`Nst_Obj *`](c_api-obj.md#nst_obj).

---

### `Nst_map_drop`

**Synopsis:**

```better-c
Nst_map_drop(map, key)
```

**Description:**

Alias for [`_Nst_map_drop`](c_api-map.md#_nst_map_drop) that casts `map` to
[`Nst_MapObj *`](c_api-map.md#nst_mapobj) and `key` to
[`Nst_Obj *`](c_api-obj.md#nst_obj).

---

### `Nst_map_get_next_idx`

**Synopsis:**

```better-c
Nst_map_get_next_idx(curr_idx, map)
```

**Description:**

Alias for [`_Nst_map_get_next_idx`](c_api-map.md#_nst_map_get_next_idx) that
casts `map` to [`Nst_MapObj *`](c_api-map.md#nst_mapobj).

---

### `Nst_map_get_prev_idx`

**Synopsis:**

```better-c
Nst_map_get_prev_idx(curr_idx, map)
```

**Description:**

Alias for [`_Nst_map_get_prev_idx`](c_api-map.md#_nst_map_get_prev_idx) that
casts `map` to [`Nst_MapObj *`](c_api-map.md#nst_mapobj).

---

### `Nst_map_set_str`

**Synopsis:**

```better-c
Nst_map_set_str(map, key, value)
```

**Description:**

Alias for [`_Nst_map_set_str`](c_api-map.md#_nst_map_set_str) that casts `map`
to [`Nst_MapObj *`](c_api-map.md#nst_mapobj) and `value` to
[`Nst_Obj *`](c_api-obj.md#nst_obj).

---

### `Nst_map_get_str`

**Synopsis:**

```better-c
Nst_map_get_str(map, key)
```

**Description:**

Alias for [`Nst_map_get_str`](c_api-map.md#nst_map_get_str) that casts `map` to
[`Nst_MapObj *`](c_api-map.md#nst_mapobj).

---

### `Nst_map_drop_str`

**Synopsis:**

```better-c
Nst_map_drop_str(map, key)
```

**Description:**

Alias for [`Nst_map_drop_str`](c_api-map.md#nst_map_drop_str) that casts map to
[`Nst_MapObj *`](c_api-map.md#nst_mapobj).

---

## Structs

### `Nst_MapNode`

**Synopsis:**

```better-c
typedef struct _Nst_MapNode {
    i32 hash;
    Nst_Obj *key;
    Nst_Obj *value;
    i32 next_idx;
    i32 prev_idx;
} Nst_MapNode
```

**Description:**

The structure representing a node of a [`Nst_MapObj`](c_api-map.md#nst_mapobj).

**Fields:**

- `hash`: the hash of the key contained in the node
- `key`: the key of the node
- `value`: the value of the node
- `next_idx`: the index of the next node inside the map
- `prev_idx`: the index of the previous node inside the map

---

### `Nst_MapObj`

**Synopsis:**

```better-c
typedef struct _Nst_MapObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    usize cap;
    usize len;
    usize mask;
    Nst_MapNode *nodes;
    i32 head_idx;
    i32 tail_idx;
} Nst_MapObj
```

**Description:**

The structure representing a Nest map object.

**Fields:**

- `cap`: the current capacity of the nodes array
- `len`: the number of nodes inside the map
- `mask`: the mask applied to the hash when inserting new nodes
- `nodes`: the array of nodes of the map
- `head_idx`: the first node in the map
- `tail_idx`: the last node in the map

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

### `_Nst_map_set`

**Synopsis:**

```better-c
bool _Nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value)
```

**Description:**

Inserts or modifies a value in the map.

**Parameters:**

- `map`: the map to update
- `key`: the key to insert or modify
- `value`: the value to associate with the key

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `_Nst_map_get`

**Synopsis:**

```better-c
Nst_Obj *_Nst_map_get(Nst_MapObj *map, Nst_Obj *key)
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

### `_Nst_map_drop`

**Synopsis:**

```better-c
Nst_Obj *_Nst_map_drop(Nst_MapObj *map, Nst_Obj *key)
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

### `_Nst_map_destroy`

**Synopsis:**

```better-c
void _Nst_map_destroy(Nst_MapObj *map)
```

**Description:**

The destructor for the map object.

---

### `_Nst_map_traverse`

**Synopsis:**

```better-c
void _Nst_map_traverse(Nst_MapObj *map)
```

**Description:**

The traverse function for the map object.

---

### `_Nst_map_track`

**Synopsis:**

```better-c
void _Nst_map_track(Nst_MapObj *map)
```

**Description:**

The track function for the map object.

---

### `_Nst_map_get_next_idx`

**Synopsis:**

```better-c
i32 _Nst_map_get_next_idx(i32 curr_idx, Nst_MapObj *map)
```

**Description:**

Gets the following index in a map given the current one.

If curr_idx is `-1`, the first index is returned.

**Parameters:**

- `curr_idx`: the current index
- `map`: the map to get the index from

**Returns:**

The following index or `-1` if the given index is the last one. No error is set.

---

### `_Nst_map_get_prev_idx`

**Synopsis:**

```better-c
i32 _Nst_map_get_prev_idx(i32 curr_idx, Nst_MapObj *map)
```

**Description:**

Gets the preceding index in a map given the current one.

If curr_idx is `-1`, the first index is returned.

**Parameters:**

- `curr_idx`: the current index
- `map`: the map to get the index from

**Returns:**

The preceding index or `-1` if the given index is the last one. No error is set.

---

### `_Nst_map_resize`

**Synopsis:**

```better-c
bool _Nst_map_resize(Nst_MapObj *map, bool force_item_reset)
```

**Description:**

Resizes the node array if necessary.

**Parameters:**

- `map`: the map to resize
- `force_item_reset`: whether to force the nodes inside the map to be
  re-inserted

**Returns:**

`true` on success and `false` on failure. The error is set. When shrinking the
function is guaranteed to succeed.

---

### `_Nst_map_set_str`

**Synopsis:**

```better-c
bool _Nst_map_set_str(Nst_MapObj *map, const i8 *key, Nst_Obj *value)
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

### `_Nst_map_get_str`

**Synopsis:**

```better-c
Nst_Obj *_Nst_map_get_str(Nst_MapObj *map, const i8 *key)
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

### `_Nst_map_drop_str`

**Synopsis:**

```better-c
Nst_Obj *_Nst_map_drop_str(Nst_MapObj *map, const i8 *key)
```

**Description:**

Drops a key from a map and returns its value.

**Parameters:**

- `map`: the map to drop the key from
- `key`: the key to drop as a C string

**Returns:**

The object associated with the removed key on success or `NULL` if the key is
not hashable or is not inside the map. No error is set.

