/**
 * @file map.h
 *
 * @brief `Nst_MapObj` interface
 *
 * @author TheSilvered
 */

#ifndef MAP_H
#define MAP_H

#include "error.h"
#include "ggc.h"

/* The minimum size of a map, must be a power of two. */
#define _Nst_MAP_MIN_SIZE 32
/* Casts `ptr` to `Nst_MapObj *`. */
#define MAP(ptr) ((Nst_MapObj *)(ptr))

/**
 * @brief Alias for `_Nst_map_set` which casts `map` to `Nst_MapObj *`, and
 * `key` and value to `Nst_Obj *`.
 */
#define Nst_map_set(map, key, value)                                          \
    _Nst_map_set(MAP(map), OBJ(key), OBJ(value))
/**
 * @brief Alias for `_Nst_map_get` that casts `map` to `Nst_MapObj *` and `key`
 * to `Nst_Obj *`.
 */
#define Nst_map_get(map, key) _Nst_map_get(MAP(map), OBJ(key))
/**
 * @brief Alias for `_Nst_map_drop` that casts `map` to `Nst_MapObj *` and
 * `key` to `Nst_Obj *`.
 */
#define Nst_map_drop(map, key) _Nst_map_drop(MAP(map), OBJ(key))
/* Alias for `_Nst_map_copy` that casts `map` to `Nst_MapObj *`. */
#define Nst_map_copy(map) _Nst_map_copy(MAP(map))

/**
 * @brief Alias for `_Nst_map_set_str` that casts `map` to `Nst_MapObj *` and
 * `value` to `Nst_Obj *`.
 */
#define Nst_map_set_str(map, key, value) \
    _Nst_map_set_str(MAP(map), key, OBJ(value))
/* Alias for `Nst_map_get_str` that casts `map` to `Nst_MapObj *`. */
#define Nst_map_get_str(map, key) _Nst_map_get_str(MAP(map), key)
/* Alias for `Nst_map_drop_str` that casts map to `Nst_MapObj *`. */
#define Nst_map_drop_str(map, key) _Nst_map_drop_str(MAP(map), key)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * The structure representing a node of a `Nst_MapObj`.
 *
 * @param hash: the hash of the key contained in the node
 * @param key: the key of the node
 * @param value: the value of the node
 * @param next_idx: the index of the next node inside the map
 * @param prev_idx: the index of the previous node inside the map
 */
NstEXP typedef struct _Nst_MapNode {
    i32 hash;
    Nst_Obj *key;
    Nst_Obj *value;
    i32 next_idx;
    i32 prev_idx;
} Nst_MapNode;

/**
 * The structure representing a Nest map object.
 *
 * @param cap: the current capacity of the nodes array
 * @param len: the number of nodes inside the map
 * @param mask: the mask applied to the hash when inserting new nodes
 * @param nodes: the array of nodes of the map
 * @param head_idx: the first node in the map
 * @param tail_idx: the last node in the map
 */
NstEXP typedef struct _Nst_MapObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    usize cap;
    usize len;
    usize mask;
    Nst_MapNode *nodes;
    i32 head_idx;
    i32 tail_idx;
} Nst_MapObj;

/**
 * Creates a new map object.
 *
 * @return The new object or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_map_new(void);
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
NstEXP bool NstC _Nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value);
/**
 * Gets the value associated with a key.
 *
 * @param map: the map to get the value from
 * @param key: the key to get
 *
 * @return The object associated with the key on success or `NULL` if the key
 * is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC _Nst_map_get(Nst_MapObj *map, Nst_Obj *key);
/**
 * Drops a key from a map and returns its value.
 *
 * @param map: the map to drop the key from
 * @param key: the key to drop
 *
 * @return The object associated with the removed key on success or `NULL` if
 * the key is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC _Nst_map_drop(Nst_MapObj *map, Nst_Obj *key);
/**
 * Creates a shallow copy of a map object.
 *
 * @param map: the map to copy
 *
 * @return The copied map or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC _Nst_map_copy(Nst_MapObj *map);

/* The destructor for the map object. */
NstEXP void NstC _Nst_map_destroy(Nst_MapObj *map);
/* The traverse function for the map object. */
NstEXP void NstC _Nst_map_traverse(Nst_MapObj *map);

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
NstEXP isize NstC Nst_map_next(isize idx, Nst_MapObj *map, Nst_Obj **out_key,
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
NstEXP isize NstC Nst_map_prev(isize idx, Nst_MapObj *map, Nst_Obj **out_key,
                               Nst_Obj **out_val);

/**
 * Resizes the node array if necessary.
 *
 * @param map: the map to resize
 * @param force_item_reset: whether to force the nodes inside the map to be
 * re-inserted
 *
 * @return `true` on success and `false` on failure. The error is set. When
 * shrinking the function is guaranteed to succeed.
 */
NstEXP bool NstC _Nst_map_resize(Nst_MapObj *map, bool force_item_reset);

/**
 * Inserts or modifies a value in the map.
 *
 * @param map: the map to update
 * @param key: the key to insert or modify as a C string
 * @param value: the value to associate with the key
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC _Nst_map_set_str(Nst_MapObj *map, const i8 *key,
                                  Nst_Obj *value);
/**
 * Gets the value associated with a key.
 *
 * @param map: the map to get the value from
 * @param key: the key to get as a C string
 *
 * @return The object associated with the key on success or `NULL` if the key
 * is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC _Nst_map_get_str(Nst_MapObj *map, const i8 *key);
/**
 * Drops a key from a map and returns its value.
 *
 * @param map: the map to drop the key from
 * @param key: the key to drop as a C string
 *
 * @return The object associated with the removed key on success or `NULL` if
 * the key is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC _Nst_map_drop_str(Nst_MapObj *map, const i8 *key);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MAP_H
