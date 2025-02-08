#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "lib_import.h"
#include "iter.h"
#include "obj_ops.h"
#include "format.h"
#include "mem.h"

/*

Bit meaning of accepted_types:

| A | B  | C             |
| 0 | 00 | 0000000000000 |

Bit A: whether the cast is to a C type or between Nest types.
Sector B: the type of C cast
Sector C: any builtin type

Setting both B and C to 0 means accept anything
*/

typedef struct _MatchType {
    u16 accepted_types;
    usize custom_types_size;
    Nst_TypeObj **custom_types;
    Nst_TypeObj *final_type;
    struct _MatchType *seq_match;
} MatchType;

enum BuiltinIdx {
    INT_IDX    = 0b0000000000000001,
    REAL_IDX   = 0b0000000000000010,
    BYTE_IDX   = 0b0000000000000100,
    BOOL_IDX   = 0b0000000000001000,
    STR_IDX    = 0b0000000000010000,
    NULL_IDX   = 0b0000000000100000,
    ARRAY_IDX  = 0b0000000001000000,
    VECTOR_IDX = 0b0000000010000000,
    MAP_IDX    = 0b0000000100000000,
    ITER_IDX   = 0b0000001000000000,
    IOFILE_IDX = 0b0000010000000000,
    FUNC_IDX   = 0b0000100000000000,
    TYPE_IDX   = 0b0001000000000000,
    C_CAST     = 0b1000000000000000,

    INT_C_CAST = 0b0000000000000000,
    REAL_C_CAST= 0b0010000000000000,
    BYTE_C_CAST= 0b0100000000000000,
    BOOL_C_CAST= 0b0110000000000000
};

static Nst_Obj *allocated_objects;

static void destroy_match_type(MatchType *mt)
{
    if (mt->custom_types != NULL)
        Nst_free(mt->custom_types);
    Nst_free(mt);
}

static MatchType *compile_type_match(i8 *types, i8 **type_end, va_list *args,
                                     bool allow_casting)
{
    MatchType *match_type = Nst_malloc_c(1, MatchType);
    if (match_type == NULL)
        return NULL;

    Nst_SBuffer custom_types;
    if (!Nst_sbuffer_init(&custom_types, sizeof(Nst_TypeObj *), 4)) {
        Nst_free(match_type);
        return NULL;
    }

    match_type->final_type = NULL;
    match_type->seq_match = NULL;
    match_type->custom_types = NULL;

    bool allow_optional = true;
    bool allow_or = false;
    bool match_any = false;
    Nst_TypeObj *custom_type = NULL;
    i8 *t = (i8 *)types;
    u16 accepted_types = 0;
    u16 pending_cast_types = 0;
    u16 pending_c_cast = 0;
    Nst_TypeObj *pending_final_type = NULL;
    bool can_cast = true;

    while (true) {
        if (allow_or && *t != '|')
            break;

        switch (*t) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            t++;
            continue;
        case 't':
            accepted_types |= TYPE_IDX;
            goto normal_type;
        case 'i':
            accepted_types |= INT_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = INT_IDX;
                pending_c_cast = C_CAST | INT_C_CAST;
            }
            goto normal_type;
        case 'r':
            accepted_types |= REAL_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = REAL_IDX;
                pending_c_cast = C_CAST | REAL_C_CAST;
            }
            goto normal_type;
        case 'b':
            accepted_types |= BOOL_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = BOOL_IDX;
                pending_c_cast = C_CAST | BOOL_C_CAST;
            }
            goto normal_type;
        case 'n':
            accepted_types |= NULL_IDX;
            goto normal_type;
        case 's':
            accepted_types |= STR_IDX;
            goto normal_type;
        case 'v':
            accepted_types |= VECTOR_IDX;
            goto normal_type;
        case 'a':
            accepted_types |= ARRAY_IDX;
            goto normal_type;
        case 'm':
            accepted_types |= MAP_IDX;
            goto normal_type;
        case 'f':
            accepted_types |= FUNC_IDX;
            goto normal_type;
        case 'I':
            accepted_types |= ITER_IDX;
            goto normal_type;
        case 'B':
            accepted_types |= BYTE_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = BYTE_IDX;
                pending_c_cast = C_CAST | BYTE_C_CAST;
            }
            goto normal_type;
        case 'F':
            accepted_types |= IOFILE_IDX;
            goto normal_type;
        case 'o':
            match_any = true;
            goto normal_type;
        case 'l':
            accepted_types |= INT_IDX;
            accepted_types |= BYTE_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = INT_IDX | BYTE_IDX;
                pending_c_cast = C_CAST | INT_C_CAST;
            }
            goto normal_type;
        case 'N':
            accepted_types |= INT_IDX;
            accepted_types |= BYTE_IDX;
            accepted_types |= REAL_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = INT_IDX | BYTE_IDX | REAL_IDX;
                pending_c_cast = C_CAST | REAL_C_CAST;
            }
            goto normal_type;
        case 'A':
            accepted_types |= ARRAY_IDX;
            accepted_types |= VECTOR_IDX;
            goto normal_type;
        case 'S':
            accepted_types |= ARRAY_IDX;
            accepted_types |= VECTOR_IDX;
            accepted_types |= STR_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = ARRAY_IDX | VECTOR_IDX | STR_IDX;
                pending_final_type = Nst_t.Array;
            }
            goto normal_type;
        case 'R':
            accepted_types |= ARRAY_IDX;
            accepted_types |= VECTOR_IDX;
            accepted_types |= STR_IDX;
            accepted_types |= ITER_IDX;
            if (can_cast) {
                pending_cast_types = ARRAY_IDX | VECTOR_IDX
                                   | STR_IDX   | ITER_IDX;
                pending_final_type = Nst_t.Iter;
            }
            goto normal_type;
        case 'y':
            match_any = true;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = 0;
                pending_c_cast = C_CAST | BOOL_C_CAST;
            }
            goto normal_type;
        case '#':
            custom_type = va_arg(*args, Nst_TypeObj *);
            if (!Nst_sbuffer_append(&custom_types, &custom_type)) {
                Nst_sbuffer_destroy(&custom_types);
                Nst_free(match_type);
                return NULL;
            }
            goto normal_type;
        case '?':
            if (!allow_optional) {
                Nst_set_value_error_c(
                    _Nst_EM_INVALID_TYPE_LETTER("Nst_extract_args"));
                Nst_sbuffer_destroy(&custom_types);
                Nst_free(match_type);
                return NULL;
            }
            accepted_types |= NULL_IDX;
            allow_optional = false;
            allow_or = false;
            t++;
            continue;
        case '|':
            if (!allow_or) {
                Nst_set_value_error_c(
                    _Nst_EM_INVALID_TYPE_LETTER("Nst_extract_args"));
                Nst_sbuffer_destroy(&custom_types);
                Nst_free(match_type);
                return NULL;
            }
            allow_optional = true;
            allow_or = false;
            t++;
            continue;
        }
        if (!allow_or) {
            Nst_set_value_error_c(
                _Nst_EM_INVALID_TYPE_LETTER("Nst_extract_args"));
            Nst_sbuffer_destroy(&custom_types);
            Nst_free(match_type);
            return NULL;
        } else
            break;

    normal_type:
        allow_optional = false;
        allow_or = true;
        t++;
    }

    Nst_sbuffer_fit(&custom_types);
    match_type->custom_types = (Nst_TypeObj **)custom_types.data;
    match_type->custom_types_size = custom_types.len;

    if (allow_casting
        && (pending_c_cast != 0 || pending_final_type != NULL)
        && can_cast
        && accepted_types == pending_cast_types
        && custom_types.len == 0)
    {
        if (pending_c_cast)
            accepted_types |= pending_c_cast;
        else
            match_type->final_type = pending_final_type;
    }

    if (*t == '_') {
        if (!allow_casting || accepted_types & NULL_IDX) {
            Nst_set_value_error_c(
                _Nst_EM_INVALID_TYPE_LETTER("Nst_extract_args"));
            destroy_match_type(match_type);
            return NULL;
        }

        t++;
        accepted_types |= C_CAST;
        switch (*t++) {
        case 'i':
            accepted_types |= INT_C_CAST;
            break;
        case 'r':
            accepted_types |= REAL_C_CAST;
            break;
        case 'b':
            accepted_types |= BOOL_C_CAST;
            break;
        case 'B':
            accepted_types |= BYTE_C_CAST;
            break;
        default:
            Nst_set_value_error_c(
                _Nst_EM_INVALID_TYPE_LETTER("Nst_extract_args"));
            destroy_match_type(match_type);
            return NULL;
        }
    } else if (*t == ':') {
        if (!allow_casting) {
            Nst_set_value_error_c(
                _Nst_EM_INVALID_TYPE_LETTER("Nst_extract_args"));
            destroy_match_type(match_type);
            return NULL;
        }
        accepted_types &= ~C_CAST;

        t++;
        switch (*t++) {
        case 't': match_type->final_type = Nst_t.Type;   break;
        case 'i': match_type->final_type = Nst_t.Int;    break;
        case 'r': match_type->final_type = Nst_t.Real;   break;
        case 'b': match_type->final_type = Nst_t.Bool;   break;
        case 's': match_type->final_type = Nst_t.Str;    break;
        case 'v': match_type->final_type = Nst_t.Vector; break;
        case 'a': match_type->final_type = Nst_t.Array;  break;
        case 'm': match_type->final_type = Nst_t.Map;    break;
        case 'f': match_type->final_type = Nst_t.Func;   break;
        case 'I': match_type->final_type = Nst_t.Iter;   break;
        case 'B': match_type->final_type = Nst_t.Byte;   break;
        case 'F': match_type->final_type = Nst_t.IOFile; break;
        case 'o': match_type->final_type = Nst_t.Null;   break;
        default:
            Nst_set_value_error_c(
                _Nst_EM_INVALID_TYPE_LETTER("Nst_extract_args"));
            Nst_sbuffer_destroy(&custom_types);
            destroy_match_type(match_type);
            return NULL;
        }
    } else if (*t == '.' && (accepted_types & C_CAST))
        accepted_types &= ~C_CAST;

    if (*t == '.') {
        if (accepted_types & C_CAST) {
            Nst_set_value_error_c(
                _Nst_EM_INVALID_TYPE_LETTER("Nst_extract_args"));
            destroy_match_type(match_type);
            return NULL;
        }
        t++;
        match_type->seq_match = compile_type_match(t, type_end, args, false);
        if (match_type->seq_match == NULL) {
            destroy_match_type(match_type);
            return NULL;
        }
    } else {
        *type_end = t;
        match_type->seq_match = NULL;
    }

    if (match_any)
        accepted_types &= C_CAST | BOOL_C_CAST;

    match_type->accepted_types = accepted_types;

    // consume trailing whitespace to make Nst_extract_args check only
    // for NUL
    while (*t == ' ' || *t == '\t' || *t == '\n' || *t == '\r')
        t++;

    return match_type;
}

static bool check_type(MatchType *type, Nst_Obj *ob, void *arg)
{
    // Any object is type-checked, casted and content-checked in this order
    u16 accepted_types = type->accepted_types;
    Nst_TypeObj *ob_t = ob->type;
    Nst_TypeObj *final_type = type->final_type;

    if ((accepted_types & NULL_IDX) && ob == Nst_c.Null_null) {
        if (arg != NULL)
            *(Nst_Obj **)arg = Nst_c.Null_null;
        return true;
    }

    // If any object is accepted
    if ((accepted_types & 0x1fff) == 0 && type->custom_types_size == 0)
        goto cast_obj;

    // If the object's type is built-in but not accepted
    if (   (ob_t == Nst_t.Int    && !(accepted_types & INT_IDX   ))
        || (ob_t == Nst_t.Real   && !(accepted_types & REAL_IDX  ))
        || (ob_t == Nst_t.Byte   && !(accepted_types & BYTE_IDX  ))
        || (ob_t == Nst_t.Bool   && !(accepted_types & BOOL_IDX  ))
        || (ob_t == Nst_t.Str    && !(accepted_types & STR_IDX   ))
        || (ob_t == Nst_t.Array  && !(accepted_types & ARRAY_IDX ))
        || (ob_t == Nst_t.Vector && !(accepted_types & VECTOR_IDX))
        || (ob_t == Nst_t.Map    && !(accepted_types & MAP_IDX   ))
        || (ob_t == Nst_t.Iter   && !(accepted_types & ITER_IDX  ))
        || (ob_t == Nst_t.IOFile && !(accepted_types & IOFILE_IDX))
        || (ob_t == Nst_t.Func   && !(accepted_types & FUNC_IDX  ))
        || (ob_t == Nst_t.Type   && !(accepted_types & TYPE_IDX  )))
    {
        return false;
    }
    // If the object's type is built-in and is accepted
    else if (   ob_t == Nst_t.Int    || ob_t == Nst_t.Real
             || ob_t == Nst_t.Byte   || ob_t == Nst_t.Array
             || ob_t == Nst_t.Str    || ob_t == Nst_t.Bool
             || ob_t == Nst_t.Vector || ob_t == Nst_t.Map
             || ob_t == Nst_t.Iter   || ob_t == Nst_t.IOFile
             || ob_t == Nst_t.Func   || ob_t == Nst_t.Type)
    {
        goto cast_obj;
    }

    // Now, only if the object's type is custom it can be accepted
    for (usize i = type->custom_types_size; i > 0; i--) {
        if (ob_t == type->custom_types[i - 1])
            goto cast_obj;
    }

    return false;

cast_obj:
    if (final_type == NULL && !(accepted_types & C_CAST))
        goto content_check;

    if (!(accepted_types & C_CAST)) {
        if (final_type == Nst_t.Null) {
            Nst_inc_ref(ob);
            if (!Nst_vector_append(allocated_objects, ob)) {
                Nst_dec_ref(ob);
                return false;
            }
            goto content_check;
        }

        // Avoid casting arrays to vectors or vectors to arrays since they are
        // both Nst_SeqObj
        if ((final_type == Nst_t.Array || final_type == Nst_t.Vector)
            && (ob_t == Nst_t.Array || ob_t == Nst_t.Vector))
        {
            Nst_inc_ref(ob);
            if (!Nst_vector_append(allocated_objects, ob)) {
                Nst_dec_ref(ob);
                return false;
            }
            goto content_check;
        }

        Nst_Obj *res = Nst_obj_cast(ob, type->final_type);
        if (res == NULL)
            return false;
        ob = res;
        if (!Nst_vector_append(allocated_objects, ob)) {
            Nst_dec_ref(ob);
            return false;
        }
        goto content_check;
    }

    if ((accepted_types & 0x7000) == INT_C_CAST) {
        Nst_Obj *res = Nst_obj_cast(ob, Nst_t.Int);
        if (res == NULL)
            return false;
        if (arg != NULL)
            *(i64 *)arg = AS_INT(res);
        Nst_dec_ref(res);
        return true;
    } else if ((accepted_types & 0x7000) == REAL_C_CAST) {
        Nst_Obj *res = Nst_obj_cast(ob, Nst_t.Real);
        if (res == NULL)
            return false;
        if (arg != NULL)
            *(f64 *)arg = AS_REAL(res);
        Nst_dec_ref(res);
        return true;
    } else if ((accepted_types & 0x7000) == BYTE_C_CAST) {
        Nst_Obj *res = Nst_obj_cast(ob, Nst_t.Real);
        if (res == NULL)
            return false;
        if (arg != NULL)
            *(u8 *)arg = AS_BYTE(res);
        Nst_dec_ref(res);
        return true;
    } else if ((accepted_types & 0x7000) == BOOL_C_CAST) {
        Nst_Obj *res = Nst_obj_cast(ob, Nst_t.Bool);
        if (res == NULL)
            return false;
        if (arg != NULL)
            *(bool *)arg = AS_BOOL(res);
        Nst_dec_ref(res);
        return true;
    }

content_check:
    if (type->seq_match == NULL) {
        if (arg != NULL)
            *(Nst_Obj **)arg = ob;
        return true;
    }

    if (ob->type != Nst_t.Array && ob->type != Nst_t.Vector) {
        if (arg != NULL)
            *(Nst_Obj **)arg = ob;
        return true;
    }

    for (usize i = 0, n = Nst_seq_len(ob); i < n; i++) {
        if (!check_type(type->seq_match, Nst_seq_getnf(ob, i), arg)) {
            if (final_type != NULL)
                Nst_dec_ref(ob);
            return false;
        }
    }
    if (arg != NULL)
        *(Nst_Obj **)arg = ob;
    return true;
}

static bool append_type(Nst_StrView *type, Nst_Buffer *buf, usize tot_types)
{
    if (!Nst_buffer_expand_by(buf, type->len + 6)) {
        Nst_error_clear();
        return false;
    }

    Nst_buffer_append_char(buf, '\'');
    Nst_buffer_append_str(buf, type->value, type->len);
    Nst_buffer_append_char(buf, '\'');

    switch (tot_types) {
    case 0:
        Nst_buffer_append_char(buf, ' ');
        break;
    case 1:
        Nst_buffer_append_c_str(buf, " or ");
        break;
    default:
        Nst_buffer_append_c_str(buf, ", ");
        break;
    }

    return true;
}

static bool append_types(MatchType *type, Nst_Buffer *buf)
{
    u16 accepted_types = type->accepted_types;
    usize tot_types = 0;
    Nst_TypeObj *type_str;
    for (u16 i = 0; i < 13; i++) {
        // if the type is selected and is not Nst_NullIdx
        if (((accepted_types >> i) & 1) != 0 && i != 5)
            tot_types++;
    }
    tot_types += type->custom_types_size;

    for (u16 i = 0; i < 13; i++) {
        if (((accepted_types >> i) & 1) == 0 || i == 5)
            continue;
        tot_types--;
        switch (i) {
        case 0: type_str = Nst_t.Int; break;
        case 1: type_str = Nst_t.Real; break;
        case 2: type_str = Nst_t.Byte; break;
        case 3: type_str = Nst_t.Bool; break;
        case 4: type_str = Nst_t.Str; break;
        case 6: type_str = Nst_t.Array; break;
        case 7: type_str = Nst_t.Vector; break;
        case 8: type_str = Nst_t.Map; break;
        case 9: type_str = Nst_t.Iter; break;
        case 10: type_str = Nst_t.IOFile; break;
        case 11: type_str = Nst_t.Func; break;
        default: type_str = Nst_t.Type; break;
        }
        if (!append_type(&type_str->name, buf, tot_types))
            return false;
    }

    for (usize i = 0, n = type->custom_types_size; i < n; i++) {
        type_str = type->custom_types[i];
        tot_types--;
        if (!append_type(&type_str->name, buf, tot_types))
            return false;
    }

    if (type->seq_match != NULL) {
        if (!Nst_buffer_append_c_str(buf, "containing only ")) {
            Nst_error_clear();
            return false;
        }
        return append_types(type->seq_match, buf);
    }
    return true;
}

static void set_err(MatchType *type, Nst_Obj *ob, usize idx)
{
    const i8 *fmt;
    if (type->accepted_types & NULL_IDX) {
        fmt = "expected optional type %sfor argument %zi "
              "but got type '%s' instead";
    } else
        fmt = "expected type %sfor argument %zi but got type '%s' instead";

    Nst_Buffer buf;
    if (!Nst_buffer_init(&buf, 256) || !append_types(type, &buf)) {
        Nst_error_clear();
        Nst_buffer_destroy(&buf);
        Nst_failed_allocation();
        return;
    }

    Nst_Obj *str = Nst_sprintf(fmt, buf.data, idx, ob->type->name.value);
    Nst_buffer_destroy(&buf);
    Nst_set_type_error(str);
}

static void free_type_match(MatchType *type)
{
    if (type->seq_match)
        free_type_match(type->seq_match);
    if (type->custom_types != NULL)
        Nst_free(type->custom_types);
    Nst_free(type);
}

bool Nst_extract_args(const i8 *types, usize arg_num, Nst_Obj **args,
                            ...)
{
    va_list args_list;
    va_start(args_list, args);
    i8 *t = (i8 *)types;
    usize idx = 0;
    allocated_objects = Nst_vector_new(0);
    if (allocated_objects == NULL)
        return false;

    do {
        MatchType *type = compile_type_match(t, &t, &args_list, true);
        if (type == NULL) {
            va_end(args_list);
            for (usize i = 0, n = Nst_seq_len(allocated_objects); i < n; i++)
                Nst_dec_ref(Nst_seq_getnf(allocated_objects, n));
            Nst_dec_ref(allocated_objects);
            return false;
        }

        Nst_Obj *ob = args[idx++];
        void *arg = va_arg(args_list, void *);

        if (!check_type(type, ob, arg)) {
            set_err(type, ob, idx);
            free_type_match(type);
            for (usize i = 0, n = Nst_seq_len(allocated_objects); i < n; i++)
                Nst_dec_ref(Nst_seq_getnf(allocated_objects, n));
            Nst_dec_ref(allocated_objects);
            return false;
        }
        free_type_match(type);
    }
    while (*t != '\0');
    va_end(args_list);
    Nst_dec_ref(allocated_objects);

    if (idx != arg_num) {
        Nst_set_value_error_c(
            _Nst_EM_INVALID_TYPE_LETTER("Nst_extract_args"));
        return false;
    }
    return true;
}
