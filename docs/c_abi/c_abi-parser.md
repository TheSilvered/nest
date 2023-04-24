# `parser.h`

This header contains the functions used to parse and print the AST of a sequence
of tokens.

## Functions

### `nst_parse`

**Synopsis**:

```better-c
Nst_Node *nst_parse(Nst_LList *tokens, Nst_Error *error)
```

**Description**:

Creates the abstract syntax tree from a list of tokens.

**Arguments**:

- `tokens`: the tokens to parse
- `error`: the error that occured

**Return value**:

The function returns the AST or `NULL` if it fails. The token list is always
freed.

---

### `nst_print_ast`

**Synopsis**:

```better-c
void nst_print_ast(Nst_Node *ast)
```

**Description**:

Prints the abstract syntax tree to stdout.
