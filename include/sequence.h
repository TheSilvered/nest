/* Nst_SeqObject interface, implements Array and Vector */

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stdarg.h>
#include "ggc.h"

#define _Nst_VECTOR_MIN_SIZE 8
#define _Nst_VECTOR_GROWTH_RATIO 1.8f
#define SEQ(ptr) ((Nst_SeqObj *)(ptr))
#define ARRAY(ptr) ((Nst_SeqObj *)(ptr))
#define VECTOR(ptr) ((Nst_SeqObj *)(ptr))

// Sets a value at index `idx` of `seq`, this cannot be called if there
// is no valid object at `idx`
#define Nst_seq_set(seq, idx, val) \
    _Nst_seq_set(SEQ(seq), idx, OBJ(val))
// Returns the value at index `idx` of `seq` increasing the reference count
#define Nst_seq_get(seq, idx) _Nst_seq_get(SEQ(seq), idx)

#define Nst_vector_set Nst_seq_set
#define Nst_vector_get Nst_seq_get
#define Nst_array_set Nst_seq_set
#define Nst_array_get Nst_seq_get

// Appends a value to the end of a vector
#define Nst_vector_append(vect, val) \
    _Nst_vector_append(SEQ(vect), OBJ(val))
// Removes the firs occurence of `val` in `vect`
#define Nst_vector_remove(vect, val) \
    _Nst_vector_remove(SEQ(vect), OBJ(val))
// Pops `quantity` values from the end of `vect` and returns the last one popped
#define Nst_vector_pop(vect, quantity) \
    _Nst_vector_pop(SEQ(vect), quantity)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef struct _Nst_SeqObj
{
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_Obj **objs;
    usize len;
    usize size;
}
Nst_SeqObj; // vector or array

NstEXP typedef Nst_SeqObj Nst_ArrayObj;
NstEXP typedef Nst_SeqObj Nst_VectorObj;

// Creates a new array of length `len`, the objects must be set manually inside
NstEXP Nst_Obj *NstC Nst_array_new(usize len);
// Creates a new vector of lenght `len`, the objects must be set manually inside
NstEXP Nst_Obj *NstC Nst_vector_new(usize len);

NstEXP Nst_Obj *NstC Nst_array_create(usize len, ...);
NstEXP Nst_Obj *NstC Nst_vector_create(usize len, ...);

NstEXP Nst_Obj *NstC Nst_array_create_c(const i8 *fmt, ...);
NstEXP Nst_Obj *NstC Nst_vector_create_c(const i8 *fmt, ...);

NstEXP void NstC _Nst_seq_destroy(Nst_SeqObj *seq);
NstEXP void NstC _Nst_seq_traverse(Nst_SeqObj *seq);
NstEXP void NstC _Nst_seq_track(Nst_SeqObj *seq);

NstEXP bool NstC _Nst_seq_set(Nst_SeqObj *seq, i64 idx, Nst_Obj *val);
NstEXP Nst_Obj *NstC _Nst_seq_get(Nst_SeqObj *seq, i64 idx);

NstEXP bool NstC _Nst_vector_resize(Nst_SeqObj *vect);
NstEXP bool NstC _Nst_vector_append(Nst_SeqObj *vect, Nst_Obj *val);
NstEXP Nst_Obj *NstC _Nst_vector_remove(Nst_SeqObj *vect, Nst_Obj *val);
NstEXP Nst_Obj *NstC _Nst_vector_pop(Nst_SeqObj *vect, usize quantity);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif //!SEQUENCE_H
