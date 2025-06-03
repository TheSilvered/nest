# `lexer.h`

Lexer and tokenizer for Nest code.

## Authors

TheSilvered

---

## Functions

### `Nst_tokenize`

**Synopsis:**

```better-c
Nst_DynArray Nst_tokenize(Nst_SourceText *text)
```

**Description:**

Tokenize a [`Nst_SourceText`](c_api-source_loader.md#nst_sourcetext).

**Parameters:**

- `text`: the text to tokenize

**Returns:**

A [`Nst_DynArray`](c_api-dyn_array.md#nst_dynarray) of
[`Nst_Tok`](c_api-tokens.md#nst_tok). On failure it has length `0`. The error is
set.
