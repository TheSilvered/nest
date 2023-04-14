# `format.h`

!!!warning
    Any funtion that uses a format string will fail to write if an invalid
    placeholder is found.

## Functions

### `nst_print`

**Synopsis**:

```better-c
isize nst_print(const i8 *buf)
```

**Description**:

Prints a nul-terminated string to `@@io._get_stdout`.

**Arguments**:

- `[in] buf`: the string to print

**Return value**:

The function returns the number of bytes written or `-1` in case an error occurs.

---

### `nst_println`

**Synopsis**:

```better-c
isize nst_println(const i8 *buf)
```

**Description**:

Prints a nul-terminated string to `@@io._get_stdout` appending a line feed at
the end.

**Arguments**:

- `[in] buf`: the string to print

**Return value**:

The function returns the number of bytes written or `-1` in case an error occurs.

---

### `nst_printf`

**Synopsis**:

```better-c
isize nst_println(const i8 *fmt, ...)
```

**Description**:

Formats a string and prints it to `@@io._get_stdout`.

**Arguments**:

- `[in] fmt`: the [format template string](#format-strings)
- `[in] ...`: the values required by the format string

**Return value**:

The function returns the number of bytes written or `-1` in case an error occurs.

---

### `nst_fprint`

**Synopsis**:

```better-c
isize nst_fprint(Nst_IOFileObj *f, const i8 *buf)
```

**Description**:

Prints a nul-terminated string to a Nest file object.

**Arguments**:

- `[in] f`: the file to print to
- `[in] buf`: the string to print

**Return value**:

The function returns the number of bytes written or `-1` in case an error occurs.

---

### `nst_fprintln`

**Synopsis**:

```better-c
isize nst_fprintln(Nst_IOFileObj *f, const i8 *buf)
```

**Description**:

Prints a nul-terminated string to a Nest file object appending a line feed at
the end.

**Arguments**:

- `[in] f`: the file to print to
- `[in] buf`: the string to print

**Return value**:

The function returns the number of bytes written or `-1` in case an error occurs.

---

### `nst_fprintf`

**Synopsis**:

```better-c
isize nst_fprintf(Nst_IOFileObj *f, const i8 *fmt, ...)
```

**Description**:

Formats a string and prints it to a Nest file object.

**Arguments**:

- `[in] f`: the file to print to
- `[in] fmt`: the [format template string](#format-strings)
- `[in] ...`: the values required by the format string

**Return value**:

The function returns the number of bytes written or `-1` in case an error occurs.

---

### `nst_vfprintf`

**Synopsis**:

```better-c
isize nst_vfprintf(Nst_IOFileObj *f, const i8 *fmt, va_list args)
```
**Description**:

Formats a string and prints it to a Nest file object.

**Arguments**:

- `[in] f`: the file to print to
- `[in] fmt`: the [format template string](#format-strings)
- `[in] args`: the values required by the format string

**Return value**:

The function returns the number of bytes written or `-1` in case an error occurs.

---

### `nst_sprintf`

**Synopsis**:

```better-c
Nst_Obj *nst_sprintf(const i8 *fmt, ...)
```

**Description**:

Creates a new Nest string object given a format string.

**Arguments**:

- `[in] fmt`: the [format template string](#format-strings)
- `[in] ...`: the values required by the format string

**Return value**:

The function returns the newly created string or `NULL` if an error occurs.

---

### `nst_vsprintf`

**Synopsis**:

```better-c
Nst_Obj *nst_vsprintf(const i8 *fmt, va_list args)
```

**Description**:

Creates a new Nest string object given a format string.

**Arguments**:

- `[in] fmt`: the [format template string](#format-strings)
- `[in] args`: the values required by the format string

**Return value**:

The function returns the newly created string or `NULL` if an error occurs.

---

## Format strings

A format string is a nul-terminated string that contains placeholders for values
to be inserted. A placeholder is introduced by a percentage sign (`%`) and
specifies various modifications of the value.

**Placeholder syntax**:

`%[flags][width][.precision][length]type`

To print a literal `%` use `%%` instead.

**Placeholder regex**:

`%([+- 0'#]*(\d+|\*)?(.(\d+|\*))?(h|hh|l|ll|z|j|t)?[diufFgGeExXopcs]|%)`

!!!note
    This regex is not meant as a definitive specification; if it matches the
    placeholder used, it can still be considered invalid.

**Flags field**:

- `-`: left-align the output
- `+`: always print the sign on signed numeric types (by default it is printed
  only in negative numbers)
- `[space]`: prepend a space on positive numbers
- `0`: when a width is specified, any extra space is padded with `0` instead of
  spaces for numeric types
- `\`: use an apostrophe as the thousand separator
- `#`: alternate form
  - does not remove trailing zeroes from `g` and `G`
  - always puts a decimal point in `f`, `F`, `e`, `E`, `g` and `G`
  - adds the prefix `0x` to `x` and `0X` to `X`
  - adds `0` to `o` if the number is not `0`

!!!note
    `+` and `[space]` or `-` and `0` cannot be used in the same sequence.
    Using `+`, `[space]` or `0` with a non-numeric type also results in an
    invalid sequence.

**Width**:

A number that specifies the minimum width of a format, does not truncate the
output and any extra characters. If an asterisk (`*`) is used, the width is
obtained from an argument of type `int` that precedes the value to format.

```better-c
nst_printf("%0*i", 3, 5); // Result: "005"
nst_printf("%3i", 1234); // Result: "1234"
```

**Precision**:

This parameter has different meanings depending on type to format:

- for `d`, `i`, `u`, `x`, `X`, `o` it specifies the minimum number of digits
  that the number must contain
- for `e`, `E`, `f` and `F` it specifies the number of digits after the decimal
  point, if it is not set it defaults to 6
- for `g` and `G` it specifies the number of significant digits to use, if not
  set it defaults to 6

You can use an asterisk to specify a dynamic precision similarly to the width.
If both the width and the precision are set as dynamic the first number will be
taken as the width and the second as the precision.

```better-c
nst_printf("%#-10.5X", 15); // Result: "0X000FF   "
nst_printf("%.2lf", 1.37); // Result: "1.3"
```

**Length**:

This parameter specifies the size of the argument passed:

- `hh`: an integer that was passed as a `char`
- `h`: an integer that was passed as a `short`
- `l`: a `long int` or a `double`
- `ll`: a `long long int`
- `z`: a `size_t`-sized argument
- `j`: a `maxint_t`-sized argument
- `t`: a `ptrdiff_t`-sized argument

!!!note
    `L`, `I`, `I32`, `I64` and `q` are not supported

**Type**:

- `d`, `i`: prints an integer
- `u`: prints an unsigned int
- `f`, `F`: prints a float
- `e`, `E`: prints a float in standard form, the difference is that `E` uses a
  capital E and `e` a lowercase one
- `g`, `G`: prints a float removing the trailing zeroes and using the standard
  form when using more than the specified significant digits, the two are the
  same when printing the number normally but `G` uses a capital E in standard
  form and `g` uses a lowercase one
- `x`, `X`: prints an unsigned int in hexadecimal, `x` uses lowercase letters
  and `X` uses uppercase
- `o`: prints an `unsigned int` in octal
- `s`: prints a null-terminated string
- `c`: prints a `char`
- `p`: prints a `void *`

!!!note
    `a`, `A` and `n` are not supported
