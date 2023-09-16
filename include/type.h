/**
 * @file type.h
 *
 * @brief Nest Type object.
 *
 * @author TheSilvered
 */

#ifndef TYPE_H
#define TYPE_H

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#include "str.h"

/* Casts `ptr` to `Nst_TypeObj *`. */
#define TYPE(ptr) ((Nst_TypeObj *)(ptr))
/* Casts `ptr` to `Nst_ContTypeObj *`. */
#define CONT_TYPE(ptr) ((Nst_ContTypeObj *)(ptr))

/* Head of a type object, should not be used for custom types. */
#define _Nst_TYPE_HEAD                                                        \
    Nst_Obj *p_head;                                                          \
    usize p_len;                                                              \
    Nst_StrObj name;                                                          \
    Nst_ObjDstr dstr

/**
 * The structure representing a Type object in Nest.
 *
 * @param p_head: the head object in the type's pool
 * @param p_len: the length of the pool
 * @param name: the name of the object as a Nest string
 * @param dstr: the destructor of the type, can be NULL
 */
NstEXP typedef struct _Nst_TypeObj {
    Nst_OBJ_HEAD;
    _Nst_TYPE_HEAD;
} Nst_TypeObj;

/**
 * The structure representing a Type object for containers in Nest.
 *
 * @param p_head: the head object in the type's pool
 * @param p_len: the length of the pool
 * @param name: the name of the object as a Nest string
 * @param dstr: the destructor of the type, can be NULL
 * @param trav: the traverse function of the type
 */
NstEXP typedef struct _Nst_ContTypeObj {
    Nst_OBJ_HEAD;
    _Nst_TYPE_HEAD;
    Nst_ObjTrav trav;
} Nst_ContTypeObj;

/**
 * Creates a new `Nst_TypeObj`.
 *
 * @brief Note: `name` can only contain 7-bit ASCII characters
 *
 * @param name: the name of the type
 * @param dstr: the destructor of the type
 *
 * @return The new object on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_TypeObj *NstC Nst_type_new(const i8 *name, Nst_ObjDstr dstr);
/**
 * Creates a new `Nst_ContTypeObj`.
 *
 * @brief Note: `name` can only contain 7-bit ASCII characters
 *
 * @param name: the name of the type
 * @param dstr: the destructor of the type
 * @param trav: the traverse function of the type
 *
 * @return The new object on success and `NULL` on failure. The error is set.
 */
NstEXP Nst_TypeObj *NstC Nst_cont_type_new(const i8 *name, Nst_ObjDstr dstr,
                                           Nst_ObjTrav trav);

/* Destructor for Nest type objects. */
NstEXP void NstC _Nst_type_destroy(Nst_TypeObj *obj);

NstEXP typedef enum _Nst_TypeFlags {
    Nst_FLAG_TYPE_IS_CONTAINER = Nst_FLAG(1),
    Nst_FLAG_TYPE_DESTROY_NAME = Nst_FLAG(2)
} Nst_TypeFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !TYPE_H
