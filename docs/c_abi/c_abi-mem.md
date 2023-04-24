# `mem.h`

This header contains

## Macros

### `nst_raw_free`

**Synopsis**:

```better-c
nst_raw_free
```

**Description**:

An alias for [nst_free](#nst_free)

---

## Functions

### `nst_raw_malloc`

**Synopsis**:

```better-c
void *nst_raw_malloc(usize size)
```

**Description**:

The normal `malloc`

---

### `nst_raw_calloc`

**Synopsis**:

```better-c
void *nst_raw_calloc(usize count, usize size)
```

**Description**:

The normal `calloc`

---

### `nst_raw_realloc`

**Synopsis**:

```better-c
void *nst_raw_realloc(void *block, usize size)
```

**Description**:

The normal `realloc`

---

### `nst_malloc`

**Synopsis**:

```better-c
void *nst_malloc(usize count, usize size, Nst_OpErr *err)
```

**Description**:

Allocates on the heap a block of memory to contain `count` objects of size `size`.

**Arguments**:

- `[in] count`: the number of objects to allocate
- `[in] size`: the size of each object
- `[out] err`: the error

**Return value**:

The function returns a pointer to the block on success or `NULL` on failure.

---

### `nst_calloc`

**Synopsis**:

```better-c
void *nst_calloc(usize count,
                 usize size,
                 void *init_value,
                 Nst_OpErr *err)
```

**Description**:

Allocates on the heap a block of memory to contain `count` objects of size `size`
initializing their value with `init_value`.

**Arguments**:

- `[in] count`: the number of objects to allocate
- `[in] size`: the size of each object
- `[in] init_value`: the value used to initialize the objects, must the same
  size as the one specified in `size` or `NULL`, in which case the whole block
  is filled with zeroes
- `[out] err`: the error

**Return value**:

The function returns a pointer to the block on success or `NULL` on failure.

---

### `nst_realloc`

**Synopsis**:

```better-c
void *nst_realloc(void *prev_block,
                  usize new_count,
                  usize size,
                  usize prev_count,
                  Nst_OpErr *err)
```

**Description**:

Reallocates a block of memory increasing or decreasing its size. When
`new_count` is less than or equal to `prev_count` the function is guaranteed to
succeed. It is not guaranteed that a new block is returned, it could be the same
as the one given.

**Arguments**:

- `[in] prev_block`: the pointer to the block to reallocate
- `[in] new_count`: the new number of objects in the block
- `[in] size`: the size of each object
- `[in] prev_count`: the previous number of objects in the block
- `[out] err`: the error

**Return value**:

The function returns a pointer to the block on success or `NULL` on failure.

---

### `nst_crealloc`

**Synopsis**:

```better-c
void *nst_crealloc(void *prev_block,
                   usize new_count,
                   usize size,
                   usize prev_count,
                   void *init_value,
                   Nst_OpErr *err)
```

**Description**:

The function reallocates a block of memory initializing any new portions with
`init_value`

**Arguments**:

- `[in] prev_block`: the pointer to the block to reallocate
- `[in] new_count`: the new number of objects in the block
- `[in] size`: the size of each object
- `[in] prev_count`: the previous number of objects in the block, if set to zero
  the whole block is cleared
- `[in] init_value`: the value used to initialize the objects, must the same
  size as the one specified in `size` or `NULL`, in which case the whole block
  is filled with zeroes
- `[out] err`: the error

**Return value**:

---

### `nst_free`

**Synopsis**:

```better-c
void nst_free(void *block)
```

**Description**:

Frees a memory block allocated on the heap.
