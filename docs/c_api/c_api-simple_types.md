# `simple_types.h`

Interface for Nst_IntObj, Nst_RealObj, Nst_ByteObj, Nst_BoolObj and
Nst_IOFileObj.

## Authors

TheSilvered

## Macros

### `AS_INT`

**Synopsis:**

```better-c
AS_INT(ptr)
```

**Description:**

Casts ptr to a Nst_IntObj * and extracts the value field.

---

### `AS_REAL`

**Synopsis:**

```better-c
AS_REAL(ptr)
```

**Description:**

Casts ptr to a Nst_RealObj * and extracts the value field.

---

### `AS_BYTE`

**Synopsis:**

```better-c
AS_BYTE(ptr)
```

**Description:**

Casts ptr to a Nst_ByteObj * and extracts the value field.

---

### `AS_BOOL`

**Synopsis:**

```better-c
AS_BOOL(ptr)
```

**Description:**

Casts ptr to a Nst_BoolObj * and extracts the value field.

---

### `Nst_number_to_u8`

**Synopsis:**

```better-c
Nst_number_to_u8(number)
```

**Description:**

Alias for _Nst_number_to_u8 that casts number to Nst_Obj *.

---

### `Nst_number_to_int`

**Synopsis:**

```better-c
Nst_number_to_int(number)
```

**Description:**

Alias for _Nst_number_to_int that casts number to Nst_Obj *.

---

### `Nst_number_to_i32`

**Synopsis:**

```better-c
Nst_number_to_i32(number)
```

**Description:**

Alias for _Nst_number_to_i32 that casts number to Nst_Obj *.

---

### `Nst_number_to_i64`

**Synopsis:**

```better-c
Nst_number_to_i64(number)
```

**Description:**

Alias for _Nst_number_to_i64 that casts number to Nst_Obj *.

---

### `Nst_number_to_f32`

**Synopsis:**

```better-c
Nst_number_to_f32(number)
```

**Description:**

Alias for _Nst_number_to_f32 that casts number to Nst_Obj *.

---

### `Nst_number_to_f64`

**Synopsis:**

```better-c
Nst_number_to_f64(number)
```

**Description:**

Alias for _Nst_number_to_f64 that casts number to Nst_Obj *.

---

### `Nst_obj_to_bool`

**Synopsis:**

```better-c
Nst_obj_to_bool(obj)
```

**Description:**

Alias for _Nst_obj_to_bool that casts obj to Nst_Obj *.

---

## Structs

### `Nst_IntObj`

**Synopsis:**

```better-c
typedef struct _Nst_IntObj {
    Nst_OBJ_HEAD;
    i64 value;
} Nst_IntObj
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
} Nst_RealObj
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
} Nst_BoolObj
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
} Nst_ByteObj
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

Creates a new Nst_IntObj.

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or NULL on failure. The error is set.

---

### `Nst_real_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_real_new(f64 value)
```

**Description:**

Creates a new Nst_RealObj.

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or NULL on failure. The error is set.

---

### `Nst_bool_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_bool_new(bool value)
```

**Description:**

Creates a new Nst_BoolObj.

This function should never be called, to get the true and false objects use
Nst_true() and Nst_false() instead. Note that these functions do not return a
new reference to the returned objects.

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or NULL on failure. The error is set.

---

### `Nst_byte_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_byte_new(u8 value)
```

**Description:**

Creates a new Nst_ByteObj.

**Parameters:**

- `value`: the value of the new object

**Returns:**

The new object on success or NULL on failure. The error is set.

---

### `_Nst_number_to_u8`

**Synopsis:**

```better-c
u8  _Nst_number_to_u8(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object (Nst_IntObj, Nst_RealObj, Nst_ByteObj) to
a u8.

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object 0 is returned. No error
is set.

---

### `_Nst_number_to_int`

**Synopsis:**

```better-c
int _Nst_number_to_int(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object (Nst_IntObj, Nst_RealObj, Nst_ByteObj) to
an int.

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object 0 is returned. No error
is set.

---

### `_Nst_number_to_i32`

**Synopsis:**

```better-c
i32 _Nst_number_to_i32(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object (Nst_IntObj, Nst_RealObj, Nst_ByteObj) to
an i32.

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object 0 is returned. No error
is set.

---

### `_Nst_number_to_i64`

**Synopsis:**

```better-c
i64 _Nst_number_to_i64(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object (Nst_IntObj, Nst_RealObj, Nst_ByteObj) to
an i64.

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object 0 is returned. No error
is set.

---

### `_Nst_number_to_f32`

**Synopsis:**

```better-c
f32 _Nst_number_to_f32(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object (Nst_IntObj, Nst_RealObj, Nst_ByteObj) to
an f32.

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object 0 is returned. No error
is set.

---

### `_Nst_number_to_f64`

**Synopsis:**

```better-c
f64 _Nst_number_to_f64(Nst_Obj *number)
```

**Description:**

Converts the value of a numeric object (Nst_IntObj, Nst_RealObj, Nst_ByteObj) to
an f64.

**Parameters:**

- `number`: the object to convert the value of

**Returns:**

The converted number. If number is not a numeric object 0 is returned. No error
is set.

---

### `_Nst_obj_to_bool`

**Synopsis:**

```better-c
bool _Nst_obj_to_bool(Nst_Obj *obj)
```

**Description:**

Converts any object to a boolean. Exactly the same as casting the object to
Nst_type()->Bool and then checking if the result matches Nst_true();

