#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "nest.h"

#ifdef Nst_MSVC

#include <windows.h>
#define dlsym GetProcAddress
#define dlopen LoadLibraryA
#define dlclose FreeLibrary
typedef HMODULE lib_t;

#define PATH_MAX 4096

#else

#include <dlfcn.h>
typedef void * lib_t;
#define dlopen(lib) dlopen(lib, RTLD_LAZY)

#endif // !__cplusplus

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

Nst_ObjRef *_Nst_obj_custom(usize size, void *data, const char *name)
{
    return _Nst_obj_custom_ex(size, data, name, NULL);
}

Nst_ObjRef *_Nst_obj_custom_ex(usize size, void *data, const char *name,
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

static Nst_DynArray lib_paths = { 0 };
static Nst_DynArray loaded_libs = { 0 };
static Nst_Obj *lib_handles = NULL;

static void close_lib(lib_t *lib)
{
    dlclose(*lib);
}

bool _Nst_import_init(void)
{
    lib_handles = Nst_map_new();
    if (lib_handles == NULL)
        return false;

    if (!Nst_da_init(&lib_paths, sizeof(Nst_Obj *), 10))
        return false;
    if (!Nst_da_init(&loaded_libs, sizeof(lib_t), 20))
        return false;
    return true;
}

void _Nst_import_quit(void)
{
    Nst_ndec_ref(lib_handles);
    Nst_da_clear_p(&lib_paths, (Nst_Destructor)Nst_dec_ref);

    for (usize i = 0; i < loaded_libs.len; i++) {
        lib_t *lib = Nst_da_get(&loaded_libs, i);
        void (*lib_quit)(void) = (void (*)(void))dlsym(*lib, "lib_quit");
        if (lib_quit != NULL)
            lib_quit();
    }
}

void _Nst_import_close_libs(void)
{
    Nst_da_clear(&loaded_libs, (Nst_Destructor)close_lib);
}

bool _Nst_import_push_path(Nst_ObjRef *path)
{
    return Nst_da_append(&lib_paths, &path);
}

void _Nst_import_pop_path(void)
{
    Nst_da_pop_p(&lib_paths, (Nst_Destructor)Nst_dec_ref);
}

void _Nst_import_clear_paths(void)
{
    Nst_da_clear(&lib_paths, (Nst_Destructor)Nst_dec_ref);
}

static Nst_Obj *import_nest_lib(Nst_Obj *file_path);
static Nst_Obj *import_c_lib(Nst_Obj *file_path);

Nst_ObjRef *Nst_import_lib(const char *path)
{
    bool c_import = false;
    usize path_len = strlen(path);

    if (path_len > 6 && path[0] == '_' && path[1] == '_' && path[2] == 'C'
        && path[3] == '_' && path[4] == '_' && path[5] == ':')
    {
        c_import = true;
        path += 6; // skip __C__:
        path_len -= 6;
    }

    Nst_Obj *import_path = _Nst_get_import_path(path, path_len);
    if (import_path == NULL)
        return NULL;

    // Check if the module is in the import stack
    for (usize i = 0; i < lib_paths.len; i++) {
        Nst_Obj *lib_path = NstOBJ(Nst_da_get_p(&lib_paths, i));
        if (Nst_str_compare(import_path, lib_path) == 0) {
            Nst_dec_ref(import_path);
            Nst_error_setc_import("circular import");
            return NULL;
        }
    }

    Nst_Obj *obj_map = Nst_map_get(lib_handles, import_path);
    if (obj_map != NULL) {
        Nst_dec_ref(import_path);
        return obj_map;
    }

    if (!_Nst_import_push_path(import_path)) {
        Nst_dec_ref(import_path);
        return NULL;
    }

    if (c_import)
        obj_map = import_c_lib(import_path);
    else
        obj_map = import_nest_lib(import_path);
    _Nst_import_pop_path();

    return obj_map;
}

static Nst_Obj *import_nest_lib(Nst_Obj *file_path)
{
    Nst_Obj *map = Nst_run_module((const char *)Nst_str_value(file_path));
    if (map == NULL)
        return NULL;

    if (!Nst_map_set(lib_handles, file_path, map)) {
        Nst_dec_ref(map);
        return NULL;
    }
    return map;
}

static Nst_Obj *import_c_lib(Nst_Obj *file_path)
{
    void (*lib_quit_func)();
    lib_t lib = dlopen((const char *)Nst_str_value(file_path));

    if (!lib) {
        Nst_dec_ref(file_path);
#ifdef Nst_MSVC
        Nst_error_setc_import("the file is not a valid DLL");
#else
        Nst_error_setc_import(dlerror());
#endif // !__cplusplus
        return NULL;
    }

    // Initialize library
    Nst_Declr *(*lib_init)() = (Nst_Declr *(*)())dlsym(lib, "lib_init");
    if (lib_init == NULL) {
        Nst_dec_ref(file_path);
        Nst_error_setc_import(
            "the library does not specify a 'lib_init' function");
        dlclose(lib);
        return NULL;
    }

    Nst_Declr *obj_ptrs = lib_init();

    if (obj_ptrs == NULL) {
        Nst_dec_ref(file_path);
        dlclose(lib);
        if (!Nst_error_occurred())
            Nst_error_setc_import("the module failed to initialize");
        return NULL;
    }

    // Populate the function map
    Nst_Obj *obj_map = Nst_map_new();
    if (obj_map == NULL)
        goto fail;

    for (usize i = 0; obj_ptrs[i].ptr != NULL; i++) {
        Nst_Declr obj_declr = obj_ptrs[i];
        Nst_Obj *obj;
        if (obj_declr.arg_num >= 0) {
            obj = Nst_func_new_c(
                obj_declr.arg_num,
                (Nst_NestCallable)obj_declr.ptr);
        } else
            obj = ((Nst_ConstFunc)obj_declr.ptr)();

        if (obj == NULL) {
            Nst_dec_ref(obj_map);
            goto fail;
        }

        if (!Nst_map_set_str(obj_map, obj_declr.name, obj)) {
            Nst_dec_ref(obj_map);
            Nst_dec_ref(obj);
            goto fail;
        }

        Nst_dec_ref(obj);
    }

    if (!Nst_da_append(&loaded_libs, &lib)) {
        Nst_dec_ref(obj_map);
        goto fail;
    }

    if (!Nst_map_set(lib_handles, file_path, obj_map)) {
        Nst_dec_ref(obj_map);
        return NULL;
    }
    return obj_map;
fail:
    lib_quit_func = (void (*)())dlsym(lib, "lib_quit");
    if (lib_quit_func)
        lib_quit_func();
    dlclose(lib);
    return NULL;
}

static Nst_Obj *search_local_directory(const char *initial_path)
{
    char *file_path;
    usize new_len = Nst_get_full_path(initial_path, &file_path, NULL);
    if (file_path == NULL)
        return NULL;

    FILE *file = Nst_fopen_unicode(file_path, "rb");
    if (file == NULL) {
        Nst_free(file_path);
        return NULL;
    }
    fclose(file);
    return Nst_str_new_allocated((u8 *)file_path, new_len);
}

static Nst_Obj *rel_path_to_abs_path_str_if_found(u8 *file_path)
{
    FILE *file = Nst_fopen_unicode((const char *)file_path, "rb");
    if (file == NULL) {
        Nst_free(file_path);
        return NULL;
    }
    fclose(file);

    u8 *abs_path;
    usize abs_path_len = Nst_get_full_path(
        (const char *)file_path,
        (char **)&abs_path,
        NULL);
    Nst_free(file_path);

    if (abs_path == NULL)
        return NULL;
    return Nst_str_new_allocated(abs_path, abs_path_len);
}

#if defined(_DEBUG) && defined(Nst_MSVC)

static Nst_Obj *search_debug_directory(const char *initial_path,
                                       usize path_len)
{
    // little hack to get the absolute path without using it explicitly
    const char *root_path = __FILE__;
    const char *obj_ops_path_suffix = "src\\lib_import.c";
    const char *nest_files = "libs\\_nest_files\\";
    usize root_len = strlen(root_path) - strlen(obj_ops_path_suffix);
    usize nest_files_len = strlen(nest_files);
    usize full_size = root_len + nest_files_len + path_len;

    u8 *file_path = Nst_malloc_c(full_size + 1, u8);
    if (file_path == NULL)
        return NULL;

    memcpy(file_path, root_path, root_len);
    memcpy(file_path + root_len, nest_files, nest_files_len);
    memcpy(file_path + root_len + nest_files_len, initial_path, path_len);
    file_path[full_size] = '\0';

    return rel_path_to_abs_path_str_if_found(file_path);
}

#endif

static Nst_Obj *search_stdlib_directory(const char *initial_path,
                                        usize path_len)
{
#if defined(_DEBUG) && defined(Nst_MSVC)
    return search_debug_directory(initial_path, path_len);
#else
#ifdef Nst_MSVC

    u8 *appdata = getenv("LOCALAPPDATA");
    if (appdata == NULL) {
        Nst_error_failed_alloc();
        return NULL;
    }
    usize appdata_len = strlen(appdata);
    const char *nest_files = "\\Programs\\nest\\nest_libs\\";
    usize nest_files_len = strlen(nest_files);
    usize tot_len = appdata_len + nest_files_len + path_len;

    u8 *file_path = Nst_malloc_c(tot_len + 1, u8);
    if (file_path == NULL)
        return NULL;
    sprintf(file_path, "%s%s%s", appdata, nest_files, initial_path);

#else

    const char *nest_files = "/usr/lib/nest/";
    usize nest_files_len = strlen(nest_files);
    usize tot_len = nest_files_len + path_len;

    u8 *file_path = Nst_malloc_c(tot_len + 1, u8);
    if (file_path == NULL)
        return NULL;
    sprintf((char *)file_path, "%s%s", nest_files, initial_path);

#endif // !Nst_MSVC

    return rel_path_to_abs_path_str_if_found(file_path);
#endif
}

Nst_Obj *_Nst_get_import_path(const char *initial_path, usize path_len)
{
    Nst_Obj *full_path = search_local_directory(initial_path);
    if (full_path != NULL)
        return full_path;
    else if (Nst_error_occurred())
        return NULL;

    full_path = search_stdlib_directory(initial_path, path_len);

    if (Nst_error_occurred())
        return NULL;
    else if (full_path == NULL) {
        Nst_error_setf_value("file '%.4096s' not found", initial_path);
        return NULL;
    }
    return full_path;
}

usize Nst_get_full_path(const char *file_path, char **buf, char **file_part)
{
    *buf = NULL;
    if (file_part != NULL)
        *file_part = NULL;

#ifdef Nst_MSVC
    wchar_t *wide_full_path = Nst_malloc_c(PATH_MAX, wchar_t);
    if (wide_full_path == NULL)
        return 0;
    wchar_t *wide_file_path = Nst_char_to_wchar_t(file_path, 0);
    if (wide_file_path == NULL) {
        Nst_free(wide_full_path);
        return 0;
    }

    DWORD full_path_len = GetFullPathNameW(
        wide_file_path,
        PATH_MAX,
        wide_full_path,
        NULL);

    if (full_path_len == 0) {
        Nst_free(wide_full_path);
        Nst_free(wide_file_path);
        Nst_error_setf_value("file '%.4096s' not found", file_path);
        return 0;
    }

    if (full_path_len > PATH_MAX) {
        Nst_free(wide_full_path);
        wide_full_path = Nst_malloc_c(full_path_len + 1, wchar_t);
        if (wide_full_path == NULL) {
            Nst_free(wide_file_path);
            return 0;
        }
        full_path_len = GetFullPathNameW(
            wide_file_path,
            full_path_len + 1,
            wide_full_path,
            NULL);

        if (full_path_len == 0) {
            Nst_free(wide_full_path);
            Nst_free(wide_file_path);
            Nst_error_setf_value("file '%.4096s' not found", file_path);
            return 0;
        }
    }
    Nst_free(wide_file_path);

    char *full_path = Nst_wchar_t_to_char(wide_full_path, full_path_len);
    Nst_free(wide_full_path);
    if (full_path == NULL)
        return 0;

    if (file_part != NULL) {
        *file_part = strrchr(full_path, '\\');

        if (*file_part == NULL)
            *file_part = full_path;
        else
            (*file_part)++;
    }

    *buf = full_path;
    return full_path_len;

#else

    char *path = Nst_malloc_c(PATH_MAX, char);
    if (path == NULL)
        return 0;

    char *result = realpath(file_path, path);

    if (result == NULL) {
        Nst_free(path);
        return 0;
    }

    if (file_part != NULL) {
        *file_part = strrchr(path, '/');

        if (*file_part == NULL)
            *file_part = path;
        else
            (*file_part)++;
    }

    *buf = path;
    return strlen(path);
#endif // !Nst_MSVC
}

