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
