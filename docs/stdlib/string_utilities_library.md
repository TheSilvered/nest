# String utilities library

## Importing

```nest
|#| 'stdsutil.nest' = su
```

---

## `fmt` format rules

### General format syntax

`{[Flags][Width][.Precision][,SeparatorWidth][Alignment]}`

Each of the things inside the square brackets is optional.

To write a curly brace `{`, write `{{`, and to write `}`, write `}}` instead.

### Format types

The `fmt` function has a special behaviour for `Int`s, `Real`s, `Byte`s and
`Bool`s; any other type is first converted to a string and then treated as one
when applying the flags.

### Flags

The available flags are:

- `g`, `G`: general representation for `Real`s
- `f`, `F`: decimal representation for `Real`s
- `e`, `E`: standard (or scientific) notation for `Real`s
- `b`: binary integer representation
- `o`: octal integer representation
- `x`: hexadecimal integer representation
- `X`: uppercase hexadecimal integer representation
- `u`: treat the integer as unsigned
- `0`: fill with zeroes
- ` ` (space): add a space in front of positive numbers
- `+`: add a plus sign in front of positive numbers
- `r`: Nest literal string representation
- `R`: escape special characters
- `a`: ASCII Nest literal string representation
- `A`: escape special and non-ASCII characters
- `p`: lowercase affixes/general lowercase
- `P`: uppercase affixes/general uppercase
- `'`: thousand separator
- `_`: padding character
- `c`: cut to width

!!!note
    If a flag is not supported by the type it is used with it is ignored. If
    incompatible flags are used in the same format, such as `{bo}`, only the
    latter will be used.

#### The `g` and `G` flags

These flags are only supported by `Real`s. They set the general
representation mode to be used though mode `g` is the default.

In general representation the number is represented with `precision`
significant digits and will alternate between the `f` or `F` mode and the `e`
or `E` modes depending on its magnetude.

The `f` or `F` mode is used when `-4 <= exp < precision` where `exp` is the
exponent in standard notation of the number. When the exponent falls outside
of this range the `e` or `E` representation is used.

The difference between `g` and `G` is that the latter will always contain a
decimal point where the former may strip it away.

For example:

```nest
'{g.3}' {100.0} @su.fmt --> '100'
'{G.3}' {100.0} @su.fmt --> '100.0'
```

!!!note
    The `g` in the first line can be omitted since it is the default mode.

When using the `G` mode significant digits are automatically increased to
properly display at least one digit after the decimal point, for example:

```nest
'{G.3}' {123.4} @su.fmt --> '123.4' even with a precision of 3
```

#### The `f` and `F` flags

These flags are only supported by `Real`s. They set the decimal representation
mode to be used.

In this representation the number is printed with `precision` digits after the
decimal point though trailing zeroes are removed by default.

For example:

```nest
'{f}' {13.5871558} @su.fmt --> '13.587156'
'{f}' {13.5} @su.fmt --> '13.5'
'{f}' {10.0} @su.fmt --> '10'
'{f}' {0.0248} @su.fmt --> '0.0248'
```

Similarly to `G` and `E`, the `F` flag will always include a decimal point,
for example:

```nest
'{F}' {10.0} @su.fmt --> '10.0'
```

#### The `e` and `E` flags

These flags are only supported by `Real`s. They set the standard notation mode
to be used.

In this representation the number is printed with one non-zero digit before
the dot (unless the number itself is zero) followed by at most `precision`
digits after the dot, any trailing zeroes are removed by default.

For example:

```nest
'{e}' {13.5871558} @su.fmt --> '1.358716e+01'
'{e}' {13.5} @su.fmt --> '1.35e+01'
'{e}' {10.0} @su.fmt --> '1e+01'
'{e}' {0.0248} @su.fmt --> '2.48e-02'
```

Similarly to `G` and `F`, the `E` flag will always include a decimal point,
for example:

```nest
'{e}' {10.0} @su.fmt --> '1.0e+01'
```

#### The `b`, `o`, `x` and `X` flags

These flags are supported by `Int`s and `Byte`s and determine the base used to
represent them.

The `b` flag will use binary, the `o` flag will use octal, the `x` flag will
use hexadecimal with lower-case `a-f` digits and `X` will used hexadecimal with
upper-case `A-F` digits. For example:

```nest
'{b}' {28} @su.fmt --> '11100'
'{o}' {28} @su.fmt --> '34'
'{}'  {28} @su.fmt --> '28'
'{x}' {28} @su.fmt --> '1c'
'{X}' {28} @su.fmt --> '1C'
```

By default no prefixes are added to the numbers.

#### The `u` flag

This flag is supported by `Int`s and indicates to treat the number as a 64-bit
unsigned integer instead of a signed one.

#### The `0` flag

This flag will have a different behaviour depending on the type.

For `Int`s it will add zeroes between the number and the sign until the number
of digits matches the precision. When using this flag the thousand separator is
put between the zeroes. For example:

```nest
'{0.4}'   {16} @su.fmt --> '0016'
"{0',.4}" {16} @su.fmt --> '0,016'
```

For `Real`s this flag will stop any trailing zeroes from being removed, for
example:

```nest
'{g0}' {10.0} @su.fmt --> '10.0000'
'{f0}' {10.0} @su.fmt --> '10.000000'
'{e0}' {10.0} @su.fmt --> '1.000000e+01'
```

#### The ` ` (space) and `+` flags

These flags are supported by `Int`s and `Real`s and add either a space or a
plus sign before positive numbers. For example:

```nest
'{}'  {10} @su.fmt --> '10'
'{+}' {10} @su.fmt --> '+10'
'{ }' {10} @su.fmt --> ' 10'
```

If the number is unsigned it is treated as positive and thus a space or a plus
is added in front.

```nest
'{u+}' {10} @su.fmt --> '+10'
```

#### The `r`, `R`, `a` and `A` flags

These flags are supported by `Str`s and specify a representation mode for the
string.

The `r` flag will make the string into a valid Nest literal, adding either
single or double quotes around it and escaping non-printable or special
characters.

```nest
'{r}' {'hello😊\n'} @su.fmt --> "'hello😊\\n'"
```

The `R` flag will instad only escape special characters (including the
backslash `\` but not quotes) and leaving everything else untouched.

```nest
'{R}' {'hello😊\n'} @su.fmt --> 'hello😊\\n'
```

The `a` flag is similar to the `r` flag but the string will be translated into
printable ASCII using unicode escapes for non-ASCII characters.

```nest
'{R}' {'hello😊\n'} @su.fmt --> "'hello\\U01f60a\\n'"
```

The `A` flag is similar to the `R` flag but it will also escape any non-ASCII
character.

```nest
'{R}' {'hello😊\n'} @su.fmt --> 'hello\\U01f60a\\n'
```

#### The `p` flag

This flag is supported by `Int`s, `Byte`s and `Real`s.

For `Byte`s it will add the `0b` prefix and the `b` suffix in binary mode, the
`0o` prefix and the `b` suffix in octal mode, the `b` suffix in decimal mode
and the `0h` prefix in hexadecimal mode (both upper and lower-case). For
example:

```nest
'{bp}' {28b} @su.fmt --> '0b11100b'
'{op}' {28b} @su.fmt --> '0o34b'
'{p}'  {28b} @su.fmt --> '28b'
'{xp}' {28b} @su.fmt --> '0h1c'
'{Xp}' {28b} @su.fmt --> '0h1C'
```

For `Int`s it will add the `0b` prefix in binary mode, the `0o` prefix in octal
mode and the `0x` prefix in hexadecimal mode (both upper and lower-case). For
example:

```nest
'{bp}' {28} @su.fmt --> '0b11100'
'{op}' {28} @su.fmt --> '0o34'
'{p}'  {28} @su.fmt --> '28'
'{xp}' {28} @su.fmt --> '0x1c'
'{Xp}' {28} @su.fmt --> '0x1C'
```

For `Real`s type it will format `NaN` and `Inf` as `nan` and `inf`.

```nest
'{}'  {math.INF} @su.fmt --> 'Inf'
'{p}' {math.INF} @su.fmt --> 'inf'
'{}'  {math.NAN} @su.fmt --> 'NaN'
'{p}' {math.NAN} @su.fmt --> 'nan'
```

#### The `P` flag

This flag is supported by `Byte`s, `Int`s, `Real`s and `Bool`s.

For `Byte`s it will add the `0B` prefix and the `B` suffix in binary mode, the
`0O` prefix and the `B` suffix in octal mode, the `B` suffix in decimal mode
and the `0H` prefix in hexadecimal mode (both upper and lower-case). For
example:

```nest
'{bP}' {28b} @su.fmt --> '0B11100b'
'{oP}' {28b} @su.fmt --> '0O34b'
'{P}'  {28b} @su.fmt --> '28B'
'{xP}' {28b} @su.fmt --> '0H1c'
'{XP}' {28b} @su.fmt --> '0H1C'
```

For `Int`s it will add the `0B` prefix in binary mode, the `0O` prefix in octal
mode and the `0X` prefix in hexadecimal mode (both upper and lower-case). For
example:

```nest
'{bp}' {28} @su.fmt --> '0B11100'
'{op}' {28} @su.fmt --> '0O34'
'{p}'  {28} @su.fmt --> '28'
'{xp}' {28} @su.fmt --> '0X1c'
'{Xp}' {28} @su.fmt --> '0X1C'
```

For `Real`s it will format `NaN` and `Inf` as `NAN` and `INF` and the `e` in
standard notation will be an upper-case `E`.

```nest
'{}'  {math.INF} @su.fmt --> 'Inf'
'{P}' {math.INF} @su.fmt --> 'INF'
'{}'  {math.NAN} @su.fmt --> 'NaN'
'{P}' {math.NAN} @su.fmt --> 'NAN'
'{eP}' {10.0} @su.fmt --> '1E+01'
```

For `Bool`s it will format `true` and `false` as `TRUE` and `FALSE`.

```nest
'{}'  true @su.fmt --> 'true'
'{P}' true @su.fmt --> 'TRUE'
```

#### The `'` flag

This flag is supported by `Byte`s, `Int`s and `Float`s, it specifies the
character to be used as the thousand separator.

It is not a standalone flag as it needs to be followed by another character
that will be the actual character used.

For example:

```nest
'{}'   {1000000} @su.fmt --> '1000000'
"{''}" {1000000} @su.fmt --> "1'000'000"
"{',}" {1000000} @su.fmt --> '1,000,000'
"{' }" {1000000} @su.fmt --> '1 000 000'
```

#### The `_` flag

This flag is supported by all types and specifies the character used to fill
the extra space to reach `width` characters on the formatted value. If the
`width` is not specified this flag has no meaning.

Just like the `'` flag it is not a standalone flag as it needs to be followed
by another character that will be the actual character used.

By default the padding character is a space.

For example:

```nest
'{}'    {'hi'} @su.fmt --> 'hi'
'{5}'   {'hi'} @su.fmt --> 'hi   '
'{_.5}' {'hi'} @su.fmt --> 'hi...'
'{__5}' {'hi'} @su.fmt --> 'hi___'
```

#### The `c` flag

This flag is supported by all types and will make the formatted value exactly
`width` characters long. If the string is shorter than `width` it will be
padded normally but if it is longer it will be trimmed.

```nest
'{4}'  {'hi'} @su.fmt --> 'hi  '
'{4}'  {'hello'} @su.fmt --> 'hello'
'{c4}' {'hi'} @su.fmt --> 'hi  '
'{c4}' {'hello'} @su.fmt --> 'hell'
```

This flag will cut the value according to the alignment: values aligned to the
left will be cut from the right, values aligned to the right will be cut to the
left and values aligned in the middle will be cut from both sides.

### Width

This field specifies the minimum width of the formatted value, if the string is
shorter than the specified width it is padded by default with spaces. The `_`
flag is used to specify what character to use instead of the space.

When the `c` flag is used this field specifies the exact width of the resulting
string, shorter strings will still be padded but strings that are too long will
be cut to size reguardless of the value, this means that digits and signs can
be cut off numbers.

The width can be specified directly with a number after the flags or by writing
an asterisk and passing an `int` value in the arguments after the value to be
formatted. For example:

```nest
'{5}' {123}    @su.fmt --> '  123'
'{*}' {123, 4} @su.fmt --> ' 123'
```

A negative width is ignored and a width of zero is useless without the `c`
flag.

### Precision

This field has different interpretations depending on the type being formatted:

For `Real`s in decimal and standard notation (with the `f` and `e` flags) it
specifies the number of digits after the dot. By default the precision is `6`.

```nest
'{f.3}' {1.234567} @su.fmt --> '1.235'
'{e.3}' {1.234567} @su.fmt --> '1.235e+00'
'{f}' {1.234567} @su.fmt --> '1.234567'
'{e}' {1.234567} @su.fmt --> '1.234567e+00'
```

For `Real`s in general notation the precision specifies the number of
significant digits to show. By default the precision is `6`.

```nest
'{.3}' {1.234567} @su.fmt --> '1.23'
'{}' {1.234567} @su.fmt --> '1.23457'
```

For `Byte`s and `Int`s it specifies the minimum number of characters before the
sign, by default the extra characters are spaces but using the `0` flag they
become zeroes.

```nest
'{}'    {123} @su.fmt --> '123'
'{.5}'  {123} @su.fmt --> '  123'
'{0.5}' {123} @su.fmt --> '00123'
```

When using a thousand separator it is displayed when using the `0` flag and is
otherwise accounted for with extra spaces in order to reach the same number of
characters in the final output.

```nest
"{0',.5}" {123} @su.fmt --> '00,123'
"{',.5}"  {123} @su.fmt --> '   123'
'{.5}'    {123} @su.fmt --> '  123'
```

For `Str`s it specifies the maximum amount of characters to write, if a string
is longer than specified it is shortened to size removing characters from the
right and adding an ellipsis (`...`) at the end.

```nest
'{.10}' {'short'} @su.fmt --> 'short'
'{.10}' {'somewhat long'} @su.fmt --> 'somewhat l...'
```

The precision can be specified directly with a number after the dot (`.`) or by
writing an asterisk and passing an `int` value in the arguments after the value
to be formatted and, if specified, the width. For example:

```nest
'{.5}' {123} @su.fmt --> '  123'
'{.*}' {123, 4} @su.fmt --> ' 123'
'{0*.*}' {123, 6, 4} @su.fmt --> '  0123'
```

A negative precision is ignored.

### Separator width

This field applies to the types that support the `'` flag and changes the
amount of digits between separators from the default.

The default values for the separator width is as followes:

- `8` for numbers in binary
- `3` for numbers in octal or decimal (including `Real`s)
- `4` for numbers in hexadecimal

Like the precision, the separator width can be specified directly with a number
after the comma (`,`) or by writing an asterisk and passing an `int` value in
the arguments after the value to be formatted and, if specified, the width and
the precision. For example:

```nest
"{''}" {12345} @su.fmt --> "12'345"
"{'',4}" {12345} @su.fmt --> "1'2345"
"{''.*}" {12345, 2} @su.fmt --> "1'23'45"
"{''0*.*,*}" {123, 6, 4, 2} @su.fmt --> " 01'23"
```

A separator width smaller than one ignored.

### Alignment

This field specifies the alignment of the formatted string. It is meaningful
only when used along side the `width` because otherwise the formatted values
will only ever be as bit as necessary.

The alignment can be one of three values:

- `<`: left align (default behaviour for most types)
- `>`: right align (default behaviour for `Byte`s, `Int`s and `Real`s)
- `^`: center align

A left alignment will cause the value to be placed on the left and the padding
on the right and will cause the string to be cut from the right.

A right alignment will cause the value to be placed on the right and the
padding on the left and will cause the string to be cut from the left.

A center alignment will cause the value to be placed in the middle with the
padding on both sides and will cause the string to be cut from both the left
and the right.

---

## Functions

### `@bin`

**Synopsis:**

```nest
[n: Int] @bin -> Str
```

**Returns:**

A string containing the binary representation of `n` without any prefix. `n` is
treated like an unsigned integer.

---

### `@cjust`

**Synopsis:**

```nest
[string: Str, width: Int, char: Str?] @cjust -> Str
```

**Description:**

Creates a new string of length `width`, copies `string` centering it and
filling the remaining space with `char`. If the string cannot be perfectly
centered it will be closer to the left side. `char` must have a length of `1`.

If `length` is smaller than or equal to the length of `string`, the string
itself is returned.

If `char` is `null` a space will be used instead.

**Arguments:**

- `string`: the string to center
- `width`: the minimum length in characters of the final string
- `char`: the character used to fill the extra space

**Returns:**

The centered string.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'hello' 7 @su.cjust --> ' hello '
'hello' 8 '.' @su.cjust --> '.hello..'
```

---

### `@decode`

**Synopsis:**

```nest
[bytes_array: Array|Vector.Byte, encoding: Str?] @decode -> Str
```

**Description:**

Decodes a string from an array of `Byte` objects with a specific encoding. If
an encoding is not provided the extUTF-8 encoding is used. To see the full list
of available encodings go [here](codecs_library.md#nest-encodings).

**Arguments:**

- `bytes_array`: the array of bytes to decode
- `encoding`: the encoding used to decode the `bytes_array`

**Returns:**

The decoded string.

!!!warning
    If the encoding of the bytes does not match the given encoding you could
    end up with an error or worse with
    [mojibake](https://en.wikipedia.org/wiki/Mojibake).

---

### `@encode`

**Synopsis:**

```nest
[string: Str, encoding: Str?] @encode -> Array.Byte
```

**Description:**

Encodes a string into an array of `Byte` objects using a specific encoding. If
an encoding is not provided the UTF-8 encoding is used. To see the full list of
available encodings go [here](codecs_library.md#nest-encodings).

**Arguments:**

- `string`: the string to encode
- `encoding`: the encoding used to encode the `string`

**Returns:**

The array containing the encoded bytes.

!!!warning
    This function by default will not be able to encode all valid Nest strings
    as they are encoded with extUTF-8 which is the same as UTF-8 but more
    characters are allowed.

---

### `@ends_with`

**Synopsis:**

```nest
[string: Str, substring: Str] @ends_with -> Bool
```

**Returns:**

`true` if `string` ends with `substring` and `false` otherwise.

---

### `@fmt`

**Synopsis:**

```nest
[format_placeholder: Str, values: Array|Vector] @fmt -> Str
```

**Description:**

Creates a new string by inserting the values from the `values` array into the
`format_placeholder`, all the rules for how the values are formatted are
specified [above](#fmt-format-rules).

**Arguments:**

- `format_placeholder`: the placeholder that specifies where the values are to
                        be inserted and how they should be formatted
- `values`: the array of values to insert in the string

**Returns:**

The newly formatted string.

---

### `@hex`

**Synopsis:**

```nest
[n: Int, upper: Bool?] @hex -> Str
```

**Returns:**

A string containing the hexadecimal representation of `n` without any
prefix. `n` is treated like an unsigned integer. If `upper` is true the
function will used letters `A-F` instead of `a-f`. If `upper` is not given it
is considered `false`.

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

### `@is_space`

**Synopsis:**

```nest
[string: Str] @is_space -> Bool
```

**Returns:**

`true` if all the characters in `string` are either a space, a newline (`\n`),
a carriage return (`\r`), a form feed (`\f`), a horizontal tab (`\t`) or a
vertical tab (`\v`).

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

### `@ljust`

**Synopsis:**

```nest
[string: Str, width: Int, char: Str?] @ljust -> Str
```

**Description:**

Creates a new string of length `width`, copies `string` justifying it to the
left and filling the remaining space with `char`. `char` must have a length
of `1`.

If `length` is smaller than or equal to the length of `string`, the string
itself is returned.

If `char` is `null` a space will be used instead.

**Arguments:**

- `string`: the string to justify
- `width`: the minimum length in characters of the final string
- `char`: the character used to fill the extra space

**Returns:**

The justified string.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'hello' 7 @su.ljust --> 'hello  '
'hello' 8 '.' @su.ljust --> 'hello...'
```

---

### `@lremove`

**Synopsis:**

```nest
[string: Str, substring: Str] @lremove -> Str
```

**Returns:**

A string with the prefix `substring` removed if `string` starts with it. The
string itself is returned if `string` does not begin with `substring`.

---

### `@lsplit`

**Synopsis:**

```nest
[string: Str, separator: Str?, max_cuts: Int?] @lsplit -> Vector.Str
```

**Description:**

Splits `string` where it finds `separator` without including it and returns a
vector with all the resulting strings. It stops when it has reached the
specified number of `max_cuts` or when there are no more separators in the
string.

If `separator` is `null`, the string is split on whitespace and empty strings
are ignored, meaning that contiguous whitespace is treated as one character.
When `separator` is `null` and the string has reached the number of cuts
specified by `max_cuts` any subsequent whitespace is left untouched.

If `max_cuts` is negative the function will split `string` at all the
occurrences of `separator`. If `max_cuts` is not given, it defaults to `-1` and
if `max_cuts` is `0` a vector is returned containing `string` unmodified.

This function splits the string from left to right.

**Arguments:**

- `string`: the string to split
- `separator`: the substring to split `string` at
- `max_cuts`: the maximum number of splits allowed

**Returns:**

A new vector containing the split strings.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'a  b' @su.lsplit --> <{'a', 'b'}>
'a  b  ' @su.lsplit --> <{'a', 'b'}>
'  a  b' @su.lsplit --> <{'a', 'b'}>
'a  b  ' null 1 @su.lsplit --> <{'a', 'b  '}>
'a  b' ' ' @su.lsplit --> <{'a', '', 'b'}>
'a.b.c.d' '.' @su.lsplit --> <{'a', 'b', 'c', 'd'}>
'a.b.c.d' '.' 2 @su.lsplit --> <{'a', 'b', 'c.d'}>
'a.b.c.d' '.' 1 @su.lsplit --> <{'a', 'b.c.d'}>
```

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

A string containing the octal representation of `n` without any prefix. `n` is
treated like an unsigned integer.

---

### `@parse_int`

**Synopsis:**

```nest
[string: Str, base: Int?] @parse_int -> Int
```

**Description:**

This function parses an integer from `string`. `base` specifies the base with
which to parse the number and can be any value between `2` and `36` inclusive
or `0`.

When using a base higher than `10` letters are used and the function is
case-insensitive.

If `base` is set to `0` it is inferred by the number: with the prefix `0b` or
`0B` the digits after are parsed in base `2`; with the prefix `0o` or `0O` the
base is `8` and with the prefix `0x` or `0X` the base is `16`. When explicitly
setting `base` to `2`, `8` or `16` the corresponding prefixes are ignored if
present.

By default
`base` is set to zero.

Additionally this function ignores any whitespace before and after the number
and ignores any underscores (`_`) between the digits.

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

### `@rjust`

**Synopsis:**

```nest
[string: Str, width: Int, char: Str?] @rjust -> Str
```

**Description:**

Creates a new string of length `width`, copies `string` justifying it to the
right and filling the remaining space with `char`. `char` must have a length
of `1`.

If `length` is smaller than or equal to the length of `string`, the string
itself is returned.

If `char` is `null` a space will be used instead.

**Arguments:**

- `string`: the string to justify
- `width`: the minimum length in characters of the final string
- `char`: the character used to fill the extra space

**Returns:**

The justified string.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'hello' 7 @su.rjust --> '  hello'
'hello' 8 '.' @su.rjust --> '...hello'
```

---

### `@rremove`

**Synopsis:**

```nest
[string: Str, substring: Str] @rremove -> Str
```

**Returns:**

A string with the suffix `substring` removed if `string` ends with it. The
string itself is returned if `string` does not end with `substring`.

---

### `@rsplit`

**Synopsis:**

```nest
[string: Str, separator: Str?, max_cuts: Int?] @rsplit -> Vector.Str
```

**Description:**

Splits `string` where it finds `separator` without including it and returns a
vector with all the resulting strings. It stops when it has reached the
specified number of `max_cuts` or when there are no more separators in the
string.

If `separator` is `null`, the string is split on whitespace and empty strings
are ignored, meaning that contiguous whitespace is treated as one character.
When `separator` is `null` and the string has reached the number of cuts
specified by `max_cuts` any subsequent whitespace is left untouched.

If `max_cuts` is negative the function will split `string` at all the
occurrences of `separator`. If `max_cuts` is not given, it defaults to `-1` and
if `max_cuts` is `0` a vector is returned containing `string` unmodified.

This function splits the string from right to left.

**Arguments:**

- `string`: the string to split
- `separator`: the substring to split `string` at
- `max_cuts`: the maximum number of splits allowed

**Returns:**

A new vector containing the split strings.

**Example:**

```nest
|#| 'stdsutil.nest' = su
'a  b' @su.lsplit --> <{'a', 'b'}>
'a  b  ' @su.lsplit --> <{'a', 'b'}>
'  a  b' @su.lsplit --> <{'a', 'b'}>
'  a  b' null 1 @su.lsplit --> <{'  a', 'b'}>
'a  b' ' ' @su.lsplit --> <{'a', '', 'b'}>
'a.b.c.d' '.' @su.lsplit --> <{'a', 'b', 'c', 'd'}>
'a.b.c.d' '.' 2 @su.lsplit --> <{'a.b', 'c', 'd'}>
'a.b.c.d' '.' 1 @su.lsplit --> <{'a.b.c', 'd'}>
```

---

### `@rtrim`

**Synopsis:**

```nest
[string: Str] @rtrim -> Str
```

**Returns:**

Creates a new string with trailing whitespace removed.

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
