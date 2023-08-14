#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "simple_types.h"
#include "global_consts.h"
#include "obj_ops.h"

#define NEW_SYMPLE_TYPE(type, type_obj)                                       \
    type *obj = Nst_obj_alloc(type, type_obj, NULL);                          \
    if (obj == NULL)                                                          \
        return NULL;                                                          \
    obj->value = value;                                                       \
    return OBJ(obj)

Nst_Obj *Nst_int_new(i64 value)
{
    NEW_SYMPLE_TYPE(Nst_IntObj, Nst_t.Int);
}

Nst_Obj *Nst_real_new(f64 value)
{
    NEW_SYMPLE_TYPE(Nst_RealObj, Nst_t.Real);
}

Nst_Obj *Nst_byte_new(u8 value)
{
    NEW_SYMPLE_TYPE(Nst_ByteObj, Nst_t.Byte);
}

Nst_Obj *Nst_bool_new(bool value)
{
    NEW_SYMPLE_TYPE(Nst_BoolObj, Nst_t.Bool);
}

u8 _Nst_number_to_u8(Nst_Obj *number)
{
    return (u8)_Nst_number_to_i64(number);
}

int _Nst_number_to_int(Nst_Obj *number)
{
    return (int)_Nst_number_to_i64(number);
}

i32 _Nst_number_to_i32(Nst_Obj *number)
{
    return (i32)_Nst_number_to_i64(number);
}

i64 _Nst_number_to_i64(Nst_Obj *number)
{
    Nst_TypeObj *t = number->type;

    if (t == Nst_t.Byte)
        return (i64)AS_BYTE(number);
    else if (t == Nst_t.Int)
        return AS_INT(number);
    else if (t == Nst_t.Real)
        return (i64)AS_REAL(number);
    return 0;
}

f32 _Nst_number_to_f32(Nst_Obj *number)
{
    return (f32)_Nst_number_to_f64(number);
}

f64 _Nst_number_to_f64(Nst_Obj *number)
{
    Nst_TypeObj *t = number->type;

    if (t == Nst_t.Byte)
        return (f64)AS_BYTE(number);
    else if (t == Nst_t.Int)
        return (f64)AS_INT(number);
    else if (t == Nst_t.Real)
        return AS_REAL(number);
    return 0.0;
}

bool _Nst_obj_to_bool(Nst_Obj *obj)
{
    Nst_Obj *bool_obj = Nst_obj_cast(obj, Nst_t.Bool);
    Nst_dec_ref(bool_obj);
    return bool_obj == Nst_c.Bool_true;
}
