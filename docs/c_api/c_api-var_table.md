# `var_table.h`

Variable table interface using Nst_MapObj.

## Authors

TheSilvered

---

## Structs

### `Nst_VarTable`

**Synopsis:**

```better-c
typedef struct _Nst_VarTable {
    Nst_Obj *vars;
    Nst_Obj *global_table;
} Nst_VarTable
```

**Description:**

Structure representing the Nest variable table

**Fields:**

- `vars`: the map of local variables
- `global_table`: the map of global variables

---

## Functions

### `Nst_vt_new`

**Synopsis:**

```better-c
Nst_VarTable *Nst_vt_new(Nst_Obj *global_table, Nst_Obj *args, bool no_default)
```

**Description:**

Creates a new var table on the heap.

**Parameters:**

- `global_table`: the current global variable table, can be `NULL`
- `cwd`: the current working directory, ignored when `global_table` is not
  `NULL` or `no_default` is `true`
- `args`: the command line arguments, ignored when `global_table` is not `NULL`
  or `no_default` is `true`
- `no_default`: whether to create predefined variables

---

### `Nst_vt_destroy`

**Synopsis:**

```better-c
void Nst_vt_destroy(Nst_VarTable *vt)
```

**Description:**

[`Nst_VarTable`](c_api-var_table.md#nst_vartable) destructor.

---

### `Nst_vt_get`

**Synopsis:**

```better-c
Nst_Obj *Nst_vt_get(Nst_VarTable *vt, Nst_Obj *name)
```

**Description:**

Retrieves a value from a variable table.

**Parameters:**

- `vt`: the variable table to get the value from
- `name`: the name of the value to get

**Returns:**

The value associated with the key or
[`Nst_null()`](c_api-global_consts.md#nst_null) if the key is not present in the
table.

---

### `Nst_vt_set`

**Synopsis:**

```better-c
bool Nst_vt_set(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val)
```

**Description:**

Sets a value in a variable table.

**Parameters:**

- `vt`: the variable table to set the value into
- `name`: the name of the value to set
- `value`: the value to associate to name

**Returns:**

`true` on success and `false` on failure. The error is set.
