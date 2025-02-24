#include <cmath>
#include <cstring>
#include "json_dumper.h"
#include "json_lexer.h"

#ifndef Nst_MSVC

#define isinf std::isinf
#define isnan std::isnan

#endif

#define EXCEPT_ERROR do {                                                     \
    if (Nst_error_occurred())                                                 \
        FAIL;                                                                 \
    } while (0)

#define FAIL do {                                                             \
    Nst_sb_destroy(&sb);                                                      \
    return;                                                                   \
    } while (0)

#define INC_RECURSION_LVL do {                                                \
    recursion_level++;                                                        \
    if (recursion_level > 1500) {                                             \
        Nst_error_setc_memory("over 1500 recursive calls, dump failed");     \
        FAIL;                                                                 \
    }                                                                         \
    } while (0)

#define DEC_RECURSION_LVL recursion_level--

static void dump_obj(Nst_Obj *obj, i32 indent);
static void dump_str(Nst_Obj *str);
static void dump_num(Nst_Obj *number);
static void dump_seq(Nst_Obj *seq, i32 indent);
static void dump_map(Nst_Obj *map, i32 indent);
static void add_comma(i32 indent);
static void add_indent(i32 indent);

static Nst_StrBuilder sb;
static i32 indent_level;
static i32 recursion_level;

Nst_Obj *json_dump(Nst_Obj *obj, i32 indent)
{
    recursion_level = 0;
    if (!Nst_sb_init(&sb, 255))
        return nullptr;

    dump_obj(obj, indent);
    if (Nst_error_occurred())
        return nullptr;

    return Nst_str_from_sb(&sb);
}

static void dump_obj(Nst_Obj *obj, i32 indent)
{
    INC_RECURSION_LVL;
    if (Nst_T(obj, Str))
        dump_str(obj);
    else if (Nst_T(obj, Int) || Nst_T(obj, Real) || Nst_T(obj, Byte))
        dump_num(obj);
    else if (Nst_T(obj, Map))
        dump_map(obj, indent);
    else if (Nst_T(obj, Array) || Nst_T(obj, Vector))
        dump_seq(obj, indent);
    else if (obj == Nst_null())
        Nst_sb_push_c(&sb, "null");
    else if (obj == Nst_true())
        Nst_sb_push_c(&sb, "true");
    else if (obj == Nst_false())
        Nst_sb_push_c(&sb, "false");
    else {
        Nst_error_setf_type(
            "JSON: an object of type %s is not serializable",
            Nst_type_name(obj->type).value);
        FAIL;
    }
    DEC_RECURSION_LVL;
}

static void dump_str(Nst_Obj *str)
{
    INC_RECURSION_LVL;

    usize unicode_bytes = 0;
    i8 *s_val = Nst_str_value(str);
    usize s_len = Nst_str_len(str);
    const i8 *hex_digits = "0123456789abcdef";

    for (usize i = 0; i < s_len; i++) {
        i32 res = Nst_check_ext_utf8_bytes((u8 *)s_val + i, s_len - i);
        if (res != 1)
            unicode_bytes++;
        if (res != -1)
            i += res - 1;
    }

    Nst_sb_reserve(&sb, s_len + unicode_bytes * 5 + 2);
    EXCEPT_ERROR;

    Nst_sb_push_char(&sb, '"');
    for (usize i = 0; i < s_len; i++) {
        i32 res = Nst_check_ext_utf8_bytes((u8 *)s_val + i, s_len - i);
        switch (res) {
        case 1:
            Nst_sb_push_char(&sb, s_val[i]);
            break;
        case -1: {
            Nst_sb_push_c(&sb, "\\u00");
            u8 c1 = u8(s_val[i]) >> 4;
            u8 c2 = u8(s_val[i]) & 0xf;
            Nst_sb_push_char(&sb, hex_digits[c1]);
            Nst_sb_push_char(&sb, hex_digits[c2]);
            break;
        }
        case 2: {
            Nst_sb_push_c(&sb, "\\u0");
            u8 c1 = u8(s_val[i]) >> 2 & 0xf;
            u8 c2 = ((u8(s_val[i]) & 0x3) << 2)
                  + ((u8(s_val[i + 1]) >> 4) & 0x3);
            u8 c3 = u8(s_val[i + 1]) & 0xf;
            i++;

            Nst_sb_push_char(&sb, hex_digits[c1]);
            Nst_sb_push_char(&sb, hex_digits[c2]);
            Nst_sb_push_char(&sb, hex_digits[c3]);
            break;
        }
        case 3: {
            Nst_sb_push_c(&sb, "\\u");
            u8 c1 = u8(s_val[i++]) & 0xf;
            u8 c2 = (u8(s_val[i]) >> 2) & 0xf;
            u8 c3 = ((u8(s_val[i]) & 0x3) << 2)
                  + ((u8(s_val[i + 1]) >> 4) & 0x3);
            u8 c4 = (u8)(s_val[i + 1]) & 0xf;
            i++;

            Nst_sb_push_char(&sb, hex_digits[c1]);
            Nst_sb_push_char(&sb, hex_digits[c2]);
            Nst_sb_push_char(&sb, hex_digits[c3]);
            Nst_sb_push_char(&sb, hex_digits[c4]);
            break;
        }
        case 4:
            Nst_error_setc_value(
                "JSON: cannot serialize characters above U+FFFF");
            FAIL;
        }
    }
    Nst_sb_push_char(&sb, '"');
    DEC_RECURSION_LVL;
}

static void dump_num(Nst_Obj *number)
{
    INC_RECURSION_LVL;
    f64 val;
    i8 loc_buf[27];
    usize len;

    if (Nst_T(number, Byte)) {
        sprintf(loc_buf, "%i", Nst_byte_u8(number));
        Nst_sb_push_c(&sb, loc_buf);
        goto finish;
    } else if (Nst_T(number, Int)) {
        sprintf(loc_buf, "%lli", Nst_int_i64(number));
        Nst_sb_push_c(&sb, loc_buf);
        goto finish;
    }
    val = Nst_real_f64(number);
    if (isinf(val) || isnan(val)) {
        if (!nan_and_inf) {
            Nst_error_setc_value(
                "JSON: cannot serialize infinities or NaNs");
            FAIL;
        }
        if (isinf(val)) {
            if (val < 0)
                Nst_sb_push_c(&sb, "-Infinity");
            else
                Nst_sb_push_c(&sb, "Infinity");
        } else
            Nst_sb_push_c(&sb, "NaN");
        goto finish;
    }
    sprintf(loc_buf, "%.16lg", val);
    len = strlen(loc_buf);
    for (usize i = 0; i < len; i++) {
        if (loc_buf[i] == '.' || loc_buf[i] == 'e')
            goto add_buf;
    }
    loc_buf[len++] = '.';
    loc_buf[len++] = '0';
    loc_buf[len++] = '\0';
add_buf:
    Nst_sb_push_c(&sb, loc_buf);

finish:
    DEC_RECURSION_LVL;
}

static void dump_seq(Nst_Obj *seq, i32 indent)
{
    if (Nst_seq_len(seq) == 0) {
        Nst_sb_push_c(&sb, "[]");
        return;
    }
    INC_RECURSION_LVL;

    Nst_sb_push_char(&sb, '[');
    EXCEPT_ERROR;
    indent_level++;

    if (indent > 1) {
        Nst_sb_reserve(&sb, indent * indent_level + 1);
        EXCEPT_ERROR;
        Nst_sb_push_char(&sb, '\n');
        for (i8 i = 0; i < indent * indent_level; i++)
            Nst_sb_push_char(&sb, ' ');
    }

    for (usize i = 0, n = Nst_seq_len(seq); i < n; i++) {
        dump_obj(Nst_seq_getnf(seq, i), indent);
        EXCEPT_ERROR;

        if (i + 1 == n) {
            DEC_RECURSION_LVL;
            indent_level--;
            Nst_sb_reserve(&sb, 1);
            EXCEPT_ERROR;
            if (indent < 1) {
                Nst_sb_push_char(&sb, ']');
                return;
            }
            Nst_sb_push_char(&sb, '\n');
            add_indent(indent);
            EXCEPT_ERROR;
            Nst_sb_reserve(&sb, 1);
            EXCEPT_ERROR;
            Nst_sb_push_char(&sb, ']');
            return;
        }

        add_comma(indent);
        EXCEPT_ERROR;
    }
}

static void dump_map(Nst_Obj *map, i32 indent)
{
    if (Nst_map_len(map) == 0) {
        Nst_sb_push_c(&sb, "{}");
        return;
    }
    INC_RECURSION_LVL;
    Nst_sb_push_char(&sb, '{');
    EXCEPT_ERROR;
    indent_level++;

    if (indent > 1) {
        Nst_sb_reserve(&sb, indent * indent_level + 1);
        EXCEPT_ERROR;
        Nst_sb_push_char(&sb, '\n');
        for (i8 i = 0; i < indent * indent_level; i++)
            Nst_sb_push_char(&sb, ' ');
    }

    usize count = 0;
    usize tot = Nst_map_len(map);
    Nst_Obj *key;
    Nst_Obj *value;
    for (isize i = Nst_map_next(-1, map, &key, &value);
         i != -1;
         i = Nst_map_next(i, map, &key, &value))
    {
        count++;
        if (!Nst_T(key, Str)) {
            Nst_error_setc_type("JSON: all keys of a map must be strings");
            FAIL;
        }

        dump_str(key);
        EXCEPT_ERROR;

        if (indent == -1)
            Nst_sb_push_char(&sb, ':');
        else
            Nst_sb_push_c(&sb, ": ");
        EXCEPT_ERROR;

        dump_obj(value, indent);
        EXCEPT_ERROR;

        if (count == tot) {
            DEC_RECURSION_LVL;
            indent_level--;
            if (indent < 1) {
                Nst_sb_push_char(&sb, '}');
                return;
            } else {
                Nst_sb_push_char(&sb, '\n');
                EXCEPT_ERROR;
            }
            add_indent(indent);
            EXCEPT_ERROR;
            Nst_sb_push_char(&sb, '}');
            return;
        }

        add_comma(indent);
        EXCEPT_ERROR;
    }
}

static void add_comma(i32 indent)
{
    INC_RECURSION_LVL;
    if (indent == -1)
        Nst_sb_reserve(&sb, 1);
    else
        Nst_sb_reserve(&sb, 2);
    EXCEPT_ERROR;
    Nst_sb_push_char(&sb, ',');
    if (indent == 0)
        Nst_sb_push_char(&sb, ' ');
    else if (indent > 0) {
        Nst_sb_push_char(&sb, '\n');
        add_indent(indent);
    }
    DEC_RECURSION_LVL;
}

static void add_indent(i32 indent)
{
    INC_RECURSION_LVL;
    Nst_sb_reserve(&sb, indent * indent_level);
    EXCEPT_ERROR;
    for (i32 i = 0; i < indent * indent_level; i++)
        Nst_sb_push_char(&sb, ' ');
    DEC_RECURSION_LVL;
}
