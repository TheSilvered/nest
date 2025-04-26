# `parser.h`

Token parser into AST.

## Authors

TheSilvered

---

## Functions

### `Nst_parse`

**Synopsis:**

```better-c
Nst_Node *Nst_parse(Nst_DynArray *tokens)
```

**Description:**

Parse a list of tokens into an abstract syntax tree.

**Parameters:**

- `tokens`: the tokens to be parsed

**Returns:**

The AST or `NULL` on failure. The error is set.
