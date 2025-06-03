# `source_loader.h`

Functions for loading Nest source files.

## Authors

TheSilvered

---

## Structs

### `Nst_SourceText`

**Synopsis:**

```better-c
typedef struct _Nst_SourceText {
    char *text;
    char *path;
    char **lines;
    usize text_len;
    usize lines_len;
} Nst_SourceText
```

**Description:**

The structure where the source text of a Nest file is kept.

**Fields:**

- `text`: the UTF-8 text of the file
- `path`: the path of the file
- `lines`: the beginning of each line of the file
- `len`: the length in bytes of `text`
- `lines_len`: the number of lines in the file

---

## Functions

### `Nst_source_load`

**Synopsis:**

```better-c
Nst_SourceText *Nst_source_load(Nst_CLArgs *inout_args)
```

**Description:**

Load a [`Nst_SourceText`](c_api-source_loader.md#nst_sourcetext) from command
line arguments.

The resulting text is managed by Nest and does not need to be freed.

**Returns:**

The source text of the specified file or command, encoded in UTF-8. On failure
the function returns `NULL` and the error is set.

---

### `Nst_source_from_sv`

**Synopsis:**

```better-c
Nst_SourceText *Nst_source_from_sv(Nst_StrView sv)
```

**Description:**

Load a [`Nst_SourceText`](c_api-source_loader.md#nst_sourcetext) from a string
view.

The resulting text is heap allocated and must be destroyed with
[`Nst_source_text_destroy`](c_api-source_loader.md#nst_source_text_destroy).

!!!warning
    Do not use this function to load code that is to be executed by Nest. Use
    [`Nst_source_load`](c_api-source_loader.md#nst_source_load) instead, putting
    the source in the `command` field of
    [`Nst_CLArgs`](c_api-argv_parser.md#nst_clargs).

**Parameters:**

- `sv`: the string view from which the contents are taken to be used in the
  source text, it is expected to be encoded in UTF-8

**Returns:**

The source text created with the contents of `sv`, encoded in UTF-8. On failure
the function returns `NULL` and the error is set.

---

### `Nst_source_from_file`

**Synopsis:**

```better-c
Nst_SourceText *Nst_source_from_file(const char *path, Nst_EncodingID encoding)
```

**Description:**

Load a [`Nst_SourceText`](c_api-source_loader.md#nst_sourcetext) from a file.

The resulting text is heap allocated and must be destroyed with
[`Nst_source_text_destroy`](c_api-source_loader.md#nst_source_text_destroy).

!!!warning
    Do not use this function to load code that is to be executed by Nest. Use
    [`Nst_source_load`](c_api-source_loader.md#nst_source_load) instead, putting
    the path in the `filename` field and the encoding in the `encoding` field of
    [`Nst_CLArgs`](c_api-argv_parser.md#nst_clargs).

**Parameters:**

- `path`: the path to the file
- `encoding`: the encoding to use when reading the file, if it is set to
  [`Nst_EID_UNKNOWN`](c_api-encoding.md#nst_encodingid) it will be determined
  automatically

**Returns:**

The source text created with the contents of `sv`, encoded in UTF-8. On failure
the function returns `NULL` and the error is set.

---

### `Nst_source_text_destroy`

**Synopsis:**

```better-c
void Nst_source_text_destroy(Nst_SourceText *text)
```

**Description:**

Destroy the contents of a
[`Nst_SourceText`](c_api-source_loader.md#nst_sourcetext) and free it.
