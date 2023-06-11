# `optimizer.h`

This header contains the functions used to optimize the code.

## Functions

### `nst_optimize_ast`

**Synopsis**:

```better-c
Nst_Node *nst_optimize_ast(Nst_Node *ast, Nst_Error *error)
```

**Description**:

Optimizes the abstract syntax tree evaluating expressions with known values.

**Arguments**:

- `[in] ast`: the AST to optimize
- `[out] error`: the error that occurred

**Return value**:

The function returns the optimized AST or `NULL` if it fails, in which case the
AST is automatically freed.

---

### `nst_optimize_bytecode`

**Synopsis**:

```better-c
Nst_InstList *nst_optimize_bytecode(Nst_InstList *bc,
                                    bool optimize_builtins,
                                    Nst_Error *error)
```

**Description**:

Optimizes the bytecode removing unreachable instructions, chained jumps,
variable accesses to predefined variables, swapping instruction blocks that can
be represented by a single instruction and some other things.

**Arguments**:

- `[in] bc`: the bytecode to optimize
- `[in] optimize_builtins`: whether to optimize built-in variables with their
  values
- `[out] error`: the error that occurred

**Return value**:

The function returns the optimized bytecode or `NULL` if it fails, in which case
the instruction list is automatically freed.
