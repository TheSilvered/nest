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

#define nst_map_set(map, key, value) _nst_map_set(MAP(map), (Nst_Obj *)key, (Nst_Obj *)value)
#define nst_map_get(map, key) _nst_map_get(MAP(map), (Nst_Obj *)key)
#define nst_map_drop(map, key) _nst_map_drop(MAP(map), (Nst_Obj *)key)

#define nst_map_get_next_idx(curr_idx, map) _nst_map_get_next_idx(curr_idx, MAP(map))

#define nst_map_set_str(map, key, value) _nst_map_set_str(MAP(map), key, (Nst_Obj *)value)
#define nst_map_get_str(map, key) _nst_map_get_str(MAP(map), key)
#define nst_map_drop_str(map, key) _nst_map_drop_str(MAP(map), key)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_MapNode
{
    int32_t hash;
    Nst_Obj *key;
    Nst_Obj *value;
    int next_idx;
    int prev_idx;
}
Nst_MapNode;

typedef struct _Nst_MapObj
{
    NST_OBJ_HEAD;
    NST_GGC_SUPPORT;
    size_t size;
    size_t item_count;
    size_t mask;
    Nst_MapNode *nodes;
    int head_idx;
    int tail_idx;
}
Nst_MapObj;

// Creates a new empty map
Nst_Obj *nst_new_map();
// Sets `key` and `value` in `map`, if the key is not hashable retunrs false
bool _nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value);
// Gets the value at `key`, returns NULL if the key is unhashable or
// the object does not exist
Nst_Obj *_nst_map_get(Nst_MapObj *map, Nst_Obj *key);
// Drops a key value pair from the map, returns NULL if the key is unhashable
// nst_c.b_true if an object was removed or nst_c.b_false if there was no key to remove
Nst_Obj *_nst_map_drop(Nst_MapObj *map, Nst_Obj *key);

void nst_destroy_map(Nst_MapObj *map);
void nst_traverse_map(Nst_MapObj *map);
void nst_track_map(Nst_MapObj *map);
// Gets the next index when iterating over a map's elements,
// when curr_idx is -1 the first index is returned
Nst_Int _nst_map_get_next_idx(Nst_Int curr_idx, Nst_MapObj *map);

// Sets a value in the map with the key that is a string
void _nst_map_set_str(Nst_MapObj *map, const char *key, Nst_Obj *value);
// Gets a value in the map with the key that is a string
Nst_Obj *_nst_map_get_str(Nst_MapObj *map, const char *key);
// Drops a value in the map with the key that is a string
Nst_Obj *_nst_map_drop_str(Nst_MapObj *map, const char *key);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MAP_H
