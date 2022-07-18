#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <stdint.h>
#include "obj.h"
#include "llist.h"

#define VECTOR_MIN_SIZE 8
#define VECTOR_GROWTH_RATIO 1.8f
#define AS_SEQ(ptr)  ((Nst_SeqObj *)(ptr))

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

Nst_Obj *new_array(size_t len);
Nst_Obj *new_vector(size_t len);
void destroy_seq(Nst_SeqObj *seq);

void resize_vector(Nst_SeqObj *vect);
void append_value_vector(Nst_SeqObj *vect, Nst_Obj *val);
bool set_value_seq(Nst_SeqObj *seq, int64_t idx, Nst_Obj *val);
Nst_Obj *rem_value_vector(Nst_SeqObj *vect, Nst_Obj *val);
Nst_Obj *pop_value_vector(Nst_SeqObj *vect, size_t quantity);
Nst_Obj *get_value_seq(Nst_SeqObj *seq, int64_t idx);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif //!SEQUENCE_H