/* Nst_MapObj interface */

#ifndef MAP_H
#define MAP_H

#include "error.h"
#include "ggc.h"

// Must be a power of 2
#define _Nst_MAP_MIN_SIZE 32
#define MAP(ptr) ((Nst_MapObj *)(ptr))

// Sets `key` and `value` in `map`, if the key is not hashable retunrs false
#define Nst_map_set(map, key, value) _Nst_map_set(MAP(map), OBJ(key), OBJ(value))
// Gets the value at `key`, returns NULL if the key is unhashable or
// the object does not exist
#define Nst_map_get(map, key) _Nst_map_get(MAP(map), OBJ(key))
// Drops a key value pair from the map, returns NULL if the key is unhashable
// nst_c.b_true if an object was removed or nst_c.b_false if there was no key
// to remove
#define Nst_map_drop(map, key) _Nst_map_drop(MAP(map), OBJ(key))

// Gets the next index when iterating over a map's elements,
// when curr_idx is -1 the first index is returned
#define Nst_map_get_next_idx(curr_idx, map) \
    _Nst_map_get_next_idx(curr_idx, MAP(map))

// Gets the previous index when iterating over a map's elements,
// when curr_idx is -1 the last index is returned
#define Nst_map_get_prev_idx(curr_idx, map) \
    _Nst_map_get_prev_idx(curr_idx, MAP(map))

// Sets a value in the map with the key that is a string
#define Nst_map_set_str(map, key, value) \
    _Nst_map_set_str(MAP(map), key, OBJ(value))
// Gets a value in the map with the key that is a string
#define Nst_map_get_str(map, key) _Nst_map_get_str(MAP(map), key)
// Drops a value in the map with the key that is a string
#define Nst_map_drop_str(map, key) _Nst_map_drop_str(MAP(map), key)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef struct _Nst_MapNode
{
    i32 hash;
    Nst_Obj *key;
    Nst_Obj *value;
    i32 next_idx;
    i32 prev_idx;
}
Nst_MapNode;

NstEXP typedef struct _Nst_MapObj
{
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    usize size;
    usize item_count;
    usize mask;
    Nst_MapNode *nodes;
    i32 head_idx;
    i32 tail_idx;
}
Nst_MapObj;

// Creates a new empty map
NstEXP Nst_Obj *NstC Nst_map_new();
NstEXP bool NstC _Nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value);
NstEXP Nst_Obj *NstC _Nst_map_get(Nst_MapObj *map, Nst_Obj *key);
NstEXP Nst_Obj *NstC _Nst_map_drop(Nst_MapObj *map, Nst_Obj *key);

NstEXP void NstC _Nst_map_destroy(Nst_MapObj *map);
NstEXP void NstC _Nst_map_traverse(Nst_MapObj *map);
NstEXP void NstC _Nst_map_track(Nst_MapObj *map);

NstEXP i32 NstC _Nst_map_get_next_idx(i32 curr_idx, Nst_MapObj *map);
NstEXP i32 NstC _Nst_map_get_prev_idx(i32 curr_idx, Nst_MapObj *map);
// Resizes the node array if necessary
// `force_item_reset` forces all the items in the map to be re-inserted
NstEXP bool NstC _Nst_map_resize(Nst_MapObj *map, bool force_item_reset);

NstEXP bool NstC _Nst_map_set_str(Nst_MapObj *map, const i8 *key,
                                  Nst_Obj *value);
NstEXP Nst_Obj *NstC _Nst_map_get_str(Nst_MapObj *map, const i8 *key);
NstEXP Nst_Obj *NstC _Nst_map_drop_str(Nst_MapObj *map, const i8 *key);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MAP_H
