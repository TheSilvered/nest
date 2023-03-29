# UTF-8 Library

## Importing

```nest
|#| 'stdutf8.nest' = utf8
```

## Functions

### `[string: Str] @is_valid`

Returns `true` if `string` contains valid UTF-8 bytes and `false` otherwise.

### `[string: Str] @get_len`

Returns the Unicode length of `string`. If it is not valid UTF-8 an error is
thrown.

!!!note
    this function operates in linear time, its result should be stored in
    a variable rather than calling it each time

### `[string: Str, index: Int] @get_at`

Returns the Unicode character at `index` in `string`. Negative indices do not
work unlike normal extraction operations.

!!!note
    similarly to `get_len` this function operates in linear time

### `[string: Str] @to_iter`

Returns an iterator that iterates over the Unicode characters inside `string`.
