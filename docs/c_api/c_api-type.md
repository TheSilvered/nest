# `type.h`

Nest Type object..

## Authors

TheSilvered

## Macros

### `TYPE`

**Synopsis:**

```better-c
TYPE(ptr)
```

**Description:**

Casts `ptr` to [`Nst_TypeObj *`](c_api-type.md#nst_typeobj).

---

### `CONT_TYPE`

**Synopsis:**

```better-c
CONT_TYPE(ptr)
```

**Description:**

Casts `ptr` to [`Nst_ContTypeObj *`](c_api-type.md#nst_conttypeobj).

---

### `_Nst_TYPE_HEAD`

**Description:**

Head of a type object, should not be used for custom types.

---

## Structs

### `Nst_TypeObj`

**Synopsis:**

```better-c
typedef struct _Nst_TypeObj {
    Nst_OBJ_HEAD;
    _Nst_TYPE_HEAD;
} Nst_TypeObj
```

**Description:**

The structure representing a Type object in Nest.

**Fields:**

- `p_head`: the head object in the type's pool
- `p_len`: the length of the pool
- `name`: the name of the object as a Nest string
- `dstr`: the destructor of the type, can be NULL

---

### `Nst_ContTypeObj`

**Synopsis:**

```better-c
typedef struct _Nst_ContTypeObj {
    Nst_OBJ_HEAD;
    _Nst_TYPE_HEAD;
    Nst_ObjTrav trav;
} Nst_ContTypeObj
```

**Description:**

The structure representing a Type object for containers in Nest.

**Fields:**

- `p_head`: the head object in the type's pool
- `p_len`: the length of the pool
- `name`: the name of the object as a Nest string
- `dstr`: the destructor of the type, can be NULL
- `trav`: the traverse function of the type

---

## Functions

### `Nst_type_new`

**Synopsis:**

```better-c
Nst_TypeObj *Nst_type_new(const i8 *name, Nst_ObjDstr dstr)
```

**Description:**

Creates a new [`Nst_TypeObj`](c_api-type.md#nst_typeobj).

!!!note
    `name` can only contain 7-bit ASCII characters

**Parameters:**

- `name`: the name of the type
- `dstr`: the destructor of the type

**Returns:**

The new object on success and `NULL` on failure. The error is set.

---

### `Nst_cont_type_new`

**Synopsis:**

```better-c
Nst_TypeObj *Nst_cont_type_new(const i8 *name, Nst_ObjDstr dstr,
                               Nst_ObjTrav trav)
```

**Description:**

Creates a new [`Nst_ContTypeObj`](c_api-type.md#nst_conttypeobj).

!!!note
    `name` can only contain 7-bit ASCII characters

**Parameters:**

- `name`: the name of the type
- `dstr`: the destructor of the type
- `trav`: the traverse function of the type

**Returns:**

The new object on success and `NULL` on failure. The error is set.

---

### `_Nst_type_destroy`

**Synopsis:**

```better-c
void _Nst_type_destroy(Nst_TypeObj *obj)
```

**Description:**

Destructor for Nest type objects.

