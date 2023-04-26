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
Sector B: the size of the custom_types array
Sector C: any builtin type

Setting both B and C to 0 means accept anything
*/

typedef struct _MatchType
{
    u16 accepted_types;
    Nst_TypeObj *custom_types[3];
    Nst_TypeObj *final_type;
    struct _MatchType *seq_match;
}
MatchType;

enum BuiltinIdx
{
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

    INT_C_CAST   = 1,
    REAL_C_CAST  = 2,
    BYTE_C_CAST  = 3,
    BOOL_C_CAST  = 4,
    STR_C_CAST   = 5
};

static MatchType *compile_type_match(i8        *types,
                                     i8       **type_end,
                                     va_list   *args,
                                     bool       allow_auto_casting,
                                     Nst_OpErr *err)
{
    /*
    Usage of the 'types' argument:

    Builtin types
     't': type
     'i': integer
     'r': real
     'b': bool
     'n': null
     's': string
     'v': vector
     'a': array
     'm': map
     'f': func
     'I': iter
     'B': byte
     'F': file

    Shorthands
     'l': i|B_i
     'N': i|r|B_r
     'A': a|v
     'S': a|v|s:a
     'R': I|a|v|s:I

    Other:
     'o': any object

    Custom types:
     You can have up to 3 custom types in a single argument and you should label
     them with #.

    Optional types:
     To have an optional type you can use ? before the type itself. Using |n is
     the same thing.

    Multiple types per argument:
     To have multiple possible types an argument can be you can use a pipe (|)
     between them.

    Automatic type casting:
     After the type specified you can add : or _ followed by exactly one letter.
     : is a cast between Nest objects, _ is a cast to a C type. When using the
     latter there cannot be any optional or custom types and it is restricted to
     only these types after the underscore: i, r, b, B or s.

    Implicit casting:
     If a type is specified as only one of i, r, b or B it automatically becomes
     i_i, r_r, b_b or B_B if it is not used to check the contents of a sequence
     To get the object itself use i:i, r:r, b:b, B:B and then immediatly
     decrease the reference (it is safe in this case since no new objects are
     created)

    Sequence type checking:
     You can additionally check the types present inside the matched sequence
     By following the type with a dot

    Example:
     i|r|B_B?A.#|i -> An Int, Real, Byte all casted to a Nst_Byte followed by an
                   optional Array or Vector that, if it exists, should contain
                   only objects of a custom type or integers.
    */

    MatchType *match_type = (MatchType *)nst_malloc(1, sizeof(MatchType), err);
    if ( match_type == NULL )
    {
        return NULL;
    }
    match_type->final_type = NULL;
    match_type->seq_match = NULL;
    match_type->custom_types[0] = NULL;
    match_type->custom_types[1] = NULL;
    match_type->custom_types[2] = NULL;

    bool allow_optional = true;
    bool allow_or = false;
    bool match_any = false;
    int custom_type_count = 0;
    Nst_TypeObj *custom_type = NULL;
    i8 *t = (i8 *)types;
    u16 accepted_types = 0;

    while ( true )
    {
        if ( allow_or && *t != '|' )
        {
            break;
        }

        switch ( *t )
        {
        case 't':
            accepted_types |= TYPE_IDX;
            goto normal_type;
        case 'i':
            accepted_types |= INT_IDX;
            goto normal_type;
        case 'r':
            accepted_types |= REAL_IDX;
            goto normal_type;
        case 'b':
            accepted_types |= BOOL_IDX;
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
            accepted_types |= C_CAST;
            match_type->final_type = (Nst_TypeObj *)INT_C_CAST;
            goto normal_type;
        case 'N':
            accepted_types |= INT_IDX;
            accepted_types |= BYTE_IDX;
            accepted_types |= REAL_IDX;
            accepted_types |= C_CAST;
            match_type->final_type = (Nst_TypeObj *)REAL_C_CAST;
            goto normal_type;
        case 'A':
            accepted_types |= ARRAY_IDX;
            accepted_types |= VECTOR_IDX;
            goto normal_type;
        case 'S':
            accepted_types |= ARRAY_IDX;
            accepted_types |= VECTOR_IDX;
            accepted_types |= STR_IDX;
            match_type->final_type = nst_t.Array;
            goto normal_type;
        case 'R':
            accepted_types |= ARRAY_IDX;
            accepted_types |= VECTOR_IDX;
            accepted_types |= STR_IDX;
            accepted_types |= ITER_IDX;
            match_type->final_type = nst_t.Iter;
            goto normal_type;

        case '#':
            if ( custom_type_count == 3 )
            {
                NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
                free(match_type);
                return NULL;
            }
            custom_type = va_arg(*args, Nst_TypeObj *);
            match_type->custom_types[custom_type_count] = custom_type;
            custom_type_count++;
            goto normal_type;

        case '?':
            if ( !allow_optional )
            {
                NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
                free(match_type);
                return NULL;
            }
            accepted_types |= NULL_IDX;
            allow_optional = false;
            allow_or = false;
            t++;
            continue;
        case '|':
            if ( !allow_or )
            {
                NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
                free(match_type);
                return NULL;
            }
            allow_optional = true;
            allow_or = false;
            t++;
            continue;
        }
        if ( !allow_or )
        {
            NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
            free(match_type);
            return NULL;
        }
        else
        {
            break;
        }

    normal_type:
        allow_optional = false;
        allow_or = true;
        t++;
    }

    if ( *t == '_' )
    {
        if ( !allow_auto_casting || accepted_types & NULL_IDX )
        {
            NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
            free(match_type);
            return NULL;
        }

        t++;
        accepted_types |= C_CAST;
        switch (*t++)
        {
        case 'i':
            match_type->final_type = (Nst_TypeObj *)INT_C_CAST;
            break;
        case 'r':
            match_type->final_type = (Nst_TypeObj *)REAL_C_CAST;
            break;
        case 'b':
            match_type->final_type = (Nst_TypeObj *)BOOL_C_CAST;
            break;
        case 'B':
            match_type->final_type = (Nst_TypeObj *)BYTE_C_CAST;
            break;
        case 's':
            match_type->final_type = (Nst_TypeObj *)STR_C_CAST;
            break;
        default:
            NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
            free(match_type);
            return NULL;
        }
    }
    else if ( *t == ':' )
    {
        if ( !allow_auto_casting )
        {
            NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
            free(match_type);
            return NULL;
        }

        switch (*t++)
        {
        case 't':
            match_type->final_type = nst_t.Type;
            break;
        case 'i':
            match_type->final_type = nst_t.Int;
            break;
        case 'r':
            match_type->final_type = nst_t.Real;
            break;
        case 'b':
            match_type->final_type = nst_t.Bool;
            break;
        case 'n':
            match_type->final_type = nst_t.Null;
            break;
        case 's':
            match_type->final_type = nst_t.Str;
            break;
        case 'v':
            match_type->final_type = nst_t.Vector;
            break;
        case 'a':
            match_type->final_type = nst_t.Array;
            break;
        case 'm':
            match_type->final_type = nst_t.Map;
            break;
        case 'f':
            match_type->final_type = nst_t.Func;
            break;
        case 'I':
            match_type->final_type = nst_t.Iter;
            break;
        case 'B':
            match_type->final_type = nst_t.Byte;
            break;
        case 'F':
            match_type->final_type = nst_t.IOFile;
            break;
        default:
            NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
            free(match_type);
            return NULL;
        }
    }
    else if ( *t != '.' && allow_auto_casting )
    {
        if ( accepted_types == INT_IDX )
        {
            match_type->final_type = (Nst_TypeObj *)INT_C_CAST;
            accepted_types |= C_CAST;
        }
        else if ( accepted_types == REAL_IDX )
        {
            match_type->final_type = (Nst_TypeObj *)REAL_C_CAST;
            accepted_types |= C_CAST;
        }
        else if ( accepted_types == BYTE_IDX )
        {
            match_type->final_type = (Nst_TypeObj *)BYTE_C_CAST;
            accepted_types |= C_CAST;
        }
        else if ( accepted_types == BOOL_IDX )
        {
            match_type->final_type = (Nst_TypeObj *)BOOL_C_CAST;
            accepted_types |= C_CAST;
        }
    }

    if ( *t == '.' )
    {
        if ( accepted_types & C_CAST )
        {
            NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
            free(match_type);
            return NULL;
        }
        t++;
        match_type->seq_match = compile_type_match(t, type_end, args, false, err);
        if ( match_type->seq_match == NULL )
        {
            nst_free(match_type);
            return NULL;
        }
    }
    else
    {
        *type_end = t;
        match_type->seq_match = NULL;
    }

    if ( match_any )
    {
        accepted_types &= C_CAST;
    }
    else
    {
        accepted_types |= custom_type_count << 13;
    }
    match_type->accepted_types = accepted_types;

    return match_type;
}

static bool check_type(MatchType *type,
                       Nst_OpErr *err,
                       Nst_Obj   *ob,
                       void      *arg)
{
    // Any object is type-checked, casted and content-checked in this order
    u16 accepted_types = type->accepted_types;
    Nst_TypeObj *ob_t = ob->type;
    Nst_TypeObj *final_type = type->final_type;

    if ( accepted_types & NULL_IDX && ob == nst_c.Null_null )
    {
        *(Nst_Obj **)arg = nst_c.Null_null;
        return true;
    }

    // If any object is accepted
    if ( accepted_types == (accepted_types & C_CAST) )
    {
        goto cast_obj;
    }

    // If the object's type is built-in but not accepted
    if ( (ob_t == nst_t.Int    && !(accepted_types & INT_IDX   )) ||
         (ob_t == nst_t.Real   && !(accepted_types & REAL_IDX  )) ||
         (ob_t == nst_t.Byte   && !(accepted_types & BYTE_IDX  )) ||
         (ob_t == nst_t.Bool   && !(accepted_types & BOOL_IDX  )) ||
         (ob_t == nst_t.Str    && !(accepted_types & STR_IDX   )) ||
         (ob_t == nst_t.Array  && !(accepted_types & ARRAY_IDX )) ||
         (ob_t == nst_t.Vector && !(accepted_types & VECTOR_IDX)) ||
         (ob_t == nst_t.Map    && !(accepted_types & MAP_IDX   )) ||
         (ob_t == nst_t.Iter   && !(accepted_types & ITER_IDX  )) ||
         (ob_t == nst_t.IOFile && !(accepted_types & IOFILE_IDX)) ||
         (ob_t == nst_t.Func   && !(accepted_types & FUNC_IDX  )) ||
         (ob_t == nst_t.Type   && !(accepted_types & TYPE_IDX  )) )
    {
        return false;
    }
    // If the object's type is built-in and is accepted
    else if ( ob_t == nst_t.Int    || ob_t == nst_t.Real || ob_t == nst_t.Byte ||
              ob_t == nst_t.Array  || ob_t == nst_t.Str  || ob_t == nst_t.Bool ||
              ob_t == nst_t.Vector || ob_t == nst_t.Map  || ob_t == nst_t.Iter ||
              ob_t == nst_t.IOFile || ob_t == nst_t.Func || ob_t == nst_t.Type )
    {
        goto cast_obj;
    }

    // Now, only if the object's type is custom it can be accepted
    for (int i = accepted_types >> 13 & 0b11; i > 0; i--)
    {
        if ( ob_t == type->custom_types[i - 1] )
        {
            goto cast_obj;
        }
    }

    return false;

cast_obj:
    if ( final_type == NULL )
    {
        goto content_check;
    }

    if ( !(accepted_types & C_CAST) )
    {
        // Avoid casting arrays to vectors or vectors to arrays since they are
        // both Nst_SeqObj
        if ( (final_type == nst_t.Array || final_type == nst_t.Vector) &&
             (ob_t == nst_t.Array || ob_t == nst_t.Vector)                )
        {
            nst_inc_ref(ob);
            goto content_check;
        }

        Nst_Obj *res = nst_obj_cast(ob, type->final_type, err);
        if ( res == NULL )
        {
            return false;
        }
        ob = res;
        goto content_check;
    }

    if ( (usize)final_type == INT_C_CAST )
    {
        Nst_Obj *res = nst_obj_cast(ob, nst_t.Int, err);
        if ( res == NULL )
        {
            return false;
        }
        *(Nst_Int *)arg = AS_INT(res);
        nst_dec_ref(res);
        return true;
    }
    else if ( (usize)final_type == REAL_C_CAST )
    {
        Nst_Obj *res = nst_obj_cast(ob, nst_t.Real, err);
        if ( res == NULL )
        {
            return false;
        }
        *(Nst_Real *)arg = AS_REAL(res);
        nst_dec_ref(res);
        return true;
    }
    else if ( (usize)final_type == BYTE_C_CAST )
    {
        Nst_Obj *res = nst_obj_cast(ob, nst_t.Real, err);
        if ( res == NULL )
        {
            return false;
        }
        *(Nst_Byte *)arg = AS_BYTE(res);
        nst_dec_ref(res);
        return true;
    }
    else if ( (usize)final_type == BOOL_C_CAST )
    {
        Nst_Obj *res = nst_obj_cast(ob, nst_t.Bool, err);
        if ( res == NULL )
        {
            return false;
        }
        *(Nst_Bool *)arg = AS_BOOL(res);
        nst_dec_ref(res);
        return true;
    }
    else if ( (usize)final_type == STR_C_CAST )
    {
        Nst_Obj *res = nst_obj_cast(ob, nst_t.Str, err);
        if ( res == NULL )
        {
            return false;
        }
        if ( res->ref_count != 1 )
        {
            nst_dec_ref(res);
            res = nst_string_copy(res, err);
            if ( res == NULL )
            {
                return false;
            }
        }

        NST_FLAG_DEL(res, NST_FLAG_STR_IS_ALLOC);
        *(i8 **)arg = STR(res)->value;
        nst_dec_ref(res);
        return true;
    }

content_check:
    if ( type->seq_match == NULL )
    {
        *(Nst_Obj **)arg = ob;
        return true;
    }

    if ( ob->type != nst_t.Array && ob->type != nst_t.Vector )
    {
        *(Nst_Obj **)arg = ob;
        return true;
    }

    for (usize i = 0, n = SEQ(ob)->len; i < n; i++)
    {
        if ( !check_type(type->seq_match, err, SEQ(ob)->objs[i], arg) )
        {
            if ( final_type != NULL )
            {
                nst_dec_ref(ob);
            }
            return false;
        }
    }
    *(Nst_Obj **)arg = ob;
    return true;
}

static bool append_type(Nst_TypeObj *type, Nst_Buffer *buf, usize tot_types)
{
    if ( !nst_buffer_expand_by(buf, type->len + 6, NULL) )
    {
        return false;
    }

    nst_buffer_append_char(buf, '\'', NULL);
    nst_buffer_append(buf, type, NULL);
    nst_buffer_append_char(buf, '\'', NULL);

    switch ( tot_types )
    {
    case 0:
        nst_buffer_append_char(buf, ' ', NULL);
        break;
    case 1:
        nst_buffer_append_c_str(buf, " or ", NULL);
        break;
    default:
        nst_buffer_append_c_str(buf, ", ", NULL);
        break;
    }

    return true;
}

static bool append_types(MatchType *type, Nst_Buffer *buf)
{
    u16 accepted_types = type->accepted_types;
    usize tot_types = 0;
    Nst_TypeObj *type_str;
    for ( u16 i = 0; i < 13; i++ )
    {
        // if the type is selected and is not Nst_NullIdx
        if ( ((accepted_types >> i) & 1) != 0 && i != 5 )
        {
            tot_types++;
        }
    }
    tot_types += accepted_types >> 13 & 0b11;

    for ( u16 i = 0; i < 13; i++ )
    {
        if ( ((accepted_types >> i) & 1) == 0 || i == 5 )
        {
            continue;
        }
        tot_types--;
        switch ( i )
        {
        case 0: type_str = nst_t.Int; break;
        case 1: type_str = nst_t.Real; break;
        case 2: type_str = nst_t.Byte; break;
        case 3: type_str = nst_t.Bool; break;
        case 4: type_str = nst_t.Str; break;
        case 6: type_str = nst_t.Array; break;
        case 7: type_str = nst_t.Vector; break;
        case 8: type_str = nst_t.Map; break;
        case 9: type_str = nst_t.Iter; break;
        case 10: type_str = nst_t.IOFile; break;
        case 11: type_str = nst_t.Func; break;
        default: type_str = nst_t.Type; break;
        }
        if ( !append_type(type_str, buf, tot_types) )
        {
            return false;
        }
    }

    for ( u16 i = 0, n = accepted_types >> 13 & 0b11; i < n; i++ )
    {
        type_str = type->custom_types[i];
        tot_types--;
        if ( !append_type(type_str, buf, tot_types) )
        {
            return false;
        }
    }

    if ( type->seq_match != NULL )
    {
        if ( !nst_buffer_append_c_str(buf, "containing only ", NULL) )
        {
            return false;
        }
        return append_types(type->seq_match, buf);
    }
    return true;
}

static void set_err(MatchType *type, Nst_Obj *ob, usize idx, Nst_OpErr *err)
{
    const i8 *fmt;
    if ( type->accepted_types & NULL_IDX )
    {
        fmt = "expected optional type %sfor argument %zi "
              "but got type '%s' instead";
    }
    else
    {
        fmt = "expected type %sfor argument %zi but got type '%s' instead";
    }

    Nst_Buffer buf;
    if ( !nst_buffer_init(&buf, 256, NULL) || !append_types(type, &buf) )
    {
        nst_buffer_destroy(&buf);
        NST_FAILED_ALLOCATION;
        return;
    }

    Nst_StrObj *str = STR(nst_sprintf(fmt, buf.data, idx, ob->type->value));
    nst_buffer_destroy(&buf);
    NST_SET_TYPE_ERROR(str);
}

static void free_type_match(MatchType *type)
{
    if ( type->seq_match )
    {
        free_type_match(type->seq_match);
    }
    nst_free(type);
}

bool nst_extract_arg_values(const i8  *types,
                            usize      arg_num,
                            Nst_Obj  **args,
                            Nst_OpErr *err,
                            ...)
{
    va_list args_list;
    va_start(args_list, err);
    i8 *t = (i8 *)types;
    usize idx = 0;

    do
    {
        MatchType *type = compile_type_match(t, &t, &args_list, true, err);
        if ( type == NULL )
        {
            va_end(args_list);
            return false;
        }

        Nst_Obj *ob = args[idx++];
        void *arg = va_arg(args_list, void *);

        if ( !check_type(type, err, ob, arg) )
        {
            set_err(type, ob, idx, err);
            return false;
        }
        free_type_match(type);
    }
    while ( *t != '\0' );
    va_end(args_list);

    if ( idx != arg_num )
    {
        NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
        return false;
    }
    return true;
}
