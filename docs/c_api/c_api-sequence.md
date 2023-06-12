# `sequence.h`

This header defines the Nest sequence object.

## Macros

### `_NST_VECTOR_MIN_SIZE`

**Description**:

The minimum number of elements inside a vector

---

### `_NST_VECTOR_GROWTH_RATIO`

**Description**:

The radio of growth and shrinking of a vector.

---

### `SEQ`, `ARRAY`, `VECTOR`

**Synopsis**:

```better-c
SEQ(ptr)
ARRAY(ptr)
VECTOR(ptr)
```

**Description**:

These macros cast `ptr` to a `Nst_SeqObj *`.

---

### `nst_seq_set`, `nst_vector_set`, `nst_array_set`

**Synopsis**:

```better-c
nst_seq_set(seq, idx, val)
nst_vector_set nst_seq_set
nst_array_set nst_seq_set
```

**Description**:

Alias for [`_nst_seq_set`](#_nst_seq_set) that casts `seq` to `Nst_SeqObj *` and
`val` to `Nst_Obj *`.

---

### `nst_seq_get`, `nst_vector_get`, `nst_array_get`

**Synopsis**:

```better-c
nst_seq_get(seq, idx)
nst_vector_get nst_seq_get
nst_array_get nst_seq_get
```

**Description**:

Alias for [`_nst_seq_get`](#_nst_seq_get) that casts `seq` to `Nst_SeqObj *`.

---

### `nst_vector_append`

**Synopsis**:

```better-c
nst_vector_append(vect, val, err)
```

**Description**:

Alias for [`_nst_vector_append`](#_nst_vector_append) that casts `vect` to
`Nst_SeqObj *` and `val` to `Nst_Obj *`.

---

### `nst_vector_remove`

**Synopsis**:

```better-c
nst_vector_remove(vect, val)
```

**Description**:

Alias for [`_nst_vector_remove`](#_nst_vector_remove) that casts `vect` to
`Nst_SeqObj *`.

---

### `nst_vector_pop`

**Synopsis**:

```better-c
nst_vector_pop(vect, quantity)
```

**Description**:

Alias for [`_nst_vector_pop`](#_nst_vector_pop) that casts `vect` to
`Nst_SeqObj *`.

---

## Structs

### `Nst_SeqObj`

**Synopsis**:

```better-c
typedef struct _Nst_SeqObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    Nst_Obj **objs;
    usize len;
    usize size;
}
Nst_SeqObj
```

**Description**:

The structure that defines a Nest sequence object.

**Fields**:

- `objs`: the objects array
- `len`: the number of objects in the sequence
- `size`: the maximum number of objects that can be added to the sequence before
  having to allocate more memory, for arrays this number coincides with `len`

---

## Type aliases

### `Nst_ArrayObj`, `Nst_VectorObj`

**Synopsis**:

```better-c
typedef Nst_SeqObj Nst_ArrayObj
typedef Nst_SeqObj Nst_VectorObj
```

---

## Functions

### `nst_array_new`

**Synopsis**:

```better-c
Nst_Obj *nst_array_new(usize len, Nst_OpErr *err)
```

**Description**:

Creates a new array of length `len`, the objects must be set manually inside.

**Arguments**:

- `[in] len`: the length of the array
- `[out] err`: the error

**Return value**:

The function returns the new array on success and `NULL` on failure.

---

### `nst_vector_new`

**Synopsis**:

```better-c
Nst_Obj *nst_vector_new(usize len, Nst_OpErr *err)
```

**Description**:

Creates a new vector of length `len`, the objects must be set manually inside.

**Arguments**:

- `[in] len`: the length of the vector
- `[out] err`: the error

**Return value**:

The function returns the new vector on success and `NULL` on failure.

---

### `nst_array_create`

**Synopsis**:

```better-c
Nst_Obj *nst_array_create(usize len, Nst_OpErr *err, ...)
```

**Description**:

Creates a new array inserting the values. The function takes one reference of
the values, regardless of success of failure.

**Arguments**:

- `[in] len`: the length of the array to create
- `[out] err`: the error
- `[in] ...`: the objects to insert

**Return value**:

The function returns the new array or `NULL` in case of failure.

**Example**:

```better-c
Nst_Obj *build_pair(Nst_Obj *ob1, Nst_obj *ob2, Nst_OpErr *err)
{
    // add the references for the array
    nst_inc_ref(ob1);
    nst_inc_ref(ob2);
    return nst_array_create(2, err, ob1, ob2);
}
```

!!!note
    The number of objects passed to the function must be equal to the specified
    length

---

### `nst_vector_create`

**Synopsis**:

```better-c
Nst_Obj *nst_vector_create(usize len, Nst_OpErr *err, ...)
```

**Description**:

Creates a new vector inserting the values. The function takes one reference of
the values, regardless of success of failure.

**Arguments**:

- `[in] len`: the length of the vector to create
- `[out] err`: the error
- `[in] ...`: the objects to insert

**Return value**:

The function returns the new vector or `NULL` in case of failure.

!!!note
    The number of objects passed to the function must be equal to the specified
    length

---

### `nst_array_create_c`

**Synopsis**:

```better-c
Nst_Obj *nst_array_create_c(const i8 *fmt, Nst_OpErr *err, ...)
```

**Description**:

Creates a new array from a specified format creating objects from C values.
The length is obtained from the type format.  
See the [sequence type format](#sequence-type-format).

**Arguments**:

- `[in] fmt`: the type format
- `[out] err`: the error
- `[in] ...`: the values to insert

**Return value**:

The function returns the new array or `NULL` on failure.

---

### `nst_vector_create_c`

**Synopsis**:

```better-c
Nst_Obj *nst_vector_create_c(const i8 *fmt, Nst_OpErr *err, ...)
```

**Description**:

Creates a new vector from a specified format creating objects from C values.
The length is obtained from the type format.  
See the [sequence type format](#sequence-type-format).

**Arguments**:

- `[in] fmt`: the type format
- `[out] err`: the error
- `[in] ...`: the values to insert

**Return value**:

The function returns the new vector or `NULL` on failure.

---

### `_nst_seq_set`

**Synopsis**:

```better-c
bool _nst_seq_set(Nst_SeqObj *seq, i64 idx, Nst_Obj *val)
```

**Description**:

Sets a value at index `idx` of `seq` increasing its reference count and
decreasing the one of the replaced object, this cannot be called if there is no
valid object at `idx`.

**Arguments**:

- `[inout] seq`: the sequence to set the object of
- `[in] idx`: the index of the object to set, if negative it starts from the end
  with the last index being `-1`
- `[in] val`: the value to insert

**Return value**:

The function returns `true` if the object was set properly or `false` if `idx`
was outside the sequence.

---

### `_nst_seq_get`

**Synopsis**:

```better-c
Nst_Obj *_nst_seq_get(Nst_SeqObj *seq, i64 idx)
```

**Description**:

Gets the object at index `idx` of `seq`. Negative integers are interpreted as
starting from the end of the sequence with the last item being `-1`, the second
to last item `-2` and so on.

**Arguments**:

- `[in] seq`: the sequence to get the object from
- `[in] idx`: the index of the object

**Return value**:

The function returns the object or `NULL` if the index is outside the sequence.

---

### `_nst_vector_resize`

**Synopsis**:

```better-c
bool _nst_vector_resize(Nst_SeqObj *vect, Nst_OpErr *err)
```

**Description**:

This function expands the vector when it is full and shrinks it when is mostly
empty. The function is guaranteed to succeed when shrinking.

**Arguments**:

- `[inout] vect`: the vector to expand
- `[out] err`: the error

**Return value**:

The function returns `false` on failure and `true` on success. It is guaranteed
to succeed when shrinking the vector.

---

### `_nst_vector_append`

**Synopsis**:

```better-c
bool _nst_vector_append(Nst_SeqObj *vect, Nst_Obj *val, Nst_OpErr *err)
```

**Description**:

Appends a value to a vector.

**Arguments**:

- `[inout] vect`: the vector to append the value to
- `[in] val`: the value to append
- `[out] err`: the error

**Return value**:

The function returns `false` on failure and `true` on success.

---

### `_nst_vector_remove`

**Synopsis**:

```better-c
Nst_Obj *_nst_vector_remove(Nst_SeqObj *vect, Nst_Obj *val)
```

**Description**:

Removes the first occurrence of `val` from a vector shifting the following items
to fill the spot.

**Arguments**:

- `[inout] vect`: the vector to remove the value from
- `[in] val`: an object equal to the value to remove

---

### `_nst_vector_pop`

**Synopsis**:

```better-c
Nst_Obj *_nst_vector_pop(Nst_SeqObj *vect, usize quantity)
```

**Description**:

Pops a certain number of values from the end of a vector.

**Arguments**:

- `[inout] vect`: the vector to pop the values from
- `[in] quantity`: the number of values to remove

**Return value**:

The function returns the last value popped.

---

### Other functions

**Synopsis**:

```better-c
void _nst_seq_destroy(Nst_SeqObj *seq)
void _nst_seq_traverse(Nst_SeqObj *seq)
void _nst_seq_track(Nst_SeqObj *seq)
```

**Description**:

These functions are used to manage sequence objects.

## Sequence type format

The format is a series of letters that specify for each index the the type of
the value that is given to the function.

- `i`: `i32`
- `I`: `i64`
- `f` and `F`: `f64`
- `b`: `bool`
- `B`: `u8`
- `o`: an object that has already a reference for the new sequence
- `O`: an object which needs its reference count to be increased
- `n`: `NULL`

### Example

```better-c
Nst_Int num;
Nst_Obj *obj;
NST_DEF_EXTRACT(iB|i|r:r, &num, &obj);

Nst_Obj *array = nst_array_create_c(
    "iIfbBoOn", err,
    123, // an Int object is created
    num, // an Int object is created here too
    4.3, // a Real object is created
    true, // the value nst_true() is added
    10, // a Byte object is created
    obj, // the object already has a reference because it was casted to Real
    nst_type()->IOFile, // the object does not have an extra reference
    nullptr /* nst_null() is added */);
```
