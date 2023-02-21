/* Inteface for Nst_IntObj, Nst_RealObj, Nst_ByteObj, Nst_BoolObj, Nst_IOFileObj */

#ifndef SYMPLE_TYPES_H
#define SYMPLE_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "obj.h"

#define NST_TRUE 1
#define NST_FALSE 0

#define AS_INT(ptr)  (((Nst_IntObj  *)(ptr))->value)
#define AS_REAL(ptr) (((Nst_RealObj *)(ptr))->value)
#define AS_BYTE(ptr) (((Nst_ByteObj *)(ptr))->value)
#define AS_BOOL(ptr) (((Nst_BoolObj *)(ptr))->value)
#define IOFILE(ptr) ((Nst_IOFileObj *)(ptr))

#define NST_IOF_IS_CLOSED(f) NST_FLAG_HAS(f, NST_FLAG_IOFILE_IS_CLOSED)
#define NST_IOF_IS_BIN(f)    NST_FLAG_HAS(f, NST_FLAG_IOFILE_IS_BIN)
#define NST_IOF_CAN_WRITE(f) NST_FLAG_HAS(f, NST_FLAG_IOFILE_CAN_WRITE)
#define NST_IOF_CAN_READ(f)  NST_FLAG_HAS(f, NST_FLAG_IOFILE_CAN_READ)

#define NST_SIMPLE_TYPE_STRUCT(type, alias) \
    typedef type alias; \
    typedef struct _ ## alias ## Obj {  \
        NST_OBJ_HEAD; \
        alias value; \
    } alias ## Obj

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NST_SIMPLE_TYPE_STRUCT(i64, Nst_Int);
NST_SIMPLE_TYPE_STRUCT(f64, Nst_Real);
NST_SIMPLE_TYPE_STRUCT(i8,  Nst_Bool);
NST_SIMPLE_TYPE_STRUCT(u8,  Nst_Byte);

typedef FILE *Nst_IOFile;
typedef usize (*Nst_IOFile_read_f) (void  *buf,
                                    usize size,
                                    usize count,
                                    void  *f_value);
typedef usize (*Nst_IOFile_write_f)(void  *buf,
                                    usize size,
                                    usize count,
                                    void  *f_value);
typedef i32   (*Nst_IOFile_flush_f)(void *f_value);
typedef i32   (*Nst_IOFile_tell_f) (void *f_value);
typedef i32   (*Nst_IOFile_seek_f) (void *f_value, i32 offset, i32 origin);
typedef i32   (*Nst_IOFile_close_f)(void *f_value);

typedef struct _Nst_IOFileObj
{
    NST_OBJ_HEAD;
    Nst_IOFile value;
    Nst_IOFile_read_f  read_f;
    Nst_IOFile_write_f write_f;
    Nst_IOFile_flush_f flush_f;
    Nst_IOFile_tell_f  tell_f;
    Nst_IOFile_seek_f  seek_f;
    Nst_IOFile_close_f close_f;
}
Nst_IOFileObj;

typedef enum _Nst_IOFileFlag
{
    NST_FLAG_IOFILE_IS_CLOSED = 0b0001,
    NST_FLAG_IOFILE_IS_BIN    = 0b0010,
    NST_FLAG_IOFILE_CAN_WRITE = 0b0100,
    NST_FLAG_IOFILE_CAN_READ  = 0b1000
}
Nst_IOFileFlag;

// Creates a new Int object
Nst_Obj *nst_int_new(Nst_Int value);
// Creates a new Real object
Nst_Obj *nst_real_new(Nst_Real value);
// Creates a new Bool object
Nst_Obj *nst_bool_new(Nst_Bool value);
// Creates a new Byte object
Nst_Obj *nst_byte_new(Nst_Byte value);
// Creates a new IOFile object, bin: is opened in binary format,
// read: supports reading, write: supports writing
Nst_Obj *nst_iof_new(Nst_IOFile value, bool bin, bool read, bool write);
Nst_Obj *nst_iof_new_fake(void *value,
                          bool bin, bool read, bool write,
                          Nst_IOFile_read_f  read_f,
                          Nst_IOFile_write_f write_f,
                          Nst_IOFile_flush_f flush_f,
                          Nst_IOFile_tell_f  tell_f,
                          Nst_IOFile_seek_f  seek_f,
                          Nst_IOFile_close_f close_f);

void _nst_destroy_iofile(Nst_IOFileObj *obj);

isize nst_print(const i8 *buf, isize len);
isize nst_println(const i8 *buf, isize len);
// isize nst_printf(const i8 *fmt, ...);
isize nst_fprint(Nst_IOFileObj *f, const i8 *buf, isize len);
isize nst_fprintln(Nst_IOFileObj *f, const i8 *buf, isize len);
// isize nst_fprintf(Nst_IOFileObj *f, const i8 *buf, ...);
// isize nst_fvprintf(Nst_IOFileObj *f, const i8 *buf, va_list args);

i32 nst_flush();
i32 nst_fflush(Nst_IOFileObj *f);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H