/**
 * @file sequence.h
 *
 * @brief `Array` and `Vector` objects interface
 *
 * @author TheSilvered
 */

/* [docs:link sequence.h <c_api-sequence.md#sequence-creation-format-types>] */

/* [docs:raw]
## Sequence creation format types

- `I`: `Int` from a 64-bit integer
- `i`: `Int` from a 32-bit integer
- `f`, `F`: `Real` from a double
- `b`: `Bool` from a boolean (promoted to an int)
- `B`: `Byte` from a 8-bit integer
- `o`: an already existing object to take one reference from
- `O`: an already existing object to add one reference to
- `n`: `null`, the vararg can be any pointer as its value is ignored but `NULL`
  is preferred

!!!note
    The `fmt` string **cannot** contain whitespace.
*/

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stdarg.h>
#include "ggc.h"

/* The minimum capacity of a Vector object. */
#define _Nst_VECTOR_MIN_CAP 8
/* Growth ratio of a Vector object. */
#define _Nst_VECTOR_GROWTH_RATIO 1.8f

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

Nst_ObjRef *_Nst_array_new_empty(usize len);
Nst_ObjRef *_Nst_vector_new_empty(usize len);

/**
 * Create a new `Array` object of the specified length. The slots are filled
 * with `null` values.
 *
 * @param len: the length of the array to create
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_array_new(usize len);
/**
 * Create a new `Vector` object of the specified length. The slots are filled
 * with `null` values.
 *
 * @param len: the length of the vector to create
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_vector_new(usize len);

/**
 * Create a new `Array` object given an array of objects. A reference is added to
 * each object.
 *
 * @param len: the number of objects in `objs`
 * @param objs: the objects to use to initialize the array
 *
 * @return The new object on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_array_from_objs(usize len, Nst_Obj **objs);
/**
 * Create a new `Vector` object given an array of objects. A reference is added
 * to each object.
 *
 * @param len: the number of objects in `objs`
 * @param objs: the objects to use to initialize the array
 *
 * @return The new object on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_vector_from_objs(usize len, Nst_Obj **objs);
/**
 * Create a new `Array` object given an array of objects. A reference is taken
 * from each object.
 *
 * @param len: the number of objects in `objs`
 * @param objs: the objects to use to initialize the array
 *
 * @return The new object on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_array_from_objsn(usize len, Nst_ObjRef **objs);
/**
 * Create a new `Vector` object given an array of objects. A reference is taken
 * from each object.
 *
 * @param len: the number of objects in `objs`
 * @param objs: the objects to use to initialize the array
 *
 * @return The new object on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_vector_from_objsn(usize len, Nst_ObjRef **objs);

/**
 * Create a new `Array` object of the length specified, inserting the objects
 * inside.
 *
 * @brief The number of varargs passed to the function must match the number
 * given in the `len` parameter.
 *
 * @param len: the length of the array to create
 * @param ...: the objects to insert in the array, a reference is taken from
 * each object
 *
 * @return The new array on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_array_create(usize len, ...);
/**
 * Create a new `Vector` object of the length specified, inserting the objects
 * inside.
 *
 * @brief The number of varargs passed to the function must match the number
 * given in the `len` parameter.
 *
 * @param len: the length of the vector to create
 * @param ...: the objects to insert in the vector, a reference is taken from
 * each object
 *
 * @return The new vector on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_vector_create(usize len, ...);

/**
 * Create a new `Array` object, creating the contained objects from C values.
 *
 * @brief The number of varargs passed to the function must match the number
 * of types in the fmt argument. For more information about the `fmt` argument
 * check the documentation in `sequence.h`
 *
 * @param fmt: the types of the values passed to the function
 * @param ...: the values passed to the function used to create the objects
 *
 * @return The new array on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_array_create_c(const char *fmt, ...);
/**
 * Creates a new `Vector` object, creating the contained objects from C values.
 *
 * @brief The number of varargs passed to the function must match the number
 * of types in the fmt argument. For more information about the `fmt` argument
 * check the documentation in `sequence.h`
 *
 * @param fmt: the types of the values passed to the function
 * @param ...: the values passed to the function used to create the objects
 *
 * @return The new array on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_vector_create_c(const char *fmt, ...);
/**
 * Create a shallow copy of a sequence.
 *
 * @param seq: the sequence to copy
 *
 * @return The new sequence or NULL on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_seq_copy(Nst_Obj *seq);

void _Nst_seq_destroy(Nst_Obj *seq);

/* `Nst_ObjTrav` function for `Array` and `Vector` objects. */
NstEXP void NstC _Nst_seq_traverse(Nst_Obj *seq);

/* @return The length of a sequence. */
NstEXP usize NstC Nst_seq_len(Nst_Obj *seq);
/* @return The capacity of a vector. */
NstEXP usize NstC Nst_vector_cap(Nst_Obj *vect);
/* @return The object array of a sequence. */
NstEXP Nst_Obj **NstC Nst_seq_objs(Nst_Obj *seq);

/**
 * Change the value of an index in a sequence. Adds a reference to `val`.
 *
 * @brief `idx` can be negative in which case it is subtracted from the length
 * of the sequence to get the new index.
 *
 * @param seq: the sequence to modify
 * @param idx: the index to update
 * @param val: the value to set the index to
 *
 * @return `true` on success and `false` on failure. The error is set. This
 * function fails when the index is outside the sequence.
 */
NstEXP bool NstC Nst_seq_set(Nst_Obj *seq, i64 idx, Nst_Obj *val);
/**
 * Change the value of an index in a sequence quickly. Adds a reference to
 * `val`. Negative indices are not supported.
 *
 * @brief Warning: use this function only if you are certain that `idx` is
 * inside `seq`. Bound checks are only performed in debug builds.
 *
 * @param seq: the sequence to modify
 * @param idx: the index to update
 * @param val: the value to set the index to
 */
NstEXP void NstC Nst_seq_setf(Nst_Obj *seq, usize idx, Nst_Obj *val);
/**
 * Change the value of an index in a sequence. Takes a reference from `val`.
 *
 * @brief `idx` can be negative in which case it is subtracted from the length
 * of the sequence to get the new index.
 *
 * @param seq: the sequence to modify
 * @param idx: the index to update
 * @param val: the value to set the index to
 *
 * @return `true` on success and `false` on failure. The error is set. This
 * function fails when the index is outside the sequence.
 */
NstEXP bool NstC Nst_seq_setn(Nst_Obj *seq, i64 idx, Nst_ObjRef *val);
/**
 * Change the value of an index in a sequence quickly. Takes a reference from
 * `val`. Negative indices are not supported.
 *
 * @brief Warning: use this function only if you are certain that `idx` is
 * inside `seq`. Bound checks are only performed in debug builds.
 *
 * @param seq: the sequence to modify
 * @param idx: the index to update
 * @param val: the value to set the index to
 */
NstEXP void NstC Nst_seq_setnf(Nst_Obj *seq, usize idx, Nst_ObjRef *val);
/**
 * Get a reference to a value in a sequence.
 *
 * @param seq: the sequence to get the value from
 * @param idx: the index of the value to get
 *
 * @return A new reference to the object at `idx` on success and `NULL` on
 * failure. The error is set. The function fails when the index is outside the
 * sequence.
 */
NstEXP Nst_ObjRef *NstC Nst_seq_get(Nst_Obj *seq, i64 idx);
/**
 * Get a reference to a value in a sequence quickly.
 *
 * @brief Warning: use this function only if you are certain that `idx` is
 * inside `seq`. Bound checks are only performed in debug builds.
 *
 * @param seq: the sequence to get the value from
 * @param idx: the index of the value to get
 *
 * @return A new reference to the object at `idx`.
 */
NstEXP Nst_ObjRef *NstC Nst_seq_getf(Nst_Obj *seq, usize idx);
/**
 * Get a value in a sequence without taking a reference.
 *
 * @param seq: the sequence to get the value from
 * @param idx: the index of the value to get
 *
 * @return A pointer to the object at `idx` on success and `NULL` on failure.
 * The error is set. The function fails when the index is outside the sequence.
 */
NstEXP Nst_Obj *NstC Nst_seq_getn(Nst_Obj *seq, i64 idx);
/**
 * Get a value in a sequence without taking a reference quickly.
 *
 * @brief Warning: use this function only if you are certain that `idx` is
 * inside `seq`. Bound checks are only performed in debug mode.
 *
 * @param seq: the sequence to get the value from
 * @param idx: the index of the value to get
 *
 * @return A pointer to the object at `idx`.
 */
NstEXP Nst_Obj *NstC Nst_seq_getnf(Nst_Obj *seq, usize idx);

/**
 * Append a value to the end of a vector adding a reference to `val`.
 *
 * @param vect: the vector to append the value to
 * @param val: the value to append
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_vector_append(Nst_Obj *vect, Nst_Obj *val);
/**
 * Remove the first occurrence of a value inside a vector.
 *
 * @param vect: the vector to remove the value from
 * @param val: an object that is equal to the value to remove, the equality is
 * checked with `Nst_obj_eq`
 *
 * @return `true` if the object was removed and `false` if there was no object
 * that matched. No error is set.
 */
NstEXP bool NstC Nst_vector_remove(Nst_Obj *vect, Nst_Obj *val);
/**
 * Pop a certain number of values from the end of a vector.
 *
 * @brief If the quantity is greater than the length of the vector, it is
 * adapted and the function does not fail.
 *
 * @param vect: the vector to pop the values from
 * @param quantity: the number of values to pop
 *
 * @return The last value popped or `NULL` if no value was popped. No error is
 * set.
 */
NstEXP Nst_ObjRef *NstC Nst_vector_pop(Nst_Obj *vect, usize quantity);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif //!SEQUENCE_H
