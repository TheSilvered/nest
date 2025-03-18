# `dyn_array.h`

Dynamic heap-allocated array.

## Authors

TheSilvered

---

## Structs

### `Nst_DynArray`

**Synopsis:**

```better-c
typedef struct _Nst_DynArray {
    usize len;
    usize cap;
    usize unit_size;
    void *data;
} Nst_DynArray
```

**Description:**

Structure representing a dynamic array of objects with an arbitrary size.

**Fields:**

- `len`: the number of objects currently in the buffer
- `cap`: the size in bytes of the allocated block
- `unit_size`: the size in bytes of one object
- `data`: the array of objects

---

## Functions

### `Nst_da_init`

**Synopsis:**

```better-c
bool Nst_da_init(Nst_DynArray *arr, usize unit_size, usize reserve)
```

**Description:**

Initializes a [`Nst_DynArray`](c_api-dyn_array.md#nst_dynarray).

**Parameters:**

- `buf`: the buffer to initialize
- `unit_size`: the size of the elements the array will contain
- `reserve`: the number of elements to initialize the array with

**Returns:**

`true` on succes and `false` on failure. The error is set.

---

### `Nst_da_init_copy`

**Synopsis:**

```better-c
bool Nst_da_init_copy(Nst_DynArray *arr, Nst_DynArray *dst)
```

**Description:**

Copy the contents of an arry into another.

The data of the source buffer is copied into a new block of memory, subsequent
changes to the source will not modify the copy.

**Parameters:**

- `src`: the array to copy from
- `dst`: the array to copy to

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_da_reserve`

**Synopsis:**

```better-c
bool Nst_da_reserve(Nst_DynArray *arr, usize amount)
```

**Description:**

Reserve `amount` free slots to avoid reallocations.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_da_append`

**Synopsis:**

```better-c
bool Nst_da_append(Nst_DynArray *arr, void *element)
```

**Description:**

Append an element to the end of the array.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_da_pop`

**Synopsis:**

```better-c
bool Nst_da_pop(Nst_DynArray *arr)
```

**Description:**

Pops the last element of the array.

**Returns:**

`true` if the element was popped successfully and `false` if there was no item
to pop. No error is set.

---

### `Nst_da_remove_swap`

**Synopsis:**

```better-c
bool Nst_da_remove_swap(Nst_DynArray *arr, usize index)
```

**Description:**

Remove the element of an array at `index` and puts the last element of the array
in its place.

!!!note
    This function operates in constant time.

**Returns:**

`true` if the element was removed successfully and `false` if there was no
element to remove. No error is set.

---

### `Nst_da_remove_shift`

**Synopsis:**

```better-c
bool Nst_da_remove_shift(Nst_DynArray *arr, usize index)
```

**Description:**

Remove the element of an array at `index` and puts the last element of the array
in its place

!!!note
    This function operates in linear time.

**Returns:**

`true` if the element was removed successfully and `false` if there was no
element to remove. No error is set.

---

### `Nst_da_at`

**Synopsis:**

```better-c
void *Nst_da_at(Nst_DynArray *arr, usize index)
```

**Description:**

Get the element of an array at `index`.

**Returns:**

A pointer to the start of the element in the array or `NULL` if the index is out
of bounds. No error is set.

---

### `Nst_da_clear`

**Synopsis:**

```better-c
void Nst_da_clear(Nst_DynArray *arr)
```

**Description:**

Clear the contents of an array freeing any allocated memory.
