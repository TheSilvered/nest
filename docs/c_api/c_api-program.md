# `program.h`

Functions for managing execution states.

## Authors

TheSilvered

---

## Structs

### `Nst_Program`

**Synopsis:**

```better-c
typedef struct _Nst_Program {
    Nst_ObjRef *main_func;
    Nst_ObjRef *argv;
    Nst_ObjRef *source_path;
} Nst_Program
```

**Description:**

A Nest program.

**Fields:**

- `main_func`: the main function of a program
- `argv`: arguments passed to the program
- `source_path`: the path of the main file

---

## Functions

### `Nst_prog_init`

**Synopsis:**

```better-c
Nst_ExecutionKind Nst_prog_init(Nst_Program *prog, Nst_CLArgs args)
```

**Description:**

Initialize a [`Nst_Program`](c_api-program.md#nst_program) to run.

**Parameters:**

- `prog`: the program to initialize
- `args`: the arguments for the program

**Returns:**

How to handle the execution of `es`, see
[`Nst_ExecutionKind`](c_api-program.md#nst_executionkind). The error is set when
[`Nst_EK_ERROR`](c_api-program.md#nst_executionkind) is returned.

---

### `Nst_prog_destroy`

**Synopsis:**

```better-c
void Nst_prog_destroy(Nst_Program *es)
```

**Description:**

Destroy the contents of a program.

---

## Enums

### `Nst_ExecutionKind`

**Synopsis:**

```better-c
typedef enum _Nst_ExecutionKind {
    Nst_EK_ERROR,
    Nst_EK_RUN,
    Nst_EK_INFO
} Nst_ExecutionKind
```

**Description:**

The kind of execution to perform a the program.

**Variants:**

- [`Nst_EK_ERROR`](c_api-program.md#nst_executionkind): an error occurred when
  creating the program
- [`Nst_EK_RUN`](c_api-program.md#nst_executionkind): the program can be run
- [`Nst_EK_INFO`](c_api-program.md#nst_executionkind): only info was requested
  (e.g. tokens), the program must not be run
