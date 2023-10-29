# String utilities library

## Importing

```nest
|#| 'stdsutil.nest' = su
```

## Functions

### `@bin`

**Synopsis:**

```nest
[n: Int] @bin -> Str
```

**Returns:**

The function returns a string containing the binary representation of `n`
without any prefix.

---

### `@center`

**Synopsis:**

```nest
[string: Str, width: Int, char: Str?] @center -> Str
```

**Description:**

Creates a new string of length `width`, copies `string` centering it and
filling the remaining space with `char`. If the string cannot be perfectly
centered it will be closer to the left side. `char` must have a length of `1`.

If `length` is smaller than or equal to `$string`, the string itself is
returned.

If `char` is `null` a space will be used instead.

**Returns:**

The centered string.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'hello' 11 @su.center --> '   hello   '
'hello' 10 '.' @su.center --> '..hello...'
```

---

### `@decode`

**Synopsis:**

```nest
[sequence: Array|Vector.Byte, encoding: Str] @decode -> Str
```

**Returns:**

Transforms an array of `Byte` objects into a string using the given encoding.

---

### `@encode`

**Synopsis:**

```nest
[string: Str, encoding: Str?] @encode -> Array.Byte
```

**Returns:**

Transforms a string into an array of `Byte` objects using the given encoding.

---

### `@ends_with`

**Synopsis:**

```nest
[string: Str, substring: Str] @ends_with -> Bool
```

**Returns:**

`true` if `string` ends with `substring` and `false` otherwise.

---

### `@hex`

**Synopsis:**

```nest
[n: Int] @hex -> Str
```

**Returns:**

A string containing the hexadecimal representation of `n` without any
prefix.

---

### `@is_alnum`

**Synopsis:**

```nest
[string: Str] @is_alnum -> Bool
```

**Returns:**

`true` if all the characters in `string` are numbers or letters and `false`
otherwise.

---

### `@is_alpha`

**Synopsis:**

```nest
[string: Str] @is_alpha -> Bool
```

**Returns:**

`true` if all the characters in `string` are letters (both uppercase and
lowercase) and `false` otherwise.

---

### `@is_charset`

**Synopsis:**

```nest
[string: Str, charset: Str] @is_charset -> Bool
```

**Returns:**

Whether all the characters in `string` are also contained in `charset`.

---

### `@is_digit`

**Synopsis:**

```nest
[string: Str] @is_digit -> Bool
```

**Returns:**

`true` if all the characters in `string` are numbers and `false` otherwise.
Dots (`.`) and signs (`+` and `-`) are not considered digits.

---

### `@is_lower`

**Synopsis:**

```nest
[string: Str] @is_lower -> Bool
```

**Returns:**

`true` if all the letters in `string` are lowercase and `false` otherwise. Any
non-alphabetical character is ignored.

---

### `@is_printable`

**Synopsis:**

```nest
[string: Str] @is_printable -> Bool
```

**Returns:**

Whether all the characters in `string` are printable. For example `\n`, a line
feed, is not but `a`, the character, is.

---

### `@is_title`

**Synopsis:**

```nest
[string: Str] @is_title -> Bool
```

**Returns:**

`true` if all the words in `string` have the first letter uppercase and the
others lowercase and `false` otherwise. Any non-alphabetical character is
ignored.

---

### `@is_upper`

**Synopsis:**

```nest
[string: Str] @is_upper -> Bool
```

**Returns:**

`true` if all the letters in `string` are uppercase and `false` otherwise. Any
non-alphabetical character is ignored.

---

### `@join`

**Synopsis:**

```nest
[seq: Array|Vector, separator: Str?] @join -> Str
```

**Description:**

Casts all the elements in `seq` and joins them in a string separating them with
`separator`.
If `separator` is `null` a space is used instead.

**Returns:**

A string with all the objects in `seq` joined together.

---

### `@justify`

**Synopsis:**

```nest
[string: Str, width: Int, char: Str?] @justify -> Str
```

**Description:**

Justifies `string` according to `width`. If `width` is positive the string is
justified to the left and if negative it is justified to the right. If
`$string` is greater than the absolute value of `width` the string itself is
returned.

`char` must have a length of one and is used to fill the extra space. If it is
`null` a space is used.

**Arguments:**

- `string`: the string to justify
- `width`: the minimum length in characters of the final string
- `char`: the character used to fill the extra space

**Returns:**

The justified string.

---

### `@lfind`

**Synopsis:**

```nest
[string: Str, substring: Str, start_idx: Int?, end_idx: Int?] @lfind -> Int
```

**Description:**

Finds the first occurrence of `substring` in `string` within `start_idx`
included and `end_idx` excluded starting from the left. If `start_idx` is
`null`, index `0` is used, if `end_idx` is `null` the end of the string is
used.

`start_idx` and `end_idx` are clamped back in the string if outside.

**Returns:**

The index where `substring` starts or `-1` if it is not inside `string`.

---

### `@ltrim`

**Synopsis:**

```nest
[string: Str] @ltrim -> Str
```

**Returns:**

Creates a new string with leading whitespace removed.

---

### `@oct`

**Synopsis:**

```nest
[n: Int] @oct -> Str
```

**Returns:**

A string containing the octal representation of `n` without any prefix.

---

### `@parse_int`

**Synopsis:**

```nest
[string: Str, base: Int?] @parse_int -> Int
```

**Description:**

This function parses an integer of base `base` from `string`. `base` can be any
integer between 2 and 36 inclusive. If it is set to 2, 8 or 16 the
corresponding prefix (`0b`, `0o` or `0x`) is ignored. If set to 0 it will use
base 10 unless it finds one of the prefixes mentioned. Setting `base` to `null`
is the same as setting it to `0`. Any underscore between the digits is ignored.
If `string` does not contain a valid integer literal, an error it thrown.

**Returns:**

The parsed integer.

**Example:**

```nest
|#| 'stdsutil.nest' = su

'0xff' @su.parse_int --> 255
'0xff' 36 @su.parse_int --> 43323
```

---

### `@replace`

**Synopsis:**

```nest
[string: Str, old_substring: Str, new_substing: Str] @replace -> Str
```

**Description:**

Replaces all the occurrences of `old_substring` in `string` with
`new_substing`. `old_substring` and `new_substing` can be of different length.
If `new_substing` is an empty string, nothing is replaced.

**Returns:**

A new string with all the occurrences of the substring replaced.

---

### `@repr`

**Synopsis:**

```nest
[object: Any] @repr -> Str
```

**Description:**

Creates a representation of the value of `object`. This is the same as a cast
to a string for most objects except for objects of type `Str` and `Byte`.

`Str` objects become their literal.

`Byte` object become their literal as well, using a decimal base.

**Returns:**

The string representation of the object given.

**Example:**

```nest
|#| 'stdsutil.nest' = su

10 @su.repr --> '10'
'hello' @su.repr --> "'hello'"
"a'''
b" @su.repr --> "\"a'''\\nb\""
0hf @su.repr --> '15b'
```

---

### `@rfind`

**Synopsis:**

```nest
[string: Str, substring: Str, start_idx: Int?, end_idx: Int?] @rfind -> Int
```

**Description:**

Finds the first occurrence of `substring` in `string` within `start_idx`
included and `end_idx` excluded starting from the right. If `start_idx` is
`null`, index `0` is used, if `end_idx` is `null` the end of the string is
used.

`start_idx` and `end_idx` are clamped back in the string if outside.

**Returns:**

The index where `substring` starts or `-1` if it is not inside `string`.

---

### `@rtrim`

**Synopsis:**

```nest
[string: Str] @rtrim -> Str
```

**Returns:**

Creates a new string with trailing whitespace removed.

---

### `@split`

**Synopsis:**

```nest
[string: Str, separator: Str?, max_splits: Int?] @split -> Vector.Str
```

**Description:**

Splits `string` where it finds `separator` without including it and returns a
vector with all the resulting strings. It stops when it has reached the
specified number of `max_splits` or the last occurrence of the separator is
found.

If `separator` is `null`, the string is split using spaces. One or more
continuous space characters count as one space.

If `max_splits` is negative the function will split `string` at all the
occurrences of `separator`. If `max_splits` is not given, it defaults to `-1`.

**Arguments:**

- `string`: the string to split
- `separator`: the substring to split `string` at
- `max_splits`: the maximum number of splits allowed

**Returns:**

A new vector containing the split strings.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'a  b' @su.split --> <{'a', 'b'}>
'a  b' ' ' @su.split --> <{'a', '', 'b'}>
'a.b.c.d' '.' @su.split --> <{'a', 'b', 'c', 'd'}>
'a.b.c.d' '.' 2 @su.split --> <{'a', 'b', 'c.d'}>
'a.b.c.d' '.' 1 @su.split --> <{'a', 'b.c.d'}>
```

---

### `@starts_with`

**Synopsis:**

```nest
[string: Str, substring: Str] @starts_with -> Bool
```

**Returns:**

`true` if `string` starts with `stubstring` and `false` otherwise.

---

### `@to_lower`

**Synopsis:**

```nest
[string: Str] @to_lower -> Str
```

**Returns:**

A new string is returned with all the letters lowercase. Any non-alphabetical
character is left untouched.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'Hello' @su.to_lower --> 'hello'
```

---

### `@to_title`

**Synopsis:**

```nest
[string: Str] @to_title -> Str
```

**Returns:**

A new string is returned with the first letter of every word uppercase and all
the others lowercase. A word is defined as a piece of text surrounded by
whitespace. Any non-alphabetical character is left untouched.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'this is a sentence' @su.to_title --> 'This Is A Sentence'
"it's three words" @su.to_title --> "It's Three Words"
```

---

### `@to_upper`

**Synopsis:**

```nest
[string: Str] @to_upper -> Str
```

**Returns:**

A new string is returned with all the letters uppercase. Any non-alphabetical
character is left untouched.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'Hello' @su.to_upper --> 'HELLO'
```

---

### `@trim`

**Synopsis:**

```nest
[string: Str] @trim -> Str
```

**Returns:**

Creates a new string with leading and trailing whitespaces removed.

---

## Constants

### `BIN_DIGITS`

Binary digit characters (`01`).

---

### `DIGITS`

Digit characters (`0-9`).

---

### `HEX_DIGITS`

Hexadecimal digit characters (`0-9a-fA-F`).

---

### `LETTERS`

ASCII letter characters both uppercase (`A-Z`) and lowercase (`a-z`).

---

### `LOWERCASE_LETTERS`

Lowercase ASCII letter characters (`a-z`).

---

### `OCT_DIGITS`

Octal digit characters (`0-7`).

---

### `PRINTABLE`

Printable characters (`\x20-\x7e`).

---

### `PUNCTUATION`

Punctuation characters.

---

### `UPPERCASE_LETTERS`

Uppercase ASCII letter characters (`A-Z`).
