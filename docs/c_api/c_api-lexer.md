# `lexer.h`

Lexer and tokenizer for Nest code.

## Authors

TheSilvered

---

## Functions

### `Nst_tokenizef`

**Synopsis:**

```better-c
Nst_DynArray Nst_tokenizef(const char *filename, Nst_EncodingID encoding,
                           i32 *opt_level, bool *no_default,
                           Nst_SourceText *src_text)
```

**Description:**

Opens and tokenizes a file.

**Parameters:**

- `filename`: the path to the file
- `encoding`: the supposed encoding of the file, if set to
  [`Nst_EID_UNKNOWN`](c_api-encoding.md#nst_encodingid) it will be detected
  automatically
- `opt_level`: where the optimization level is stored if specified with file
  arguments
- `no_default`: where the `--no-default` option is stored if specified with file
  arguments
- `src_text`: where the source of the file is saved

**Returns:**

A [`Nst_DynArray`](c_api-dyn_array.md#nst_dynarray) of
[`Nst_Tok`](c_api-tokens.md#nst_tok). On failure it has length `0`. The error is
set.

---

### `Nst_tokenize`

**Synopsis:**

```better-c
Nst_DynArray Nst_tokenize(Nst_SourceText *text)
```

**Description:**

Tokenizes text.

**Parameters:**

- `text`: the text to tokenize

**Returns:**

A [`Nst_DynArray`](c_api-dyn_array.md#nst_dynarray) of
[`Nst_Tok`](c_api-tokens.md#nst_tok). On failure it has length `0`. The error is
set.

---

### `Nst_add_lines`

**Synopsis:**

```better-c
bool Nst_add_lines(Nst_SourceText *text)
```

**Description:**

Adds the `lines` array to the given text.

On failure the lines field of the struct is set to `NULL` and lines_len to `0`.

**Parameters:**

- `text`: the text to add the line starts to

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_normalize_encoding`

**Synopsis:**

```better-c
bool Nst_normalize_encoding(Nst_SourceText *text, Nst_EncodingID encoding)
```

**Description:**

Re-encodes a file to be UTF-8.

**Parameters:**

- `text`: the text to re-encode
- `encoding`: the encoding of the text, if
  [`Nst_EID_UNKNOWN`](c_api-encoding.md#nst_encodingid) it is detected
  automatically

**Returns:**

`true` on success and `false` on failure. The error is set.
