# `optimizer.h`

Ast and bytecode optimizer.

## Authors

TheSilvered

---

## Functions

### `Nst_optimize_ast`

**Synopsis:**

```better-c
Nst_Node *Nst_optimize_ast(Nst_Node *ast)
```

**Description:**

Optimizes the AST.

The AST is destroyed if an error occurs.

**Parameters:**

- `ast`: the AST to optimize

**Returns:**

The optimized AST on success and `NULL` on failure. The error is set.

---

### `Nst_optimize_bytecode`

**Synopsis:**

```better-c
Nst_InstList *Nst_optimize_bytecode(Nst_InstList *bc, bool optimize_builtins)
```

**Description:**

Optimizes bytecode.

The bytecode is destroyed if an error occurs.

**Parameters:**

- `bc`: the bytecode to optimize
- `optimize_builtins`: whether accesses to builtins such as Int and false should
  be replaced by their values when they are not modified

**Returns:**

The optimized bytecode on success and `NULL` on failure. The error is set.
