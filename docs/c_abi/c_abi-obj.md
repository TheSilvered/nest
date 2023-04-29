# `obj.h`

This header contains

## Macros

### `OBJ`

**Synopsis**:

```better-c
OBJ(obj)
```

**Description**:

Casts `obj` to `Nst_Obj *`.

---

### `nst_inc_ref`

**Synopsis**:

```better-c
nst_inc_ref(obj)
```

**Description**:

Alias for [`_nst_inc_ref`](#_nst_inc_ref) that casts `obj` to `Nst_Obj *`.

---

### `nst_dec_ref`

**Synopsis**:

```better-c
nst_dec_ref(obj)
```

**Description**:

Alias for [`_nst_dec_ref`](#_nst_dec_ref) that casts `obj` to `Nst_Obj *`.

---

### `nst_obj_destroy`

**Synopsis**:

```better-c
nst_obj_destroy(obj)
```

**Description**:

Alias for [`_nst_obj_destroy`](#_nst_obj_destroy) that casts `obj` to
`Nst_Obj *`.

---

### `nst_obj_alloc`

**Synopsis**:

```better-c
nst_obj_alloc(size, type, destructor, err)
```

**Description**:

Alias for [`_nst_obj_alloc`](#_nst_obj_alloc) that casts `type` to
`Nst_TypeObj *` and `destructor` to `Nst_ObjDestructor`

---

### `NST_FLAG_SET`

**Synopsis**:

```better-c
NST_FLAG_SET(obj, flag)
```

**Description**:

Adds a flag on the object.

**Arguments**:

- `obj`: the object to add the flag to
- `flag`: the flag to add

---

### `NST_FLAG_DEL`

**Synopsis**:

```better-c
NST_FLAG_DEL(obj, flag)
```

**Description**:

Removes a flag from the object.

**Arguments**:

- `obj`: the object to remove the flag from
- `flag`: the flag to remove

---

### `NST_FLAG_HAS`

**Synopsis**:

```better-c
NST_FLAG_HAS(obj, flag)
```

**Description**:

Checks if `obj` has `flag`.

---

### `NST_OBJ_HEAD`

**Description**:

Expands into the fields that every object requires, that is the fields of
`Nst_Obj`.

---

## Structs

### `Nst_Obj`

**Synopsis**:

```better-c
typedef struct _Nst_Obj
{
    i32 ref_count;
    Nst_TypeObj *type;
    void (*destructor)(void *);
    i32 hash;
    u8 flags;
}
Nst_Obj
```

**Description**:

The structure that defines the simplest Nest object.

**Fields**:

- `ref_count`: the reference count of the object
- `type`: the type of the object
- `destructor`: the destructor of the object
- `hash`: the hash of the object
- `flags`: the flags of the object

!!!note
    Only the lower four bits of the `flags` field can be set with custom flags,
    the top four are reserved for the garbage collector.

---

## Type aliases

### `Nst_ObjDestructor`

**Synopsis**:

```better-c
typedef void (*Nst_ObjDestructor)(void *)
```

### `Nst_NullObj`

**Synopsis**:

```better-c
typedef Nst_Obj Nst_NullObj
```

---

## Functions

### `_nst_obj_alloc`

**Synopsis**:

```better-c
Nst_Obj *_nst_obj_alloc(usize               size,
                        struct _Nst_StrObj *type,
                        void (*destructor)(void *),
                        struct _Nst_OpErr  *err)
```

**Description**:

Creates a new object struct of size `size` on the heap and initializes its
fields.

**Arguments**:

- `size`: the size of the object
- `type`: the type of the object, if set to `NULL` the object itself will be
  used
- `destructor`: the destructor of the object, if `NULL` the object does not have
  a destructor
- `err`: the error

**Return value**:

The function returns the new object or `NULL` on failure.

---

### `_nst_obj_destroy`

**Synopsis**:

```better-c
void _nst_obj_destroy(Nst_Obj *obj)
```

**Description**:

The function called when an object's reference count reaches zero or the garbage
collector disposes it. **Must never be called**.

**Arguments**:

- `obj`: the object to destroy

**Return value**:

---

### `_nst_inc_ref`

**Synopsis**:

```better-c
Nst_Obj *_nst_inc_ref(Nst_Obj *obj)
```

**Description**:

Increases the reference count of `obj`.

**Return value**:

The function returns `obj`.

---

### `_nst_dec_ref`

**Synopsis**:

```better-c
void _nst_dec_ref(Nst_Obj *obj)
```

**Description**:

Decreases the reference count of `obj`.
