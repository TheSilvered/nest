/**
 * @file simple_types.h
 *
 * @brief Interface for `Int`, `Real`, `Byte` and `Bool`
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
 * Creates a new `Int` object.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_int_new(i64 value);

/**
 * Creates a new `Real` object.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_real_new(f64 value);
/* Get the value of a `Real` object as an `f64`. */
NstEXP f64 NstC Nst_real_f64(Nst_Obj *obj);
/* Get the value of a `Real` object as an `f32`. */
NstEXP f32 NstC Nst_real_f32(Nst_Obj *obj);

/**
 * Creates a new `Byte` object.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_byte_new(u8 value);
/* Get the value of a `Byte` object. */
NstEXP u8 NstC Nst_byte_u8(Nst_Obj *obj);

/**
 * Converts the value of a numeric object (`Int`, `Real`, `Byte`) to a `u8`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP u8  NstC Nst_number_to_u8(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Int`, `Real`, `Byte`) to an `int`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP int NstC Nst_number_to_int(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Int`, `Real`, `Byte`) to an `i32`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP i32 NstC Nst_number_to_i32(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Int`, `Real`, `Byte`) to an `i64`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP i64 NstC Nst_number_to_i64(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Int`, `Real`, `Byte`) to an `f32`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0.0` is
 * returned. No error is set.
 */
NstEXP f32 NstC Nst_number_to_f32(Nst_Obj *number);
/**
 * Converts the value of a numeric object (`Int`, `Real`, `Byte`) to an `f64`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object `0.0f` is
 * returned. No error is set.
 */
NstEXP f64 NstC Nst_number_to_f64(Nst_Obj *number);

/**
 * @brief Converts any object to a boolean. Exactly the same as casting the
 * object to `Nst_type()->Bool` and then checking if the result matches
 * `Nst_true()`.
 */
NstEXP bool NstC Nst_obj_to_bool(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H
