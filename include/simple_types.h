/* Inteface for Nst_IntObj, Nst_RealObj, Nst_ByteObj, Nst_BoolObj, Nst_IOFileObj */

#ifndef SYMPLE_TYPES_H
#define SYMPLE_TYPES_H

#include <stdio.h>
#include "error.h"

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

#define nst_number_to_u8(number) _nst_number_to_u8(OBJ(number))
#define nst_number_to_int(number) _nst_number_to_int(OBJ(number))
#define nst_number_to_i32(number) _nst_number_to_i32(OBJ(number))
#define nst_number_to_i64(number) _nst_number_to_i64(OBJ(number))
#define nst_number_to_f32(number) _nst_number_to_f32(OBJ(number))
#define nst_number_to_f64(number) _nst_number_to_f64(OBJ(number))
#define nst_obj_to_bool(obj) _nst_number_to_f64(OBJ(obj))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef i64 Nst_Int;
EXPORT typedef f64 Nst_Real;
EXPORT typedef i8 Nst_Bool;
EXPORT typedef u8 Nst_Byte;

EXPORT typedef struct _Nst_IntObj
{
    NST_OBJ_HEAD;
    Nst_Int value;
}
Nst_IntObj;

EXPORT typedef struct _Nst_RealObj
{
    NST_OBJ_HEAD;
    Nst_Real value;
}
Nst_RealObj;

EXPORT typedef struct _Nst_BoolObj
{
    NST_OBJ_HEAD;
    Nst_Bool value;
}
Nst_BoolObj;

EXPORT typedef struct _Nst_ByteObj
{
    NST_OBJ_HEAD;
    Nst_Byte value;
}
Nst_ByteObj;

EXPORT typedef FILE *Nst_IOFile;
EXPORT typedef usize (*Nst_IOFile_read_f) (void  *buf,
                                           usize size,
                                           usize count,
                                           void  *f_value);
EXPORT typedef usize (*Nst_IOFile_write_f)(void  *buf,
                                           usize size,
                                           usize count,
                                           void  *f_value);
EXPORT typedef i32   (*Nst_IOFile_flush_f)(void *f_value);
EXPORT typedef i32   (*Nst_IOFile_tell_f) (void *f_value);
EXPORT typedef i32   (*Nst_IOFile_seek_f) (void *f_value, i32 offset, i32 origin);
EXPORT typedef i32   (*Nst_IOFile_close_f)(void *f_value);

EXPORT typedef struct _Nst_IOFileObj
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

EXPORT typedef enum _Nst_IOFileFlag
{
    NST_FLAG_IOFILE_IS_CLOSED = 0b0001,
    NST_FLAG_IOFILE_IS_BIN    = 0b0010,
    NST_FLAG_IOFILE_CAN_WRITE = 0b0100,
    NST_FLAG_IOFILE_CAN_READ  = 0b1000
}
Nst_IOFileFlag;

// Creates a new Int object
EXPORT Nst_Obj *nst_int_new(Nst_Int value, Nst_OpErr *err);
// Creates a new Real object
EXPORT Nst_Obj *nst_real_new(Nst_Real value, Nst_OpErr *err);
// Creates a new Bool object
EXPORT Nst_Obj *nst_bool_new(Nst_Bool value, Nst_OpErr *err);
// Creates a new Byte object
EXPORT Nst_Obj *nst_byte_new(Nst_Byte value, Nst_OpErr *err);
// Creates a new IOFile object, bin: is opened in binary format,
// read: supports reading, write: supports writing
EXPORT Nst_Obj *nst_iof_new(Nst_IOFile value,
                            bool       bin,
                            bool       read,
                            bool       write,
                            Nst_OpErr *err);
EXPORT Nst_Obj *nst_iof_new_fake(void *value,
                                 bool bin, bool read, bool write,
                                 Nst_IOFile_read_f  read_f,
                                 Nst_IOFile_write_f write_f,
                                 Nst_IOFile_flush_f flush_f,
                                 Nst_IOFile_tell_f  tell_f,
                                 Nst_IOFile_seek_f  seek_f,
                                 Nst_IOFile_close_f close_f,
                                 Nst_OpErr *err);

EXPORT void _nst_iofile_destroy(Nst_IOFileObj *obj);

EXPORT isize nst_fread(void          *buf,
                       usize          size,
                       usize          count,
                       Nst_IOFileObj *f);
EXPORT isize nst_fwrite(void          *buf,
                        usize          size,
                        usize          count,
                        Nst_IOFileObj *f);
EXPORT i32 nst_fflush(Nst_IOFileObj *f);
EXPORT i32 nst_ftell(Nst_IOFileObj *f);
EXPORT i32 nst_fseek(Nst_IOFileObj *f, i32 offset, i32 origin);
EXPORT i32 nst_fclose(Nst_IOFileObj *f);

EXPORT u8  _nst_number_to_u8(Nst_Obj *number);
EXPORT int _nst_number_to_int(Nst_Obj *number);
EXPORT i32 _nst_number_to_i32(Nst_Obj *number);
EXPORT i64 _nst_number_to_i64(Nst_Obj *number);
EXPORT f32 _nst_number_to_f32(Nst_Obj *number);
EXPORT f64 _nst_number_to_f64(Nst_Obj *number);
EXPORT Nst_Bool _nst_obj_to_bool(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H
