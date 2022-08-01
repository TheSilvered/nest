#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stdint.h>
#include "obj.h"
#include "llist.h"

#define VECTOR_MIN_SIZE 8
#define VECTOR_GROWTH_RATIO 1.8f
#define AS_SEQ(ptr)  ((Nst_SeqObj *)(ptr))

#define nst_resize_vector(vect) _nst_resize_vector(AS_SEQ(vect))
#define nst_append_value_vector(vect, val) _nst_append_value_vector(AS_SEQ(vect), (Nst_Obj *)val)
#define nst_set_value_seq(seq, idx, val) _nst_set_value_seq(AS_SEQ(seq), idx, (Nst_Obj *)val)
#define nst_rem_value_vector(vect, val) _nst_rem_value_vector(AS_SEQ(vect), (Nst_Obj *)val)
#define nst_pop_value_vector(vect, quantity) _nst_pop_value_vector(AS_SEQ(vect), quantity);
#define nst_get_value_seq(seq, idx) _nst_get_value_seq(AS_SEQ(seq), idx)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    OBJ_HEAD;
    Nst_Obj **objs;
    size_t len;
    size_t size;
}
Nst_SeqObj; // vector or array

Nst_Obj *nst_new_array(size_t len);
Nst_Obj *nst_new_vector(size_t len);
void nst_destroy_seq(Nst_SeqObj *seq);

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