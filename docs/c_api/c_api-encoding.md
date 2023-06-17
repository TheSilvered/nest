# `encoding.h`

The header that contains the function linked to encoding and decoding of bytes.

## Structs

### `Nst_CP`

**Synopsis**:

```better-c
typedef struct _Nst_CP
{
    usize ch_size;
    usize mult_max_sz;
    usize mult_min_sz;
    const i8 *name;
    Nst_CheckBytesFunc check_bytes;
    Nst_ToUTF32Func to_utf32;
    Nst_FromUTF32Func from_utf32;
}
Nst_CP
```

**Description**:

The structure defining an encoding.

**Fields**:

- `ch_size`: the size in bytes of a unit, for instance 2 in UTF-16 and 4 in
   UTF-32
- `mult_max_sz`: the size in bytes of the longest character, for instance 4 in
   UTF-16
- `mult_min_sz`: the size in bytes of the shortest character, for instance 2 in
   UTF-16
- `name`: a string with the name of the encoding
- `check_bytes`: a function to check the validity of a sequence of encoded bytes
- `to_utf32`: a function that decodes a sequence of bytes
- `from_utf32`: a function that encodes a character

---

## Typedefs

### `Nst_CheckBytesFunc`

**Synopsis**:

```better-c
typedef i32 (*Nst_CheckBytesFunc)(void *str, usize len)
```

---

### `Nst_ToUTF32Func`

**Synopsis**:

```better-c
typedef u32 (*Nst_ToUTF32Func)(void *str)
```

---

### `Nst_FromUTF32Func`

**Synopsis**:

```better-c
typedef i32 (*Nst_FromUTF32Func)(u32 ch, void *buf)
```

---

## Functions

### `nst_check_[CP]_bytes`

**Synopsis**:

```better-c
i32 nst_check_ascii_bytes(u8 *str, usize len)
i32 nst_check_utf8_bytes(u8 *str, usize len)
i32 nst_check_utf16_bytes(u16 *str, usize len)
i32 nst_check_utf32_bytes(u32 *str, usize len)
i32 nst_check_1252_bytes(u8 *str, usize len)
```

**Description**:

These functions check that the first byte of `str` is a valid sequence in the
corresponding encoding.

**Arguments**:

- `[in] str`: the string to check
- `[in] len`: the length of `str`

**Return value**:

The function returns the number of units in the encoding the character occupies,
a unit is the size of one single character, for example `nst_check_utf16_bytes`
can return, on success 1 or 2 and that means that the character occupies either
2 or 4 bytes.  
When an invalid sequence is detected, these functions return -1.

**Example**:

```better-c
// Check if a string is encoded in UTF-8
// `string` is of type `Nst_StrObj *`

i8 *value = string->value;
usize len = string->len;
bool is_valid = true;

for ( int i = 0; i < len; )
{
    int char_size = nst_check_utf8_bytes(value + i, len - i);
    if ( char_size == -1 )
    {
        is_valid = false;
        break;
    }
    i += char_size;
}
```

---

### `nst_[CP]_to_utf32`

**Synopsis**:

```better-c
u32 nst_ascii_to_utf32(u8 *str)
u32 nst_utf8_to_utf32(u8 *str)
u32 nst_utf16_to_utf32(u16 *str)
u32 nst_utf32_to_utf32(u32 *str)
u32 nst_1252_to_utf32(u8 *str)
```

**Description**:

These functions return the Unicode code point of the first character of `str`,
they expect a valid sequence that can be checked with the corresponding
`nst_check_[CP]_bytes` function.

**Arguments**:

- `[in] str`: the string to decode

**Return value**:

These functions return the Unicode Code Point of the first character.

---

### `nst_[CP]_from_utf32`

**Synopsis**:

```better-c
i32 nst_ascii_from_utf32(u32 ch, u8 *str)
i32 nst_utf8_from_utf32(u32 ch, u8 *str)
i32 nst_utf16_from_utf32(u32 ch, u16 *str)
i32 nst_utf32_from_utf32(u32 ch, u32 *str)
i32 nst_1252_from_utf32(u32 ch, u8 *str)
```

**Description**:

These functions encode a Unicode character into a string.

**Arguments**:

- `[in] ch`: the character to encode
- `[inout] str`: the buffer to decode it into

**Return value**:

The function returns the number of units written or `-1` if the character cannot
be decoded.

---

### `nst_cp1252_to_utf8`

**Synopsis**:

```better-c
i32 nst_cp1252_to_utf8(i8 *str, i8 byte)
```

**Description**:

Converts a byte encoded in CP1252 to UTF-8.

**Arguments**:

- `[out] str` the buffer where the bytes will be written
- `[in] byte` the CP1252 byte to convert

**Return value**:

Returns the number of bytes written or `-1` if `byte` was not valid.

---

### `nst_utf16_to_utf8`

**Synopsis**:

```better-c
i32 nst_utf16_to_utf8(i8 *out_str, u16 *in_str, usize in_str_len)
```

**Description**:

Converts a character encoded in UTF-16 to UTF-8.

**Arguments**:

- `[out] out_str` the buffer where the bytes will be written
- `[in] in_str` the character to convert
- `[in] in_str_len` the length of `in_str`

**Return value**:

Returns the number of bytes written or `-1` if an invalid sequence is detected.

---

### `nst_translate_cp`

**Synopsis**:

```better-c
bool nst_translate_cp(Nst_CP *from,
                      Nst_CP *to,
                      void   *from_buf,
                      usize   from_len,
                      void  **to_buf,
                      usize  *to_len,
                      Nst_OpErr *err)
```

**Description**:

This function translates a string from one encoding to another allocating the
necessary space.

**Arguments**:

- `[in] from`: the encoding to translate from
- `[in] to`: the encoding to translate to
- `[in] from_buf`: the buffer to translate
- `[in] from_len`: the length of `from_buf` in units returned by `check_bytes`
  functions
- `[out] to_buf`: the pointer to the translated buffer
- `[out] to_len`: the length of the translated string, may be `NULL`
- `[out] err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

**Example**:

```better-c
// Translating a UTF-16 string to CP-1252
// `wstr` is of type `wchar_t *`
i8 *cp1252_str;
usize cp1252_str_len;

nst_translate_cp(
    nst_cp(NST_CP_UTF16), nst_cp(NST_CP_1252),
    (void *)wstr, wcslen(wstr),
    (void **)&cp1252_str, &cp1252_str_len,
    err);
```

---

### `nst_cp`

**Synopsis**:

```better-c
Nst_CP *nst_cp(Nst_CPID cpid)
```

**Return value**:

Returns a pointer to the specified encoding.

---

### `nst_char_to_wchar_t`

**Synopsis**:

```better-c
wchar_t *nst_char_to_wchar_t(i8 *str, usize len, Nst_OpErr *err)
```

**Description**:

Translates `str` from UTF-8 to UTF-16.

**Arguments**:

- `[in] str`: the string to translate
- `[in] len`: the length of `str`, if 0 `strlen` is used
- `[out] err`: the error

**Return value**:

The function returns the translated string or `NULL` in case of error.

---

### `nst_wchar_t_to_char`

**Synopsis**:

```better-c
i8 *nst_wchar_t_to_char(wchar_t *str, usize len, Nst_OpErr *err)
```

**Description**:

Translates `str` from UTF-16 to UTF-8.

**Arguments**:

- `[in] str`: the string to translate
- `[in] len`: the length of `str`, if 0 `wcslen` is used
- `[out] err`: the error

**Return value**:

The function returns the translated string or `NULL` in case of error.

---

## Enums

### `Nst_CPID`

**Synopsis**:

```better-c
typedef enum _Nst_CPID
{
    NST_CP_ASCII,
    NST_CP_UTF8,
    NST_CP_UTF16,
    NST_CP_UTF32,
    NST_CP_1252
}
Nst_CPID
```

**Description**:

The available encodings in Nest
