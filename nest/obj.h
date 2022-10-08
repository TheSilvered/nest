/* Nest Object interface */

#ifndef OBJ_H
#define OBJ_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define nst_inc_ref(obj) _nst_inc_ref((Nst_Obj *)(obj))
#define nst_dec_ref(obj) _nst_dec_ref((Nst_Obj *)(obj))
#define nst_destroy_obj(obj) _nst_destroy_obj((Nst_Obj *)(obj))
#define nst_alloc_obj(size, type, destructor) _nst_alloc_obj(size, type, (Nst_ObjDestructor)(destructor))

#define NST_SET_FLAG(obj, flag) ((obj)->flags |= flag)
#define NST_UNSET_FLAG(obj, flag) ((obj)->flags &= ~(flag))
#define NST_HAS_FLAG(obj, flag) ((obj)->flags & (flag))

// flags can support up to 4 flags
// the top 4 are reserved for the garbage collector

#define NST_OBJ_HEAD \
    int ref_count; \
    struct Nst_Obj *type; \
    void (*destructor)(void *); \
    int32_t hash; \
    unsigned char flags

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct Nst_Obj
{
    NST_OBJ_HEAD;
}
Nst_Obj;

typedef void (*Nst_ObjDestructor)(void *);

// Create a new object of size `size` on the heap
Nst_Obj *_nst_alloc_obj(size_t size, Nst_Obj *type, void (*destructor)(void *));
// Create constant object (nst_t_*, nst_true, nst_false, nst_null)
void _nst_init_obj(void);
// Delete constant objects (nst_t_*, nst_true, nst_false, nst_null)
void _nst_del_obj(void);
// Call the objec's destructor and deallocate its memory
void _nst_destroy_obj(Nst_Obj *obj);

// Increase the reference count of the object
Nst_Obj *_nst_inc_ref(Nst_Obj *obj);
// Decrease the reference count of the object
void _nst_dec_ref(Nst_Obj *obj);

extern Nst_Obj *nst_t_type;
extern Nst_Obj *nst_t_int;
extern Nst_Obj *nst_t_real;
extern Nst_Obj *nst_t_bool;
extern Nst_Obj *nst_t_null;
extern Nst_Obj *nst_t_str;
extern Nst_Obj *nst_t_arr;
extern Nst_Obj *nst_t_vect;
extern Nst_Obj *nst_t_map;
extern Nst_Obj *nst_t_func;
extern Nst_Obj *nst_t_iter;
extern Nst_Obj *nst_t_byte;
extern Nst_Obj *nst_t_file;

extern Nst_Obj *nst_true;
extern Nst_Obj *nst_false;
extern Nst_Obj *nst_null;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_H
