# `str_view.h`

String view.

## Authors

TheSilvered

---

## Structs

### `Nst_StrView`

**Synopsis:**

```better-c
typedef struct _Nst_StrView {
    u8 *value;
    usize len;
} Nst_StrView
```

**Description:**

A structure representing a string view. It does not own the data in `value`.

**Fields:**

- `value`: the string data
- `len`: the string length

---

## Functions

### `Nst_sv_new`

**Synopsis:**

```better-c
Nst_StrView Nst_sv_new(u8 *value, usize len)
```

**Description:**

Create a new [`Nst_StrView`](c_api-str_view.md#nst_strview) given the `value`
and the `len`.

---

### `Nst_sv_new_c`

**Synopsis:**

```better-c
Nst_StrView Nst_sv_new_c(const char *value)
```

**Description:**

Create a new [`Nst_StrView`](c_api-str_view.md#nst_strview) given a
NUL-terminated string.

---

### `Nst_sv_from_str`

**Synopsis:**

```better-c
Nst_StrView Nst_sv_from_str(Nst_Obj *str)
```

**Description:**

Create a new [`Nst_StrView`](c_api-str_view.md#nst_strview) from a Nest `Str`
object.

---

### `Nst_sv_from_str_slice`

**Synopsis:**

```better-c
Nst_StrView Nst_sv_from_str_slice(Nst_Obj *str, usize start_idx, usize end_idx)
```

**Description:**

Create a new [`Nst_StrView`](c_api-str_view.md#nst_strview) from a slice of a
Nest `Str` object.

**Parameters:**

- `str`: the string to slice
- `start_idx`: the starting character index, included in the slice
- `end_idx`: the ending character index, excluded from the slice

**Returns:**

The new [`Nst_StrView`](c_api-str_view.md#nst_strview). It will have a `len` of
`0` and a `value` of `NULL` if the indices are the same or are invalid. Any
`end_idx` beyond the end of the string is clamped to the length of the string.

---

### `Nst_str_from_sv`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_from_sv(Nst_StrView sv)
```

**Description:**

Create a new Nest `Str` object from an
[`Nst_StrView`](c_api-str_view.md#nst_strview).

---

### `Nst_sv_next`

**Synopsis:**

```better-c
isize Nst_sv_next(Nst_StrView sv, isize idx, u32 *out_ch)
```

**Description:**

Iterate through a [`Nst_StrView`](c_api-str_view.md#nst_strview), assuming it
contains `UTF-8`-encoded data.

**Parameters:**

- `sv`: the string view to iterate through
- `idx`: the current iteration index, use `-1` to begin iterating
- `out_ch`: pointer set to the codepoint of the character, it may be `NULL`

**Returns:**

The next index to pass to this function to continue iterating or a negative
number when no more characters are available.

---

### `Nst_sv_prev`

**Synopsis:**

```better-c
isize Nst_sv_prev(Nst_StrView sv, isize idx, u32 *out_ch)
```

**Description:**

Iterate through a [`Nst_StrView`](c_api-str_view.md#nst_strview) from the end,
assuming it contains `UTF-8`-encoded data.

**Parameters:**

- `sv`: the string view to iterate through
- `idx`: the current iteration index, use `-1` to begin iterating
- `out_ch`: pointer set to the codepoint of the character, it may be `NULL`

**Returns:**

The next index to pass to this function to continue iterating or a negative
number when no more characters are available.

---

### `Nst_sv_parse_int`

**Synopsis:**

```better-c
bool Nst_sv_parse_int(Nst_StrView sv, u8 base, u32 flags, i64 *out_num,
                      Nst_StrView *out_rest)
```

**Description:**

Parse an integer from `sv`. Any leading whitespace is ignored. Whitespace is
determined using
[`Nst_unicode_is_whitespace`](c_api-unicode_db.md#nst_unicode_is_whitespace).

The following flags are accepted and modify the behaviour of the function:

- [`Nst_SVFLAG_CAN_OVERFLOW`](c_api-str_view.md#nst_svnumflags): allows the
  integer to overflow over 2^63-1 or underflow below `-2^63`;
- [`Nst_SVFLAG_FULL_MATCH`](c_api-str_view.md#nst_svnumflags): requires the
  whole string to be matched as the number leaving no characters behind,
  whitespace after the number is trimmed;
- [`Nst_SVFLAG_DISABLE_SEP`](c_api-str_view.md#nst_svnumflags): if set does not
  allow `_` to be used as a separator.

The integer is composed of the following parts:

- an optional sign (`+` or `-`)
- an optional `0b` or `0B` prefix if `base` is `0` or `2`
- an optional `0o` or `0O` prefix if `base` is `0` or `8`
- an optional `0x` or `0X` prefix if `base` is `0` or `16`
- a run of digits, optionally separated by underscores (`_`)

The function parses as many digits as it can. If the base is `0` it is
determined with the prefix: (`0b` or `0B` for binary, `0o` or `0O` for octal,
`0x` or `0X` for hexadecimal and no prefix for decimal).

For bases 2 to 10 the run of digits consists of characters `[0-9]` from base 11
to 36 the letters `[a-z]` and `[A-Z]` are used, the casing is ignored. Hence,
valid bases are `{0, 2, 3, ..., 35, 36}`.

**Parameters:**

- `sv`: the string view to parse
- `base`: the integer base
- `flags`: parsing flags
- `out_num`: pointer set to the parsed number, can be NULL
- `out_rest`: pointer set to the remaining part of `sv`, can be NULL; any
  whitespace after the number is always trimmed

**Returns:**

`true` on success and `false` on failure. On failure `out_num` is set to `0` if
not NULL and `out_rest` is equal to `sv`.

---

### `Nst_sv_parse_byte`

**Synopsis:**

```better-c
bool Nst_sv_parse_byte(Nst_StrView sv, u8 base, u32 flags, u8 *out_num,
                       Nst_StrView *out_rest)
```

**Description:**

Parse an unsigned byte from `sv`. Any leading whitespace is ignored. Whitespace
is determined using
[`Nst_unicode_is_whitespace`](c_api-unicode_db.md#nst_unicode_is_whitespace).

The following flags are accepted and modify the behaviour of the function:

- [`Nst_SVFLAG_CAN_OVERFLOW`](c_api-str_view.md#nst_svnumflags): allows the byte
  to overflow over 255 or underflow below `0`;
- [`Nst_SVFLAG_FULL_MATCH`](c_api-str_view.md#nst_svnumflags): requires the
  whole string to be matched as the number leaving no characters behind,
  whitespace after the number is trimmed;
- [`Nst_SVFLAG_DISABLE_SEP`](c_api-str_view.md#nst_svnumflags): if set does not
  allow `_` to be used as a separator;
- [`Nst_SVFLAG_CHAR_BYTE`](c_api-str_view.md#nst_svnumflags): allows single
  ASCII characters to be parsed as bytes.

The byte is composed of the following parts:

- an optional sign (`+` or `-`), if there is a minus sign and overflow is not
  allowed only `-0` is a valid byte.
- an optional `0b` or `0B` prefix if `base` is `0` or `2`
- an optional `0o` or `0O` prefix if `base` is `0` or `8`
- an optional `0x`, `0X`, `0h` or `0H` prefix if `base` is `0` or `16`
- a run of digits, optionally separated by underscores (`_`)
- an optional suffix `b` or `B`, this is not considered with `base > 10` as `b`
  becomes a digit. This suffix is required when
  [`Nst_SVFLAG_CHAR_BYTE`](c_api-str_view.md#nst_svnumflags) is set.

The function parses as many digits as it can. If the base is `0` it is
determined with the prefix: (`0b` or `0B` for binary, `0o` or `0O` for octal,
`0x` or `0X` for hexadecimal and no prefix for decimal).

For bases 2 to 10 the run of digits consists of characters `[0-9]` from base 11
to 36 the letters `[a-z]` and `[A-Z]` are used, the casing is ignored. Hence,
valid bases are `{0, 2, 3, ..., 35, 36}`.

**Parameters:**

- `sv`: the string view to parse
- `base`: the number base
- `flags`: parsing flags
- `out_num`: pointer set to the parsed number, can be NULL
- `out_rest`: pointer set to the remaining part of `sv`, can be NULL; any
  whitespace after the number is always trimmed

**Returns:**

`true` on success and `false` on failure. On failure `out_num` is set to `0` if
not NULL and `out_rest` is equal to `sv`.

---

### `Nst_sv_parse_real`

**Synopsis:**

```better-c
bool Nst_sv_parse_real(Nst_StrView sv, u32 flags, f64 *out_num,
                       Nst_StrView *out_rest)
```

**Description:**

Parse a double precision floating point number from `sv`. Any leading whitespace
is ignored. Whitespace is determined using
[`Nst_unicode_is_whitespace`](c_api-unicode_db.md#nst_unicode_is_whitespace).

The following flags are accepted and modify the behaviour of the function:

- [`Nst_SVFLAG_FULL_MATCH`](c_api-str_view.md#nst_svnumflags): requires the
  whole string to be matched as the number leaving no characters behind,
  whitespace after the number is trimmed;
- [`Nst_SVFLAG_DISABLE_SEP`](c_api-str_view.md#nst_svnumflags): if set does not
  allow `_` to be used as a separator;
- [`Nst_SVFLAG_STRICT_REAL`](c_api-str_view.md#nst_svnumflags): requires the
  decimal point and that one digit be inserted both before and after it.

The number is composed of the following parts:

- an optional sign (`+` or `-`)
- an optional run of digits, optionally separated by underscores (`_`), required
  with [`Nst_SVFLAG_STRICT_REAL`](c_api-str_view.md#nst_svnumflags)
- an optional dot (`.`), required with
  [`Nst_SVFLAG_STRICT_REAL`](c_api-str_view.md#nst_svnumflags)
- an optional run of digits, optionally separated by underscores (`_`), required
  with [`Nst_SVFLAG_STRICT_REAL`](c_api-str_view.md#nst_svnumflags)
- an optional exponent: `e` or `E` followed by an optional sign (`+` or `-`)
  followed by a run of digits, optionally separated by underscores (`_`)

!!!note
    At least one digit is always required between the sign and the exponent.

**Parameters:**

- `sv`: the string view to parse
- `flags`: parsing flags
- `out_num`: pointer set to the parsed number, can be NULL
- `out_rest`: pointer set to the remaining part of `sv`, can be NULL; any
  whitespace after the number is always trimmed

**Returns:**

`true` on success and `false` on failure. On failure `out_num` is set to `0` if
not NULL and `out_rest` is equal to `sv`.

---

### `Nst_sv_compare`

**Synopsis:**

```better-c
i32 Nst_sv_compare(Nst_StrView str1, Nst_StrView str2)
```

**Description:**

Compare two [`Nst_StrView`](c_api-str_view.md#nst_strview)'s.

**Returns:**

The one of the following values:

- `> 0` if `str1 > str2`
- `== 0` if `str1 == str2`
- `< 0` if `str1 < str2`

---

### `Nst_sv_lfind`

**Synopsis:**

```better-c
isize Nst_sv_lfind(Nst_StrView str, Nst_StrView substr)
```

**Description:**

Search for `substr` inside `str` from the beginning.

**Returns:**

The index where the first occurrence of `substr` appears. If `substr` is not
found in `str` then the function returns `-1`.

---

### `Nst_sv_rfind`

**Synopsis:**

```better-c
isize Nst_sv_rfind(Nst_StrView str, Nst_StrView substr)
```

**Description:**

Search for `substr` inside `str` from the end.

**Returns:**

The index where the last occurrence of `substr` appears. If `substr` is not
found in `str` then the function returns `-1`.

---

### `Nst_sv_ltok`

**Synopsis:**

```better-c
Nst_StrView Nst_sv_ltok(Nst_StrView str, Nst_StrView substr)
```

**Description:**

Search for `substr` inside `str` from the beginning.

**Returns:**

A new string view starting from the end of the first occurrence of `substr`. If
`substr` is not found, the view will have length `0` and a `NULL` value.

---

### `Nst_sv_rtok`

**Synopsis:**

```better-c
Nst_StrView Nst_sv_rtok(Nst_StrView str, Nst_StrView substr)
```

**Description:**

Search for `substr` inside `str` from the end.

**Returns:**

A new string view starting from the end of the last occurrence of `substr`. If
`substr` is not found, the view will have length `0` and a `NULL` value.

---

## Enums

### `Nst_SvNumFlags`

**Synopsis:**

```better-c
typedef enum _Nst_SvNumFlags {
    Nst_SVFLAG_CAN_OVERFLOW =  1,
    Nst_SVFLAG_FULL_MATCH   =  2,
    Nst_SVFLAG_CHAR_BYTE    =  4,
    Nst_SVFLAG_DISABLE_SEP  =  8,
    Nst_SVFLAG_STRICT_REAL  = 16
} Nst_SvNumFlags
```

**Description:**

Flags for [`Nst_sv_parse_int`](c_api-str_view.md#nst_sv_parse_int),
[`Nst_sv_parse_byte`](c_api-str_view.md#nst_sv_parse_byte) and
[`Nst_sv_parse_real`](c_api-str_view.md#nst_sv_parse_real).
