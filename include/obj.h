/**
 * @file obj.h
 *
 * @brief Nest Object interface
 *
 * @author TheSilvered
 */

#ifndef OBJ_H
#define OBJ_H

#include "typedefs.h"

#ifdef Nst_DBG_DISABLE_POOLS
/* Maximum size for an object pool. */
#define _Nst_P_LEN_MAX 0
#else
/* [docs:ignore] Maximum size for an object pool. */
#define _Nst_P_LEN_MAX 256
#endif

/* Cast `obj` to `Nst_Obj *`. */
#define NstOBJ(obj) ((Nst_Obj *)(obj))

/**
 * @brief Wrapper for `_Nst_obj_alloc`. `type` is used to get the size of the
 * object to allocate and to cast the result into the correct pointer type.
 */
#define Nst_obj_alloc(type, type_obj)                                         \
    (type *)_Nst_obj_alloc(sizeof(type), (type_obj))

/* Sets `flag` of `obj` to `true`. */
#define Nst_SET_FLAG(obj, flag) ((obj)->flags |= (flag))
/* Sets `flag` of `obj` to `false`. */
#define Nst_DEL_FLAG(obj, flag) ((obj)->flags &= ~(flag))
/* Check if `flag` is set. */
#define Nst_HAS_FLAG(obj, flag) ((obj)->flags & (flag))
/* Create a flag from an id. `n` can be between 1 and 28 included.  */
#define Nst_FLAG(n) (1 << ((n) - 1))
/* Clear all flags from an object, except for the reserved ones. */
#define Nst_CLEAR_FLAGS(obj) ((obj)->flags &= 0xff000000)

/**
 * The type of an object destructor.
 *
 * @brief This function, in an object's type, is called when the object is
 * deleted and should free any memory associated with it apart from the
 * object's own memory, which is handled by Nest. This function should also
 * remove any references that the object being deleted has with other objects.
 */
NstEXP typedef void (*Nst_ObjDstr)(Nst_Obj *);
/**
 * The type of an object traverse function for the garbage collector.
 *
 * @brief This function is called during a garbage collection and should call
 * the function `Nst_ggc_obj_reachable` with any object that it directly
 * references. Any indirect references, such as objects within objects, should
 * be left untouched.
 */
NstEXP typedef void (*Nst_ObjTrav)(Nst_Obj *);

#ifdef Nst_DBG_TRACK_OBJ_INIT_POS

/** [docs:ignore]
 * The macro used to make a struct an object.
 *
 * @brief It must be placed before any other arguments in the struct. Custom
 * flags cannot occupy the four most significant bits of the flags field
 * because they are reserved for the garbage collector.
 */
#define Nst_OBJ_HEAD                                                          \
    Nst_ObjRef *type;                                                         \
    Nst_Obj *p_next;                                                          \
    isize ref_count;                                                          \
    i32 hash;                                                                 \
    u32 flags;                                                                \
    i32 init_line;                                                            \
    i32 init_col;                                                             \
    char *init_path

#else

/**
 * The macro used to make a struct an object.
 *
 * @brief It must be placed before any other arguments in the struct. Custom
 * flags cannot occupy the four most significant bits of the flags field
 * because they are reserved for the garbage collector.
 */
#define Nst_OBJ_HEAD                                                          \
    Nst_ObjRef *type;                                                         \
    Nst_Obj *p_next;                                                          \
    isize ref_count;                                                          \
    i32 hash;                                                                 \
    u32 flags
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Allocates an object on the heap and initializes the fields in
 * `Nst_OBJ_HEAD`.
 *
 * @param size: the size in bytes of the memory to allocate
 * @param type: the type of the object, if it is `NULL`, the object itself is
 * used as the type
 *
 * @return The newly allocate object or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC _Nst_obj_alloc(usize size, Nst_Obj *type);

/**
 * Traverse an object for the GGC. If the object's type does not have a
 * traverse function, this function does nothing.
 */
NstEXP void NstC Nst_obj_traverse(Nst_Obj *obj);

void _Nst_obj_destroy(Nst_Obj *obj);
void _Nst_obj_free(Nst_Obj *obj);

/* Increase the reference count of an object. Returns `obj`. */
NstEXP Nst_ObjRef *NstC Nst_inc_ref(Nst_Obj *obj);
/* Call `Nst_inc_ref` if `obj` is not a `NULL` pointer. Returns `obj`. */
NstEXP Nst_ObjRef *NstC Nst_ninc_ref(Nst_Obj *obj);
/* Decrease the reference count of an object. */
NstEXP void NstC Nst_dec_ref(Nst_ObjRef *obj);
/* Call `Nst_dec_ref` if `obj` is not a `NULL` pointer. */
NstEXP void NstC Nst_ndec_ref(Nst_ObjRef *obj);

/* Flags of a Nest object. */
NstEXP typedef enum _Nst_ObjFlags {
    Nst_FLAG_OBJ_DESTROYED = Nst_FLAG(29)
} Nst_ObjFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_H
