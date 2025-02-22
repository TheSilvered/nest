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

#include "obj.h"
#include "str_view.h"

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
NstEXP Nst_Obj *NstC Nst_type_new(const i8 *name, Nst_ObjDstr dstr);
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
NstEXP Nst_Obj *NstC Nst_cont_type_new(const i8 *name, Nst_ObjDstr dstr,
                                       Nst_ObjTrav trav);

/* Get the name of the type. */
NstEXP Nst_StrView NstC Nst_type_name(Nst_Obj *type);
/**
 * Get the traverse function of the type. If the type does not represent a
 * container it returns `NULL`.
 */
NstEXP Nst_ObjTrav NstC Nst_type_trav(Nst_Obj *type);

Nst_Obj *_Nst_type_new_no_err(const i8 *name, Nst_ObjDstr dstr);

void NstC _Nst_type_destroy(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !TYPE_H
