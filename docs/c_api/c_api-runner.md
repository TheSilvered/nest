# `runner.h`

Functions for managing execution states.

## Authors

TheSilvered

---

## Structs

### `Nst_ExecutionState`

**Synopsis:**

```better-c
typedef struct _Nst_ExecutionState {
    Nst_Traceback traceback;
    Nst_VarTable *vt;
    i64 idx;
    Nst_Obj *argv;
    Nst_StrObj *curr_path;
    Nst_StrObj *source_path;
    Nst_ValueStack v_stack;
    Nst_CallStack  f_stack;
    Nst_CatchStack c_stack;
} Nst_ExecutionState
```

**Description:**

Execution state of a Nest program.

**Fields:**

- `traceback`: traceback of the current running program
- `vt`: current variable table
- `idx`: current instruction index
- `argv`: arguments passed to the program
- `curr_path`: the current working directory
- `source_path`: the path of the main file
- `v_stack`: value stack
- `f_stack`: call stack
- `c_stack`: catch stack

---

## Functions

### `Nst_es_init`

**Synopsis:**

```better-c
bool Nst_es_init(Nst_ExecutionState *es)
```

**Description:**

Initializes an execution state.

---

### `Nst_es_destroy`

**Synopsis:**

```better-c
void Nst_es_destroy(Nst_ExecutionState *es)
```

**Description:**

Destroys the contents of an execution state.

---

### `Nst_func_call_from_es`

**Synopsis:**

```better-c
Nst_FuncCall Nst_func_call_from_es(Nst_Obj *func, Nst_Pos start, Nst_Pos end,
                                   Nst_ExecutionState *es)
```

**Description:**

Initializes a [`Nst_FuncCall`](c_api-runtime_stack.md#nst_funccall) using the
fields of a [`Nst_ExecutionState`](c_api-runner.md#nst_executionstate).

!!!warning
    The `cwd` argument is set to `NULL` and its value must be set manually.

**Parameters:**

- `func`: the function in the function call
- `start`: the starting position of the function call
- `end`: the ending position of the function call
- `es`: the execution state from which to take `idx`, `cstack_len` and `vt`

**Returns:**

An initialized [`Nst_FuncCall`](c_api-runtime_stack.md#nst_funccall) structure.

---

### `Nst_es_init_vt`

**Synopsis:**

```better-c
bool Nst_es_init_vt(Nst_ExecutionState *es, Nst_CLArgs *cl_args)
```

**Description:**

Initializes the variable table and command-line arguments array of an execution
state.

**Parameters:**

- `es`: the execution state to set the value of
- `argc`: the number of command-line arguments passed, ignoring the filename; if
  this is `0`, `argv` is ignored
- `argv`: an array of strings containing the command-line arguments passed
- `filename`: the name of the file being executed
- `no_default`: whether the variable table should contain any default variable
  (with the exception of `_vars_`)

---

### `Nst_execute`

**Synopsis:**

```better-c
i32 Nst_execute(Nst_CLArgs args, Nst_ExecutionState *es, Nst_SourceText *src)
```

**Description:**

Executes a Nest program given the arguments.

!!!note
    `es` and `src` are not destroyed when the function ends and must be
    destroyed manually with [`Nst_es_destroy`](c_api-runner.md#nst_es_destroy)
    and [`Nst_source_text_destroy`](c_api-error.md#nst_source_text_destroy)
    respectively.

**Parameters:**

- `args`: the arguments for the program
- `es`: the execution state that will be filled by the program
- `src`: the source of the opened file that will be filled by the program

**Returns:**

The exit code of the program. If it is different from zero an error could have
occurred, to check use
[`Nst_error_occurred`](c_api-error.md#nst_error_occurred).

---

### `Nst_es_set_cwd`

**Synopsis:**

```better-c
void Nst_es_set_cwd(Nst_ExecutionState *es, Nst_StrObj *cwd)
```

**Description:**

Sets the `curr_path` field of an
[`Nst_ExecutionState`](c_api-runner.md#nst_executionstate). This function takes
a reference from `cwd` and removes one from the previous value in the state.

**Parameters:**

- `es`: the execution state of which to change `curr_path`
- `cwd`: the new value for `curr_path`

---

### `Nst_es_push_module`

**Synopsis:**

```better-c
bool Nst_es_push_module(Nst_ExecutionState *es, i8 *filename,
                        Nst_SourceText *source_text)
```

**Description:**

Compiles a module given a path and sets up an execution state to run it.

**Parameters:**

- `es`: the execution state on which to run the module
- `filename`: the path of the module to run
- `source_text`: the source text filled with the source of the module

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_es_push_func`

**Synopsis:**

```better-c
bool Nst_es_push_func(Nst_ExecutionState *es, Nst_Obj *func, Nst_Pos start,
                      Nst_Pos end, i64 arg_num, Nst_Obj **args)
```

**Description:**

Pushes a function on the call stack and creates a new local variable table for
the function.

!!!note
    If you pass less arguments than the function expects, the remaining ones are
    filled with `null`.

**Parameters:**

- `es`: the execution state to push the function onto
- `func`: the function to push on the execution state
- `start`: the starting position of the call
- `end`: the ending position of the call
- `arg_num`: the number of arguments passed to the function
- `args`: the values of the arguments to pass to the function, if `NULL`
  `arg_num` values are taken from the value stack in reverse order

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_es_push_paused_coroutine`

**Synopsis:**

```better-c
bool Nst_es_push_paused_coroutine(Nst_ExecutionState *es, Nst_Obj *func,
                                  Nst_Pos start, Nst_Pos end, i64 idx,
                                  Nst_VarTable *vt)
```

**Description:**

Pushes a coroutine that is already running on the call stack of the given
execution state.

**Parameters:**

- `es`: the execution state to push the coroutine onto
- `func`: the function of the coroutine to push
- `start`: the starting position of the call
- `end`: the ending position of the call
- `idx`: the instruction index where the coroutine was paused at
- `vt`: the variable table of the coroutine

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_es_force_function_end`

**Synopsis:**

```better-c
void Nst_es_force_function_end(Nst_ExecutionState *es)
```

**Description:**

Forces the top function of the execution state to end.
