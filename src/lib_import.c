#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "lib_import.h"
#include "iter.h"
#include "obj_ops.h"
#include "format.h"
#include "mem.h"

#define SET_TYPE_ERROR(type) { \
    NST_SET_TYPE_ERROR(nst_sprintf( \
        _NST_EM_WRONG_TYPE_FOR_ARG(type), \
        idx + 1, TYPE_NAME(ob))); \
    return false; }

#if 0

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
    Nst_Obj *custom_types[3];
    Nst_Obj *final_type;
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
    Nst_StrCast   = 4
};

static MatchType *compile_type_match(const i8  *types,
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
     only these types after the underscore: i, r, B or s.

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

    bool allow_optional = true;
    bool allow_or = false;
    bool match_any = false;
    int custom_type_count = 0;
    Nst_Obj *custom_type = NULL;
    i8 *t = (i8 *)types;
    u16 accepted_types = 0;

    while ( true )
    {
        switch ( *t++ )
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
            match_type->final_type = (Nst_Obj *)Nst_IntCast;
            goto normal_type;
        case 'N':
            accepted_types |= Nst_IntIdx;
            accepted_types |= Nst_ByteIdx;
            accepted_types |= Nst_RealIdx;
            accepted_types |= Nst_CastC;
            match_type->final_type = (Nst_Obj *)Nst_RealCast;
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
            custom_type = va_arg(args, void *);
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
            continue;
        case '|':
            if ( !allow_or )
            {
                NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
                free(match_type);
                return NULL;
            }
            allow_optional = false;
            allow_or = false;
            continue;
        }
        if ( t == types + 1 )
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
            match_type->final_type = Nst_IntCast;
            break;
        case 'r':
            match_type->final_type = Nst_RealCast;
            break;
        case 'B':
            match_type->final_type = Nst_ByteCast;
            break;
        case 's':
            match_type->final_type = Nst_StrCast;
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
    Nst_Obj *final_type = type->final_type;

    if ( accepted_types & Nst_NullIdx && ob == nst_c.Null_null )
    {
        *(void **)arg = NULL;
        return true;
    }

    // If any object is accepted
    if ( accepted_types == accepted_types & Nst_CastC )
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
    for (int i = accepted_types >> 136 & 0b11; i > 0; i--)
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

    if ( (int)final_type == Nst_IntCast )
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
    else if ( (int)final_type == Nst_RealCast )
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
    else if ( (int)final_type == Nst_ByteCast )
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
    else if ( (int)final_type == Nst_StrCast )
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
        return true;
    }

    if ( ob->type != nst_t.Array && ob->type != nst_t.Vector )
    {
        return true;
    }

    for (usize i = 0, n = SEQ(ob)->len; i < n; i++)
    {
        if ( !check_type(type->seq_match, err, ob, arg) )
        {
            if ( final_type != NULL )
            {
                nst_dec_ref(ob);
            }
            return false;
        }
    }
}

#else

// ---------------------------- Older type checks --------------------------- //

static bool extract_builtin_type(const i8   type,
                                 i32        idx,
                                 Nst_OpErr *err,
                                 Nst_Obj   *ob,
                                 void      *arg,
                                 bool       always_use_objects)
{
    switch (type)
    {
    case 't':
        if ( ob->type != nst_t.Type )
        {
            SET_TYPE_ERROR("Type");
        }
        *(Nst_StrObj **)arg = STR(ob);
        break;
    case 'i':
        if ( ob->type != nst_t.Int )
        {
            SET_TYPE_ERROR("Int");
        }

        if ( always_use_objects )
        {
            *(Nst_IntObj**)arg = (Nst_IntObj*)ob;
        }
        else
        {
            *(Nst_Int*)arg = AS_INT(ob);
        }
        break;
    case 'l':
        if ( ob->type != nst_t.Int && ob->type == nst_t.Byte )
        {
            SET_TYPE_ERROR("Int");
        }

        if ( always_use_objects )
        {
            *(void**)arg = ob;
        }
        else
        {
            if ( ob->type == nst_t.Int )
            {
                *(Nst_Int*)arg = AS_INT(ob);
            }
            else if ( ob->type == nst_t.Byte )
            {
                *(Nst_Int*)arg = (Nst_Int)AS_BYTE(ob);
            }
        }
        break;
    case 'r':
        if ( ob->type != nst_t.Real )
        {
            SET_TYPE_ERROR("Real");
        }

        if ( always_use_objects )
        {
            *(Nst_RealObj**)arg = (Nst_RealObj*)ob;
        }
        else
        {
            *(Nst_Real*)arg = AS_REAL(ob);
        }
        break;
    case 'N':
        if ( ob->type != nst_t.Real && ob->type != nst_t.Int )
        {
            SET_TYPE_ERROR("Real', 'Int' or 'Byte");
        }

        if ( always_use_objects )
        {
            *(void**)arg = ob;
        }
        else
        {
            if ( ob->type == nst_t.Real )
            {
                *(Nst_Real*)arg = AS_REAL(ob);
            }
            else if ( ob->type == nst_t.Int )
            {
                *(Nst_Real*)arg = (Nst_Real)AS_INT(ob);
            }
            else
            {
                *(Nst_Real*)arg = (Nst_Real)AS_BYTE(ob);
            }
        }
        break;
    case 'b':
        if ( ob->type != nst_t.Bool )
        {
            SET_TYPE_ERROR("Bool");
        }

        if ( always_use_objects )
        {
            *(Nst_BoolObj**)arg = (Nst_BoolObj*)ob;
        }
        else
        {
            *(Nst_Bool*)arg = AS_BOOL(ob);
        }
        break;
    case 'n':
        if ( ob->type != nst_t.Null )
        {
            SET_TYPE_ERROR("Null");
        }
        *(Nst_NullObj **)arg = nst_c.Null_null;
        break;
    case 's':
        if ( ob->type != nst_t.Str )
        {
            SET_TYPE_ERROR("Str");
        }
        *(Nst_StrObj **)arg = STR(ob);
        break;
    case 'v':
        if ( ob->type != nst_t.Vector )
        {
            SET_TYPE_ERROR("Vector");
        }
        *(Nst_SeqObj **)arg = SEQ(ob);
        break;
    case 'a':
        if ( ob->type != nst_t.Array )
        {
            SET_TYPE_ERROR("Array");
        }
        *(Nst_SeqObj **)arg = SEQ(ob);
        break;
    case 'A':
        if ( ob->type != nst_t.Array && ob->type != nst_t.Vector )
        {
            SET_TYPE_ERROR("Array' or 'Vector");
        }
        *(Nst_SeqObj **)arg = SEQ(ob);
        break;
    case 'S':
        if ( ob->type != nst_t.Array  &&
            ob->type != nst_t.Vector &&
            ob->type != nst_t.Str )
            SET_TYPE_ERROR("Array', 'Vector' or 'String");
        if ( ob->type == nst_t.Str )
        {
            *(Nst_Obj**)arg = nst_obj_cast(ob, nst_t.Array, err);
        }
        else
        {
            *(Nst_Obj**)arg = nst_inc_ref(ob);
        }
        break;
    case 'm':
        if ( ob->type != nst_t.Map )
        {
            SET_TYPE_ERROR("Map");
        }
        *(Nst_MapObj **)arg = MAP(ob);
        break;
    case 'f':
        if ( ob->type != nst_t.Func )
        {
            SET_TYPE_ERROR("Func");
        }
        *(Nst_FuncObj **)arg = FUNC(ob);
        break;
    case 'I':
        if ( ob->type != nst_t.Iter )
        {
            SET_TYPE_ERROR("Iter");
        }
        *(Nst_IterObj **)arg = ITER(ob);
        break;
    case 'R':
        if ( ob->type != nst_t.Iter   &&
            ob->type != nst_t.Array  &&
            ob->type != nst_t.Vector &&
            ob->type != nst_t.Str)
        {
            SET_TYPE_ERROR("Iter', 'Array', 'Vector' or 'String");
        }
        if ( ob->type != nst_t.Iter )
        {
            *(Nst_Obj**)arg = nst_obj_cast(ob, nst_t.Iter, err);
        }
        else
        {
            *(Nst_Obj**)arg = nst_inc_ref(ob);
        }
        break;
    case 'B':
        if ( ob->type != nst_t.Byte )
        {
            SET_TYPE_ERROR("Byte");
        }
        if ( always_use_objects )
        {
            *(Nst_ByteObj**)arg = (Nst_ByteObj*)ob;
        }
        else
        {
            *(Nst_Byte*)arg = AS_BYTE(ob);
        }
        break;
    case 'F':
        if ( ob->type != nst_t.IOFile )
        {
            SET_TYPE_ERROR("IOFile");
        }
        *(Nst_IOFileObj **)arg = IOFILE(ob);
        break;
    case 'o':
        *(Nst_Obj **)arg = ob;
        break;
    default:
        NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
        return false;
    }
    return true;
}

bool nst_extract_arg_values(const i8  *types,
                            usize      arg_num,
                            Nst_Obj  **args,
                            Nst_OpErr *err,
                            ...)
{
    /*
    Usage of the 'types' argument:

    The following characters denote a specific type for the argument:
    't': type
    'i': integer
    'l': integer or byte, always returns an int
    'r': real
    'N': real, integer or byte, always returns a real
    'b': bool
    'n': null
    's': string
    'v': vector
    'a': array
    'A': array or vector
    'S': array, vector or string, always returns a Nst_SeqObj *
    'm': map
    'f': func
    'I': iter
    'R': iter, array, vector or str returns always an iter
    'B': byte
    'F': file
    'o': any object

    If one of the above characters is preceded by '?' it means that the value
    is optional and may be null. When using this option the pointer to store
    the value in should always be a pointer to Nst_Obj *.

    If the character is '#' this means that you need to provide the type
    yourself before the pointer to store the value.

    Examples:
        Nst_Obj *num;
        NST_D_EXTRACT("?N", &num); // num can be of type Null, Byte, Int or Real

        CoroutineObj *co;
        NST_D_EXTRACT('#', t_Coroutine, &co); // the function will succede if
                                              // the type of the first argument
                                              // is t_Coroutine
    */

    va_list arglist;
    va_start(arglist, err);
    void *arg;
    Nst_Obj *ob;
    u32 arg_idx = 0;
    bool succeded = true;
    bool use_objects;

    for ( usize i = 0, n = strlen(types); i < n; i++ )
    {
        if ( arg_idx >= arg_num )
        {
            NST_SET_RAW_VALUE_ERROR(_NST_EM_ARG_NUM_DOESNT_MATCH);
            succeded = false;
            goto end;
        }

        arg = va_arg(arglist, void *);
        ob = args[arg_idx];
        use_objects = false;

        if ( types[i] == '?' )
        {
            if ( i + 1 == n ) // if it is the last character
            {
                NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
                succeded = false;
                goto end;
            }
            use_objects = true;
            i++;
            if ( ob->type == nst_t.Null )
            {
                *(void**)arg = nst_c.Null_null;
                goto next_arg;
            }
        }

        if ( types[i] == '#' )
        {
            Nst_TypeObj *custom_type = TYPE(arg);
            arg = va_arg(arglist, void *);

            if ( ob->type == custom_type )
            {
                *(void**)arg = ob;
            }
            else
            {
                NST_SET_TYPE_ERROR(nst_sprintf(
                    _NST_EM_WRONG_TYPE_FOR_ARG2,
                    custom_type->value, arg_idx + 1, TYPE_NAME(ob)));
                succeded = false;
                goto end;
            }
            goto next_arg;
        }

        if ( !extract_builtin_type(types[i], arg_idx, err, ob, arg, use_objects) )
        {
            succeded = false;
            goto end;
        }

    next_arg:
        arg_idx++;
    }

    if ( arg_idx != arg_num )
    {
        NST_SET_RAW_VALUE_ERROR(_NST_EM_ARG_NUM_DOESNT_MATCH);
        succeded = false;
    }

end:
    va_end(arglist);
    return succeded;
}

#endif
