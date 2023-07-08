#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include "format.h"
#include "global_consts.h"
#include "mem.h"

#define MIN(a, b) ((b) < (a) ? (b) : (a))
#define MAX(a, b) ((b) > (a) ? (b) : (a))

#define I8__MAX_DEC_LEN 4  // length of "-127"
#define I16_MAX_DEC_LEN 7  // length of "-32'768"
#define I32_MAX_DEC_LEN 14 // length of "-2'147'483'647"
#define I64_MAX_DEC_LEN 52 // length of "18'446'744'073'709'551'615"

#define I8__MAX_OCT_LEN 3  // length of "377"
#define I16_MAX_OCT_LEN 6  // length of "177777"
#define I32_MAX_OCT_LEN 11 // length of "37777777777"
#define I64_MAX_OCT_LEN 22 // length of "1777777777777777777777"

#define I8__MAX_HEX_LEN 2  // length of "ff"
#define I16_MAX_HEX_LEN 4  // length of "ffff"
#define I32_MAX_HEX_LEN 8  // length of "ffffffff"
#define I64_MAX_HEX_LEN 16 // length of "ffffffffffffffff"

#define PTR_MAX_LEN 18 // length of "0xffffffffffffffff"

#define E_EXTRA_SIZE 8 // the length of "+1.e+308"
#define G_EXTRA_SIZE 7 // the length of "+.e+308"

static int get_f_size(f64 val)
{
    if (isinf(val) || isnan(val))
        return 4;

    int exp;
    (void)frexp(val, &exp);
    if (exp < 100)
        return exp + exp / 3;
    else {
        exp = (int)((f64)exp * (1.0 / 3.0));
        return exp + exp / 3;
    }
}

static isize get_seq_size(const i8 **fmt, va_list *args)
{
    (*fmt)++;

    bool alternate_form = false;
    bool prepend_sign   = false;
    bool left_align     = false;
    bool zero_padding   = false;
    bool prepend_space  = false;
    bool thousand_sep   = false;

    int width = -1;
    int precision = -1;
    usize int_size = 0;
    usize float_size = 0;
    usize total_size = 0;

    if (**fmt == '%') {
        (*fmt)++;
        return 1;
    }

    while (true) {
        switch (**fmt) {
        case '#':
            alternate_form = true;
            (*fmt)++;
            continue;
        case '+':
            if (prepend_space)
                return -1;
            prepend_sign = true;
            (*fmt)++;
            continue;
        case '-':
            if (zero_padding)
                return -1;
            left_align = true;
            (*fmt)++;
            continue;
        case ' ':
            if (prepend_sign)
                return -1;
            prepend_space = true;
            (*fmt)++;
            continue;
        case '0':
            if (left_align)
                return -1;
            zero_padding = true;
            (*fmt)++;
            continue;
        case '\'':
            thousand_sep = true;
            (*fmt)++;
            continue;
        }
        break;
    }

    if (isdigit((u8)**fmt)) {
        i8 *end;
        width = (int)strtol(*fmt, &end, 10);
        *fmt = end;
    } else if (**fmt == '*') {
        width = va_arg(*args, int);
        (*fmt)++;
    }

    if (**fmt == '.') {
        (*fmt)++;
        if (isdigit((u8)**fmt)) {
            i8 *end;
            precision = (int)strtol(*fmt, &end, 10);
            *fmt = end;
        } else if (**fmt == '*') {
            precision = va_arg(*args, int);
            (*fmt)++;
        }
    }

    switch (**fmt) {
    case 'h':
        (*fmt)++;
        if (**fmt == 'h') {
            int_size = sizeof(int);
            (*fmt)++;
            break;
        }
        int_size = sizeof(int);
        break;
    case 'l':
        (*fmt)++;
        if (**fmt == 'l') {
            int_size = sizeof(long long int);
            (*fmt)++;
            break;
        }
        int_size = sizeof(long int);
        float_size = sizeof(double);
        break;
    case 'z':
        (*fmt)++;
        int_size = sizeof(size_t);
        break;
    case 'j':
        (*fmt)++;
        int_size = sizeof(intmax_t);
        break;
    case 't':
        (*fmt)++;
        int_size = sizeof(ptrdiff_t);
    case 'q':
    case 'L': // long double is not supported
    case 'I': // neither are I, I32 and I64
        return -1;
    }

    switch (**fmt) {
    case 'u':
        if (prepend_sign || prepend_space)
            return -1;
    case 'd':
    case 'i':
        if (int_size == 0 && float_size == 0)
            int_size = sizeof(int);
        if (int_size == 0 || alternate_form)
            return -1;
        switch (int_size) {
        case 1:
            total_size = I8__MAX_DEC_LEN;
            (void)va_arg(*args, int);
            break;
        case 2:
            total_size = I16_MAX_DEC_LEN;
            (void)va_arg(*args, int);
            break;
        case 4:
            total_size = I32_MAX_DEC_LEN;
            (void)va_arg(*args, i32);
            break;
        case 8:
            total_size = I64_MAX_DEC_LEN;
            (void)va_arg(*args, i64);
            break;
        }
        total_size = MAX(width, (isize)total_size);
        total_size = MAX(precision, (isize)total_size);
        (*fmt)++;
        return total_size;
    case 'x':
    case 'X':
        if (int_size == 0 && float_size == 0)
            int_size = sizeof(int);

        if (int_size == 0 || thousand_sep || prepend_sign || prepend_space)
            return -1;

        switch (int_size) {
        case 1:
            total_size = I8__MAX_HEX_LEN;
            (void)va_arg(*args, int);
            break;
        case 2:
            total_size = I16_MAX_HEX_LEN;
            (void)va_arg(*args, int);
            break;
        case 4:
            total_size = I32_MAX_HEX_LEN;
            (void)va_arg(*args, i32);
            break;
        case 8:
            total_size = I64_MAX_HEX_LEN;
            (void)va_arg(*args, i64);
            break;
        }
        if (alternate_form)
            total_size += 2;
        total_size = MAX(width, (isize)total_size);
        total_size = MAX(precision, (isize)total_size);
        (*fmt)++;
        return total_size;
    case 'o':
        if (int_size == 0 && float_size == 0)
            int_size = sizeof(int);

        if (int_size == 0 || thousand_sep || prepend_sign || prepend_space)
            return -1;

        switch (int_size) {
        case 1:
            total_size = I8__MAX_OCT_LEN;
            (void)va_arg(*args, int);
            break;
        case 2:
            total_size = I16_MAX_OCT_LEN;
            (void)va_arg(*args, int);
            break;
        case 4:
            total_size = I32_MAX_OCT_LEN;
            (void)va_arg(*args, i32);
            break;
        case 8:
            total_size = I64_MAX_OCT_LEN;
            (void)va_arg(*args, i64);
            break;
        }
        if (alternate_form)
            total_size++;
        total_size = MAX(width, (isize)total_size);
        total_size = MAX(precision, (isize)total_size);
        (*fmt)++;
        return total_size;
    case 'c':
        if (int_size != 0 || float_size != 0 || thousand_sep || prepend_sign
            || prepend_space || alternate_form  || precision >= 0)
        {
            return -1;
        }
        total_size = MAX(1, width);
        return total_size;
    case 's':
        if (int_size != 0 || float_size != 0 || thousand_sep || prepend_sign
            || prepend_space || alternate_form)
        {
            return -1;
        }
        i8 *str = va_arg(*args, i8 *);
        total_size = strlen(str);
        if ( precision >= 0 )
        {
            total_size = MIN((isize)total_size, precision);
        }
        total_size = MAX((isize)total_size, width);
        (*fmt)++;
        return total_size;
    case 'p':
        if (int_size != 0 || float_size != 0 || thousand_sep || prepend_sign
            || prepend_space || alternate_form  || precision >= 0)
        {
            return -1;
        }
        (void)va_arg(*args, void *);
        total_size = MAX(PTR_MAX_LEN, width);
        (*fmt)++;
        return total_size;
    case 'e':
    case 'E':
        if (int_size == 0 && float_size == 0)
            float_size = sizeof(double);

        if (float_size == 0 || thousand_sep)
            return -1;

        if (precision < 0)
            precision = 6;

        total_size = precision + E_EXTRA_SIZE;
        total_size = MAX((isize)total_size, width);
        (void)va_arg(*args, f64);
        (*fmt)++;
        return total_size;
    case 'g':
    case 'G':
        if (int_size == 0 && float_size == 0)
            float_size = sizeof(double);

        if (float_size == 0)
            return -1;

        if (precision < 0)
            precision = 6;

        total_size = precision + G_EXTRA_SIZE;
        total_size = MAX((isize)total_size, width);
        (void)va_arg(*args, f64);
        (*fmt)++;
        return total_size;
    case 'f':
    case 'F':
        if (int_size == 0 && float_size == 0)
            float_size = sizeof(double);

        if (float_size == 0)
            return -1;

        if (precision < 0)
            precision = 6;
        double val;

        val = va_arg(*args, f64);

        total_size = precision + get_f_size(val) + 2;
        total_size = MAX((isize)total_size, width);

        (*fmt)++;
        return total_size;
    }

    return -1;
}

static isize get_max_size_printf(const i8 *fmt, va_list orig_args)
{
    usize tot_size = strlen(fmt);
    va_list args;
    va_copy(args, orig_args);

    while (*fmt != '\0') {
        if (*fmt != '%') {
            fmt++;
            continue;
        }
        isize seq_size = get_seq_size(&fmt, &args);
        if (seq_size < 0) {
            return -1;
        }
        tot_size += seq_size;
    }
    return tot_size + 1;
}

isize Nst_print(const i8 *buf)
{
    return Nst_fprint(nst_io.out, buf);
}

isize Nst_println(const i8 *buf)
{
    return Nst_fprintln(nst_io.out, buf);
}

isize Nst_printf(Nst_WIN_FMT const i8 *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return nst_vfprintf(nst_io.out, fmt, args);
}

isize Nst_fprint(Nst_IOFileObj *f, const i8 *buf)
{
    if (NST_IOF_IS_CLOSED(f))
        return -1;

    usize len = strlen(buf);
    return nst_fwrite((i8 *)buf, sizeof(i8), (usize)len, f);
}

isize Nst_fprintln(Nst_IOFileObj *f, const i8 *buf)
{
    if (NST_IOF_IS_CLOSED(f))
        return -1;

    usize len = strlen(buf);
    size_t a = f->write_f((void *)buf, 1, (size_t)len, f->value);
    size_t b = f->write_f((void *)"\n", 1, 1, f->value);
    return a + b;
}

isize Nst_fprintf(Nst_IOFileObj *f, Nst_WIN_FMT const i8 *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return nst_vfprintf(f, fmt, args);
}

Nst_Obj *Nst_sprintf(Nst_WIN_FMT const i8 *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return Nst_vsprintf(fmt, args);
}

Nst_Obj *Nst_vsprintf(const i8 *fmt, va_list args)
{
    usize buf_size = get_max_size_printf(fmt, args);
    if (buf_size < 0) {
        va_end(args);
        return NULL;
    }
    i8 *buf = nst_calloc_c(buf_size, i8, NULL);
    if (buf == NULL) {
        Nst_error_clear();
        va_end(args);
        return NULL;
    }
    int len = vsprintf(buf, fmt, args);
    va_end(args);

    if (len < 0) {
        nst_free(buf);
        return NULL;
    }

    Nst_Obj *str = nst_string_new(buf, len, true);
    if (str == NULL) {
        Nst_error_clear();
        nst_free(buf);
    }
    return str;
}

isize nst_vfprintf(Nst_IOFileObj *f, const i8 *fmt, va_list args)
{
    if (NST_IOF_IS_CLOSED(f)) {
        va_end(args);
        return -2;
    }

    isize buf_size = get_max_size_printf(fmt, args);
    if (buf_size < 0) {
        va_end(args);
        return -3;
    }
    i8 *buf = nst_calloc_c(buf_size, i8, NULL);
    if (buf == NULL) {
        Nst_error_clear();
        va_end(args);
        return -4;
    }
    int len = vsprintf(buf, fmt, args);
    va_end(args);

    if (len < 0) {
        nst_free(buf);
        return -1;
    }
    isize result = nst_fwrite(buf, sizeof(i8), len, f);
    nst_free(buf);
    return result;
}
