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

## `nst_fmt` format rules

### General format

`{Type[:[Flags][Width][.Precision][Alignment]]}`

Note that the things inside the square brackets are optional.

### Type

The type determines how the argument passed is read. The following types are
allowed:

- `s` for `char *` (string)
- `i` for `int`
- `l` for `i32` (`long`)
- `L` for `i64` (`long long`)
- `b` for `bool` (takes an int but writes `true` or `false` instead of `1` and
  `0`)
- `u` for `isize` (`size_t`)
- `c` for `char`
- `r` or `f` for `double` or `float`
- `p` for `void *`

Additionally you can pass a Nest object to be formatted, in this case the
`Type` field should begin with a hash symbol (`#`) and no type needs to be
specified (since it is already inside the object itself). Optionally you can
add a letter after the hash to specify a type the object should be casted to
before being formatted.

The type specifiers for Nest objects are the following:

- `#` any Nest object
- `#i` an object casted to an Int
- `#r` an object casted to a Real
- `#b` an object casted to a Bool
- `#s` an object casted to a Str
- `#v` an object casted to a Vector
- `#a` an object casted to an Array
- `#m` an object casted to a Map
- `#I` an object casted to an Iter
- `#B` an object casted to a Byte

!!!note
    The letters for the type casts are the same used in `Nst_extract_args`, but
    `n`, `f`, `F` and `t` are omitted since no objects can be casted to these
    types other than objects of the same type.

### Flags

- `z`: normalize negative zero (`-0`) to zero (`0`) for floating point numbers
- `0`: with integers pad with zeroes to reach the number of digits specified
       in the precision before the prefix and any sign
- `f`: represent floats with a decimal representation, by default the minimum
       length representation is used
- `e`: represent floats in standard (or scientific) notation, by default the
       minimum length representation is used
- `p`: adds prefixes and specifies that prefixes, suffixes and infixes are
  written in lowercase, this flag applies to
  - integers: along the flags `x` and `X` adds the prefix `0x`, with the flag
    `o` adds the prefix `0o` and with the flag `b` adds the prefix `0b`, the
    prefix is added before the sign
  - floats when written in standard notation (the `e` is lowercase, default
    behaviour)
  - strings when using a different representation non-printable characters or
    non-ASCII characters will have the digits in the escape sequence in
    lowercase (es `\xff` or `\u00e8`, this is the default behaviour)
  - Nest Byte objects the suffix `b` is in lowercase (default behaviour)
- `P`: adds prefixes and specifies that prefixes, suffixes and infixes are
  written in uppercase, this flag applies to
  - integers: along the flags `x` and `X` adds the prefix `0X`, with the flag
    `o` adds the prefix `0O` and with the flag `b` adds the prefix `0B`, the
    prefix is added before the sign
  - floats when written in standard notation (the `E` is uppercase)
  - in strings when using a different representation non-printable characters
    or non-ASCII characters will have the digits in the escape sequence in
    lowercase (es `\xFF` or `\u00E8`)
  - Nest Byte objects the suffix `B` is in uppercase
- `c`: cut the formatted value to the specified width if it exceeds it, it cuts
       from the right if the value is left aligned, from the right if the value
       is left aligned and from both sides if it is center-aligned, this flag
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
- `'`: the character following this flag is used as the thousand separator in
       integers and floats, it is not interpreted for the formatted string
       (e.g. it can be anything like ` ` (a space), `.` or `'`)
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

### Precision

This field has different interpretations depending on the type being formatted:

- for floating point numbers in decimal notations it specifies the number of
  digits after the dot
- for floating point numbers in standard notation it specifies the number of
  significant digits to use
- for integers it specifies the minimum number of digits to use
- for strings it specifies the maximum amount of characters to write, if a
  string is longer than specified it is cut to size removing characters from
  the right (unlike the `c` flag). When using both the precision and the `c`
  flag and a width is specified the precision is ignored

### Alignment

This field specifies the alignment of the formatted string and is used when
either the string is too short and is padded to reach `width`, or it is too
long and the `c` flag is specified.

The alignment can be one of three values:

- `<`: align left (default behaviour for strings)
- `>`: align right (default behaviour for floats and integers)
- `^`: align center


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

Check the full format placeholder specification in
[`format.h`](c_api-format.md#format-placeholder-specification).

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

Check the full format placeholder specification in
[`format.h`](c_api-format.md#format-placeholder-specification).

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

Check the full format placeholder specification in
[`format.h`](c_api-format.md#format-placeholder-specification).

**Parameters:**

- `fmt`: the format placeholder
- `...`: the arguments to be formatted

**Returns:**

The function returns the number of characters written or `-1` on failure. No
error is set.

---

### `Nst_vsprintf`

**Synopsis:**

```better-c
Nst_Obj *Nst_vsprintf(const i8 *fmt, va_list args)
```

**Description:**

`va_list` variant of [`Nst_sprintf`](c_api-format.md#nst_sprintf).
