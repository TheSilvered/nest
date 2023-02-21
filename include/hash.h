/* Object hashing */

#ifndef HASH_H
#define HASH_H

#include "typedefs.h"
#include "obj.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Returns the hash of `obj` or -1 in case of faiulre
i32 nst_obj_hash(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !HASH_H