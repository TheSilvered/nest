#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include "map.h"
#include "hash.h"
#include "obj_ops.h"
#include "nst_types.h"

static void set_clean(Nst_map *map, int32_t hash, Nst_Obj *key, Nst_Obj *value)
{
    if ( key == NULL )
        return;

    register size_t mask = map->mask;
    register MapNode *nodes = map->nodes;
    register int32_t i = hash & mask;
    register MapNode curr_node = nodes[i];

    for ( size_t perturb = (size_t)hash;
        curr_node.key != NULL && curr_node.key != key;
        perturb >>= 5 )
    {
        if ( curr_node.hash == hash && AS_BOOL(obj_eq(key, curr_node.key, NULL)) )
            break;
        i = (int32_t)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];
    }

    (nodes + (i & mask))->hash = hash;
    (nodes + (i & mask))->key = key;
    (nodes + (i & mask))->value = value;
}

void resize_map(Nst_map *map)
{
    size_t old_size = map->size;
    MapNode *old_nodes = map->nodes;
    register size_t size = (map->size *= 2);
    map->mask = size - 1;

    map->nodes = calloc(size, sizeof(MapNode));
    if ( map->nodes == NULL )
    {
        map->nodes = old_nodes;
        destroy_map(map);
        errno = ENOMEM;
        return;
    }

    for ( size_t i = 0; i < old_size; i++ )
    {
        set_clean(
            map,
            old_nodes[i].hash,
            old_nodes[i].key,
            old_nodes[i].value
        );
    }

    free(old_nodes);
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
    register int32_t hash = key->hash;

    if ( hash == -1 )
    {
        hash = hash_obj(key);
        if ( hash == -1 )
            return false;
    }

    register size_t mask = map->mask;
    register MapNode *nodes = map->nodes;
    register size_t i = hash & mask;
    register MapNode curr_node = nodes[i];
    register size_t map_size = map->size;

    for ( size_t perturb = (size_t)hash;
          curr_node.key != NULL && curr_node.key != key;
          perturb >>= 5 )
    {
        if ( curr_node.hash == hash && AS_BOOL(obj_eq(key, curr_node.key, NULL)) )
            break;
        i = (int32_t)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];
    }

    inc_ref(key);
    inc_ref(value);

    if ( curr_node.key != NULL )
    {
        dec_ref(curr_node.key);
        dec_ref(curr_node.value);
    }
    else
        map->item_count++;

    (nodes + (i & mask))->hash = hash;
    (nodes + (i & mask))->key = key;
    (nodes + (i & mask))->value = value;

    if ( map_size - map->item_count < map_size >> 2 )
        resize_map(map);

    return true;
}

Nst_Obj *map_get(Nst_map *map, Nst_Obj *key)
{
    register int32_t hash = key->hash;

    if ( hash == -1 )
    {
        hash = hash_obj(key);
        if ( hash == -1 )
            return NULL;
    }

    register size_t mask = map->mask;
    register MapNode *nodes = map->nodes;
    register int32_t i = hash & mask;
    register MapNode curr_node = nodes[i];

    if ( curr_node.key != NULL &&
        (curr_node.key == key || AS_BOOL(obj_eq(key, curr_node.key, NULL))) )
    {
        inc_ref(curr_node.value);
        return curr_node.value;
    }

    for ( size_t perturb = (size_t)hash; ; perturb >>= 5 )
    {
        i = (int32_t)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];

        if ( curr_node.key == NULL )
            return NULL;

        if ( curr_node.hash == hash && 
           ( curr_node.key == key || AS_BOOL(obj_eq(key, curr_node.key, NULL)) ) )
        {
            inc_ref(curr_node.value);
            return curr_node.value;
        }
    }
}

void map_set_str(Nst_map *map, const char *key, Nst_Obj *value)
{
    Nst_Obj *key_obj = new_str_obj(new_string_raw(key, false));
    map_set(map, key_obj, value);
    dec_ref(key_obj);
}

Nst_Obj *map_get_str(Nst_map *map, const char *key)
{
    Nst_Obj *key_obj = new_str_obj(new_string_raw(key, false));
    Nst_Obj *value = map_get(map, key_obj);
    dec_ref(key_obj);
    return value;
}

void destroy_map(Nst_map *map)
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