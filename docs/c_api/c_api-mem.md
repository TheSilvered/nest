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

Calls [`Nst_malloc`](c_api-mem.md#nst_malloc) using `sizeof(type)` for the size
and casting the result to a pointer of `type`.

---

### `Nst_calloc_c`

**Synopsis:**

```better-c
#define Nst_calloc_c(count, type, init_value)
```

**Description:**

Calls [`Nst_calloc`](c_api-mem.md#nst_calloc) using `sizeof(type)` for the size
and casting the result to a pointer of `type`.

---

### `Nst_realloc_c`

**Synopsis:**

```better-c
#define Nst_realloc_c(block, new_count, type, count)
```

**Description:**

Calls [`Nst_realloc`](c_api-mem.md#nst_realloc) using `sizeof(type)` for the
size and casting the result to a pointer of `type`.

---

### `Nst_crealloc_c`

**Synopsis:**

```better-c
#define Nst_crealloc_c(block, new_count, type, count, init_value)
```

**Description:**

Calls [`Nst_crealloc`](c_api-mem.md#nst_crealloc) using `sizeof(type)` for the
size and casting the result to a pointer of `type`.

---

### `Nst_free`

**Description:**

Alias for [`Nst_raw_free`](c_api-mem.md#nst_raw_free).

---

## Structs

### `Nst_SBuffer`

**Synopsis:**

```better-c
typedef struct _Nst_SBuffer {
    usize len;
    usize cap;
    usize unit_size;
    void *data;
} Nst_SBuffer
```

**Description:**

Structure representing a buffer of objects with an arbitrary size.

**Fields:**

- `len`: the number of objects currently in the buffer
- `cap`: the size in bytes of the allocated block
- `unit_size`: the size in bytes of one object
- `data`: the array of objects

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

Allocates memory on the heap.

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

Allocates memory on the heap initializing it.

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

Changes the size of an allocated memory block.

This function never fails when the block is shrunk because if the call to
[`realloc`](https://man7.org/linux/man-pages/man3/malloc.3.html) fails, the old
block is returned.

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

Changes the size of an allocated memory block initializing new memory.

This function never fails when the block is shrunk because if the call to
[`realloc`](https://man7.org/linux/man-pages/man3/malloc.3.html) fails, the old
block is returned. If `init_value` is `NULL`, the function just fills the new
memory with zeroes. `init_value` is expected to have a size of `size`.

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

Sets the value of an array in memory.

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

---

### `Nst_sbuffer_init`

**Synopsis:**

```better-c
bool Nst_sbuffer_init(Nst_SBuffer *buf, usize unit_size, usize count)
```

**Description:**

Initializes a [`Nst_SBuffer`](c_api-mem.md#nst_sbuffer).

**Parameters:**

- `buf`: the buffer to initialize
- `unit_size`: the size of the elements the buffer will contain
- `count`: the number of elements to initialize the buffer with

**Returns:**

`true` on succes and `false` on failure. The error is set.

---

### `Nst_sbuffer_expand_by`

**Synopsis:**

```better-c
bool Nst_sbuffer_expand_by(Nst_SBuffer *buf, usize amount)
```

**Description:**

Expands a sized buffer to contain a specified amount new elements.

The buffer is expanded only if needed.

**Parameters:**

- `buf`: the buffer to expand
- `amount`: the number of new elements the buffer needs to contain

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_sbuffer_expand_to`

**Synopsis:**

```better-c
bool Nst_sbuffer_expand_to(Nst_SBuffer *buf, usize count)
```

**Description:**

Expands a sized buffer to contain a total amount of elements.

The buffer is expanded only if needed. If the new size is smaller than the
current one nothing is done.

**Parameters:**

- `buf`: the buffer to expand
- `amount`: the number of elements the buffer needs to contain

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_sbuffer_fit`

**Synopsis:**

```better-c
void Nst_sbuffer_fit(Nst_SBuffer *buf)
```

**Description:**

Shrinks the capacity of a sized buffer to match its length.

---

### `Nst_sbuffer_append`

**Synopsis:**

```better-c
bool Nst_sbuffer_append(Nst_SBuffer *buf, void *element)
```

**Description:**

Appends an element to the end of the buffer.

The buffer is expanded more than needed to reduce the overall number of
reallocations.

If necessary, the buffer is expanded automatically.

**Parameters:**

- `buf`: the buffer to append the element to
- `element`: a pointer to the element to append

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_sbuffer_pop`

**Synopsis:**

```better-c
bool Nst_sbuffer_pop(Nst_SBuffer *buf)
```

**Description:**

Pops the last element of a sized buffer.

**Parameters:**

- `buf`: the buffer to pop the element from

**Returns:**

`true` if the buffer was popped successfully and `false` if there was no item to
pop. No error is set.

---

### `Nst_sbuffer_at`

**Synopsis:**

```better-c
void *Nst_sbuffer_at(Nst_SBuffer *buf, usize index)
```

**Description:**

Gets the element of a buffer at a specified index.

**Parameters:**

- `buf`: the buffer to index
- `index`: the index of the element to get

**Returns:**

A pointer to the start of the element in the array or `NULL` if the index was
out of bounds. No error is set.

---

### `Nst_sbuffer_shrink_auto`

**Synopsis:**

```better-c
void Nst_sbuffer_shrink_auto(Nst_SBuffer *buf)
```

**Description:**

Shrinks the size of a sized buffer.

The size is not shrunk to the minimum but some slots are kept for possible new
values.

**Parameters:**

- `buf`: the buffer to shrink

---

### `Nst_sbuffer_copy`

**Synopsis:**

```better-c
bool Nst_sbuffer_copy(Nst_SBuffer *src, Nst_SBuffer *dst)
```

**Description:**

Copies the contents of a sized buffer into another.

The data of the source buffer is copied into a new block of memory, subsequent
changes to the source buffer will not modify the copied one.

**Parameters:**

- `src`: the buffer to copy from
- `dst`: the buffer to copy to

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_sbuffer_destroy`

**Synopsis:**

```better-c
void Nst_sbuffer_destroy(Nst_SBuffer *buf)
```

**Description:**

Destroys the contents of a sized buffer. The buffer itself is not freed.

If the data is set to `NULL` the function returns immediately and and leaves the
buffer untouched.
