# `simple_types.h`

Interface for [`Nst_IntObj`](c_api-simple_types.md/#nst_intobj),
[`Nst_RealObj`](c_api-simple_types.md/#nst_realobj),
[`Nst_ByteObj`](c_api-simple_types.md/#nst_byteobj) and
[`Nst_BoolObj`](c_api-simple_types.md/#nst_boolobj).

## Authors

TheSilvered

## Macros

### `AS_INT`

**Synopsis:**

```better-c
AS_INT(ptr)
```

**Description:**

Casts `ptr` to a [`Nst_IntObj *`](c_api-simple_types.md/#nst_intobj) and
extracts the value field.

---

### `AS_REAL`

**Synopsis:**

```better-c
AS_REAL(ptr)
```

**Description:**

Casts `ptr` to a [`Nst_RealObj *`](c_api-simple_types.md/#nst_realobj) and
extracts the value field.

---

### `AS_BYTE`

**Synopsis:**

```better-c
AS_BYTE(ptr)
```

**Description:**

Casts `ptr` to a [`Nst_ByteObj *`](c_api-simple_types.md/#nst_byteobj) and
extracts the value field.

---

### `AS_BOOL`

**Synopsis:**

```better-c
AS_BOOL(ptr)
```

**Description:**

Casts `ptr` to a [`Nst_BoolObj *`](c_api-simple_types.md/#nst_boolobj) and
extracts the value field.

---

### `Nst_number_to_u8`

**Synopsis:**

```better-c
Nst_number_to_u8(number)
```

**Description:**

Alias for [`_Nst_number_to_u8`](c_api-simple_types.md/#_nst_number_to_u8) that
casts `number` to [`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_number_to_int`

**Synopsis:**

```better-c
Nst_number_to_int(number)
```

**Description:**

Alias for [`_Nst_number_to_int `](c_api-simple_types.md/#_nst_number_to_int)that
casts `number` to [`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_number_to_i32`

**Synopsis:**

```better-c
Nst_number_to_i32(number)
```

**Description:**

Alias for [`_Nst_number_to_i32`](c_api-simple_types.md/#_nst_number_to_i32) that
casts `number` to [`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_number_to_i64`

**Synopsis:**

```better-c
Nst_number_to_i64(number)
```

**Description:**

Alias for [`_Nst_number_to_i64`](c_api-simple_types.md/#_nst_number_to_i64) that
casts `number` to [`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_number_to_f32`

**Synopsis:**

```better-c
Nst_number_to_f32(number)
```

**Description:**

Alias for [`_Nst_number_to_f32`](c_api-simple_types.md/#_nst_number_to_f32) that
casts `number` to [`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_number_to_f64`

**Synopsis:**

```better-c
Nst_number_to_f64(number)
```

**Description:**

Alias for [`_Nst_number_to_f64`](c_api-simple_types.md/#_nst_number_to_f64) that
casts `number` to [`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_obj_to_bool`

**Synopsis:**

```better-c
Nst_obj_to_bool(obj)
```

**Description:**

Alias for [`_Nst_obj_to_bool`](c_api-simple_types.md/#_nst_obj_to_bool) that
casts `obj` to [`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

## Structs

### `Nst_IntObj`

**Synopsis:**

```better-c
typedef struct _Nst_IntObj {
    Nst_OBJ_HEAD;
    i64 value;
} struct _Nst_IntObj
```

**Description:**

A structure representing a Nest integer object.

**Fields:**

- `value`: the value of the integer

---

### `Nst_RealObj`

**Synopsis:**

```better-c
typedef struct _Nst_RealObj {
    Nst_OBJ_HEAD;
    f64 value;
} struct _Nst_RealObj
```

**Description:**

A structure representing a Nest real number (floating-point) object.

**Fields:**

- `value`: the value of the real number

---

### `Nst_BoolObj`

**Synopsis:**

```better-c
typedef struct _Nst_BoolObj {
    Nst_OBJ_HEAD;
    bool value;
} struct _Nst_BoolObj
```

**Description:**

A structure representing a Nest boolean object.

**Fields:**

- `value`: the value of the boolean

---

### `Nst_ByteObj`

**Synopsis:**

```better-c
typedef struct _Nst_ByteObj {
    Nst_OBJ_HEAD;
    u8 value;
} struct _Nst_ByteObj
```

**Description:**

A structure representing a Nest byte object.

**Fields:**

- `value`: the value of the byte

---

## Functions

### `Nst_int_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_int_new(i64 value)
```

**Description:**

Creates a new [`Nst_IntObj`](c_api-simple_types.md/#nst_intobj).

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_real_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_real_new(f64 value)
```

**Description:**

Creates a new [`Nst_RealObj`](c_api-simple_types.md/#nst_realobj).

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_bool_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_bool_new(bool value)
```

**Description:**

Creates a new [`Nst_BoolObj`](c_api-simple_types.md/#nst_boolobj).

This function should never be called, to get the `true` and `false` objects use
[`Nst_true()`](c_api-global_consts.md/#nst_true) and
[`Nst_false()`](c_api-global_consts.md/#nst_false) instead. Note that these
functions do not return a new reference to the respective objects.

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `Nst_byte_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_byte_new(u8 value)
```

**Description:**

Creates a new [`Nst_ByteObj`](c_api-simple_types.md/#nst_byteobj).

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or `NULL` on failure. The error is set.

---

### `_Nst_number_to_u8`

**Synopsis:**

```better-c
u8  _Nst_number_to_u8(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object
([`Nst_IntObj`](c_api-simple_types.md/#nst_intobj),
[`Nst_RealObj`](c_api-simple_types.md/#nst_realobj),
[`Nst_ByteObj`](c_api-simple_types.md/#nst_byteobj)) to a
[`u8`](c_api.md/#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0` is returned. No
error is set.

---

### `_Nst_number_to_int`

**Synopsis:**

```better-c
int _Nst_number_to_int(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object
([`Nst_IntObj`](c_api-simple_types.md/#nst_intobj),
[`Nst_RealObj`](c_api-simple_types.md/#nst_realobj),
[`Nst_ByteObj`](c_api-simple_types.md/#nst_byteobj)) to an `int`.

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0` is returned. No
error is set.

---

### `_Nst_number_to_i32`

**Synopsis:**

```better-c
i32 _Nst_number_to_i32(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object
([`Nst_IntObj`](c_api-simple_types.md/#nst_intobj),
[`Nst_RealObj`](c_api-simple_types.md/#nst_realobj),
[`Nst_ByteObj`](c_api-simple_types.md/#nst_byteobj)) to an
[`i32`](c_api.md/#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0` is returned. No
error is set.

---

### `_Nst_number_to_i64`

**Synopsis:**

```better-c
i64 _Nst_number_to_i64(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object
([`Nst_IntObj`](c_api-simple_types.md/#nst_intobj),
[`Nst_RealObj`](c_api-simple_types.md/#nst_realobj),
[`Nst_ByteObj`](c_api-simple_types.md/#nst_byteobj)) to an
[`i64`](c_api.md/#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0` is returned. No
error is set.

---

### `_Nst_number_to_f32`

**Synopsis:**

```better-c
f32 _Nst_number_to_f32(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object
([`Nst_IntObj`](c_api-simple_types.md/#nst_intobj),
[`Nst_RealObj`](c_api-simple_types.md/#nst_realobj),
[`Nst_ByteObj`](c_api-simple_types.md/#nst_byteobj)) to an
[`f32`](c_api.md/#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object, `0.0` is returned. No
error is set.

---

### `_Nst_number_to_f64`

**Synopsis:**

```better-c
f64 _Nst_number_to_f64(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object
([`Nst_IntObj`](c_api-simple_types.md/#nst_intobj),
[`Nst_RealObj`](c_api-simple_types.md/#nst_realobj),
[`Nst_ByteObj`](c_api-simple_types.md/#nst_byteobj)) to an
[`f64`](c_api.md/#type-definitions).

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object `0.0f` is returned. No
error is set.

---

### `_Nst_obj_to_bool`

**Synopsis:**

```better-c
bool _Nst_obj_to_bool(Nst_Obj *obj)
```

**Description:**

Converts any object to a boolean. Exactly the same as casting the object to
[`Nst_type()->Bool`](c_api-global_consts.md/#nst_type) and then checking if the
result matches [`Nst_true()`](c_api-global_consts.md/#nst_true).

