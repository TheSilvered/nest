# `encoding.h`

Functions to manage various encodings.

## Authors

TheSilvered

---

## Macros

### `Nst_CP_MULTIBYTE_MAX_SIZE`

**Description:**

Maximum size of a multi-byte character across all supported encodings.

---

### `Nst_CP_BOM_MAX_SIZE`

**Description:**

Maximum size of the BOM across all supported encodings.

---

## Structs

### `Nst_CP`

**Synopsis:**

```better-c
typedef struct _Nst_CP {
    const usize ch_size;
    const usize mult_max_sz;
    const usize mult_min_sz;
    const i8 *name;
    const i8 *bom;
    const usize bom_size;
    const Nst_CheckBytesFunc check_bytes;
    const Nst_ToUTF32Func to_utf32;
    const Nst_FromUTF32Func from_utf32;
} Nst_CP
```

**Description:**

The structure that represents an encoding.

**Fields:**

- `ch_size`: the size of one unit in bytes (e.g. is 1 in UTF-8 but 2 in UTF-16)
- `mult_max_sz`: the size in bytes of the longest character
- `mult_min_sz`: the size in bytes of the shortest character (usually the same
  as `ch_size`)
- `name`: the name of the encoding displayed in errors
- `check_bytes`: the
  [`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) function of the
  encoding
- `to_utf32`: the [`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func)
  function of the encoding
- `from_utf32`: the [`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func)
  function of the encoding

---

## Type aliases

### `Nst_CheckBytesFunc`

**Synopsis:**

```better-c
typedef i32 (*Nst_CheckBytesFunc)(void *str, usize len)
```

**Description:**

The signature of a function that checks the length of the first character in a
string of a certain encoding.

---

### `Nst_ToUTF32Func`

**Synopsis:**

```better-c
typedef u32 (*Nst_ToUTF32Func)(void *str)
```

**Description:**

The signature of a function that returns the code point of the first character
in a string of a certain encoding, expecting a valid sequence of bytes.

---

### `Nst_FromUTF32Func`

**Synopsis:**

```better-c
typedef i32 (*Nst_FromUTF32Func)(u32 ch, void *buf)
```

**Description:**

The signature of a function that encodesa code point in a certain encoding and
writes the output to a buffer.

---

## Functions

### `Nst_check_ascii_bytes`

**Synopsis:**

```better-c
i32 Nst_check_ascii_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for ASCII.

---

### `Nst_ascii_to_utf32`

**Synopsis:**

```better-c
u32 Nst_ascii_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for ASCII

---

### `Nst_ascii_from_utf32`

**Synopsis:**

```better-c
i32 Nst_ascii_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for ASCII

---

### `Nst_check_utf8_bytes`

**Synopsis:**

```better-c
i32 Nst_check_utf8_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for UTF-8.

---

### `Nst_utf8_to_utf32`

**Synopsis:**

```better-c
u32 Nst_utf8_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for UTF-8.

---

### `Nst_utf8_from_utf32`

**Synopsis:**

```better-c
i32 Nst_utf8_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for UTF-8.

---

### `Nst_check_ext_utf8_bytes`

**Synopsis:**

```better-c
i32 Nst_check_ext_utf8_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for extUTF-8.

---

### `Nst_ext_utf8_to_utf32`

**Synopsis:**

```better-c
u32 Nst_ext_utf8_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for extUTF-8.

---

### `Nst_ext_utf8_from_utf32`

**Synopsis:**

```better-c
i32 Nst_ext_utf8_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for extUTF-8.

---

### `Nst_check_utf16_bytes`

**Synopsis:**

```better-c
i32 Nst_check_utf16_bytes(u16 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for UTF-16.

---

### `Nst_utf16_to_utf32`

**Synopsis:**

```better-c
u32 Nst_utf16_to_utf32(u16 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for UTF-16.

---

### `Nst_utf16_from_utf32`

**Synopsis:**

```better-c
i32 Nst_utf16_from_utf32(u32 ch, u16 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for UTF-16.

---

### `Nst_check_utf16be_bytes`

**Synopsis:**

```better-c
i32 Nst_check_utf16be_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for UTF-16BE.

---

### `Nst_utf16be_to_utf32`

**Synopsis:**

```better-c
u32 Nst_utf16be_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for UTF-16BE.

---

### `Nst_utf16be_from_utf32`

**Synopsis:**

```better-c
i32 Nst_utf16be_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for UTF-16BE.

---

### `Nst_check_utf16le_bytes`

**Synopsis:**

```better-c
i32 Nst_check_utf16le_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for UTF-16LE.

---

### `Nst_utf16le_to_utf32`

**Synopsis:**

```better-c
u32 Nst_utf16le_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for UTF-16LE.

---

### `Nst_utf16le_from_utf32`

**Synopsis:**

```better-c
i32 Nst_utf16le_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for UTF-16LE.

---

### `Nst_check_ext_utf16_bytes`

**Synopsis:**

```better-c
i32 Nst_check_ext_utf16_bytes(u16 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for extUTF-16.

---

### `Nst_ext_utf16_to_utf32`

**Synopsis:**

```better-c
u32 Nst_ext_utf16_to_utf32(u16 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for extUTF-16.

---

### `Nst_ext_utf16_from_utf32`

**Synopsis:**

```better-c
i32 Nst_ext_utf16_from_utf32(u32 ch, u16 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for extUTF-16.

---

### `Nst_check_ext_utf16be_bytes`

**Synopsis:**

```better-c
i32 Nst_check_ext_utf16be_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for extUTF-16BE.

---

### `Nst_ext_utf16be_to_utf32`

**Synopsis:**

```better-c
u32 Nst_ext_utf16be_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for extUTF-16BE.

---

### `Nst_ext_utf16be_from_utf32`

**Synopsis:**

```better-c
i32 Nst_ext_utf16be_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for extUTF-16BE.

---

### `Nst_check_ext_utf16le_bytes`

**Synopsis:**

```better-c
i32 Nst_check_ext_utf16le_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for extUTF-16LE.

---

### `Nst_ext_utf16le_to_utf32`

**Synopsis:**

```better-c
u32 Nst_ext_utf16le_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for extUTF-16LE.

---

### `Nst_ext_utf16le_from_utf32`

**Synopsis:**

```better-c
i32 Nst_ext_utf16le_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for extUTF-16LE.

---

### `Nst_check_utf32_bytes`

**Synopsis:**

```better-c
i32 Nst_check_utf32_bytes(u32 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for UTF-32.

---

### `Nst_utf32_to_utf32`

**Synopsis:**

```better-c
u32 Nst_utf32_to_utf32(u32 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for UTF-32.

---

### `Nst_utf32_from_utf32`

**Synopsis:**

```better-c
i32 Nst_utf32_from_utf32(u32 ch, u32 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for UTF-32.

---

### `Nst_check_utf32be_bytes`

**Synopsis:**

```better-c
i32 Nst_check_utf32be_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for UTF-32BE.

---

### `Nst_utf32be_to_utf32`

**Synopsis:**

```better-c
u32 Nst_utf32be_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for UTF-32BE.

---

### `Nst_utf32be_from_utf32`

**Synopsis:**

```better-c
i32 Nst_utf32be_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for UTF-32BE.

---

### `Nst_check_utf32le_bytes`

**Synopsis:**

```better-c
i32 Nst_check_utf32le_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for UTF-32LE.

---

### `Nst_utf32le_to_utf32`

**Synopsis:**

```better-c
u32 Nst_utf32le_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for UTF-32LE.

---

### `Nst_utf32le_from_utf32`

**Synopsis:**

```better-c
i32 Nst_utf32le_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for UTF-32LE.

---

### `Nst_check_1250_bytes`

**Synopsis:**

```better-c
i32 Nst_check_1250_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for CP1250.

---

### `Nst_1250_to_utf32`

**Synopsis:**

```better-c
u32 Nst_1250_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for CP1250.

---

### `Nst_1250_from_utf32`

**Synopsis:**

```better-c
i32 Nst_1250_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for CP1250.

---

### `Nst_check_1251_bytes`

**Synopsis:**

```better-c
i32 Nst_check_1251_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for CP1251.

---

### `Nst_1251_to_utf32`

**Synopsis:**

```better-c
u32 Nst_1251_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for CP1251.

---

### `Nst_1251_from_utf32`

**Synopsis:**

```better-c
i32 Nst_1251_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for CP1251.

---

### `Nst_check_1252_bytes`

**Synopsis:**

```better-c
i32 Nst_check_1252_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for CP1252.

---

### `Nst_1252_to_utf32`

**Synopsis:**

```better-c
u32 Nst_1252_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for CP1252.

---

### `Nst_1252_from_utf32`

**Synopsis:**

```better-c
i32 Nst_1252_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for CP1252.

---

### `Nst_check_1253_bytes`

**Synopsis:**

```better-c
i32 Nst_check_1253_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for CP1253.

---

### `Nst_1253_to_utf32`

**Synopsis:**

```better-c
u32 Nst_1253_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for CP1253.

---

### `Nst_1253_from_utf32`

**Synopsis:**

```better-c
i32 Nst_1253_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for CP1253.

---

### `Nst_check_1254_bytes`

**Synopsis:**

```better-c
i32 Nst_check_1254_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for CP1254.

---

### `Nst_1254_to_utf32`

**Synopsis:**

```better-c
u32 Nst_1254_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for CP1254.

---

### `Nst_1254_from_utf32`

**Synopsis:**

```better-c
i32 Nst_1254_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for CP1254.

---

### `Nst_check_1255_bytes`

**Synopsis:**

```better-c
i32 Nst_check_1255_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for CP1255.

---

### `Nst_1255_to_utf32`

**Synopsis:**

```better-c
u32 Nst_1255_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for CP1255.

---

### `Nst_1255_from_utf32`

**Synopsis:**

```better-c
i32 Nst_1255_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for CP1255.

---

### `Nst_check_1256_bytes`

**Synopsis:**

```better-c
i32 Nst_check_1256_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for CP1256.

---

### `Nst_1256_to_utf32`

**Synopsis:**

```better-c
u32 Nst_1256_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for CP1256.

---

### `Nst_1256_from_utf32`

**Synopsis:**

```better-c
i32 Nst_1256_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for CP1256.

---

### `Nst_check_1257_bytes`

**Synopsis:**

```better-c
i32 Nst_check_1257_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for CP1257.

---

### `Nst_1257_to_utf32`

**Synopsis:**

```better-c
u32 Nst_1257_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for CP1257.

---

### `Nst_1257_from_utf32`

**Synopsis:**

```better-c
i32 Nst_1257_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for CP1257.

---

### `Nst_check_1258_bytes`

**Synopsis:**

```better-c
i32 Nst_check_1258_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for CP1258.

---

### `Nst_1258_to_utf32`

**Synopsis:**

```better-c
u32 Nst_1258_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for CP1258.

---

### `Nst_1258_from_utf32`

**Synopsis:**

```better-c
i32 Nst_1258_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for CP1258.

---

### `Nst_check_iso8859_1_bytes`

**Synopsis:**

```better-c
i32 Nst_check_iso8859_1_bytes(u8 *str, usize len)
```

**Description:**

[`Nst_CheckBytesFunc`](c_api-encoding.md#nst_checkbytesfunc) for ISO-8859-1.

---

### `Nst_iso8859_1_to_utf32`

**Synopsis:**

```better-c
u32 Nst_iso8859_1_to_utf32(u8 *str)
```

**Description:**

[`Nst_ToUTF32Func`](c_api-encoding.md#nst_toutf32func) for ISO-8859-1.

---

### `Nst_iso8859_1_from_utf32`

**Synopsis:**

```better-c
i32 Nst_iso8859_1_from_utf32(u32 ch, u8 *str)
```

**Description:**

[`Nst_FromUTF32Func`](c_api-encoding.md#nst_fromutf32func) for ISO-8859-1.

---

### `Nst_utf16_to_utf8`

**Synopsis:**

```better-c
i32 Nst_utf16_to_utf8(i8 *out_str, u16 *in_str, usize in_str_len)
```

**Description:**

Translate the first character of a Unicode (UTF-16) string to UTF-8.

**Parameters:**

- `out_str`: the buffer where the UTF-8 character is written
- `in_str`: the input string to read
- `in_str_len`: the length of the input string

**Returns:**

The function returns the number of bytes written or `-1` on error, no error is
set.

---

### `Nst_translate_cp`

**Synopsis:**

```better-c
bool Nst_translate_cp(Nst_CP *from, Nst_CP *to, void *from_buf, usize from_len,
                      void **to_buf, usize *to_len)
```

**Description:**

Translate a string to another encoding.

All pointers are expected to be valid and not `NULL` except for `to_len` that
can be `NULL` if there is no need to get the length of the output string.

**Parameters:**

- `from`: the encoding of the given string
- `to`: the encoding to translate the string to
- `from_buf`: the initial string
- `from_len`: the length in units of the given string (a unit is 1 byte for
  `char8_t` strings, two bytes for `char16_t` strings etc.)
- `to_buf`: the pointer where the newly translated string is put
- `to_len`: the pointer where the length of the translated string is put, it can
  be `NULL`

**Returns:**

`true` on success and `false` on failure. On failure the error is always set.

---

### `Nst_check_string_cp`

**Synopsis:**

```better-c
isize Nst_check_string_cp(Nst_CP *cp, void *str, usize str_len)
```

**Description:**

Checks the validity of the encoding of a string.

**Parameters:**

- `cp`: the expected encoding of the string
- `str`: the string to check
- `str_len`: the length in units of the string (a unit is 1 byte for `char8_t`
  strings, two bytes for `char16_t` strings etc.)

**Returns:**

The index in units of the first invalid byte or `-1` if the string is correctly
encoded. No error is set.

---

### `Nst_cp`

**Synopsis:**

```better-c
Nst_CP *Nst_cp(Nst_CPID cpid)
```

**Returns:**

The corresponding encoding structure given its ID. If an invalid ID is given,
`NULL` is returned and no error is set.

---

### `Nst_acp`

**Synopsis:**

```better-c
Nst_CPID Nst_acp(void)
```

**Description:**

**WINDOWS ONLY** Returns the Nest code page ID of the local ANSI code page. If
the ANSI code page is not supported,
[`Nst_CP_LATIN1`](c_api-encoding.md#nst_cpid) is returned.

---

### `Nst_char_to_wchar_t`

**Synopsis:**

```better-c
wchar_t *Nst_char_to_wchar_t(i8 *str, usize len)
```

**Description:**

Translates a UTF-8 string to Unicode (UTF-16).

The new string is heap-allocated. str is assumed to be a valid non-NULL pointer.

**Parameters:**

- `str`: the string to translate
- `len`: the length of the string, if 0, it is calculated with
  [`strlen`](https://man7.org/linux/man-pages/man3/strlen.3.html)

**Returns:**

The function returns the new string or NULL on failure. If the function fails,
the error is set.

---

### `Nst_wchar_t_to_char`

**Synopsis:**

```better-c
i8 *Nst_wchar_t_to_char(wchar_t *str, usize len)
```

**Description:**

Translates a Unicode (UTF-16) string to UTF-8.

The new string is heap-allocated. str is assumed to be a valid non-NULL pointer.

**Parameters:**

- `str`: the string to translate
- `len`: the length of the string, if `0`, it is calculated with
  [`wcslen`](https://man7.org/linux/man-pages/man3/wcslen.3.html)

**Returns:**

The function returns the new string or `NULL` on failure. If the function fails,
the error is set.

---

### `Nst_is_valid_cp`

**Synopsis:**

```better-c
bool Nst_is_valid_cp(u32 cp)
```

**Description:**

Returns whether a code point is valid. A valid code point is smaller or equal to
U+10FFFF and is not a high or low surrogate.

---

### `Nst_is_non_character`

**Synopsis:**

```better-c
bool Nst_is_non_character(u32 cp)
```

**Description:**

Returns whether a code is a non character.

---

### `Nst_check_bom`

**Synopsis:**

```better-c
Nst_CPID Nst_check_bom(i8 *str, usize len, i32 *bom_size)
```

**Returns:**

The [`Nst_CPID`](c_api-encoding.md#nst_cpid) deduced from the Byte Order Mark or
[`Nst_CP_UNKNOWN`](c_api-encoding.md#nst_cpid) if no BOM was detected.

---

### `Nst_detect_encoding`

**Synopsis:**

```better-c
Nst_CPID Nst_detect_encoding(i8 *str, usize len, i32 *bom_size)
```

**Description:**

Detects the encoding of a file.

If no valid encoding is detected, [`Nst_CP_LATIN1`](c_api-encoding.md#nst_cpid)
is returned. No error is set.

---

### `Nst_encoding_from_name`

**Synopsis:**

```better-c
Nst_CPID Nst_encoding_from_name(i8 *name)
```

**Returns:**

The encoding ID from a C string, if no matching encoding is found,
[`Nst_CP_UNKNOWN`](c_api-encoding.md#nst_cpid) is returned. No error is set.

---

### `Nst_single_byte_cp`

**Synopsis:**

```better-c
Nst_CPID Nst_single_byte_cp(Nst_CPID cpid)
```

**Returns:**

The little endian variation of a multi-byte encoding or the encoding itself,
though always one with a unit size of one byte.

---

## Enums

### `Nst_CPID`

**Synopsis:**

```better-c
typedef enum _Nst_CPID {
    Nst_CP_UNKNOWN = -1,
    Nst_CP_ASCII,
    Nst_CP_UTF8,
    Nst_CP_EXT_UTF8,
    Nst_CP_UTF16,
    Nst_CP_UTF16BE,
    Nst_CP_UTF16LE,
    Nst_CP_EXT_UTF16,
    Nst_CP_EXT_UTF16BE,
    Nst_CP_EXT_UTF16LE,
    Nst_CP_UTF32,
    Nst_CP_UTF32BE,
    Nst_CP_UTF32LE,
    Nst_CP_1250,
    Nst_CP_1251,
    Nst_CP_1252,
    Nst_CP_1253,
    Nst_CP_1254,
    Nst_CP_1255,
    Nst_CP_1256,
    Nst_CP_1257,
    Nst_CP_1258,
    Nst_CP_LATIN1,
    Nst_CP_ISO8859_1 = Nst_CP_LATIN1
} Nst_CPID
```

**Description:**

The supported encodings in Nest.

[`Nst_CP_UNKNOWN`](c_api-encoding.md#nst_cpid) is -1,
[`Nst_CP_LATIN1`](c_api-encoding.md#nst_cpid) and
[`Nst_CP_ISO8859_1`](c_api-encoding.md#nst_cpid) are equivalent.

!!!note
    [`Nst_CP_EXT_UTF8`](c_api-encoding.md#nst_cpid) is a UTF-8 encoding that
    allows surrogates to be encoded.

!!!note
    [`Nst_CP_EXT_UTF16`](c_api-encoding.md#nst_cpid) along with the little and
    big endian versions are UTF-16 encodings that allow for unpaired surrogates
    with the only constraint being that a high surrogate cannot be the last
    character.
