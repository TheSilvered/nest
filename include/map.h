/**
 * @file map.h
 *
 * @brief `Map` object interface
 *
 * @author TheSilvered
 */

#ifndef MAP_H
#define MAP_H

#include "error.h"
#include "ggc.h"

/* The minimum size of a map, must be a power of two. */
#define _Nst_MAP_MIN_SIZE 32

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Creates a new map object.
 *
 * @return The new object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_map_new(void);
/**
 * Creates a shallow copy of a map object.
 *
 * @param map: the map to copy
 *
 * @return The copied map or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_map_copy(Nst_Obj *map);

void _Nst_map_destroy(Nst_Obj *map);

/* The traverse function for the map object. */
NstEXP void NstC _Nst_map_traverse(Nst_Obj *map);

/* Get the number of key-value pairs in a map. */
NstEXP usize NstC Nst_map_len(Nst_Obj *map);
/* Get the current capacity of a map. */
NstEXP usize NstC Nst_map_cap(Nst_Obj *map);

/**
 * Inserts or modifies a value in the map. Adds a reference to both the key and
 * the value.
 *
 * @param map: the map to update
 * @param key: the key to insert or modify
 * @param value: the value to associate with the key
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_map_set(Nst_Obj *map, Nst_Obj *key, Nst_Obj *value);
/**
 * Gets the value associated with a key.
 *
 * @param map: the map to get the value from
 * @param key: the key to get
 *
 * @return The object associated with the key on success or `NULL` if the key
 * is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC Nst_map_get(Nst_Obj *map, Nst_Obj *key);
/**
 * Drops a key from a map and returns its value.
 *
 * @param map: the map to drop the key from
 * @param key: the key to drop
 *
 * @return The object associated with the removed key on success or `NULL` if
 * the key is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC Nst_map_drop(Nst_Obj *map, Nst_Obj *key);

/**
 * Inserts or modifies a value in the map.
 *
 * @param map: the map to update
 * @param key: the key to insert or modify as a C string
 * @param value: the value to associate with the key
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_map_set_str(Nst_Obj *map, const char *key, Nst_Obj *value);
/**
 * Gets the value associated with a key.
 *
 * @param map: the map to get the value from
 * @param key: the key to get as a C string
 *
 * @return The object associated with the key on success or `NULL` if the key
 * is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC Nst_map_get_str(Nst_Obj *map, const char *key);
/**
 * Drops a key from a map and returns its value.
 *
 * @param map: the map to drop the key from
 * @param key: the key to drop as a C string
 *
 * @return The object associated with the removed key on success or `NULL` if
 * the key is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC Nst_map_drop_str(Nst_Obj *map, const char *key);

/**
 * Get the next key-value pair in the map given an index.
 *
 * @brief To get the first item pass `-1` to `idx` and to continue looping pass
 * the previously returned value as `idx`. The function returns `-1` when there
 * are no more items.
 *
 * @param idx: the previous returned index or `-1` for the first item
 * @param map: the map to iterate over
 * @param out_key: pointer set to the key, can be `NULL`, no reference is added
 * to the key
 * @param out_val: pointer set to the value, can be `NULL`, no reference is
 * added to the value
 *
 * @return The index of the current pair or `-1` if the map contains no more
 * pairs. If the return value is `-1`, `out_key` and `out_val` are set to
 * `NULL`.
 */
NstEXP isize NstC Nst_map_next(isize idx, Nst_Obj *map, Nst_Obj **out_key,
                               Nst_Obj **out_val);
/**
 * Get the previous key-value pair in the map given an index.
 *
 * @brief To get the last item pass `-1` to `idx` and to continue looping pass
 * the previously returned value as `idx`. The function returns `-1` when there
 * are no more items.
 *
 * @param idx: the previous returned index or `-1` for the first item
 * @param map: the map to iterate over
 * @param out_key: pointer set to the key, can be `NULL`, no reference is added
 * to the key
 * @param out_val: pointer set to the value, can be `NULL`, no reference is
 * added to the value
 *
 * @return The index of the current pair or `-1` if the map contains no more
 * pairs. If the return value is `-1`, `out_key` and `out_val` are set to
 * `NULL`.
 */
NstEXP isize NstC Nst_map_prev(isize idx, Nst_Obj *map, Nst_Obj **out_key,
                               Nst_Obj **out_val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MAP_H
