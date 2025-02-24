# `interpreter.h`

Bytecode interpreter.

## Authors

TheSilvered

---

## Structs

### `Nst_IntrState`

**Synopsis:**

```better-c
typedef struct _Nst_IntrState {
    Nst_GarbageCollector ggc;
    i32 opt_level;
    Nst_LList *loaded_libs;
    Nst_LList *lib_paths;
    Nst_Obj *lib_handles;
    Nst_LList *lib_srcs;
    Nst_ExecutionState *es;
    Nst_Traceback global_traceback;
} Nst_IntrState
```

**Description:**

Global state of the Nest interpreter.

**Fields:**

- `ggc`: generational garbage collector
- `opt_level`: maximum optimization level when importing libraries
- `loaded_libs`: dynamic library handles
- `lib_paths`: import stack
- `lib_handles`: maps of imported libraries
- `lib_srcs`: sources of imported Nest libraries
- `es`: the program that is being executed

---

## Functions

### `Nst_init`

**Synopsis:**

```better-c
bool Nst_init(Nst_CLArgs *args)
```

**Description:**

Initializes the Nest libraray.

!!!note
    [`Nst_error_set_color`](c_api-error.md#nst_error_set_color) is called with
    the value returned by
    [`Nst_supports_color`](c_api-argv_parser.md#nst_supports_color).

**Parameters:**

- `args`: the options for the libraray, currently only `opt_level` is used; if
  `NULL` is passed `opt_level` is set to 3.

**Returns:**

`true` on success and `false` on failure. No error is set as it failed to
initialize.

---

### `Nst_quit`

**Synopsis:**

```better-c
void Nst_quit(void)
```

**Description:**

Destroys all the components of the libraray. It is not safe to access any object
created while the library was initialized after this function is called. Any
destructors that may access Nest objects must be called before.

---

### `Nst_run`

**Synopsis:**

```better-c
i32 Nst_run(Nst_Obj *main_func)
```

**Description:**

Runs the main program.

!!!warning
    It must never be called inside a library.

**Parameters:**

- `main_func`: the function object of the main program

**Returns:**

The exit code of the program.

---

### `Nst_run_module`

**Synopsis:**

```better-c
bool Nst_run_module(i8 *file_name, Nst_SourceText *lib_src)
```

**Description:**

Runs an external Nest file.

**Parameters:**

- `file_name`: the name of the file to run, must exist since no checking is done
- `lib_src`: the pointer where to store the source of the file, if an error
  occurs the source of the library is expected to be on the lib_srcs list of the
  global state

**Returns:**

`true` on success and `false` on failure. If the function succeeds, the result
of the module is on top of the value stack. The global operation error is not
set but an internal one is, hence the caller must not set the error.

---

### `Nst_func_call`

**Synopsis:**

```better-c
Nst_Obj *Nst_func_call(Nst_Obj *func, i64 arg_num, Nst_Obj **args)
```

**Description:**

Calls a `Func` object.

!!!note
    If the function is passed less arguments than it expects, the extra ones are
    filled with `null` objects.

**Parameters:**

- `func`: the function to call
- `arg_num`: the number of arguments passed
- `args`: the array of arguments to pass to it

**Returns:**

The result of the function or `NULL` on failure. The error is set.

---

### `Nst_run_paused_coroutine`

**Synopsis:**

```better-c
Nst_Obj *Nst_run_paused_coroutine(Nst_Obj *func, i64 idx, Nst_VarTable *vt)
```

**Description:**

Executes the body of a `Func` object that has a Nest body using a given context.

The context is set according to the arguments passed.

**Parameters:**

- `func`: the function to execute
- `idx`: the instruction index from which to start the execution of the body
- `vars`: the local variable table
- `globals`: the global variable table, it may be `NULL`, in which case it is
  determined automatically

**Returns:**

The result of the function or `NULL` on failure. The error is set internally and
must not be set by the caller.

---

### `Nst_get_full_path`

**Synopsis:**

```better-c
usize Nst_get_full_path(i8 *file_path, i8 **buf, i8 **file_part)
```

**Description:**

Returns the absolute path to a file system object.

!!!note
    The absolute path is allocated on the heap and should be freed with
    [`Nst_free`](c_api-mem.md#nst_free) when appropriate.

**Parameters:**

- `file_path`: the relative path to the object
- `buf`: the buf where the absolute path is placed
- `file_part`: where the start of the file name inside the file path is put,
  this may be `NULL` in which case it is ignored

**Returns:**

The length in bytes of the absolute path or 0 on failure. The error is set.

---

### `Nst_current_inst`

**Synopsis:**

```better-c
Nst_Inst *Nst_current_inst(void)
```

**Description:**

Returns a pointer to the current instruction being executed. On failure `NULL`
is returned. No error is set.

---

### `Nst_was_init`

**Synopsis:**

```better-c
bool Nst_was_init(void)
```

**Description:**

Returns `true` if the state was initialized and `false` otherwise.

---

### `Nst_state_get_es`

**Synopsis:**

```better-c
Nst_ExecutionState *Nst_state_get_es(void)
```

**Description:**

Gets the current execution state.

---

### `Nst_state_set_es`

**Synopsis:**

```better-c
Nst_ExecutionState *Nst_state_set_es(Nst_ExecutionState *es)
```

**Description:**

Sets a new execution state in the global interpreter state.

!!!note
    The current working directory is changed according to `curr_path` in `es`.

**Parameters:**

- `es`: the new [`Nst_ExecutionState`](c_api-runner.md#nst_executionstate) to
  set

**Returns:**

The previous execution state. The error is set but it is not reflected in the
return value. Use [`Nst_error_occurred`](c_api-error.md#nst_error_occurred) to
check.

---

### `Nst_state_get`

**Synopsis:**

```better-c
Nst_IntrState *Nst_state_get(void)
```

**Description:**

Returns a pointer to the global
[`Nst_IntrState`](c_api-interpreter.md#nst_intrstate).

---

### `Nst_chdir`

**Synopsis:**

```better-c
i32 Nst_chdir(Nst_Obj *str)
```

**Description:**

Changes the current working directory using a Nest `Str` object.

**Returns:**

`0` on success and `-1` on failure. The error is set.

---

### `Nst_getcwd`

**Synopsis:**

```better-c
Nst_Obj *Nst_getcwd(void)
```

**Description:**

Gets the current working directory as a Nest `Str` object.

**Returns:**

the new string or `NULL` on failure. The error is set.
