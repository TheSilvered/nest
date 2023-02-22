/* Nst_SeqObject interface, implements Array and Vector */

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "ggc.h"

#define _NST_VECTOR_MIN_SIZE 8
#define _NST_VECTOR_GROWTH_RATIO 1.8f
#define SEQ(ptr) ((Nst_SeqObj *)(ptr))
#define ARRAY(ptr) ((Nst_SeqObj *)(ptr))
#define VECTOR(ptr) ((Nst_SeqObj *)(ptr))

// Sets a value at index `idx` of `seq`, this cannot be called if there
// is no valid object at `idx`
#define nst_seq_set(seq, idx, val) \
    _nst_seq_set(SEQ(seq), idx, OBJ(val))
// Returns the value at index `idx` of `seq` increasing the reference count
#define nst_seq_get(seq, idx) _nst_seq_get(SEQ(seq), idx)

#define nst_vector_set nst_seq_set
#define nst_vector_get nst_seq_get
#define nst_array_set nst_seq_set
#define nst_array_get nst_seq_get

// Resizes the vector if it is full, callled automatically with
// nst_append_value_vector, nst_rem_value_vector and nst_pop_value_vector
#define nst_vector_resize(vect) _nst_vector_resize(SEQ(vect))
// Appends a value to the end of a vector
#define nst_vector_append(vect, val) \
    _nst_vector_append(SEQ(vect), OBJ(val))
// Removes the firs occurence of `val` in `vect`
#define nst_vector_remove(vect, val) \
    _nst_vector_remove(SEQ(vect), OBJ(val))
// Pops `quantity` values from the end of `vect` and returns the last one popped
#define nst_vector_pop(vect, quantity) \
    _nst_vector_pop(SEQ(vect), quantity)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_SeqObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    Nst_Obj **objs;
    usize len;
    usize size;
}
Nst_SeqObj; // vector or array

typedef Nst_SeqObj Nst_ArrayObj;
typedef Nst_SeqObj Nst_VectorObj;

// Creates a new array of length `len`, the objects must be set manually inside
Nst_Obj *nst_array_new(usize len);
// Creates a new vector of lenght `len`, the objects must be set manually inside
Nst_Obj *nst_vector_new(usize len);
void _nst_seq_destroy(Nst_SeqObj *seq);
void _nst_seq_traverse(Nst_SeqObj *seq);
void _nst_seq_track(Nst_SeqObj *seq);

bool _nst_seq_set(Nst_SeqObj *seq, i64 idx, Nst_Obj *val);
Nst_Obj *_nst_seq_get(Nst_SeqObj *seq, i64 idx);

void _nst_vector_resize(Nst_SeqObj *vect);
void _nst_vector_append(Nst_SeqObj *vect, Nst_Obj *val);
Nst_Obj *_nst_vector_remove(Nst_SeqObj *vect, Nst_Obj *val);
Nst_Obj *_nst_vector_pop(Nst_SeqObj *vect, usize quantity);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif //!SEQUENCE_H