#include <stdlib.h>
#include <stdio.h>
#include "simple_types.h"
#include "global_consts.h"

#define NEW_SYMPLE_TYPE(type, type_obj) \
    type *obj = (type *)nst_obj_alloc(sizeof(type), type_obj, NULL); \
    if ( obj == NULL ) \
        return NULL; \
    obj->value = value; \
    return OBJ(obj)

Nst_Obj *nst_int_new(Nst_Int value)
{
    NEW_SYMPLE_TYPE(Nst_IntObj, nst_t.Int);
}

Nst_Obj *nst_real_new(Nst_Real value)
{
    NEW_SYMPLE_TYPE(Nst_RealObj, nst_t.Real);
}

Nst_Obj *nst_byte_new(Nst_Byte value)
{
    NEW_SYMPLE_TYPE(Nst_ByteObj, nst_t.Byte);
}

Nst_Obj *nst_bool_new(Nst_Bool value)
{
    NEW_SYMPLE_TYPE(Nst_BoolObj, nst_t.Bool);
}

Nst_Obj *nst_iof_new(Nst_IOFile value, bool bin, bool read, bool write)
{
    Nst_IOFileObj *obj = IOFILE(nst_obj_alloc(
        sizeof(Nst_IOFileObj),
        nst_t.IOFile,
        nst_destroy_iofile));
    if ( obj == NULL )
    {
        return NULL;
    }

    obj->value = value;
    obj->read_f = (Nst_IOFile_read_f)fread;
    obj->write_f = (Nst_IOFile_write_f)fwrite;
    obj->flush_f = (Nst_IOFile_flush_f)fflush;
    obj->tell_f = (Nst_IOFile_tell_f)ftell;
    obj->seek_f = (Nst_IOFile_seek_f)fseek;
    obj->close_f = (Nst_IOFile_close_f)fclose;

    if ( bin )
    {
        NST_FLAG_SET(obj, NST_FLAG_IOFILE_IS_BIN);
    }
    if ( read )
    {
        NST_FLAG_SET(obj, NST_FLAG_IOFILE_CAN_READ);
    }
    if ( write )
    {
        NST_FLAG_SET(obj, NST_FLAG_IOFILE_CAN_WRITE);
    }

    return OBJ(obj);
}

Nst_Obj *nst_iof_new_fake(void *value,
                          bool bin, bool read, bool write,
                          Nst_IOFile_read_f  read_f,
                          Nst_IOFile_write_f write_f,
                          Nst_IOFile_flush_f flush_f,
                          Nst_IOFile_tell_f  tell_f,
                          Nst_IOFile_seek_f  seek_f,
                          Nst_IOFile_close_f close_f)
{
    Nst_IOFileObj *obj = IOFILE(nst_obj_alloc(
        sizeof(Nst_IOFileObj),
        nst_t.IOFile,
        nst_destroy_iofile));
    if ( obj == NULL ) return NULL;

    obj->value = (Nst_IOFile)value;
    obj->read_f = read_f;
    obj->write_f = write_f;
    obj->flush_f = flush_f;
    obj->tell_f = tell_f;
    obj->seek_f = seek_f;
    obj->close_f = close_f;

    if ( bin )
    {
        NST_FLAG_SET(obj, NST_FLAG_IOFILE_IS_BIN);
    }
    if ( read )
    {
        NST_FLAG_SET(obj, NST_FLAG_IOFILE_CAN_READ);
    }
    if ( write )
    {
        NST_FLAG_SET(obj, NST_FLAG_IOFILE_CAN_WRITE);
    }

    return OBJ(obj);
}

void nst_destroy_iofile(Nst_IOFileObj *obj)
{
    if ( !NST_IOF_IS_CLOSED(obj) )
    {
        obj->flush_f(obj->value);
        obj->close_f(obj->value);
    }
}
