# `dtoa.h`

Functions to format floating point numbers. Header for the library by David M.
Gay.

## Authors

TheSilvered

---

## Functions

### `Nst_dtoa`

**Synopsis:**

```better-c
i8 *Nst_dtoa(f64 value, int mode, int n_digits, int *decimal_point, int *sign,
             i8 **str_end)
```

**Description:**

Converts a double to an ASCII string.

Modes:

- `0`: shortest string that yields `value` when read in and rounded to nearest.
- `1`: like 0, but with Steele & White stopping rule; e.g. with IEEE P754
  arithmetic, mode `0` gives `1e23` whereas mode `1` gives
  9.999999999999999e22`.
- `2`: `max(1, n_digits)` significant digits; this gives a return value similar
  to that of `ecvt`, except that trailing zeros are suppressed.
- `3`: through `n_digits` past the decimal point; this gives a return value
  similar to that from `fcvt`, except that trailing zeros are suppressed, and
  `n_digits` can be negative.
- `4`, `5`: similar to `2` and `3`, respectively, but (in round-nearest mode)
  with the tests of mode `0` to possibly return a shorter string that rounds to
  `value`.
- `6` through `9`: Debugging modes similar to mode - 4: don't try fast
  floating-point estimate (if applicable).

**Parameters:**

- `value`: the double to convert
- `mode`: the mode to use to convert the number, valid modes range from
- `n_digits`: the number of digits to output, the specific amount varies in
  regards to the `mode` used
- `decimal_point`: pointer where to place the position of the decimal point
  relative to the string of digits returned, it is set to `9999` if `value` is
  NaN or Infinity
- `sign`: set to `0` for positive values and to `1` for negative values
- `str_end`: a pointer set to the end of the returned string, it is ignored if
  set to `NULL`

**Returns:**

An allocated string of digits where trailing zeroes are suppressed. This value
must be freed with [`Nst_freedtoa`](c_api-dtoa.md#nst_freedtoa).

---

### `Nst_freedtoa`

**Synopsis:**

```better-c
void Nst_freedtoa(i8 *str)
```

**Description:**

Frees a string returned by [`Nst_dtoa`](c_api-dtoa.md#nst_dtoa).

---

### `Nst_strtod`

**Synopsis:**

```better-c
f64 Nst_strtod(const i8 *str, i8 **str_end)
```

**Description:**

Converts a string to a double.

This function works like
[`strtod`](https://man7.org/linux/man-pages/man3/strtod.3p.html) in the C
standard.

**Parameters:**

- `str`: the string to convert
- `str_end`: pointer set to the character after the last digit of the parsed
  number, it is ignored if set to `NULL`

**Returns:**

The parsed number.
