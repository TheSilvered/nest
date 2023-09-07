# `interpreter.h`

Bytecode interpreter.

## Authors

TheSilvered

## Structs

### `Nst_ExecutionState`

**Synopsis:**

```better-c
typedef struct _Nst_ExecutionState {
    Nst_Traceback traceback;
    Nst_VarTable *vt;
    i64 idx;
    Nst_GarbageCollector ggc;
    Nst_StrObj *curr_path;
    Nst_SeqObj *argv;
    i32 opt_level;
    Nst_ValueStack v_stack;
    Nst_CallStack  f_stack;
    Nst_CatchStack c_stack;
    Nst_LList *loaded_libs;
    Nst_LList *lib_paths;
    Nst_MapObj *lib_handles;
    Nst_LList *lib_srcs;
} Nst_ExecutionState
```

**Description:**

Global execution state of Nest.

**Fields:**

- `traceback`: traceback of the current running program
- `vt`: current variable table
- `idx`: current instruction index
- `ggc`: generational garbage collector
- `curr_path`: current working directory
- `argv`: arguments passed to the program
- `opt_level`: maximum optimization level when importing libraries
- `v_stack`: value stack
- `f_stack`: call stack
- `c_stack`: catch stack
- `loaded_libs`: dynamic library handles
- `lib_paths`: import stack
- `lib_handles`: maps of the imported libraries
- `lib_srcs`: sources of the imported Nest libraries

---

## Functions

### `Nst_run`

**Synopsis:**

```better-c
i32 Nst_run(Nst_FuncObj *main_func)
```

**Description:**

Runs the main program.

This function requires [`Nst_state_init`](c_api-interpreter.md#nst_state_init)
to be called before. It will call
[`Nst_state_free`](c_api-interpreter.md#nst_state_free) automatically.

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

### `Nst_call_func`

**Synopsis:**

```better-c
Nst_Obj *Nst_call_func(Nst_FuncObj *func, Nst_Obj **args)
```

**Description:**

Calls a [`Nst_FuncObj`](c_api-function.md#nst_funcobj).

It can have both a Nest or C body.

!!!warning
    No checking is done on the number of arguments.

**Parameters:**

- `func`: the function to call
- `args`: the array of arguments to pass to it, the correct number of arguments
  must be given, no `null` arguments are added

**Returns:**

The result of the function or `NULL` on failure. When a function with a Nest
body fails the error is set internally and the caller must not set it. When a
function with a C body fails, the error should always set.

---

### `Nst_run_func_context`

**Synopsis:**

```better-c
Nst_Obj *Nst_run_func_context(Nst_FuncObj *func, i64 idx, Nst_MapObj *vars,
                              Nst_MapObj *globals)
```

**Description:**

Executes the body of a [`Nst_FuncObj`](c_api-function.md#nst_funcobj) that has a
Nest body using a given context.

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

### `Nst_state_init`

**Synopsis:**

```better-c
bool Nst_state_init(i32 argc, i8 **argv, i8 *filename, i32 opt_level,
                    bool no_default)
```

**Description:**

Initializes the global [`Nst_state`](c_api-interpreter.md#nst_get_state).

**Parameters:**

- `argc`: the command line argument count
- `argv`: the command line arguments
- `filename`: the name of the file of the main program
- `opt_level`: the maximum optimization level
- `no_default`: whether to initialize the variable table of the main program
  with built-in values

**Returns:**

`true` if the state initialized successfully and `false` otherwise. No error is
set.

---

### `Nst_state_was_init`

**Synopsis:**

```better-c
bool Nst_state_was_init(void)
```

**Description:**

Returns `true` if the state was initialized and `false` otherwise.

---

### `Nst_state_free`

**Synopsis:**

```better-c
void Nst_state_free(void)
```

**Description:**

Frees the variables inside the global state, calls `free_lib` in the libraries
that define it and deletes the objects inside the garbage collector.

---

### `_Nst_unload_libs`

**Synopsis:**

```better-c
void _Nst_unload_libs(void)
```

**Description:**

Frees `loaded_libs`, must be called after
[`_Nst_del_objects`](c_api-global_consts.md#_nst_del_objects).

---

### `Nst_get_state`

**Synopsis:**

```better-c
Nst_ExecutionState *Nst_get_state(void)
```

**Description:**

Returns a pointer to the global
[`Nst_ExecutionState`](c_api-interpreter.md#nst_executionstate).

---

### `Nst_chdir`

**Synopsis:**

```better-c
i32 Nst_chdir(Nst_StrObj *str)
```

**Description:**

Changes the current working directory using a
[`Nst_StrObj`](c_api-str.md#nst_strobj).

**Returns:**

`0` on success and `-1` on failure. The error is set.

---

### `Nst_getcwd`

**Synopsis:**

```better-c
Nst_StrObj *Nst_getcwd(void)
```

**Description:**

Gets the current working directory as a [`Nst_StrObj`](c_api-str.md#nst_strobj).

**Returns:**

the new string or `NULL` on failure. The error is set.

