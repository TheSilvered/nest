# `compiler.h`

Compiler from AST to bytecode.

## Authors

TheSilvered

## Functions

### `Nst_compile`

**Synopsis:**

```better-c
Nst_InstList *Nst_compile(Nst_Node *ast, bool is_module, Nst_Error *error)
```

**Description:**

Compiles the AST.

Both `ast` and `error` are expected to be not NULL. The AST is destroyed when
passed to this function.

**Parameters:**

- `ast`: the AST to compile, will be freed by the function
- `is_module`: whether the AST is of an imported module or of the main file
- `error`: the error set if one occurs

**Returns:**

The compiled [`Nst_InstList`](c_api-instructions.md#nst_instlist) or `NULL` if
an error occurred.

---

### `Nst_print_bytecode`

**Synopsis:**

```better-c
void Nst_print_bytecode(Nst_InstList *ls)
```

**Description:**

Prints an [`Nst_InstList`](c_api-instructions.md#nst_instlist).

This function is called when using the -b option.

**Parameters:**

- `ls`: the instruction list to print, it is expected to be valid

