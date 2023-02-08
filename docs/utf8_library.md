# UTF-8 Library (`stdutf8.nest` - `utf8`)

## Functions

### `[string: Str] @is_valid`

Returns `true` if `string` contains valid UTF-8 bytes and `false` otherwise.

### `[string: Str] @get_len`

Returns the Unicode length of `string`. If it is not valid UTF-8 an error is
thrown.

> NOTE: since this operation operates in linear time rather than constant it is
> better to store the length in a variable rather than calling this function
> each time

### `[string: Str, index: Int] @get_at`

Returns the Unicode character at `index` in `string`. Negative indices do not
work unlike normal extraction operations.

> NOTE: similarly to `get_len` this function does not operate on constant time

### `[string: Str] @to_iter`

Returns an iterator that iterates over the Unicode characters inside `string`.
