# `compiler.h`

Compiler from AST to bytecode.

## Authors

TheSilvered

---

## Functions

### `Nst_compile`

**Synopsis:**

```better-c
Nst_InstList *Nst_compile(Nst_Node *ast, bool is_module)
```

**Description:**

Compiles the AST.

Both `ast` and `error` are expected to be not NULL. The AST is destroyed when
passed to this function.

**Parameters:**

- `ast`: the AST to compile, will be freed by the function
- `is_module`: whether the AST is of an imported module or of the main file

**Returns:**

The compiled [`Nst_InstList`](c_api-instructions.md#nst_instlist) or `NULL` if
an error occurred. The error is set.
