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
    i8 *value;
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
Nst_StrView Nst_sv_new(i8 *value, usize len)
```

**Description:**

Create a new [`Nst_StrView`](c_api-str_view.md#nst_strview) given the `value`
and the `len`.

---

### `Nst_sv_new_c`

**Synopsis:**

```better-c
Nst_StrView Nst_sv_new_c(const i8 *value)
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
