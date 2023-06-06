#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "simple_types.h"
#include "global_consts.h"
#include "obj_ops.h"

#define NEW_SYMPLE_TYPE(type, type_obj) \
    type *obj = nst_obj_alloc(type, type_obj, NULL, err); \
    if ( obj == NULL ) \
        return NULL; \
    obj->value = value; \
    return OBJ(obj)

Nst_Obj *nst_int_new(Nst_Int value, Nst_OpErr *err)
{
    NEW_SYMPLE_TYPE(Nst_IntObj, nst_t.Int);
}

Nst_Obj *nst_real_new(Nst_Real value, Nst_OpErr *err)
{
    NEW_SYMPLE_TYPE(Nst_RealObj, nst_t.Real);
}

Nst_Obj *nst_byte_new(Nst_Byte value, Nst_OpErr *err)
{
    NEW_SYMPLE_TYPE(Nst_ByteObj, nst_t.Byte);
}

Nst_Obj *nst_bool_new(Nst_Bool value, Nst_OpErr *err)
{
    NEW_SYMPLE_TYPE(Nst_BoolObj, nst_t.Bool);
}

Nst_Obj *nst_iof_new(Nst_IOFile value, bool bin, bool read, bool write, Nst_OpErr *err)
{
    Nst_IOFileObj *obj = nst_obj_alloc(
        Nst_IOFileObj,
        nst_t.IOFile,
        _nst_iofile_destroy,
        err);
    if ( obj == NULL )
    {
        return NULL;
    }

    obj->value   = value;
    obj->read_f  = (Nst_IOFile_read_f)fread;
    obj->write_f = (Nst_IOFile_write_f)fwrite;
    obj->flush_f = (Nst_IOFile_flush_f)fflush;
    obj->tell_f  = (Nst_IOFile_tell_f)ftell;
    obj->seek_f  = (Nst_IOFile_seek_f)fseek;
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
                          Nst_IOFile_close_f close_f,
                          Nst_OpErr *err)
{
    Nst_IOFileObj *obj = nst_obj_alloc(
        Nst_IOFileObj,
        nst_t.IOFile,
        _nst_iofile_destroy,
        err);
    if ( obj == NULL ) return NULL;

    obj->value   = (Nst_IOFile)value;
    obj->read_f  = read_f;
    obj->write_f = write_f;
    obj->flush_f = flush_f;
    obj->tell_f  = tell_f;
    obj->seek_f  = seek_f;
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

void _nst_iofile_destroy(Nst_IOFileObj *obj)
{
    if ( !NST_IOF_IS_CLOSED(obj) )
    {
        obj->flush_f(obj->value);
        obj->close_f(obj->value);
    }
}

isize nst_fread(void  *buf,
                usize size,
                usize count,
                Nst_IOFileObj *f)
{
    if ( NST_IOF_IS_CLOSED(f) || !NST_IOF_CAN_READ(f) )
    {
        return -1;
    }
    return f->read_f(buf, size, count, f->value);
}

isize nst_fwrite(void  *buf,
                 usize size,
                 usize count,
                 Nst_IOFileObj *f)
{
    if ( NST_IOF_IS_CLOSED(f) || !NST_IOF_CAN_WRITE(f) )
    {
        return -1;
    }
    return f->write_f(buf, size, count, f->value);
}

i32 nst_fflush(Nst_IOFileObj *f)
{
    if ( NST_IOF_IS_CLOSED(f) )
    {
        return EOF;
    }
    return f->flush_f(f->value);
}

i32 nst_ftell(Nst_IOFileObj *f)
{
    if ( NST_IOF_IS_CLOSED(f) )
    {
        return -1;
    }
    return f->tell_f(f->value);
}

i32 nst_fseek(Nst_IOFileObj *f, i32 offset, i32 origin)
{
    if ( NST_IOF_IS_CLOSED(f) )
    {
        return -1;
    }
    return f->seek_f(f->value, offset, origin);
}

i32 nst_fclose(Nst_IOFileObj *f)
{
    if ( NST_IOF_IS_CLOSED(f) )
    {
        return EOF;
    }

    f->close_f(f->value);
    f->value = NULL;
    NST_FLAG_SET(f, NST_FLAG_IOFILE_IS_CLOSED);
    return 0;
}

u8 _nst_number_to_u8(Nst_Obj *number)
{
    return (u8)_nst_number_to_i64(number);
}

int _nst_number_to_int(Nst_Obj *number)
{
    return (int)_nst_number_to_i64(number);
}

i32 _nst_number_to_i32(Nst_Obj *number)
{
    return (i32)_nst_number_to_i64(number);
}

i64 _nst_number_to_i64(Nst_Obj *number)
{
    Nst_TypeObj *t = number->type;

    if ( t == nst_t.Byte )
    {
        return (i64)AS_BYTE(number);
    }
    else if ( t == nst_t.Int )
    {
        return AS_INT(number);
    }
    else if ( t == nst_t.Real )
    {
        return (i64)AS_REAL(number);
    }
    return 0;
}

f32 _nst_number_to_f32(Nst_Obj *number)
{
    return (f32)_nst_number_to_f64(number);
}

f64 _nst_number_to_f64(Nst_Obj *number)
{
    Nst_TypeObj *t = number->type;

    if ( t == nst_t.Byte )
    {
        return (f64)AS_BYTE(number);
    }
    else if ( t == nst_t.Int )
    {
        return (f64)AS_INT(number);
    }
    else if ( t == nst_t.Real )
    {
        return AS_REAL(number);
    }
    return 0.0;
}

Nst_Bool _nst_obj_to_bool(Nst_Obj *obj)
{
    Nst_Obj *bool_obj = nst_obj_cast(obj, nst_t.Bool, NULL);
    Nst_Bool v = AS_BOOL(bool_obj);
    nst_dec_ref(bool_obj);
    return v;
}
