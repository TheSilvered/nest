# `encoding.h`

The header that contains the function linked to encoding and decoding of bytes.

## Functions

### `nst_check_utf8_bytes`

**Synopsis**:

```better-c
i32 nst_check_utf8_bytes(u8 *byte, usize len)
```

**Description**:

Checks that the first character of `byte` is encoded in UTF-8, this means that
for multi-byte characters more than one byte is checked.

**Arguments**:

- `[in] byte` the bytes to check
- `[in] len` the size of `byte` in bytes

**Return value**:

The return value is the number of bytes that make up the caracter or `-1` if an
invalid sequence is detected.

```better-c
// Check if a string is encoded in UTF-8
// string is of type Nst_StrObj *

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

### `nst_check_utf16_bytes`

**Synopsis**:

```better-c
i32 nst_check_utf16_bytes(u16 *byte, usize len)
```

**Description**:

Checks that the first character of `byte` is encoded in UTF-16, this means that
for multi-byte characters more than one byte is checked.

**Arguments**:

- `[in] byte` the bytes to check
- `[in] len` the length of `byte`

**Return value**:

Returns the length of the character or `-1` if an invalid sequence is found.

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
