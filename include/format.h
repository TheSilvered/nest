#ifndef FORMAT_H
#define FORMAT_H

#include "simple_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
Format placeholder specification:

NOTE: if an invalid sequence is invalid the function fails and nothing is printed

Format placeholder regex:
%([+- 0'#]*(\d*|\*)?(.(\d*|\*))?(h|hh|l|ll|z|j|t)?[diufFgGeExXopcs]|%)

Format placeholder:
%[flags][width][.precision][length]type
To print a literal % use %%

Flags:
- '-': left-align the output
- '+': always print the sign on signed numeric types (by default '+' is not
       printed but '-' is)
- ' ': prepend a space on positive signed numbers
- '0': when a width is specifies any extra space is padded with '0' instead of
       ' ' for numeric types
- '\'': use an apostrophe as the thousand separator
- '#': alternate form
       * does not remove trailing zeroes from g and G
       * always puts a decimal point in f, F, e, E, g and G
       * adds the prefix '0x' to x and '0X' to X
       * adds '0' to o if the number is not 0

NOTE: together '+' and ' ' or using together '-' and '0' results in an invalid
      sequence

Width:
A number that specifies the minimum width of a format, does not truncate the
output and any extra characters. If an asterisk (*) is used, the width is
obtained from an argument of type int that precedes the value to format

nst_printf("%0*i", 3, 5); // Result: "005"
nst_printf("%3i", 1234); // Result: "1234"

Precision:
This parameter has different meanings depending on type to format:
- for d, i, u, x, X, o it specifies the minimum number of digits that the number
  must contain
- for e, E, f and F it specifies the number of digits after the decimal point,
  if it is not set it defaults to 6
- for g and G it specifies the number of significant digits to use, if not set
  it defaults to 6

nst_printf("%#-10.5X", 15); // Result: "0X000FF   "
nst_printf("%.2lf", 1.37); // Result: "1.3"

Length:
This parameter specifies the size of the argument passed:
- hh: an integer that was passed as a char
- h: an integer that was passed as a short
- l: a long int or a double
- ll: a long long int
- z: a size_t-sized argument
- j: a maxint_t-sized argument
- t: a ptrdiff_t-sized argument

NOTE: L, I, I32, I64 and q are not supported

Type:
- d, i: prints an integer
- u: prints an unsigned int
- f, F: prints a float
- e, E: prints a float in standard form
- g, G: prints a float removing the trailing zeroes and using the standard form
        when using more than the specified significant digits
- x, X: prints an unsigned int in hexadecimal, x uses lowercase letters and X
        uses uppercase
- o: prints an unsigned int in octal
- s: prints a null-terminated string
- c: prints a char
- p: prints a void *

NOTE: a, A and n are not supported
NOTE: when not specifying a length f, F, e, E, g and G expect float arguments,
      not doubles
*/

EXPORT isize nst_print(const i8 *buf);
EXPORT isize nst_println(const i8 *buf);
EXPORT isize nst_printf(const i8 *fmt, ...);
EXPORT isize nst_fprint(Nst_IOFileObj *f, const i8 *buf);
EXPORT isize nst_fprintln(Nst_IOFileObj *f, const i8 *buf);
EXPORT isize nst_fprintf(Nst_IOFileObj *f, const i8 *buf, ...);
EXPORT isize nst_vfprintf(Nst_IOFileObj *f, const i8 *buf, va_list args);
EXPORT Nst_Obj *nst_sprintf(const i8 *fmt, ...);
EXPORT Nst_Obj *nst_vsprintf(const i8 *fmt, va_list args);

#ifdef __cplusplus
}
#endif

#endif