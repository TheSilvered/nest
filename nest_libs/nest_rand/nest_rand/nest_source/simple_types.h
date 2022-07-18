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
#define AS_BYTE(ptr) (((Nst_BoolObj *)(ptr))->value)
#define AS_BOOL(ptr) (((Nst_ByteObj *)(ptr))->value)
#define AS_FILE(ptr) ((Nst_IOFileObj *)(ptr))

#define SIMPLE_TYPE_STRUCT(type, type_name, obj_name) \
    typedef type type_name; \
    typedef struct \
    { \
        OBJ_HEAD; \
        type_name value; \
    } \
    obj_name

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef long long Nst_int;
typedef double Nst_real;
typedef char Nst_bool;
typedef unsigned char Nst_byte;
typedef FILE *Nst_iofile;

typedef struct
{
    OBJ_HEAD;
    Nst_int value;
}
Nst_IntObj;

typedef struct
{
    OBJ_HEAD;
    Nst_real value;
}
Nst_RealObj;

typedef struct
{
    OBJ_HEAD;
    Nst_bool value;
}
Nst_BoolObj;

typedef struct
{
    OBJ_HEAD;
    Nst_byte value;
}
Nst_ByteObj;

typedef struct
{
    OBJ_HEAD;
    Nst_iofile value;
    bool is_closed;
    bool is_bin;
    bool can_write;
    bool can_read;
}
Nst_IOFileObj;

Nst_Obj *new_int(Nst_int value);
Nst_Obj *new_real(Nst_real value);
Nst_Obj *new_bool(Nst_bool value);
Nst_Obj *new_byte(Nst_byte value);
Nst_Obj *new_file(Nst_iofile value, bool bin, bool read, bool write);

void destroy_iofile(Nst_IOFileObj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H