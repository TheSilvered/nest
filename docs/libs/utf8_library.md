# UTF-8 Library

## Importing

```nest
|#| 'stdutf8.nest' = utf8
```

## Functions

### `@is_valid`

**Synopsis**:

`[string: Str] @is_valid -> Bool`

**Return value**:

Returns `true` if `string` is encoded with UTF-8 and `false` otherwise.

---

### `@get_at`

**Synopsis**:

`[string: Str, index: Int] @get_at -> Str`

**Return value**:

Returns the Unicode character at `index` in `string`. Negative indices do not
work unlike normal extraction operations.

!!!note
    This function operates in linear time, its result should be stored in
    a variable rather than calling it each time.

---

### `@get_len`

**Synopsis**:

`[string: Str] @get_len -> Int`

**Return value**:

Returns the Unicode length of `string`. If it is not valid UTF-8 an error is
thrown.

!!!note
    Similarly to `get_at` this function operates in linear time.

---

### `@to_iter`

**Synopsis**:

`[string: Str] @to_iter -> Iter`

**Return value**:

Returns an iterator that iterates over the Unicode characters inside `string`.
