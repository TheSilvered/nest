# `parser.h`

Token parser into AST.

## Authors

TheSilvered

---

## Functions

### `Nst_parse`

**Synopsis:**

```better-c
Nst_Node *Nst_parse(Nst_LList *tokens, Nst_Error *error)
```

**Description:**

Parses a list of tokens into an abstract syntax tree.

The list of tokens is destroyed when calling this function.

**Parameters:**

- `tokens`: the tokens to be parsed
- `error`: a pointer to put the error into

**Returns:**

The AST or `NULL` on failure. No operation error is set.

---

### `Nst_print_ast`

**Synopsis:**

```better-c
void Nst_print_ast(Nst_Node *ast)
```

**Description:**

Prints the AST to the Nest standard output.
