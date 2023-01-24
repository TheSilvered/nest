# String utilities library (`stdsutil.nest` - `su`)

## Functions

### `[string: Str, substring: Str] @lfind`

Finds the first occurrence of `substring` in `string` starting from the left and
returns the index of the first character.

### `[string: Str, substring: Str] @rfind`

Finds the first occurrence of `substring` in `string` starting from the right
and returns the index of the first character.

### `[string: Str, substring: Str] @starts_with`

Checks if `substring` is equal to the start of `string`.

### `[string: Str, substring: Str] @ends_with`

Checks if `substring` is equal to the end of `string`.

### `[string: Str] @trim`

Creates a new string with leading and trailing whitespaces removed.

### `[string: Str] @ltrim`

Creates a new string with leading whitespace removed.

### `[string: Str] @rtrim`

Creates a new string with trailing whitespace removed.

### `[string: Str, length: Int, char: Str?] @ljust`

Creates a new string of length `length`, copies `string` justifying it to the
left and fills the remaining space with `char`.  
`char` must be of length 1.  
If `length` is smaller than or equal to `$string`, the string itself is returned.
If `char` is `null` a space will be used instead.

### `[string: Str, length: Int, char: Str?] @rjust`

Creates a new string of length `length`, copies `string` justifying it to the
right and fills the remaining space with `char`.  
`char` must be of length 1.  
If `length` is smaller than or equal to `$string`, the string itself is returned.
If `char` is `null` a space will be used instead.

### `[string: Str] @to_upper`

A new string is returned with all the letters uppercase.

### `[string: Str] @to_lower`

A new string is returned with all the letters lowercase.

### `[string: Str] @is_upper`

Returns whether all the letters in `string` are uppercase, any punctuation is
ignored.

### `[string: Str] @is_lower`

Returns whether all the letters in `string` are lowercase, any punctuation is
ignored.

### `[string: Str] @is_alpha`

Returns whether all the characters in `string` are letters, both uppercase or
lowercase.

### `[string: Str] @is_digit`

Returns whether all the characters in `string` are numbers, dots (`.`) and signs
(`+` and `-`) are not considered digits.

### `[string: Str] @is_alnum`

Returns whether all the characters in `string` are numbers or letters.

### `[string: Str, charset: Str] @is_charset`

Returns whether all the characters in `string` are contained in a charset.  
`'hello' LETTERS @is_charset` is equal to `'hello' @is_alpha` but slower.

### `[string: Str] @is_printable`

Returns whether all the characters in `string` are printable. For example `\n`,
a line feed, is not but `a` is.

### `[string: Str, old_substring: Str, new_substing: Str] @replace_substr`

Replaces all the occurrences of `old_substring` in `string` with `new_substing`.  
`old_substring` and `new_substing` can be of different length.  
If `new_substing` is an empty string, nothing is replaced.

### `[sequence: Array|Vector] @bytearray_to_str`

Transforms an array of `Byte` objects into a string.

### `[string: Str] @str_to_bytearray`

Transforms a string into an array of `Byte` objects.

### `[string: Str] @repr`

Creates a new string representing the string literal of `string`.

### `[seq: Array|Vector, separator: Str?] @join`

Casts all the elements in `seq` and joins them in a string separating them with
`separator`.
If `separator` is `null` a space is used instead.

### `[string: Str, separator: Str?] @split`

Splits `string` where it finds `separator` without including it and returns a
vector with all the resulting strings.
If `separator` is `null`, the string is split using spaces. One or more
continuous space character count as one space.

```text
|#| 'stdsutil.nest' = su
'a  b' @su.split --> <{ 'a', 'b' }>
'a  b' ' ' @su.split --> <{ 'a', '', 'b' }>
```

## Constants

### `DIGITS`

Digit characters (`0-9`).

### `BIN_DIGITS`

Binary digit characters (`01`).

### `OCT_DIGITS`

Octal digit characters (`0-7`).

### `HEX_DIGITS`

Hexadecimal digit characters (`0-9a-fA-F`).

### `LETTERS`

The letter characters both uppercase (`A-Z`) and lowercase (`a-z`).

### `LOWERCASE_LETTERS`

The lowercase letter characters (`a-z`).

### `UPPERCASE_LETTERS`

The uppercase letter characters (`A-A`).

### `PRINTABLE`

All the printable characters (`\x20-\x7e`).

### `PUNCTUATION`

All punctuation characters.
