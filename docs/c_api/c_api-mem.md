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

## Structs

### `Nst_Buffer`

**Synopsis**:

```better-c
struct _Nst_Buffer
{
    usize len;
    usize size;
    i8 *data;
}
Nst_Buffer
```

**Description**:

A dynaimc buffer for repeated string concatenation.

**Fields**:

- `len`: the length of the string contained in the buffer
- `size`: the maximum size of the buffer
- `data`: the data of the buffer

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

---

### `nst_buffer_init`

**Synopsis**:

```better-c
bool nst_buffer_init(Nst_Buffer *buf, usize initial_size, Nst_OpErr *err)
```

**Description**:

Initializes a buffer struct allocating `initial_size` bytes for `data`.

**Arguments**:

- `[out] buf`: the buffer
- `[in] initial_size`: the initial size of the buffer
- `[out] err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_buffer_expand_by`

**Synopsis**:

```better-c
bool nst_buffer_expand_by(Nst_Buffer *buf, usize amount, Nst_OpErr *err)
```

**Description**:

Expands the buffer capacity by `amount` bytes. This does not always allocate
memory as the buffer is made larger than requested when expanding it to decrease
the allocations.

**Arguments**:

- `[inout] buf`: the buffer
- `[in] amount`: the number of bytes to increase the buffer by
- `[out] err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_buffer_expand_to`

**Synopsis**:

```better-c
bool nst_buffer_expand_to(Nst_Buffer *buf, usize size, Nst_OpErr *err)
```

**Description**:

Expands the buffer capacity to `size` bytes. This does not always allocate
memory as the buffer is made larger than requested when expanding it to decrease
the allocations.

**Arguments**:

- `[inout] buf`: the buffer
- `[in] size`: the minimum size to reach
- `[out] err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_buffer_fit`

**Synopsis**:

```better-c
void nst_buffer_fit(Nst_Buffer *buf)
```

**Description**:

Makes the `size` of the buffer `len + 1`.

**Arguments**:

- `[inout] buf`: the buffer

---

### `nst_buffer_append`

**Synopsis**:

```better-c
bool nst_buffer_append(Nst_Buffer *buf, Nst_StrObj *str, Nst_OpErr *err)
```

**Description**:

Appends the contents of a Nest string object to the end of the buffer, the
buffer is automatically expanded.

**Arguments**:

- `[inout] buf`: the buffer
- `[in] str`: the string to append
- `[out] err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_buffer_append_c_str`

**Synopsis**:

```better-c
bool nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str, Nst_OpErr *err)
```

**Description**:

Appends the contents a NUL-terminated string to the end of the buffer, the
buffer is automatically expanded.

**Arguments**:

- `[inout] buf`: the buffer
- `[in] str`: the string to append
- `[out] err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_buffer_append_char`

**Synopsis**:

```better-c
bool nst_buffer_append_char(Nst_Buffer *buf, i8 ch, Nst_OpErr *err)
```

**Description**:

Appends a character to the end of the buffer, the buffer is automatically
expanded.

**Arguments**:



**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_buffer_to_string`

**Synopsis**:

```better-c
Nst_StrObj *nst_buffer_to_string(Nst_Buffer *buf, Nst_OpErr *err)
```

**Description**:

Calls [`nst_buffer_fit`](#nst_buffer_fit) on `buf` and creates a string with it.

**Arguments**:

- `[in] buf`: the buffer to turn into a string
- `[out] err`: the error

**Return value**:

The function returns the created string or `NULL` in case of failure. On failure
the contents of the buffer are freed.

---

### `nst_buffer_destroy`

**Synopsis**:

```better-c
void nst_buffer_destroy(Nst_Buffer *buf)
```

**Description**:

Frees the contents of the buffer but not the buffer itself. If the buffer was
already this function exists early.
