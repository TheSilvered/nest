#include <cmath>
#include <cstring>
#include "json_dumper.h"

#ifndef WINDOWS

#define isinf std::isinf
#define isnan std::isnan

#endif

#define EXCEPT_ERROR \
    do { \
        if ( NST_ERROR_OCCURRED ) \
        { \
            FAIL; \
        } \
    } while ( 0 )

#define FAIL \
    do { \
        nst_buffer_destroy(&str_buf); \
        return; \
    } while ( 0 )

#define INC_RECURSION_LVL \
    do { \
        recursion_level++; \
        if ( recursion_level > 1500 ) \
        { \
            NST_SET_RAW_MEMORY_ERROR("over 1500 recursive calls, dump failed"); \
            FAIL; \
        } \
    } while ( 0 )
#define DEC_RECURSION_LVL recursion_level--

static void dump_obj(Nst_Obj *obj, i32 indent, Nst_OpErr *err);
static void dump_str(Nst_StrObj *str, Nst_OpErr *err);
static void dump_num(Nst_Obj *number, Nst_OpErr *err);
static void dump_seq(Nst_SeqObj *seq, i32 indent, Nst_OpErr *err);
static void dump_map(Nst_MapObj *map, i32 indent, Nst_OpErr *err);
static void add_comma(i32 indent, Nst_OpErr *err);
static void add_indent(i32 indent, Nst_OpErr *err);

static Nst_Buffer str_buf;
static i32 indent_level;
static i32 recursion_level;

Nst_Obj *json_dump(Nst_Obj *obj, i32 indent, Nst_OpErr *err)
{
    recursion_level = 0;
    if ( !nst_buffer_init(&str_buf, 255, err) )
    {
        return nullptr;
    }

    dump_obj(obj, indent, err);
    if ( NST_ERROR_OCCURRED )
    {
        return nullptr;
    }

    return OBJ(nst_buffer_to_string(&str_buf, err));
}

static void dump_obj(Nst_Obj *obj, i32 indent, Nst_OpErr *err)
{
    INC_RECURSION_LVL;
    if ( obj->type == nst_type()->Str )
    {
        dump_str(STR(obj), err);
    }
    else if ( obj->type == nst_type()->Int  ||
              obj->type == nst_type()->Real ||
              obj->type == nst_type()->Byte )
    {
        dump_num(obj, err);
    }
    else if ( obj->type == nst_type()->Map )
    {
        dump_map(MAP(obj), indent, err);
    }
    else if ( obj->type == nst_type()->Array || obj->type == nst_type()->Vector )
    {
        dump_seq(SEQ(obj), indent, err);
    }
    else if ( obj == nst_null() )
    {
        nst_buffer_append_c_str(&str_buf, "null", err);
    }
    else if ( obj == nst_true() )
    {
        nst_buffer_append_c_str(&str_buf, "true", err);
    }
    else if ( obj == nst_false() )
    {
        nst_buffer_append_c_str(&str_buf, "false", err);
    }
    else
    {
        NST_SET_TYPE_ERROR(nst_sprintf(
            "JSON: an object of type %s is not serializable",
            TYPE_NAME(obj)));
        FAIL;
    }
    DEC_RECURSION_LVL;
}

static void dump_str(Nst_StrObj *str, Nst_OpErr *err)
{
    INC_RECURSION_LVL;
    usize unicode_bytes = 0;
    i8 *s_val = str->value;
    usize s_len = str->len;
    const i8 *hex_digits = "0123456789abcdef";

    for ( usize i = 0; i < s_len; i++ )
    {
        i32 res = nst_check_utf8_bytes((u8 *)s_val + i, s_len - i);
        if ( res != 1 )
        {
            unicode_bytes++;
        }
        if ( res != -1 )
        {
            i += res - 1;
        }
    }

    nst_buffer_expand_by(&str_buf, s_len + unicode_bytes * 5 + 2, err);
    EXCEPT_ERROR;

    nst_buffer_append_char(&str_buf, '"', NULL);
    for ( usize i = 0; i < s_len; i++ )
    {
        i32 res = nst_check_utf8_bytes((u8 *)s_val + i, s_len - i);
        switch ( res )
        {
        case 1:
            nst_buffer_append_char(&str_buf, s_val[i], NULL);
            break;
        case -1:
        {
            nst_buffer_append_c_str(&str_buf, "\\u00", err);
            u8 c1 = u8(s_val[i]) >> 4;
            u8 c2 = u8(s_val[i]) & 0xf;
            nst_buffer_append_char(&str_buf, hex_digits[c1], NULL);
            nst_buffer_append_char(&str_buf, hex_digits[c2], NULL);
            break;
        }
        case 2:
        {
            nst_buffer_append_c_str(&str_buf, "\\u0", err);
            u8 c1 = u8(s_val[i]) >> 2 & 0xf;
            u8 c2 = ((u8(s_val[i]) & 0x3) << 2)
                  + ((u8(s_val[i + 1]) >> 4) & 0x3);
            u8 c3 = u8(s_val[i + 1]) & 0xf;
            i++;

            nst_buffer_append_char(&str_buf, hex_digits[c1], NULL);
            nst_buffer_append_char(&str_buf, hex_digits[c2], NULL);
            nst_buffer_append_char(&str_buf, hex_digits[c3], NULL);
            break;
        }
        case 3:
        {
            nst_buffer_append_c_str(&str_buf, "\\u", err);
            u8 c1 = u8(s_val[i++]) & 0xf;
            u8 c2 = (u8(s_val[i]) >> 2) & 0xf;
            u8 c3 = ((u8(s_val[i]) & 0x3) << 2)
                  + ((u8(s_val[i + 1]) >> 4) & 0x3);
            u8 c4 = (u8)(s_val[i + 1]) & 0xf;
            i++;

            nst_buffer_append_char(&str_buf, hex_digits[c1], NULL);
            nst_buffer_append_char(&str_buf, hex_digits[c2], NULL);
            nst_buffer_append_char(&str_buf, hex_digits[c3], NULL);
            nst_buffer_append_char(&str_buf, hex_digits[c4], NULL);
            break;
        }
        case 4:
            NST_SET_RAW_VALUE_ERROR(
                "JSON: cannot serialize characters above U+FFFF");
            FAIL;
        }
    }
    nst_buffer_append_char(&str_buf, '"', NULL);
    DEC_RECURSION_LVL;
}

static void dump_num(Nst_Obj *number, Nst_OpErr *err)
{
    INC_RECURSION_LVL;
    if ( number->type == nst_type()->Byte )
    {
        i8 loc_buf[4];
        sprintf(loc_buf, "%i", AS_BYTE(number));
        nst_buffer_append_c_str(&str_buf, loc_buf, err);
    }
    else if ( number->type == nst_type()->Int )
    {
        i8 loc_buf[21];
        sprintf(loc_buf, "%lli", AS_INT(number));
        nst_buffer_append_c_str(&str_buf, loc_buf, err);
    }
    else
    {
        Nst_Real val = AS_REAL(number);
        if ( isinf(val) || isnan(val) )
        {
            NST_SET_RAW_VALUE_ERROR(
                "JSON: cannot serialize infinities or NaNs");
            FAIL;
        }
        i8 loc_buf[27];
        sprintf(loc_buf, "%.16lg", val);
        usize len = strlen(loc_buf);
        for ( usize i = 0; i < len; i++ )
        {
            if ( loc_buf[i] == '.' || loc_buf[i] == 'e' )
            {
                goto finish;
            }
        }
        loc_buf[len++] = '.';
        loc_buf[len++] = '0';
        loc_buf[len++] = '\0';
    finish:
        nst_buffer_append_c_str(&str_buf, loc_buf, err);
    }
    DEC_RECURSION_LVL;
}

static void dump_seq(Nst_SeqObj *seq, i32 indent, Nst_OpErr *err)
{
    if ( seq->len == 0 )
    {
        nst_buffer_append_c_str(&str_buf, "[]", err);
        return;
    }
    INC_RECURSION_LVL;

    nst_buffer_append_char(&str_buf, '[', err);
    EXCEPT_ERROR;
    indent_level++;

    if ( indent > 1 )
    {
        nst_buffer_expand_by(&str_buf, indent * indent_level + 1, err);
        EXCEPT_ERROR;
        nst_buffer_append_char(&str_buf, '\n', err);
        for ( i8 i = 0; i < indent * indent_level; i++ )
        {
            nst_buffer_append_char(&str_buf, ' ', err);
        }
    }

    for ( usize i = 0, n = seq->len; i < n; i++ )
    {
        dump_obj(seq->objs[i], indent, err);
        EXCEPT_ERROR;

        if ( i + 1 == n )
        {
            DEC_RECURSION_LVL;
            indent_level--;
            nst_buffer_expand_by(&str_buf, 1, err);
            EXCEPT_ERROR;
            if ( indent < 1 )
            {
                nst_buffer_append_char(&str_buf, ']', err);
                return;
            }
            nst_buffer_append_char(&str_buf, '\n', err);
            add_indent(indent, err);
            EXCEPT_ERROR;
            nst_buffer_expand_by(&str_buf, 1, err);
            EXCEPT_ERROR;
            nst_buffer_append_char(&str_buf, ']', err);
            return;
        }

        add_comma(indent, err);
        EXCEPT_ERROR;
    }
}

static void dump_map(Nst_MapObj *map, i32 indent, Nst_OpErr *err)
{
    if ( map->item_count == 0 )
    {
        nst_buffer_append_c_str(&str_buf, "{}", err);
        return;
    }
    INC_RECURSION_LVL;
    nst_buffer_append_char(&str_buf, '{', err);
    EXCEPT_ERROR;
    indent_level++;

    if ( indent > 1 )
    {
        nst_buffer_expand_by(&str_buf, indent * indent_level + 1, err);
        EXCEPT_ERROR;
        nst_buffer_append_char(&str_buf, '\n', err);
        for ( i8 i = 0; i < indent * indent_level; i++ )
        {
            nst_buffer_append_char(&str_buf, ' ', err);
        }
    }

    usize count = 0;
    usize tot = map->item_count;
    Nst_MapNode *nodes = map->nodes;
    for ( i32 i = nst_map_get_next_idx(-1, map);
          i != -1;
          i = nst_map_get_next_idx(i, map) )
    {
        count++;
        Nst_Obj *key = nodes[i].key;
        Nst_Obj *value = nodes[i].value;
        if ( key->type != nst_type()->Str )
        {
            NST_SET_RAW_TYPE_ERROR("JSON: all keys of a map must be strings");
            FAIL;
        }

        dump_str(STR(key), err);
        EXCEPT_ERROR;

        if ( indent == -1 )
        {
            nst_buffer_append_char(&str_buf, ':', err);
        }
        else
        {
            nst_buffer_append_c_str(&str_buf, ": ", err);
        }
        EXCEPT_ERROR;

        dump_obj(value, indent, err);
        EXCEPT_ERROR;

        if ( count == tot )
        {
            DEC_RECURSION_LVL;
            indent_level--;
            if ( indent < 1 )
            {
                nst_buffer_append_char(&str_buf, '}', err);
                return;
            }
            else
            {
                nst_buffer_append_char(&str_buf, '\n', err);
                EXCEPT_ERROR;
            }
            add_indent(indent, err);
            EXCEPT_ERROR;
            nst_buffer_append_char(&str_buf, '}', err);
            return;
        }

        add_comma(indent, err);
        EXCEPT_ERROR;
    }
}

static void add_comma(i32 indent, Nst_OpErr *err)
{
    INC_RECURSION_LVL;
    if ( indent == -1 )
    {
        nst_buffer_expand_by(&str_buf, 1, err);
    }
    else
    {
        nst_buffer_expand_by(&str_buf, 2, err);
    }
    EXCEPT_ERROR;
    nst_buffer_append_char(&str_buf, ',', err);
    if ( indent == 0 )
    {
        nst_buffer_append_char(&str_buf, ' ', err);
    }
    else if ( indent > 0 )
    {
        nst_buffer_append_char(&str_buf, '\n', err);
        add_indent(indent, err);
    }
    DEC_RECURSION_LVL;
}

static void add_indent(i32 indent, Nst_OpErr *err)
{
    INC_RECURSION_LVL;
    nst_buffer_expand_by(&str_buf, indent * indent_level, err);
    EXCEPT_ERROR;
    for ( i32 i = 0; i < indent * indent_level; i++ )
    {
        nst_buffer_append_char(&str_buf, ' ', err);
    }
    DEC_RECURSION_LVL;
}
