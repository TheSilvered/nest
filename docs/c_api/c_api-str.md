# `str.h`

Nst_StrObj interface.

## Authors

TheSilvered

---

## Macros

### `STR`

**Synopsis:**

```better-c
#define STR(ptr)
```

**Description:**

Casts `ptr` to [`Nst_StrObj *`](c_api-str.md#nst_strobj).

---

### `TYPE_NAME`

**Synopsis:**

```better-c
#define TYPE_NAME(obj)
```

**Description:**

Gets the name of the type of an object as a C string.

---

### `Nst_TYPE_STR`

**Synopsis:**

```better-c
#define Nst_TYPE_STR(type)
```

**Description:**

Gets the name of the type as a Nest string.

---

### `Nst_STR_IS_ALLOC`

**Synopsis:**

```better-c
#define Nst_STR_IS_ALLOC(str)
```

**Description:**

Checks if the value of a string is allocated.

---

### `Nst_str_copy`

**Synopsis:**

```better-c
#define Nst_str_copy(src)
```

**Description:**

Alias of [`_Nst_str_copy`](c_api-str.md#_nst_str_copy) that casts `src` to
[`Nst_StrObj *`](c_api-str.md#nst_strobj).

---

### `Nst_str_repr`

**Synopsis:**

```better-c
#define Nst_str_repr(src)
```

**Description:**

Alias of [`_Nst_str_repr`](c_api-str.md#_nst_str_repr) that casts `src` to
[`Nst_StrObj *`](c_api-str.md#nst_strobj).

---

### `Nst_str_get`

**Synopsis:**

```better-c
#define Nst_str_get(str, idx)
```

**Description:**

Alias of [`_Nst_str_get`](c_api-str.md#_nst_str_get) that casts `str` to
[`Nst_StrObj *`](c_api-str.md#nst_strobj).

---

### `Nst_STR_LOOP_ERROR`

**Description:**

Value of `idx` in case an error occurs when iterating over a string.

---

## Structs

### `Nst_StrObj`

**Synopsis:**

```better-c
typedef struct _Nst_StrObj {
    Nst_OBJ_HEAD;
    usize len;
    usize char_len;
    i8 *value;
    u8 *indexable_str;
} Nst_StrObj
```

**Description:**

Structure representing a Nest string.

**Fields:**

- `len`: the length in bytes of `value`
- `char_len`: the length in characters of `value`
- `value`: the value of the string
- `indexable_str`: the string in UTF-16 or UTF-32 depending on the characters it
  contains

---

## Functions

### `Nst_str_new_c_raw`

**Synopsis:**

```better-c
Nst_Obj *Nst_str_new_c_raw(const i8 *val, bool allocated)
```

**Description:**

Creates a new string object with a value taken from a C string of unknown
length.

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
Nst_Obj *Nst_str_new_c(const i8 *val, usize len, bool allocated)
```

**Description:**

Creates a new string object from a string literal of known length.

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
Nst_Obj *Nst_str_new(i8 *val, usize len, bool allocated)
```

**Description:**

Creates a new string object.

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
Nst_Obj *Nst_str_new_allocated(i8 *val, usize len)
```

**Description:**

Creates a new string object from a heap-allocated value.

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
Nst_Obj *Nst_str_new_len(i8 *val, usize len, usize char_len, bool allocated)
```

**Description:**

Creates a new string object with known length.

**Parameters:**

- `val`: the value of the string to create
- `len`: the length in bytes of `val`
- `char_len`: the length in characters of `val`
- `allocated`: whether `val` is allocated on the heap

**Returns:**

The new string on success and `NULL` on failure. The error is set.

---

### `Nst_str_temp`

**Synopsis:**

```better-c
Nst_StrObj Nst_str_temp(i8 *val, usize len)
```

**Description:**

Creates a new temporary read-only string object.

This object is not allocated on the heap and cannot be returned by a function,
its intended use is only on functions where a string object is needed but you
have the string in another form. Nothing is allocated and it must not be
destroyed in any way. If the string is indexed, `val` is assumed to contain only
7-bit ASCII characters. If it may not contain only those characters, create a
string with [`Nst_str_new`](c_api-str.md#nst_str_new) or other similar
functions.

**Parameters:**

- `val`: the value of the string
- `len`: the length of the string

**Returns:**

A [`Nst_StrObj`](c_api-str.md#nst_strobj) struct, **NOT POINTER**. This function
never fails.

---

### `_Nst_str_copy`

**Synopsis:**

```better-c
Nst_Obj *_Nst_str_copy(Nst_StrObj *src)
```

**Description:**

Creates a new string copying the contents of an already existing one.

`src` remains untouched even if the function fails.

**Parameters:**

- `src`: the string to copy

**Returns:**

The copied string on success and `NULL` on failure. The error is set.

---

### `_Nst_str_repr`

**Synopsis:**

```better-c
Nst_Obj *_Nst_str_repr(Nst_StrObj *src)
```

**Description:**

Creates a new string by making a string representation of an existing one that
replaces any special characters such as newlines and tabs with their code
representation.

`src` remains untouched even if the function fails.

**Parameters:**

- `src`: the string to make the representation of

**Returns:**

The copied string on success and `NULL` on failure. The error is set.

---

### `_Nst_str_get`

**Synopsis:**

```better-c
Nst_Obj *_Nst_str_get(Nst_StrObj *str, i64 idx)
```

**Description:**

Creates a new one-character string with a character of an existing string.

If `idx` negative it is subtracted to the length to get the actual index.

**Parameters:**

- `str`: the string to get the character from
- `idx`: the index of the character to get

**Returns:**

The new string on success and `NULL` on failure. The error is set. The function
fails if the index falls outside the string.

---

### `Nst_str_next`

**Synopsis:**

```better-c
isize Nst_str_next(Nst_StrObj *str, isize idx)
```

**Description:**

Iterates over the characters of a string.

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
Nst_Obj *Nst_str_next_obj(Nst_StrObj *str, isize *idx)
```

**Description:**

Iterates over the characters of a string.

In order to start set `idx` to `-1`, this will start from the first character.

**Parameters:**

- `str`: the string to iterate
- `idx`: the address to the current index of the iteration

**Returns:**

A [`Nst_StrObj`](c_api-str.md#nst_strobj) that contains the character being
iterated. It returns `NULL` when there are no more characters to iterate over or
when an error occurs. In case an error occurs `idx` is set to
[`Nst_STR_LOOP_ERROR`](c_api-str.md#nst_str_loop_error).

---

### `Nst_str_next_utf32`

**Synopsis:**

```better-c
i32 Nst_str_next_utf32(Nst_StrObj *str, isize *idx)
```

**Description:**

Iterates over the characters of a string.

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
i32 Nst_str_next_utf8(Nst_StrObj *str, isize *idx, i8 *ch_buf)
```

**Description:**

Iterates over the characters of a string.

In order to start set `idx` to `-1`, this will start from the first character.

**Parameters:**

- `str`: the string to iterate
- `idx`: the address to the current index of the iteration
- `ch_buf`: a buffer of length 4 where the bytes of the charcter are copied, any
  extra bytes are set to `0`

**Returns:**

The length of the character in bytes. It returns `0` when there are no more
characters to iterate over or when an error occurs. In case an error occurs
`idx` is set to [`Nst_STR_LOOP_ERROR`](c_api-str.md#nst_str_loop_error).

---

### `Nst_str_parse_int`

**Synopsis:**

```better-c
Nst_Obj *Nst_str_parse_int(Nst_StrObj *str, i32 base)
```

**Description:**

Parses a [`Nst_IntObj`](c_api-simple_types.md#nst_intobj) from a string.

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
Nst_Obj *Nst_str_parse_byte(Nst_StrObj *str)
```

**Description:**

Parses a Nst_ByteObj from a string.

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
Nst_Obj *Nst_str_parse_real(Nst_StrObj *str)
```

**Description:**

Parses a [`Nst_RealObj`](c_api-simple_types.md#nst_realobj) from a string.

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
i32 Nst_str_compare(Nst_StrObj *str1, Nst_StrObj *str2)
```

**Description:**

Compares two Nest strings, similarly to
[`strcmp`](https://man7.org/linux/man-pages/man3/strcmp.3.html) but takes into
account possible `NUL` bytes inside the compared string.

**Parameters:**

- `str1`: the first string to compare
- `str2`: the second string to compare

**Returns:**

`0` if the two strings are equal, a value `< 0` if `str2` is greater than `str1`
and a value `> 0` if `str1` is greater than `str2`.

---

### `_Nst_str_destroy`

**Synopsis:**

```better-c
void _Nst_str_destroy(Nst_StrObj *str)
```

**Description:**

String object destructor.

---

### `Nst_str_find`

**Synopsis:**

```better-c
i8 *Nst_str_find(i8 *s1, usize l1, i8 *s2, usize l2)
```

**Description:**

Finds the first occurrence of a substring inside a string.

If the pointer returned is not `NULL` it is guaranteed to be between `s1 <= p <
s1 + l1`, where `p` is the pointer.

**Parameters:**

- `s1`: the main string
- `l1`: the length of `s1` in bytes
- `s2`: the substring to find inside the main string
- `l2`: the length of `s2` in bytes

**Returns:**

The pointer to the start of `s1` or `NULL` if the string could not be found. No
error is set.

---

### `Nst_str_rfind`

**Synopsis:**

```better-c
i8 *Nst_str_rfind(i8 *s1, usize l1, i8 *s2, usize l2)
```

**Description:**

Finds the first occurrence of a substring inside a string starting from the
right.

If the pointer returned is not `NULL` it is guaranteed to be between `s1 <= p <
s1 + l1`, where `p` is the pointer.

**Parameters:**

- `s1`: the main string
- `l1`: the length of `s1`
- `s2`: the substring to find inside the main string
- `l2`: the length of `s2`

**Returns:**

The pointer to the start of `s1` or `NULL` if the string could not be found. No
error is set.

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

[`Nst_StrObj`](c_api-str.md#nst_strobj)-specific flags.
