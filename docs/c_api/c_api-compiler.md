# `compiler.h`

Compiler from AST to bytecode.

## Authors

TheSilvered

---

## Functions

### `Nst_compile`

**Synopsis:**

```better-c
Nst_InstList Nst_compile(Nst_Node *ast, bool is_module)
```

**Description:**

Compile the AST.

**Parameters:**

- `ast`: the AST to compile
- `is_module`: whether the AST is of an imported module or of the main file

**Returns:**

The compiled [`Nst_InstList`](c_api-instructions.md#nst_instlist). If an error
occurred the resulting instruction list has no instructions. The error is set.
