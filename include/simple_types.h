/**
 * @file simple_types.h
 *
 * @brief Interface for `Nst_IntObj`, `Nst_RealObj`, `Nst_ByteObj` and
 * `Nst_BoolObj`
 *
 * @author TheSilvered
 */

#ifndef SYMPLE_TYPES_H
#define SYMPLE_TYPES_H

#include <stdio.h>
#include "error.h"
#include "encoding.h"

/* Casts `ptr` to a `Nst_IntObj *` and extracts the value field. */
#define AS_INT(ptr)  (((Nst_IntObj  *)(ptr))->value)
/* Casts `ptr` to a `Nst_RealObj *` and extracts the value field. */
#define AS_REAL(ptr) (((Nst_RealObj *)(ptr))->value)
/* Casts `ptr` to a `Nst_ByteObj *` and extracts the value field. */
#define AS_BYTE(ptr) (((Nst_ByteObj *)(ptr))->value)
/* Casts `ptr` to a `Nst_BoolObj *` and extracts the value field. */
#define AS_BOOL(ptr) (((Nst_BoolObj *)(ptr))->value)

/* Alias for `_Nst_number_to_u8` that casts `number` to `Nst_Obj *`. */
#define Nst_number_to_u8(number) _Nst_number_to_u8(OBJ(number))
/* Alias for `_Nst_number_to_int `that casts `number` to `Nst_Obj *`. */
#define Nst_number_to_int(number) _Nst_number_to_int(OBJ(number))
/* Alias for `_Nst_number_to_i32` that casts `number` to `Nst_Obj *`. */
#define Nst_number_to_i32(number) _Nst_number_to_i32(OBJ(number))
/* Alias for `_Nst_number_to_i64` that casts `number` to `Nst_Obj *`. */
#define Nst_number_to_i64(number) _Nst_number_to_i64(OBJ(number))
/* Alias for `_Nst_number_to_f32` that casts `number` to `Nst_Obj *`. */
#define Nst_number_to_f32(number) _Nst_number_to_f32(OBJ(number))
/* Alias for `_Nst_number_to_f64` that casts `number` to `Nst_Obj *`. */
#define Nst_number_to_f64(number) _Nst_number_to_f64(OBJ(number))
/* Alias for `_Nst_obj_to_bool` that casts `obj` to `Nst_Obj *`. */
#define Nst_obj_to_bool(obj) _Nst_obj_to_bool(OBJ(obj))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * A structure representing a Nest integer object.
 *
 * @param value: the value of the integer
 */
NstEXP typedef struct _Nst_IntObj {
    Nst_OBJ_HEAD;
    i64 value;
} Nst_IntObj;

/**
 * A structure representing a Nest real number (floating-point) object.
 *
 * @param value: the value of the real number
 */
NstEXP typedef struct _Nst_RealObj {
    Nst_OBJ_HEAD;
    f64 value;
} Nst_RealObj;

/**
 * A structure representing a Nest boolean object.
 *
 * @param value: the value of the boolean
 */
NstEXP typedef struct _Nst_BoolObj {
    Nst_OBJ_HEAD;
    bool value;
} Nst_BoolObj;

/**
 * A structure representing a Nest byte object.
 *
 * @param value: the value of the byte
 */
NstEXP typedef struct _Nst_ByteObj {
    Nst_OBJ_HEAD;
    u8 value;
} Nst_ByteObj;

/**
 * Creates a new `Nst_IntObj`.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_int_new(i64 value);
/**
 * Creates a new `Nst_RealObj`.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_real_new(f64 value);
/**
 * Creates a new `Nst_BoolObj`.
 *
 * @brief This function should never be called, to get the `true` and `false`
 * objects use `Nst_true()` and `Nst_false()` instead. Note that these
 * functions do not return a new reference to the respective objects.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_bool_new(bool value);
/**
 * Creates a new `Nst_ByteObj`.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_byte_new(u8 value);

/**
 * Converts the value of a numeric object (`Nst_IntObj`, `Nst_RealObj`,
 * `Nst_ByteObj`) to a `u8`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP u8  NstC _Nst_number_to_u8(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Nst_IntObj`, `Nst_RealObj`,
 * `Nst_ByteObj`) to an `int`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP int NstC _Nst_number_to_int(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Nst_IntObj`, `Nst_RealObj`,
 * `Nst_ByteObj`) to an `i32`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP i32 NstC _Nst_number_to_i32(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Nst_IntObj`, `Nst_RealObj`,
 * `Nst_ByteObj`) to an `i64`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP i64 NstC _Nst_number_to_i64(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Nst_IntObj`, `Nst_RealObj`,
 * `Nst_ByteObj`) to an `f32`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0.0` is
 * returned. No error is set.
 */
NstEXP f32 NstC _Nst_number_to_f32(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Nst_IntObj`, `Nst_RealObj`,
 * `Nst_ByteObj`) to an `f64`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object `0.0f` is
 * returned. No error is set.
 */
NstEXP f64 NstC _Nst_number_to_f64(Nst_Obj *number);
/* [docs:link Nst_type()->Bool c_api-global_consts.md#nst_type] */
/**
 * @brief Converts any object to a boolean. Exactly the same as casting the
 * object to `Nst_type()->Bool` and then checking if the result matches
 * `Nst_true()`.
 */
NstEXP bool NstC _Nst_obj_to_bool(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H
