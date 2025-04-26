# `type.h`

Nest `Type` object.

## Authors

TheSilvered

---

## Functions

### `Nst_type_new`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_type_new(const char *name, Nst_ObjDstr dstr)
```

**Description:**

Create a new `Type` object.

!!!note
    `name` must be encoded in UTF-8

**Parameters:**

- `name`: the name of the type
- `dstr`: the destructor of the type

**Returns:**

The new object on success and `NULL` on failure. The error is set.

---

### `Nst_cont_type_new`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_cont_type_new(const char *name, Nst_ObjDstr dstr,
                              Nst_ObjTrav trav)
```

**Description:**

Create a new `Type` for containers.

!!!note
    `name` must be encoded in UTF-8

**Parameters:**

- `name`: the name of the type
- `dstr`: the destructor of the type
- `trav`: the traverse function of the type

**Returns:**

The new object on success and `NULL` on failure. The error is set.

---

### `Nst_type_name`

**Synopsis:**

```better-c
Nst_StrView Nst_type_name(Nst_Obj *type)
```

**Description:**

@return The name of the type.

---

### `Nst_type_trav`

**Synopsis:**

```better-c
Nst_ObjTrav Nst_type_trav(Nst_Obj *type)
```

**Returns:**

The traverse function of the type. If the type does not represent a container it
returns `NULL`.
