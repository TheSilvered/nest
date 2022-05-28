#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>
#include "obj.h"

#define MAP_MIN_SIZE 32

typedef struct MapNode
{
    size_t hash;
    Nst_Obj *key;
    Nst_Obj *value;
}
MapNode;

typedef struct Nst_map
{
    size_t size;
    size_t item_count;
    size_t mask;
    MapNode *nodes;
}
Nst_map;

Nst_map *new_map();
bool map_set(Nst_map *map, Nst_Obj *key, Nst_Obj *value);
Nst_Obj *map_get(Nst_map *map, Nst_Obj *key);
void map_destroy(Nst_map *map);

#endif // !MAP_H
