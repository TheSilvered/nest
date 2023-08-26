# Codecs Library

## Importing

```nest
|#| 'stdcodecs.nest' = cc
```

## Functions

### `@cp_is_valid`

**Synopsis:**

`[cp: Int|Byte] @cp_is_valid -> Bool`

**Returns:**

`true` if `cp` is a valid unicode code point and `false` otherwise.

---

### `@from_cp`

**Synopsis:**

---

### `@get_at`

**Synopsis:**

`[string: Str, index: Int] @get_at -> Str`

**Returns:**

Returns the Unicode character at `index` in `string`. Negative indices do not
work unlike normal extraction operations.

!!!note
    This function operates in linear time, its result should be stored in
    a variable rather than calling it each time.

---

### `@get_len`

**Synopsis:**

`[string: Str] @get_len -> Int`

**Returns**:

The Unicode length of `string`. If it is not valid UTF-8 an error is thrown.

!!!note
    Similarly to [`get_at`](codecs_library.md#get_at) this function operates in
    linear time.

---

### '@to_cp`

**Synopsis:**

`[character: Str] @to_cp -> Int`

**Return value**:

---

### `@to_iter`

**Synopsis:**

`[string: Str] @to_iter -> Iter`

**Return value**:

Returns an iterator that iterates over the Unicode characters inside `string`.
