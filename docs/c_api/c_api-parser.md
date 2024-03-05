# `parser.h`

Token parser into AST.

## Authors

TheSilvered

---

## Functions

### `Nst_parse`

**Synopsis:**

```better-c
Nst_Node *Nst_parse(Nst_LList *tokens)
```

**Description:**

Parses a list of tokens into an abstract syntax tree.

The list of tokens is destroyed when calling this function.

**Parameters:**

- `tokens`: the tokens to be parsed

**Returns:**

The AST or `NULL` on failure. The error is set.
