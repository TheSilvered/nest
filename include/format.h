/**
 * @file format.h
 *
 * @brief Various function to print formatted strings to file objects
 *
 * @author TheSilvered
 */

/* [docs:link format.h c_api-format.md#format-placeholder-specification] */

/* [docs:raw]
## Format placeholder specification

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
 * @brief Check the full format placeholder specification in `format.h`.
 *
 * @param fmt: the format placeholder
 * @param ...: the arguments to be formatted
 *
 * @return The number of characters written. On failure a negative value is
 * returned and no error is set. The negative value returned depends on the
 * type of the error:
 * !`-1` signals a failure of vsprintf,
 * !`-2` that the output file is closed,
 * !`-3` an error in the format string and
 * !`-4` a memory allocation error.
 */
NstEXP isize NstC Nst_printf(Nst_WIN_FMT const i8 *fmt, ...)
                             Nst_GNU_FMT(1, 2);
/**
 * Prints a formatted string to a Nest file object.
 *
 * @brief Check the full format placeholder specification in `format.h`.
 *
 * @param f: the file to print the string to
 * @param fmt: the format placeholder
 * @param ...: the arguments to be formatted
 *
 * @return The number of characters written. On failure a negative value is
 * returned and no error is set. The negative value returned depends on the
 * type of the error:
 * !`-1` signals a failure of vsprintf,
 * !`-2` that the output file is closed,
 * !`-3` an error in the format string and
 * !`-4` a memory allocation error.
 */
NstEXP isize NstC Nst_fprintf(Nst_IOFileObj *f, Nst_WIN_FMT const i8 *fmt, ...)
                                                Nst_GNU_FMT(2, 3);
/* `va_list` variant of `Nst_fprintf`. */
NstEXP isize NstC Nst_vfprintf(Nst_IOFileObj *f, const i8 *fmt, va_list args);

/**
 * Creates a Nest string object from a format placeholder.
 *
 * @brief Check the full format placeholder specification in `format.h`.
 *
 * @param fmt: the format placeholder
 * @param ...: the arguments to be formatted
 *
 * @return The function returns the number of characters written or `-1` on
 * failure. No error is set.
 */
NstEXP Nst_Obj *NstC Nst_sprintf(Nst_WIN_FMT const i8 *fmt, ...)
                                 Nst_GNU_FMT(1, 2);
/* `va_list` variant of `Nst_sprintf`. */
NstEXP Nst_Obj *NstC Nst_vsprintf(const i8 *fmt, va_list args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FORMAT_H
