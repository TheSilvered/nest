# `interpreter.h`

Bytecode interpreter.

## Authors

TheSilvered

---

## Structs

### `Nst_InterpreterState`

**Synopsis:**

```better-c
typedef struct _Nst_InterpreterState {
    Nst_Program *prog;
    Nst_ValueStack v_stack;
    Nst_CallStack  f_stack;
    Nst_CatchStack c_stack;
    Nst_Obj *func;
    Nst_VarTable vt;
    i64 idx;
} Nst_InterpreterState
```

**Description:**

A structure representing the current state of the Nest interpreter.

**Fields:**

- `prog`: the program being run, if NULL all other fields could have invalid
  data and are not safe to read
- `v_stack`: the value stack
- `f_stack`: the call stack
- `c_stack`: the catch frame stack
- `func`: the function currently being executed
- `vt`: the current variable table
- `idx`: the index of the current bytecode instruction

---

## Functions

### `Nst_init`

**Synopsis:**

```better-c
bool Nst_init(void)
```

**Description:**

Initialize the Nest libraray.

!!!note
    [`Nst_error_set_color`](c_api-error.md#nst_error_set_color) is called with
    the value returned by
    [`Nst_supports_color`](c_api-argv_parser.md#nst_supports_color).

**Returns:**

`true` on success and `false` on failure. The error cannot be accessed if the
library fails to initialize.

---

### `Nst_quit`

**Synopsis:**

```better-c
void Nst_quit(void)
```

**Description:**

Destroy all the components of the libraray. It is not safe to access any object
created while the library was initialized after this function is called. Any
destructors that may access Nest objects must be called before calling this
function.

---

### `Nst_was_init`

**Synopsis:**

```better-c
bool Nst_was_init(void)
```

**Description:**

Returns `true` if the state was initialized and `false` otherwise.

---

### `Nst_run`

**Synopsis:**

```better-c
i32 Nst_run(Nst_Program *prog)
```

**Description:**

Run a program.

!!!warning
    It must never be called inside a library.

**Parameters:**

- `prog`: the program to run

**Returns:**

The exit code of the program.

---

### `Nst_run_module`

**Synopsis:**

```better-c
Nst_Obj *Nst_run_module(const char *file_name)
```

**Description:**

Run an external Nest file.

**Parameters:**

- `file_name`: the name of the file to run

**Returns:**

A map containing the variables of the module or NULL on failure. The error is
set.

---

### `Nst_func_call`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_func_call(Nst_Obj *func, usize arg_num, Nst_Obj **args)
```

**Description:**

Call a `Func` object.

!!!note
    If the function is passed less arguments than it expects, the extra ones are
    filled with `null` objects.

**Parameters:**

- `func`: the function to call
- `arg_num`: the number of arguments passed
- `args`: the array of arguments to pass to it

**Returns:**

The return value of the function or `NULL` on failure. The error is set.

---

### `Nst_coroutine_yield`

**Synopsis:**

```better-c
Nst_Obj *Nst_coroutine_yield(Nst_ObjRef **out_stack, usize *out_stack_size,
                             i64 *out_idx, Nst_VarTable *out_vt)
```

**Description:**

Yield a coroutine.

If `stack` is `NULL` the function will only set `out_stack_size` and return
without modifying the state of the interpreter. This is to allow the caller to
allocate enough memory to hold the stack.

**Parameters:**

- `out_stack`: buffer filled with the values from the top function call, each
  object added is a reference to be handled by the caller
- `out_stack_size`: the number of object added to `out_stack`
- `out_idx`: the index of the current instruction
- `out_vt`: the current variable table

**Returns:**

The paused function.

---

### `Nst_coroutine_resume`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_coroutine_resume(Nst_Obj *func, i64 idx,
                                 Nst_ObjRef **value_stack,
                                 usize value_stack_len, Nst_VarTable vt)
```

**Description:**

Execute the body of a `Func` object that has a Nest body using a given context.

The context is set according to the arguments passed.

**Parameters:**

- `func`: the function to execute
- `idx`: the instruction index from which to start the execution of the body
- `value_stack`: the values to push on the value stack
- `value_stack_len`: the length of `value_stack`
- `vt`: variable table to use

**Returns:**

The result of the function or `NULL` on failure. The error is set.

---

### `Nst_state_span`

**Synopsis:**

```better-c
Nst_Span Nst_state_span(void)
```

**Description:**

@return The position of the current operation.

---

### `Nst_state`

**Synopsis:**

```better-c
const Nst_InterpreterState *Nst_state(void)
```

**Description:**

@return The current state of the interpreter.

---

### `Nst_chdir`

**Synopsis:**

```better-c
i32 Nst_chdir(Nst_Obj *str)
```

**Description:**

Change the current working directory using a Nest `Str` object.

**Returns:**

`0` on success and `-1` on failure. The error is set.

---

### `Nst_getcwd`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_getcwd(void)
```

**Description:**

Get the current working directory as a Nest `Str` object.

**Returns:**

the new string or `NULL` on failure. The error is set.
