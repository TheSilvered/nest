# `mem.h`

Heap & dynamic memory management functions.

## Authors

TheSilvered

## Macros

### `Nst_raw_free`

**Description:**

Alias for C free.

---

### `Nst_malloc_c`

**Synopsis:**

```better-c
Nst_malloc_c(count, type)
```

**Description:**

Calls Nst_malloc using sizeof(type) for the size and casting the result to a
pointer of the type.

---

### `Nst_calloc_c`

**Synopsis:**

```better-c
Nst_calloc_c(count, type, init_value)
```

**Description:**

Calls Nst_calloc using sizeof(type) for the size and casting the result to a
pointer of the type.

---

### `Nst_realloc_c`

**Synopsis:**

```better-c
Nst_realloc_c(block, new_count, type, count)
```

**Description:**

Calls Nst_realloc using sizeof(type) for the size and casting the result to a
pointer of the type.

---

### `Nst_crealloc_c`

**Synopsis:**

```better-c
Nst_crealloc_c(block, new_count, type, count, init_value)
```

**Description:**

Calls Nst_crealloc using sizeof(type) for the size and casting the result to a
pointer of the type.

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

Uses the same layout of Nst_SizedBuffer to re-use the same functions. Ensures to
always contain a valid string if not modified by custom functions.

**Fields:**

- `len`: the length of the string in the buffer
- `cap`: the size in bytes of the allocated block
- `unit_size`: always 1
- `data`: the string

---

## Functions

### `Nst_raw_malloc`

**Synopsis:**

```better-c
void *Nst_raw_malloc(usize size)
```

**Description:**

Alias for C malloc.

---

### `Nst_raw_calloc`

**Synopsis:**

```better-c
void *Nst_raw_calloc(usize count, usize size)
```

**Description:**

Alias for C calloc.

---

### `Nst_raw_realloc`

**Synopsis:**

```better-c
void *Nst_raw_realloc(void *block, usize size)
```

**Description:**

Alias for C realloc.

---

### `Nst_free`

**Synopsis:**

```better-c
void Nst_free(void *block)
```

**Description:**

Alias for C free.

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

A pointer to the allocated memory block or NULL on failure. The error is set.

---

### `Nst_calloc`

**Synopsis:**

```better-c
void *Nst_calloc(usize count, usize size, void *init_value)
```

**Description:**

Allocates memory on the heap initializing it.

The elements are contiguous in memory. If init_value is NULL, the function has a
similar behaviour to calloc filling the memory with zeroes. init_value is
expected to be the same size as the one given for the elements.

**Parameters:**

- `count`: the number of elements to allocate
- `size`: the size in bytes of each element
- `init_value`: a pointer to the value to initialize each element with

**Returns:**

A pointer to the allocated memory block or NULL on failure. The error is set.

---

### `Nst_realloc`

**Synopsis:**

```better-c
void *Nst_realloc(void *block, usize new_count, usize size, usize count)
```

**Description:**

Changes the size of an allocated memory block.

This function never fails when the block is shrinked because if the call to
realloc fails, the old block is returned.

**Parameters:**

- `block`: the block to reallocate
- `new_count`: the new number of elements of the block
- `size`: the size in bytes of each element
- `count`: the current number of elements in the block

**Returns:**

A pointer to the reallocated memory block or NULL on failure. The error is set.
If either new_count or size is zero, block is freed and NULL is returned with no
error.

---

### `Nst_crealloc`

**Synopsis:**

```better-c
void *Nst_crealloc(void *block, usize new_count, usize size, usize count,
                   void *init_value)
```

**Description:**

Changes the size of an allocated memory block initializing new memory.

This function never fails when the block is shrinked because if the call to
realloc fails, the old block is returned. If init_value is NULL, the function
just fills the new memory with zeroes. init_value is expected to be the same
size as the one given for the elements.

**Parameters:**

- `block`: the block to reallocate
- `new_count`: the new number of elements of the block
- `size`: the size in bytes of each element
- `count`: the current number of elements in the block
- `init_value`: a pointer to the value to initialize the new elements with

**Returns:**

A pointer to the reallocated memory block or NULL on failure. The error is set.
If either new_count or size is zero, block is freed and NULL is returned with no
error.

---

### `Nst_sbuffer_init`

**Synopsis:**

```better-c
bool Nst_sbuffer_init(Nst_SizedBuffer *buf, usize unit_size, usize count)
```

**Description:**

Initializes a Nst_SizedBuffer.

**Parameters:**

- `buf`: the buffer to initialize
- `unit_size`: the size of the elements the buffer will contain
- `count`: the number of elements to initialize the buffer with

**Returns:**

true on succes and false on failure. The error is set.

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

true on succes and false on failure. The error is set.

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

true on succes and false on failure. The error is set.

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

If necessary, the buffer is expanded automatically. The data pointed to by
element is expected to be at least a number of bytes that matches the one of the
elements contained by the buffer.

**Parameters:**

- `buf`: the buffer to append the element to
- `element`: a pointer to the element to append

**Returns:**

true on success and false on failure. The error is set.

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

Initializes a Nst_Buffer.

**Parameters:**

- `buf`: the buffer to initialize
- `initial_size`: the initial capacity of the buffer

**Returns:**

true on succes and false on failure. The error is set.

---

### `Nst_buffer_expand_by`

**Synopsis:**

```better-c
bool Nst_buffer_expand_by(Nst_Buffer *buf, usize amount)
```

**Description:**

Expands a buffer to contain a specified amount new characters.

The buffer is expanded only if needed. One is added to the amount to take into
account the NUL character at the end.

**Parameters:**

- `buf`: the buffer to expand
- `amount`: the number of new characters the buffer needs to contain

**Returns:**

true on succes and false on failure. The error is set.

---

### `Nst_buffer_expand_to`

**Synopsis:**

```better-c
bool Nst_buffer_expand_to(Nst_Buffer *buf, usize size)
```

**Description:**

Expands a sized buffer to contain a total amount of characters.

The buffer is expanded only if needed. If the new size is smaller than the
current one nothing is done. One is added to the size to take into account the
NUL character at the end.

**Parameters:**

- `buf`: the buffer to expand
- `amount`: the number of characters the buffer needs to contain

**Returns:**

true on succes and false on failure. The error is set.

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

Appends a Nst_StrObj to the end of the buffer.

**Parameters:**

- `buf`: the buffer to append the string to
- `str`: the string to append

**Returns:**

true on success and false on failure. The error is set.

---

### `Nst_buffer_append_c_str`

**Synopsis:**

```better-c
bool Nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str)
```

**Description:**

Appends a C string to the end of the buffer.

**Parameters:**

- `buf`: the buffer to append the string to
- `str`: the string to append

**Returns:**

true on success and false on failure. The error is set.

---

### `Nst_buffer_append_char`

**Synopsis:**

```better-c
bool Nst_buffer_append_char(Nst_Buffer *buf, i8 ch)
```

**Description:**

Appends a character to the end of the buffer.

**Parameters:**

- `buf`: the buffer to append the string to
- `ch`: the character to append

**Returns:**

true on success and false on failure. The error is set.

---

### `Nst_buffer_to_string`

**Synopsis:**

```better-c
Nst_StrObj *Nst_buffer_to_string(Nst_Buffer *buf)
```

**Description:**

Creates a Nst_StrObj from a buffer.

The data of the buffer is set to NULL and its len and size are set to 0. The
function automatically calls Nst_buffer_fit.

**Parameters:**

- `buf`: the buffer to create the string from

**Returns:**

The new string on success and NULL on failure. The error is set.

---

### `Nst_buffer_destroy`

**Synopsis:**

```better-c
void Nst_buffer_destroy(Nst_Buffer *buf)
```

**Description:**

Destroys the contents of a buffer. The buffer itself is not freed.

