#include <stdlib.h>
#include <stdio.h>
#include "simple_types.h"

#define NEW_SYMPLE_TYPE(type, type_obj) \
    type *obj = (type *)alloc_obj(sizeof(type), type_obj, NULL); \
    if ( obj == NULL ) return NULL; \
    obj->value = value; \
    return (Nst_Obj *)obj

Nst_Obj *nst_new_int(Nst_Int value)
{
    NEW_SYMPLE_TYPE(Nst_IntObj, nst_t_int);
}

Nst_Obj *nst_new_real(Nst_Real value)
{
    NEW_SYMPLE_TYPE(Nst_RealObj, nst_t_real);
}

Nst_Obj *nst_new_byte(Nst_Byte value)
{
    NEW_SYMPLE_TYPE(Nst_ByteObj, nst_t_byte);
}

Nst_Obj *nst_new_bool(Nst_Bool value)
{
    NEW_SYMPLE_TYPE(Nst_BoolObj, nst_t_bool);
}

Nst_Obj *nst_new_file(Nst_IOfile value, bool bin, bool read, bool write)
{
    Nst_IOFileObj *obj = AS_FILE(alloc_obj(sizeof(Nst_IOFileObj), nst_t_file, nst_destroy_iofile));
    if ( obj == NULL ) return NULL;
    obj->value = value;
    obj->is_closed = false;
    obj->is_bin = false;
    obj->is_bin = bin;
    obj->can_read = read;
    obj->can_write = write;
    return (Nst_Obj *)obj;
}

void nst_destroy_iofile(Nst_IOFileObj *obj)
{
    if ( !obj->is_closed )
        fclose(obj->value);
}
