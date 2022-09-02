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
typedef double Nst_Real;
typedef char Nst_Bool;
typedef unsigned char Nst_Byte;
typedef FILE *Nst_IOfile;

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
    Nst_IOfile value;
    bool is_closed;
    bool is_bin;
    bool can_write;
    bool can_read;
}
Nst_IOFileObj;

Nst_Obj *nst_new_int(Nst_Int value);
Nst_Obj *nst_new_real(Nst_Real value);
Nst_Obj *nst_new_bool(Nst_Bool value);
Nst_Obj *nst_new_byte(Nst_Byte value);
Nst_Obj *nst_new_file(Nst_IOfile value, bool bin, bool read, bool write);

void nst_destroy_iofile(Nst_IOFileObj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H