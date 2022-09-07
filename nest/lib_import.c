#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "lib_import.h"
#include "nst_types.h"
#include "obj_ops.h"

#define _NST_SET_TYPE_ERROR(type) { \
    err->name = (char *)"Type Error"; \
    err->message = _nst_format_arg_error( \
        WRONG_TYPE_FOR_ARG(type), \
        ob->type_name, i \
    ); \
    error_occurred = true; break; }

Nst_FuncDeclr *nst_new_func_list(size_t count)
{
    return (Nst_FuncDeclr *)calloc(count + 1, sizeof(Nst_FuncDeclr));
}

bool nst_extract_arg_values(const char *types,
                        size_t arg_num,
                        Nst_Obj **args,
                        Nst_OpErr *err,
                        ...)
{

    if ( strlen(types) != arg_num )
    {
        err->name = (char *)"Value Error";
        err->message = (char *)ARG_NUM_DOESNT_MATCH;
        return false;
    }

    va_list arglist;
    va_start(arglist, err);
    void *arg;
    Nst_Obj *ob;
    bool error_occurred = false;

    /* Characters for each type:
       't': type
       'i': integer
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
     */
    for ( size_t i = 0; i < arg_num; i++ )
    {
        arg = va_arg(arglist, void *);
        ob = args[i];

        switch (types[i])
        {
        case 't':
            if ( ob->type != nst_t_type )
                _NST_SET_TYPE_ERROR("Type");
            *(Nst_StrObj **)arg = AS_STR(ob);
            break;
        case 'i':
            if ( ob->type != nst_t_int )
                _NST_SET_TYPE_ERROR("Int");
            *(Nst_Int *)arg = AS_INT(ob);
            break;
        case 'r':
            if ( ob->type != nst_t_real )
                _NST_SET_TYPE_ERROR("Real");
            *(Nst_Real *)arg = AS_REAL(ob);
            break;
        case 'N':
            if ( ob->type != nst_t_real && ob->type != nst_t_int )
                _NST_SET_TYPE_ERROR("Real', 'Int' or 'Byte");
            if ( ob->type == nst_t_real )
                *(Nst_Real *)arg = AS_REAL(ob);
            else if ( ob->type == nst_t_int )
                *(Nst_Real *)arg = (Nst_Real)AS_INT(ob);
            else
                *(Nst_Real *)arg = (Nst_Real)AS_BYTE(ob);
            break;
        case 'b':
            if ( ob->type != nst_t_bool )
                _NST_SET_TYPE_ERROR("Bool");
            *(Nst_Bool *)arg = AS_BOOL(ob);
            break;
        case 'n':
            if ( ob->type != nst_t_null )
                _NST_SET_TYPE_ERROR("Null");
            *(void **)arg = NULL;
            break;
        case 's':
            if ( ob->type != nst_t_str )
                _NST_SET_TYPE_ERROR("Str");
            *(Nst_StrObj **)arg = AS_STR(ob);
            break;
        case 'v':
            if ( ob->type != nst_t_vect )
                _NST_SET_TYPE_ERROR("Vector");
            *(Nst_SeqObj **)arg = AS_SEQ(ob);
            break;
        case 'a':
            if ( ob->type != nst_t_arr )
                _NST_SET_TYPE_ERROR("Array");
            *(Nst_SeqObj **)arg = AS_SEQ(ob);
            break;
        case 'A':
            if ( ob->type != nst_t_arr && ob->type != nst_t_vect )
                _NST_SET_TYPE_ERROR("Array' or 'Vector");
            *(Nst_SeqObj **)arg = AS_SEQ(ob);
            break;
        case 'S':
            if ( ob->type != nst_t_arr && ob->type != nst_t_vect
                 && ob->type != nst_t_str )
                _NST_SET_TYPE_ERROR("Array', 'Vector' or 'String");
            if ( ob->type == nst_t_str )
                *(Nst_Obj **)arg = nst_obj_cast(ob, nst_t_arr, err);
            else
                *(Nst_Obj **)arg = nst_inc_ref(ob);
            break;
        case 'm':
            if ( ob->type != nst_t_map )
                _NST_SET_TYPE_ERROR("Map");
            *(Nst_MapObj **)arg = AS_MAP(ob);
            break;
        case 'f':
            if ( ob->type != nst_t_func )
                _NST_SET_TYPE_ERROR("Func");
            *(Nst_FuncObj **)arg = AS_FUNC(ob);
            break;
        case 'I':
            if ( ob->type != nst_t_iter )
                _NST_SET_TYPE_ERROR("Iter");
            *(Nst_IterObj **)arg = AS_ITER(ob);
            break;
        case 'B':
            if ( ob->type != nst_t_byte )
                _NST_SET_TYPE_ERROR("Byte");
            *(Nst_Byte *)arg = AS_BYTE(ob);
            break;
        case 'F':
            if ( ob->type != nst_t_file )
                _NST_SET_TYPE_ERROR("IOfile");
            *(Nst_IOFileObj **)arg = AS_FILE(ob);
            break;
        case 'o':
            *(Nst_Obj **)arg = ob;
            break;
        default:
            err->name = (char *)"Value Error";
            err->message = (char *)INCVALID_TYPE_LETTER;
            error_occurred = true;
        }

        if ( error_occurred )
        {
            va_end(arglist);
            return false;
        }
    }

    va_end(arglist);
    return true;
}
