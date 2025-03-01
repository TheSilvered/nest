/**
 * @file format.h
 *
 * @brief Various function to print formatted strings to file objects
 *
 * @author TheSilvered
 */

/* [docs:link full\format\rules <c_api-format.md#nst_fmt-format-rules> t] */
/* [docs:link printf <https://man7.org/linux/man-pages/man3/printf.3.html>] */

/* [docs:raw]
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
*/

#ifndef FORMAT_H
#define FORMAT_H

#include "file.h"
#include "sequence.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Prints a string to the Nest standard output.
 *
 * @brief Warning: do not use this function to print `Str` objects, use
 * `Nst_fwrite` instead.
 *
 * @param buf: the NUL-terminated string to print
 *
 * @return The number of bytes written. If the file is closed `-1` is returned.
 * No error is set.
 */
NstEXP isize NstC Nst_print(const char *buf);
/**
 * Prints a string to a Nest file object.
 *
 * @brief Warning: do not use this function to print `Str` objects, use
 * `Nst_fwrite` instead.
 *
 * @param f: the file to print the string to
 * @param buf: the NUL-terminated string to print
 *
 * @return The number of bytes written. If the file is closed `-1` is returned.
 * No error is set.
 */
NstEXP isize NstC Nst_fprint(Nst_Obj *f, const char *buf);

/**
 * Prints a string to the Nest standard output appending a newline character.
 *
 * @brief Warning: do not use this function to print `Str` objects, use
 * `Nst_fwrite` instead.
 *
 * @brief On all platforms only a newline (U+000A) is appended, NOT a carriage
 * return.
 *
 * @param buf: the NUL-terminated string to print
 *
 * @return The number of bytes written, including the newline character. If the
 * file is closed `-1` is returned. No error is set.
 */
NstEXP isize NstC Nst_println(const char *buf);
/**
 * Prints a string to a Nest file object appending a newline character.
 *
 * @brief On all platforms only a newline (U+000A) is appended, NOT a carriage
 * return.
 *
 * @brief Warning: do not use this function to print `Str` objects, use
 * `Nst_fwrite` instead.
 *
 * @param f: the file to print the string to
 * @param buf: the NUL-terminated string to print
 *
 * @return The number of bytes written, including the newline character. If the
 * file is closed `-1` is returned. No error is set.
 */
NstEXP isize NstC Nst_fprintln(Nst_Obj *f, const char *buf);

/**
 * Prints a formatted string to the Nest standard output. The format specifier
 * works like that of C's `printf`.
 *
 * @param fmt: the format placeholder
 * @param ...: the arguments to be formatted
 *
 * @return The number of characters written. On failure a negative value is
 * returned and no error is set. The negative value returned depends on the
 * type of the error:
 *! `-1` signals a failure of vsprintf,
 *! `-2` that the output file is closed,
 *! `-3` an error in the format string and
 *! `-4` a memory allocation error.
 */
NstEXP isize NstC Nst_printf(Nst_WIN_FMT const char *fmt, ...)
                             Nst_NIX_FMT(1, 2);
/**
 * Prints a formatted string to a Nest file object. The format specifier
 * works like that of C's `printf`.
 *
 * @param f: the file to print the string to
 * @param fmt: the format placeholder
 * @param ...: the arguments to be formatted
 *
 * @return The number of characters written. On failure a negative value is
 * returned and no error is set. The negative value returned depends on the
 * type of the error:
 *! `-1` signals a failure of vsprintf,
 *! `-2` that the output file is closed,
 *! `-3` an error in the format string and
 *! `-4` a memory allocation error.
 */
NstEXP isize NstC Nst_fprintf(Nst_Obj *f, Nst_WIN_FMT const char *fmt, ...)
                                          Nst_NIX_FMT(2, 3);
/* `va_list` variant of `Nst_fprintf`. */
NstEXP isize NstC Nst_vfprintf(Nst_Obj *f, const char *fmt, va_list args);

/**
 * Creates a Nest string object from a format placeholder. The format specifier
 * works like that of C's `printf`.
 *
 * @brief Check the full `printf format rules`.
 *
 * @param fmt: the format placeholder
 * @param ...: the arguments to be formatted
 *
 * @return The function returns the newly created string object.
 */
NstEXP Nst_Obj *NstC Nst_sprintf(Nst_WIN_FMT const char *fmt, ...)
                                 Nst_NIX_FMT(1, 2);
/* `va_list` variant of `Nst_sprintf`. */
NstEXP Nst_Obj *NstC Nst_vsprintf(const char *fmt, va_list args);

/**
 * Creates a heap-allocated string formatted with a more customizable format
 * placeholder.
 *
 * @brief Check the `full format rules` for this function.
 *
 * @param fmt: the format placeholder
 * @param fmt_len: the length of `fmt`, if set to `0` is it determined using
 * `strlen`
 * @param out_len: pointer to a value filled with the final lenght of the
 * formatted string, it can be `NULL`
 * @param ...: the values to format
 *
 * @return The newly created string or `NULL` on failure, the error is set.
 * When the function fails and `out_len` is not `NULL` it is set to `0`.
 */
NstEXP u8 *NstC Nst_fmt(const char *fmt, usize fmt_len, usize *out_len, ...);
/* `va_list` variant of `Nst_fmt`. */
NstEXP u8 *NstC Nst_vfmt(const char *fmt, usize fmt_len, usize *out_len,
                         va_list args);
/**
 * Similar to `Nst_fmt`, creates a string object formatted with the values
 * given.
 *
 * @param fmt: the format placeholder
 * @param values: the values to format
 *
 * @return A new object of type `Str` or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_fmt_objs(Nst_Obj *fmt, Nst_Obj *values);

/**
 * Make a string into its representation, like using the `r`, `R`, `a` and `A`
 * flags in `Nst_fmt`.
 *
 * @param str: the initial string
 * @param str_len: the length in bytes of the string, if set to zero it will be
 * calculated with `strlen`
 * @param out_len: pointer to be set with the lengh of the output string, if
 * set to `NULL` it will be ignored
 * @param shallow: if set to true it will only escape special characters, like
 * the `R` and `A` flags
 * @param ascii: if set to true it will produce an output that uses only
 * printable ASCII characters
 *
 * @return A pointer to the string representation or `NULL` on failure. The
 * error is set.
 */
NstEXP u8 *NstC Nst_repr(u8 *str, usize str_len, usize *out_len,
                         bool shallow, bool ascii);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FORMAT_H
