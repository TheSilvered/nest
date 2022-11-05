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

#define OBJ(obj) ((Nst_Obj *)(obj))

// flags can support up to 4 flags
// the top 4 are reserved for the garbage collector
#define NST_OBJ_HEAD \
    int ref_count; \
    struct _Nst_StrObj *type; \
    void (*destructor)(void *); \
    int32_t hash; \
    unsigned char flags

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

struct _Nst_StrObj;

typedef struct _Nst_Obj
{
    NST_OBJ_HEAD;
}
Nst_Obj;

typedef void (*Nst_ObjDestructor)(void *);

typedef Nst_Obj Nst_NullObj;

// Create a new object of size `size` on the heap
Nst_Obj *_nst_alloc_obj(size_t size, struct _Nst_StrObj *type, void (*destructor)(void *));
// Call the objec's destructor and deallocate its memory
void _nst_destroy_obj(Nst_Obj *obj);

// Increase the reference count of the object
Nst_Obj *_nst_inc_ref(Nst_Obj *obj);
// Decrease the reference count of the object
void _nst_dec_ref(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_H
