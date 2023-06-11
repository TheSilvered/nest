# `lexer.h`

This header contains the functions used by the lexer to create a list of tokens.

## Functions

### `nst_tokenizef`

**Synopsis**:

```better-c
Nst_LList *nst_tokenizef(i8             *filename,
                         bool            force_cp1252,
                         i32            *opt_level,
                         bool           *no_default,
                         Nst_SourceText *src_text,
                         Nst_Error      *error)
```

**Description**:

Creates an list of tokens from a file.

**Arguments**:

- `[in] filename`: the path to the file to tokenize
- `[in] force_cp1252`: force the file to be decoded with CP-1252 encoding
- `[out] opt_level`, `no_default`: the parameters modified by the file arguments
- `[out] src_text`: the source of the file
- `[out] error`: the error that occurred

**Return value**:

The function returns the list of tokens on success and `NULL` on failure.

---

### `nst_tokenize`

**Synopsis**:

```better-c
Nst_LList *nst_tokenize(Nst_SourceText *text, Nst_Error *error)
```

**Description**:

Tokenizes a `Nst_SourceText`.

**Arguments**:

- `[in] text`: the text to tokenize
- `[out] error`: the error

**Return value**:

The function returns the list of tokens on success and `NULL` on failure.

---

### `nst_add_lines`

**Synopsis**:

```better-c
void nst_add_lines(Nst_SourceText *text, i32 start_offset)
```

**Description**:

Populates the `lines` array of `text`.

**Arguments**:

- `[inout] text`: the text to add the lines to
- `[in] start_offset`: an offset in bytes from the beginning of the text that
  sets the starting point

---

### `nst_normalize_encoding`

**Synopsis**:

```better-c
i32 nst_normalize_encoding(Nst_SourceText *text,
                           bool            is_cp1252,
                           Nst_Error      *error)
```

**Description**:

Re-encodes the text with UTF-8.

**Arguments**:

- `[inout] text`: the text to re-encode
- `[in] is_cp1252`: whether to force the detected encoding to be CP-1252
- `[out] error`: the error

**Return value**:

The function returns the offset to pass to `nst_add_lines` or `-1` on failure.
