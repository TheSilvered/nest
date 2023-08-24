# `obj.h`

Nest Object interface.

## Authors

TheSilvered

## Macros

### `OBJ`

**Synopsis:**

```better-c
OBJ(obj)
```

**Description:**

Casts `obj` to [`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_inc_ref`

**Synopsis:**

```better-c
Nst_inc_ref(obj)
```

**Description:**

Alias for [`_Nst_inc_ref`](c_api-obj.md/#_nst_inc_ref) that casts `obj` to
[`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_ninc_ref`

**Synopsis:**

```better-c
Nst_ninc_ref(obj)
```

**Description:**

Calls [`Nst_inc_ref`](c_api-obj.md/#nst_inc_ref) if `obj` is not a `NULL`
pointer.

---

### `Nst_dec_ref`

**Synopsis:**

```better-c
Nst_dec_ref(obj)
```

**Description:**

Alias for [`_Nst_dec_ref`](c_api-obj.md/#_nst_dec_ref) that casts `obj` to
[`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_ndec_ref`

**Synopsis:**

```better-c
Nst_ndec_ref(obj)
```

**Description:**

Calls [`Nst_dec_ref`](c_api-obj.md/#nst_dec_ref) if the object is not a `NULL`
pointer.

---

### `Nst_obj_destroy`

**Synopsis:**

```better-c
Nst_obj_destroy(obj)
```

**Description:**

Alias for [`_Nst_obj_destroy`](c_api-obj.md/#_nst_obj_destroy) that casts obj to
[`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

### `Nst_obj_alloc`

**Synopsis:**

```better-c
Nst_obj_alloc(type, type_obj, destructor)
```

**Description:**

Wrapper for [`_Nst_obj_alloc`](c_api-obj.md/#_nst_obj_alloc). `type` is used to
get the size of the object to allocate and to cast the result into the correct
pointer type.

---

### `Nst_FLAG_SET`

**Synopsis:**

```better-c
Nst_FLAG_SET(obj, flag)
```

**Description:**

Sets `flag` of `obj` to `true`.

---

### `Nst_FLAG_DEL`

**Synopsis:**

```better-c
Nst_FLAG_DEL(obj, flag)
```

**Description:**

Sets `flag` of `obj` to `false`.

---

### `Nst_FLAG_HAS`

**Synopsis:**

```better-c
Nst_FLAG_HAS(obj, flag)
```

**Description:**

Checks if `flag` is set.

---

### `Nst_OBJ_HEAD`

**Description:**

The macro used to make a struct an object.

It must be placed before any other arguments in the struct. Custom flags cannot
occupy the four most significant bits of the flags field because they are
reserved for the garbage collector.

---

## Structs

### `Nst_Obj`

**Synopsis:**

```better-c
typedef struct _Nst_Obj {
    Nst_OBJ_HEAD;
} struct _Nst_Obj
```

**Description:**

The structure representing a basic Nest object.

**Fields:**

- `ref_count`: the reference count of the object
- `type`: the type of the object
- `destructor`: the destructor of the object
- `hash`: the hash of the object, `-1` if it has not yet been hashed or is not
  hashable
- `flags`: the flags of the object
- `init_line`: **THIS FIELD ONLY EXISTS WHEN `Nst_TRACK_OBJ_INIT_POS` IS
  DEFINED** the line of the instruction that initialized the object
- `init_col`: **THIS FIELD ONLY EXISTS WHEN `Nst_TRACK_OBJ_INIT_POS` IS
  DEFINED** the column of the instruction that initialized the object
- `init_path`: **THIS FIELD ONLY EXISTS WHEN `Nst_TRACK_OBJ_INIT_POS` IS
  DEFINED** the path to the file where the object was initialized

---

## Type aliases

### `Nst_ObjDestructor`

**Synopsis:**

```better-c
typedef void (*Nst_ObjDestructor)(void *)
```

**Description:**

The type of an object destructor.

---

### `Nst_NullObj`

**Synopsis:**

```better-c
typedef Nst_Obj Nst_NullObj;
```

**Description:**

A [`Nst_NullObj`](c_api-obj.md/#nst_nullobj) is just a
[`Nst_Obj`](c_api-obj.md/#nst_obj) as it does not have any special fields.

---

## Functions

### `_Nst_obj_alloc`

**Synopsis:**

```better-c
Nst_Obj *_Nst_obj_alloc(usize size, Nst_StrObj *type,
                        void (*destructor)(void *))
```

**Description:**

Allocates an object on the heap and initializes the fields in
[`Nst_OBJ_HEAD`](c_api-obj.md/#nst_obj_head).

**Parameters:**

- `size`: the size in bytes of the memory to allocate
- `type`: the type of the object, if it is `NULL`, the object itself is used as
  the type
- `destructor`: the destructor of the object, it can be `NULL`

**Returns:**

The newly allocate object or `NULL` on failure. The error is set.

---

### `_Nst_obj_destroy`

**Synopsis:**

```better-c
void _Nst_obj_destroy(Nst_Obj *obj)
```

**Description:**

Calls an object's destructor and then frees its memory.

This function should not be called on most occasions, use
[`Nst_dec_ref`](c_api-obj.md/#nst_dec_ref) instead.

---

### `_Nst_inc_ref`

**Synopsis:**

```better-c
Nst_Obj *_Nst_inc_ref(Nst_Obj *obj)
```

**Description:**

Increases the reference count of an object.

---

### `_Nst_dec_ref`

**Synopsis:**

```better-c
void _Nst_dec_ref(Nst_Obj *obj)
```

**Description:**

Decreases the reference count of an object and calls
[`_Nst_obj_destroy`](c_api-obj.md/#_nst_obj_destroy) if it reaches zero.

