#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include "format.h"
#include "global_consts.h"
#include "mem.h"
#include "str_builder.h"
#include "file.h"
#include "dtoa.h"
#include "obj_ops.h"
#include "unicode_db.h"
#include "interpreter.h"

#define MIN(a, b) ((b) < (a) ? (b) : (a))
#define MAX(a, b) ((b) > (a) ? (b) : (a))

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

isize Nst_fprint(Nst_Obj *f, const i8 *buf)
{
    Nst_assert(f->type == Nst_t.IOFile);
    usize len = strlen(buf);
    usize count;
    if (Nst_fwrite((i8 *)buf, (usize)len, &count, f) < 0)
        return -1;
    return count;
}

isize Nst_fprintln(Nst_Obj *f, const i8 *buf)
{
    Nst_assert(f->type == Nst_t.IOFile);
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

isize Nst_fprintf(Nst_Obj *f, Nst_WIN_FMT const i8 *fmt, ...)
{
    Nst_assert(f->type == Nst_t.IOFile);
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
    va_list args_copy;
    va_copy(args_copy, args);
    isize buf_size = vsnprintf(NULL, 0, fmt, args_copy) + 1;
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

    Nst_Obj *str = Nst_str_new(buf, len, true);
    if (str == NULL) {
        Nst_error_clear();
        Nst_free(buf);
    }
    return str;
}

isize Nst_vfprintf(Nst_Obj *f, const i8 *fmt, va_list args)
{
    Nst_assert(f->type == Nst_t.IOFile);
    if (Nst_IOF_IS_CLOSED(f)) {
        va_end(args);
        return -2;
    }

    va_list args_copy;
    va_copy(args_copy, args);
    isize buf_size = vsnprintf(NULL, 0, fmt, args_copy) + 1;
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

typedef enum _IntRepr {
    Nst_FMT_INT_BIN,
    Nst_FMT_INT_OCT,
    Nst_FMT_INT_DEC,
    Nst_FMT_INT_HEX,
    Nst_FMT_INT_HEX_UPPER
} IntRepr;

#define INCLUDE_POINT 0b1000

typedef enum _DoubleRepr {
    Nst_FMT_DBL_GEN_REPR = 0,
    Nst_FMT_DBL_DEC_REPR = 1,
    Nst_FMT_DBL_STD_FORM = 2,
    Nst_FMT_DBL_GEN_REPR_P = Nst_FMT_DBL_GEN_REPR | INCLUDE_POINT,
    Nst_FMT_DBL_DEC_REPR_P = Nst_FMT_DBL_DEC_REPR | INCLUDE_POINT,
    Nst_FMT_DBL_STD_FORM_P = Nst_FMT_DBL_STD_FORM | INCLUDE_POINT
} DoubleRepr;

#define REPR_FULL 0b100
#define REPR_ASCII 0b10

typedef enum _StrRepr {
    Nst_FMT_STR_NO_REPR       = 0,
    Nst_FMT_STR_SHALLOW       = 1,
    Nst_FMT_STR_SHALLOW_ASCII = 1 | REPR_ASCII,
    Nst_FMT_STR_FULL          = 1 | REPR_FULL,
    Nst_FMT_STR_FULL_ASCII    = 1 | REPR_ASCII | REPR_FULL
} StrRepr;

typedef enum _Sign {
    Nst_FMT_SIGN_NONE,
    Nst_FMT_SIGN_SPACE,
    Nst_FMT_SIGN_PLUS
} Sign;

typedef enum _Alignment {
    Nst_FMT_ALIGN_AUTO,
    Nst_FMT_ALIGN_LEFT,
    Nst_FMT_ALIGN_CENTER,
    Nst_FMT_ALIGN_RIGHT
} Alignment;

typedef enum _PrefSuff {
    Nst_FMT_PREF_SUFF_NONE,
    Nst_FMT_PREF_SUFF_LOWER,
    Nst_FMT_PREF_SUFF_UPPER
} PrefSuff;

typedef enum _FmtValueType {
    Nst_VALUES_VA_ARGS,
    Nst_VALUES_ARR
} FmtValueType;

typedef struct _FmtValues {
    FmtValueType type;
    union {
        va_list *va;
        struct {
            Nst_Obj *obj;
            usize i;
        } arr;
    } v;
} FmtValues;

void fmt_values_init_va_args(FmtValues *values, va_list *args)
{
    values->type = Nst_VALUES_VA_ARGS;
    values->v.va = args;
}

void fmt_values_init_sequence(FmtValues *values, Nst_Obj *obj)
{
    values->type = Nst_VALUES_ARR;
    values->v.arr.obj = obj;
    values->v.arr.i = 0;
}

#define fmt_values_get_va_arg(fmt_vals, type)                                 \
    va_arg(*((fmt_vals)->v.va), type)

static Nst_Obj *fmt_values_get_obj(FmtValues *values)
{
    if (Nst_seq_len(values->v.arr.obj) <= values->v.arr.i)
        return NULL;
    return Nst_seq_getnf(values->v.arr.obj, values->v.arr.i++);
}

typedef struct _Format {
    bool normalize_neg_zero;
    bool cut;
    bool as_unsigned;
    bool pad_zeroes_precision;
    Sign sign;
    Alignment alignment;
    PrefSuff pref_suff;
    DoubleRepr double_repr;
    IntRepr int_repr;
    StrRepr str_repr;
    u8 separator[4];
    u8 fill_ch[4];
    i32 width;
    i32 precision;
    i32 separator_width;
} Format;

static void format_init(Format *format)
{
    format->normalize_neg_zero = false;
    format->cut = false;
    format->as_unsigned = false;
    format->pad_zeroes_precision = false;
    format->sign = Nst_FMT_SIGN_NONE;
    format->alignment = Nst_FMT_ALIGN_AUTO;
    format->pref_suff = Nst_FMT_PREF_SUFF_NONE;
    format->double_repr = Nst_FMT_DBL_GEN_REPR;
    format->int_repr = Nst_FMT_INT_DEC;
    format->str_repr = Nst_FMT_STR_NO_REPR;
    memset(format->separator, 255, 4);
    memset(format->fill_ch, 255, 4);
    format->width = -1;
    format->precision = -1;
    format->separator_width = -1;
}

static const i8 *format_set_separator(Format *format, const i8 *ch)
{
    memset(format->separator, 0, 4);
    i32 ch_len = Nst_check_utf8_bytes((u8 *)ch, Nst_ENCODING_MULTIBYTE_MAX_SIZE);
    memcpy(format->separator, ch, (usize)ch_len);
    return ch + ch_len;
}

static bool format_has_separator(Format *format)
{
    return format->separator[0] != 255;
}

static usize format_separator_len(Format *format)
{
    if (format->separator[1] == 0)
        return 1;
    else if (format->separator[2] == 0)
        return 2;
    else if (format->separator[3] == 0)
        return 3;
    return 4;
}

static const i8 *format_set_fill_ch(Format *format, const i8 *ch)
{
    memset(format->fill_ch, 0, 4);
    i32 ch_len = Nst_check_utf8_bytes((u8 *)ch, Nst_ENCODING_MULTIBYTE_MAX_SIZE);
    memcpy(format->fill_ch, ch, (usize)ch_len);
    return ch + ch_len;
}

static bool format_has_fill_ch(Format *format)
{
    return format->fill_ch[0] != 255;
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

static i8 *general_fmt(const i8 *fmt, usize fmt_len, usize *out_len,
                       FmtValues *values);

static const i8 *fmt_value(Nst_StrBuilder *sb, const i8 *fmt, FmtValues *vals);

static void fmt_cut(i8 *str, usize str_len, usize char_len, i8 **out_str,
                    usize *out_len, Format *format,
                    Alignment default_alignment);
static bool fmt_align(Nst_StrBuilder *sb, i8 *str, usize str_len,
                      usize char_str_len, Format *format,
                      Alignment default_alignment);
static bool fmt_sep_and_ndigits(Nst_StrBuilder *sb, i8 *digits, usize digits_len,
                                usize min_digits, i8 *sep, usize sep_len,
                                usize sep_width, bool pad_zeroes);
static bool fmt_align_or_cut(Nst_StrBuilder *sb, i8 *str, usize str_len,
                             usize char_str_len, Format *format,
                             Alignment default_alignment);

static bool  fmt_str(Nst_StrBuilder *sb, i8 *str, isize str_len, Format *format);
static isize fmt_str_repr(Nst_StrBuilder *sb, i8 *str, usize str_len,
                          Format *format);
static bool  fmt_str_more_double_quotes(u8 *str, usize str_len);
static isize fmt_str_ascii_escape(Nst_StrBuilder *sb, u8 c, Format *format);
static isize fmt_str_unicode_escape(Nst_StrBuilder *sb, u32 c, Format *format);

static bool fmt_general_int(Nst_StrBuilder *sb, u64 val, bool negative,
                            bool is_byte, Format *format);

static bool fmt_uint(Nst_StrBuilder *sb, u64 val, Format *format);
static bool fmt_uint_prefix(Nst_StrBuilder *sb, bool is_byte, Format *format);
static bool fmt_uint_digits(Nst_StrBuilder *sb, u64 val, Format *format);
static u8   fmt_uint_msb64(u64 val);
static bool fmt_uint_bin(Nst_StrBuilder *sb, u64 val);
static bool fmt_uint_oct(Nst_StrBuilder *sb, u64 val);
static bool fmt_uint_dec(Nst_StrBuilder *sb, u64 val);
static bool fmt_uint_hex(Nst_StrBuilder *sb, u64 val, bool upper);
static bool fmt_uint_sep_and_precision(Nst_StrBuilder *sb, i8 *digits,
                                       usize digits_len, Format *format);

static bool fmt_int(Nst_StrBuilder *sb, i64 val, Format *format);
static bool fmt_int_add_sign(Nst_StrBuilder *sb, bool negative, Format *format);

static bool fmt_byte(Nst_StrBuilder *sb, u8 val, Format *format);

static bool fmt_double(Nst_StrBuilder *sb, f64 val, Format *format);
static bool fmt_double_dec(Nst_StrBuilder *sb, f64 val, Format *format);
static bool fmt_double_dec_digits(Nst_StrBuilder *sb, i8 *digits,
                                  usize digits_len, int decpt, Format *format);
static bool fmt_double_std(Nst_StrBuilder *sb, f64 val, Format *format);
static bool fmt_double_std_digits(Nst_StrBuilder *sb, i8 *digits,
                                  usize digits_len, int decpt, Format *format);
static bool fmt_double_gen(Nst_StrBuilder *sb, f64 val, Format *format);

static bool fmt_bool(Nst_StrBuilder *sb, bool val, Format *format);
static bool fmt_ptr(Nst_StrBuilder *sb, void *val, Format *format);
static bool fmt_char(Nst_StrBuilder *sb, i8 val, Format *format);

i8 *Nst_fmt(const i8 *fmt, usize fmt_len, usize *out_len, ...)
{
    va_list args;
    va_start(args, out_len);
    return Nst_vfmt(fmt, fmt_len, out_len, args);
}

NstEXP Nst_Obj *NstC Nst_fmt_objs(Nst_Obj *fmt, Nst_Obj *values)
{
    FmtValues fmt_values;
    fmt_values_init_sequence(&fmt_values, values);
    usize str_len;
    i8 *str = general_fmt(
        (const i8 *)Nst_str_value(fmt),
        Nst_str_len(fmt),
        &str_len,
        &fmt_values);
    if (str == NULL)
        return NULL;
    if (fmt_values.v.arr.i != Nst_seq_len(fmt_values.v.arr.obj)) {
        Nst_set_value_error_c("too many values for the format placeholder");
        Nst_free(str);
        return NULL;
    }
    return Nst_str_new_allocated(str, str_len);
}

i8 *Nst_vfmt(const i8 *fmt, usize fmt_len, usize *out_len, va_list args)
{
    va_list args_cpy;
    va_copy(args_cpy, args);
    FmtValues values;
    fmt_values_init_va_args(&values, &args_cpy);
    return general_fmt(fmt, fmt_len, out_len, &values);
}

i8 *NstC Nst_repr(i8 *str, usize str_len, usize *out_len, bool shallow,
                  bool ascii)
{
    if (str_len == 0)
        str_len = strlen((const i8 *)str);
    if (out_len != NULL)
        *out_len = 0;

    Format format;
    format_init(&format);
    if (shallow && ascii)
        format.str_repr = Nst_FMT_STR_SHALLOW_ASCII;
    else if (shallow)
        format.str_repr = Nst_FMT_STR_SHALLOW;
    else if (ascii)
        format.str_repr = Nst_FMT_STR_FULL_ASCII;
    else
        format.str_repr = Nst_FMT_STR_FULL;

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, str_len))
        return NULL;
    isize char_len = fmt_str_repr(&sb, str, str_len, &format);
    if (char_len < 0) {
        Nst_sb_destroy(&sb);
        return NULL;
    }
    if (out_len != NULL)
        *out_len = sb.len;
    return sb.value;
}

static i8 *general_fmt(const i8 *fmt, usize fmt_len, usize *out_len,
                       FmtValues *values)
{
    Nst_StrBuilder sb;
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

    isize encoding_error = Nst_encoding_check(
        Nst_encoding(Nst_EID_EXT_UTF8),
        (void *)fmt,
        fmtlen);
    if (encoding_error != -1) {
        Nst_set_value_error_c("`fmt` is not valid UTF-8");
        return false;
    }

    if (!Nst_sb_init(&sb, fmtlen))
        return NULL;

    for (usize i = 0; i < fmtlen; i++) {
        if (fmt[i] != '{' && fmt[i] != '}') {
            if (!Nst_sb_push_char(&sb, fmt[i]))
                goto failure;
        }

        if (fmt[i] == '}') {
            if (fmt[i + 1] != '}') {
                Nst_set_value_error_c("found single '}' in format string");
                goto failure;
            }
            if (!Nst_sb_push_char(&sb, '}'))
                goto failure;
            i++;
            continue;
        }

        if (fmt[i] == '{') {
            if (fmt[i + 1] == '{') {
                if (!Nst_sb_push_char(&sb, '{'))
                    goto failure;
                i++;
                continue;
            }
            const i8 *fmt_end = fmt_value(&sb, fmt + i, values);
            if (fmt_end == NULL)
                goto failure;
            i += fmt_end - fmt - i - 1;
        }
    }

    if (out_len != NULL)
        *out_len = sb.len;
    return sb.value;

failure:
    Nst_sb_destroy(&sb);
    return NULL;
}

static const i8 *parse_format(const i8 *fmt, Format *format)
{
    while (true) {
        bool end_loop = false;
        switch (*fmt) {
        case '0':
            format->pad_zeroes_precision = true;
            break;
        case 'g':
            format->double_repr = Nst_FMT_DBL_GEN_REPR;
            break;
        case 'G':
            format->double_repr = Nst_FMT_DBL_GEN_REPR_P;
            break;
        case 'f':
            format->double_repr = Nst_FMT_DBL_DEC_REPR;
            break;
        case 'F':
            format->double_repr = Nst_FMT_DBL_DEC_REPR_P;
            break;
        case 'e':
            format->double_repr = Nst_FMT_DBL_STD_FORM;
            break;
        case 'E':
            format->double_repr = Nst_FMT_DBL_STD_FORM_P;
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
            format->int_repr = Nst_FMT_INT_BIN;
            break;
        case 'o':
            format->int_repr = Nst_FMT_INT_OCT;
            break;
        case 'x':
            format->int_repr = Nst_FMT_INT_HEX;
            break;
        case 'X':
            format->int_repr = Nst_FMT_INT_HEX_UPPER;
            break;
        case ' ':
            format->sign = Nst_FMT_SIGN_SPACE;
            break;
        case '+':
            format->sign = Nst_FMT_SIGN_PLUS;
            break;
        case 'r':
            format->str_repr = Nst_FMT_STR_FULL;
            break;
        case 'R':
            format->str_repr = Nst_FMT_STR_SHALLOW;
            break;
        case 'a':
            format->str_repr = Nst_FMT_STR_FULL_ASCII;
            break;
        case 'A':
            format->str_repr = Nst_FMT_STR_SHALLOW_ASCII;
            break;
        case 'u':
            format->as_unsigned = true;
            break;
        case '\'': {
                fmt = format_set_separator(format, fmt + 1);
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
        format->width = -2;
        fmt++;
    }

    if (*fmt == '.') {
        fmt++;
        if ((*fmt < '0' || *fmt > '9') && *fmt != '*') {
            Nst_set_value_error_c("expected a number for precision in format");
            return NULL;
        }
        if (*fmt == '*') {
            format->precision = -2;
            fmt++;
        } else
            format->precision = strtol(fmt, (i8 **)&fmt, 10);
    }

    if (*fmt == ',') {
        fmt++;
        if ((*fmt < '0' || *fmt > '9') && *fmt != '*') {
            Nst_set_value_error_c(
                "expected a number for separator width in format");
            return NULL;
        }
        if (*fmt == '*') {
            format->separator_width = -2;
            fmt++;
        } else
            format->separator_width = strtol(fmt, (i8 **)&fmt, 10);
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

static bool set_fmt_field(FmtValues *values, const i8 *field_name, i32 *field)
{
    if (*field != -2)
        return true;

    Nst_Obj *obj = fmt_values_get_obj(values);
    if (obj == NULL) {
        Nst_set_value_error_c("not enough values for the format placeholder");
        return false;
    }
    if (obj->type != Nst_t.Int) {
        Nst_set_type_errorf("the value for '%s' is not an Int", field_name);
        return false;
    }
    i64 val = AS_INT(obj);
    if (val > INT32_MAX) {
        Nst_set_value_errorf("the value for '%s' is too big", field_name);
        return false;
    } else if (val < INT32_MIN)
        *field = -1;
    else
        *field = (i32)val;
    return true;
}

static bool add_format_values(Format *format, FmtValues *values)
{
    if (values->type == Nst_VALUES_VA_ARGS) {
        if (format->width == -2)
            format->width = fmt_values_get_va_arg(values, i32);
        if (format->precision == -2)
            format->precision = fmt_values_get_va_arg(values, i32);
        if (format->separator_width == -2)
            format->separator_width = fmt_values_get_va_arg(values, i32);
        return true;
    }
    if (!set_fmt_field(values, "width", &format->width))
        return false;
    if (!set_fmt_field(values, "precision", &format->precision))
        return false;
    if (!set_fmt_field(values, "separator width", &format->separator_width))
        return false;
    return true;
}

static const i8 *fmt_value(Nst_StrBuilder *sb, const i8 *fmt, FmtValues *vals)
{
    const i8 *type = NULL;
    Nst_Obj *obj = NULL;
    bool result = true;

    Format format;
    format_init(&format);

    fmt++;
    if (vals->type == Nst_VALUES_VA_ARGS) {
        type = fmt++;
        if (*fmt == '}') {
            fmt++;
            goto format_type;
        } else if (*fmt != ':') {
            Nst_set_value_error_c("expected ':' in format string");
            return NULL;
        }
        fmt++;
    } else {
        obj = fmt_values_get_obj(vals);
        if (obj == NULL) {
            Nst_set_value_error_c(
                "not enough values for the format placeholder");
            return NULL;
        }
        if (!add_format_values(&format, vals))
            return NULL;
        if (*fmt == '}') {
            fmt++;
            goto format_type;
        }
    }

    fmt = parse_format(fmt, &format);
    if (fmt == NULL)
        return NULL;

    if (*fmt != '}') {
        Nst_set_value_error_c("invalid format string");
        return NULL;
    }
    fmt++;

format_type:
    if (obj != NULL) {
        if (obj->type == Nst_t.Str)
            result = fmt_str(sb, Nst_str_value(obj), Nst_str_len(obj), &format);
        else if (obj->type == Nst_t.Int) {
            if (format.as_unsigned)
                result = fmt_uint(sb, (u64)AS_INT(obj), &format);
            else
                result = fmt_int(sb, AS_INT(obj), &format);
        } else if (obj->type == Nst_t.Real)
            result = fmt_double(sb, AS_REAL(obj), &format);
        else if (obj->type == Nst_t.Bool)
            result = fmt_bool(sb, AS_BOOL(obj), &format);
        else if (obj->type == Nst_t.Byte)
            result = fmt_byte(sb, AS_BYTE(obj), &format);
        else {
            Nst_Obj *casted_obj = Nst_obj_cast(obj, Nst_t.Str);
            if (casted_obj == NULL)
                return NULL;
            result = fmt_str(
                sb,
                Nst_str_value(casted_obj),
                Nst_str_len(casted_obj),
                &format);
            Nst_dec_ref(casted_obj);
        }
        goto end;
    }

    switch (*type) {
    case 's': {
        i8 *str = fmt_values_get_va_arg(vals, i8 *);
        add_format_values(&format, vals);
        result = fmt_str(sb, str, -1, &format);
        break;
    }
    case 'i':
        if (format.as_unsigned) {
            uint val = fmt_values_get_va_arg(vals, uint);
            add_format_values(&format, vals);
            result = fmt_uint(sb, (u64)val, &format);
        } else {
            int val = fmt_values_get_va_arg(vals, int);
            add_format_values(&format, vals);
            result = fmt_int(sb, (i64)val, &format);
        }
        break;
    case 'l':
        if (format.as_unsigned) {
            u32 val = fmt_values_get_va_arg(vals, u32);
            add_format_values(&format, vals);
            result = fmt_uint(sb, (u64)val, &format);
        } else {
            i32 val = fmt_values_get_va_arg(vals, i32);
            add_format_values(&format, vals);
            result = fmt_int(sb, (i64)val, &format);
        }
        break;
    case 'L':
        if (format.as_unsigned) {
            u64 val = fmt_values_get_va_arg(vals, u64);
            add_format_values(&format, vals);
            result = fmt_uint(sb, val, &format);
        } else {
            i64 val = fmt_values_get_va_arg(vals, i64);
            add_format_values(&format, vals);
            result = fmt_int(sb, val, &format);
        }
        break;
    case 'z':
        if (format.as_unsigned) {
            usize val = fmt_values_get_va_arg(vals, usize);
            add_format_values(&format, vals);
            result = fmt_uint(sb, (u64)val, &format);
        } else {
            isize val = fmt_values_get_va_arg(vals, isize);
            add_format_values(&format, vals);
            result = fmt_int(sb, (i64)val, &format);
        }
        break;
    case 'b': {
        int val = fmt_values_get_va_arg(vals, int);
        add_format_values(&format, vals);
        result = fmt_bool(sb, (bool)val, &format);
        break;
    }
    case 'B': {
        int val = fmt_values_get_va_arg(vals, int);
        add_format_values(&format, vals);
        result = fmt_byte(sb, (u8)val, &format);
        break;
    }
    case 'c': {
        int val = fmt_values_get_va_arg(vals, int);
        add_format_values(&format, vals);
        result = fmt_char(sb, (i8)val, &format);
        break;
    }
    case 'f': {
        f64 val = fmt_values_get_va_arg(vals, f64);
        add_format_values(&format, vals);
        result = fmt_double(sb, val, &format);
        break;
    }
    case 'p': {
        void *ptr = fmt_values_get_va_arg(vals, void *);
        add_format_values(&format, vals);
        result = fmt_ptr(sb, ptr, &format);
        break;
    }
    default:
        Nst_set_value_errorf("invalid type letter '%c' in format", *type);
        return NULL;
    }

end:
    if (!result)
        return NULL;
    return fmt;
}

/* Cuts a string keeping it left-aligned. */
static void fmt_cut_left(i8 *str, usize str_len, usize char_len, i8 **out_str,
                         usize *out_len, usize width)
{
    Nst_UNUSED(char_len);
    *out_str = str;
    *out_len = str_len;

    Nst_StrView sv = Nst_sv_new(str, str_len);
    usize count = 0;

    for (isize i = Nst_sv_next(sv, -1, NULL);
         i >= 0;
         i = Nst_sv_next(sv, i, NULL))
    {
        if (count == width) {
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
    Nst_StrView sv = Nst_sv_new(str, str_len);

    for (isize i = Nst_sv_next(sv, -1, NULL);
         i >= 0;
         i = Nst_sv_next(sv, i, NULL))
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
    fmt_cut_right(str, str_len, char_len, &str, &str_len, width + right_chars);
    fmt_cut_left(str, str_len, char_len - left_chars, out_str, out_len, width);
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

static bool fmt_align(Nst_StrBuilder *sb, i8 *str, usize str_len,
                      usize char_str_len, Format *format,
                      Alignment default_alignment)
{
    Alignment alignment = format->alignment;
    if (alignment == Nst_FMT_ALIGN_AUTO)
        alignment = default_alignment;

    i8 *fill_ch;
    usize fill_ch_len;
    if (format_has_fill_ch(format)) {
        fill_ch = (i8 *)format->fill_ch;
        fill_ch_len = format_fill_ch_len(format);
    } else {
        fill_ch = " ";
        fill_ch_len = 1;
    }

    usize fill_width = format->width - char_str_len;

    if (!Nst_sb_reserve(sb, str_len + fill_width * fill_ch_len))
        return false;

    usize left_chars = 0;
    usize right_chars = 0;

    switch (alignment) {
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
    case Nst_FMT_ALIGN_AUTO:
        break;
    }

    for (usize i = 0; i < left_chars; i++)
        Nst_sb_push(sb, fill_ch, fill_ch_len);
    Nst_sb_push(sb, str, str_len);
    for (usize i = 0; i < right_chars; i++)
        Nst_sb_push(sb, fill_ch, fill_ch_len);
    return true;
}

static bool fmt_sep_and_ndigits(Nst_StrBuilder *sb, i8 *digits, usize digits_len,
                                usize min_digits, i8 *sep, usize sep_len,
                                usize sep_width, bool pad_zeroes)
{
    if (sep == NULL)
        sep_len = 0;
    usize digit_count = MAX(min_digits, digits_len);
    if (sep_width == 0)
        sep_width = 3;
    // guarantees always enough bytes (sometimes it's a little more)
    bool result = Nst_sb_reserve(
        sb,
        digit_count + sep_len * (digit_count / sep_width));
    if (!result)
        return false;

    // add precision
    u8 fill_digit = pad_zeroes ? '0' : ' ';
    usize precision_digits = digit_count - digits_len;
    for (usize i = 0; i < digit_count - digits_len; i++) {
        Nst_sb_push_char(sb, fill_digit);
        if (sep_len != 0 && (digit_count - i - 1) % sep_width == 0) {
            if (fill_digit == ' ')
                Nst_sb_push_char(sb, ' ');
            else
                Nst_sb_push(sb, sep, sep_len);
        }
    }
    // add digits
    for (usize i = precision_digits; i < digit_count; i++) {
        Nst_sb_push_char(sb, digits[i - precision_digits]);
        if (sep_len != 0
            && i + 1 != digit_count
            && (digit_count - i - 1) % sep_width == 0)
        {
            Nst_sb_push(sb, sep, sep_len);
        }
    }
    return true;
}

static bool fmt_align_or_cut(Nst_StrBuilder *sb, i8 *str, usize str_len,
                             usize char_str_len, Format *format,
                             Alignment default_alignment)
{
    if (format->width < 0)
        ;
    else if (char_str_len < (usize)format->width) {
        return fmt_align(
            sb, str,
            str_len, char_str_len,
            format, default_alignment);
    } else if (char_str_len > (usize)format->width && format->cut) {
        fmt_cut(
            str, str_len, char_str_len,
            &str, (usize *)&str_len,
            format, default_alignment);
    }
    return Nst_sb_push(sb, str, str_len);
}

/* =========================== String formatting =========================== */

static bool fmt_str(Nst_StrBuilder *sb, i8 *str, isize str_len, Format *format)
{
    bool result = true;
    if (str == NULL) {
        str = "(null)";
        str_len = 6;
    }

    if (str_len < 0)
        str_len = strlen(str);

    isize vaild = Nst_encoding_check(
        Nst_encoding(Nst_EID_EXT_UTF8),
        (void *)str,
        str_len);
    if (vaild != -1) {
        Nst_set_value_error_c("the string to format is not valid UTF-8");
        return false;
    }

    i32 precision = format->cut && format->width > 0
        ? -1
        : format->precision;

    Nst_StrBuilder stack_sb;
    Nst_StrBuilder *temp_sb;
    if (format->width <= 0 && precision < 0)
        temp_sb = sb;
    else {
        if (!Nst_sb_init(&stack_sb, str_len))
            return false;
        temp_sb = &stack_sb;
    }

    isize char_str_len = fmt_str_repr(temp_sb, str, str_len, format);
    if (char_str_len < 0) {
        result = false;
        goto finish;
    }
    if (format->width <= 0 && precision < 0)
        goto finish; // the buffer was already appended in fmt_str_repr

    // from here until finish we are sure that temp_buf is a new buffer and not
    // a reference to buf

    if (precision >= 0 && precision < char_str_len) {
        fmt_cut_left(
            temp_sb->value, temp_sb->len, char_str_len,
            &str, &temp_sb->len,
            precision);
        char_str_len = precision + 3;
        result = Nst_sb_push(temp_sb, "...", 3);
        if (!result)
            goto finish;
    }

    result = fmt_align_or_cut(
        sb, temp_sb->value,
        temp_sb->len, char_str_len,
        format, Nst_FMT_ALIGN_LEFT);

finish:
    if (temp_sb != sb)
        Nst_sb_destroy(temp_sb);
    return result;
}

static isize fmt_str_repr(Nst_StrBuilder *sb, i8 *str, usize str_len,
                          Format *format)
{
    // Not always the correct size but reduces allocations
    if (!Nst_sb_reserve(sb, str_len))
        return -1;

    StrRepr repr = format->str_repr;
    if (repr == Nst_FMT_STR_NO_REPR) {
        Nst_sb_push(sb, str, str_len);
        return Nst_encoding_utf8_char_len((u8 *)str, str_len);
    }

    usize tot_char_len = 0;
    bool escape_single_quotes = false;
    bool escape_double_quotes = false;
    if (repr & REPR_FULL) {
        escape_single_quotes = fmt_str_more_double_quotes((u8 *)str, str_len);
        escape_double_quotes = !escape_single_quotes;
        if (!Nst_sb_push_char(sb, escape_single_quotes ? '\'' : '"'))
            return -1;
        tot_char_len += 1;
    }

    Nst_StrView sv = Nst_sv_new(str, str_len);
    u32 c = 0;
    for (isize i = Nst_sv_next(sv, -1, &c);
         i >= 0;
         i = Nst_sv_next(sv, i, &c))
    {
        if (c == '"' && escape_double_quotes) {
            if (!Nst_sb_push(sb, "\\\"", 2))
                return -1;
            tot_char_len += 2;
        } else if (c == '\'' && escape_single_quotes) {
            if (!Nst_sb_push(sb, "\\'", 2))
                return -1;
            tot_char_len += 2;
        } else if (c < 0x7f && c >= 0x20 && c != '\\') {
            if (!Nst_sb_push_char(sb, (i8)c))
                return -1;
            tot_char_len += 1;
        } else if (c <= 0x9f) {
            isize char_len = fmt_str_ascii_escape(sb, (u8)c, format);
            if (char_len < 0)
                return -1;
            tot_char_len += char_len;
        } else if (repr & REPR_ASCII) {
            isize char_len = fmt_str_unicode_escape(sb, c, format);
            if (char_len < 0)
                return -1;
            tot_char_len += char_len;
        } else {
            Nst_UnicodeChInfo info = Nst_unicode_get_ch_info((u32)c);
            if (info.flags & Nst_UCD_MASK_PRINTABLE) {
                if (!Nst_sb_push_cps(sb, &c, 1))
                    return -1;
                tot_char_len += 1;
            } else {
                isize char_len = fmt_str_unicode_escape(sb, c, format);
                if (char_len < 0)
                    return -1;
                tot_char_len += char_len;
            }
        }
    }

    if (repr & REPR_FULL) {
        if (!Nst_sb_push_char(sb, escape_single_quotes ? '\'' : '"'))
            return -1;
        tot_char_len += 1;
    }

    return tot_char_len;
}

static bool fmt_str_more_double_quotes(u8 *str, usize str_len)
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

static isize fmt_str_ascii_escape(Nst_StrBuilder *sb, u8 c, Format *format)
{
    const i8 *hex_chars = format->pref_suff == Nst_FMT_PREF_SUFF_UPPER
        ? "0123456789ABCDEF"
        : "0123456789abcdef";

    switch (c) {
    case '\0': Nst_sb_push(sb, "\\0", 2); return 2;
    case '\a': Nst_sb_push(sb, "\\a", 2); return 2;
    case '\b': Nst_sb_push(sb, "\\b", 2); return 2;
    case'\x1b':Nst_sb_push(sb, "\\e", 2); return 2;
    case '\f': Nst_sb_push(sb, "\\f", 2); return 2;
    case '\n': Nst_sb_push(sb, "\\n", 2); return 2;
    case '\r': Nst_sb_push(sb, "\\r", 2); return 2;
    case '\t': Nst_sb_push(sb, "\\t", 2); return 2;
    case '\v': Nst_sb_push(sb, "\\v", 2); return 2;
    case '\\': Nst_sb_push(sb, "\\\\",2); return 2;
    default:
        if (!Nst_sb_push(sb, "\\x", 2))
            return -1;
        if (!Nst_sb_push_char(sb, hex_chars[c >> 4]))
            return -1;
        if (!Nst_sb_push_char(sb, hex_chars[c & 0xf]))
            return -1;
        return 3;
    }
}

static isize fmt_str_unicode_escape(Nst_StrBuilder *sb, u32 c, Format *format)
{
    const i8 *hex_chars = format->pref_suff == Nst_FMT_PREF_SUFF_UPPER
        ? "0123456789ABCDEF"
        : "0123456789abcdef";

    if (!Nst_sb_reserve(sb, c <= 0xffff ? 6 : 8))
        return -1;
    Nst_sb_push_char(sb, '\\');
    Nst_sb_push_char(sb, c <= 0xffff ? 'u' : 'U');
    for (usize i = 1, n = c <= 0xffff ? 4 : 6; i <= n; i++)
        Nst_sb_push_char(sb, hex_chars[(c >> ((n - i) * 4)) & 0xf]);
    return c <= 0xffff ? 6 : 8;
}

/* ====================== Unsigned Integer formatting ====================== */

static bool fmt_general_int(Nst_StrBuilder *sb, u64 val, bool negative,
                            bool is_byte, Format *format)
{
    bool result = true;
    Nst_StrBuilder stack_sb, stack_digit_sb;
    Nst_StrBuilder *temp_sb, *digit_sb;
    if (format->width < 0)
        temp_sb = sb;
    else {
        if (!Nst_sb_init(&stack_sb, 10))
            return false;
        temp_sb = &stack_sb;
    }

    if (format->precision <= 0 && !format_has_separator(format))
        digit_sb = temp_sb;
    else {
        if (!Nst_sb_init(&stack_digit_sb, 10))
            return false;
        digit_sb = &stack_digit_sb;
    }

    result = fmt_int_add_sign(temp_sb, negative, format);
    if (!result)
        goto finish;

    if (!fmt_uint_prefix(temp_sb, is_byte, format)) {
        result = false;
        goto finish;
    }

    if (!fmt_uint_digits(digit_sb, val, format)) {
        result = false;
        goto finish;
    }

    if (format->precision <= 0 && !format_has_separator(format))
        goto suffix;

    result = fmt_uint_sep_and_precision(
        temp_sb,
        digit_sb->value, digit_sb->len,
        format);
    if (!result)
        goto finish;

suffix:
    if (is_byte
        && format->int_repr != Nst_FMT_INT_HEX
        && format->int_repr != Nst_FMT_INT_HEX_UPPER)
    {
        if (format->pref_suff == Nst_FMT_PREF_SUFF_LOWER)
            result = Nst_sb_push_char(temp_sb, 'b');
        else if (format->pref_suff == Nst_FMT_PREF_SUFF_UPPER)
            result = Nst_sb_push_char(temp_sb, 'B');
    }
    if (!result)
        goto finish;

    if (format->width < 0)
        goto finish;
    result = fmt_align_or_cut(
        sb,
        temp_sb->value,
        temp_sb->len,
        Nst_encoding_utf8_char_len((u8 *)temp_sb->value, temp_sb->len),
        format, Nst_FMT_ALIGN_RIGHT);

finish:
    if (temp_sb != sb)
        Nst_sb_destroy(temp_sb);
    if (digit_sb != temp_sb)
        Nst_sb_destroy(digit_sb);
    return result;
}

static bool fmt_uint(Nst_StrBuilder *sb, u64 val, Format *format)
{
    return fmt_general_int(sb, val, false, false, format);
}

static bool fmt_uint_prefix(Nst_StrBuilder *sb, bool is_byte, Format *format)
{
    if (format->pref_suff == Nst_FMT_PREF_SUFF_NONE)
        return true;
    const i8 *pref = NULL;
    bool pref_upper = format->pref_suff == Nst_FMT_PREF_SUFF_UPPER;
    switch (format->int_repr) {
    case Nst_FMT_INT_BIN:
        pref = pref_upper ? "0B" : "0b";
        break;
    case Nst_FMT_INT_OCT:
        pref = pref_upper ? "0O" : "0o";
        break;
    case Nst_FMT_INT_DEC:
        break;
    case Nst_FMT_INT_HEX:
    case Nst_FMT_INT_HEX_UPPER:
        if (is_byte)
            pref = pref_upper ? "0H" : "0h";
        else
            pref = pref_upper ? "0X" : "0x";
        break;
    }
    if (pref != NULL && !Nst_sb_push(sb, (i8 *)pref, 2))
        return false;
    return true;
}

static bool fmt_uint_digits(Nst_StrBuilder *sb, u64 val, Format *format)
{
    switch (format->int_repr) {
    case Nst_FMT_INT_BIN:
        return fmt_uint_bin(sb, val);
    case Nst_FMT_INT_OCT:
        return fmt_uint_oct(sb, val);
    case Nst_FMT_INT_DEC:
        return fmt_uint_dec(sb, val);
    case Nst_FMT_INT_HEX:
        return fmt_uint_hex(sb, val, false);
    case Nst_FMT_INT_HEX_UPPER:
        return fmt_uint_hex(sb, val, true);
    }
    return true;
}

static u8 fmt_uint_msb64(u64 val) {
    u8  k = 0;
    if (val > 0xFFFFFFFFu) { val >>= 32; k  = 32; }
    if (val > 0x0000FFFFu) { val >>= 16; k |= 16; }
    if (val > 0x000000FFu) { val >>= 8;  k |= 8;  }
    if (val > 0x0000000Fu) { val >>= 4;  k |= 4;  }
    if (val > 0x00000003u) { val >>= 2;  k |= 2;  }
    k |= (val & 2) >> 1;
    return k;
}

static bool fmt_uint_bin(Nst_StrBuilder *sb, u64 val)
{
    u8 msb = fmt_uint_msb64(val); // most significant bit
    if (!Nst_sb_reserve(sb, msb + 1))
        return false;
    for (i32 i = 0; i < msb + 1; i++) {
        sb->value[sb->len + msb - i] = val & 1 ? '1' : '0';
        val >>= 1;
    }
    sb->value[sb->len + msb + 1] = 0;
    sb->len += msb + 1;
    return true;
}

static bool fmt_uint_oct(Nst_StrBuilder *sb, u64 val)
{
    u8 msb = fmt_uint_msb64(val); // most significant bit
    u8 str_len = (msb / 3) + 1;
    if (!Nst_sb_reserve(sb, str_len))
        return false;
    for (i32 i = 0; i < str_len; i++) {
        i8 ch = (val & 0b111) + '0';
        sb->value[sb->len + str_len - i - 1] = ch;
        val >>= 3;
    }
    sb->value[sb->len + str_len] = 0;
    sb->len += str_len;
    return true;
}

static bool fmt_uint_dec(Nst_StrBuilder *sb, u64 val)
{
    usize initial_len = sb->len;
    if (val == 0)
        return Nst_sb_push_char(sb, '0');
    while (val != 0) {
        if (!Nst_sb_push_char(sb, (val % 10) + '0'))
            return false;
        val /= 10;
    }
    usize final_len = sb->len;
    // reverse the digits
    for (usize i = 0, n = (final_len - initial_len) / 2; i < n; i++) {
        i8 temp = sb->value[final_len - i - 1];
        sb->value[final_len - i - 1] = sb->value[initial_len + i];
        sb->value[initial_len + i] = temp;
    }
    return true;
}

static bool fmt_uint_hex(Nst_StrBuilder *sb, u64 val, bool upper)
{
    u8 msb = fmt_uint_msb64(val); // most significant bit
    u8 str_len = (msb / 4) + 1;
    if (!Nst_sb_reserve(sb, str_len))
        return false;
    const i8 *hex_chars = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    for (i32 i = 0; i < str_len; i++) {
        sb->value[sb->len + str_len - i - 1] = hex_chars[val & 0xF];
        val >>= 4;
    }
    sb->value[sb->len + str_len] = 0;
    sb->len += str_len;
    return true;
}

static bool fmt_uint_sep_and_precision(Nst_StrBuilder *sb, i8 *digits,
                                       usize digits_len, Format *format)
{
    i8 *sep = NULL;
    usize sep_len = 0;
    if (format_has_separator(format)) {
        sep = (i8 *)format->separator;
        sep_len = format_separator_len(format);
    }
    usize min_digits = format->precision < 0 ? 0 : (usize)format->precision;
    usize sep_width = format->separator_width;
    if (format->separator_width <= 0) {
        switch (format->int_repr) {
        case Nst_FMT_INT_BIN:
            sep_width = 8;
            break;
        case Nst_FMT_INT_HEX:
        case Nst_FMT_INT_HEX_UPPER:
            sep_width = 4;
            break;
        case Nst_FMT_INT_OCT:
        case Nst_FMT_INT_DEC:
        default:
            sep_width = 3;
            break;
        }
    }
    return fmt_sep_and_ndigits(
        sb,
        digits, digits_len, min_digits,
        sep, sep_len, sep_width,
        format->pad_zeroes_precision);
}

/* =========================== Integer formatting ========================== */

static bool fmt_int(Nst_StrBuilder *sb, i64 val, Format *format)
{
    return fmt_general_int(
        sb,
        val < 0 ? (u64)-val : (u64)val,
        val < 0,
        false,
        format);
}

static bool fmt_int_add_sign(Nst_StrBuilder *sb, bool negative, Format *format)
{
    if (negative)
        return Nst_sb_push_char(sb, '-');
    else if (format->sign == Nst_FMT_SIGN_PLUS)
        return Nst_sb_push_char(sb, '+');
    else if (format->sign == Nst_FMT_SIGN_SPACE)
        return Nst_sb_push_char(sb, ' ');
    return true;
}

/* ============================ Byte formatting ============================ */

static bool fmt_byte(Nst_StrBuilder *sb, u8 val, Format *format)
{
    return fmt_general_int(sb, (u64)val, false, true, format);
}

/* =========================== Double formatting =========================== */

static bool fmt_double(Nst_StrBuilder *sb, f64 val, Format *format)
{
    bool result = true;
    Nst_StrBuilder *temp_sb, stack_sb;
    if (format->width < 0)
        temp_sb = sb;
    else {
        if (!Nst_sb_init(&stack_sb, 10))
            return false;
        temp_sb = &stack_sb;
    }

    if (format->precision < 0)
        format->precision = 6;

#ifndef Nst_MSVC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

    result = fmt_int_add_sign(
        temp_sb,
        (*(i64 *)(f64 *)&val >> 63) & 1,
        format);

#ifndef Nst_MSVC
#pragma GCC diagnostic pop
#endif

    if (!result)
        goto finish;
    if (isinf(val)) {
        switch (format->pref_suff) {
        case Nst_FMT_PREF_SUFF_LOWER:
            result = Nst_sb_push_c(temp_sb, "inf");
            break;
        case Nst_FMT_PREF_SUFF_UPPER:
            result = Nst_sb_push_c(temp_sb, "INF");
            break;
        default:
            result = Nst_sb_push_c(temp_sb, "Inf");
        }
    } else if (isnan(val)) {
        switch (format->pref_suff) {
        case Nst_FMT_PREF_SUFF_LOWER:
            result = Nst_sb_push_c(temp_sb, "nan");
            break;
        case Nst_FMT_PREF_SUFF_UPPER:
            result = Nst_sb_push_c(temp_sb, "NAN");
            break;
        default:
            result = Nst_sb_push_c(temp_sb, "NaN");
        }
    } else {
        switch (format->double_repr) {
        case Nst_FMT_DBL_GEN_REPR:
        case Nst_FMT_DBL_GEN_REPR_P:
            result = fmt_double_gen(temp_sb, val, format);
            break;
        case Nst_FMT_DBL_STD_FORM:
        case Nst_FMT_DBL_STD_FORM_P:
            result = fmt_double_std(temp_sb, val, format);
            break;
        case Nst_FMT_DBL_DEC_REPR:
        case Nst_FMT_DBL_DEC_REPR_P:
        default:
            result = fmt_double_dec(temp_sb, val, format);
        }
    }
    if (!result || format->width < 0)
        goto finish;

    result = fmt_align_or_cut(
        sb,
        temp_sb->value,
        temp_sb->len,
        Nst_encoding_utf8_char_len((u8 *)temp_sb->value, temp_sb->len),
        format, Nst_FMT_ALIGN_RIGHT);

finish:
    if (temp_sb != sb)
        Nst_sb_destroy(temp_sb);
    return result;
}

static bool fmt_double_dec(Nst_StrBuilder *sb, f64 val, Format *format)
{
    int decpt;
    i8 *str_end;
    if (format->double_repr & INCLUDE_POINT && format->precision == 0)
        format->precision++;
    i8 *digits = Nst_dtoa(val, 3, format->precision, &decpt, NULL, &str_end);

    bool result = fmt_double_dec_digits(
        sb,
        digits, str_end - digits,
        decpt,
        format);
    Nst_freedtoa(digits);
    return result;
}

static bool fmt_double_dec_digits(Nst_StrBuilder *sb, i8 *digits,
                                  usize digits_len, int decpt, Format *format)
{
    i32 precision = format->precision;
    bool include_point = format->double_repr & INCLUDE_POINT;
    bool pad_zeroes = format->pad_zeroes_precision;

    // before the decimal point
    if (decpt <= 0) {
        Nst_sb_push_char(sb, '0');
    } else if ((usize)decpt < digits_len) {
        if (format_has_separator(format)) {
            fmt_sep_and_ndigits(
                sb,
                digits, decpt, decpt,
                (i8 *)format->separator, format_separator_len(format),
                (usize)MAX(format->separator_width, 0),
                false);
        } else
            Nst_sb_push(sb, digits, decpt);
    } else {
        if (format_has_separator(format)) {
            i8 *all_digits = Nst_malloc_c(decpt, i8);
            if (all_digits == NULL)
                return false;
            memcpy(all_digits, digits, digits_len);
            memset(all_digits + digits_len, (int)'0', decpt - digits_len);
            fmt_sep_and_ndigits(
                sb,
                all_digits, decpt, decpt,
                (i8 *)format->separator, format_separator_len(format),
                format->separator_width,
                false);
            Nst_free(all_digits);
        } else {
            Nst_sb_push(sb, digits, digits_len);
        for (usize i = digits_len; i < (usize)decpt; i++)
            Nst_sb_push_char(sb, '0');
        }
    }

    if (precision == 0)
        return true;
    else if (!pad_zeroes && !include_point) {
        if (decpt >= (isize)digits_len || -decpt >= precision)
            return true;
    }

    if (!Nst_sb_reserve(sb, precision + 1))
        return false;

    Nst_sb_push_char(sb, '.');

    // if all digits after the point would be zeroes
    if (-decpt >= precision && !pad_zeroes) {
        Nst_sb_push_char(sb, '0');
        return true;
    }

    // after the decimal point
    for (i32 i = 0; i < precision; i++) {
        if (i + decpt < 0)
            Nst_sb_push_char(sb, '0');
        else if ((usize)(i + decpt) >= digits_len) {
            if (!pad_zeroes && i != 0)
                break;
            Nst_sb_push_char(sb, '0');
        } else
            Nst_sb_push_char(sb, digits[i + decpt]);
    }
    return true;
}

static bool fmt_double_std(Nst_StrBuilder *sb, f64 val, Format *format)
{
    int decpt;
    i8 *str_end;
    if (format->double_repr & INCLUDE_POINT && format->precision == 0)
        format->precision++;
    i8 *digits = Nst_dtoa(
        val, 2,
        format->precision + 1,
        &decpt, NULL, &str_end);

    bool result = fmt_double_std_digits(
        sb,
        digits, str_end - digits,
        decpt,
        format);
    Nst_freedtoa(digits);
    return result;
}

static bool fmt_double_std_digits(Nst_StrBuilder *sb, i8 *digits,
                                  usize digits_len, int decpt, Format *format)
{
    i32 exponent = decpt - 1;
    i32 precision = format->precision;
    bool include_point = format->double_repr & INCLUDE_POINT;
    bool pad_zeroes = format->pad_zeroes_precision;

    if (!Nst_sb_reserve(sb, 2 + precision + 5))
        return false;

    if (digits_len > 0)
        Nst_sb_push_char(sb, digits[0]);
    else
        Nst_sb_push_char(sb, '0');

    if (precision > 0) {
        if (!include_point && !pad_zeroes && digits_len == 1)
            precision = 0;
        else
            Nst_sb_push_char(sb, '.');
    }

    for (i32 i = 1; i < precision + 1; i++) {
        if ((usize)i < digits_len)
            Nst_sb_push_char(sb, digits[i]);
        else {
            if (!format->pad_zeroes_precision && i != 1)
                break;
            Nst_sb_push_char(sb, '0');
        }
    }

    if (format->pref_suff == Nst_FMT_PREF_SUFF_UPPER)
        Nst_sb_push_char(sb, 'E');
    else
        Nst_sb_push_char(sb, 'e');
    if (exponent < 0) {
        Nst_sb_push_char(sb, '-');
        exponent = -exponent;
    } else
        Nst_sb_push_char(sb, '+');
    if (exponent < 10)
        Nst_sb_push_char(sb, '0');
    fmt_uint_dec(sb, (u64)(exponent));
    return true;
}

static bool fmt_double_gen(Nst_StrBuilder *sb, f64 val, Format *format)
{
    i32 precision = format->precision;
    int decpt;
    i8 *str_end;
    i8 *digits;

    if (format->double_repr & INCLUDE_POINT
        && val >= pow(10.0, (double)precision - 1)
        && val < pow(10.0, (double)precision))
    {
        digits = Nst_dtoa(val, 2, precision + 1, &decpt, NULL, &str_end);
    } else
        digits = Nst_dtoa(val, 2, precision, &decpt, NULL, &str_end);
    usize digits_len = str_end - digits;
    i32 exponent = decpt - 1;
    bool result;

    if (-4 <= exponent && exponent < precision) {
        format->precision = precision - (exponent + 1);
        if (format->precision == 0 && format->double_repr & INCLUDE_POINT)
            format->precision++;
        result = fmt_double_dec_digits(sb, digits, digits_len, decpt, format);
    } else {
        format->precision = precision - 1;
        if (format->precision == 0 && format->double_repr & INCLUDE_POINT)
            format->precision++;
        result = fmt_double_std_digits(sb, digits, digits_len, decpt, format);
    }
    Nst_freedtoa(digits);
    return result;
}

/* =========================== Boolean formatting ========================== */

static bool fmt_bool(Nst_StrBuilder *sb, bool val, Format *format)
{
    const i8 *str;
    usize str_len;
    if (val) {
        str = format->pref_suff == Nst_FMT_PREF_SUFF_UPPER ? "TRUE": "true";
        str_len = 4;
    } else {
        str = format->pref_suff == Nst_FMT_PREF_SUFF_UPPER ? "FALSE": "false";
        str_len = 5;
    }

    if (format->width < 0
        || (usize)format->width == str_len
        || ((usize)format->width < str_len && !format->cut))
    {
        return Nst_sb_push(sb, (i8 *)str, str_len);
    }

    return fmt_align_or_cut(
        sb, (i8 *)str,
        str_len, str_len,
        format, Nst_FMT_ALIGN_LEFT);
}

/* =========================== Pointer formatting ========================== */

static bool fmt_ptr(Nst_StrBuilder *sb, void *val, Format *format)
{
    bool result = true;
    Nst_StrBuilder *temp_sb, stack_sb;
    if (format->width < 0)
        temp_sb = sb;
    else {
        if (!Nst_sb_init(&stack_sb, 2 + sizeof(usize) * 2))
            return false;
        temp_sb = &stack_sb;
    }

    u64 int_val = (u64)(usize)val;
    Nst_sb_push_c(
        temp_sb,
        format->pref_suff == Nst_FMT_PREF_SUFF_UPPER ? "0X" : "0x");
    u8 msb = fmt_uint_msb64(int_val);
    if (format->pad_zeroes_precision) {
        usize digit_count = (msb / 4) + 1;
        for (usize i = sizeof(usize) * 2; i > digit_count; i--)
            Nst_sb_push_char(sb, '0');
    }

    bool upper = format->int_repr == Nst_FMT_INT_HEX_UPPER;
    fmt_uint_hex(temp_sb, int_val, upper);

    if (format->width < 0)
        goto finish;
    result = fmt_align_or_cut(
        sb,
        temp_sb->value, temp_sb->len, temp_sb->len,
        format, Nst_FMT_ALIGN_RIGHT);

finish:
    if (temp_sb != sb)
        Nst_sb_destroy(sb);
    return result;
}

/* ============================ Char formatting ============================ */

static bool fmt_char(Nst_StrBuilder *sb, i8 val, Format *format)
{
    u8 ch_buf[3];
    i32 ch_len = Nst_ext_utf8_from_utf32((u32)(u8)val, ch_buf);
    ch_buf[ch_len] = '\0';
    return fmt_str(sb, (i8 *)ch_buf, ch_len, format);
}
