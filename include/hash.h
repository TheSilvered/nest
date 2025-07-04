/**
 * @file hash.h
 *
 * @brief Object hashing functions
 *
 * @author TheSilvered
 */

#ifndef HASH_H
#define HASH_H

#include "obj.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Hash a Nest object setting its hash field.
 *
 * @brief If the object is not hashable `-1` is set.
 *
 * @param obj: the object to be hashed
 *
 * @return The hash of the object or `-1` if the object cannot be hashed. No
 * error is set.
 */
NstEXP i32 NstC Nst_obj_hash(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !HASH_H
