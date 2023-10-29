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

#ifdef ENABLE_NEST_FMT

typedef enum _Alignment {
    AUTO,
    LEFT,
    CENTER,
    RIGHT
} Alignment;

typedef enum _IntReprMode {
    BIN,
    OCT,
    DEC,
    HEX,
    UHEX
} IntReprMode;

typedef enum _AlignSign {
    NO_SIGN,
    SPACE,
    PLUS
} AlignSign;

typedef enum _ReprMode {
    NO_REPR,
    SHALLOW,
    FULL,
    FULL_ASCII,
    SHALLOW_ASCII
} ReprMode;

typedef enum _PrefSuffMode {
    NO_PREF_OR_SUFF,
    PREF_SUFF_LOWER,
    PREF_SUFF_UPPER
} PrefSuffMode;

typedef enum _DblRepr {
    MIN_REPR,
    DEC_REPR,
    STD_FORM
} DblRepr;

static bool nest_fmt_string(Nst_Buffer *buf, i8 *str, isize str_len,
                            bool exact_width, PrefSuffMode preff_suff,
                            ReprMode repr, const i8 *fill_ch, i32 width,
                            Alignment align);

static bool nest_fmt_value(Nst_Buffer *buf, const i8 *fmt, va_list args)
{
    bool nest_obj = false;
    const i8 *type;

    fmt++;
    if (*fmt == '#') {
        nest_obj = true;
        fmt++;
        type = fmt;
    } else
        type = fmt;

    bool normalize_neg_zero = false;
    bool exact_width = false;
    bool fill_zeroes = false;
    bool as_unsigned = false;
    bool pad_zeroes_precision = false;
    DblRepr dbl_repr = MIN_REPR;
    PrefSuffMode pref_suff = NO_PREF_OR_SUFF;
    ReprMode repr = NO_REPR;
    AlignSign sign = NO_SIGN;
    IntReprMode mode = DEC;
    const i8 *thousand_sep = NULL;
    const i8 *fill_ch = NULL;
    i32 width = -1;
    i32 precision = -1;
    Alignment align = AUTO;

    fmt++;
    if (*fmt == '}')
        goto format_type;
    else if (*fmt != ':') {
        Nst_set_value_error("expected ':' in format string");
        return false;
    }
    fmt++;

    while (true) {
        bool end_loop = false;
        switch (*fmt) {
        case 'z':
            normalize_neg_zero = true;
            break;
        case '0':
            pad_zeroes_precision = true;
            break;
        case 'f':
            dbl_repr = DEC_REPR;
            break;
        case 'e':
            dbl_repr = STD_FORM;
            break;
        case 'p':
            pref_suff = PREF_SUFF_LOWER;
            break;
        case 'P':
            pref_suff = PREF_SUFF_UPPER;
            break;
        case 'c':
            exact_width = true;
            break;
        case 'b':
            mode = BIN;
            break;
        case 'o':
            mode = OCT;
            break;
        case 'x':
            mode = HEX;
            break;
        case 'X':
            mode = UHEX;
            break;
        case ' ':
            sign = SPACE;
            break;
        case '+':
            sign = PLUS;
            break;
        case 'r':
            repr = FULL;
            break;
        case 'R':
            repr = SHALLOW;
            break;
        case 'a':
            repr = FULL_ASCII;
            break;
        case 'A':
            repr = SHALLOW_ASCII;
            break;
        case 'u':
            as_unsigned = true;
            break;
        case '\'': {
            thousand_sep = fmt + 1;
            fmt++;
            i32 ch_len = Nst_check_ext_utf8_bytes((u8 *)fmt, strnlen(fmt, 4));
            if (ch_len == -1) {
                Nst_set_value_error("the fill character is not valid UTF-8");
                return false;
            }
            fmt += ch_len - 1;
            break;
        }
        case '_': {
            fill_ch = fmt + 1;
            fmt++;
            i32 ch_len = Nst_check_ext_utf8_bytes((u8 *)fmt, strnlen(fmt, 4));
            if (ch_len == -1) {
                Nst_set_value_error("the thousand separator is not valid UTF-8");
                return false;
            }
            fmt += ch_len - 1;
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
        width = strtol(fmt, (i8 **)&fmt, 10);
    else if (*fmt == '*')
        width = va_arg(args, i32);

    if (*fmt == '.') {
        fmt++;
        if ((*fmt < '0' || *fmt > '0') && *fmt != '*') {
            Nst_set_value_error("expected a number for precision in format");
            return false;
        }
        if (*fmt == '*')
            precision = va_arg(args, i32);
        else
            precision = strtol(fmt, (i8 **)&fmt, 10);
    }

    switch (*fmt) {
    case '<':
        align = LEFT;
        break;
    case '>':
        align = RIGHT;
        break;
    case '^':
        align = CENTER;
        break;
    case '=':
        fill_zeroes = true;
        break;
    }

    if (*fmt != '}') {
        Nst_set_value_error("invalid format sting");
        return false;
    }

    (void)precision;
    (void)thousand_sep;
    (void)sign;
    (void)dbl_repr;
    (void)pad_zeroes_precision;
    (void)as_unsigned;
    (void)fill_zeroes;
    (void)normalize_neg_zero;
    (void)mode;

format_type:
    fmt++;

    if (nest_obj)
        goto format_nest_obj;

    switch (*type) {
    case 's': {
        i8 *str = va_arg(args, i8 *);
        return nest_fmt_string(
            buf,
            str, -1,
            exact_width, pref_suff, repr, fill_ch,
            width,
            align);
    }
    case 'i':
        Nst_set_value_error("formatting for 'int' is not yet supported");
        return false;
    case 'l':
        Nst_set_value_error("formatting for 'long' is not yet supported");
        return false;
    case 'L':
        Nst_set_value_error("formatting for 'long long' is not yet supported");
        return false;
    case 'b':
        Nst_set_value_error("formatting for 'boolean' is not yet supported");
        return false;
    case 'u':
        Nst_set_value_error("formatting for 'isize' is not yet supported");
        return false;
    case 'c':
        Nst_set_value_error("formatting for 'char' is not yet supported");
        return false;
    case 'r':
    case 'f':
        Nst_set_value_error("formatting for 'float' is not yet supported");
        return false;
    case 'p':
        Nst_set_value_error("formatting for 'pointer' is not yet supported");
        return false;
    default:
        Nst_set_value_errorf("invalid type letter '%c' in format", *type);
        return false;
    }

format_nest_obj:
    Nst_set_value_error("formatting for Nest objects is not yet supported");
    return false;
}

static bool more_double_quotes(u8 *str, usize str_len)
{
    usize single_count = 0;
    usize double_count = 0;

    for (usize i = 0; i < str_len; i++) {
        if (str[i] == '\'')
            single_count++;
        else if (str[i] == '\"')
            double_count++;
    }

    return double_count > single_count;
}

static bool is_simple_char(u8 c)
{
    return c < 0x7f && c >= ' ' && c != '\\' && c != '\'' && c != '"';
}

static bool write_ascii_escape(Nst_Buffer *buf, u8 c, u8 esc_ap, ReprMode repr)
{
    if (repr != SHALLOW && repr != SHALLOW_ASCII) {
        if (c == '\'') {
            if (esc_ap)
                return Nst_buffer_append_c_str(buf, "\\'");
            return Nst_buffer_append_char(buf, '\'');
        } else if (c == '"') {
            if (!esc_ap)
                return Nst_buffer_append_c_str(buf, "\\\"");
            return Nst_buffer_append_char(buf, '"');
        } else if (c == '\\') {
            return Nst_buffer_append_c_str(buf, "\\\\");
        }
    } else if (c == '\'' || c == '"' || c == '\\')
        return Nst_buffer_append_char(buf, (i8)c);

    const i8 *hex_chars = "0123456789abcdef";

    switch (c) {
    case '\0': return Nst_buffer_append_c_str(buf, "\\0");
    case '\a': return Nst_buffer_append_c_str(buf, "\\a");
    case '\b': return Nst_buffer_append_c_str(buf, "\\b");
    case'\x1b':return Nst_buffer_append_c_str(buf, "\\e");
    case '\f': return Nst_buffer_append_c_str(buf, "\\f");
    case '\n': return Nst_buffer_append_c_str(buf, "\\n");
    case '\r': return Nst_buffer_append_c_str(buf, "\\r");
    case '\t': return Nst_buffer_append_c_str(buf, "\\t");
    case '\v': return Nst_buffer_append_c_str(buf, "\\v");
    default:
        if (!Nst_buffer_append_c_str(buf, "\\x"))
            return false;
        if (!Nst_buffer_append_char(buf, hex_chars[c >> 4]))
            return false;
        return Nst_buffer_append_char(buf, hex_chars[c & 0xf]);
    }
}

static bool write_unicode_escape(Nst_Buffer *buf, u8 *str, usize str_len,
                                 ReprMode repr)
{
    i32 ch_len = Nst_check_ext_utf8_bytes(str, str_len);
    if (ch_len == -1) {
        Nst_set_value_error("the string to format is not valid UTF-8");
        return false;
    }

    u32 ch = Nst_ext_utf8_to_utf32(str);
    if (repr != FULL_ASCII && repr != SHALLOW_ASCII
        && Nst_is_valid_cp(ch) && !Nst_is_non_character(ch))
    {
        if (!Nst_buffer_expand_by(buf, (usize)ch_len))
            return false;
        for (i32 i = 0; i < ch_len; i++)
            Nst_buffer_append_char(buf, str[i]);
        return true;
    }

    const i8 *hex_chars = "0123456789abcdef";

    if (!Nst_buffer_expand_by(buf, ch <= 0xffff ? 6 : 8))
        return false;
    Nst_buffer_append_char(buf, '\\');
    Nst_buffer_append_char(buf, ch <= 0xffff ? 'u' : 'U');
    for (usize i = 1, n = ch <= 0xffff ? 4 : 6; i <= n; i++) {
        Nst_buffer_append_char(buf, hex_chars[(ch >> ((n - i) * 4)) & 0xf]);
    }
    return true;
}

static i8 *repr_string(u8 *str, usize str_len, usize *out_len, ReprMode repr)
{
    // repr is assumed to not be NO_REPR
    Nst_Buffer buf;
    if (!Nst_buffer_init(&buf, str_len))
        return NULL;

    bool escape_single_quotes = more_double_quotes(str, str_len);

    for (usize i = 0; i < str_len; i++) {
        u8 c = str[i];
        if (is_simple_char(c)) {
            if (!Nst_buffer_append_char(&buf, (i8)c))
                goto fail;
        }

        if (c < 0x80) {
            if (!write_ascii_escape(&buf, c, escape_single_quotes, repr))
                goto fail;
            continue;
        }

        if (!write_unicode_escape(&buf, str + i, str_len - i, repr))
            goto fail;
    }

    *out_len = buf.len;
    return buf.data;

fail:
    Nst_buffer_destroy(&buf);
    return NULL;
}

static bool nest_fmt_string(Nst_Buffer *buf, i8 *str, isize str_len,
                            bool exact_width, PrefSuffMode preff_suff,
                            ReprMode repr, const i8 *fill_ch, i32 width,
                            Alignment align)
{
    if (str == NULL) {
        str = "(null)";
        str_len = 6;
    }

    if (str_len < 0)
        str_len = strlen(str);

    if (width == -1 && repr == NO_REPR) {
        Nst_StrObj temp = Nst_string_temp(str, str_len);
        return Nst_buffer_append(buf, &temp);
    }

    i8 *final_str;
    usize final_str_len;
    if (repr != NO_REPR)
        final_str = repr_string((u8 *)str, str_len, &final_str_len, repr);
    else {
        final_str = str;
        final_str_len = str_len;
    }
    if (final_str == NULL)
        return false;

    i32 fill_ch_len =
        fill_ch == NULL ? 0 : Nst_check_ext_utf8_bytes((u8 *)fill_ch, 4);

    usize fill_width = (isize)final_str_len > width ? 0 : width - final_str_len;
    usize str_width;
    if (exact_width && width != -1)
        str_width = (isize)final_str_len > width ? (usize)width : final_str_len;
    else
        str_width = final_str_len;

    if (!Nst_buffer_expand_by(buf, str_width + fill_width * fill_ch_len))
        goto fail;

    (void)align;
    (void)preff_suff;

fail:
    if (repr != NO_REPR)
        Nst_free(final_str);
    return false;
}

i8 *Nst_fmt(const i8 *fmt, usize *len, ...)
{
    va_list args;
    va_start(args, len);
    return Nst_vfmt(fmt, len, args);
}

i8 *Nst_vfmt(const i8 *fmt, usize *len, va_list args)
{
    Nst_Buffer buf;
    // Nst_SizedBuffer args_list;

    usize fmtlen = strlen(fmt);
    if (!Nst_buffer_init(&buf, fmtlen))
        return NULL;

    for (usize i = 0; i < fmtlen; i++) {
        if (fmt[i] != '{' && fmt[i] != '}') {
            if (!Nst_buffer_append_char(&buf, fmt[i]))
                goto failure;
        }

        if (fmt[i] == '}') {
            if (fmt[i + 1] != '}') {
                Nst_set_value_error("found single '}' in format string");
                goto failure;
            }
            if (!Nst_buffer_append_char(&buf, '}'))
                goto failure;
            i++;
            continue;
        }

        if (fmt[i] == '{') {
            if (fmt[i + 1] == '{') {
                if (!Nst_buffer_append_char(&buf, '}'))
                    goto failure;
                i++;
                continue;
            }

            if (!nest_fmt_value(&buf, fmt + i, args))
                goto failure;
        }
    }

    if (len != NULL)
        *len = buf.len;
    return buf.data;

failure:
    Nst_buffer_destroy(&buf);
    return NULL;
}

#endif // !ENABLE_NEST_FMT

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
