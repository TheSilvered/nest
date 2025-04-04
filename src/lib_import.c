#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "nest.h"

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
    Nst_Obj **custom_types;
    Nst_Obj *final_type;
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

static MatchType *compile_type_match(const char *types, const char **type_end,
                                     const char *full_types, va_list *args,
                                     bool allow_casting)
{
    MatchType *match_type = Nst_malloc_c(1, MatchType);
    if (match_type == NULL)
        return NULL;

    Nst_DynArray custom_types;
    if (!Nst_da_init(&custom_types, sizeof(Nst_Obj *), 4)) {
        Nst_free(match_type);
        return NULL;
    }

    match_type->final_type = NULL;
    match_type->seq_match = NULL;
    match_type->custom_types = NULL;

    bool allow_optional = true;
    bool allow_or = false;
    bool match_any = false;
    Nst_Obj *custom_type = NULL;
    u16 accepted_types = 0;
    u16 pending_cast_types = 0;
    u16 pending_c_cast = 0;
    Nst_Obj *pending_final_type = NULL;
    bool can_cast = true;

    while (true) {
        if (allow_or && *types != '|')
            break;

        switch (*types) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            types++;
            continue;
        case 't':
            accepted_types |= TYPE_IDX;
            break;
        case 'i':
            accepted_types |= INT_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = INT_IDX;
                pending_c_cast = C_CAST | INT_C_CAST;
            }
            break;
        case 'r':
            accepted_types |= REAL_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = REAL_IDX;
                pending_c_cast = C_CAST | REAL_C_CAST;
            }
            break;
        case 'b':
            accepted_types |= BOOL_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = BOOL_IDX;
                pending_c_cast = C_CAST | BOOL_C_CAST;
            }
            break;
        case 'n':
            accepted_types |= NULL_IDX;
            break;
        case 's':
            accepted_types |= STR_IDX;
            break;
        case 'v':
            accepted_types |= VECTOR_IDX;
            break;
        case 'a':
            accepted_types |= ARRAY_IDX;
            break;
        case 'm':
            accepted_types |= MAP_IDX;
            break;
        case 'f':
            accepted_types |= FUNC_IDX;
            break;
        case 'I':
            accepted_types |= ITER_IDX;
            break;
        case 'B':
            accepted_types |= BYTE_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = BYTE_IDX;
                pending_c_cast = C_CAST | BYTE_C_CAST;
            }
            break;
        case 'F':
            accepted_types |= IOFILE_IDX;
            break;
        case 'o':
            match_any = true;
            break;
        case 'l':
            accepted_types |= INT_IDX;
            accepted_types |= BYTE_IDX;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = INT_IDX | BYTE_IDX;
                pending_c_cast = C_CAST | INT_C_CAST;
            }
            break;
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
            break;
        case 'A':
            accepted_types |= ARRAY_IDX;
            accepted_types |= VECTOR_IDX;
            break;
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
            break;
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
            break;
        case 'y':
            match_any = true;
            if (pending_c_cast != 0 || pending_final_type != NULL)
                can_cast = false;

            if (can_cast) {
                pending_cast_types = 0;
                pending_c_cast = C_CAST | BOOL_C_CAST;
            }
            break;
        case '#':
            custom_type = va_arg(*args, Nst_Obj *);
            if (custom_type->type != Nst_t.Type) {
                Nst_error_setf_type(
                    "argument extraction: expected a 'Type' object at %zi, got"
                    " '%s'",
                    (types - full_types) + 1,
                    Nst_type_name(custom_type->type).value);
                Nst_da_clear(&custom_types, NULL);
                Nst_free(match_type);
                return NULL;
            }

            if (!Nst_da_append(&custom_types, &custom_type)) {
                Nst_da_clear(&custom_types, NULL);
                Nst_free(match_type);
                return NULL;
            }
            break;
        case '?':
            if (!allow_optional) {
                Nst_error_setf_value(
                    "argument extraction: '?' not allowed at %zi",
                    (types - full_types) + 1);
                Nst_da_clear(&custom_types, NULL);
                Nst_free(match_type);
                return NULL;
            }
            accepted_types |= NULL_IDX;
            allow_optional = false;
            allow_or = false;
            types++;
            continue;
        case '|':
            if (!allow_or) {
                Nst_error_setf_value(
                    "argument extraction: '|' not allowed at %zi",
                    (types - full_types) + 1);
                Nst_da_clear(&custom_types, NULL);
                Nst_free(match_type);
                return NULL;
            }
            allow_optional = true;
            allow_or = false;
            types++;
            continue;
        default:
            Nst_error_setf_value(
                "argument extraction: syntax error at %zi",
                (types - full_types) + 1);
            Nst_da_clear(&custom_types, NULL);
            Nst_free(match_type);
            return NULL;
        }

        allow_optional = false;
        allow_or = true;
        types++;
    }

    match_type->custom_types = (Nst_Obj **)custom_types.data;
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

    if (*types == '_') {
        if (!allow_casting || accepted_types & NULL_IDX) {
            Nst_error_setf_value(
                "argument extraction: casting is not allowed at %zi",
                (types - full_types) + 1);
            destroy_match_type(match_type);
            return NULL;
        }

        types++;
        accepted_types |= C_CAST;
        switch (*types++) {
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
            Nst_error_setf_value(
                "argument extraction: unknown C cast at %zi",
                (types - full_types) + 1);
            destroy_match_type(match_type);
            return NULL;
        }
    } else if (*types == ':') {
        if (!allow_casting) {
            Nst_error_setf_value(
                "argument extraction: casting is not allowed at %zi",
                (types - full_types) + 1);
            destroy_match_type(match_type);
            return NULL;
        }
        accepted_types &= ~C_CAST;

        types++;
        switch (*types++) {
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
            Nst_error_setf_value(
                "argument extraction: unknown cast at %zi",
                (types - full_types) + 1);
            Nst_da_clear(&custom_types, NULL);
            destroy_match_type(match_type);
            return NULL;
        }
    } else if (*types == '.' && (accepted_types & C_CAST))
        accepted_types &= ~C_CAST;

    if (*types == '.') {
        if (accepted_types & C_CAST) {
            Nst_error_setf_value(
                "argument extraction: sequence matching is not allowed with a "
                "C cast at %zi",
                (types - full_types) + 1);
            destroy_match_type(match_type);
            return NULL;
        }
        types++;
        match_type->seq_match = compile_type_match(
            types, type_end, full_types,
            args,
            false);
        if (match_type->seq_match == NULL) {
            destroy_match_type(match_type);
            return NULL;
        }
    } else {
        *type_end = types;
        match_type->seq_match = NULL;
    }

    if (match_any)
        accepted_types &= C_CAST | BOOL_C_CAST;

    match_type->accepted_types = accepted_types;

    // consume trailing whitespace to make Nst_extract_args check only
    // for NUL
    while (*types == ' ' || *types == '\t' || *types == '\n' || *types == '\r')
        types++;

    return match_type;
}

static bool check_type(MatchType *type, Nst_Obj *ob, void *arg)
{
    // Any object is type-checked, casted and content-checked in this order
    u16 accepted_types = type->accepted_types;
    Nst_Obj *ob_t = ob->type;
    Nst_Obj *final_type = type->final_type;

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
            *(i64 *)arg = Nst_int_i64(res);
        Nst_dec_ref(res);
        return true;
    } else if ((accepted_types & 0x7000) == REAL_C_CAST) {
        Nst_Obj *res = Nst_obj_cast(ob, Nst_t.Real);
        if (res == NULL)
            return false;
        if (arg != NULL)
            *(f64 *)arg = Nst_real_f64(res);
        Nst_dec_ref(res);
        return true;
    } else if ((accepted_types & 0x7000) == BYTE_C_CAST) {
        Nst_Obj *res = Nst_obj_cast(ob, Nst_t.Real);
        if (res == NULL)
            return false;
        if (arg != NULL)
            *(u8 *)arg = Nst_byte_u8(res);
        Nst_dec_ref(res);
        return true;
    } else if ((accepted_types & 0x7000) == BOOL_C_CAST) {
        Nst_Obj *res = Nst_obj_cast(ob, Nst_t.Bool);
        if (res == NULL)
            return false;
        if (arg != NULL)
            *(bool *)arg = res == Nst_c.Bool_true;
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

static bool append_type(Nst_StrView type, Nst_StrBuilder *sb, usize tot_types)
{
    if (!Nst_sb_reserve(sb, type.len + 6)) {
        Nst_error_clear();
        return false;
    }

    Nst_sb_push_char(sb, '\'');
    Nst_sb_push_sv(sb, type);
    Nst_sb_push_char(sb, '\'');

    switch (tot_types) {
    case 0:
        Nst_sb_push_char(sb, ' ');
        break;
    case 1:
        Nst_sb_push_c(sb, " or ");
        break;
    default:
        Nst_sb_push_c(sb, ", ");
        break;
    }

    return true;
}

static bool append_types(MatchType *type, Nst_StrBuilder *sb)
{
    u16 accepted_types = type->accepted_types;
    usize tot_types = 0;
    Nst_Obj *type_str;
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
        if (!append_type(Nst_type_name(type_str), sb, tot_types))
            return false;
    }

    for (usize i = 0, n = type->custom_types_size; i < n; i++) {
        type_str = type->custom_types[i];
        tot_types--;
        if (!append_type(Nst_type_name(type_str), sb, tot_types))
            return false;
    }

    if (type->seq_match != NULL) {
        if (!Nst_sb_push_c(sb, "containing only ")) {
            Nst_error_clear();
            return false;
        }
        return append_types(type->seq_match, sb);
    }
    return true;
}

static void set_err(MatchType *type, Nst_Obj *ob, usize idx)
{
    const char *fmt;
    if (type->accepted_types & NULL_IDX) {
        fmt = "expected optional type %sfor argument %zi "
              "but got type '%s' instead";
    } else
        fmt = "expected type %sfor argument %zi but got type '%s' instead";

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, 256) || !append_types(type, &sb)) {
        Nst_error_clear();
        Nst_sb_destroy(&sb);
        Nst_error_failed_alloc();
        return;
    }

    Nst_Obj *str = Nst_sprintf(
        fmt,
        sb.value,
        idx,
        Nst_type_name(ob->type).value);
    Nst_sb_destroy(&sb);
    Nst_error_set_type(str);
}

static void free_type_match(MatchType *type)
{
    if (type->seq_match)
        free_type_match(type->seq_match);
    if (type->custom_types != NULL)
        Nst_free(type->custom_types);
    Nst_free(type);
}

bool Nst_extract_args(const char *types, usize arg_num, Nst_Obj **args, ...)
{
    va_list args_list;
    va_start(args_list, args);
    const char *tp = types;
    usize idx = 0;
    allocated_objects = Nst_vector_new(0);
    if (allocated_objects == NULL)
        return false;

    do {
        MatchType *type = compile_type_match(tp, &tp, types, &args_list, true);
        if (type == NULL) {
            va_end(args_list);
            for (usize i = 0, n = Nst_seq_len(allocated_objects); i < n; i++)
                Nst_dec_ref(Nst_seq_getnf(allocated_objects, n));
            Nst_dec_ref(allocated_objects);
            return false;
        }

        if (idx == arg_num) {
            Nst_error_setc_value("argument extraction: too few arguments");
            free_type_match(type);
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
    while (*tp != '\0');
    va_end(args_list);
    Nst_dec_ref(allocated_objects);

    if (idx != arg_num) {
        Nst_error_setc_value("argument extraction: too many arguments");
        return false;
    }
    return true;
}

Nst_Obj *_Nst_obj_custom(usize size, void *data, const char *name)
{
    return _Nst_obj_custom_ex(size, data, name, NULL);
}

Nst_Obj *_Nst_obj_custom_ex(usize size, void *data, const char *name,
                            Nst_ObjDstr dstr)
{
    Nst_Obj *type = Nst_type_new(name, dstr);
    if (type == NULL)
        return NULL;
    Nst_Obj *obj = _Nst_obj_alloc(sizeof(Nst_Obj) + size, type);
    Nst_dec_ref(type);
    if (obj == NULL)
        return NULL;
    memcpy(obj + 1, data, size);
    return obj;
}

void *Nst_obj_custom_data(Nst_Obj *obj)
{
    return (void *)(obj + 1);
}
