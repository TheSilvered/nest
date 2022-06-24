#ifndef SYMPLE_TYPES_H
#define SYMPLE_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include "obj.h"

#define NST_TRUE 1
#define NST_FALSE 0

#define AS_INT(ptr)  (*(Nst_int  *)(ptr->value))
#define AS_REAL(ptr) (*(Nst_real *)(ptr->value))
#define AS_BYTE(ptr) (*(Nst_byte *)(ptr->value))
#define AS_BOOL(ptr) (*(Nst_bool *)(ptr->value))
#define AS_FILE(ptr) ((Nst_iofile *)(ptr->value))

#define AS_INT_V(ptr)  (*(Nst_int  *)(ptr))
#define AS_REAL_V(ptr) (*(Nst_real *)(ptr))
#define AS_BYTE_V(ptr) (*(Nst_byte *)(ptr))
#define AS_BOOL_V(ptr) (*(Nst_bool *)(ptr))
#define AS_FILE_V(ptr) ((Nst_iofile *)(value))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef int64_t Nst_int;
typedef double Nst_real;
typedef char Nst_bool;
typedef unsigned char Nst_byte;
typedef FILE Nst_iofile;

Nst_int *new_int(Nst_int value);
Nst_real *new_real(Nst_real value);
Nst_bool *new_bool(Nst_bool value);
Nst_byte *new_byte(Nst_byte value);

Nst_Obj *new_int_obj(Nst_int value);
Nst_Obj *new_real_obj(Nst_real value);
Nst_Obj *new_byte_obj(Nst_byte value);
Nst_Obj *new_file_obj(Nst_iofile *file);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H