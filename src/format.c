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

typedef enum _Alignment {
    Nst_FMT_ALIGN_AUTO,
    Nst_FMT_ALIGN_LEFT,
    Nst_FMT_ALIGN_CENTER,
    Nst_FMT_ALIGN_RIGHT
} Alignment;

typedef enum _IntReprMode {
    Nst_FMT_INTR_BIN,
    Nst_FMT_INTR_OCT,
    Nst_FMT_INTR_DEC,
    Nst_FMT_INTR_HEX,
    Nst_FMT_INTR_UPPER_HEX
} IntReprMode;

typedef enum _AlignSign {
    Nst_FMT_SIGN_NO_SIGN,
    Nst_FMT_SIGN_SPACE,
    Nst_FMT_SIGN_PLUS
} AlignSign;

typedef enum _ReprMode {
    Nst_FMT_REPR_NO_REPR,
    Nst_FMT_REPR_SHALLOW,
    Nst_FMT_REPR_FULL,
    Nst_FMT_REPR_FULL_ASCII,
    Nst_FMT_REPR_SHALLOW_ASCII
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

static const i8 *nest_fmt_value(Nst_Buffer *buf, const i8 *fmt, va_list *args)
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
    ReprMode repr = Nst_FMT_REPR_NO_REPR;
    AlignSign sign = Nst_FMT_SIGN_NO_SIGN;
    IntReprMode mode = Nst_FMT_INTR_DEC;
    const i8 *thousand_sep = NULL;
    const i8 *fill_ch = NULL;
    i32 width = -1;
    i32 precision = -1;
    Alignment align = Nst_FMT_ALIGN_AUTO;

    fmt++;
    if (*fmt == '}')
        goto format_type;
    else if (*fmt != ':') {
        Nst_set_value_error_c("expected ':' in format string");
        return NULL;
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
            mode = Nst_FMT_INTR_BIN;
            break;
        case 'o':
            mode = Nst_FMT_INTR_OCT;
            break;
        case 'x':
            mode = Nst_FMT_INTR_HEX;
            break;
        case 'X':
            mode = Nst_FMT_INTR_UPPER_HEX;
            break;
        case ' ':
            sign = Nst_FMT_SIGN_SPACE;
            break;
        case '+':
            sign = Nst_FMT_SIGN_PLUS;
            break;
        case 'r':
            repr = Nst_FMT_REPR_FULL;
            break;
        case 'R':
            repr = Nst_FMT_REPR_SHALLOW;
            break;
        case 'a':
            repr = Nst_FMT_REPR_FULL_ASCII;
            break;
        case 'A':
            repr = Nst_FMT_REPR_SHALLOW_ASCII;
            break;
        case 'u':
            as_unsigned = true;
            break;
        case '\'': {
            thousand_sep = fmt + 1;
            fmt++;
            i32 ch_len = Nst_check_ext_utf8_bytes((u8 *)fmt, strnlen(fmt, 4));
            if (ch_len == -1) {
                Nst_set_value_error_c("the fill character is not valid UTF-8");
                return NULL;
            }
            fmt += ch_len - 1;
            break;
        }
        case '_': {
            fill_ch = fmt + 1;
            fmt++;
            i32 ch_len = Nst_check_ext_utf8_bytes((u8 *)fmt, strnlen(fmt, 4));
            if (ch_len == -1) {
                Nst_set_value_error_c(
                    "the thousand separator is not valid UTF-8");
                return NULL;
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

    if (*fmt > '0' && *fmt <= '9') {
        width = strtol(fmt, (i8 **)&fmt, 10);
        while (*fmt >= 0 && *fmt <= 9)
            fmt++;
    }
    else if (*fmt == '*') {
        width = va_arg(*args, i32);
        fmt++;
    }

    if (*fmt == '.') {
        fmt++;
        if ((*fmt < '0' || *fmt > '0') && *fmt != '*') {
            Nst_set_value_error_c("expected a number for precision in format");
            return false;
        }
        if (*fmt == '*') {
            precision = va_arg(*args, i32);
            while (*fmt >= 0 && *fmt <= 9)
                fmt++;
        } else {
            precision = strtol(fmt, (i8 **)&fmt, 10);
            fmt++;
        }
    }

    switch (*fmt) {
    case '<':
        align = Nst_FMT_ALIGN_LEFT;
        fmt++;
        break;
    case '>':
        align = Nst_FMT_ALIGN_RIGHT;
        fmt++;
        break;
    case '^':
        align = Nst_FMT_ALIGN_CENTER;
        fmt++;
        break;
    case '=':
        fill_zeroes = true;
        fmt++;
        break;
    }

    if (*fmt != '}') {
        Nst_set_value_error_c("invalid format string");
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
        i8 *str = va_arg(*args, i8 *);
        bool result = nest_fmt_string(
            buf,
            str, -1,
            exact_width, pref_suff, repr, fill_ch,
            width,
            align);
        if (!result)
            return NULL;
        return fmt;
    }
    case 'i':
        Nst_set_value_error_c("formatting for 'int' is not yet supported");
        return NULL;
    case 'l':
        Nst_set_value_error_c("formatting for 'long' is not yet supported");
        return NULL;
    case 'L':
        Nst_set_value_error_c("formatting for 'long long' is not yet supported");
        return NULL;
    case 'b':
        Nst_set_value_error_c("formatting for 'boolean' is not yet supported");
        return NULL;
    case 'u':
        Nst_set_value_error_c("formatting for 'isize' is not yet supported");
        return NULL;
    case 'c':
        Nst_set_value_error_c("formatting for 'char' is not yet supported");
        return NULL;
    case 'r':
    case 'f':
        Nst_set_value_error_c("formatting for 'float' is not yet supported");
        return NULL;
    case 'p':
        Nst_set_value_error_c("formatting for 'pointer' is not yet supported");
        return NULL;
    default:
        Nst_set_value_errorf("invalid type letter '%c' in format", *type);
        return NULL;
    }

format_nest_obj:
    Nst_set_value_error_c("formatting for Nest objects is not yet supported");
    return NULL;
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

    return double_count >= single_count;
}

static bool is_simple_char(u8 c)
{
    return c < 0x7f && c >= ' ' && c != '\\' && c != '\'' && c != '"';
}

static bool write_ascii_escape(Nst_Buffer *buf, u8 c, u8 esc_ap, ReprMode repr)
{
    if (repr != Nst_FMT_REPR_SHALLOW && repr != Nst_FMT_REPR_SHALLOW_ASCII) {
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
    u32 ch = Nst_ext_utf8_to_utf32(str);
    if (repr != Nst_FMT_REPR_FULL_ASCII && repr != Nst_FMT_REPR_SHALLOW_ASCII
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
    // repr is assumed to not be Nst_FMT_REPR_NO_REPR
    Nst_Buffer buf;
    if (!Nst_buffer_init(&buf, str_len))
        return NULL;

    bool escape_single_quotes = more_double_quotes(str, str_len);

    if (repr != Nst_FMT_REPR_SHALLOW && repr != Nst_FMT_REPR_SHALLOW_ASCII) {
        if (escape_single_quotes && !Nst_buffer_append_char(&buf, '\''))
            goto fail;
        else if (!escape_single_quotes && !Nst_buffer_append_char(&buf, '"'))
            goto fail;
    }

    Nst_StrObj str_ob = Nst_string_temp((i8 *)str, str_len);
    isize i = 0;

    while (i < (isize)str_len && i >= 0) {
        u8 c = str[i];
        if (is_simple_char(c)) {
            if (!Nst_buffer_append_char(&buf, (i8)c))
                goto fail;
        } else if (c < 0x80) {
            if (!write_ascii_escape(&buf, c, escape_single_quotes, repr))
                goto fail;
        } else if (!write_unicode_escape(&buf, str + i, str_len - i, repr))
            goto fail;
        _Nst_string_next_ch(&str_ob, &i, NULL);
    }

    if (i < 0)
        goto fail;

    if (repr != Nst_FMT_REPR_SHALLOW && repr != Nst_FMT_REPR_SHALLOW_ASCII) {
        if (escape_single_quotes && !Nst_buffer_append_char(&buf, '\''))
            goto fail;
        else if (!escape_single_quotes && !Nst_buffer_append_char(&buf, '"'))
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

    isize vaild = Nst_check_string_cp(
        Nst_cp(Nst_CP_EXT_UTF8),
        (void *)str,
        str_len);
    if (vaild != -1) {
        Nst_set_value_error_c(
            "Nst_fmt: the string to format is not valid UTF-8");
        return false;
    }

    if (width == -1 && repr == Nst_FMT_REPR_NO_REPR) {
        return Nst_buffer_append_str(buf, str, str_len);
    }

    i8 *final_str;
    usize final_str_len;
    if (repr != Nst_FMT_REPR_NO_REPR)
        final_str = repr_string((u8 *)str, str_len, &final_str_len, repr);
    else {
        final_str = str;
        final_str_len = str_len;
    }
    if (final_str == NULL)
        return false;

    if (width == -1) {
        if (!Nst_buffer_append_str(buf, final_str, final_str_len))
            goto fail;
        Nst_free(final_str);
        return true;
    }

    usize str_ch_len = Nst_string_utf8_char_len((u8 *)final_str, final_str_len);

    if (exact_width && (isize)str_ch_len > width) {
        str_ch_len = width;
        usize curr_len = 0;
        for (usize i = 0; i < final_str_len; i++) {
            i8 ch = final_str[i];
            if ((ch & 0b11000000) == 0b10000000)
                continue;
            curr_len += 1;
            if ((isize)curr_len > width) {
                final_str_len = i;
                break;
            }
        }
    } else if ((isize)str_ch_len > width)
        width = (i32)str_ch_len;

    usize fill_ch_size;
    if (fill_ch == NULL) {
        fill_ch_size = 1;
        fill_ch = " ";
    } else
        fill_ch_size = Nst_check_ext_utf8_bytes((u8 *)fill_ch, 4);

    usize fill_width = width - str_ch_len;

    if (!Nst_buffer_expand_by(buf, final_str_len + fill_width * fill_ch_size))
        goto fail;

    usize left_chars = 0;
    usize right_chars = 0;

    switch (align) {
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
        Nst_buffer_append_str(buf, (i8 *)fill_ch, fill_ch_size);
    Nst_buffer_append_str(buf, final_str, final_str_len);
    for (usize i = 0; i < right_chars; i++)
        Nst_buffer_append_str(buf, (i8 *)fill_ch, fill_ch_size);

    (void)preff_suff;

    return true;

fail:
    if (repr != Nst_FMT_REPR_NO_REPR)
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
    va_list args_cpy;
    va_copy(args_cpy, args);

    usize fmtlen = len != NULL ? *len : strlen(fmt);

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
                if (!Nst_buffer_append_char(&buf, '}'))
                    goto failure;
                i++;
                continue;
            }
            const i8 *fmt_end = nest_fmt_value(&buf, fmt + i, &args_cpy);
            if (fmt_end == NULL)
                goto failure;
            i += fmt_end - fmt - i - 1;
        }
    }

    if (len != NULL)
        *len = buf.len;
    return buf.data;

failure:
    Nst_buffer_destroy(&buf);
    return NULL;
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
