# `function.h`

The header defining the Nest function object.

## Macros

### `FUNC`

**Synopsis**:

```better-c
FUNC(ptr)
```

**Description**:

Casts `ptr` to a `Nst_FuncObj *`.

---

### `nst_func_set_vt`

**Synopsis**:

```better-c
nst_func_set_vt(func, map)
```

**Description**:

Wrapper for [`_nst_func_set_vt`](#_nst_func_set_vt) that automatically casts
`func` to `Nst_FuncObj *` and `map` to `Nst_MapObj *`.

---

## Unions

### `Nst_FuncBody`

**Synopsis**:

```better-c
typedef union _Nst_FuncBody
{
    Nst_InstList *bytecode;
    Nst_Obj *(*c_func)(usize arg_num, Nst_Obj **args, Nst_OpErr *err);
}
Nst_FuncBody;
```

**Description**:

The union that contains the executable part of a function.

**Variants**:

- `bytecode`: the function is written in Nest
- `c_func`: the function is written in C

---

## Structs

### `Nst_FuncObj`

**Synopsis**:

```better-c
typedef struct _Nst_FuncObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    Nst_FuncBody body;
    Nst_Obj **args;
    usize arg_num;
    Nst_MapObj *mod_globals;
}
Nst_FuncObj;
```

**Description**:

The structure that defines a Nest function object.

**Fields**:

- `body`: the code to execute when the function is called
- `args`: the name of the arguments of the function, they are all `Nst_StrObj *`.
  This parameter is unused when the function uses a C body
- `arg_num`: the number of arguments expected by the function
- `mod_globals`: the global variables of the module the function was defined in

---

## Functions

### `nst_func_new`

**Synopsis**:

```better-c
Nst_Obj *nst_func_new(usize arg_num, Nst_InstList *bytecode, Nst_OpErr *err)
```

**Description**:

Creates a new Nest function object with a Nest body.

**Arguments**:

- `[in] arg_num`: the number of arguments expected by the function
- `[in] bytecode`: the body of the function
- `[out] err`: set if an error occurs

**Return value**:

The function returns a new `Func` object or `NULL` if an error occurs.

---

### `nst_func_new_c`

**Synopsis**:

```better-c
Nst_Obj *nst_func_new_c(usize arg_num,
                        Nst_Obj *(*cbody)(usize     arg_num,
                                          Nst_Obj  **args,
                                          Nst_OpErr *err),
                        Nst_OpErr *err)
```

**Description**:

Creates a new Nest function object with a C body.

**Arguments**:

- `[in] arg_num`: the number of arguments expected by the function
- `[in] cbody`: the body of the function
- `[out] err`: set if an error occurs

**Return value**:

The function returns a new `Func` object or `NULL` if an error occurs.

---

### `_nst_func_set_vt`

**Synopsis**:

```better-c
void _nst_func_set_vt(Nst_FuncObj *func, Nst_MapObj *map)
```

**Description**:

Sets `mod_globals` of `func` if it has a Nest body and of all the functions
defined inside it.

**Arguments**:

- `[in] func`: the function of which to change the global table
- `[in] map`: the global table to be set

---

### Other functions

```better-c
void _nst_func_traverse(Nst_FuncObj *func)
void _nst_func_track(Nst_FuncObj *func)
void _nst_func_destroy(Nst_FuncObj *func)
```

These functions are used internally by Nest and should never be called by
extenal libraries.

---

## Enums

### `Nst_FuncFlags`

**Synopsis**:

```better-c
typedef enum _Nst_FuncFlags
{
    NST_FLAG_FUNC_IS_C = 0b1
}
Nst_FuncFlags;
```

**Description**:

The flags used for functions. To check if a function `func` has a C body do the
following:

```better-c
NST_FLAG_HAS(func, NST_FLAG_FUNC_IS_C)
```
