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

/* Hashes a Nest object setting its hash field and returning the hash. */
NstEXP i32 NstC Nst_obj_hash(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !HASH_H
