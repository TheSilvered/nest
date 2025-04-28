# `simple_types.h`

Interface for `Int`, `Real`, `Byte` and `Bool`.

## Authors

TheSilvered

---

## Functions

### `Nst_int_new`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_int_new(i64 value)
```

**Description:**

Create a new `Int` object.

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_int_i64`

**Synopsis:**

```better-c
i64 Nst_int_i64(Nst_Obj *obj)
```

**Returns:**

The value of an `Int` object as an [`i64`](c_api_index.md#type-definitions).

---

### `Nst_real_new`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_real_new(f64 value)
```

**Description:**

Create a new `Real` object.

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_real_f64`

**Synopsis:**

```better-c
f64 Nst_real_f64(Nst_Obj *obj)
```

**Returns:**

The value of a `Real` object as an [`f64`](c_api_index.md#type-definitions).

---

### `Nst_real_f32`

**Synopsis:**

```better-c
f32 Nst_real_f32(Nst_Obj *obj)
```

**Returns:**

The value of a `Real` object as an [`f32`](c_api_index.md#type-definitions).

---

### `Nst_byte_new`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_byte_new(u8 value)
```

**Description:**

Create a new `Byte` object.

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_byte_u8`

**Synopsis:**

```better-c
u8 Nst_byte_u8(Nst_Obj *obj)
```

**Returns:**

The value of a `Byte` object.

---

### `Nst_number_to_u8`

**Synopsis:**

```better-c
u8 Nst_number_to_u8(Nst_Obj *number)
```

**Description:**

Convert the value of a numeric object (`Int`, `Real`, `Byte`) to a
[`u8`](c_api_index.md#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0` is returned. No
error is set.

---

### `Nst_number_to_int`

**Synopsis:**

```better-c
int Nst_number_to_int(Nst_Obj *number)
```

**Description:**

Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an `int`.

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0` is returned. No
error is set.

---

### `Nst_number_to_i32`

**Synopsis:**

```better-c
i32 Nst_number_to_i32(Nst_Obj *number)
```

**Description:**

Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an
[`i32`](c_api_index.md#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0` is returned. No
error is set.

---

### `Nst_number_to_i64`

**Synopsis:**

```better-c
i64 Nst_number_to_i64(Nst_Obj *number)
```

**Description:**

Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an
[`i64`](c_api_index.md#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0` is returned. No
error is set.

---

### `Nst_number_to_f32`

**Synopsis:**

```better-c
f32 Nst_number_to_f32(Nst_Obj *number)
```

**Description:**

Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an
[`f32`](c_api_index.md#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0.0` is returned. No
error is set.

---

### `Nst_number_to_f64`

**Synopsis:**

```better-c
f64 Nst_number_to_f64(Nst_Obj *number)
```

**Description:**

Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an
[`f64`](c_api_index.md#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object `0.0f` is returned. No
error is set.

---

### `Nst_obj_to_bool`

**Synopsis:**

```better-c
bool Nst_obj_to_bool(Nst_Obj *obj)
```

**Returns:**

`true` if the object is truthy and `false` otherwise.
