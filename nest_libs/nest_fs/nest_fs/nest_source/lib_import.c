#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lib_import.h"
#include "error.h"
#include "nst_types.h"

#define SET_TYPE_ERROR(type) do { \
    err->name = (char *)"Type Error"; \
    err->message = format_arg_error(WRONG_TYPE_FOR_ARG(type), ob->type_name, i); \
    error_occurred = true; } while ( 0 )

FuncDeclr *new_func_list(size_t count)
{
    return (FuncDeclr *)calloc(count + 1, sizeof(FuncDeclr));
}

bool extract_arg_values(const char *types,
                        size_t arg_num,
                        Nst_Obj **args,
                        OpErr *err,
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
       'R': real or integer, always returns a real
       'b': bool
       'n': null
       's': string
       'v': vector
       'a': array
       'A': array or vector
       'm': map
       'f': func
       'I': iter
       'B': byte
       'F': file
     */
    for ( size_t i = 0; i < arg_num; i++ )
    {
        arg = va_arg(arglist, void *);
        ob = args[i];

        switch (types[i])
        {
        case 't':
            if ( ob->type != nst_t_type )
                SET_TYPE_ERROR("Type");
            *(Nst_string **)arg = AS_STR(ob);
            break;
        case 'i':
            if ( ob->type != nst_t_int )
                SET_TYPE_ERROR("Int");
            *(Nst_int *)arg = AS_INT(ob);
            break;
        case 'r':
            if ( ob->type != nst_t_real )
                SET_TYPE_ERROR("Real");
            *(Nst_real *)arg = AS_REAL(ob);
            break;
        case 'N':
            if ( ob->type != nst_t_real && ob->type != nst_t_int )
                SET_TYPE_ERROR("Real' or 'Int");
            if ( ob->type == nst_t_real )
                *(Nst_real *)arg = AS_REAL(ob);
            else
                *(Nst_real *)arg = (Nst_real)AS_INT(ob);
            break;
        case 'b':
            if ( ob->type != nst_t_bool )
                SET_TYPE_ERROR("Bool");
            *(Nst_bool *)arg = AS_BOOL(ob);
            break;
        case 'n':
            if ( ob->type != nst_t_null )
                SET_TYPE_ERROR("Null");
            *(void **)arg = NULL;
            break;
        case 's':
            if ( ob->type != nst_t_str )
                SET_TYPE_ERROR("Str");
            *(Nst_string **)arg = AS_STR(ob);
            break;
        case 'v':
            if ( ob->type != nst_t_vect )
                SET_TYPE_ERROR("Vector");
            *(Nst_sequence **)arg = AS_SEQ(ob);
            break;
        case 'a':
            if ( ob->type != nst_t_arr )
                SET_TYPE_ERROR("Array");
            *(Nst_sequence **)arg = AS_SEQ(ob);
            break;
        case 'A':
            if ( ob->type != nst_t_arr && ob->type != nst_t_vect )
                SET_TYPE_ERROR("Array' or 'Vector");
            *(Nst_sequence **)arg = AS_SEQ(ob);
            break;
        case 'm':
            if ( ob->type != nst_t_map )
                SET_TYPE_ERROR("Map");
            *(Nst_map **)arg = AS_MAP(ob);
            break;
        case 'f':
            if ( ob->type != nst_t_func )
                SET_TYPE_ERROR("Func");
            *(Nst_func **)arg = AS_FUNC(ob);
            break;
        case 'I':
            if ( ob->type != nst_t_iter )
                SET_TYPE_ERROR("Iter");
            *(Nst_iter **)arg = AS_ITER(ob);
            break;
        case 'B':
            if ( ob->type != nst_t_byte )
                SET_TYPE_ERROR("Byte");
            *(Nst_byte *)arg = AS_BYTE(ob);
            break;
        case 'F':
            if ( ob->type != nst_t_file )
                SET_TYPE_ERROR("IOfile");
            *(Nst_iofile **)arg = AS_FILE(ob);
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
