#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include "format.h"
#include "global_consts.h"
#include "mem.h"
#include "file.h"

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
        break;
    case 'q':
    case 'L': // long double is not supported
    case 'I': // neither are I, I32 and I64
        return -1;
    }

    switch (**fmt) {
    case 'u':
        if (prepend_sign || prepend_space)
            return -1;
        // fall through
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
        if (precision >= 0)
            total_size = MIN((isize)total_size, precision);
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
        if (seq_size < 0)
            return -1;
        tot_size += seq_size;
    }
    return tot_size + 1;
}

isize Nst_print(const i8 *buf)
{
    return Nst_fprint(Nst_io.out, buf);
}

isize Nst_println(const i8 *buf)
{
    return Nst_fprintln(Nst_io.out, buf);
}

isize Nst_printf(Nst_WIN_FMT const i8 *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return Nst_vfprintf(Nst_io.out, fmt, args);
}

isize Nst_fprint(Nst_IOFileObj *f, const i8 *buf)
{
    usize len = strlen(buf);
    usize count;
    if (Nst_fwrite((i8 *)buf, (usize)len, &count, f) < 0)
        return -1;
    return count;
}

isize Nst_fprintln(Nst_IOFileObj *f, const i8 *buf)
{
    if (Nst_IOF_IS_CLOSED(f))
        return -1;

    usize len = strlen(buf);
    usize count_a, count_b;
    if (Nst_fwrite((i8 *)buf, len, &count_a, f) < 0)
        return -1;
    if (Nst_fwrite((i8 *)"\n", 1, &count_b, f) < 0)
        return -1;
    return count_a + count_b;
}

isize Nst_fprintf(Nst_IOFileObj *f, Nst_WIN_FMT const i8 *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return Nst_vfprintf(f, fmt, args);
}

Nst_Obj *Nst_sprintf(Nst_WIN_FMT const i8 *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return Nst_vsprintf(fmt, args);
}

Nst_Obj *Nst_vsprintf(const i8 *fmt, va_list args)
{
    isize buf_size = get_max_size_printf(fmt, args);
    if (buf_size < 0) {
        va_end(args);
        return NULL;
    }
    i8 *buf = Nst_calloc_c(buf_size, i8, NULL);
    if (buf == NULL) {
        Nst_error_clear();
        va_end(args);
        return NULL;
    }
    int len = vsprintf(buf, fmt, args);
    va_end(args);

    if (len < 0) {
        Nst_free(buf);
        return NULL;
    }

    Nst_Obj *str = Nst_string_new(buf, len, true);
    if (str == NULL) {
        Nst_error_clear();
        Nst_free(buf);
    }
    return str;
}

isize Nst_vfprintf(Nst_IOFileObj *f, const i8 *fmt, va_list args)
{
    if (Nst_IOF_IS_CLOSED(f)) {
        va_end(args);
        return -2;
    }

    isize buf_size = get_max_size_printf(fmt, args);
    if (buf_size < 0) {
        va_end(args);
        return -3;
    }
    i8 *buf = Nst_calloc_c(buf_size, i8, NULL);
    if (buf == NULL) {
        Nst_error_clear();
        va_end(args);
        return -4;
    }
    int len = vsprintf(buf, fmt, args);
    va_end(args);

    if (len < 0) {
        Nst_free(buf);
        return -1;
    }
    usize count;
    Nst_IOResult result = Nst_fwrite(buf, len, &count, f);
    Nst_free(buf);
    return result >= 0 ? (isize)count : -1;
}

/* ============================== Nest format ============================== */

typedef enum _Alignment {
    Nst_FMT_ALIGN_AUTO,
    Nst_FMT_ALIGN_LEFT,
    Nst_FMT_ALIGN_CENTER,
    Nst_FMT_ALIGN_RIGHT
} Alignment;

typedef enum _IntRepr {
    Nst_FMT_INTR_BIN,
    Nst_FMT_INTR_OCT,
    Nst_FMT_INTR_DEC,
    Nst_FMT_INTR_HEX,
    Nst_FMT_INTR_UPPER_HEX
} IntRepr;

typedef enum _AlignSign {
    Nst_FMT_SIGN_NO_SIGN,
    Nst_FMT_SIGN_SPACE,
    Nst_FMT_SIGN_PLUS
} AlignSign;

#define REPR_FULL 0b100
#define REPR_ASCII 0b10

typedef enum _StrRepr {
    Nst_FMT_REPR_NO_REPR       = 0,
    Nst_FMT_REPR_SHALLOW       = 1,
    Nst_FMT_REPR_SHALLOW_ASCII = 1 | REPR_ASCII,
    Nst_FMT_REPR_FULL          = 1 | REPR_FULL,
    Nst_FMT_REPR_FULL_ASCII    = 1 | REPR_ASCII | REPR_FULL
} StrRepr;

typedef enum _PrefSuffMode {
    Nst_FMT_NO_PREF_OR_SUFF,
    Nst_FMT_PREF_SUFF_LOWER,
    Nst_FMT_PREF_SUFF_UPPER
} PrefSuffMode;

typedef enum _DoubleRepr {
    Nst_FMT_MIN_REPR,
    Nst_FMT_DEC_REPR,
    Nst_FMT_STD_FORM
} DoubleRepr;

typedef struct _Format {
    bool normalize_neg_zero;
    bool cut;
    bool as_unsigned;
    bool pad_zeroes_precision;
    DoubleRepr double_repr;
    PrefSuffMode pref_suff;
    StrRepr str_repr;
    AlignSign sign;
    IntRepr int_repr;
    i8 thousand_sep[4];
    i8 fill_ch[4];
    i32 width;
    i32 precision;
    Alignment alignment;
} Format;

static void format_init(Format *format)
{
    format->normalize_neg_zero = false;
    format->cut = false;
    format->as_unsigned = false;
    format->pad_zeroes_precision = false;
    format->double_repr = Nst_FMT_MIN_REPR;
    format->pref_suff = Nst_FMT_NO_PREF_OR_SUFF;
    format->str_repr = Nst_FMT_REPR_NO_REPR;
    format->sign = Nst_FMT_SIGN_NO_SIGN;
    format->int_repr = Nst_FMT_INTR_DEC;
    memset(format->thousand_sep, 255, 4);
    memset(format->fill_ch, 255, 4);
    format->width = -1;
    format->precision = -1;
    format->alignment = Nst_FMT_ALIGN_AUTO;
}

// TODO: removes the warning, remove later
/* static */ const i8 *format_set_thousand_sep(Format *format, const i8 *ch)
{
    memset(format->thousand_sep, 0, 4);
    i32 ch_len = Nst_check_utf8_bytes((u8 *)ch, Nst_CP_MULTIBYTE_MAX_SIZE);
    memcpy(format->thousand_sep, ch, (usize)ch_len);
    return ch + ch_len;
}

// TODO: removes the warning, remove later
/* static */ bool format_has_thousand_sep(Format *format)
{
    return (u8)format->thousand_sep[0] != 255;
}

// TODO: removes the warning, remove later
/* static */ usize format_thousand_sep_len(Format *format)
{
    if (format->thousand_sep[1] == 0)
        return 1;
    else if (format->thousand_sep[2] == 0)
        return 2;
    else if (format->thousand_sep[3] == 0)
        return 3;
    return 4;
}

static const i8 *format_set_fill_ch(Format *format, const i8 *ch)
{
    memset(format->fill_ch, 0, 4);
    i32 ch_len = Nst_check_utf8_bytes((u8 *)ch, Nst_CP_MULTIBYTE_MAX_SIZE);
    memcpy(format->fill_ch, ch, (usize)ch_len);
    return ch + ch_len;
}

static bool format_has_fill_ch(Format *format)
{
    return (u8)format->fill_ch[0] != 255;
}

static usize format_fill_ch_len(Format *format)
{
    if (format->fill_ch[1] == 0)
        return 1;
    else if (format->fill_ch[2] == 0)
        return 2;
    else if (format->fill_ch[3] == 0)
        return 3;
    return 4;
}

static const i8 *fmt_value(Nst_Buffer *buf, const i8 *fmt, va_list *args);

static bool fmt_str(Nst_Buffer *buf, i8 *str, isize str_len, Format *format);
static isize fmt_str_repr(Nst_Buffer *buf, i8 *str, usize str_len,
                          Format *format);
static bool more_double_quotes(u8 *str, usize str_len);
static isize ascii_escape(Nst_Buffer *buf, u8 c, Format *format);
static isize unicode_escape(Nst_Buffer *buf, i32 c, Format *format);
static bool fmt_str_align(Nst_Buffer *buf, i8 *str, usize str_len,
                          usize char_str_len, Format *format);

static bool fmt_int(Nst_Buffer *buf, i64 val, Format *format);
static bool fmt_float(Nst_Buffer *buf, f64 val, Format *format);
static bool fmt_bool(Nst_Buffer *buf, bool val, Format *format);
static bool fmt_ptr(Nst_Buffer *buf, void *val, Format *format);
static bool fmt_char(Nst_Buffer *buf, i8 val, Format *format);

static void fmt_cut(i8 *str, usize str_len, usize char_len, i8 **out_str,
                    usize *out_len, Format *format,
                    Alignment default_alignment);

i8 *Nst_fmt(const i8 *fmt, usize fmt_len, usize *out_len, ...)
{
    va_list args;
    va_start(args, out_len);
    return Nst_vfmt(fmt, fmt_len, out_len, args);
}

i8 *Nst_vfmt(const i8 *fmt, usize fmt_len, usize *out_len, va_list args)
{
    Nst_Buffer buf;
    va_list args_cpy;
    va_copy(args_cpy, args);

    usize fmtlen = fmt_len != 0 ? fmt_len : strlen(fmt);
    if (out_len != NULL)
        *out_len = 0;
    if (fmtlen == 0) {
        i8 *out_str = Nst_malloc_c(1, i8);
        if (out_str == NULL)
            return NULL;
        out_str[0] = 0;
        return out_str;
    }

    if (Nst_check_string_cp(Nst_cp(Nst_CP_UTF8), (void *)fmt, fmtlen) != -1) {
        Nst_set_value_error_c("Nst_fmt: `fmt` is not valid UTF-8");
        return false;
    }

    if (!Nst_buffer_init(&buf, fmtlen))
        return NULL;

    for (usize i = 0; i < fmtlen; i++) {
        if (fmt[i] != '{' && fmt[i] != '}') {
            if (!Nst_buffer_append_char(&buf, fmt[i]))
                goto failure;
        }

        if (fmt[i] == '}') {
            if (fmt[i + 1] != '}') {
                Nst_set_value_error_c("found single '}' in format string");
                goto failure;
            }
            if (!Nst_buffer_append_char(&buf, '}'))
                goto failure;
            i++;
            continue;
        }

        if (fmt[i] == '{') {
            if (fmt[i + 1] == '{') {
                if (!Nst_buffer_append_char(&buf, '{'))
                    goto failure;
                i++;
                continue;
            }
            const i8 *fmt_end = fmt_value(&buf, fmt + i, &args_cpy);
            if (fmt_end == NULL)
                goto failure;
            i += fmt_end - fmt - i - 1;
        }
    }

    if (out_len != NULL)
        *out_len = buf.len;
    return buf.data;

failure:
    Nst_buffer_destroy(&buf);
    return NULL;
}

static const i8 *parse_format(const i8 *fmt, Format *format, va_list *args)
{
    while (true) {
        bool end_loop = false;
        switch (*fmt) {
        case 'z':
            format->normalize_neg_zero = true;
            break;
        case '0':
            format->pad_zeroes_precision = true;
            break;
        case 'f':
            format->double_repr = Nst_FMT_DEC_REPR;
            break;
        case 'e':
            format->double_repr = Nst_FMT_STD_FORM;
            break;
        case 'p':
            format->pref_suff = Nst_FMT_PREF_SUFF_LOWER;
            break;
        case 'P':
            format->pref_suff = Nst_FMT_PREF_SUFF_UPPER;
            break;
        case 'c':
            format->cut = true;
            break;
        case 'b':
            format->int_repr = Nst_FMT_INTR_BIN;
            break;
        case 'o':
            format->int_repr = Nst_FMT_INTR_OCT;
            break;
        case 'x':
            format->int_repr = Nst_FMT_INTR_HEX;
            break;
        case 'X':
            format->int_repr = Nst_FMT_INTR_UPPER_HEX;
            break;
        case ' ':
            format->sign = Nst_FMT_SIGN_SPACE;
            break;
        case '+':
            format->sign = Nst_FMT_SIGN_PLUS;
            break;
        case 'r':
            format->str_repr = Nst_FMT_REPR_FULL;
            break;
        case 'R':
            format->str_repr = Nst_FMT_REPR_SHALLOW;
            break;
        case 'a':
            format->str_repr = Nst_FMT_REPR_FULL_ASCII;
            break;
        case 'A':
            format->str_repr = Nst_FMT_REPR_SHALLOW_ASCII;
            break;
        case 'u':
            format->as_unsigned = true;
            break;
        case '\'': {
                fmt = format_set_thousand_sep(format, fmt + 1);
                fmt--;
                break;
        }
        case '_': {
            fmt = format_set_fill_ch(format, fmt + 1);
            fmt--;
            break;
        }
        default:
            end_loop = true;
        }
        if (end_loop)
            break;
        fmt++;
    }

    if (*fmt > '0' && *fmt <= '9')
        format->width = strtol(fmt, (i8 **)&fmt, 10);
    else if (*fmt == '*') {
        format->width = va_arg(*args, i32);
        fmt++;
    }

    if (*fmt == '.') {
        fmt++;
        if ((*fmt < '0' || *fmt > '9') && *fmt != '*') {
            Nst_set_value_error_c("expected a number for precision in format");
            return NULL;
        }
        if (*fmt == '*') {
            format->precision = va_arg(*args, i32);
            fmt++;
        } else
            format->precision = strtol(fmt, (i8 **)&fmt, 10);
    }

    switch (*fmt) {
    case '<':
        format->alignment = Nst_FMT_ALIGN_LEFT;
        fmt++;
        break;
    case '>':
        format->alignment = Nst_FMT_ALIGN_RIGHT;
        fmt++;
        break;
    case '^':
        format->alignment = Nst_FMT_ALIGN_CENTER;
        fmt++;
        break;
    }
    return fmt;
}

static const i8 *fmt_value(Nst_Buffer *buf, const i8 *fmt, va_list *args)
{
    bool nest_obj = false;
    const i8 *type;
    Format format;
    format_init(&format);

    fmt++;
    if (*fmt == '#') {
        nest_obj = true;
        fmt++;
        type = fmt;
    } else
        type = fmt;
    fmt++;

    if (*fmt == '}') {
        fmt++;
        goto format_type;
    } else if (*fmt != ':') {
        Nst_set_value_error_c("expected ':' in format string");
        return NULL;
    }
    fmt++;

    fmt = parse_format(fmt, &format, args);
    if (fmt == NULL)
        return NULL;

    if (*fmt != '}') {
        Nst_set_value_error_c("invalid format string");
        return NULL;
    }
    fmt++;

format_type:
    if (nest_obj)
        goto format_nest_obj;

    bool result = true;
    switch (*type) {
    case 's': {
        i8 *str = va_arg(*args, i8 *);
        result = fmt_str(buf, str, -1, &format);
        break;
    }
    case 'i': {
        int val = va_arg(*args, int);
        result = fmt_int(buf, (i64)val, &format);
        break;
    }
    case 'l': {
        i32 val = va_arg(*args, i32);
        result = fmt_int(buf, (i64)val, &format);
        break;
    }
    case 'L': {
        i64 val = va_arg(*args, i64);
        result = fmt_int(buf, val, &format);
        break;
    }
    case 'b': {
        int val = va_arg(*args, int);
        result = fmt_bool(buf, (bool)val, &format);
        break;
    }
    case 'u': {
        isize val = va_arg(*args, isize);
        result = fmt_int(buf, (i64)val, &format);
        break;
    }
    case 'c': {
        int val = va_arg(*args, int);
        result = fmt_char(buf, (i8)val, &format);
        break;
    }
    case 'r':
    case 'f': {
        f64 val = va_arg(*args, f64);
        result = fmt_float(buf, val, &format);
        break;
    }
    case 'p': {
        void *ptr = va_arg(*args, void *);
        result = fmt_ptr(buf, ptr, &format);
        break;
    }
    default:
        Nst_set_value_errorf("invalid type letter '%c' in format", *type);
        return NULL;
    }

    if (!result)
        return NULL;
    return fmt;

format_nest_obj:
    Nst_set_value_error_c("formatting for Nest objects is not yet supported");
    return NULL;

}

/* Cuts a string keeping it left-aligned. */
static void fmt_cut_left(i8 *str, usize str_len, usize char_len, i8 **out_str,
                         usize *out_len, usize width)
{
    Nst_UNUSED(char_len);
    *out_str = str;
    *out_len = str_len;

    Nst_StrObj str_ob = Nst_string_temp((i8 *)str, str_len);
    usize count = 0;

    for (isize i = Nst_string_next(&str_ob, -1);
        i >= 0;
        i = Nst_string_next(&str_ob, i))
    {
        if (count == (usize)width) {
            *out_len = i;
            return;
        }
        count++;
    }
}

/* Cuts a string keeping it right-aligned. */
static void fmt_cut_right(i8 *str, usize str_len, usize char_len, i8 **out_str,
                          usize *out_len, usize width)
{
    usize cut_size = char_len - width;
    *out_str = str;
    *out_len = str_len;
    Nst_StrObj str_ob = Nst_string_temp(str, str_len);

    for (isize i = Nst_string_next(&str_ob, -1);
        i >= 0;
        i = Nst_string_next(&str_ob, i))
    {
        if (cut_size == 0) {
            *out_str = str + i;
            *out_len = str_len - i;
            return;
        }
        cut_size--;
    }
}

/* Cuts a string keeping it center-aligned. */
static void fmt_cut_center(i8 *str, usize str_len, usize char_len,
                           i8 **out_str, usize *out_len, usize width)
{
    usize cut_size = char_len - width;
    // characters to remove from the left
    usize left_chars = cut_size / 2;
    // characters to remove from the right
    usize right_chars = cut_size - left_chars;

    // removing the characters from the left first ensurest that no
    // character is iterated on more than once
    fmt_cut_right(str, str_len, char_len, &str, &str_len, width + left_chars);
    fmt_cut_left(str, str_len, char_len - right_chars, out_str, out_len, width);
}

static void fmt_cut(i8 *str, usize str_len, usize char_len, i8 **out_str,
                    usize *out_len, Format *format,
                    Alignment default_alignment)
{
    Alignment alignment = format->alignment;
    if (alignment == Nst_FMT_ALIGN_AUTO)
        alignment = default_alignment;

    // should change later but ensures always valid values
    *out_str = str;
    *out_len = str_len;

    switch (alignment) {
    case Nst_FMT_ALIGN_LEFT:
        fmt_cut_left(str, str_len, char_len, out_str, out_len, format->width);
        break;
    case Nst_FMT_ALIGN_RIGHT:
        fmt_cut_right(str, str_len, char_len, out_str, out_len, format->width);
        break;
    case Nst_FMT_ALIGN_CENTER:
        fmt_cut_center(str, str_len, char_len, out_str, out_len, format->width);
        break;
    case Nst_FMT_ALIGN_AUTO:
        break;
    }
}

/* =========================== String formatting =========================== */

static bool fmt_str(Nst_Buffer *buf, i8 *str, isize str_len, Format *format)
{
    bool result = true;
    if (str == NULL) {
        str = "(null)";
        str_len = 6;
    }

    if (str_len < 0)
        str_len = strlen(str);

    isize vaild = Nst_check_string_cp(
        Nst_cp(Nst_CP_EXT_UTF8),
        (void *)str,
        str_len);
    if (vaild != -1) {
        Nst_set_value_error_c(
            "Nst_fmt: the string to format is not valid UTF-8");
        return false;
    }

    i32 precision = format->cut && format->width > 0
        ? -1
        : format->precision;

    Nst_Buffer *temp_buf;
    if (format->width <= 0 && precision < 0)
        temp_buf = buf;
    else {
        temp_buf = Nst_malloc_c(1, Nst_Buffer);
        if (temp_buf == NULL)
            return false;
        if (!Nst_buffer_init(temp_buf, str_len)) {
            Nst_free(temp_buf);
            return false;
        }
    }

    isize char_str_len = fmt_str_repr(temp_buf, str, str_len, format);
    if (char_str_len < 0) {
        result = false;
        goto finish;
    }
    if (format->width <= 0 && precision < 0)
        goto finish; // the buffer was already appended in fmt_str_repr

    // from here until finish we are sure that temp_buf is a new buffer and not
    // a reference to buf

    str = temp_buf->data;
    str_len = temp_buf->len;

    if (precision >= 0 && precision < char_str_len) {
        fmt_cut_left(
            str, str_len, char_str_len,
            &str, (usize *)&str_len,
            precision);
        char_str_len = precision;
    }

    // in this case the string does not need to be cut, just aligned
    if (char_str_len < format->width) {
        result = fmt_str_align(buf, str, str_len, char_str_len, format);
        goto finish;
    } else if (char_str_len > format->width && format->cut) {
        fmt_cut(
            str, str_len, char_str_len,
            &str, (usize *)&str_len,
            format, Nst_FMT_ALIGN_LEFT);
    }

    result = Nst_buffer_append_str(buf, str, str_len);

finish:
    if (temp_buf != buf) {
        Nst_buffer_destroy(temp_buf);
        Nst_free(temp_buf);
    }
    return result;
}

static isize fmt_str_repr(Nst_Buffer *buf, i8 *str, usize str_len,
                          Format *format)
{
    // Not always the correct size but reduces allocations
    if (!Nst_buffer_expand_by(buf, str_len))
        return -1;

    StrRepr repr = format->str_repr;
    if (repr == Nst_FMT_REPR_NO_REPR) {
        Nst_buffer_append_str(buf, str, str_len);
        return Nst_string_utf8_char_len((u8 *)str, str_len);
    }

    usize tot_char_len = 0;
    bool escape_single_quotes = false;
    bool escape_double_quotes = false;
    if (repr & REPR_FULL) {
        escape_single_quotes = more_double_quotes((u8 *)str, str_len);
        escape_double_quotes = !escape_single_quotes;
        if (!Nst_buffer_append_char(buf, escape_single_quotes ? '\'' : '"'))
            return -1;
        tot_char_len += 1;
    }

    Nst_StrObj str_ob = Nst_string_temp((i8 *)str, str_len);
    isize i = -1;

    for (i32 c = Nst_string_next_utf32(&str_ob, &i);
         c != -1;
         c = Nst_string_next_utf32(&str_ob, &i))
    {
        if (c == '"' && escape_double_quotes) {
            if (!Nst_buffer_append_str(buf, "\\\"", 2))
                return -1;
            tot_char_len += 2;
        } else if (c == '\'' && escape_single_quotes) {
            if (!Nst_buffer_append_str(buf, "\\'", 2))
                return -1;
            tot_char_len += 2;
        } else if (c < 0x7f && c >= 0x20 && c != '\\') {
            if (!Nst_buffer_append_char(buf, (i8)c))
                return -1;
            tot_char_len += 1;
        } else if (c <= 0x7f) {
            isize char_len = ascii_escape(buf, (u8)c, format);
            if (char_len < 0)
                return -1;
            tot_char_len += char_len;
        } else if (repr & REPR_ASCII) {
            isize char_len = unicode_escape(buf, c, format);
            if (char_len < 0)
                return -1;
            tot_char_len += char_len;
        } else {
            if (!Nst_buffer_expand_by(buf, Nst_CP_MULTIBYTE_MAX_SIZE + 1))
                return -1;
            i32 bytes_written = Nst_ext_utf8_from_utf32(
                (u32)c,
                (u8 *)buf->data + buf->len);
            buf->len += bytes_written;
            buf->data[buf->len] = 0;
            tot_char_len += 1;
        }
    }

    if (repr & REPR_FULL) {
        if (!Nst_buffer_append_char(buf, escape_single_quotes ? '\'' : '"'))
            return -1;
        tot_char_len += 1;
    }

    return tot_char_len;
}

static bool more_double_quotes(u8 *str, usize str_len)
{
    usize single_count = 0;
    usize double_count = 0;

    for (usize i = 0; i < str_len; i++) {
        if (str[i] == '\'')
            single_count++;
        else if (str[i] == '"')
            double_count++;
    }

    return double_count >= single_count;
}

static isize ascii_escape(Nst_Buffer *buf, u8 c, Format *format)
{
    const i8 *hex_chars = format->pref_suff == Nst_FMT_PREF_SUFF_UPPER
        ? "0123456789ABCDEF"
        : "0123456789abcdef";

    switch (c) {
    case '\0': Nst_buffer_append_c_str(buf, "\\0"); return 2;
    case '\a': Nst_buffer_append_c_str(buf, "\\a"); return 2;
    case '\b': Nst_buffer_append_c_str(buf, "\\b"); return 2;
    case'\x1b':Nst_buffer_append_c_str(buf, "\\e"); return 2;
    case '\f': Nst_buffer_append_c_str(buf, "\\f"); return 2;
    case '\n': Nst_buffer_append_c_str(buf, "\\n"); return 2;
    case '\r': Nst_buffer_append_c_str(buf, "\\r"); return 2;
    case '\t': Nst_buffer_append_c_str(buf, "\\t"); return 2;
    case '\v': Nst_buffer_append_c_str(buf, "\\v"); return 2;
    case '\\': Nst_buffer_append_c_str(buf, "\\\\");return 2;
    default:
        if (!Nst_buffer_append_c_str(buf, "\\x"))
            return -1;
        if (!Nst_buffer_append_char(buf, hex_chars[c >> 4]))
            return -1;
        if (!Nst_buffer_append_char(buf, hex_chars[c & 0xf]))
            return -1;
        return 3;
    }
}

static isize unicode_escape(Nst_Buffer *buf, i32 c, Format *format)
{
    const i8 *hex_chars = format->pref_suff == Nst_FMT_PREF_SUFF_UPPER
        ? "0123456789ABCDEF"
        : "0123456789abcdef";

    if (!Nst_buffer_expand_by(buf, c <= 0xffff ? 6 : 8))
        return -1;
    Nst_buffer_append_char(buf, '\\');
    Nst_buffer_append_char(buf, c <= 0xffff ? 'u' : 'U');
    for (usize i = 1, n = c <= 0xffff ? 4 : 6; i <= n; i++)
        Nst_buffer_append_char(buf, hex_chars[(c >> ((n - i) * 4)) & 0xf]);
    return c <= 0xffff ? 6 : 8;
}

static bool fmt_str_align(Nst_Buffer *buf, i8 *str, usize str_len,
                          usize char_str_len, Format *format)
{
    i8 *fill_ch;
    usize fill_ch_len;
    if (format_has_fill_ch(format)) {
        fill_ch = format->fill_ch;
        fill_ch_len = format_fill_ch_len(format);
    } else {
        fill_ch_len = 1;
        fill_ch = " ";
    }

    usize fill_width = format->width - char_str_len;

    if (!Nst_buffer_expand_by(buf, str_len + fill_width * fill_ch_len))
        return false;

    usize left_chars = 0;
    usize right_chars = 0;

    switch (format->alignment) {
    case Nst_FMT_ALIGN_AUTO:
    case Nst_FMT_ALIGN_LEFT:
        right_chars = fill_width;
        break;
    case Nst_FMT_ALIGN_RIGHT:
        left_chars = fill_width;
        break;
    case Nst_FMT_ALIGN_CENTER:
        left_chars = fill_width / 2;
        right_chars = fill_width - left_chars;
        break;
    }

    for (usize i = 0; i < left_chars; i++)
        Nst_buffer_append_str(buf, (i8 *)fill_ch, fill_ch_len);
    Nst_buffer_append_str(buf, str, str_len);
    for (usize i = 0; i < right_chars; i++)
        Nst_buffer_append_str(buf, (i8 *)fill_ch, fill_ch_len);
    return true;
}

/* =========================== Integer formatting ========================== */

static bool fmt_int(Nst_Buffer *buf, i64 val, Format *format)
{
    Nst_UNUSED(buf);
    Nst_UNUSED(val);
    Nst_UNUSED(format);
    Nst_set_type_error_c("formatting integers is not supported");
    return false;
}

/* ============================ Float formatting =========================== */

static bool fmt_float(Nst_Buffer *buf, f64 val, Format *format)
{
    Nst_UNUSED(buf);
    Nst_UNUSED(val);
    Nst_UNUSED(format);
    Nst_set_type_error_c("formatting floats is not supported");
    return false;
}

/* =========================== Boolean formatting ========================== */

static bool fmt_bool(Nst_Buffer *buf, bool val, Format *format)
{
    Nst_UNUSED(buf);
    Nst_UNUSED(val);
    Nst_UNUSED(format);
    Nst_set_type_error_c("formatting booleans is not supported");
    return false;
}

/* =========================== Pointer formatting ========================== */

static bool fmt_ptr(Nst_Buffer *buf, void *val, Format *format)
{
    Nst_UNUSED(buf);
    Nst_UNUSED(val);
    Nst_UNUSED(format);
    Nst_set_type_error_c("formatting pointers is not supported");
    return false;
}

/* ============================ Char formatting ============================ */

static bool fmt_char(Nst_Buffer *buf, i8 val, Format *format)
{
    Nst_UNUSED(buf);
    Nst_UNUSED(val);
    Nst_UNUSED(format);
    Nst_set_type_error_c("formatting chars is not supported");
    return false;
}
