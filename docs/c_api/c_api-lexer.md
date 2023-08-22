# `iter.h`

Lexer and tokenizer for Nest code.

## Authors

TheSilvered

## Functions

### `Nst_tokenizef`

**Synopsis:**

```better-c
Nst_LList *Nst_tokenizef(i8 *filename, Nst_CPID encoding, i32 *opt_level,
                         bool *no_default, Nst_SourceText *src_text,
                         Nst_Error *error)
```

**Description:**

Opens and tokenizes a file.

**Parameters:**

- `filename`: the path to the file
- `encoding`: the supposed encoding of the file, if set to Nst_CP_UNKNOWN it
  will be detected automatically
- `opt_level`: where the optimization level is stored if specified with file
  arguments
- `no_default`: where the --no-default option is stored if specified with file
  arguments
- `src_text`: where the source of the file is saved
- `error`: where the error is put if it occurrs

**Returns:**

A Nst_LList of tokens or NULL on failure. No error is set.

---

### `Nst_tokenize`

**Synopsis:**

```better-c
Nst_LList *Nst_tokenize(Nst_SourceText *text, Nst_Error *error)
```

**Description:**

Tokenizes text.

**Parameters:**

- `text`: the text to tokenize
- `error`: where the error is put if it occurrs

**Returns:**

A Nst_LList of tokens or NULL on failure. No global operation error is set.

---

### `Nst_add_lines`

**Synopsis:**

```better-c
bool Nst_add_lines(Nst_SourceText *text)
```

**Description:**

Adds the lines array to the given text.

On failure the lines field of the struct is set to NULL and lines_len to 0.

**Parameters:**

- `text`: the text to add the line starts to

**Returns:**

true on success and false on failure. No error is set.

---

### `Nst_normalize_encoding`

**Synopsis:**

```better-c
bool Nst_normalize_encoding(Nst_SourceText *text, Nst_CPID encoding,
                            Nst_Error *error)
```

**Description:**

Re-encodes a file to be UTF-8.

**Parameters:**

- `text`: the text to re-encode
- `encoding`: the encoding of the text, if Nst_CP_UNKNWON it is detected
  automatically
- `error`: where the error is put if it occurrs

**Returns:**

true on success and false on failure. No global operation error is set.

