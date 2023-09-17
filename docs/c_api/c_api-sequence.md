# `sequence.h`

Nst_SeqObject interface for Arrays and Vectors.

## Authors

TheSilvered

## Sequence creation format types

- `I`: `Int` from a 64-bit integer
- `i`: `Int` from a 32-bit integer
- `f`, `F`: `Real` from a double
- `b`: `Bool` from a boolean (promoted to an int)
- `B`: `Byte` from a 8-bit integer
- `o`: an already existing object to take one reference from
- `O`: an already existing object to add one reference to
- `n`: `null`, the vararg can be any pointer as its value is ignored but `NULL`
  is preferred

!!!note
    The `fmt` string **cannot** contain whitespace since its length is used to
    determine the length of the sequence.

---

## Macros

### `Nst_seq_set`

**Synopsis:**

```better-c
Nst_seq_set(seq, idx, val)
```

**Description:**

Alias for [`_Nst_seq_set`](c_api-sequence.md#_nst_seq_set) that casts `seq` to
[`Nst_SeqObj *`](c_api-sequence.md#nst_seqobj) and `val` to
[`Nst_Obj *`](c_api-obj.md#nst_obj).

---

### `Nst_seq_get`

**Synopsis:**

```better-c
Nst_seq_get(seq, idx)
```

**Description:**

Alias for [`_Nst_seq_get`](c_api-sequence.md#_nst_seq_get) that casts `seq` to
[`Nst_SeqObj *`](c_api-sequence.md#nst_seqobj).

---

### `Nst_seq_copy`

**Synopsis:**

```better-c
Nst_seq_copy(seq)
```

**Description:**

Alias for [`_Nst_seq_copy`](c_api-sequence.md#_nst_seq_copy) that casts `seq` to
[`Nst_SeqObj *`](c_api-sequence.md#nst_seqobj).

---

### `Nst_vector_set`

**Description:**

Alias of [`Nst_seq_set`](c_api-sequence.md#nst_seq_set).

---

### `Nst_vector_get`

**Description:**

Alias of [`Nst_seq_get`](c_api-sequence.md#nst_seq_get).

---

### `Nst_array_set`

**Description:**

Alias of [`Nst_seq_set`](c_api-sequence.md#nst_seq_set).

---

### `Nst_array_get`

**Description:**

Alias of [`Nst_seq_set`](c_api-sequence.md#nst_seq_set).

---

### `Nst_vector_append`

**Synopsis:**

```better-c
Nst_vector_append(vect, val)
```

**Description:**

Alias of [`_Nst_vector_append`](c_api-sequence.md#_nst_vector_append) that casts
`vect` to [`Nst_SeqObj *`](c_api-sequence.md#nst_seqobj) and `val` to
[`Nst_Obj *`](c_api-obj.md#nst_obj).

---

### `Nst_vector_remove`

**Synopsis:**

```better-c
Nst_vector_remove(vect, val)
```

**Description:**

Alias of [`_Nst_vector_remove`](c_api-sequence.md#_nst_vector_remove) that casts
`vect` to [`Nst_SeqObj *`](c_api-sequence.md#nst_seqobj) and `val` to
[`Nst_Obj *`](c_api-obj.md#nst_obj).

---

### `Nst_vector_pop`

**Synopsis:**

```better-c
Nst_vector_pop(vect, quantity)
```

**Description:**

Alias of [`_Nst_vector_pop`](c_api-sequence.md#_nst_vector_pop) that casts
`vect` to [`Nst_SeqObj *`](c_api-sequence.md#nst_seqobj).

---

## Structs

### `Nst_SeqObj`

**Synopsis:**

```better-c
typedef struct _Nst_SeqObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_Obj **objs;
    usize len;
    usize cap;
} Nst_SeqObj
```

**Description:**

A structure representing a Nest sequence object.

**Fields:**

- `objs`: the array of objects inside the sequence
- `len`: the length of the sequence
- `cap`: the capacity of the sequence

---

## Type aliases

### `Nst_ArrayObj`

**Synopsis:**

```better-c
typedef Nst_SeqObj Nst_ArrayObj;
```

**Description:**

Type added for C type completion.

---

### `Nst_VectorObj`

**Synopsis:**

```better-c
typedef Nst_SeqObj Nst_VectorObj;
```

**Description:**

Type added for C type completion.

---

## Functions

### `Nst_array_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_array_new(usize len)
```

**Description:**

Creates a new array object of the specified length.

!!!warning
    The objects inside the array must be set manually, each entry in the array
    takes one reference of the object inserted.

**Parameters:**

- `len`: the length of the array to create

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_vector_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_vector_new(usize len)
```

**Description:**

Creates a new vector object of the specified length.

!!!warning
    The objects inside the vector must be set manually, each entry in the vector
    takes one reference of the object inserted.

**Parameters:**

- `len`: the length of the vector to create

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_array_create`

**Synopsis:**

```better-c
Nst_Obj *Nst_array_create(usize len, ...)
```

**Description:**

Creates an array object of the length specified, inserting the objects inside.

The number of varargs passed to the function must match the number given in the
len parameter.

**Parameters:**

- `len`: the length of the array to create
- `...`: the objects to insert in the array, a reference is taken from each
  object

**Returns:**

The new array on success or `NULL` on failure. The error is set.

---

### `Nst_vector_create`

**Synopsis:**

```better-c
Nst_Obj *Nst_vector_create(usize len, ...)
```

**Description:**

Creates a vector object of the length specified, inserting the objects inside.

The number of varargs passed to the function must match the number given in the
`len` parameter.

**Parameters:**

- `len`: the length of the vector to create
- `...`: the objects to insert in the vector, a reference is taken from each
  object

**Returns:**

The new vector on success or `NULL` on failure. The error is set.

---

### `Nst_array_create_c`

**Synopsis:**

```better-c
Nst_Obj *Nst_array_create_c(const i8 *fmt, ...)
```

**Description:**

Creates an array object, creating the contained objects from C values.

The number of varargs passed to the function must match the number of types in
the fmt argument. For more information about the `fmt` argument check the
documentation in
[`sequence.h`](c_api-sequence.md#sequence-creation-format-types)

**Parameters:**

- `fmt`: the types of the values passed to the function
- `...`: the values passed to the function used to create the objects

**Returns:**

The new array on success or `NULL` on failure. The error is set.

---

### `Nst_vector_create_c`

**Synopsis:**

```better-c
Nst_Obj *Nst_vector_create_c(const i8 *fmt, ...)
```

**Description:**

Creates a vector object, creating the contained objects from C values.

The number of varargs passed to the function must match the number of types in
the fmt argument. For more information about the `fmt` argument check the
documentation in
[`sequence.h`](c_api-sequence.md#sequence-creation-format-types)

**Parameters:**

- `fmt`: the types of the values passed to the function
- `...`: the values passed to the function used to create the objects

**Returns:**

The new array on success or `NULL` on failure. The error is set.

---

### `_Nst_seq_copy`

**Synopsis:**

```better-c
Nst_Obj *_Nst_seq_copy(Nst_SeqObj *seq)
```

**Description:**

Creates a shallow copy of a sequence.

**Parameters:**

- `seq`: the sequence to copy

**Returns:**

The new sequence or NULL on failure. The error is set.

---

### `_Nst_seq_destroy`

**Synopsis:**

```better-c
void _Nst_seq_destroy(Nst_SeqObj *seq)
```

**Description:**

Destructor for sequence objects.

---

### `_Nst_seq_traverse`

**Synopsis:**

```better-c
void _Nst_seq_traverse(Nst_SeqObj *seq)
```

**Description:**

Traverse function for sequence objects.

---

### `_Nst_seq_set`

**Synopsis:**

```better-c
bool _Nst_seq_set(Nst_SeqObj *seq, i64 idx, Nst_Obj *val)
```

**Description:**

Changes the value of an index in a sequence.

The sequence to change must already contain valid values. `idx` can be negative
in which case it is subtracted from the length of the sequence to get the new
index.

**Parameters:**

- `seq`: the sequence to modify
- `idx`: the index to update
- `val`: the value to set the index to

**Returns:**

`true` on success and `false` on failure. The error is set. This function fails
when the index is outside the sequence.

---

### `_Nst_seq_get`

**Synopsis:**

```better-c
Nst_Obj *_Nst_seq_get(Nst_SeqObj *seq, i64 idx)
```

**Description:**

Gets the value at an index of a sequence.

The sequence must already contain valid values.

**Parameters:**

- `seq`: the sequence to get the value from
- `idx`: the index of the value to get

**Returns:**

A new reference to the object at `idx` on success and `NULL` on failure. The
error is set. The function fails when the index is outside the sequence.

---

### `_Nst_vector_resize`

**Synopsis:**

```better-c
bool _Nst_vector_resize(Nst_SeqObj *vect)
```

**Description:**

Resizes a vector if needed.

**Parameters:**

- `vect`: the vector to resize

**Returns:**

`true` on success and `false` on failure. The error is set. The function never
fails if the vector is untouched or is shrunk.

---

### `_Nst_vector_append`

**Synopsis:**

```better-c
bool _Nst_vector_append(Nst_SeqObj *vect, Nst_Obj *val)
```

**Description:**

Appends a value to the end of a vector.

**Parameters:**

- `vect`: the vector to append the value to
- `val`: the value to append

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `_Nst_vector_remove`

**Synopsis:**

```better-c
bool _Nst_vector_remove(Nst_SeqObj *vect, Nst_Obj *val)
```

**Description:**

Removes the first occurrence of a value inside a vector.

**Parameters:**

- `vect`: the vector to remove the value from
- `val`: an object that is equal to the value to remove, the equality is checked
  with `Nst_obj_eq`

**Returns:**

`true` if the object was removed and `false` if there was no object that
matched. No error is set.

---

### `_Nst_vector_pop`

**Synopsis:**

```better-c
Nst_Obj *_Nst_vector_pop(Nst_SeqObj *vect, usize quantity)
```

**Description:**

Pops a certain number of values from the end of a vector.

If the quantity is greater than the length of the vector, it is adapted and the
function does not fail.

**Parameters:**

- `vect`: the vector to pop the values from
- `quantity`: the number of values to pop

**Returns:**

The last value popped or `NULL` if no value was popped. No error is set.

