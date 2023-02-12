#include <cmath>
#include "json_dumper.h"

#if !defined(_WIN32) && !defined(WIN32)

#define isinf std::isinf
#define isnan std::isnan

#endif

#define EXCEPT_ERROR if ( str_buf == nullptr ) return
#define FAIL do { delete[] str_buf; str_buf = nullptr; return; } while ( 0 )

static void expand_to(size_t size_to_reach, Nst_OpErr *err);
static void expand_by(size_t size_to_add, Nst_OpErr *err);
static void append_buf(const char *str, size_t len, Nst_OpErr *err);
static void fit_buf();
static void dump_obj(Nst_Obj *obj, int indent, Nst_OpErr *err);
static void dump_str(Nst_StrObj *str, Nst_OpErr *err);
static void dump_num(Nst_Obj *number, Nst_OpErr *err);
static void dump_seq(Nst_SeqObj *seq, int indent, Nst_OpErr *err);
static void dump_map(Nst_MapObj *map, int indent, Nst_OpErr *err);
static void add_comma(int indent, Nst_OpErr *err);
static void add_indent(int indent, Nst_OpErr *err);

static char *str_buf;
static size_t buf_size;
static size_t str_len;
static int indent_level;

Nst_Obj *json_dump(Nst_Obj *obj, int indent, Nst_OpErr *err)
{
    str_buf = new char[255];
    buf_size = 255;
    str_len = 0;
    indent_level = 0;

    dump_obj(obj, indent, err);
    if ( str_buf == nullptr )
    {
        return nullptr;
    }

    str_buf[str_len] = 0;
    fit_buf();
    return nst_string_new(str_buf, str_len, true);
}

static void expand_to(size_t size_to_reach, Nst_OpErr *err)
{
    if ( buf_size >= size_to_reach )
    {
        return;
    }

    size_t new_size = size_t(size_to_reach * 1.5);
    char *new_buf = (char *)realloc(str_buf, new_size);
    if ( new_buf == nullptr )
    {
        NST_FAILED_ALLOCATION;
        FAIL;
    }

    str_buf = new_buf;
    buf_size = new_size;
}

static void expand_by(size_t size_to_add, Nst_OpErr *err)
{
    expand_to(str_len + size_to_add + 1, err);
}

static void append_buf(const char *str, size_t len, Nst_OpErr *err)
{
    if ( len == 0 )
    {
        len = strlen(str);
    }

    expand_by(len, err);
    EXCEPT_ERROR;
    memcpy(str_buf + str_len, str, len);
    str_len += len;
}

static void fit_buf()
{
    char *new_buf = (char *)realloc(str_buf, size_t(str_len + 1));
    if ( new_buf != nullptr )
    {
        str_buf = new_buf;
        buf_size = str_len + 1;
    }
}

static void dump_obj(Nst_Obj *obj, int indent, Nst_OpErr *err)
{
    if ( obj->type == nst_t.Str )
    {
        dump_str(STR(obj), err);
    }
    else if ( obj->type == nst_t.Int  ||
              obj->type == nst_t.Real ||
              obj->type == nst_t.Byte )
    {
        dump_num(obj, err);
    }
    else if ( obj->type == nst_t.Map )
    {
        dump_map(MAP(obj), indent, err);
    }
    else if ( obj->type == nst_t.Array || obj->type == nst_t.Vector )
    {
        dump_seq(SEQ(obj), indent, err);
    }
    else if ( obj == nst_c.Null_null )
    {
        append_buf("null", 4, err);
    }
    else if ( obj == nst_c.Bool_true )
    {
        append_buf("true", 4, err);
    }
    else if ( obj == nst_c.Bool_false )
    {
        append_buf("false", 5, err);
    }
    else
    {
        NST_SET_TYPE_ERROR(nst_format_error(
            "JSON: an object of type %s is not serializable", "s",
            TYPE_NAME(obj)));
        FAIL;
    }
}

typedef unsigned char uc;

static void dump_str(Nst_StrObj *str, Nst_OpErr *err)
{
    size_t unicode_bytes = 0;
    char *s_val = str->value;
    size_t s_len = str->len;
    const char *hex_digits = "0123456789abcdef";

    for ( size_t i = 0; i < s_len; i++ )
    {
        int res = nst_check_utf8_bytes((uc *)s_val + i, s_len - i);
        if ( res != 1 )
        {
            unicode_bytes++;
        }
        if ( res != -1 )
        {
            i += res - 1;
        }
    }

    expand_by(s_len + unicode_bytes * 5 + 2, err);
    EXCEPT_ERROR;

    str_buf[str_len++] = '"';
    for ( size_t i = 0; i < s_len; i++ )
    {
        int res = nst_check_utf8_bytes((uc *)s_val + i, s_len - i);
        switch ( res )
        {
        case 1:
            str_buf[str_len++] = s_val[i];
            break;
        case -1:
            append_buf("\\u00", 4, err);
            str_buf[str_len++] = hex_digits[(uc)(s_val[i]) >> 4];
            str_buf[str_len++] = hex_digits[(uc)(s_val[i]) & 0xf];
            break;
        case 2:
            append_buf("\\u0", 3, err);
            str_buf[str_len++] = hex_digits[(uc)(s_val[i]) >> 2 & 0xf];
            str_buf[str_len++] = hex_digits[(((uc)(s_val[i]) & 0x3) << 2)
                                          + (((uc)(s_val[i + 1]) >> 4) & 0x3)];
            i++;
            str_buf[str_len++] = hex_digits[(uc)(s_val[i]) & 0xf];
            break;
        case 3:
            append_buf("\\u", 2, err);
            str_buf[str_len++] = hex_digits[(uc)(s_val[i++]) & 0xf];
            str_buf[str_len++] = hex_digits[((uc)(s_val[i]) >> 2) & 0xf];
            str_buf[str_len++] = hex_digits[(((uc)(s_val[i]) & 0x3) << 2)
                                          + (((uc)(s_val[i + 1]) >> 4) & 0x3)];
            i++;
            str_buf[str_len++] = hex_digits[(uc)(s_val[i]) & 0xf];
            break;
        case 4:
            NST_SET_RAW_VALUE_ERROR(
                "JSON: cannot serialize characters above U+FFFF");
            FAIL;
        }
    }
    str_buf[str_len++] = '"';
}

static void dump_num(Nst_Obj *number, Nst_OpErr *err)
{
    if ( number->type == nst_t.Byte )
    {
        char loc_buf[4];
        sprintf(loc_buf, "%i", AS_BYTE(number));
        append_buf((const char *)loc_buf, 0, err);
    }
    else if ( number->type == nst_t.Int )
    {
        char loc_buf[21];
        sprintf(loc_buf, "%lli", AS_INT(number));
        append_buf((const char *)loc_buf, 0, err);
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
        char loc_buf[26];
        sprintf(loc_buf, "%.16lg", val);
        size_t len = strlen(loc_buf);
        for ( size_t i = 0; i < len; i++ )
        {
            if ( loc_buf[i] == '.' || loc_buf[i] == 'e' )
            {
                goto finish;
            }
        }
        loc_buf[len++] = '.';
        loc_buf[len++] = '0';
    finish:
        append_buf((const char *)loc_buf, len, err);
    }
}

static void dump_seq(Nst_SeqObj *seq, int indent, Nst_OpErr *err)
{
    if ( seq->len == 0 )
    {
        append_buf("[]", 2, err);
        return;
    }

    append_buf("[", 1, err);
    EXCEPT_ERROR;
    indent_level++;

    if ( indent > 1 )
    {
        expand_by(indent * indent_level + 1, err);
        EXCEPT_ERROR;
        str_buf[str_len++] = '\n';
        for ( int i = 0; i < indent * indent_level; i++ )
        {
            str_buf[str_len++] = ' ';
        }
    }

    for ( size_t i = 0, n = seq->len; i < n; i++ )
    {
        dump_obj(seq->objs[i], indent, err);
        EXCEPT_ERROR;

        if ( i + 1 == n )
        {
            indent_level--;
            expand_by(1, err);
            EXCEPT_ERROR;
            if ( indent < 1 )
            {
                str_buf[str_len++] = ']';
                return;
            }
            str_buf[str_len++] = '\n';
            add_indent(indent, err);
            EXCEPT_ERROR;
            expand_by(1, err);
            EXCEPT_ERROR;
            str_buf[str_len++] = ']';
            return;
        }

        add_comma(indent, err);
        EXCEPT_ERROR;
    }
}

static void dump_map(Nst_MapObj *map, int indent, Nst_OpErr *err)
{
    if ( map->item_count == 0 )
    {
        append_buf("{}", 2, err);
        return;
    }

    append_buf("{", 1, err);
    EXCEPT_ERROR;
    indent_level++;

    if ( indent > 1 )
    {
        expand_by(indent * indent_level + 1, err);
        EXCEPT_ERROR;
        str_buf[str_len++] = '\n';
        for ( int i = 0; i < indent * indent_level; i++ )
        {
            str_buf[str_len++] = ' ';
        }
    }

    size_t count = 0;
    size_t tot = map->item_count;
    Nst_MapNode *nodes = map->nodes;
    for ( int i = nst_map_get_next_idx(-1, map);
          i != -1;
          i = nst_map_get_next_idx(i, map) )
    {
        count++;
        Nst_Obj *key = nodes[i].key;
        Nst_Obj *value = nodes[i].value;
        if ( key->type != nst_t.Str )
        {
            NST_SET_RAW_TYPE_ERROR("JSON: all keys of a map must be strings");
            FAIL;
        }

        dump_str(STR(key), err);
        EXCEPT_ERROR;

        if ( indent == -1 )
        {
            append_buf(":", 1, err);
        }
        else
        {
            append_buf(": ", 2, err);
        }
        EXCEPT_ERROR;

        dump_obj(value, indent, err);
        EXCEPT_ERROR;

        if ( count == tot )
        {
            indent_level--;
            if ( indent < 1 )
            {
                append_buf("}", 1, err);
                return;
            }
            else
            {
                append_buf("\n", 1, err);
                EXCEPT_ERROR;
            }
            add_indent(indent, err);
            EXCEPT_ERROR;
            append_buf("}", 1, err);
            return;
        }

        add_comma(indent, err);
        EXCEPT_ERROR;
    }
}

static void add_comma(int indent, Nst_OpErr *err)
{
    if ( indent == -1 )
    {
        expand_by(1, err);
    }
    else
    {
        expand_by(2, err);
    }
    EXCEPT_ERROR;
    str_buf[str_len++] = ',';
    if ( indent == 0 )
    {
        str_buf[str_len++] = ' ';
    }
    else if ( indent > 0 )
    {
        str_buf[str_len++] = '\n';
        add_indent(indent, err);
        EXCEPT_ERROR;
    }
}

static void add_indent(int indent, Nst_OpErr *err)
{
    expand_by(indent * indent_level, err);
    EXCEPT_ERROR;
    for ( int i = 0; i < indent * indent_level; i++ )
    {
        str_buf[str_len++] = ' ';
    }
}
