#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stddef.h>
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
Nst_sequence *new_array_values(LList *values);

Nst_sequence *new_vector_empty(size_t len);
Nst_sequence *new_vector_values(LList *values);

void resize_vector(Nst_sequence *vect);
void append_value_vector(Nst_sequence *vect, Nst_Obj *val);
void set_value_seq(Nst_sequence *seq, size_t idx, Nst_Obj *val);
void rem_value_vector(Nst_sequence *vect, Nst_Obj *val);
Nst_Obj *pop_value_vector(Nst_sequence *vect, size_t quantity);

#define VECTOR_MIN_SIZE 8
#define VECTOR_GROWTH_RATIO 1.8f

#endif //!SEQUENCE_H