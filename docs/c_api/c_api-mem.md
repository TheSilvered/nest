# `mem.h`

Heap & dynamic memory management functions.

## Authors

TheSilvered

---

## Macros

### `Nst_malloc_c`

**Synopsis:**

```better-c
#define Nst_malloc_c(count, type)
```

**Description:**

Call [`Nst_malloc`](c_api-mem.md#nst_malloc) using `sizeof(type)` for the size
and casting the result to a pointer of `type`.

---

### `Nst_calloc_c`

**Synopsis:**

```better-c
#define Nst_calloc_c(count, type, init_value)
```

**Description:**

Call [`Nst_calloc`](c_api-mem.md#nst_calloc) using `sizeof(type)` for the size
and casting the result to a pointer of `type`.

---

### `Nst_realloc_c`

**Synopsis:**

```better-c
#define Nst_realloc_c(block, new_count, type, count)
```

**Description:**

Call [`Nst_realloc`](c_api-mem.md#nst_realloc) using `sizeof(type)` for the size
and casting the result to a pointer of `type`.

---

### `Nst_crealloc_c`

**Synopsis:**

```better-c
#define Nst_crealloc_c(block, new_count, type, count, init_value)
```

**Description:**

Call [`Nst_crealloc`](c_api-mem.md#nst_crealloc) using `sizeof(type)` for the
size and casting the result to a pointer of `type`.

---

### `Nst_free`

**Description:**

Alias for [`Nst_raw_free`](c_api-mem.md#nst_raw_free).

---

## Functions

### `Nst_raw_malloc`

**Synopsis:**

```better-c
void *Nst_raw_malloc(usize size)
```

**Description:**

Alias for C [`malloc`](https://man7.org/linux/man-pages/man3/malloc.3.html).

---

### `Nst_raw_calloc`

**Synopsis:**

```better-c
void *Nst_raw_calloc(usize count, usize size)
```

**Description:**

Alias for C [`calloc`](https://man7.org/linux/man-pages/man3/malloc.3.html).

---

### `Nst_raw_realloc`

**Synopsis:**

```better-c
void *Nst_raw_realloc(void *block, usize size)
```

**Description:**

Alias for C [`realloc`](https://man7.org/linux/man-pages/man3/malloc.3.html).

---

### `Nst_raw_free`

**Synopsis:**

```better-c
void Nst_raw_free(void *block)
```

**Description:**

Alias for C [`free`](https://man7.org/linux/man-pages/man3/malloc.3.html).

---

### `Nst_log_alloc_count`

**Synopsis:**

```better-c
void Nst_log_alloc_count(void)
```

**Description:**

Prints the current allocation count to `stdout`. Works only if
[`Nst_DBG_COUNT_ALLOC`](c_api-typedefs.md#nst_dbg_count_alloc) is defined,
otherwise does nothing.

---

### `Nst_log_alloc_info`

**Synopsis:**

```better-c
void Nst_log_alloc_info(void)
```

**Description:**

Prints information about the current allocations to `stdout`. Works only if
[`Nst_DBG_COUNT_ALLOC`](c_api-typedefs.md#nst_dbg_count_alloc) is defined,
otherwise does nothing.

---

### `Nst_malloc`

**Synopsis:**

```better-c
void *Nst_malloc(usize count, usize size)
```

**Description:**

Allocate memory on the heap.

The elements are contiguous in memory.

**Parameters:**

- `count`: the number of elements to allocate
- `size`: the size in bytes of each element

**Returns:**

A pointer to the allocated memory block or `NULL` on failure. The error is set.

---

### `Nst_calloc`

**Synopsis:**

```better-c
void *Nst_calloc(usize count, usize size, void *init_value)
```

**Description:**

Allocate memory on the heap initializing it.

The elements are contiguous in memory. If `init_value` is `NULL`, the function
has a similar behaviour to
[`calloc`](https://man7.org/linux/man-pages/man3/malloc.3.html) filling the
memory with zeroes. `init_value` is expected to be the same size as the one
given for the elements.

**Parameters:**

- `count`: the number of elements to allocate
- `size`: the size in bytes of each element
- `init_value`: a pointer to the value to initialize each element with

**Returns:**

A pointer to the allocated memory block or `NULL` on failure. The error is set.

---

### `Nst_realloc`

**Synopsis:**

```better-c
void *Nst_realloc(void *block, usize new_count, usize size, usize count)
```

**Description:**

Change the size of an allocated memory block.

This function never fails when the block shrinks in size.

**Parameters:**

- `block`: the block to reallocate
- `new_count`: the new number of elements of the block
- `size`: the size in bytes of each element
- `count`: the current number of elements in the block

**Returns:**

A pointer to the reallocated memory block or `NULL` on failure. The error is
set. If either `new_count` or `size` is zero, block is freed and `NULL` is
returned with no error.

---

### `Nst_crealloc`

**Synopsis:**

```better-c
void *Nst_crealloc(void *block, usize new_count, usize size, usize count,
                   void *init_value)
```

**Description:**

Change the size of an allocated memory block initializing new memory.

This function never fails when the block shrinks. If `init_value` is `NULL`, the
function just fills the new memory with zeroes. `init_value` is expected to have
a size of `size`.

**Parameters:**

- `block`: the block to reallocate
- `new_count`: the new number of elements of the block
- `size`: the size in bytes of each element
- `count`: the current number of elements in the block
- `init_value`: a pointer to the value to initialize the new elements with

**Returns:**

A pointer to the reallocated memory block or `NULL` on failure. The error is
set. If either `new_count` or `size` is zero, block is freed and `NULL` is
returned with no error.

---

### `Nst_memset`

**Synopsis:**

```better-c
void Nst_memset(void *block, usize size, usize count, void *value)
```

**Description:**

Set the value of an array in memory.

!!!note
    Unlike [`memset`](https://man7.org/linux/man-pages/man3/memset.3.html) in
    `string.h` this function does not return a value.

!!!warning
    The behaviour of this function is undefined if `block` and `value` overlap.

**Parameters:**

- `block`: the pointer to the block of memory to edit
- `size`: the size in bytes of a unit inside `block`
- `count`: the number of units inside `block`
- `value`: a pointer to the value to copy for each unit, if it is NULL the block
  is filled with zeroes
