#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include "map.h"
#include "hash.h"
#include "obj_ops.h"

void set_clean(Nst_map *map, size_t hash, Nst_Obj *key, Nst_Obj *value)
{
    register size_t i = hash & map->mask;
    register MapNode *node = &map->nodes[i];
    i = ((5 * i) + 1) % map->size;

    for ( ; node->key != NULL; i = ((5 * i) + 1) % map->size )
    {
        node = &map->nodes[i];
    }

    node->hash = hash;
    node->key = key;
    node->value = value;
}

void resize_map(Nst_map *map, size_t new_size)
{
    size_t old_size = map->size;
    MapNode *old_nodes = map->nodes;

    map->size = new_size;

    map->nodes = calloc(new_size, sizeof(MapNode));
    if ( map->nodes == NULL )
    {
        map->nodes = old_nodes;
        map_destroy(map);
        errno = ENOMEM;
        return;
    }

    map->mask = new_size - 1;

    for ( size_t i = 0; i < old_size; i++ )
    {
        set_clean(
            map,
            old_nodes[i].hash,
            old_nodes[i].key,
            old_nodes[i].value
        );
    }
}

Nst_map *new_map()
{
    Nst_map *map = malloc(sizeof(Nst_map));
    if ( map == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    map->item_count = 0;
    map->nodes = calloc(MAP_MIN_SIZE, sizeof(MapNode));
    map->mask = MAP_MIN_SIZE - 1;
    map->size = MAP_MIN_SIZE;

    if ( map->nodes == NULL )
    {
        free(map);
        return NULL;
    }

    return map;
}

bool map_set(Nst_map *map, Nst_Obj *key, Nst_Obj *value)
{
    register size_t i = hash_obj(key);
    if ( i == -1 )
        return false;
    i &= map->mask;
    register MapNode *node = &map->nodes[i];
    i = ((5 * i) + 1) % map->size;

    for ( ; node->key != NULL && !obj_eq(node->key, key, NULL); i = ((5 * i) + 1) % map->size )
    {
        node = &map->nodes[i];
    }

    inc_ref(key);
    inc_ref(value);

    // decrease after increasing to prevent deletion if they are the same object
    if ( node->key != NULL )
        dec_ref(node->key);
    else
        map->item_count++;

    if ( node->value != NULL )
        dec_ref(node->value);

    node->key = key;
    node->value = value;

    if ( (float)map->item_count / map->size > 0.6f )
        resize_map(map, map->size * 2);

    return true;
}

Nst_Obj *map_get(Nst_map *map, Nst_Obj *key)
{
    register size_t i = hash_obj(key);
    if ( i == -1 )
        return NULL;
    i &= map->mask;

    register MapNode *node = &map->nodes[i];
    i = ((5*i) + 1) % map->size;

    for ( ; node->key != NULL && !obj_eq(node->key, key, NULL); i = ((5*i) + 1) % map->size )
    {
        node = &map->nodes[i];
    }

    if ( node->value != NULL )
        inc_ref(node->value);

    return node->value;
}

void map_destroy(Nst_map *map)
{
    for ( size_t i = 0; i < map->size; i++ )
    {
        if ( map->nodes[i].key != NULL )
            dec_ref(map->nodes[i].key);
        if ( map->nodes[i].value != NULL )
            dec_ref(map->nodes[i].value);
    }

    free(map->nodes);
    free(map);
}
