# `function.h`

`Func` object interface.

## Authors

TheSilvered

---

## Functions

### `Nst_func_new_c`

**Synopsis:**

```better-c
Nst_Obj *Nst_func_new_c(usize arg_num, Nst_NestCallable cbody)
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

### `Nst_func_arg_num`

**Synopsis:**

```better-c
usize Nst_func_arg_num(Nst_Obj *func)
```

**Description:**

Gets the number of arguments a function takes.

---

### `Nst_func_args`

**Synopsis:**

```better-c
Nst_Obj **Nst_func_args(Nst_Obj *func)
```

**Description:**

Gets the argument names as a list of objects.

---

### `Nst_func_c_body`

**Synopsis:**

```better-c
Nst_NestCallable Nst_func_c_body(Nst_Obj *func)
```

**Description:**

Gets the body of a C-function wrapper.

---

### `Nst_func_nest_body`

**Synopsis:**

```better-c
Nst_InstList *Nst_func_nest_body(Nst_Obj *func)
```

**Description:**

Gets the body of a Nest function.

---

### `Nst_func_mod_globals`

**Synopsis:**

```better-c
Nst_Obj *Nst_func_mod_globals(Nst_Obj *func)
```

**Description:**

Gets the `_globals_` variable map of a function. No reference is added. It may
be `NULL`.

---

### `Nst_func_outer_vars`

**Synopsis:**

```better-c
Nst_Obj *Nst_func_outer_vars(Nst_Obj *func)
```

**Description:**

Gets the outer variables that the function can access when defined.

---

### `_Nst_func_traverse`

**Synopsis:**

```better-c
void _Nst_func_traverse(Nst_Obj *func)
```

**Description:**

Traverse function for `Func` objects.

---

## Enums

### `Nst_FuncFlags`

**Synopsis:**

```better-c
typedef enum _Nst_FuncFlags {
    Nst_FLAG_FUNC_IS_C = Nst_FLAG(1)
} Nst_FuncFlags
```

**Description:**

Flags for `Func` objects.
