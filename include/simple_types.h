/**
 * @file simple_types.h
 *
 * @brief Interface for `Int`, `Real`, `Byte` and `Bool`
 *
 * @author TheSilvered
 */

#ifndef SIMPLE_TYPES_H
#define SIMPLE_TYPES_H

#include <stdio.h>
#include "error.h"
#include "encoding.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Create a new `Int` object.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_int_new(i64 value);
/**
 * @return The value of an `Int` object as an `i64`.
 */
NstEXP i64 NstC Nst_int_i64(Nst_Obj *obj);

void _Nst_counter_dec(Nst_Obj *counter);

/**
 * Create a new `Real` object.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_real_new(f64 value);
/**
 * @return The value of a `Real` object as an `f64`.
 */
NstEXP f64 NstC Nst_real_f64(Nst_Obj *obj);
/**
 * @return The value of a `Real` object as an `f32`.
 */
NstEXP f32 NstC Nst_real_f32(Nst_Obj *obj);

/**
 * Create a new `Byte` object.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_byte_new(u8 value);
/**
 * @return The value of a `Byte` object.
 */
NstEXP u8 NstC Nst_byte_u8(Nst_Obj *obj);

/**
 * Convert the value of a numeric object (`Int`, `Real`, `Byte`) to a `u8`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP u8 NstC Nst_number_to_u8(Nst_Obj *number);
/**
 * Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an `int`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP int NstC Nst_number_to_int(Nst_Obj *number);
/**
 * Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an `i32`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP i32 NstC Nst_number_to_i32(Nst_Obj *number);
/**
 * Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an `i64`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0` is
 * returned. No error is set.
 */
NstEXP i64 NstC Nst_number_to_i64(Nst_Obj *number);
/**
 * Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an `f32`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object, `0.0` is
 * returned. No error is set.
 */
NstEXP f32 NstC Nst_number_to_f32(Nst_Obj *number);
/**
 * Convert the value of a numeric object (`Int`, `Real`, `Byte`) to an `f64`.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object `0.0f` is
 * returned. No error is set.
 */
NstEXP f64 NstC Nst_number_to_f64(Nst_Obj *number);

/**
 * @return `true` if the object is truthy and `false` otherwise.
 */
NstEXP bool NstC Nst_obj_to_bool(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SIMPLE_TYPES_H
