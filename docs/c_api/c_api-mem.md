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

### `Nst_SizedBuffer`

**Synopsis:**

```better-c
typedef struct _Nst_SizedBuffer {
    usize len;
    usize cap;
    usize unit_size;
    void *data;
} Nst_SizedBuffer
```

**Description:**

Structure representing a buffer of objects with an arbitrary size.

**Fields:**

- `len`: the number of objects currently in the buffer
- `cap`: the size in bytes of the allocated block
- `unit_size`: the size in bytes of one object
- `data`: the array of objects

---

### `Nst_Buffer`

**Synopsis:**

```better-c
typedef struct _Nst_Buffer {
    usize len;
    usize cap;
    usize unit_size;
    i8 *data;
} Nst_Buffer
```

**Description:**

Structure representing a buffer of chars.

Uses the same layout of [`Nst_SizedBuffer`](c_api-mem.md#nst_sizedbuffer) to
re-use the same functions. Ensures to always contain a valid string if not
modified by external functions.

**Fields:**

- `len`: the length of the string in the buffer
- `cap`: the size in bytes of the allocated block
- `unit_size`: always `1`
- `data`: the string

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
void Nst_log_alloc_count()
```

**Description:**

Prints the current allocation count to `stdout`. Declared only if
[`Nst_COUNT_ALLOC`](c_api-typedefs.md#nst_count_alloc) is defined.

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

### `Nst_sbuffer_init`

**Synopsis:**

```better-c
bool Nst_sbuffer_init(Nst_SizedBuffer *buf, usize unit_size, usize count)
```

**Description:**

Initializes a [`Nst_SizedBuffer`](c_api-mem.md#nst_sizedbuffer).

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
bool Nst_sbuffer_expand_by(Nst_SizedBuffer *buf, usize amount)
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
bool Nst_sbuffer_expand_to(Nst_SizedBuffer *buf, usize count)
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
void Nst_sbuffer_fit(Nst_SizedBuffer *buf)
```

**Description:**

Shrinks the capacity of a sized buffer to match its length.

---

### `Nst_sbuffer_append`

**Synopsis:**

```better-c
bool Nst_sbuffer_append(Nst_SizedBuffer *buf, void *element)
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
bool Nst_sbuffer_pop(Nst_SizedBuffer *buf)
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
void *Nst_sbuffer_at(Nst_SizedBuffer *buf, usize index)
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
void Nst_sbuffer_shrink_auto(Nst_SizedBuffer *buf)
```

**Description:**

Shrinks the size of a sized buffer.

The size is not shrunk to the minimum but some slots are kept for possible new
values.

**Parameters:**

- `buf`: the buffer to shrink

---

### `Nst_sbuffer_shrink_min`

**Synopsis:**

```better-c
void Nst_sbuffer_shrink_min(Nst_SizedBuffer *buf)
```

**Description:**

Shrinks the size of a sized buffer to the smallest it can be.

**Parameters:**

- `buf`: the buffer to shrink

---

### `Nst_sbuffer_copy`

**Synopsis:**

```better-c
bool Nst_sbuffer_copy(Nst_SizedBuffer *src, Nst_SizedBuffer *dst)
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
void Nst_sbuffer_destroy(Nst_SizedBuffer *buf)
```

**Description:**

Destroys the contents of a sized buffer. The buffer itself is not freed.

---

### `Nst_buffer_init`

**Synopsis:**

```better-c
bool Nst_buffer_init(Nst_Buffer *buf, usize initial_size)
```

**Description:**

Initializes a [`Nst_Buffer`](c_api-mem.md#nst_buffer).

**Parameters:**

- `buf`: the buffer to initialize
- `initial_size`: the initial capacity of the buffer

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_buffer_expand_by`

**Synopsis:**

```better-c
bool Nst_buffer_expand_by(Nst_Buffer *buf, usize amount)
```

**Description:**

Expands a buffer to contain a specified amount new characters.

The buffer is expanded only if needed. `1` is added to `amount` to take into
account the `NUL` character at the end.

**Parameters:**

- `buf`: the buffer to expand
- `amount`: the number of new characters the buffer needs to contain

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_buffer_expand_to`

**Synopsis:**

```better-c
bool Nst_buffer_expand_to(Nst_Buffer *buf, usize size)
```

**Description:**

Expands a sized buffer to contain a total amount of characters.

The buffer is expanded only if needed. If the new size is smaller than the
current one nothing is done. `1` is added to the size to take into account the
`NUL` character at the end.

**Parameters:**

- `buf`: the buffer to expand
- `amount`: the number of characters the buffer needs to contain

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_buffer_fit`

**Synopsis:**

```better-c
void Nst_buffer_fit(Nst_Buffer *buf)
```

**Description:**

Shrinks the capacity of a buffer to match its length.

---

### `Nst_buffer_append`

**Synopsis:**

```better-c
bool Nst_buffer_append(Nst_Buffer *buf, Nst_StrObj *str)
```

**Description:**

Appends a [`Nst_StrObj`](c_api-str.md#nst_strobj) to the end of the buffer.

The buffer is expanded if needed.

**Parameters:**

- `buf`: the buffer to append the string to
- `str`: the string to append

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_buffer_append_c_str`

**Synopsis:**

```better-c
bool Nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str)
```

**Description:**

Appends a C string to the end of the buffer.

The buffer is expanded if needed.

**Parameters:**

- `buf`: the buffer to append the string to
- `str`: the string to append

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_buffer_append_str`

**Synopsis:**

```better-c
bool Nst_buffer_append_str(Nst_Buffer *buf, i8 *str, usize len)
```

**Description:**

Appends a string of a known length to the end of the buffer.

**Parameters:**

- `buf`: the buffer to append the string to
- `str`: the string to append
- `len`: the length of the string to append, excluding the NUL character

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_buffer_append_char`

**Synopsis:**

```better-c
bool Nst_buffer_append_char(Nst_Buffer *buf, i8 ch)
```

**Description:**

Appends a character to the end of the buffer.

The buffer is expanded if needed.

**Parameters:**

- `buf`: the buffer to append the string to
- `ch`: the character to append

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_buffer_to_string`

**Synopsis:**

```better-c
Nst_StrObj *Nst_buffer_to_string(Nst_Buffer *buf)
```

**Description:**

Creates a [`Nst_StrObj`](c_api-str.md#nst_strobj) from a buffer.

The data of the buffer is set to `NULL` and its `len` and `cap` are set to `0`.
The function automatically calls
[`Nst_buffer_fit`](c_api-mem.md#nst_buffer_fit).

**Parameters:**

- `buf`: the buffer to create the string from

**Returns:**

The new string on success and `NULL` on failure. The error is set.

---

### `Nst_buffer_copy`

**Synopsis:**

```better-c
bool Nst_buffer_copy(Nst_Buffer *src, Nst_Buffer *dst)
```

**Description:**

Copies the contents of a buffer into another.

The data of the source buffer is copied into a new block of memory, subsequent
changes to the source buffer will not modify the copied one.

**Parameters:**

- `src`: the buffer to copy from
- `dst`: the buffer to copy to

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_buffer_destroy`

**Synopsis:**

```better-c
void Nst_buffer_destroy(Nst_Buffer *buf)
```

**Description:**

Destroys the contents of a buffer. The buffer itself is not freed.
