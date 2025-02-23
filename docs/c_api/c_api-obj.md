# `obj.h`

Nest Object interface.

## Authors

TheSilvered

---

## Macros

### `_Nst_P_LEN_MAX`

**Description:**

Maximum size for an object pool.

---

### `NstOBJ`

**Synopsis:**

```better-c
#define NstOBJ(obj)
```

**Description:**

Casts `obj` to [`Nst_Obj *`](c_api-obj.md#nst_obj).

---

### `Nst_obj_alloc`

**Synopsis:**

```better-c
#define Nst_obj_alloc(type, type_obj)
```

**Description:**

Wrapper for [`_Nst_obj_alloc`](c_api-obj.md#_nst_obj_alloc). `type` is used to
get the size of the object to allocate and to cast the result into the correct
pointer type.

---

### `Nst_SET_FLAG`

**Synopsis:**

```better-c
#define Nst_SET_FLAG(obj, flag)
```

**Description:**

Sets `flag` of `obj` to `true`.

---

### `Nst_DEL_FLAG`

**Synopsis:**

```better-c
#define Nst_DEL_FLAG(obj, flag)
```

**Description:**

Sets `flag` of `obj` to `false`.

---

### `Nst_HAS_FLAG`

**Synopsis:**

```better-c
#define Nst_HAS_FLAG(obj, flag)
```

**Description:**

Checks if `flag` is set.

---

### `Nst_FLAG`

**Synopsis:**

```better-c
#define Nst_FLAG(n)
```

**Description:**

Creates a flag from an id. `n` can be between 1 and 28 included.

---

### `Nst_CLEAR_FLAGS`

**Synopsis:**

```better-c
#define Nst_CLEAR_FLAGS(obj)
```

**Description:**

Clears all flags from an object, except for the reserved ones.

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
} Nst_Obj
```

**Description:**

The structure representing a basic Nest object.

**Fields:**

- `ref_count`: the reference count of the object
- `type`: the type of the object
- `p_next`: the next object in the type's pool
- `hash`: the hash of the object, `-1` if it has not yet been hashed or is not
  hashable
- `flags`: the flags of the object
- `init_line`: **THIS FIELD ONLY EXISTS WHEN `Nst_DBG_TRACK_OBJ_INIT_POS` IS
  DEFINED** the line of the instruction that initialized the object
- `init_col`: **THIS FIELD ONLY EXISTS WHEN `Nst_DBG_TRACK_OBJ_INIT_POS` IS
  DEFINED** the column of the instruction that initialized the object
- `init_path`: **THIS FIELD ONLY EXISTS WHEN `Nst_DBG_TRACK_OBJ_INIT_POS` IS
  DEFINED** the path to the file where the object was initialized

---

## Type aliases

### `Nst_ObjDstr`

**Synopsis:**

```better-c
typedef void (*Nst_ObjDstr)(Nst_Obj *)
```

**Description:**

The type of an object destructor.

This function, in an object's type, is called when the object is deleted and
should free any memory associated with it apart from the object's own memory,
which is handled by Nest. This function should also remove any references that
the object being deleted has with other objects.

---

### `Nst_ObjTrav`

**Synopsis:**

```better-c
typedef void (*Nst_ObjTrav)(Nst_Obj *)
```

**Description:**

The type of an object traverse function for the garbage collector.

This function is called during a garbage collection and should call the function
[`Nst_ggc_obj_reachable`](c_api-ggc.md#nst_ggc_obj_reachable) with any object
that it directly references. Any indirect references, such as objects within
objects, should be left untouched.

---

## Functions

### `_Nst_obj_alloc`

**Synopsis:**

```better-c
Nst_Obj *_Nst_obj_alloc(usize size, Nst_Obj *type)
```

**Description:**

Allocates an object on the heap and initializes the fields in
[`Nst_OBJ_HEAD`](c_api-obj.md#nst_obj_head).

**Parameters:**

- `size`: the size in bytes of the memory to allocate
- `type`: the type of the object, if it is `NULL`, the object itself is used as
  the type

**Returns:**

The newly allocate object or `NULL` on failure. The error is set.

---

### `Nst_obj_traverse`

**Synopsis:**

```better-c
void Nst_obj_traverse(Nst_Obj *obj)
```

**Description:**

Traverse an object for the GGC. If the object's type does not have a traverse
function, this function does nothing.

---

### `Nst_inc_ref`

**Synopsis:**

```better-c
Nst_Obj *Nst_inc_ref(Nst_Obj *obj)
```

**Description:**

Increases the reference count of an object. Returns `obj`.

---

### `Nst_ninc_ref`

**Synopsis:**

```better-c
Nst_Obj *Nst_ninc_ref(Nst_Obj *obj)
```

**Description:**

Calls [`Nst_inc_ref`](c_api-obj.md#nst_inc_ref) if `obj` is not a `NULL`
pointer. Returns `obj`.

---

### `Nst_dec_ref`

**Synopsis:**

```better-c
void Nst_dec_ref(Nst_Obj *obj)
```

**Description:**

Decreases the reference count of an object.

---

### `Nst_ndec_ref`

**Synopsis:**

```better-c
void Nst_ndec_ref(Nst_Obj *obj)
```

**Description:**

Calls [`Nst_dec_ref`](c_api-obj.md#nst_dec_ref) if `obj` is not a `NULL`
pointer. Returns `obj`.

---

## Enums

### `Nst_ObjFlags`

**Synopsis:**

```better-c
typedef enum _Nst_ObjFlags {
    Nst_FLAG_OBJ_DESTROYED = Nst_FLAG(29)
} Nst_ObjFlags
```

**Description:**

Flags of a Nest object.
