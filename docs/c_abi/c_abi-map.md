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
nst_map_set(map, key, value, err)
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

**Arguments**:

- `arg`:

---

### `nst_map_set_str`

**Synopsis**:

```better-c
nst_map_set_str(map, key, value, err)
```

**Description**:

Alias for [`_nst_map_set_str`](#_nst_map_set_str) that casts `map` to `Nst_MapObj *` and
casts `value` to `Nst_Obj *`.
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
---

### `nst_map_get_str`

**Synopsis**:

```better-c
```

**Description**:



**Arguments**:

- `arg`:

---

### `nst_map_drop_str`

**Synopsis**:

```better-c
```

**Description**:



**Arguments**:

- `arg`:

---

## Structs

### `struct`

**Synopsis**:

```better-c
```

**Description**:



**Fields**:

- `field`:

---

## Functions

### `function`

**Synopsis**:

```better-c
```

**Description**:



**Arguments**:

- `arg`:

**Return value**:

---

## Enums

### `enum`

**Synopsis**:

```better-c
```

**Description**:



**Fields**:

- `field`:
