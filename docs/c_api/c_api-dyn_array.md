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

- `len`: the number of objects currently in `data`
- `cap`: the capacity of `data`
- `unit_size`: the size in bytes of one object
- `data`: the array of objects

---

### `Nst_PtrArray`

**Synopsis:**

```better-c
typedef struct _Nst_PtrArray {
    usize len;
    usize cap;
    void **data;
} Nst_PtrArray
```

**Description:**

Structure representing a dynamic array of pointers.

**Fields:**

- `len`: the number of pointers currently in in `data`
- `cap`: the capacity of `data`
- `data`: the array of pointers

---

## Functions

### `Nst_da_init`

**Synopsis:**

```better-c
bool Nst_da_init(Nst_DynArray *arr, usize unit_size, usize reserve)
```

**Description:**

Initialize a [`Nst_DynArray`](c_api-dyn_array.md#nst_dynarray).

If `reserve` is `0` no memory is allocated.

**Parameters:**

- `buf`: the buffer to initialize
- `unit_size`: the size of the elements the array will contain
- `reserve`: the capacity to initialize the array with

**Returns:**

`true` on succes and `false` on failure. The error is set.

---

### `Nst_da_init_copy`

**Synopsis:**

```better-c
bool Nst_da_init_copy(Nst_DynArray *src, Nst_DynArray *dst)
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
bool Nst_da_pop(Nst_DynArray *arr, Nst_Destructor dstr)
```

**Description:**

Pop the last element of the array.

**Returns:**

`true` if the element was popped successfully and `false` if there was no item
to pop. No error is set.

---

### `Nst_da_remove_swap`

**Synopsis:**

```better-c
bool Nst_da_remove_swap(Nst_DynArray *arr, usize index, Nst_Destructor dstr)
```

**Description:**

Remove the element of an array at `index` and put the last element of the array
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
bool Nst_da_remove_shift(Nst_DynArray *arr, usize index, Nst_Destructor dstr)
```

**Description:**

Remove the element of an array at `index` and puts the last element of the array
in its place.

!!!note
    This function operates in linear time.

**Returns:**

`true` if the element was removed successfully and `false` if there was no
element to remove. No error is set.

---

### `Nst_da_get`

**Synopsis:**

```better-c
void *Nst_da_get(Nst_DynArray *arr, usize index)
```

**Description:**

Get the pointer to the element of an array at `index`.

**Returns:**

A pointer to the start of the element in the array or `NULL` if the index is out
of bounds. No error is set.

---

### `Nst_da_clear`

**Synopsis:**

```better-c
void Nst_da_clear(Nst_DynArray *arr, Nst_Destructor dstr)
```

**Description:**

Clear the contents of an array freeing any allocated memory.

---

### `Nst_da_set`

**Synopsis:**

```better-c
void Nst_da_set(Nst_DynArray *arr, usize index, void *element,
                Nst_Destructor dstr)
```

**Description:**

Set an element in an [`Nst_DynArray`](c_api-dyn_array.md#nst_dynarray).

**Parameters:**

- `arr`: the array to modify
- `index`: the index to set
- `element`: a pointer to the value to copy
- `dstr`: the destructor used to destroy the previous element at `index`

---

### `Nst_pa_init`

**Synopsis:**

```better-c
bool Nst_pa_init(Nst_PtrArray *arr, usize reserve)
```

**Description:**

Initialize a [`Nst_PtrArray`](c_api-dyn_array.md#nst_ptrarray).

If `reserve` is `0` no memory is allocated.

**Parameters:**

- `buf`: the buffer to initialize
- `reserve`: the capacity to initialize the array with

**Returns:**

`true` on succes and `false` on failure. The error is set.

---

### `Nst_pa_init_copy`

**Synopsis:**

```better-c
bool Nst_pa_init_copy(Nst_PtrArray *src, Nst_PtrArray *dst)
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

### `Nst_pa_reserve`

**Synopsis:**

```better-c
bool Nst_pa_reserve(Nst_PtrArray *arr, usize amount)
```

**Description:**

Reserve `amount` free slots to avoid reallocations.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_pa_append`

**Synopsis:**

```better-c
bool Nst_pa_append(Nst_PtrArray *arr, void *element)
```

**Description:**

Append an element to the end of the array.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_pa_pop`

**Synopsis:**

```better-c
bool Nst_pa_pop(Nst_PtrArray *arr, Nst_Destructor dstr)
```

**Description:**

Pop the last element of the array.

**Returns:**

`true` if the element was popped successfully and `false` if there was no item
to pop. No error is set.

---

### `Nst_pa_remove_swap`

**Synopsis:**

```better-c
bool Nst_pa_remove_swap(Nst_PtrArray *arr, usize index, Nst_Destructor dstr)
```

**Description:**

Remove the element of an array at `index` and put the last element of the array
in its place.

!!!note
    This function operates in constant time.

**Returns:**

`true` if the element was removed successfully and `false` if there was no
element to remove. No error is set.

---

### `Nst_pa_remove_shift`

**Synopsis:**

```better-c
bool Nst_pa_remove_shift(Nst_PtrArray *arr, usize index, Nst_Destructor dstr)
```

**Description:**

Remove the element of an array at `index` and puts the last element of the array
in its place.

!!!note
    This function operates in linear time.

**Returns:**

`true` if the element was removed successfully and `false` if there was no
element to remove. No error is set.

---

### `Nst_pa_get`

**Synopsis:**

```better-c
void *Nst_pa_get(Nst_PtrArray *arr, usize index)
```

**Description:**

Get the pointer to the element of an array at `index`.

**Returns:**

A pointer to the start of the element in the array or `NULL` if the index is out
of bounds. No error is set.

---

### `Nst_pa_clear`

**Synopsis:**

```better-c
void Nst_pa_clear(Nst_PtrArray *arr, Nst_Destructor dstr)
```

**Description:**

Clear the contents of an array freeing any allocated memory.

---

### `Nst_pa_set`

**Synopsis:**

```better-c
void Nst_pa_set(Nst_PtrArray *arr, usize index, void *element,
                Nst_Destructor dstr)
```

**Description:**

Set an element in an [`Nst_PtrArray`](c_api-dyn_array.md#nst_ptrarray).

**Parameters:**

- `arr`: the array to modify
- `index`: the index to set
- `element`: a pointer to the value to copy
- `dstr`: the destructor used to destroy the previous element at `index`
