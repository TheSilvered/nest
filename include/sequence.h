/* Nst_SeqObject interface, implements Array and Vector */

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stdint.h>
#include "obj.h"
#include "llist.h"
#include "ggc.h"

#define VECTOR_MIN_SIZE 8
#define VECTOR_GROWTH_RATIO 1.8f
#define SEQ(ptr)  ((Nst_SeqObj *)(ptr))

// Resizes the vector if it is full, callled automatically with
// nst_append_value_vector, nst_rem_value_vector and nst_pop_value_vector
#define nst_resize_vector(vect) _nst_resize_vector(SEQ(vect))
// Appends a value to the end of a vector
#define nst_append_value_vector(vect, val) \
    _nst_append_value_vector(SEQ(vect), OBJ(val))
// Sets a value at index `idx` of `seq`, this cannot be called if there
// is no valid object at `idx`
#define nst_set_value_seq(seq, idx, val) \
    _nst_set_value_seq(SEQ(seq), idx, OBJ(val))
// Removes the firs occurence of `val` in `vect`
#define nst_rem_value_vector(vect, val) \
    _nst_rem_value_vector(SEQ(vect), OBJ(val))
// Pops `quantity` values from the end of `vect` and returns the last one popped
#define nst_pop_value_vector(vect, quantity) \
    _nst_pop_value_vector(SEQ(vect), quantity)
// Returns the value at index `idx` of `seq` increasing the reference count
#define nst_get_value_seq(seq, idx) _nst_get_value_seq(SEQ(seq), idx)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_SeqObj
{
    NST_OBJ_HEAD;
    NST_GGC_SUPPORT;
    Nst_Obj **objs;
    size_t len;
    size_t size;
}
Nst_SeqObj; // vector or array

typedef Nst_SeqObj Nst_ArrayObj;
typedef Nst_SeqObj Nst_VectorObj;

// Creates a new array of length `len`, the objects must be set manually inside
Nst_Obj *nst_new_array(size_t len);
// Creates a new vector of lenght `len`, the objects must be set manually inside
Nst_Obj *nst_new_vector(size_t len);
void nst_destroy_seq(Nst_SeqObj *seq);
void nst_traverse_seq(Nst_SeqObj *seq);
void nst_track_seq(Nst_SeqObj *seq);
void _nst_resize_vector(Nst_SeqObj *vect);
void _nst_append_value_vector(Nst_SeqObj *vect, Nst_Obj *val);
bool _nst_set_value_seq(Nst_SeqObj *seq, int64_t idx, Nst_Obj *val);
Nst_Obj *_nst_rem_value_vector(Nst_SeqObj *vect, Nst_Obj *val);
Nst_Obj *_nst_pop_value_vector(Nst_SeqObj *vect, size_t quantity);
Nst_Obj *_nst_get_value_seq(Nst_SeqObj *seq, int64_t idx);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif //!SEQUENCE_H