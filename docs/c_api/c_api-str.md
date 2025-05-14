# `str.h`

Nest `Str` object interface.

## Authors

TheSilvered

---

## Macros

### `Nst_STR_LOOP_ERROR`

**Description:**

Value of `idx` in case an error occurs when iterating over a string.

---

## Functions

### `Nst_str_new_c_raw`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_new_c_raw(const char *val, bool allocated)
```

**Description:**

Create a new string object with a value taken from a C string of unknown length.

**Parameters:**

- `val`: the value of the string in extUTF-8 encoding
- `allocated`: whether the value is heap allocated and should be freed with the
  string

**Returns:**

The new string on success and `NULL` on failure. The error is set.

---

### `Nst_str_new_c`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_new_c(const char *val, usize len, bool allocated)
```

**Description:**

Create a new string object from a string literal of known length.

**Parameters:**

- `val`: the value of the string in extUTF-8 encoding
- `len`: the length of `val` in bytes
- `allocated`: whether the value is heap allocated and should be freed with the
  string

**Returns:**

The new string on success and `NULL` on failure. The error is set.

---

### `Nst_str_new`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_new(u8 *val, usize len, bool allocated)
```

**Description:**

Create a new string object.

**Parameters:**

- `val`: the value of the string to create in extUTF-8 encoding
- `len`: the length of `val` in bytes
- `allocated`: whether the value is heap allocated and should be freed with the
  string

**Returns:**

The new string on success and `NULL` on failure. The error is set.

---

### `Nst_str_new_allocated`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_new_allocated(u8 *val, usize len)
```

**Description:**

Create a new string object from a heap-allocated value.

`val` is freed if the string fails to be created.

**Parameters:**

- `val`: the value of the string to create
- `len`: the length of `val` in bytes

**Returns:**

The new string on success and `NULL` on failure. The error is set.

---

### `Nst_str_new_len`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_new_len(u8 *val, usize len, usize char_len, bool allocated)
```

**Description:**

Create a new string object with known length.

**Parameters:**

- `val`: the value of the string to create
- `len`: the length in bytes of `val`
- `char_len`: the length in characters of `val`
- `allocated`: whether `val` is allocated on the heap

**Returns:**

The new string on success and `NULL` on failure. The error is set.

---

### `Nst_str_copy`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_copy(Nst_Obj *src)
```

**Description:**

Create a new string copying the contents of an already existing one.

`src` remains untouched even if the function fails.

**Parameters:**

- `src`: the string to copy

**Returns:**

The copied string on success and `NULL` on failure. The error is set.

---

### `Nst_str_repr`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_repr(Nst_Obj *src)
```

**Description:**

Create a new string by making a string representation of an existing one that
replaces any special characters such as newlines and tabs with their code
representation.

`src` remains untouched even if the function fails.

**Parameters:**

- `src`: the string to make the representation of

**Returns:**

The copied string on success and `NULL` on failure. The error is set.

---

### `Nst_str_get_obj`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_get_obj(Nst_Obj *str, i64 idx)
```

**Description:**

Create a new one-character string with a character of an existing string.

If `idx` negative it is subtracted to the length to get the actual index.

**Parameters:**

- `str`: the string to get the character from
- `idx`: the index of the character to get

**Returns:**

The new string on success and `NULL` on failure. The error is set. The function
fails if the index falls outside the string.

---

### `Nst_str_get`

**Synopsis:**

```better-c
i32 Nst_str_get(Nst_Obj *str, i64 idx)
```

**Description:**

Get the character at index `idx` of the string.

If `idx` negative it is subtracted to the length to get the actual index.

**Returns:**

The character on success and `-1` on failure. The error is set. The function
fails if the index falls outside the string.

---

### `Nst_str_next`

**Synopsis:**

```better-c
isize Nst_str_next(Nst_Obj *str, isize idx)
```

**Description:**

Iterate over the characters of a string.

In order to start pass `-1` as `idx`, this will start from the first character.

**Parameters:**

- `str`: the string to iterate
- `idx`: the current index of the iteration

**Returns:**

The index of the first byte of the character currently being iterated. When
there are no more characters to iterate over a negative value is returned. No
errors can occur.

---

### `Nst_str_next_obj`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_next_obj(Nst_Obj *str, isize *idx)
```

**Description:**

Iterate over the characters of a string.

In order to start set `idx` to `-1`, this will start from the first character.

**Parameters:**

- `str`: the string to iterate
- `idx`: the address to the current index of the iteration

**Returns:**

A `Str` object that contains the character being iterated. It returns `NULL`
when there are no more characters to iterate over or when an error occurs. In
case an error occurs `idx` is set to
[`Nst_STR_LOOP_ERROR`](c_api-str.md#nst_str_loop_error).

---

### `Nst_str_next_utf32`

**Synopsis:**

```better-c
i32 Nst_str_next_utf32(Nst_Obj *str, isize *idx)
```

**Description:**

Iterate over the characters of a string.

In order to start set `idx` to `-1`, this will start from the first character.

**Parameters:**

- `str`: the string to iterate
- `idx`: the address to the current index of the iteration

**Returns:**

The Unicode value of the character. It returns `-1` when there are no more
characters to iterate over or when an error occurs. In case an error occurs
`idx` is set to [`Nst_STR_LOOP_ERROR`](c_api-str.md#nst_str_loop_error).

---

### `Nst_str_next_utf8`

**Synopsis:**

```better-c
i32 Nst_str_next_utf8(Nst_Obj *str, isize *idx, u8 *ch_buf)
```

**Description:**

Iterate over the characters of a string.

In order to start set `idx` to `-1`, this will start from the first character.

**Parameters:**

- `str`: the string to iterate
- `idx`: the address to the current index of the iteration
- `ch_buf`: a buffer of length 4 where the bytes of the character are copied,
  any extra bytes are set to `0`

**Returns:**

The length of the character in bytes. It returns `0` when there are no more
characters to iterate over or when an error occurs. In case an error occurs
`idx` is set to [`Nst_STR_LOOP_ERROR`](c_api-str.md#nst_str_loop_error).

---

### `Nst_str_parse_int`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_parse_int(Nst_Obj *str, i32 base)
```

**Description:**

Parse an `Int` object from a string. Use
[`Nst_sv_parse_int`](c_api-str_view.md#nst_sv_parse_int) for better control.

`base` can be any number between `2` and `36`, where above `10` letters start to
be used and the function is case-insensitive. If the base is `0` the function
uses prefixes such as `0x`, `0b` and `0o` to change the base to `16`, `2` and
`8` respectively. If `base` is set to `2`, `8` or `16` when calling the function
the prefix is optional and is ignored if found. Any underscores (`_`) inside the
number are ignored and any whitespace around the number is ignored.

**Parameters:**

- `str`: the string to parse
- `base`: the base to parse the string in

**Returns:**

The new int object or `NULL` on failure. The error is set.

---

### `Nst_str_parse_byte`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_parse_byte(Nst_Obj *str)
```

**Description:**

Parse a `Byte` object from a string. Use
[`Nst_sv_parse_byte`](c_api-str_view.md#nst_sv_parse_byte) for better control.

If an invalid literal is found, the function fails rather than returning zero.
Valid literals follow Nest's byte syntax, because of this 10 is considered
invalid but 10b is valid.

**Parameters:**

- `str`: the string to parse

**Returns:**

The new byte object or NULL on failure. The error is set.

---

### `Nst_str_parse_real`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_parse_real(Nst_Obj *str)
```

**Description:**

Parse a `Real` object from a string. Use
[`Nst_sv_parse_real`](c_api-str_view.md#nst_sv_parse_real) for better control.

If an invalid literal is found, the function fails rather than returning zero.
Valid literals follow Nest's real syntax, because of this `3.` or `.5` are
considered invalid and must be written as `3.0` and `0.5`.

**Parameters:**

- `str`: the string to parse

**Returns:**

The new real object or `NULL` on failure. The error is set.

---

### `Nst_str_compare`

**Synopsis:**

```better-c
i32 Nst_str_compare(Nst_Obj *str1, Nst_Obj *str2)
```

**Description:**

Compare two Nest strings, similarly to
[`strcmp`](https://man7.org/linux/man-pages/man3/strcmp.3.html).

**Parameters:**

- `str1`: the first string to compare
- `str2`: the second string to compare

**Returns:**

`0` if the two strings are equal, a value `< 0` if `str2` is greater than `str1`
and a value `> 0` if `str1` is greater than `str2`.

---

### `Nst_str_value`

**Synopsis:**

```better-c
u8 *Nst_str_value(Nst_Obj *str)
```

**Returns:**

The value of a Nest `Str` object.

---

### `Nst_str_len`

**Synopsis:**

```better-c
usize Nst_str_len(Nst_Obj *str)
```

**Returns:**

The length in bytes of the value of a Nest `Str` object.

---

### `Nst_str_char_len`

**Synopsis:**

```better-c
usize Nst_str_char_len(Nst_Obj *str)
```

**Returns:**

The number of characters in a Nest `Str` object.

---

## Enums

### `Nst_StrFlags`

**Synopsis:**

```better-c
typedef enum _Nst_StrFlags {
    Nst_FLAG_STR_IS_ALLOC  = Nst_FLAG(1),
    Nst_FLAG_STR_IS_ASCII  = Nst_FLAG(2),
    Nst_FLAG_STR_INDEX_16  = Nst_FLAG(3),
    Nst_FLAG_STR_INDEX_32  = Nst_FLAG(4),
    Nst_FLAG_STR_CAN_INDEX = Nst_FLAG(5)
} Nst_StrFlags
```

**Description:**

Flags for `Str` objects.
