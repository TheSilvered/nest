#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stdint.h>
#include "obj.h"
#include "llist.h"

typedef struct Nst_sequence // vector or array
{
    Nst_Obj **objs;
    size_t len;
    size_t size;
}
Nst_sequence;

Nst_sequence *new_array_empty(size_t len);
Nst_sequence *new_vector_empty(size_t len);
void destroy_seq(Nst_sequence *seq);

void resize_vector(Nst_sequence *vect);
void append_value_vector(Nst_sequence *vect, Nst_Obj *val);
bool set_value_seq(Nst_sequence *seq, int64_t idx, Nst_Obj *val);
Nst_Obj *rem_value_vector(Nst_sequence *vect, Nst_Obj *val);
Nst_Obj *pop_value_vector(Nst_sequence *vect, size_t quantity);
Nst_Obj *get_value_seq(Nst_sequence *seq, int64_t idx);

#define VECTOR_MIN_SIZE 8
#define VECTOR_GROWTH_RATIO 1.8f
#define AS_SEQ(ptr)  ((Nst_sequence *)(ptr->value))
#define AS_SEQ_V(ptr)  ((Nst_sequence *)(ptr))

#endif //!SEQUENCE_H