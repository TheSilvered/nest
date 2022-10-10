/* Inteface for Nst_IntObj, Nst_RealObj, Nst_ByteObj, Nst_BoolObj, Nst_IOFileObj */

#ifndef SYMPLE_TYPES_H
#define SYMPLE_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "obj.h"

#define NST_TRUE 1
#define NST_FALSE 0

#define AS_INT(ptr)  (((Nst_IntObj  *)(ptr))->value)
#define AS_REAL(ptr) (((Nst_RealObj *)(ptr))->value)
#define AS_BYTE(ptr) (((Nst_ByteObj *)(ptr))->value)
#define AS_BOOL(ptr) (((Nst_BoolObj *)(ptr))->value)
#define AS_FILE(ptr) ((Nst_IOFileObj *)(ptr))

#define NST_IOF_IS_CLOSED(f) ( NST_HAS_FLAG(f, NST_FLAG_IOFILE_IS_CLOSED) )
#define NST_IOF_IS_BIN(f)    ( NST_HAS_FLAG(f, NST_FLAG_IOFILE_IS_BIN) )
#define NST_IOF_CAN_WRITE(f) ( NST_HAS_FLAG(f, NST_FLAG_IOFILE_CAN_WRITE) )
#define NST_IOF_CAN_READ(f)  ( NST_HAS_FLAG(f, NST_FLAG_IOFILE_CAN_READ) )

#define SIMPLE_TYPE_STRUCT(type, type_name, obj_name) \
    typedef type type_name; \
    typedef struct \
    { \
        NST_OBJ_HEAD; \
        type_name value; \
    } \
    obj_name

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef long long Nst_Int;
typedef long double Nst_Real;
typedef char Nst_Bool;
typedef unsigned char Nst_Byte;
typedef FILE *Nst_IOFile;

typedef struct
{
    NST_OBJ_HEAD;
    Nst_Int value;
}
Nst_IntObj;

typedef struct
{
    NST_OBJ_HEAD;
    Nst_Real value;
}
Nst_RealObj;

typedef struct
{
    NST_OBJ_HEAD;
    Nst_Bool value;
}
Nst_BoolObj;

typedef struct
{
    NST_OBJ_HEAD;
    Nst_Byte value;
}
Nst_ByteObj;

typedef struct
{
    NST_OBJ_HEAD;
    Nst_IOFile value;
}
Nst_IOFileObj;

enum Nst_IOFileFlags {
    NST_FLAG_IOFILE_IS_CLOSED = 0b0001,
    NST_FLAG_IOFILE_IS_BIN    = 0b0010,
    NST_FLAG_IOFILE_CAN_WRITE = 0b0100,
    NST_FLAG_IOFILE_CAN_READ  = 0b1000
};

// Creates a new Int object
Nst_Obj *nst_new_int(Nst_Int value);
// Creates a new Real object
Nst_Obj *nst_new_real(Nst_Real value);
// Creates a new Bool object
Nst_Obj *nst_new_bool(Nst_Bool value);
// Creates a new Byte object
Nst_Obj *nst_new_byte(Nst_Byte value);
// Creates a new IOFile object, bin: is opened in binary format,
// read: supports reading, write: supports writing
Nst_Obj *nst_new_file(Nst_IOFile value, bool bin, bool read, bool write);

void nst_destroy_iofile(Nst_IOFileObj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H