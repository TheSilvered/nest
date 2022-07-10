#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>
#include "obj.h"
#include "simple_types.h"

#define MAP_MIN_SIZE 32
#define AS_MAP(ptr) ((Nst_map *)(ptr->value))
#define AS_MAP_V(ptr) ((Nst_map *)(ptr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    int32_t hash;
    Nst_Obj *key;
    Nst_Obj *value;
}
MapNode;

typedef struct
{
    size_t size;
    size_t item_count;
    size_t mask;
    MapNode *nodes;
}
Nst_map;

Nst_Obj *new_map_obj(Nst_map *map);
Nst_map *new_map();
bool map_set(Nst_map *map, Nst_Obj *key, Nst_Obj *value);
Nst_Obj *map_get(Nst_map *map, Nst_Obj *key);
Nst_Obj *map_drop(Nst_map *map, Nst_Obj *key);
void map_set_str(Nst_map *map, const char *key, Nst_Obj *value);
Nst_Obj *map_get_str(Nst_map *map, const char *key);
Nst_Obj *map_drop_str(Nst_map *map, const char *key);
void destroy_map(Nst_map *map);
Nst_int get_next_idx(Nst_int curr_idx, Nst_map *map);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MAP_H
