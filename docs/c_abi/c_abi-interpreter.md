# `interpreter.h`

This header contains the structures and functions used to execute Nest bytecode.

## Structs

### `Nst_ExecutionState`

**Synopsis**:

```better-c
typedef struct _Nst_ExecutionState
{
    Nst_Traceback traceback;
    Nst_VarTable *vt;
    Nst_Int idx;
    Nst_GarbageCollector ggc;
    Nst_StrObj *curr_path;
    Nst_SeqObj *argv;
    i32 opt_level;
    Nst_ValueStack *v_stack;
    Nst_CallStack  *f_stack;
    Nst_CatchStack *c_stack;
    Nst_LList *loaded_libs;
    Nst_LList *lib_paths;
    Nst_MapObj *lib_handles;
    Nst_LList *lib_srcs;
}
Nst_ExecutionState
```

**Description**:

This structure holds many variables regarding the state of the code.

**Fields**:

- `traceback`: the back trace of the errors
- `vt`: the current variable table
- `idx`: the index of the current instruction
- `ggc`: the generational garbage collector
- `curr_path`: the current working directory
- `argv`: the arguments passed to Nest
- `opt_level`: the maximum level of optimization for the files
- `v_stack`: the value stack
- `f_stack`: the call (or function) stack
- `c_stack`: the catch frame stack
- `loaded_libs`: the handles to the C libraries loaded
- `lib_paths`: the import stack, used to detect circular imports
- `lib_handles`: a map with the path of the library as the key and a map of the
  contents as the value, used when re-importing a library
- `lib_srcs`: the source texts of the Nest library imported, used to print the
  errors

---

## Functions

### `nst_run`

**Synopsis**:

```better-c
i32 nst_run(Nst_FuncObj *main_func,
            i32          argc,
            i8         **argv,
            i8          *filename,
            i32          opt_lvl,
            bool         no_default)
```

**Description**:

Runs the main program, must never be called.

---

### `nst_run_module`

**Synopsis**:

```better-c
i32 nst_run_module(i8 *file_name, Nst_SourceText *lib_src)
```

**Description**:

Runs an external Nest file. Should not be called, use `nst_obj_import` instead
in `obj_ops.h`.

**Arguments**:

- `[in] file_name`: the path of the module, must exist no checks are done
- `[out] lib_src`: the source of the module

**Return value**:
The function returns -1 on fail and 0 on success

---

### `nst_call_func`

**Synopsis**:

```better-c
Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err)
```

**Description**:

Calls a function object.

**Arguments**:

- `[in] func`: the function to run
- `[in] args`: the arguments to pass to the function
- `[out] err`: the error

**Return value**:

The function returns the value returned by the function or `NULL` on failure.

---

### `nst_run_func_context`

**Synopsis**:

```better-c
Nst_Obj *nst_run_func_context(Nst_FuncObj *func,
                              Nst_Int      idx,
                              Nst_MapObj  *vars,
                              Nst_MapObj  *globals)
```

**Description**:

Runs a *Nest* function giving full control on its execution.

**Arguments**:

- `[in] func`: the function to run
- `[in] idx`: the instruction index at which to start the execution
- `[in] vars`: the local variables to be used
- `[in] globals`: the global variables to be used

**Return value**:

The function returns the value returned by the function or `NULL` on failure.

---

### `nst_get_full_path`

**Synopsis**:

```better-c
usize nst_get_full_path(i8 *file_path, i8 **buf, i8 **file_part, Nst_OpErr *err)
```

**Description**:

Gets the full path from a relative one allocating it on the heap.

**Arguments**:

- `[in] file_path`: the relative path
- `[out] buf`: the place where the buffer is stored
- `[out] file_part`: the place where the file name begins, can be `NULL`
- `[out] err`: the error

**Return value**:

The function returns the length of the path or `0` on failure.

---

### `nst_state_free`

**Synopsis**:

```better-c
void nst_state_free(void)
```

**Description**:

Frees all the variables inside the global `Nst_ExecutionState` except for
`loaded_libs`. Must never be called by a library.

---

### `_nst_unload_libs`

**Synopsis**:

```better-c
void _nst_unload_libs(void)
```

**Description**:

Frees `loaded_libs` of the global `Nst_ExecutionState`, must be called after
`_nst_streams_del` but never by a library.

---

### `nst_get_state`

**Synopsis**:

```better-c
Nst_ExecutionState *nst_get_state(void)
```

**Return value**:

Returns a pointer to the global `Nst_ExecutionState`.
