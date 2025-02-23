#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "simple_types.h"
#include "global_consts.h"
#include "obj_ops.h"

#define NEW_SIMPLE_TYPE(type, type_obj)                                       \
    type *obj = Nst_obj_alloc(type, type_obj);                                \
    if (obj == NULL)                                                          \
        return NULL;                                                          \
    obj->value = value;                                                       \
    return NstOBJ(obj)

NstEXP typedef struct _Nst_IntObj {
    Nst_OBJ_HEAD;
    i64 value;
} Nst_IntObj;

typedef struct _Nst_RealObj {
    Nst_OBJ_HEAD;
    f64 value;
} Nst_RealObj;

typedef struct _Nst_ByteObj {
    Nst_OBJ_HEAD;
    u8 value;
} Nst_ByteObj;

Nst_Obj *Nst_int_new(i64 value)
{
    NEW_SIMPLE_TYPE(Nst_IntObj, Nst_t.Int);
}

i64 Nst_int_i64(Nst_Obj *obj)
{
    return ((Nst_IntObj *)obj)->value;
}

void _Nst_counter_dec(Nst_Obj *counter)
{
    --((Nst_IntObj *)counter)->value;
}

Nst_Obj *Nst_real_new(f64 value)
{
    NEW_SIMPLE_TYPE(Nst_RealObj, Nst_t.Real);
}

f64 Nst_real_f64(Nst_Obj *obj)
{
    return ((Nst_RealObj *)obj)->value;
}

f32 Nst_real_f32(Nst_Obj *obj)
{
    return (f32)((Nst_RealObj *)obj)->value;
}

Nst_Obj *Nst_byte_new(u8 value)
{
    NEW_SIMPLE_TYPE(Nst_ByteObj, Nst_t.Byte);
}

u8 Nst_byte_u8(Nst_Obj *obj)
{
    return ((Nst_ByteObj *)obj)->value;
}

u8 Nst_number_to_u8(Nst_Obj *number)
{
    return (u8)Nst_number_to_i64(number);
}

int Nst_number_to_int(Nst_Obj *number)
{
    return (int)Nst_number_to_i64(number);
}

i32 Nst_number_to_i32(Nst_Obj *number)
{
    return (i32)Nst_number_to_i64(number);
}

i64 Nst_number_to_i64(Nst_Obj *number)
{
    Nst_Obj *t = number->type;

    if (t == Nst_t.Byte)
        return (i64)Nst_byte_u8(number);
    else if (t == Nst_t.Int)
        return Nst_int_i64(number);
    else if (t == Nst_t.Real)
        return (i64)Nst_real_f64(number);
    return 0;
}

f32 Nst_number_to_f32(Nst_Obj *number)
{
    return (f32)Nst_number_to_f64(number);
}

f64 Nst_number_to_f64(Nst_Obj *number)
{
    Nst_Obj *t = number->type;

    if (t == Nst_t.Byte)
        return (f64)Nst_byte_u8(number);
    else if (t == Nst_t.Int)
        return (f64)Nst_int_i64(number);
    else if (t == Nst_t.Real)
        return Nst_real_f64(number);
    return 0.0;
}

bool Nst_obj_to_bool(Nst_Obj *obj)
{
    Nst_Obj *bool_obj = Nst_obj_cast(obj, Nst_t.Bool);
    Nst_dec_ref(bool_obj);
    return bool_obj == Nst_c.Bool_true;
}
