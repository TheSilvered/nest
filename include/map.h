/* Nst_MapObj interface */

#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>
#include "str.h"
#include "ggc.h"

// Must be a power of 2
#define MAP_MIN_SIZE 32
#define MAP(ptr) ((Nst_MapObj *)(ptr))

// Sets `key` and `value` in `map`, if the key is not hashable retunrs false
#define nst_map_set(map, key, value) _nst_map_set(MAP(map), OBJ(key), OBJ(value))
// Gets the value at `key`, returns NULL if the key is unhashable or
// the object does not exist
#define nst_map_get(map, key) _nst_map_get(MAP(map), OBJ(key))
// Drops a key value pair from the map, returns NULL if the key is unhashable
// nst_c.b_true if an object was removed or nst_c.b_false if there was no key
// to remove
#define nst_map_drop(map, key) _nst_map_drop(MAP(map), OBJ(key))

// Gets the next index when iterating over a map's elements,
// when curr_idx is -1 the first index is returned
#define nst_map_get_next_idx(curr_idx, map) \
    _nst_map_get_next_idx(curr_idx, MAP(map))

// Sets a value in the map with the key that is a string
#define nst_map_set_str(map, key, value) \
    _nst_map_set_str(MAP(map), key, OBJ(value))
// Gets a value in the map with the key that is a string
#define nst_map_get_str(map, key) _nst_map_get_str(MAP(map), key)
// Drops a value in the map with the key that is a string
#define nst_map_drop_str(map, key) _nst_map_drop_str(MAP(map), key)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_MapNode
{
    i32 hash;
    Nst_Obj *key;
    Nst_Obj *value;
    i32 next_idx;
    i32 prev_idx;
}
Nst_MapNode;

typedef struct _Nst_MapObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
    usize size;
    usize item_count;
    usize mask;
    Nst_MapNode *nodes;
    i32 head_idx;
    i32 tail_idx;
}
Nst_MapObj;

// Creates a new empty map
Nst_Obj *nst_map_new();
bool _nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value);
Nst_Obj *_nst_map_get(Nst_MapObj *map, Nst_Obj *key);
Nst_Obj *_nst_map_drop(Nst_MapObj *map, Nst_Obj *key);

void _nst_map_destroy(Nst_MapObj *map);
void _nst_map_traverse(Nst_MapObj *map);
void _nst_map_track(Nst_MapObj *map);

i32 _nst_map_get_next_idx(i32 curr_idx, Nst_MapObj *map);
// Resizes the node array if necessary
// `force_item_reset` forces all the items in the map to be re-inserted
void _nst_map_resize(Nst_MapObj *map, bool force_item_reset);

void _nst_map_set_str(Nst_MapObj *map, const i8 *key, Nst_Obj *value);
Nst_Obj *_nst_map_get_str(Nst_MapObj *map, const i8 *key);
Nst_Obj *_nst_map_drop_str(Nst_MapObj *map, const i8 *key);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MAP_H
