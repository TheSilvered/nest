# `var_table.h`

This header contains the definition for the Nest variable table.

## Macros

### `nst_vt_get`

**Synopsis**:

```better-c
nst_vt_get(vt, name)
```

**Description**:

Alias for [`_nst_vt_get`](#_nst_vt_get) that casts `name` to `Nst_Obj *`.

---

### `nst_vt_set`

**Synopsis**:

```better-c
nst_vt_set(vt, name, val, err)
```

**Description**:

Alias for [`_nst_vt_set`](#_nst_vt_set) that casts `name` and `val` to
`Nst_Obj *`.

---

## Structs

### `Nst_VarTable`

**Synopsis**:

```better-c
typedef struct _Nst_VarTable
{
    Nst_MapObj *vars;
    Nst_MapObj *global_table;
}
Nst_VarTable
```

**Description**:

The structure defining a variable table in Nest.

**Fields**:

- `vars`: the local variables
- `global_table`: the global variables

---

## Functions

### `nst_vt_new`

**Synopsis**:

```better-c
Nst_VarTable *nst_vt_new(Nst_MapObj *global_table,
                         Nst_StrObj *cwd,
                         Nst_SeqObj *args,
                         Nst_OpErr  *err)
```

**Description**:

Creates a new variable table.

**Arguments**:

- `[in] global_table`: the global table
- `[in] cwd`: the current working directory
- `[in] args`: the command line arguments
- `[out] err`: the error

**Return value**:

The function returns the new variable table or `NULL` on failure.

---

### `_nst_vt_get`

**Synopsis**:

```better-c
Nst_Obj *_nst_vt_get(Nst_VarTable *vt, Nst_Obj *name)
```

**Description**:

Gets a value from a variable table.

**Arguments**:

- `[in] vt`: the variable table
- `[in] name`: the name of the variable

**Return value**:

The function returns the value associated with `name` or `nst_null()` if it is
not found.

---

### `_nst_vt_set`

**Synopsis**:

```better-c
void _nst_vt_set(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val, Nst_OpErr *err)
```

**Description**:

Sets a value in a variable table.

**Arguments**:

- `[inout] vt`: the variable table
- `[in] name`: the key of the value
- `[in] val`: the value associated with `name`
- `[out] err`: the error
