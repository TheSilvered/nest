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
Nst_Obj *Nst_str_from_sv(Nst_StrView sv)
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

### `Nst_sv_nextr`

**Synopsis:**

```better-c
isize Nst_sv_nextr(Nst_StrView sv, isize idx, u32 *out_ch)
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

### `Nst_sv_compare`

**Synopsis:**

```better-c
i32 Nst_sv_compare(Nst_StrView str1, Nst_StrView str2)
```

**Description:**

Compare two [`Nst_StrView`](c_api-str_view.md#nst_strview)'s.

**Returns:**

The one of the following values:

- `> 0`: `str1 > str2`
- `== 0`: `str1 == str2`
- `< 0`: `str1 < str2`

---

### `Nst_sv_lfind`

**Synopsis:**

```better-c
isize Nst_sv_lfind(Nst_StrView str, Nst_StrView substr)
```

**Description:**

Searches for `substr` inside `str` from the beginning.

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

Searches for `substr` inside `str` from the end.

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

Searches for `substr` inside `str` from the beginning.

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

Searches for `substr` inside `str` from the end.

**Returns:**

A new string view starting from the end of the last occurrence of `substr`. If
`substr` is not found, the view will have length `0` and a `NULL` value.
