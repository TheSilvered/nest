# `str.h`

This header defines the Nest string object.

## Macros

### `STR`

**Synopsis**:

```better-c
STR(ptr)
```

**Description**:

Casts `ptr` to `Nst_StrObj *`.

---

### `TYPE`

**Synopsis**:

```better-c
TYPE(ptr)
```

**Description**:

Casts `ptr` to `Nst_TypeObj *`.

---

### `TYPE_NAME`

**Synopsis**:

```better-c
TYPE_NAME(obj)
```

**Description**:

Gets the C string of the name of the type.

---

### `NST_STR_IS_ALLOC`

**Synopsis**:

```better-c
NST_STR_IS_ALLOC(str)
```

**Description**:

Checks if a string has the `NST_FLAG_STR_IS_ALLOC` flag.

---

### `nst_string_copy`

**Synopsis**:

```better-c
nst_string_copy(src, err)
```

**Description**:

Alias of [`_nst_string_copy`](#_nst_string_copy) that casts `src` to a
`Nst_StrObj *`.

---

### `nst_string_repr`

**Synopsis**:

```better-c
nst_string_repr(src, err)
```

**Description**:

Alias of [`_nst_string_repr`](#_nst_string_repr) that casts `src` to a
`Nst_StrObj *`.

---

### `nst_string_get`

**Synopsis**:

```better-c
nst_string_get(str, idx, err)
```

**Description**:

Alias of [`_nst_string_get`](#_nst_string_get) that casts `str` to a
`Nst_StrObj *`.

---

## Structs

### `Nst_StrObj`

**Synopsis**:

```better-c
typedef struct _Nst_StrObj
{
    NST_OBJ_HEAD;
    usize len;
    i8 *value;
}
Nst_StrObj
```

**Description**:

The structure defining the Nest string object.

**Fields**:

- `len`: the length of the string
- `value`: the raw string data

---

## Type Aliases

### `Nst_TypeObj`

**Synopsis**:

```better-c
typedef Nst_StrObj Nst_TypeObj
```

---

## Functions

### `nst_string_new_c_raw`

**Synopsis**:

```better-c
Nst_Obj *nst_string_new_c_raw(const i8 *val, bool allocated, Nst_OpErr *err)
```

**Description**:

Creates a new string from a C string using `strlen` for the length.

**Arguments**:

- `[in] val`: the value of the string
- `[in] allocated`: whether the value should be freed when the string is deleted
- `[out] err`: the error

**Return value**:

The function returns the new string or `NULL` on failure.

---

### `nst_string_new_c`

**Synopsis**:

```better-c
Nst_Obj *nst_string_new_c(const i8 *val, usize len, bool allocated, Nst_OpErr *err)
```

**Description**:

Creates a new string from a C string with a given length.

**Arguments**:

- `[in] val`: the value of the string
- `[in] len`: the length in bytes of the string
- `[in] allocated`: whether the value should be freed when the string is deleted
- `[out] err`: the error

**Return value**:

The function returns the new string or `NULL` on failure.

---

### `nst_string_new`

**Synopsis**:

```better-c
Nst_Obj *nst_string_new(i8 *val, usize len, bool allocated, Nst_OpErr *err)
```

**Description**:

Creates a new string from a buffer of known length.

**Arguments**:

- `[in] val`: the value of the string
- `[in] len`: the length in bytes of the string
- `[in] allocated`: whether the value should be freed when the string is deleted
- `[out] err`: the error

**Return value**:

The function returns the new string or `NULL` on failure.

---

### `nst_type_new`

**Synopsis**:

```better-c
Nst_TypeObj *nst_type_new(const i8 *val, usize len, Nst_OpErr *err)
```

**Description**:

Creates a new type object. This assumes that `val` is not heap-allocated.

**Arguments**:

- `[in] val`: the name of the type
- `[in] len`: the length of the name
- `[out] err`: the error

**Return value**:

The function returns the new type or `NULL` on failure.

---

### `_nst_string_copy`

**Synopsis**:

```better-c
Nst_Obj *_nst_string_copy(Nst_StrObj *src, Nst_OpErr *err)
```

**Description**:

Creates a new string copying the contents.

**Arguments**:

- `[in] src`: the string to copy
- `[out] err`: the error

**Return value**:

The function returns the copied string or `NULL` on failure.

---

### `_nst_string_repr`

**Synopsis**:

```better-c
Nst_Obj *_nst_string_repr(Nst_StrObj *src, Nst_OpErr *err)
```

**Description**:

Creates a new string that is the source-code representation of `src`.

**Arguments**:

- `[in] src`: the string to transform
- `[out] err`: the error


**Return value**:

The function returns the new string or `NULL` on failure.

**Example**:

```better-c
Nst_StrObj *str = nst_string_new_c_raw("Hello, world!\n", false, NULL);
Nst_StrObj *repr = nst_string_repr(str, NULL);
repr->value; // "'Hello, world!\\n'"
```

---

### `_nst_string_get`

**Synopsis**:

```better-c
Nst_Obj *_nst_string_get(Nst_StrObj *str, i64 idx, Nst_OpErr *err)
```

**Description**:

Creates a one-character string with the character at `idx` of the string.
Negative indices are interpreted as starting from the end of the string.

**Arguments**:

- `[in] str`: the string to the the character of
- `[in] idx`: the index of the character
- `[out] err`: the error

**Return value**:

The function returns the new string or `NULL` on failure.

---

### `nst_string_parse_int`

**Synopsis**:

```better-c
Nst_Obj *nst_string_parse_int(Nst_StrObj *str, i32 base, struct _Nst_OpErr *err)
```

**Description**:

Parses a `Nst_IntObj` from a string. When `base` is `0` the number is
interpreted in base 10 unless a prefix is used (`0b`, `0o` or `0x`).  
When using bases 2, 8 and 16 the prefix is ignored if found.  
Valid values for `base` are between 2 and 36 or 0, any other base is invalid.  
This function accepts underscores (`_`) between the digits.

**Arguments**:

- `[in] str`: the string to parse
- `[in] base`: the base of the number to parse, bases above 10 use the alphabet
- `[out] err`: the error

**Return value**:

The function returns the parsed integer or `NULL` on failure.

---

### `nst_string_parse_byte`

**Synopsis**:

```better-c
Nst_Obj *nst_string_parse_byte(Nst_StrObj* str, struct _Nst_OpErr* err)
```

**Description**:

Parses a `Nst_ByteObj *` from a string. When the string is one character long,
the character's ASCII is returned otherwise the normal Nest byte literals are
parsed. If the literal is not a byte (e.g. `"10"` instead of `"10b"`) the
function fails.

**Arguments**:

- `[in] str`: the string to parse
- `[out] err`: the error

**Return value**:

The function returns the parsed byte or `NULL` on failure.

---

### `nst_string_parse_real`

**Synopsis**:

```better-c
Nst_Obj *nst_string_parse_real(Nst_StrObj *str, struct _Nst_OpErr *err)
```

**Description**:

Parses a `Nst_RealObj *` from a string using normal Nest real literals are used.

**Arguments**:

- `[in] str`: the string to parse
- `[out] err`: the error

**Return value**:

The function returns the parsed real or `NULL` on failure.

---

### `nst_string_compare`

**Synopsis**:

```better-c
i32 nst_string_compare(Nst_StrObj *str1, Nst_StrObj *str2)
```

**Description**:

Compares two Nest strings.

**Arguments**:

- `[in] str1`: the first string to compare
- `[in] str2`: the second string to compare

**Return value**:

The function returns:

- `0` if the strings are equal
- `> 0` if `str1` is greater than `str2`
- `< 0` if `str2` is greater than `str1`

---

### `nst_string_find`

**Synopsis**:

```better-c
i8 *nst_string_find(i8 *s1, usize l1, i8 *s2, usize l2)
```

**Description**:

Finds the position of a substring.

**Arguments**:

- `[in] s1`: the main string
- `[in] l1`: the length of the main string
- `[in] s2`: the substring of which to find the position
- `[in] l2`: the length of the substring

**Return value**:

The function returns the pointer to the start of the firs occurrence of `s2` in
`s1` or `NULL` if it could not be found.

---

### `_nst_string_destroy`

**Synopsis**:

```better-c
void _nst_string_destroy(Nst_StrObj *str)
```

**Description**:

Destroys a string object, should never be called.

---

## Enums

### `Nst_StrFlags`

**Synopsis**:

```better-c
typedef enum _Nst_StrFlags
{
    NST_FLAG_STR_IS_ALLOC = 0b1
}
Nst_StrFlags
```

**Description**:

The flags used by the string object.

**Fields**:

- `NST_FLAG_STR_IS_ALLOC`: whether the `value` of the string should be freed
    when the string is destroyed
