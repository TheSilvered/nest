# `sequence.h`

`Array` and `Vector` objects interface.

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
    The `fmt` string **cannot** contain whitespace.

---

## Macros

### `_Nst_VECTOR_MIN_CAP`

**Description:**

The minimum capacity of a Vector object.

---

### `_Nst_VECTOR_GROWTH_RATIO`

**Description:**

Growth ratio of a Vector object.

---

## Functions

### `Nst_array_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_array_new(usize len)
```

**Description:**

Creates a new array object of the specified length. The slots are filled with
`null` values.

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

Creates a new vector object of the specified length. The slots are filled with
`null` values.

**Parameters:**

- `len`: the length of the vector to create

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_array_from_objs`

**Synopsis:**

```better-c
Nst_Obj *Nst_array_from_objs(usize len, Nst_Obj **objs)
```

**Description:**

Create a new Array object given an array of objects. A reference is added to
each object.

**Parameters:**

- `len`: the number of objects in `objs`
- `objs`: the objects to use to initialize the array

**Returns:**

The new object on success and `NULL` on failure. The error is set.

---

### `Nst_vector_from_objs`

**Synopsis:**

```better-c
Nst_Obj *Nst_vector_from_objs(usize len, Nst_Obj **objs)
```

**Description:**

Create a new Vector object given an array of objects. A reference is added to
each object.

**Parameters:**

- `len`: the number of objects in `objs`
- `objs`: the objects to use to initialize the array

**Returns:**

The new object on success and `NULL` on failure. The error is set.

---

### `Nst_array_from_objsn`

**Synopsis:**

```better-c
Nst_Obj *Nst_array_from_objsn(usize len, Nst_Obj **objs)
```

**Description:**

Create a new Array object given an array of objects. A reference is taken from
each object.

**Parameters:**

- `len`: the number of objects in `objs`
- `objs`: the objects to use to initialize the array

**Returns:**

The new object on success and `NULL` on failure. The error is set.

---

### `Nst_vector_from_objsn`

**Synopsis:**

```better-c
Nst_Obj *Nst_vector_from_objsn(usize len, Nst_Obj **objs)
```

**Description:**

Create a new Vector object given an array of objects. A reference is taken from
each object.

**Parameters:**

- `len`: the number of objects in `objs`
- `objs`: the objects to use to initialize the array

**Returns:**

The new object on success and `NULL` on failure. The error is set.

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

### `Nst_seq_copy`

**Synopsis:**

```better-c
Nst_Obj *Nst_seq_copy(Nst_Obj *seq)
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
void _Nst_seq_destroy(Nst_Obj *seq)
```

**Description:**

Destructor for sequence objects.

---

### `_Nst_seq_traverse`

**Synopsis:**

```better-c
void _Nst_seq_traverse(Nst_Obj *seq)
```

**Description:**

Traverse function for sequence objects.

---

### `Nst_seq_len`

**Synopsis:**

```better-c
usize Nst_seq_len(Nst_Obj *seq)
```

**Description:**

Get the length of a sequence.

---

### `Nst_vector_cap`

**Synopsis:**

```better-c
usize Nst_vector_cap(Nst_Obj *vect)
```

**Description:**

Get the capacity of a vector.

---

### `_Nst_seq_objs`

**Synopsis:**

```better-c
Nst_Obj **_Nst_seq_objs(Nst_Obj *seq)
```

**Description:**

Get the undelying object array of a sequence. Use this only if you know what you
are doing.

---

### `Nst_seq_set`

**Synopsis:**

```better-c
bool Nst_seq_set(Nst_Obj *seq, i64 idx, Nst_Obj *val)
```

**Description:**

Changes the value of an index in a sequence. Adds a reference to `val`.

`idx` can be negative in which case it is subtracted from the length of the
sequence to get the new index.

**Parameters:**

- `seq`: the sequence to modify
- `idx`: the index to update
- `val`: the value to set the index to

**Returns:**

`true` on success and `false` on failure. The error is set. This function fails
when the index is outside the sequence.

---

### `Nst_seq_setf`

**Synopsis:**

```better-c
void Nst_seq_setf(Nst_Obj *seq, usize idx, Nst_Obj *val)
```

**Description:**

Changes the value of an index in a sequence quickly. Adds a reference to `val`.
Negative indices are not supported.

!!!warning
    Use this function only if you are certain that `idx` is inside `seq`. Bound
    checks are only performed in debug mode.

**Parameters:**

- `seq`: the sequence to modify
- `idx`: the index to update
- `val`: the value to set the index to

---

### `Nst_seq_setn`

**Synopsis:**

```better-c
bool Nst_seq_setn(Nst_Obj *seq, i64 idx, Nst_Obj *val)
```

**Description:**

Changes the value of an index in a sequence. Takes a reference from `val`.

`idx` can be negative in which case it is subtracted from the length of the
sequence to get the new index.

**Parameters:**

- `seq`: the sequence to modify
- `idx`: the index to update
- `val`: the value to set the index to

**Returns:**

`true` on success and `false` on failure. The error is set. This function fails
when the index is outside the sequence.

---

### `Nst_seq_setnf`

**Synopsis:**

```better-c
void Nst_seq_setnf(Nst_Obj *seq, usize idx, Nst_Obj *val)
```

**Description:**

Changes the value of an index in a sequence quickly. Takes a reference from
`val`. Negative indices are not supported.

!!!warning
    Use this function only if you are certain that `idx` is inside `seq`. Bound
    checks are only performed in debug mode.

**Parameters:**

- `seq`: the sequence to modify
- `idx`: the index to update
- `val`: the value to set the index to

---

### `Nst_seq_get`

**Synopsis:**

```better-c
Nst_Obj *Nst_seq_get(Nst_Obj *seq, i64 idx)
```

**Description:**

Gets a reference to a value in a sequence.

**Parameters:**

- `seq`: the sequence to get the value from
- `idx`: the index of the value to get

**Returns:**

A new reference to the object at `idx` on success and `NULL` on failure. The
error is set. The function fails when the index is outside the sequence.

---

### `Nst_seq_getf`

**Synopsis:**

```better-c
Nst_Obj *Nst_seq_getf(Nst_Obj *seq, usize idx)
```

**Description:**

Gets a reference to a value in a sequence quickly.

!!!warning
    Use this function only if you are certain that `idx` is inside `seq`. Bound
    checks are only performed in debug mode.

**Parameters:**

- `seq`: the sequence to get the value from
- `idx`: the index of the value to get

**Returns:**

A new reference to the object at `idx`.

---

### `Nst_seq_getn`

**Synopsis:**

```better-c
Nst_Obj *Nst_seq_getn(Nst_Obj *seq, i64 idx)
```

**Description:**

Gets a value in a sequence without taking a reference.

**Parameters:**

- `seq`: the sequence to get the value from
- `idx`: the index of the value to get

**Returns:**

A pointer to the object at `idx` on success and `NULL` on failure. The error is
set. The function fails when the index is outside the sequence.

---

### `Nst_seq_getnf`

**Synopsis:**

```better-c
Nst_Obj *Nst_seq_getnf(Nst_Obj *seq, usize idx)
```

**Description:**

Gets a value in a sequence without taking a reference quickly.

!!!warning
    Use this function only if you are certain that `idx` is inside `seq`. Bound
    checks are only performed in debug mode.

**Parameters:**

- `seq`: the sequence to get the value from
- `idx`: the index of the value to get

**Returns:**

A pointer to the object at `idx`.

---

### `Nst_vector_append`

**Synopsis:**

```better-c
bool Nst_vector_append(Nst_Obj *vect, Nst_Obj *val)
```

**Description:**

Appends a value to the end of a vector adding a reference to `val`.

**Parameters:**

- `vect`: the vector to append the value to
- `val`: the value to append

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_vector_remove`

**Synopsis:**

```better-c
bool Nst_vector_remove(Nst_Obj *vect, Nst_Obj *val)
```

**Description:**

Removes the first occurrence of a value inside a vector.

**Parameters:**

- `vect`: the vector to remove the value from
- `val`: an object that is equal to the value to remove, the equality is checked
  with [`Nst_obj_eq`](c_api-obj_ops.md#nst_obj_eq)

**Returns:**

`true` if the object was removed and `false` if there was no object that
matched. No error is set.

---

### `Nst_vector_pop`

**Synopsis:**

```better-c
Nst_Obj *Nst_vector_pop(Nst_Obj *vect, usize quantity)
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
