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
    Nst_buffer_destroy(&str_buf);                                             \
    return;                                                                   \
    } while (0)

#define INC_RECURSION_LVL do {                                                \
    recursion_level++;                                                        \
    if (recursion_level > 1500) {                                             \
        Nst_set_memory_error_c("over 1500 recursive calls, dump failed");     \
        FAIL;                                                                 \
    }                                                                         \
    } while (0)

#define DEC_RECURSION_LVL recursion_level--

static void dump_obj(Nst_Obj *obj, i32 indent);
static void dump_str(Nst_StrObj *str);
static void dump_num(Nst_Obj *number);
static void dump_seq(Nst_SeqObj *seq, i32 indent);
static void dump_map(Nst_MapObj *map, i32 indent);
static void add_comma(i32 indent);
static void add_indent(i32 indent);

static Nst_Buffer str_buf;
static i32 indent_level;
static i32 recursion_level;

Nst_Obj *json_dump(Nst_Obj *obj, i32 indent)
{
    recursion_level = 0;
    if (!Nst_buffer_init(&str_buf, 255))
        return nullptr;

    dump_obj(obj, indent);
    if (Nst_error_occurred())
        return nullptr;

    return OBJ(Nst_buffer_to_string(&str_buf));
}

static void dump_obj(Nst_Obj *obj, i32 indent)
{
    INC_RECURSION_LVL;
    if (Nst_T(obj, Str))
        dump_str(STR(obj));
    else if (Nst_T(obj, Int) || Nst_T(obj, Real) || Nst_T(obj, Byte))
        dump_num(obj);
    else if (Nst_T(obj, Map))
        dump_map(MAP(obj), indent);
    else if (Nst_T(obj, Array) || Nst_T(obj, Vector))
        dump_seq(SEQ(obj), indent);
    else if (obj == Nst_null())
        Nst_buffer_append_c_str(&str_buf, "null");
    else if (obj == Nst_true())
        Nst_buffer_append_c_str(&str_buf, "true");
    else if (obj == Nst_false())
        Nst_buffer_append_c_str(&str_buf, "false");
    else {
        Nst_set_type_errorf(
            "JSON: an object of type %s is not serializable",
            TYPE_NAME(obj));
        FAIL;
    }
    DEC_RECURSION_LVL;
}

static void dump_str(Nst_StrObj *str)
{
    INC_RECURSION_LVL;
    usize unicode_bytes = 0;
    i8 *s_val = str->value;
    usize s_len = str->len;
    const i8 *hex_digits = "0123456789abcdef";

    for (usize i = 0; i < s_len; i++) {
        i32 res = Nst_check_ext_utf8_bytes((u8 *)s_val + i, s_len - i);
        if (res != 1)
            unicode_bytes++;
        if (res != -1)
            i += res - 1;
    }

    Nst_buffer_expand_by(&str_buf, s_len + unicode_bytes * 5 + 2);
    EXCEPT_ERROR;

    Nst_buffer_append_char(&str_buf, '"');
    for (usize i = 0; i < s_len; i++) {
        i32 res = Nst_check_ext_utf8_bytes((u8 *)s_val + i, s_len - i);
        switch (res) {
        case 1:
            Nst_buffer_append_char(&str_buf, s_val[i]);
            break;
        case -1: {
            Nst_buffer_append_c_str(&str_buf, "\\u00");
            u8 c1 = u8(s_val[i]) >> 4;
            u8 c2 = u8(s_val[i]) & 0xf;
            Nst_buffer_append_char(&str_buf, hex_digits[c1]);
            Nst_buffer_append_char(&str_buf, hex_digits[c2]);
            break;
        }
        case 2: {
            Nst_buffer_append_c_str(&str_buf, "\\u0");
            u8 c1 = u8(s_val[i]) >> 2 & 0xf;
            u8 c2 = ((u8(s_val[i]) & 0x3) << 2)
                  + ((u8(s_val[i + 1]) >> 4) & 0x3);
            u8 c3 = u8(s_val[i + 1]) & 0xf;
            i++;

            Nst_buffer_append_char(&str_buf, hex_digits[c1]);
            Nst_buffer_append_char(&str_buf, hex_digits[c2]);
            Nst_buffer_append_char(&str_buf, hex_digits[c3]);
            break;
        }
        case 3: {
            Nst_buffer_append_c_str(&str_buf, "\\u");
            u8 c1 = u8(s_val[i++]) & 0xf;
            u8 c2 = (u8(s_val[i]) >> 2) & 0xf;
            u8 c3 = ((u8(s_val[i]) & 0x3) << 2)
                  + ((u8(s_val[i + 1]) >> 4) & 0x3);
            u8 c4 = (u8)(s_val[i + 1]) & 0xf;
            i++;

            Nst_buffer_append_char(&str_buf, hex_digits[c1]);
            Nst_buffer_append_char(&str_buf, hex_digits[c2]);
            Nst_buffer_append_char(&str_buf, hex_digits[c3]);
            Nst_buffer_append_char(&str_buf, hex_digits[c4]);
            break;
        }
        case 4:
            Nst_set_value_error_c(
                "JSON: cannot serialize characters above U+FFFF");
            FAIL;
        }
    }
    Nst_buffer_append_char(&str_buf, '"');
    DEC_RECURSION_LVL;
}

static void dump_num(Nst_Obj *number)
{
    INC_RECURSION_LVL;
    f64 val;
    i8 loc_buf[27];
    usize len;

    if (Nst_T(number, Byte)) {
        sprintf(loc_buf, "%i", AS_BYTE(number));
        Nst_buffer_append_c_str(&str_buf, loc_buf);
        goto finish;
    } else if (Nst_T(number, Int)) {
        sprintf(loc_buf, "%lli", AS_INT(number));
        Nst_buffer_append_c_str(&str_buf, loc_buf);
        goto finish;
    }
    val = AS_REAL(number);
    if (isinf(val) || isnan(val)) {
        if (!nan_and_inf) {
            Nst_set_value_error_c(
                "JSON: cannot serialize infinities or NaNs");
            FAIL;
        }
        if (isinf(val)) {
            if (val < 0)
                Nst_buffer_append_c_str(&str_buf, "-Infinity");
            else
                Nst_buffer_append_c_str(&str_buf, "Infinity");
        } else
            Nst_buffer_append_c_str(&str_buf, "NaN");
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
    Nst_buffer_append_c_str(&str_buf, loc_buf);

finish:
    DEC_RECURSION_LVL;
}

static void dump_seq(Nst_SeqObj *seq, i32 indent)
{
    if (seq->len == 0) {
        Nst_buffer_append_c_str(&str_buf, "[]");
        return;
    }
    INC_RECURSION_LVL;

    Nst_buffer_append_char(&str_buf, '[');
    EXCEPT_ERROR;
    indent_level++;

    if (indent > 1) {
        Nst_buffer_expand_by(&str_buf, indent * indent_level + 1);
        EXCEPT_ERROR;
        Nst_buffer_append_char(&str_buf, '\n');
        for (i8 i = 0; i < indent * indent_level; i++)
            Nst_buffer_append_char(&str_buf, ' ');
    }

    for (usize i = 0, n = seq->len; i < n; i++) {
        dump_obj(seq->objs[i], indent);
        EXCEPT_ERROR;

        if (i + 1 == n) {
            DEC_RECURSION_LVL;
            indent_level--;
            Nst_buffer_expand_by(&str_buf, 1);
            EXCEPT_ERROR;
            if (indent < 1) {
                Nst_buffer_append_char(&str_buf, ']');
                return;
            }
            Nst_buffer_append_char(&str_buf, '\n');
            add_indent(indent);
            EXCEPT_ERROR;
            Nst_buffer_expand_by(&str_buf, 1);
            EXCEPT_ERROR;
            Nst_buffer_append_char(&str_buf, ']');
            return;
        }

        add_comma(indent);
        EXCEPT_ERROR;
    }
}

static void dump_map(Nst_MapObj *map, i32 indent)
{
    if (map->len == 0) {
        Nst_buffer_append_c_str(&str_buf, "{}");
        return;
    }
    INC_RECURSION_LVL;
    Nst_buffer_append_char(&str_buf, '{');
    EXCEPT_ERROR;
    indent_level++;

    if (indent > 1) {
        Nst_buffer_expand_by(&str_buf, indent * indent_level + 1);
        EXCEPT_ERROR;
        Nst_buffer_append_char(&str_buf, '\n');
        for (i8 i = 0; i < indent * indent_level; i++)
            Nst_buffer_append_char(&str_buf, ' ');
    }

    usize count = 0;
    usize tot = map->len;
    Nst_MapNode *nodes = map->nodes;
    for (i32 i = Nst_map_get_next_idx(-1, map);
         i != -1;
         i = Nst_map_get_next_idx(i, map))
    {
        count++;
        Nst_Obj *key = nodes[i].key;
        Nst_Obj *value = nodes[i].value;
        if (!Nst_T(key, Str)) {
            Nst_set_type_error_c("JSON: all keys of a map must be strings");
            FAIL;
        }

        dump_str(STR(key));
        EXCEPT_ERROR;

        if (indent == -1)
            Nst_buffer_append_char(&str_buf, ':');
        else
            Nst_buffer_append_c_str(&str_buf, ": ");
        EXCEPT_ERROR;

        dump_obj(value, indent);
        EXCEPT_ERROR;

        if (count == tot) {
            DEC_RECURSION_LVL;
            indent_level--;
            if (indent < 1) {
                Nst_buffer_append_char(&str_buf, '}');
                return;
            } else {
                Nst_buffer_append_char(&str_buf, '\n');
                EXCEPT_ERROR;
            }
            add_indent(indent);
            EXCEPT_ERROR;
            Nst_buffer_append_char(&str_buf, '}');
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
        Nst_buffer_expand_by(&str_buf, 1);
    else
        Nst_buffer_expand_by(&str_buf, 2);
    EXCEPT_ERROR;
    Nst_buffer_append_char(&str_buf, ',');
    if (indent == 0)
        Nst_buffer_append_char(&str_buf, ' ');
    else if (indent > 0) {
        Nst_buffer_append_char(&str_buf, '\n');
        add_indent(indent);
    }
    DEC_RECURSION_LVL;
}

static void add_indent(i32 indent)
{
    INC_RECURSION_LVL;
    Nst_buffer_expand_by(&str_buf, indent * indent_level);
    EXCEPT_ERROR;
    for (i32 i = 0; i < indent * indent_level; i++)
        Nst_buffer_append_char(&str_buf, ' ');
    DEC_RECURSION_LVL;
}
