/* Object hashing */

#ifndef HASH_H
#define HASH_H

#include "obj.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Returns the hash of `obj` or -1 in case of faiulre
int32_t nst_hash_obj(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !HASH_H