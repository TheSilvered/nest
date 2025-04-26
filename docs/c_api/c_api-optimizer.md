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

Optimize the AST.

The AST is destroyed if an error occurs.

**Parameters:**

- `ast`: the AST to optimize

**Returns:**

The optimized AST on success and `NULL` on failure. The error is set.

---

### `Nst_optimize_ilist`

**Synopsis:**

```better-c
void Nst_optimize_ilist(Nst_InstList *ls, bool optimize_builtins)
```

**Description:**

Optimize an instruction list. This function is guaranteed to not fail.

**Parameters:**

- `ls`: the instruction list to optimize
- `optimize_builtins`: whether accesses to builtins such as Int and false should
  be replaced by their values when they are not modified
