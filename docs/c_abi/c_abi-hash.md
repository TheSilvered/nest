# `hash.h`

This header contains the general hashing function used by Nest in maps.

## Functions

### `nst_obj_hash`

**Synopsis**:

```better-c
i32 nst_obj_hash(Nst_Obj *obj)
```

**Return value**:

This function returns the hash of `obj` or `-1` if it cannot be hashed.
