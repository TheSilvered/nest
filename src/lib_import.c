#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "lib_import.h"
#include "nst_types.h"
#include "obj_ops.h"
#include "global_consts.h"

#define SET_TYPE_ERROR(type) { \
    NST_SET_TYPE_ERROR(_nst_format_error( \
        _NST_EM_WRONG_TYPE_FOR_ARG(type), \
        "us", \
        idx + 1, TYPE_NAME(ob))); \
    return false; }

Nst_FuncDeclr *nst_new_func_list(size_t count)
{
    return (Nst_FuncDeclr *)calloc(count + 1, sizeof(Nst_FuncDeclr));
}

static bool extract_builtin_type(const char type,
                                 int        idx,
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
        *(Nst_NullObj **)arg = nst_c.null;
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

bool nst_extract_arg_values(const char *types,
                            size_t      arg_num,
                            Nst_Obj   **args,
                            Nst_OpErr  *err,
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
    unsigned int arg_idx = 0;
    bool succeded = true;

    for ( size_t i = 0, n = strlen(types); i < n; i++ )
    {
        if ( arg_idx >= arg_num )
        {
            NST_SET_RAW_VALUE_ERROR(_NST_EM_ARG_NUM_DOESNT_MATCH);
            succeded = false;
            goto end;
        }

        arg = va_arg(arglist, void *);
        ob = args[arg_idx];

        switch (types[i])
        {
        case '?':
            if ( i + 1 == n ) // if it is the last character
            {
                NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_TYPE_LETTER);
                succeded = false;
                goto end;
            }
            i++;
            if ( ob->type == nst_t.Null )
            {
                *(void**)arg = nst_c.null;
            }
            else
            {
                if ( !extract_builtin_type(types[i], arg_idx,
                                           err, ob, arg, true) )
                {
                    succeded = false;
                    goto end;
                }
            }
            break;
        case '#':
        {
            Nst_TypeObj *custom_type = TYPE(arg);
            arg = va_arg(arglist, void *);

            if ( ob->type == custom_type )
            {
                *(void**)arg = ob;
            }
            else
            {
                NST_SET_TYPE_ERROR(_nst_format_error(
                    _NST_EM_WRONG_TYPE_FOR_ARG2,
                    "sus",
                    custom_type->value, arg_idx + 1, TYPE_NAME(ob)));
                succeded = false;
                goto end;
            }
            break;
        }
        default:
            if ( !extract_builtin_type(types[i], arg_idx,
                                       err, ob, arg, false) )
            {
                succeded = false;
                goto end;
            }
        }
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
