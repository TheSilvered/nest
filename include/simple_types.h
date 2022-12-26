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
#define IOFILE(ptr) ((Nst_IOFileObj *)(ptr))

#define NST_IOF_IS_CLOSED(f) ( NST_HAS_FLAG(f, NST_FLAG_IOFILE_IS_CLOSED) )
#define NST_IOF_IS_BIN(f)    ( NST_HAS_FLAG(f, NST_FLAG_IOFILE_IS_BIN) )
#define NST_IOF_CAN_WRITE(f) ( NST_HAS_FLAG(f, NST_FLAG_IOFILE_CAN_WRITE) )
#define NST_IOF_CAN_READ(f)  ( NST_HAS_FLAG(f, NST_FLAG_IOFILE_CAN_READ) )

#define NST_SIMPLE_TYPE_STRUCT(type, alias) \
    typedef type alias; \
    typedef struct _ ## alias ## Obj {  \
        NST_OBJ_HEAD; \
        alias value; \
    } alias ## Obj

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NST_SIMPLE_TYPE_STRUCT(long long, Nst_Int);
NST_SIMPLE_TYPE_STRUCT(long double, Nst_Real);
NST_SIMPLE_TYPE_STRUCT(char, Nst_Bool);
NST_SIMPLE_TYPE_STRUCT(unsigned char, Nst_Byte);
NST_SIMPLE_TYPE_STRUCT(FILE *, Nst_IOFile);

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