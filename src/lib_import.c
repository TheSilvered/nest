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

0 00 0  0000  0000  0000
|A||B||---------C--------|

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
    Nst_IntIdx    = 0b0000000000000001,
    Nst_RealIdx   = 0b0000000000000010,
    Nst_ByteIdx   = 0b0000000000000100,
    Nst_BoolIdx   = 0b0000000000001000,
    Nst_StrIdx    = 0b0000000000010000,
    Nst_NullIdx   = 0b0000000000100000,
    Nst_ArrayIdx  = 0b0000000001000000,
    Nst_VectorIdx = 0b0000000010000000,
    Nst_MapIdx    = 0b0000000100000000,
    Nst_IterIdx   = 0b0000001000000000,
    Nst_IOFileIdx = 0b0000010000000000,
    Nst_FuncIdx   = 0b0000100000000000,
    Nst_TypeIdx   = 0b0001000000000000,
    Nst_CastC     = 0b1000000000000000,

    Nst_IntCast   = 1,
    Nst_RealCast  = 2,
    Nst_ByteCast  = 3,
    Nst_BoolCast  = 4,
    Nst_StrCast   = 5
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
            accepted_types |= Nst_TypeIdx;
            goto normal_type;
        case 'i':
            accepted_types |= Nst_IntIdx;
            goto normal_type;
        case 'r':
            accepted_types |= Nst_RealIdx;
            goto normal_type;
        case 'b':
            accepted_types |= Nst_BoolIdx;
            goto normal_type;
        case 'n':
            accepted_types |= Nst_NullIdx;
            goto normal_type;
        case 's':
            accepted_types |= Nst_StrIdx;
            goto normal_type;
        case 'v':
            accepted_types |= Nst_VectorIdx;
            goto normal_type;
        case 'a':
            accepted_types |= Nst_ArrayIdx;
            goto normal_type;
        case 'm':
            accepted_types |= Nst_MapIdx;
            goto normal_type;
        case 'f':
            accepted_types |= Nst_FuncIdx;
            goto normal_type;
        case 'I':
            accepted_types |= Nst_IterIdx;
            goto normal_type;
        case 'B':
            accepted_types |= Nst_ByteIdx;
            goto normal_type;
        case 'F':
            accepted_types |= Nst_IOFileIdx;
            goto normal_type;

        case 'o':
            match_any = true;
            goto normal_type;

        case 'l':
            accepted_types |= Nst_IntIdx;
            accepted_types |= Nst_ByteIdx;
            accepted_types |= Nst_CastC;
            match_type->final_type = (Nst_TypeObj *)Nst_IntCast;
            goto normal_type;
        case 'N':
            accepted_types |= Nst_IntIdx;
            accepted_types |= Nst_ByteIdx;
            accepted_types |= Nst_RealIdx;
            accepted_types |= Nst_CastC;
            match_type->final_type = (Nst_TypeObj *)Nst_RealCast;
            goto normal_type;
        case 'A':
            accepted_types |= Nst_ArrayIdx;
            accepted_types |= Nst_VectorIdx;
            goto normal_type;
        case 'S':
            accepted_types |= Nst_ArrayIdx;
            accepted_types |= Nst_VectorIdx;
            accepted_types |= Nst_StrIdx;
            match_type->final_type = nst_t.Array;
            goto normal_type;
        case 'R':
            accepted_types |= Nst_ArrayIdx;
            accepted_types |= Nst_VectorIdx;
            accepted_types |= Nst_StrIdx;
            accepted_types |= Nst_IterIdx;
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
            accepted_types |= Nst_NullIdx;
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
        if ( !allow_auto_casting || accepted_types & Nst_NullIdx )
        {
            NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
            free(match_type);
            return NULL;
        }

        t++;
        accepted_types |= Nst_CastC;
        switch (*t++)
        {
        case 'i':
            match_type->final_type = (Nst_TypeObj *)Nst_IntCast;
            break;
        case 'r':
            match_type->final_type = (Nst_TypeObj *)Nst_RealCast;
            break;
        case 'b':
            match_type->final_type = (Nst_TypeObj *)Nst_BoolCast;
            break;
        case 'B':
            match_type->final_type = (Nst_TypeObj *)Nst_ByteCast;
            break;
        case 's':
            match_type->final_type = (Nst_TypeObj *)Nst_StrCast;
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
        if ( accepted_types == Nst_IntIdx )
        {
            match_type->final_type = (Nst_TypeObj *)Nst_IntCast;
            accepted_types |= Nst_CastC;
        }
        else if ( accepted_types == Nst_RealIdx )
        {
            match_type->final_type = (Nst_TypeObj *)Nst_RealCast;
            accepted_types |= Nst_CastC;
        }
        else if ( accepted_types == Nst_ByteIdx )
        {
            match_type->final_type = (Nst_TypeObj *)Nst_ByteCast;
            accepted_types |= Nst_CastC;
        }
        else if ( accepted_types == Nst_BoolIdx )
        {
            match_type->final_type = (Nst_TypeObj *)Nst_BoolCast;
            accepted_types |= Nst_CastC;
        }
    }

    if ( *t == '.' )
    {
        if ( accepted_types & Nst_CastC )
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
        accepted_types &= Nst_CastC;
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

    if ( accepted_types & Nst_NullIdx && ob == nst_c.Null_null )
    {
        *(Nst_Obj **)arg = nst_c.Null_null;
        return true;
    }

    // If any object is accepted
    if ( accepted_types == (accepted_types & Nst_CastC) )
    {
        goto cast_obj;
    }

    // If the object's type is built-in but not accepted
    if ( (ob_t == nst_t.Int    && !(accepted_types & Nst_IntIdx   )) ||
         (ob_t == nst_t.Real   && !(accepted_types & Nst_RealIdx  )) ||
         (ob_t == nst_t.Byte   && !(accepted_types & Nst_ByteIdx  )) ||
         (ob_t == nst_t.Bool   && !(accepted_types & Nst_BoolIdx  )) ||
         (ob_t == nst_t.Str    && !(accepted_types & Nst_StrIdx   )) ||
         (ob_t == nst_t.Array  && !(accepted_types & Nst_ArrayIdx )) ||
         (ob_t == nst_t.Vector && !(accepted_types & Nst_VectorIdx)) ||
         (ob_t == nst_t.Map    && !(accepted_types & Nst_MapIdx   )) ||
         (ob_t == nst_t.Iter   && !(accepted_types & Nst_IterIdx  )) ||
         (ob_t == nst_t.IOFile && !(accepted_types & Nst_IOFileIdx)) ||
         (ob_t == nst_t.Func   && !(accepted_types & Nst_FuncIdx  )) ||
         (ob_t == nst_t.Type   && !(accepted_types & Nst_TypeIdx  )) )
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

    if ( !(accepted_types & Nst_CastC) )
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

    if ( (usize)final_type == Nst_IntCast )
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
    else if ( (usize)final_type == Nst_RealCast )
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
    else if ( (usize)final_type == Nst_ByteCast )
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
    else if ( (usize)final_type == Nst_BoolCast )
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
    else if ( (usize)final_type == Nst_StrCast )
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

static bool expand_buf(i8 **buf, usize buf_len, usize *buf_size, usize amount)
{
    if ( *buf_size - buf_len - 1 >= amount )
    {
        return true;
    }
    i8 *temp = (i8 *)nst_realloc(*buf, *buf_size + amount + 1, sizeof(i8), 0, NULL);
    if ( temp == NULL )
    {
        free(*buf);
        return false;
    }
    *buf = temp;
    *buf_size += amount + 1;
    return true;
}

static bool append_type(Nst_TypeObj *type,
                        i8 **buf,
                        usize *buf_len,
                        usize *buf_size,
                        usize tot_types)
{
    i32 extra_bytes = 0;
    switch ( tot_types )
    {
    case 0: extra_bytes = 1; break;
    case 1: extra_bytes = 4; break;
    default: extra_bytes = 2; break;
    }

    if ( !expand_buf(buf, *buf_len, buf_size, type->len + extra_bytes + 2) )
    {
        return false;
    }
    (*buf + *buf_len)[0] = '\'';
    memcpy(*buf + *buf_len + 1, type->value, type->len);
    *buf_len += type->len + 2;
    (*buf + *buf_len)[-1] = '\'';
    if ( extra_bytes == 2 )
    {
        (*buf + *buf_len)[0] = ',';
        (*buf + *buf_len)[1] = ' ';
        *buf_len += 2;
    }
    else if ( extra_bytes == 4 )
    {
        (*buf + *buf_len)[0] = ' ';
        (*buf + *buf_len)[1] = 'o';
        (*buf + *buf_len)[2] = 'r';
        (*buf + *buf_len)[3] = ' ';
        *buf_len += 4;
    }
    else
    {
        (*buf + *buf_len)[0] = ' ';
        *buf_len += 1;
    }
    return true;
}

static bool append_types(MatchType *type, i8 **buf, usize *buf_len, usize *buf_size)
{
    u16 accepted_types = type->accepted_types;
    usize tot_types = 0;
    Nst_TypeObj *type_str;
    for ( u16 i = 0; i < 13; i++ )
    {
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
        if ( !append_type(type_str, buf, buf_len, buf_size, tot_types) )
        {
            return false;
        }
    }

    for ( u16 i = 0, n = accepted_types >> 13 & 0b11; i < n; i++ )
    {
        type_str = type->custom_types[i];
        tot_types--;
        if ( !append_type(type_str, buf, buf_len, buf_size, tot_types) )
        {
            return false;
        }
    }

    if ( type->seq_match != NULL )
    {
        const i8 *msg = "containing only ";
        if ( !expand_buf(buf, *buf_len, buf_size, 16) )
        {
            return false;
        }
        memcpy(*buf + *buf_len, msg, 16);
        *buf_len += 16;
        return append_types(type->seq_match, buf, buf_len, buf_size);
    }
    return true;
}

static Nst_StrObj *make_err_str(MatchType *type, Nst_Obj *ob, usize idx)
{
    i8 *buf = (i8 *)nst_malloc(256, sizeof(i8), NULL);
    if ( buf == NULL )
    {
        return NULL;
    }
    usize buf_size = 256;
    usize buf_len = 0;

    if ( type->accepted_types & Nst_NullIdx )
    {
        const i8 *msg = "expected optional type ";
        memcpy(buf, msg, 23);
        buf_len = 23;
    }
    else
    {
        const i8 *msg = "expected type ";
        memcpy(buf, msg, 14);
        buf_len = 14;
    }

    if ( !append_types(type, &buf, &buf_len, &buf_size) )
    {
        return NULL;
    }
    const i8 *final_msg = "at index but got type '' instead";

    if ( !expand_buf(&buf, buf_len, &buf_size, 43 + ob->type->len) )
    {
        return NULL;
    }

    memcpy(buf + buf_len, final_msg, 9);
    buf_len += 9;
    buf_len += sprintf(buf + buf_len, "%zi", idx);
    memcpy(buf + buf_len, final_msg + 8, 15);
    buf_len += 15;
    memcpy(buf + buf_len, ob->type->value, ob->type->len);
    buf_len += ob->type->len;
    strcpy(buf + buf_len, final_msg + 23);
    buf_len += 9;
    Nst_StrObj *str = STR(nst_string_new(buf, buf_len, true, NULL));
    if ( str == NULL )
    {
        nst_free(buf);
    }
    return str;
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
    i8 *type_end = (i8 *)types;
    MatchType *type;
    Nst_Obj *ob;
    void *arg;
    usize ob_idx = 0;

    do
    {
        type = compile_type_match(type_end, &type_end, &args_list, true, err);
        if ( type == NULL )
        {
            va_end(args_list);
            return false;
        }

        ob = args[ob_idx++];
        arg = va_arg(args_list, void *);
        if ( !check_type(type, err, ob, arg) )
        {
            Nst_StrObj *err_message = make_err_str(type, ob, ob_idx);
            if ( err_message == NULL )
            {
                NST_FAILED_ALLOCATION;
            }
            else
            {
                NST_SET_ERROR(nst_s.e_TypeError, err_message);
            }
            va_end(args_list);
            return false;
        }
        free_type_match(type);
    }
    while ( *type_end != '\0' );
    va_end(args_list);

    if ( ob_idx != arg_num )
    {
        NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
        return false;
    }
    return true;
}
