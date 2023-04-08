# String utilities library

## Importing

```nest
|#| 'stdsutil.nest' = su
```

## Functions

### `@bin`

**Synopsis**:

`[n: Int] @bin -> Str`

**Return value**:

The function returns a string containing the binary representation of `n`
without any prefix.

---

### `@bytearray_to_str`

**Synopsis**:

`[sequence: Array|Vector] @bytearray_to_str -> Str`

**Return value**:

Transforms an array of `Byte` objects into a string.

---

### `@center`

**Synopsis**:

`[string: Str, length: Int, char: Str?] @center -> Str`

**Description**:

Creates a new string of length `length`, copies `string` centering it and
filling the remaining space with `char`. If the string cannot be perfectly
centered it will be closer to the left side.  
`char` must be of length 1.  
If `length` is smaller than or equal to `$string`, the string itself is returned.
If `char` is `null` a space will be used instead.

**Return value**:

The function returns the centered string.

**Example**:

```nest
|#| 'stdsutil.nest' = su
'hello' 11 '*' @su.center --> '***hello***'
'hello' 10 '.' @su.center --> '..hello...'
```

---

### `@ends_with`

**Synopsis**:

`[string: Str, substring: Str] @ends_with -> Bool`

**Return value**:

Returns `true` if `substring` is equal to the end of `string` and `false`
otherwise.

---

### `@hex`

**Synopsis**:

`[n: Int] @hex -> Str`

**Return value**:

The function returns a string containing the hexadecimal representation of `n`
without any prefix.

---

### `@is_alnum`

**Synopsis**:

`[string: Str] @is_alnum -> Bool`

**Return value**:

Returns whether all the characters in `string` are numbers or letters.

---

### `@is_alpha`

**Synopsis**:

`[string: Str] @is_alpha -> Bool`

**Return value**:

Returns whether all the characters in `string` are letters, both uppercase or
lowercase.

---

### `@is_charset`

**Synopsis**:

`[string: Str, charset: Str] @is_charset -> Bool`

**Return value**:

Returns whether all the characters in `string` are contained in a charset.  
`'hello' LETTERS @is_charset` is equal to `'hello' @is_alpha` but slower.

---

### `@is_digit`

**Synopsis**:

`[string: Str] @is_digit -> Bool`

**Return value**:

Returns whether all the characters in `string` are numbers, dots (`.`) and signs
(`+` and `-`) are not considered digits.

---

### `@is_lower`

**Synopsis**:

`[string: Str] @is_lower -> Bool`

**Return value**:

Returns whether all the letters in `string` are lowercase, any punctuation is
ignored.

---

### `@is_printable`

**Synopsis**:

`[string: Str] @is_printable -> Bool`

**Return value**:

Returns whether all the characters in `string` are printable. For example `\n`,
a line feed, is not but `a` is.

---

### `@is_title`

**Synopsis**:

`[string: Str] @is_title -> Bool`

**Return value**:

Returns whether all the words in `string` have the first letter uppercase and
the others lowercase.

---

### `@is_upper`

**Synopsis**:

`[string: Str] @is_upper -> Bool`

**Return value**:

Returns whether all the letters in `string` are uppercase, any punctuation is
ignored.

---

### `@join`

**Synopsis**:

`[seq: Array|Vector, separator: Str?] @join -> Str`

**Description**:

Casts all the elements in `seq` and joins them in a string separating them with
`separator`.  
If `separator` is `null` a space is used instead.

**Return value**:

The function returns a string with all the objects in `seq` joined together.

---

### `@lfind`

**Synopsis**:

`[string: Str, substring: Str] @lfind -> Int`

**Description**:

Finds the first occurrence of `substring` in `string` starting from the left and
returns the index of the first character.

**Return value**:

The function returns the index where `substring` starts or `-1` if it is not
inside `string`.

---

### `@ljust`

**Synopsis**:

`[string: Str, length: Int, char: Str?] @ljust -> Str`

**Description**:

Creates a new string of length `length`, copies `string` justifying it to the
left and fills the remaining space with `char`.  
`char` must be of length 1.  
If `length` is smaller than or equal to `$string`, the string itself is returned.
If `char` is `null` it defaults to `' '`.

**Return value**:

The function returns the justified string.

---

### `@ltrim`

**Synopsis**:

`[string: Str] @ltrim -> Str`

**Return value**:

Creates a new string with leading whitespace removed.

---

### `@oct`

**Synopsis**:

`[n: Int] @oct -> Str`

**Return value**:

The function returns a string containing the octal representation of `n` without
any prefix.

---

### `@parse_int`

**Synopsis**:

`[string: Str, base: Int?] @parse_int -> Int`

**Description**:

This function parses an integer of base `base` from `string`. `base` can be any
integer between 2 and 36 inclusive. If it is set to 2, 8 or 16 the corresponding
prefix (`0b`, `0o` or `0x`) is ignored. If set to 0 it will use base 10 unless
it finds one of the specified prefixes. Setting `base` to `null` is the same as
setting it to `0`. Any underscore between the digits is ignored.  
If `string` does not contain a valid integer literal, an error it thrown.

**Return value**:

The function returns the parsed digit.

**Example**:

```nest
|#| 'stdsutil.nest' = su

'0xff' @su.parse_int --> 255
'0xff' 36 @su.parse_int --> 43323
```

---

### `@replace_substr`

**Synopsis**:

`[string: Str, old_substring: Str, new_substing: Str] @replace_substr -> Str`

**Description**:

Replaces all the occurrences of `old_substring` in `string` with `new_substing`.  
`old_substring` and `new_substing` can be of different length.  
If `new_substing` is an empty string, nothing is replaced.

**Return value**:

The function returns a new string with all the occurrences of the substring
replaced.

---

### `@repr`

**Synopsis**:

`[object: Any] @repr -> Str`

**Description**:

Creates a representation of the value of `object`. This is the same as a cast to
a string for most objects except for objects of type `Str` and `Byte`.  
`Str` objects become their literal so a string that when printed outputs `'\n'`,
becomes `"'\\n'"`.  
`Byte` object become their literal as well, using a decimal base.

**Return value**:

The function returns the string representation of the object given.

**Example**:

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

**Synopsis**:

`[string: Str, substring: Str] @rfind -> Int`

**Description**:

Finds the first occurrence of `substring` in `string` starting from the right
and returns the index of the first character.

**Return value**:

The function returns the index where `substring` starts or `-1` if it is not
inside `string`.

---

### `@rjust`

**Synopsis**:

`[string: Str, length: Int, char: Str?] @rjust -> Str`

**Description**:

Creates a new string of length `length`, copies `string` justifying it to the
right and fills the remaining space with `char`.  
`char` must be of length 1.  
If `length` is smaller than or equal to `$string`, the string itself is returned.
If `char` is `null` a space will be used instead.

**Return value**:

The function returns the justified string.

---

### `@rtrim`

**Synopsis**:

`[string: Str] @rtrim -> Str`

**Return value**:

Creates a new string with trailing whitespace removed.

---

### `@split`

**Synopsis**:

`[string: Str, separator: Str?] @split -> Vector`

**Description**:

Splits `string` where it finds `separator` without including it and returns a
vector with all the resulting strings.
If `separator` is `null`, the string is split using spaces. One or more
continuous space character count as one space.

**Return value**:

The function returns a new vector containing the split strings.

**Example**:

```nest
|#| 'stdsutil.nest' = su
'a  b' @su.split --> <{ 'a', 'b' }>
'a  b' ' ' @su.split --> <{ 'a', '', 'b' }>
```

---

### `@starts_with`

**Synopsis**:

`[string: Str, substring: Str] @starts_with -> Bool`

**Return value**:

Returns `true` if `substring` is equal to the start of `string` and `false`
otherwise.

---

### `@str_to_bytearray`

**Synopsis**:

`[string: Str] @str_to_bytearray -> Array`

**Return value**:

Transforms a string into an array of `Byte` objects.

---

### `@to_lower`

**Synopsis**:

`[string: Str] @to_lower -> Str`

**Return value**:

A new string is returned with all the letters lowercase.

**Example**:

```nest
|#| 'stdsutil.nest' = su
'Hello' @su.to_lower --> 'hello'
```

---

### `@to_title`

**Synopsis**:

`[string: Str] @to_title -> Str`

**Return value**:

A new string is returned with the first letter of every word uppercase and all
the others lowercase. A word is defined as a piece of text surrounded by
whitespace.

**Example**:

```nest
|#| 'stdsutil.nest' = su
'this is a sentence' @su.to_title --> 'This Is A Sentence'
"it's three words" @su.to_title --> "It's Three Words"
```

---

### `@to_upper`

**Synopsis**:

`[string: Str] @to_upper -> Str`

**Return value**:

A new string is returned with all the letters uppercase.

**Example**:

```nest
|#| 'stdsutil.nest' = su
'Hello' @su.to_upper --> 'HELLO'
```

---

### `@trim`

**Synopsis**:

`[string: Str] @trim -> Str`

**Return value**:

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

The letter characters both uppercase (`A-Z`) and lowercase (`a-z`).

---

### `LOWERCASE_LETTERS`

The lowercase letter characters (`a-z`).

---

### `OCT_DIGITS`

Octal digit characters (`0-7`).

---

### `PRINTABLE`

All the printable characters (`\x20-\x7e`).

---

### `PUNCTUATION`

All punctuation characters.

---

### `UPPERCASE_LETTERS`

The uppercase letter characters (`A-A`).
