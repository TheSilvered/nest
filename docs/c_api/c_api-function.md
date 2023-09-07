# `function.h`

[`Nst_FuncObj`](c_api-function.md#nst_funcobj) interface.

## Authors

TheSilvered

## Macros

### `FUNC`

**Synopsis:**

```better-c
FUNC(ptr)
```

**Description:**

Casts `ptr` to [`Nst_FuncObj *`](c_api-function.md#nst_funcobj).

---

### `Nst_func_set_vt`

**Synopsis:**

```better-c
Nst_func_set_vt(func, map)
```

**Description:**

Alias for [`_Nst_func_set_vt`](c_api-function.md#_nst_func_set_vt) that casts
`func` to [`Nst_FuncObj *`](c_api-function.md#nst_funcobj) and `map` to
[`Nst_MapObj *`](c_api-map.md#nst_mapobj).

---

## Unions

### `Nst_FuncBody`

**Synopsis:**

```better-c
typedef union _Nst_FuncBody {
    Nst_InstList *bytecode;
    Nst_Obj *(*c_func)(usize arg_num, Nst_Obj **args);
} Nst_FuncBody
```

**Description:**

The union representing the body of a function object.

**Variants:**

- `bytecode`: the body is an instruction list
- `c_func`: the body is a C function

---

## Structs

### `Nst_FuncObj`

**Synopsis:**

```better-c
typedef struct _Nst_FuncObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_FuncBody body;
    Nst_Obj **args;
    usize arg_num;
    Nst_MapObj *mod_globals;
} Nst_FuncObj
```

**Description:**

The structure representing a Nest function object.

**Fields:**

- `body`: the body of the function
- `args`: the array of names of the arguments
- `arg_num`: the maximum number of arguments
- `mod_globals`: the global variable table when the function was defined

---

## Functions

### `Nst_func_new`

**Synopsis:**

```better-c
Nst_Obj *Nst_func_new(usize arg_num, Nst_InstList *bytecode)
```

**Description:**

Creates a new function object with an instruction-list body.

!!!note
    The `args` array must be set manually after instantiation.

**Parameters:**

- `arg_num`: the maximum number of arguments the function accepts
- `bytecode`: the body of the function

**Returns:**

The new function object or `NULL` on failure. The error is set.

---

### `Nst_func_new_c`

**Synopsis:**

```better-c
Nst_Obj *Nst_func_new_c(usize arg_num, Nst_Obj *(*cbody)(usize, Nst_Obj **))
```

**Description:**

Creates a new function object with a C function body.

!!!note
    The `args` array must NOT be set since it is not used.

**Parameters:**

- `arg_num`: the maximum number of arguments the function accepts
- `cbody`: the body of the function

**Returns:**

The new function object or `NULL` on failure. The error is set.

---

### `_Nst_func_set_vt`

**Synopsis:**

```better-c
void _Nst_func_set_vt(Nst_FuncObj *func, Nst_MapObj *map)
```

**Description:**

Sets the `mod_globals` table of a function and all the functions defined inside
it.

If the field is already set or the function has a C body, it is not modified.

**Parameters:**

- `func`: the function to change the `mod_globals` field of
- `map`: the map to set as the new value

---

### `_Nst_func_traverse`

**Synopsis:**

```better-c
void _Nst_func_traverse(Nst_FuncObj *func)
```

**Description:**

Traverse function for [`Nst_FuncObj`](c_api-function.md#nst_funcobj).

---

### `_Nst_func_destroy`

**Synopsis:**

```better-c
void _Nst_func_destroy(Nst_FuncObj *func)
```

**Description:**

Destructor for [`Nst_FuncObj`](c_api-function.md#nst_funcobj).

---

## Enums

### `Nst_FuncFlags`

**Synopsis:**

```better-c
typedef enum _Nst_FuncFlags {
    Nst_FLAG_FUNC_IS_C = 0b1
} Nst_FuncFlags
```

**Description:**

The flags for [`Nst_FuncObj`](c_api-function.md#nst_funcobj).

