#include <stdlib.h>
#include "simple_types.h"

Nst_int *new_int(Nst_int value)
{
    Nst_int *num = malloc(sizeof(Nst_int));
    if ( num == NULL ) return NULL;
    *num = value;
    return num;
}

Nst_real *new_real(Nst_real value)
{
    Nst_real *num = malloc(sizeof(Nst_real));
    if ( num == NULL ) return NULL;
    *num = value;
    return num;
}

Nst_bool *new_bool(Nst_bool value)
{
    Nst_bool *num = malloc(sizeof(Nst_bool));
    if ( num == NULL ) return NULL;
    *num = value;
    return num;
}

Nst_byte *new_byte(Nst_byte value)
{
    Nst_byte *num = malloc(sizeof(Nst_byte));
    if ( num == NULL ) return NULL;
    *num = value;
    return num;
}

Nst_Obj *new_int_obj(Nst_int value)
{
    return make_obj_free(new_int(value), nst_t_int);
}

Nst_Obj *new_real_obj(Nst_real value)
{
    return make_obj_free(new_real(value), nst_t_real);
}

Nst_Obj *new_byte_obj(Nst_byte value)
{
    return make_obj_free(new_byte(value), nst_t_byte);
}

Nst_Obj *new_file_obj(Nst_iofile *file)
{
    return make_obj_free(file, nst_t_file);
}