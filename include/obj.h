/* Nest Object interface */

#ifndef OBJ_H
#define OBJ_H

#include "typedefs.h"

#define OBJ(obj) ((Nst_Obj *)(obj))

// Increase the reference count of the object
#define Nst_inc_ref(obj) _Nst_inc_ref(OBJ(obj))
#define Nst_ninc_ref(obj) (obj == NULL ? NULL : _Nst_inc_ref(OBJ(obj)))
// Decrease the reference count of the object
#define Nst_dec_ref(obj) _Nst_dec_ref(OBJ(obj))
#define Nst_ndec_ref(obj) do { \
    if (obj != NULL) { \
        _Nst_dec_ref(OBJ(obj)); \
    }} while ( 0 )
// Call the object's destructor and deallocate its memory
#define Nst_obj_destroy(obj) _Nst_obj_destroy(OBJ(obj))
// Create a new object of size `size` on the heap
#define Nst_obj_alloc(type, type_obj, destructor) \
    (type *)_Nst_obj_alloc( \
        sizeof(type), \
        (struct _Nst_StrObj *)type_obj, \
        (Nst_ObjDestructor)(destructor))

#define Nst_FLAG_SET(obj, flag) ((obj)->flags |= flag)
#define Nst_FLAG_DEL(obj, flag) ((obj)->flags &= ~(flag))
#define Nst_FLAG_HAS(obj, flag) ((obj)->flags & (flag))

#if !defined(_DEBUG) && defined(Nst_TRACK_OBJ_INIT_POS)
#undef Nst_TRACK_OBJ_INIT_POS
#endif

#ifdef Nst_TRACK_OBJ_INIT_POS

#define Nst_OBJ_HEAD \
    i32 ref_count; \
    struct _Nst_StrObj *type; \
    void (*destructor)(void *); \
    i32 hash; \
    u8 flags; \
    i32 init_line; \
    i32 init_col; \
    i8 *init_path

#else
// only the lower 4 bits of 'flags' can be used
// the top 4 are reserved for the garbage collector
#define Nst_OBJ_HEAD \
    i32 ref_count; \
    struct _Nst_StrObj *type; \
    void (*destructor)(void *); \
    i32 hash; \
    u8 flags
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

struct _Nst_StrObj;

NstEXP typedef struct _Nst_Obj
{
    Nst_OBJ_HEAD;
}
Nst_Obj;

NstEXP typedef void (*Nst_ObjDestructor)(void *);

NstEXP typedef Nst_Obj Nst_NullObj;

NstEXP Nst_Obj *NstC _Nst_obj_alloc(usize size, struct _Nst_StrObj *type,
                                    void (*destructor)(void *));
NstEXP void NstC _Nst_obj_destroy(Nst_Obj *obj);

NstEXP Nst_Obj *NstC _Nst_inc_ref(Nst_Obj *obj);
NstEXP void NstC _Nst_dec_ref(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_H
