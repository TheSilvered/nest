#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "simple_types.h"
#include "global_consts.h"

#define NEW_SYMPLE_TYPE(type, type_obj) \
    type *obj = (type *)nst_obj_alloc(sizeof(type), type_obj, NULL, err); \
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
    Nst_IOFileObj *obj = IOFILE(nst_obj_alloc(
        sizeof(Nst_IOFileObj),
        nst_t.IOFile,
        _nst_iofile_destroy,
        err));
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
    Nst_IOFileObj *obj = IOFILE(nst_obj_alloc(
        sizeof(Nst_IOFileObj),
        nst_t.IOFile,
        _nst_iofile_destroy,
        err));
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

isize nst_print(const i8 *buf, isize len)
{
    return nst_fprint(nst_io->out, buf, len);
}

isize nst_println(const i8 *buf, isize len)
{
    return nst_fprintln(nst_io->out, buf, len);
}
/*
ptrdiff_t nst_printf(const i8 *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return nst_fvprintf(nst_io->out, fmt, args);
}
*/
isize nst_fprint(Nst_IOFileObj *f, const i8 *buf, isize len)
{
    if ( NST_IOF_IS_CLOSED(f) )
    {
        return -1;
    }
    if ( len == -1 )
    {
        len = strlen(buf);
    }
    
    return f->write_f((void *)buf, 1, (size_t)len, f->value);;
}

isize nst_fprintln(Nst_IOFileObj *f, const i8 *buf, isize len)
{
    if ( NST_IOF_IS_CLOSED(f) )
    {
        return -1;
    }
    if ( len == -1 )
    {
        len = strlen(buf);
    }
    size_t a = f->write_f((void *)buf, 1, (size_t)len, f->value);
    size_t b = f->write_f((void *)"\n", 1, 1, f->value);
    return a + b;
}
/*
isize nst_fprintf(Nst_IOFileObj *f, const i8 *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return nst_fvprintf(f, fmt, args);
}

isize nst_fvprintf(Nst_IOFileObj *f, const i8 *fmt, va_list args)
{
    if ( NST_IOF_IS_CLOSED(f) )
    {
        return -1;
    }

    char *buf;
}
*/
i32 nst_flush()
{
    return nst_fflush(nst_io->out);
}

i32 nst_fflush(Nst_IOFileObj *f)
{
    if ( NST_IOF_IS_CLOSED(f) )
    {
        return -1;
    }
    return f->flush_f(f->value);
}
