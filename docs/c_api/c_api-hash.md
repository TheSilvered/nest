# `hash.h`

Object hashing functions.

## Authors

TheSilvered

## Functions

### `Nst_obj_hash`

**Synopsis:**

```better-c
i32 Nst_obj_hash(Nst_Obj *obj)
```

**Description:**

Hashes a Nest object setting its hash field.

If the object is unhashable `-1` is set.

**Parameters:**

- `obj`: the object to be hashed

**Returns:**

The hash of the object or `-1` if the object cannot be hashed. No error is set.

