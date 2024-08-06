/**
 * @file format.h
 *
 * @brief Various function to print formatted strings to file objects
 *
 * @author TheSilvered
 */

/* [docs:link printf\format\rules <c_api-format.md#nst_printf-format-rules> t] */
/* [docs:link full\format\rules <c_api-format.md#nst_fmt-format-rules> t] */

/* [docs:raw]
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

### General format

`{Type[:[Flags][Width][.Precision][,SeparatorWidth][Alignment]]}`

Note that the things inside the square brackets are optional.

### Type

The type determines how the argument passed is read. The following types are
allowed:

- `s` for `char *` (string)
- `i` for `int` or `uint` (`unsigned int`)
- `l` for `i32` (`long`) or `u32` (`unsigned long`)
- `L` for `i64` (`long long`) or `u64` (`unsigned long long`)
- `z` for `isize` (`size_t`) or `usize` (`ptrdiff_t`)
- `b` for `bool` (takes an int but writes `true` or `false` instead of `1` and
  `0`)
- `f` for `f32` (`float`) or `f64` (`double`)
- `c` for `char`
- `p` for `void *`

Additionally you can pass a Nest object to be formatted, in this case the
`Type` field should begin with a hash symbol (`#`) and no type needs to be
specified (since it is already inside the object itself). Optionally you can
add a letter after the hash to specify a type the object should be casted to
before being formatted.

The type specifiers for Nest objects are the following:

- `#` any Nest object
- `#i` a Nest object casted to an `Int`
- `#r` a Nest object casted to a `Real`
- `#b` a Nest object casted to a `Bool`
- `#s` a Nest object casted to a `Str`
- `#v` a Nest object casted to a `Vector`
- `#a` a Nest object casted to an `Array`
- `#m` a Nest object casted to a `Map`
- `#I` a Nest object casted to an `Iter`
- `#B` a Nest object casted to a `Byte`

!!!note
    The letters for the type casts are the same used in `Nst_extract_args`, but
    `n`, `f`, `F` and `t` are omitted since no objects can be casted to these
    types other than objects of the same type.

### Flags

- `z`: normalize negative zero (`-0`) to zero (`0`) for floating point numbers
- `0`: with integers pad with zeroes to reach the number of digits specified
       in the precision before the prefix and any sign
- `f`: represent floats with a decimal representation, by default the general
       representation is used
- `e`: represent floats in standard (or scientific) notation, by default the
       gemeral representation is used
- `p`: adds lowercase prefixes and suffixes, this flag applies to:
  - integers: along the flags `x` and `X` adds the prefix `0x`, with the flag
    `o` adds the prefix `0o` and with the flag `b` adds the prefix `0b`, the
    prefix is added after the sign
  - floats: causes `Inf` and `NaN` to be written `inf` and `nan` respectively
  - Nest Byte objects: adds the suffix `b`
- `P`: adds uppercase prefixes to ingegers and specifies that some lowercase
  parts of other types is uppercase, specifically this flag applies to:
  - integers: along the flags `x` and `X` adds the prefix `0X`, with the flag
    `o` adds the prefix `0O` and with the flag `b` adds the prefix `0B`, the
    prefix is added after the sign
  - floats: when written in standard notation the `E` is in uppercase, by
    default it is lowercase, and causes `Inf` and `NaN` to be written `INF` and
    `NAN` respectively
  - strings: when using a different representation non-printable characters
    or non-ASCII characters will have the digits in the escape sequence in
    uppercase (es `\xFF` or `\u00E8`), by default they are lowercase
  - booleans: writes `TRUE` and `FALSE` in uppercase
  - Nest Byte objects: adds the suffix `B`
- `c`: cut the formatted value to the specified width if it exceeds it, it cuts
       from the right if the value is left-aligned, from the left if the value
       is right-aligned and from both sides if it is center-aligned, this flag
       ignores the type of value that is formatted
- `b`: represent integers in binary format without the prefix
- `o`: represent integers in octal format without the prefix
- `x`: represent integers in lowercase hexadecimal format without the prefix
- `X`: represent integers in uppercase hexadecimal format without the prefix
- ` ` (space): add a space in front of positive integers or floats,
      (compensates the absence of a minus sign `-`)
- `+`: add a plus sign (`+`) in front of positive integers or floats
- `r`: write a string as a valid Nest literal, escaping non-printable or
       special characters and adding quotes (either single or double) around
       the string
- `R`: write a string as a valid Nest literal, escaping non-printable or
       special characters but surrounding quotes are not added and not escaped
       inside the string
- `a`: write a string as a valid Nest literal using only ASCII characters,
       escaping non-printable, special or non-ASCII characters and adding
       quotes (either single or double) around the string
- `A`: write a string as a valid Nest literal using only ASCII characters,
       escaping non-printable, special or non-ASCII characters but surrounding
       quotes are not added and not escaped inside the string
- `u`: treat the integer as unsigned
- `'`: the character following this flag is used as the separator in integers
  and floats, it is not interpreted for the formatted string (e.g. it can be
  anything like ` ` (a space), `.` or `'`), this separator is written as
  follows:
  - for decimal integers it is placed every three digits from the right
    (e.g. 2,863,311,530)
  - for binary integers it is placed every eight digits from the right
    (e.g. 110,01011011,11010110)
  - for octal integers it is placed every three digits from the right
    (e.g. 73,627,452,474)
  - for hexadecimal integers it is placed every four digits from the right
    (e.g. D1,DC44,5250,244C)
- `_`: the character following this flag is used as the padding character to
       reach the specified width in any formatted value, it is not interpreted
       for the formatted string (e.g. it can be anything like ` ` (a space),
       `.` or `'`), by default the padding character is a space (` `)
!!!note
    If flags are incompatible only the last one is considered (e.g. if you
    write `pP` the prefixes will be uppercase), if the flag is incompatible
    with the type it is ignored.

### Width

In general this field specifies the minimum width of the formatted string, if
the string is shorter than the specified width it is padded with spaces or with
the character specified with the `_` flag. A string longer than the specified
with is left untouched.

When paired with the `c` flag is instead specifies the exact width of the
formatted string, any string longer than width is cut to size as specified
in the description of the `c` flag.

The width can be specified directly with a number after the flags or by writing
an asterisk and passing an `int` value in the arguments after the value to be
formatted.

A negative width is ignored.

### Precision

This field has different interpretations depending on the type being formatted:

- for floating point numbers in decimal and standard notation it specifies the
  number of digits after the dot
- for floating point numbers in general notation it specifies the number of
  significant digits to show
- for integers it specifies the minimum number of digits to use
- for strings it specifies the maximum amount of characters to write, if a
  string is longer than specified it is cut to size removing characters from
  the right (unlike the `c` flag). When using both the precision and the `c`
  flag and a width is specified the precision is ignored

The precision can be specified directly with a number after the dot (`.`) or by
writing an asterisk and passing an `int` value in the arguments after the value
to be formatted and, if specified, the width.

A negative precision is ignored.

### Separator width

This field applies to numbers and changes the amount of digits between
separators from the default. For example using `{i:b' ,4}` will write a binary
number with groups of four digits separated by a space, `{i:b' } would write
groups of eight digits. This field is ignored when it has a value below one.

The separator width can be specified directly with a number after the comma
(`,`) or by writing an asterisk and passing an `int` value in the arguments
after the value to be formatted and, if specified, the width and the precision.

A separator width smaller than or equal to zero is ignored.

### Alignment

This field specifies the alignment of the formatted string and is used when
either the string is too short and is padded to reach `width`, or it is too
long and the `c` flag is specified.

The alignment can be one of three values:

- `<`: align left (default behaviour for strings)
- `>`: align right (default behaviour for floats and integers)
- `^`: align center
*/

#ifndef FORMAT_H
#define FORMAT_H

#include "file.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Prints a string to the Nest standard output.
 *
 * @brief Warning: do not use this function to print `Nst_StrObj` objects, use
 * `Nst_fwrite` instead.
 *
 * @param buf: the NUL-terminated string to print
 *
 * @return The number of bytes written. If the file is closed `-1` is returned.
 * No error is set.
 */
NstEXP isize NstC Nst_print(const i8 *buf);
/**
 * Prints a string to a Nest file object.
 *
 * @brief Warning: do not use this function to print `Nst_StrObj` objects, use
 * `Nst_fwrite` instead.
 *
 * @param f: the file to print the string to
 * @param buf: the NUL-terminated string to print
 *
 * @return The number of bytes written. If the file is closed `-1` is returned.
 * No error is set.
 */
NstEXP isize NstC Nst_fprint(Nst_IOFileObj *f, const i8 *buf);

/**
 * Prints a string to the Nest standard output appending a newline character.
 *
 * @brief Warning: do not use this function to print `Nst_StrObj` objects, use
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
NstEXP isize NstC Nst_println(const i8 *buf);
/**
 * Prints a string to a Nest file object appending a newline character.
 *
 * @brief On all platforms only a newline (U+000A) is appended, NOT a carriage
 * return.
 *
 * @brief Warning: do not use this function to print `Nst_StrObj` objects, use
 * `Nst_fwrite` instead.
 *
 * @param f: the file to print the string to
 * @param buf: the NUL-terminated string to print
 *
 * @return The number of bytes written, including the newline character. If the
 * file is closed `-1` is returned. No error is set.
 */
NstEXP isize NstC Nst_fprintln(Nst_IOFileObj *f, const i8 *buf);

/**
 * Prints a formatted string to the Nest standard output.
 *
 * @brief Check the full `printf format rules`.
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
NstEXP isize NstC Nst_printf(Nst_WIN_FMT const i8 *fmt, ...)
                             Nst_GNU_FMT(1, 2);
/**
 * Prints a formatted string to a Nest file object.
 *
 * @brief Check the full `printf format rules`.
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
NstEXP isize NstC Nst_fprintf(Nst_IOFileObj *f, Nst_WIN_FMT const i8 *fmt, ...)
                                                Nst_GNU_FMT(2, 3);
/* `va_list` variant of `Nst_fprintf`. */
NstEXP isize NstC Nst_vfprintf(Nst_IOFileObj *f, const i8 *fmt, va_list args);

/**
 * Creates a Nest string object from a format placeholder.
 *
 * @brief Check the full `printf format rules`.
 *
 * @param fmt: the format placeholder
 * @param ...: the arguments to be formatted
 *
 * @return The function returns the newly created string object.
 */
NstEXP Nst_Obj *NstC Nst_sprintf(Nst_WIN_FMT const i8 *fmt, ...)
                                 Nst_GNU_FMT(1, 2);
/* `va_list` variant of `Nst_sprintf`. */
NstEXP Nst_Obj *NstC Nst_vsprintf(const i8 *fmt, va_list args);

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
NstEXP i8 *NstC Nst_fmt(const i8 *fmt, usize fmt_len, usize *out_len, ...);
/* `va_list` variant of `Nst_fmt`. */
NstEXP i8 *NstC Nst_vfmt(const i8 *fmt, usize fmt_len, usize *out_len,
                         va_list args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FORMAT_H
