# `format.h`

Various function to print formatted strings to file objects.

## Authors

TheSilvered

## `Nst_*printf` format rules

!!!note
    If a sequence is invalid the function fails and nothing is printed.

### Format placeholder

```text
%[flags][width][.precision][length]type
```

To print a literal `%` use `%%`.

### Flags

- `-`: left-align the output
- `+`: always print the sign on signed numeric types (by default `+` is not
  printed but `-` is)
- `[space]`: prepend a space on positive signed numbers
- `0`: when a width is specified any extra space is padded with `0` instead
  of `[space]` for numeric types
- `'`: use an apostrophe as the thousand separator
- `#`: alternate form
  - does not remove trailing zeroes from `g` and `G`
  - always puts a decimal point in `f`, `F`, `e`, `E`, `g` and `G`
  - adds the prefix `0x` to `x` and `0X` to `X`
  - adds the prefix `0` to `o` if the number is not `0`

!!!note
    Using together `+` and `[space]` or using together `-` and `0` results in an
    invalid sequence

### Width

A number that specifies the minimum width of a format, does not truncate the
output and any extra characters. If an asterisk (`*`) is used, the width is
obtained from an argument of type `int` that precedes the value to format.

```better-c
Nst_printf("%0*i", 3, 5); // Result: "005"
Nst_printf("%3i", 1234); // Result: "1234"
```

### Precision

This parameter has different meanings depending on type to format:

- for `d`, `i`, `u`, `x`, `X`, `o` it specifies the minimum number of digits
  that the number must contain
- for `e`, `E`, `f` and `F` it specifies the number of digits after the decimal
  point, if it is not set it defaults to `6`
- for `g` and `G` it specifies the number of significant digits to use, if not
  set it defaults to `6`

```better-c
Nst_printf("%#-10.5X", 15); // Result: "0X000FF   "
Nst_printf("%.2lf", 1.37); // Result: "1.3"
```

### Length

This parameter specifies the size of the argument passed:

- `hh`: an integer that was passed as a char
- `h`: an integer that was passed as a short
- `l`: a long int or a double
- `ll`: a long long int
- `z`: a size_t-sized argument
- `j`: a maxint_t-sized argument
- `t`: a ptrdiff_t-sized argument

!!!note
    `L`, `I`, `I32`, `I64` and `q` are not supported.

### Type

- `d`, `i`: prints an `int`
- `u`: prints an `unsigned int`
- `f`, `F`: prints a `double`
- `e`, `E`: prints a `double` in standard form
- `g`, `G`: prints a `double` removing the trailing zeroes and using the
  standard form when using more than the specified significant digits
- `x`, `X`: prints an unsigned int in hexadecimal, `x` uses lowercase letters
  and `X` uses uppercase
- `o`: prints an `unsigned int` in octal
- `s`: prints a null-terminated `char *`
- `c`: prints a `char`
- `p`: prints a `void *`

!!!note
  `a`, `A` and `n` are not supported.

---

## `Nst_fmt` format rules

### General format syntax

`{Type[:[Flags][Width][.Precision][,SeparatorWidth][Alignment]]}`

The things inside the square brackets are optional.

To write a curly brace `{`, write `{{`, and to write `}`, write `}}` instead.

### Type

The type determines how the argument passed is read. The following types are
allowed:

- `s` for `char *` (string)
- `B` for `u8` (takes a `uint` but expects values in the range `[0, 255]`)
- `i` for `int` or `uint` (`unsigned int`)
- `l` for `i32` (`long`) or `u32` (`unsigned long`)
- `L` for `i64` (`long long`) or `u64` (`unsigned long long`)
- `z` for `isize` (`size_t`) or `usize` (`ptrdiff_t`)
- `b` for `bool` (takes an int but writes `true` or `false` instead of `1` and
  `0`)
- `f` for `f32` (`float`) or `f64` (`double`)
- `c` for `char`
- `p` for `void *`

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
    incompatible flags are used in the same format, such as `{i:bo}`, only the
    latter will be used.

#### The `g` and `G` flags

These flags are only supported by the `f` type. They set the general
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

```better-c
Nst_fmt("{f:g.3}", 0, NULL, 100.0); // results in "100"
Nst_fmt("{f:G.3}", 0, NULL, 100.0); // results in "100.0"
```

!!!note
    The `g` in the first line can be omitted since it is the default mode.

When using the `G` mode significant digits are automatically increased to
properly display the digit after the decimal point, for example:

```better-c
Nst_fmt("{f:G.3}", 0, NULL, 123.4); // results in "123.4" even with
                                    // a precision of 3
```

#### The `f` and `F` flags

These flags are only supported by the `f` type. They set the decimal
representation mode to be used.

In this representation the number is printed with `precision` digits after the
decimal point though trailing zeroes are removed.

For example:

```better-c
Nst_fmt("{f:f}", 0, NULL, 13.5871558); // results in "13.587156"
Nst_fmt("{f:f}", 0, NULL, 13.5); // results in "13.5"
Nst_fmt("{f:f}", 0, NULL, 10.0); // results in "10"
Nst_fmt("{f:f}", 0, NULL, 0.0248); // results in "0.0248"
```

Similarly to `G` and `E`, the `F` flag will always include a decimal point,
for example:

```better-c
Nst_fmt("{f:F}", 0, NULL, 10.0); // results in "10.0"
```

#### The `e` and `E` flags

These flags are only supported by the `f` type. They set the standard notation
mode to be used.

In this representation the number is printed with one non-zero digit before
the dot (unless the number itself is zero) followed by at most `precision`
digits after the dot, any trailing zeroes are removed by default.

For example:

```better-c
Nst_fmt("{f:e}", 0, NULL, 13.5871558); // results in "1.358716e+01"
Nst_fmt("{f:e}", 0, NULL, 13.5); // results in "1.35e+01"
Nst_fmt("{f:e}", 0, NULL, 10.0); // results in "1e+01"
Nst_fmt("{f:e}", 0, NULL, 0.0248); // results in "2.48e-02"
```

Similarly to `G` and `F`, the `E` flag will always include a decimal point,
for example:

```better-c
Nst_fmt("{f:e}", 0, NULL, 10.0); // results in "1.0e+01"
```

#### The `b`, `o`, `x` and `X` flags

These flags are supported by `B`, `i`, `l`, `L` and `z` and determine the base
used to represent them. Additionally the `X` flag is supported by the `p` type.

The `b` flag will use binary, the `o` flag will use octal, the `x` flag will
use hexadecimal with lower-case `a-f` digits and `X` will used hexadecimal with
upper-case `A-F` digits. For example:

```better-c
Nst_fmt("{i:b}", 0, NULL, 28); // results in "11100"
Nst_fmt("{i:o}", 0, NULL, 28); // results in "34"
Nst_fmt("{i}",   0, NULL, 28); // results in "28"
Nst_fmt("{i:x}", 0, NULL, 28); // results in "1c"
Nst_fmt("{i:X}", 0, NULL, 28); // results in "1C"
```

By default no prefixes are added to the numbers.

On the `p` type by default the `0x` prefix is added, using the `X` flag you
can add the `0X` prefix instead.

```better-c
Nst_fmt("{p}", 0, NULL, (void *)0x325c4e4); // results in "0x325c4e4"
Nst_fmt("{p:X}", 0, NULL, (void *)0x325c4e4); // results in "0X325c4e4"
```

#### The `u` flag

This flag is supported by `i`, `l`, `L` and `z` and indicates to use the
unsigned variant of the types. For `i` this flag will read a `uint` instead of
an `int`, for `l` it will read a `u32` instad of a `i32` and so on.

#### The `0` flag

This flag will have a different behaviour depending on the type.

For `B`, `i`, `l`, `L` and `z` it will add zeroes between the number and the
sign until the number of digits matches the precision. When using this flag the
thousand separator is put between the zeroes. For example:

```better-c
Nst_fmt("{i:0.4}", 0, NULL, 16); // results in "0016"
Nst_fmt("{i:0',.4}", 0, NULL, 16); // results in "0,016"
```

For the `p` type this flag will add zeroes before the `0x` or `0X` prefix to
reach `16` characters on 64-bit machines and `8` characters on 32-bit ones. For
example:

```better-c
// on x64
Nst_fmt("{p:0}", 0, NULL, (void *)0x325c4e4); // results in "0x000000000325c4e4"
// on x86
Nst_fmt("{p:0}", 0, NULL, (void *)0x325c4e4); // results in "0x0325c4e4"
```

For the `f` type this flag will stop any trailing zeroes from being removed.
for example:

```better-c
Nst_fmt("{f:g0}", 0, NULL, 10.0); // results in "10.0000"
Nst_fmt("{f:f0}", 0, NULL, 10.0); // results in "10.000000"
Nst_fmt("{f:e0}", 0, NULL, 10.0); // results in "1.000000e+01"
```

#### The ` ` (space) and `+` flags

These flags are supported by `B`, `i`, `l`, `L`, `z` and `f` and add either a
space or a plus sign before positive numbers. For example:

```better-c
Nst_fmt("{i}",   0, NULL, 10); // results in "10"
Nst_fmt("{i:+}", 0, NULL, 10); // results in "+10"
Nst_fmt("{i: }", 0, NULL, 10); // results in " 10"
```

If the number is unsigned it is treated as positive and thus a space or a plus
is added in front.

```better-c
Nst_fmt("{i:u+}", 0, NULL, 10); // results in "+10"
```

#### The `r`, `R`, `a` and `A` flags

These flags are supported by the `s` type and specify a representation mode
for the string.

The `r` flag will make the string into a valid Nest literal, adding either
single or double quotes around it and escaping non-printable or special
characters.

```better-c
Nst_fmt("{s:r}", 0, NULL, "hello😊\n"); // results in "'hello😊\\n'"
```

The `R` flag will instad only escape special characters (including the
backslash `\` but not quotes) and leaving everything else untouched.

```better-c
Nst_fmt("{s:R}", 0, NULL, "hello😊\n"); // results in "hello😊\\n"
```

The `a` flag is similar to the `r` flag but the string will be translated into
printable ASCII using unicode escapes for non-ASCII characters.

```better-c
Nst_fmt("{s:R}", 0, NULL, "hello😊\n"); // results in "'hello\\U01f60a\\n'"
```

The `A` flag is similar to the `R` flag but it will also escape any non-ASCII
character.

```better-c
Nst_fmt("{s:R}", 0, NULL, "hello😊\n"); // results in "hello\\U01f60a\\n"
```

#### The `p` flag

This flag is supported by `B`, `i`, `l`, `L`, `z` and `f`.

For the `B` type it will add the `0b` prefix and the `b` suffix in binary mode,
the `0o` prefix and the `b` suffix in octal mode, the `b` suffix in decimal
mode and the `0h` prefix in hexadecimal mode (both upper and lower-case). For
example:

```better-c
Nst_fmt("{B:bp}", 0, NULL, 28); // results in "0b11100b"
Nst_fmt("{B:op}", 0, NULL, 28); // results in "0o34b"
Nst_fmt("{B:p}",  0, NULL, 28); // results in "28b"
Nst_fmt("{B:xp}", 0, NULL, 28); // results in "0h1c"
Nst_fmt("{B:Xp}", 0, NULL, 28); // results in "0h1C"
```

For `i`, `l`, `L` and `z` it will add the `0b` prefix in binary mode, the `0o`
prefix in octal mode and the `0x` prefix in hexadecimal mode (both upper and
lower-case). For example:

```better-c
Nst_fmt("{i:bp}", 0, NULL, 28); // results in "0b11100"
Nst_fmt("{i:op}", 0, NULL, 28); // results in "0o34"
Nst_fmt("{i:p}",  0, NULL, 28); // results in "28"
Nst_fmt("{i:xp}", 0, NULL, 28); // results in "0x1c"
Nst_fmt("{i:Xp}", 0, NULL, 28); // results in "0x1C"
```

For the `f` type it will format `NaN` and `Inf` as `nan` and `inf`.

```better-c
Nst_fmt("{f}"  , 0, NULL, INFINITY); // results in "Inf"
Nst_fmt("{f:p}", 0, NULL, INFINITY); // results in "inf"
Nst_fmt("{f}",   0, NULL, NAN); // results in "NaN"
Nst_fmt("{f:p}", 0, NULL, NAN); // results in "nan"
```

#### The `P` flag

This flag is supported by `B`, `i`, `l`, `L`, `z`, `f`, `b` and `p`

For the `B` type it will add the `0B` prefix and the `B` suffix in binary mode,
the `0O` prefix and the `B` suffix in octal mode, the `B` suffix in decimal
mode and the `0H` prefix in hexadecimal mode (both upper and lower-case). For
example:

```better-c
Nst_fmt("{B:bP}", 0, NULL, 28); // results in "0B11100b"
Nst_fmt("{B:oP}", 0, NULL, 28); // results in "0O34b"
Nst_fmt("{B:P}",  0, NULL, 28); // results in "28B"
Nst_fmt("{B:xP}", 0, NULL, 28); // results in "0H1c"
Nst_fmt("{B:XP}", 0, NULL, 28); // results in "0H1C"
```

For `i`, `l`, `L` and `z` it will add the `0B` prefix in binary mode, the `0O`
prefix in octal mode and the `0X` prefix in hexadecimal mode (both upper and
lower-case). For example:

```better-c
Nst_fmt("{i:bp}", 0, NULL, 28); // results in "0B11100"
Nst_fmt("{i:op}", 0, NULL, 28); // results in "0O34"
Nst_fmt("{i:p}",  0, NULL, 28); // results in "28"
Nst_fmt("{i:xp}", 0, NULL, 28); // results in "0X1c"
Nst_fmt("{i:Xp}", 0, NULL, 28); // results in "0X1C"
```

For the `f` type it will format `NaN` and `Inf` as `NAN` and `INF` and the `e`
in standard notation will be an upper-case `E`.

```better-c
Nst_fmt("{f}"  , 0, NULL, INFINITY); // results in "Inf"
Nst_fmt("{f:P}", 0, NULL, INFINITY); // results in "INF"
Nst_fmt("{f}",   0, NULL, NAN); // results in "NaN"
Nst_fmt("{f:P}", 0, NULL, NAN); // results in "NAN"
Nst_fmt("{f:eP}", 0, NULL, 10.0); // results in "1E+01"
```

For the `b` type it will format `true` and `false` as `TRUE` and `FALSE`.

```better-c
Nst_fmt("{b}"  , 0, NULL, true); // results in "true"
Nst_fmt("{b:P}", 0, NULL, true); // results in "TRUE"
```

For the `p` type it will make the prefix be `0X` instead of `0x`.

```better-c
Nst_fmt("{p}", 0, NULL, (void *)0x325c4e4); // results in "0x325c4e4"
Nst_fmt("{p:P}", 0, NULL, (void *)0x325c4e4); // results in "0X325c4e4"
```

#### The `'` flag

This flag is supported by `B`, `i`, `l`, `L` and `f`, it specifies the
character to be used as the thousand separator.

It is not a standalone flag as it needs to be followed by another character
that will be the actual character used.

For example:

```better-c
Nst_fmt("{i}",    0, NULL, 1000000); // results in "1000000"
Nst_fmt("{i:''}", 0, NULL, 1000000); // results in "1'000'000"
Nst_fmt("{i:',}", 0, NULL, 1000000); // results in "1,000,000"
Nst_fmt("{i:' }", 0, NULL, 1000000); // results in "1 000 000"
```

#### The `_` flag

This flag is supported by all types and specifies the character used to fill
the extra space to reach `width` characters on the formatted value. If the
`width` is not specified this flag has no meaning.

Just like the `'` flag it is not a standalone flag as it needs to be followed
by another character that will be the actual character used.

By default the padding character is a space.

For example:

```better-c
Nst_fmt("{s}",     0, NULL, "hi"); // results in "hi"
Nst_fmt("{s:5}",   0, NULL, "hi"); // results in "hi   "
Nst_fmt("{s:_.5}", 0, NULL, "hi"); // results in "hi..."
Nst_fmt("{s:__5}", 0, NULL, "hi"); // results in "hi___"
```

#### The `c` flag

This flag is supported by all types and will make the formatted value exactly
`width` characters long. If the string is shorter than `width` it will be
padded normally but if it is longer it will be trimmed.

```better-c
Nst_fmt("{s:4}",  0, NULL, "hi");    // results in "hi  "
Nst_fmt("{s:4}",  0, NULL, "hello"); // results in "hello"
Nst_fmt("{s:c4}", 0, NULL, "hi");    // results in "hi  "
Nst_fmt("{s:c4}", 0, NULL, "hello"); // results in "hell"
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

```better-c
Nst_fmt("{i:5}", 0, NULL, 123);    // results in "  123"
Nst_fmt("{i:*}", 0, NULL, 123, 4); // results in " 123"
```

A negative width is ignored and a width of zero is useless without the `c`
flag.

### Precision

This field has different interpretations depending on the type being formatted:

For the `f` type in decimal and standard notation (with the `f` and `e` flags)
it specifies the number of digits after the dot. By default the precision is
`6`.

```better-c
Nst_fmt("{f:f.3}", 0, NULL, 1.234567); // results in "1.235"
Nst_fmt("{f:e.3}", 0, NULL, 1.234567); // results in "1.235e+00"
Nst_fmt("{f:f}", 0, NULL, 1.234567); // results in "1.234567"
Nst_fmt("{f:e}", 0, NULL, 1.234567); // results in "1.234567e+00"
```

For the `f` type in general notation the precision specifies the number of
significant digits to show. By default the precision is `6`.

```better-c
Nst_fmt("{f:.3}", 0, NULL, 1.234567); // results in "1.23"
Nst_fmt("{f}", 0, NULL, 1.234567); // results in "1.23457"
```

For `B`, `i`, `l` and `L` it specifies the minimum number of characters before
the sign, by default the extra characters are spaces but using the `0` flag
they become zeroes.

```better-c
Nst_fmt("{i}", 0, NULL, 123); // results in "123"
Nst_fmt("{i:.5}", 0, NULL, 123); // results in "  123"
Nst_fmt("{i:0.5}", 0, NULL, 123); // results in "00123"
```

When using a thousand separator it is displayed when using the `0` flag and is
otherwise accounted for with extra spaces in order to reach the same number of
characters in the final output.

```better-c
Nst_fmt("{i:0',.5}", 0, NULL, 123); // results in "00,123"
Nst_fmt("{i:',.5}",  0, NULL, 123); // results in "   123"
Nst_fmt("{i:.5}", 0, NULL, 123);    // results in "  123"
```

For the `s` type it specifies the maximum amount of characters to write, if a
string is longer than specified it is shortened to size removing characters
from the right and adding an ellipsis (`...`) at the end.

```better-c
Nst_fmt("{s:.5}", 0, NULL, "short"); // results in "short"
Nst_fmt("{s:.5}", 0, NULL, "somewhat long"); // results in "somew..."
```

The precision can be specified directly with a number after the dot (`.`) or by
writing an asterisk and passing an `int` value in the arguments after the value
to be formatted and, if specified, the width. For example:

```better-c
Nst_fmt("{i:.5}", 0, NULL, 123);    // results in "  123"
Nst_fmt("{i:.*}", 0, NULL, 123, 4); // results in " 123"
Nst_fmt("{i:0*.*}", 0, NULL, 123, 6, 4); // results in "  0123"
```

A negative precision is ignored.

### Separator width

This field applies to the types that support the `'` flag and changes the
amount of digits between separators from the default.

The default values for the separator width is as followes:

- `8` for numbers in binary
- `3` for numbers in octal or decimal (including all floats)
- `4` for numbers in hexadecimal

Like the precision, the separator width can be specified directly with a number
after the comma (`,`) or by writing an asterisk and passing an `int` value in
the arguments after the value to be formatted and, if specified, the width and
the precision. For example:

```better-c
Nst_fmt("{i:''}", 0, NULL, 12345);    // results in "12'345"
Nst_fmt("{i:'',4}", 0, NULL, 12345);    // results in "1'2345"
Nst_fmt("{i:''.*}", 0, NULL, 12345, 2); // results in "1'23'45"
Nst_fmt("{i:''0*.*,*}", 0, NULL, 123, 6, 4, 2); // results in " 01'23"
```

A separator width smaller than one ignored.

### Alignment

This field specifies the alignment of the formatted string. It is meaningful
only when used along side the `width` because otherwise the formatted values
will only ever be as bit as necessary.

The alignment can be one of three values:

- `<`: left align (default behaviour for most types)
- `>`: right align (default behaviour for `B`, `i`, `l`, `L` and `f`)
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

### `Nst_print`

**Synopsis:**

```better-c
isize Nst_print(const i8 *buf)
```

**Description:**

Prints a string to the Nest standard output.

!!!warning
    Do not use this function to print [`Nst_StrObj`](c_api-str.md#nst_strobj)
    objects, use [`Nst_fwrite`](c_api-file.md#nst_fwrite) instead.

**Parameters:**

- `buf`: the NUL-terminated string to print

**Returns:**

The number of bytes written. If the file is closed `-1` is returned. No error is
set.

---

### `Nst_fprint`

**Synopsis:**

```better-c
isize Nst_fprint(Nst_IOFileObj *f, const i8 *buf)
```

**Description:**

Prints a string to a Nest file object.

!!!warning
    Do not use this function to print [`Nst_StrObj`](c_api-str.md#nst_strobj)
    objects, use [`Nst_fwrite`](c_api-file.md#nst_fwrite) instead.

**Parameters:**

- `f`: the file to print the string to
- `buf`: the NUL-terminated string to print

**Returns:**

The number of bytes written. If the file is closed `-1` is returned. No error is
set.

---

### `Nst_println`

**Synopsis:**

```better-c
isize Nst_println(const i8 *buf)
```

**Description:**

Prints a string to the Nest standard output appending a newline character.

!!!warning
    Do not use this function to print [`Nst_StrObj`](c_api-str.md#nst_strobj)
    objects, use [`Nst_fwrite`](c_api-file.md#nst_fwrite) instead.

On all platforms only a newline (U+000A) is appended, NOT a carriage return.

**Parameters:**

- `buf`: the NUL-terminated string to print

**Returns:**

The number of bytes written, including the newline character. If the file is
closed `-1` is returned. No error is set.

---

### `Nst_fprintln`

**Synopsis:**

```better-c
isize Nst_fprintln(Nst_IOFileObj *f, const i8 *buf)
```

**Description:**

Prints a string to a Nest file object appending a newline character.

On all platforms only a newline (U+000A) is appended, NOT a carriage return.

!!!warning
    Do not use this function to print [`Nst_StrObj`](c_api-str.md#nst_strobj)
    objects, use [`Nst_fwrite`](c_api-file.md#nst_fwrite) instead.

**Parameters:**

- `f`: the file to print the string to
- `buf`: the NUL-terminated string to print

**Returns:**

The number of bytes written, including the newline character. If the file is
closed `-1` is returned. No error is set.

---

### `Nst_printf`

**Synopsis:**

```better-c
isize Nst_printf(Nst_WIN_FMT const i8 *fmt, ...)
```

**Description:**

Prints a formatted string to the Nest standard output.

Check the full [printf format rules](c_api-format.md#nst_printf-format-rules).

**Parameters:**

- `fmt`: the format placeholder
- `...`: the arguments to be formatted

**Returns:**

The number of characters written. On failure a negative value is returned and no
error is set. The negative value returned depends on the type of the error:

- `-1` signals a failure of vsprintf,
- `-2` that the output file is closed,
- `-3` an error in the format string and
- `-4` a memory allocation error.

---

### `Nst_fprintf`

**Synopsis:**

```better-c
isize Nst_fprintf(Nst_IOFileObj *f, Nst_WIN_FMT const i8 *fmt, ...)
```

**Description:**

Prints a formatted string to a Nest file object.

Check the full [printf format rules](c_api-format.md#nst_printf-format-rules).

**Parameters:**

- `f`: the file to print the string to
- `fmt`: the format placeholder
- `...`: the arguments to be formatted

**Returns:**

The number of characters written. On failure a negative value is returned and no
error is set. The negative value returned depends on the type of the error:

- `-1` signals a failure of vsprintf,
- `-2` that the output file is closed,
- `-3` an error in the format string and
- `-4` a memory allocation error.

---

### `Nst_vfprintf`

**Synopsis:**

```better-c
isize Nst_vfprintf(Nst_IOFileObj *f, const i8 *fmt, va_list args)
```

**Description:**

`va_list` variant of [`Nst_fprintf`](c_api-format.md#nst_fprintf).

---

### `Nst_sprintf`

**Synopsis:**

```better-c
Nst_Obj *Nst_sprintf(Nst_WIN_FMT const i8 *fmt, ...)
```

**Description:**

Creates a Nest string object from a format placeholder.

Check the full [printf format rules](c_api-format.md#nst_printf-format-rules).

**Parameters:**

- `fmt`: the format placeholder
- `...`: the arguments to be formatted

**Returns:**

The function returns the newly created string object.

---

### `Nst_vsprintf`

**Synopsis:**

```better-c
Nst_Obj *Nst_vsprintf(const i8 *fmt, va_list args)
```

**Description:**

`va_list` variant of [`Nst_sprintf`](c_api-format.md#nst_sprintf).

---

### `Nst_fmt`

**Synopsis:**

```better-c
i8 *Nst_fmt(const i8 *fmt, usize fmt_len, usize *out_len, ...)
```

**Description:**

Creates a heap-allocated string formatted with a more customizable format
placeholder.

Check the [full format rules](c_api-format.md#nst_fmt-format-rules) for this
function.

**Parameters:**

- `fmt`: the format placeholder
- `fmt_len`: the length of `fmt`, if set to `0` is it determined using
  [`strlen`](https://man7.org/linux/man-pages/man3/strlen.3.html)
- `out_len`: pointer to a value filled with the final lenght of the formatted
  string, it can be `NULL`
- `...`: the values to format

**Returns:**

The newly created string or `NULL` on failure, the error is set. When the
function fails and `out_len` is not `NULL` it is set to `0`.

---

### `Nst_vfmt`

**Synopsis:**

```better-c
i8 *Nst_vfmt(const i8 *fmt, usize fmt_len, usize *out_len, va_list args)
```

**Description:**

`va_list` variant of [`Nst_fmt`](c_api-format.md#nst_fmt).

---

### `Nst_fmt_objs`

**Synopsis:**

```better-c
Nst_Obj *Nst_fmt_objs(Nst_StrObj *fmt, Nst_SeqObj *values)
```

**Description:**

Similar to [`Nst_fmt`](c_api-format.md#nst_fmt), creates a string object
formatted with the values given.

**Parameters:**

- `fmt`: the format placeholder
- `values`: the values to format

**Returns:**

A new object of type `Str` or `NULL` on failure. The error is set.
