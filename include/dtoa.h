/**
 * @file dtoa.h
 *
 * @brief Functions to format floating point numbers. Header for the library
 * by David M. Gay
 *
 * @author TheSilvered
 */

#ifndef DTOA_H
#define DTOA_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* [docs:link ecvt <https://man7.org/linux/man-pages/man3/ecvt.3.html>] */
/* [docs:link fcvt <https://man7.org/linux/man-pages/man3/ecvt.3.html>] */
/* [docs:link strtod <https://man7.org/linux/man-pages/man3/strtod.3p.html>] */

/**
 * Convert a `double` to an ASCII string.
 *
 * @brief Modes:
 *! `0`: shortest string that yields `value` when read in and rounded to
 * nearest.
 *! `1`: like 0, but with Steele & White stopping rule; e.g. with IEEE P754
 * arithmetic, mode `0` gives `1e23` whereas mode `1` gives
 * 9.999999999999999e22`.
 *! `2`: `max(1, n_digits)` significant digits; this gives a return value
 * similar to that of `ecvt`, except that trailing zeros are suppressed.
 *! `3`: through `n_digits` past the decimal point; this gives a return value
 * similar to that from `fcvt`, except that trailing zeros are suppressed, and
 * `n_digits` can be negative.
 *! `4`, `5`: similar to `2` and `3`, respectively, but (in round-nearest mode)
 * with the tests of mode `0` to possibly return a shorter string that rounds
 * to `value`.
 *! `6` through `9`: Debugging modes similar to mode - 4: don't try fast
 * floating-point estimate (if applicable).
 *
 * @param value: the double to convert
 * @param mode: the mode to use to convert the number, valid modes range from
 * @param n_digits: the number of digits to output, the specific amount varies
 * in regards to the `mode` used
 * @param decimal_point: pointer where to place the position of the decimal
 * point relative to the string of digits returned, it is set to `9999` if
 * `value` is NaN or Infinity
 * @param sign: set to `0` for positive values and to `1` for negative values,
 * it is ignored if set to `NULL`
 * @param str_end: a pointer set to the end of the returned string, it is
 * ignored if set to `NULL`
 *
 * @return An allocated string of digits where trailing zeroes are suppressed.
 * This value must be freed with `Nst_freedtoa`.
 */
NstEXP char *NstC Nst_dtoa(f64 value, int mode, int n_digits,
                           int *decimal_point, int *sign, char **str_end);
/* Free a string returned by `Nst_dtoa`. */
NstEXP void NstC Nst_freedtoa(char *str);
/**
 * Convert a string to a double.
 *
 * @brief This function works like `strtod` in the C standard.
 *
 * @param str: the string to convert
 * @param str_end: pointer set to the character after the last digit of the
 * parsed number, it is ignored if set to `NULL`
 *
 * @return The parsed number.
 */
NstEXP f64 NstC Nst_strtod(const char *str, char **str_end);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !DTOA_H
