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

/* Maximum size for an object pool. */
#define _Nst_P_LEN_MAX 20

/* Casts `obj` to `Nst_Obj *`. */
#define OBJ(obj) ((Nst_Obj *)(obj))

/* Alias for `_Nst_inc_ref` that casts `obj` to `Nst_Obj *`. */
#define Nst_inc_ref(obj) _Nst_inc_ref(OBJ(obj))
/* Calls `Nst_inc_ref` if `obj` is not a `NULL` pointer. */
#define Nst_ninc_ref(obj) (obj == NULL ? NULL : _Nst_inc_ref(OBJ(obj)))
/* Alias for `_Nst_dec_ref` that casts `obj` to `Nst_Obj *`. */
#define Nst_dec_ref(obj) _Nst_dec_ref(OBJ(obj))
/* Calls `Nst_dec_ref` if the object is not a `NULL` pointer. */
#define Nst_ndec_ref(obj) do {                                                \
    if (obj != NULL)                                                          \
        _Nst_dec_ref(OBJ(obj));                                               \
    } while (0)
/* Alias for `_Nst_obj_destroy` that casts obj to `Nst_Obj *`. */
#define Nst_obj_destroy(obj) _Nst_obj_destroy(OBJ(obj))
/**
 * @brief Wrapper for `_Nst_obj_alloc`. `type` is used to get the size of the
 * object to allocate and to cast the result into the correct pointer type.
 */
#define Nst_obj_alloc(type, type_obj)                                         \
    (type *)_Nst_obj_alloc(                                                   \
        sizeof(type),                                                         \
        (struct _Nst_TypeObj *)(type_obj))

/* Sets `flag` of `obj` to `true`. */
#define Nst_SET_FLAG(obj, flag) ((obj)->flags |= (flag))
/* Sets `flag` of `obj` to `false`. */
#define Nst_DEL_FLAG(obj, flag) ((obj)->flags &= ~(flag))
/* Checks if `flag` is set. */
#define Nst_HAS_FLAG(obj, flag) ((obj)->flags & (flag))
/* Creates a flag from an id. `n` can be between 1 and 28 included.  */
#define Nst_FLAG(n) (1 << ((n) - 1))
/* Clears all flags from an object, except for the reserved ones. */
#define Nst_CLEAR_FLAGS(obj) ((obj)->flags &= 0xff00000000000000)

struct _Nst_StrObj;
struct _Nst_TypeObj;

/* The type of an object destructor. */
NstEXP typedef void (*Nst_ObjDstr)(void *);
/* The type of an object traverse function for the garbage collector. */
NstEXP typedef void (*Nst_ObjTrav)(void *);

#ifdef Nst_TRACK_OBJ_INIT_POS

/** [docs:ignore]
 * The macro used to make a struct an object.
 *
 * @brief It must be placed before any other arguments in the struct. Custom
 * flags cannot occupy the four most significant bits of the flags field
 * because they are reserved for the garbage collector.
 */
#define Nst_OBJ_HEAD                                                          \
    struct _Nst_TypeObj *type;                                                \
    struct _Nst_Obj *p_next;                                                  \
    i32 ref_count;                                                            \
    i32 hash;                                                                 \
    u32 flags;                                                                \
    i32 init_line;                                                            \
    i32 init_col;                                                             \
    i8 *init_path

#else

/**
 * The macro used to make a struct an object.
 *
 * @brief It must be placed before any other arguments in the struct. Custom
 * flags cannot occupy the four most significant bits of the flags field
 * because they are reserved for the garbage collector.
 */
#define Nst_OBJ_HEAD                                                          \
    struct _Nst_TypeObj *type;                                                \
    struct _Nst_Obj *p_next;                                                  \
    i32 ref_count;                                                            \
    i32 hash;                                                                 \
    u32 flags
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* [docs:ignore_sym Nst_TRACK_OBJ_INIT_POS] */

/**
 * The structure representing a basic Nest object.
 *
 * @param ref_count: the reference count of the object
 * @param type: the type of the object
 * @param p_next: the next object in the type's pool
 * @param hash: the hash of the object, `-1` if it has not yet been hashed or
 * is not hashable
 * @param flags: the flags of the object
 * @param init_line: **THIS FIELD ONLY EXISTS WHEN `Nst_TRACK_OBJ_INIT_POS` IS
 * DEFINED** the line of the instruction that initialized the object
 * @param init_col: **THIS FIELD ONLY EXISTS WHEN `Nst_TRACK_OBJ_INIT_POS` IS
 * DEFINED** the column of the instruction that initialized the object
 * @param init_path: **THIS FIELD ONLY EXISTS WHEN `Nst_TRACK_OBJ_INIT_POS` IS
 * DEFINED** the path to the file where the object was initialized
 */
NstEXP typedef struct _Nst_Obj {
    Nst_OBJ_HEAD;
} Nst_Obj;

/**
 * @brief A `Nst_NullObj` is just a `Nst_Obj` as it does not have any special
 * fields.
 */
NstEXP typedef Nst_Obj Nst_NullObj;

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
NstEXP Nst_Obj *NstC _Nst_obj_alloc(usize size, struct _Nst_TypeObj *type);
/**
 * Calls an object's destructor.
 *
 * @brief This function should not be called on most occasions, use
 * `Nst_dec_ref` instead.
 */
NstEXP void NstC _Nst_obj_destroy(Nst_Obj *obj);

/**
 * Frees the memory of the object or adds it to the object pool.
 *
 * @param obj: the pointer to the object to free
 */
NstEXP void NstC _Nst_obj_free(Nst_Obj *obj);

/* Increases the reference count of an object. */
NstEXP Nst_Obj *NstC _Nst_inc_ref(Nst_Obj *obj);
/**
 * @brief Decreases the reference count of an object and calls
 * `_Nst_obj_destroy` if it reaches zero.
 */
NstEXP void NstC _Nst_dec_ref(Nst_Obj *obj);

/* Flags of a Nest object. */
NstEXP typedef enum _Nst_ObjFlags {
    Nst_FLAG_OBJ_DESTROYED = Nst_FLAG(29)
} Nst_ObjFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_H
