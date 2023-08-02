/**
 * @file map.h
 *
 * @brief Nst_MapObj interface.
 *
 * @author TheSilvered
 */

#ifndef MAP_H
#define MAP_H

#include "error.h"
#include "ggc.h"

/* The minimum size of a map, must be a power of two. */
#define _Nst_MAP_MIN_SIZE 32
/* Casts ptr to Nst_MapObj *. */
#define MAP(ptr) ((Nst_MapObj *)(ptr))

/**
 * @brief Alias for _Nst_map_set which casts map to Nst_MapObj *, and key and
 * value to Nst_Obj *.
 */
#define Nst_map_set(map, key, value)                                          \
    _Nst_map_set(MAP(map), OBJ(key), OBJ(value))
/**
 * @brief Alias for _Nst_map_get that casts map to Nst_MapObj * and key to
 * Nst_Obj *.
*/
#define Nst_map_get(map, key) _Nst_map_get(MAP(map), OBJ(key))
/**
 * @brief Alias for _Nst_map_drop that casts map to Nst_MapObj * and key to
 * Nst_Obj *.
 */
#define Nst_map_drop(map, key) _Nst_map_drop(MAP(map), OBJ(key))

/* Alias for _Nst_map_get_next_idx that casts map to Nst_MapObj *. */
#define Nst_map_get_next_idx(curr_idx, map) \
    _Nst_map_get_next_idx(curr_idx, MAP(map))
/* Alias for _Nst_map_get_prev_idx that casts map to Nst_MapObj *. */
#define Nst_map_get_prev_idx(curr_idx, map) \
    _Nst_map_get_prev_idx(curr_idx, MAP(map))

// Sets a value in the map with the key that is a string
/**
 * @brief Alias for _Nst_map_set_str that casts map to Nst_MapObj * and value
 * to Nst_Obj *.
 */
#define Nst_map_set_str(map, key, value) \
    _Nst_map_set_str(MAP(map), key, OBJ(value))
/* Alias for Nst_map_get_str that casts map to Nst_MapObj *. */
#define Nst_map_get_str(map, key) _Nst_map_get_str(MAP(map), key)
/* Alias for Nst_map_drop_str that casts map to Nst_MapObj *. */
#define Nst_map_drop_str(map, key) _Nst_map_drop_str(MAP(map), key)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * The structure representing a node of a Nst_MapObj.
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
 * @param size: the current size of the nodes array
 * @param item_count: the number of nodes inside the map
 * @param mask: the mask applied to the hash when inserting new nodes
 * @param nodes: the array of nodes of the map
 * @param head_idx: the first node in the map
 * @param tail_idx: the last node in the map
 */
NstEXP typedef struct _Nst_MapObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    usize size;
    usize item_count;
    usize mask;
    Nst_MapNode *nodes;
    i32 head_idx;
    i32 tail_idx;
} Nst_MapObj;

/**
 * Creates a new map object.
 *
 * @return The new object or NULL on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_map_new(void);
/**
 * Inserts or modifies a value in the map.
 *
 * @param map: the map to update
 * @param key: the key to insert or modify
 * @param value: the value to associate with the key
 *
 * @return true on success and false on failure. The error is set.
 */
NstEXP bool NstC _Nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value);
/**
 * Gets the value associated with a key.
 *
 * @param map: the map to get the value from
 * @param key: the key to get
 *
 * @return The object associated with the key on success and NULL if the key
 * is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC _Nst_map_get(Nst_MapObj *map, Nst_Obj *key);
/**
 * Drops a key from a map and returns its value.
 *
 * @param map: the map to drop the key from
 * @param key: the key to drop
 *
 * @return The object associated with the removed key on success and NULL if
 * the key is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC _Nst_map_drop(Nst_MapObj *map, Nst_Obj *key);

/* The destructor for the map object. */
NstEXP void NstC _Nst_map_destroy(Nst_MapObj *map);
/* The traverse function for the map object. */
NstEXP void NstC _Nst_map_traverse(Nst_MapObj *map);
/* The track function for the map object. */
NstEXP void NstC _Nst_map_track(Nst_MapObj *map);

/**
 * Gets the next index in a map given the current one.
 *
 * @brief If curr_idx is -1, the first index is returned.
 *
 * @param curr_idx: the current index
 * @param map: the map to get the index from
 *
 * @return The next index or -1 if the given index is the last one.
 */
NstEXP i32 NstC _Nst_map_get_next_idx(i32 curr_idx, Nst_MapObj *map);
/**
 * Gets the next index in a map given the current one.
 *
 * @brief If curr_idx is -1, the first index is returned.
 *
 * @param curr_idx: the current index
 * @param map: the map to get the index from
 *
 * @return The next index or -1 if the given index is the last one.
 */
NstEXP i32 NstC _Nst_map_get_prev_idx(i32 curr_idx, Nst_MapObj *map);

/**
 * Resizes the node array if necessary.
 *
 * @param map: the map to resize
 * @param force_item_reset: whether to force the nodes inside the map to be
 * re-inserted
 *
 * @return true on success and false on failure. The error is set. When
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
 * @return true on success and false on failure. The error is set.
 */
NstEXP bool NstC _Nst_map_set_str(Nst_MapObj *map, const i8 *key,
                                  Nst_Obj *value);
/**
 * Gets the value associated with a key.
 *
 * @param map: the map to get the value from
 * @param key: the key to get as a C string
 *
 * @return The object associated with the key on success and NULL if the key
 * is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC _Nst_map_get_str(Nst_MapObj *map, const i8 *key);
/**
 * Drops a key from a map and returns its value.
 *
 * @param map: the map to drop the key from
 * @param key: the key to drop as a C string
 *
 * @return The object associated with the removed key on success and NULL if
 * the key is not hashable or is not inside the map. No error is set.
 */
NstEXP Nst_Obj *NstC _Nst_map_drop_str(Nst_MapObj *map, const i8 *key);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MAP_H
