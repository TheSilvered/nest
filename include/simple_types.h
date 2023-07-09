/* Inteface for Nst_IntObj, Nst_RealObj, Nst_ByteObj, Nst_BoolObj, Nst_IOFileObj */

#ifndef SYMPLE_TYPES_H
#define SYMPLE_TYPES_H

#include <stdio.h>
#include "error.h"

#define Nst_TRUE 1
#define Nst_FALSE 0

#define AS_INT(ptr)  (((Nst_IntObj  *)(ptr))->value)
#define AS_REAL(ptr) (((Nst_RealObj *)(ptr))->value)
#define AS_BYTE(ptr) (((Nst_ByteObj *)(ptr))->value)
#define AS_BOOL(ptr) (((Nst_BoolObj *)(ptr))->value)
#define IOFILE(ptr) ((Nst_IOFileObj *)(ptr))

#define Nst_IOF_IS_CLOSED(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_IS_CLOSED)
#define Nst_IOF_IS_BIN(f)    Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_IS_BIN)
#define Nst_IOF_CAN_WRITE(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_CAN_WRITE)
#define Nst_IOF_CAN_READ(f)  Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_CAN_READ)

#define Nst_number_to_u8(number) _Nst_number_to_u8(OBJ(number))
#define Nst_number_to_int(number) _Nst_number_to_int(OBJ(number))
#define Nst_number_to_i32(number) _Nst_number_to_i32(OBJ(number))
#define Nst_number_to_i64(number) _Nst_number_to_i64(OBJ(number))
#define Nst_number_to_f32(number) _Nst_number_to_f32(OBJ(number))
#define Nst_number_to_f64(number) _Nst_number_to_f64(OBJ(number))
#define Nst_obj_to_bool(obj) _Nst_obj_to_bool(OBJ(obj))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef i64 Nst_Int;
NstEXP typedef f64 Nst_Real;
NstEXP typedef i8 Nst_Bool;
NstEXP typedef u8 Nst_Byte;

NstEXP typedef struct _Nst_IntObj
{
    Nst_OBJ_HEAD;
    Nst_Int value;
}
Nst_IntObj;

NstEXP typedef struct _Nst_RealObj
{
    Nst_OBJ_HEAD;
    Nst_Real value;
}
Nst_RealObj;

NstEXP typedef struct _Nst_BoolObj
{
    Nst_OBJ_HEAD;
    Nst_Bool value;
}
Nst_BoolObj;

NstEXP typedef struct _Nst_ByteObj
{
    Nst_OBJ_HEAD;
    Nst_Byte value;
}
Nst_ByteObj;

NstEXP typedef FILE *Nst_IOFile;
NstEXP typedef usize (*Nst_IOFile_read_f) (void  *buf,
                                           usize size,
                                           usize count,
                                           void  *f_value);
NstEXP typedef usize (*Nst_IOFile_write_f)(void  *buf,
                                           usize size,
                                           usize count,
                                           void  *f_value);
NstEXP typedef i32   (*Nst_IOFile_flush_f)(void *f_value);
NstEXP typedef i32   (*Nst_IOFile_tell_f) (void *f_value);
NstEXP typedef i32   (*Nst_IOFile_seek_f) (void *f_value, i32 offset, i32 origin);
NstEXP typedef i32   (*Nst_IOFile_close_f)(void *f_value);

NstEXP typedef struct _Nst_IOFileObj
{
    Nst_OBJ_HEAD;
    Nst_IOFile value;
    Nst_IOFile_read_f  read_f;
    Nst_IOFile_write_f write_f;
    Nst_IOFile_flush_f flush_f;
    Nst_IOFile_tell_f  tell_f;
    Nst_IOFile_seek_f  seek_f;
    Nst_IOFile_close_f close_f;
}
Nst_IOFileObj;

NstEXP typedef enum _Nst_IOFileFlag
{
    Nst_FLAG_IOFILE_IS_CLOSED = 0b0001,
    Nst_FLAG_IOFILE_IS_BIN    = 0b0010,
    Nst_FLAG_IOFILE_CAN_WRITE = 0b0100,
    Nst_FLAG_IOFILE_CAN_READ  = 0b1000
}
Nst_IOFileFlag;

// Creates a new Int object
NstEXP Nst_Obj *NstC Nst_int_new(Nst_Int value);
// Creates a new Real object
NstEXP Nst_Obj *NstC Nst_real_new(Nst_Real value);
// Creates a new Bool object
NstEXP Nst_Obj *NstC Nst_bool_new(Nst_Bool value);
// Creates a new Byte object
NstEXP Nst_Obj *NstC Nst_byte_new(Nst_Byte value);
// Creates a new IOFile object, bin: is opened in binary format,
// read: supports reading, write: supports writing
NstEXP Nst_Obj *NstC Nst_iof_new(Nst_IOFile value, bool bin, bool read,
                                 bool write);
NstEXP Nst_Obj *NstC Nst_iof_new_fake(void *value, bool bin, bool read,
                                      bool write, Nst_IOFile_read_f read_f,
                                      Nst_IOFile_write_f write_f,
                                      Nst_IOFile_flush_f flush_f,
                                      Nst_IOFile_tell_f tell_f,
                                      Nst_IOFile_seek_f seek_f,
                                      Nst_IOFile_close_f close_f);

NstEXP void NstC _Nst_iofile_destroy(Nst_IOFileObj *obj);

NstEXP isize NstC Nst_fread(void *buf, usize size, usize count,
                            Nst_IOFileObj *f);
NstEXP isize NstC Nst_fwrite(void *buf, usize size, usize count,
                             Nst_IOFileObj *f);
NstEXP i32 NstC Nst_fflush(Nst_IOFileObj *f);
NstEXP i32 NstC Nst_ftell(Nst_IOFileObj *f);
NstEXP i32 NstC Nst_fseek(Nst_IOFileObj *f, i32 offset, i32 origin);
NstEXP i32 NstC Nst_fclose(Nst_IOFileObj *f);

NstEXP u8  NstC _Nst_number_to_u8(Nst_Obj *number);
NstEXP int NstC _Nst_number_to_int(Nst_Obj *number);
NstEXP i32 NstC _Nst_number_to_i32(Nst_Obj *number);
NstEXP i64 NstC _Nst_number_to_i64(Nst_Obj *number);
NstEXP f32 NstC _Nst_number_to_f32(Nst_Obj *number);
NstEXP f64 NstC _Nst_number_to_f64(Nst_Obj *number);
NstEXP Nst_Bool NstC _Nst_obj_to_bool(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H
