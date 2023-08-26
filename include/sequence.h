/**
 * @file sequence.h
 *
 * @brief Nst_SeqObject interface for Arrays and Vectors
 *
 * @author TheSilvered
 */

/* [docs:link sequence.h c_api-sequence.md#sequence-creation-format-types] */

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
    The `fmt` string **cannot** contain whitespace since its length is used to
    determine the length of the sequence.
*/

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stdarg.h>
#include "ggc.h"

#define _Nst_VECTOR_MIN_SIZE 8
#define _Nst_VECTOR_GROWTH_RATIO 1.8f
#define SEQ(ptr) ((Nst_SeqObj *)(ptr))
#define ARRAY(ptr) ((Nst_SeqObj *)(ptr))
#define VECTOR(ptr) ((Nst_SeqObj *)(ptr))

/**
 * @brief Alias for `_Nst_seq_set` that casts `seq` to `Nst_SeqObj *` and `val`
 * to `Nst_Obj *`.
 */
#define Nst_seq_set(seq, idx, val) _Nst_seq_set(SEQ(seq), idx, OBJ(val))
/* Alias for `_Nst_seq_get` that casts `seq` to `Nst_SeqObj *`. */
#define Nst_seq_get(seq, idx) _Nst_seq_get(SEQ(seq), idx)

/* Alias of `Nst_seq_set`. */
#define Nst_vector_set Nst_seq_set
/* Alias of `Nst_seq_get`. */
#define Nst_vector_get Nst_seq_get
/* Alias of `Nst_seq_set`. */
#define Nst_array_set Nst_seq_set
/* Alias of `Nst_seq_set`. */
#define Nst_array_get Nst_seq_get

/**
 * @brief Alias of `_Nst_vector_append` that casts `vect` to `Nst_SeqObj *` and
 * `val` to `Nst_Obj *`.
 */
#define Nst_vector_append(vect, val) _Nst_vector_append(SEQ(vect), OBJ(val))
/**
 * @brief Alias of `_Nst_vector_remove` that casts `vect` to `Nst_SeqObj *` and
 * `val` to `Nst_Obj *`.
 */
#define Nst_vector_remove(vect, val) _Nst_vector_remove(SEQ(vect), OBJ(val))
/* Alias of `_Nst_vector_pop` that casts `vect` to `Nst_SeqObj *`. */
#define Nst_vector_pop(vect, quantity) _Nst_vector_pop(SEQ(vect), quantity)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * A structure representing a Nest sequence object.
 *
 * @param objs: the array of objects inside the sequence
 * @param len: the length of the sequence
 * @param cap: the capacity of the sequence
 */
NstEXP typedef struct _Nst_SeqObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_Obj **objs;
    usize len;
    usize cap;
} Nst_SeqObj;

/* Type added for C type completion. */
NstEXP typedef Nst_SeqObj Nst_ArrayObj;
/* Type added for C type completion. */
NstEXP typedef Nst_SeqObj Nst_VectorObj;

/**
 * Creates a new array object of the specified length.
 *
 * @brief Warning: the objects inside the array must be set manually, each
 * entry in the array takes one reference of the object inserted.
 *
 * @param len: the length of the array to create
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_array_new(usize len);
/**
 * Creates a new vector object of the specified length.
 *
 * @brief Warning: the objects inside the vector must be set manually, each
 * entry in the vector takes one reference of the object inserted.
 *
 * @param len: the length of the vector to create
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_vector_new(usize len);

/**
 * Creates an array object of the length specified, inserting the objects
 * inside.
 *
 * @brief The number of varargs passed to the function must match the number
 * given in the len parameter.
 *
 * @param len: the length of the array to create
 * @param ...: the objects to insert in the array, a reference is taken from
 * each object
 *
 * @return The new array on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_array_create(usize len, ...);
/**
 * Creates a vector object of the length specified, inserting the objects
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
NstEXP Nst_Obj *NstC Nst_vector_create(usize len, ...);

/**
 * Creates an array object, creating the contained objects from C values.
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
NstEXP Nst_Obj *NstC Nst_array_create_c(const i8 *fmt, ...);
/**
 * Creates a vector object, creating the contained objects from C values.
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
NstEXP Nst_Obj *NstC Nst_vector_create_c(const i8 *fmt, ...);

/* Destructor for sequence objects. */
NstEXP void NstC _Nst_seq_destroy(Nst_SeqObj *seq);
/* Traverse function for sequence objects. */
NstEXP void NstC _Nst_seq_traverse(Nst_SeqObj *seq);
/* Track function for sequence objects. */
NstEXP void NstC _Nst_seq_track(Nst_SeqObj *seq);

/**
 * Changes the value of an index in a sequence.
 *
 * @brief The sequence to change must already contain valid values. `idx` can
 * be negative in which case it is subtracted from the length of the sequence
 * to get the new index.
 *
 * @param seq: the sequence to modify
 * @param idx: the index to update
 * @param val: the value to set the index to
 *
 * @return `true` on success and `false` on failure. The error is set. This
 * function fails when the index is outside the sequence.
 */
NstEXP bool NstC _Nst_seq_set(Nst_SeqObj *seq, i64 idx, Nst_Obj *val);
/**
 * Gets the value at an index of a sequence.
 *
 * @brief The sequence must already contain valid values.
 *
 * @param seq: the sequence to get the value from
 * @param idx: the index of the value to get
 *
 * @return A new reference to the object at `idx` on success and `NULL` on
 * failure. The error is set. The function fails when the index is outside the
 * sequence.
 */
NstEXP Nst_Obj *NstC _Nst_seq_get(Nst_SeqObj *seq, i64 idx);

/**
 * Resizes a vector if needed.
 *
 * @param vect: the vector to resize
 *
 * @return `true` on success and `false` on failure. The error is set. The
 * function never fails if the vector is untouched or is shrunk.
 */
NstEXP bool NstC _Nst_vector_resize(Nst_SeqObj *vect);
/**
 * Appends a value to the end of a vector.
 *
 * @param vect: the vector to append the value to
 * @param val: the value to append
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC _Nst_vector_append(Nst_SeqObj *vect, Nst_Obj *val);
/**
 * Removes the first occurrence of a value inside a vector.
 *
 * @param vect: the vector to remove the value from
 * @param val: an object that is equal to the value to remove, the equality is
 * checked with `Nst_obj_eq`
 *
 * @return `true` if the object was removed and `false` if there was no object
 * that matched. No error is set.
 */
NstEXP bool NstC _Nst_vector_remove(Nst_SeqObj *vect, Nst_Obj *val);
/**
 * Pops a certain number of values from the end of a vector.
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
NstEXP Nst_Obj *NstC _Nst_vector_pop(Nst_SeqObj *vect, usize quantity);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif //!SEQUENCE_H
