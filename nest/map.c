#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "hash.h"
#include "map.h"
#include "nst_types.h"
#include "obj_ops.h"

Nst_Obj *nst_new_map()
{
    Nst_MapObj *map = AS_MAP(nst_alloc_obj(sizeof(Nst_MapObj), nst_t_map, nst_destroy_map));
    if ( map == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    map->item_count = 0;
    map->nodes = (Nst_MapNode *)calloc(MAP_MIN_SIZE, sizeof(Nst_MapNode));

    if ( map->nodes == NULL )
    {
        free(map);
        return NULL;
    }

    map->mask = MAP_MIN_SIZE - 1;
    map->size = MAP_MIN_SIZE;
    map->head_idx = -1;
    map->tail_idx = -1;

    NST_GGC_SUPPORT_INIT(map, nst_traverse_map);

    return (Nst_Obj *)map;
}

static int32_t set_clean(Nst_MapObj *map, int32_t hash, Nst_Obj *key, Nst_Obj *value, int prev_idx)
{
    assert(key != NULL);

    size_t mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    int32_t i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    // I already know there are no duplicate keys, every new key must end in an empty node
    for ( size_t perturb = (size_t)hash;
          curr_node.key != NULL;
          perturb >>= 5 )
    {
        i = (int32_t)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];
    }

    (nodes + (i & mask))->hash = hash;
    (nodes + (i & mask))->key = key;
    (nodes + (i & mask))->value = value;
    (nodes + (i & mask))->prev_idx = prev_idx;
    (nodes + (i & mask))->next_idx = -1;
    return i & mask;
}

void resize_map(Nst_MapObj *map, bool force_item_reset)
{
    size_t old_size = map->size;
    Nst_MapNode *old_nodes = map->nodes;
    size_t size;
    if ( old_size - map->item_count < old_size >> 2 )
        size = old_size << 1;
    else if ( old_size > MAP_MIN_SIZE && old_size >> 2 >= map->item_count )
        size = old_size >> 1;
    else if ( force_item_reset )
        size = old_size; // resize_map only resets the items
    else
        return;
    map->mask = size - 1;
    map->size = size;
    map->nodes = (Nst_MapNode *)calloc(size, sizeof(Nst_MapNode));
    if ( map->nodes == NULL )
    {
        map->nodes = old_nodes;
        nst_destroy_map(map);
        errno = ENOMEM;
        return;
    }

    int prev_idx = -1;
    int new_idx = 0;

    for ( int i = map->head_idx; i != -1; i = old_nodes[i].next_idx )
    {
        new_idx = set_clean(
            map,
            old_nodes[i].hash,
            old_nodes[i].key,
            old_nodes[i].value,
            prev_idx
        );

        if ( prev_idx != -1 )
            map->nodes[prev_idx].next_idx = new_idx;
        else
            map->head_idx = new_idx;
        prev_idx = new_idx;
    }
    map->tail_idx = prev_idx;

    free(old_nodes);
}

bool _nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value)
{
    int32_t hash = key->hash;

    if ( hash == -1 )
    {
        hash = nst_hash_obj(key);
        if ( hash == -1 )
            return false;
    }

    size_t mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    size_t i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    for ( size_t perturb = (size_t)hash;
          curr_node.key != NULL && curr_node.key != key;
          perturb >>= 5 )
    {
        if ( curr_node.hash == hash && AS_BOOL(nst_obj_eq(key, curr_node.key, NULL)) )
            break;
        i = (int32_t)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];
    }

    nst_inc_ref(key);
    nst_inc_ref(value);

    if ( curr_node.key != NULL )
    {
        nst_dec_ref(curr_node.key);
        nst_dec_ref(curr_node.value);

        // if it's not the last node
        if ( curr_node.next_idx != -1 )
        {
            nodes[curr_node.next_idx].prev_idx = curr_node.prev_idx;

            if ( curr_node.prev_idx != -1 )
                nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
            else
                map->head_idx = curr_node.next_idx;
            (nodes + (i & mask))->next_idx = -1;
        }
    }
    else
    {
        map->item_count++;
        
        // if it's the first node inserted
        if ( map->head_idx == -1 )
        {
            map->head_idx = (int)(i & mask);
            (nodes + (i & mask))->prev_idx = -1;
        }
        else
        {
            nodes[map->tail_idx].next_idx = (int)(i & mask);
            (nodes + (i & mask))->prev_idx = map->tail_idx;
        }
        map->tail_idx = (int)(i & mask);
        (nodes + (i & mask))->next_idx = -1;
    }

    (nodes + (i & mask))->hash = hash;
    (nodes + (i & mask))->key = key;
    (nodes + (i & mask))->value = value;

    if ( NST_HAS_FLAG(value, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        if ( !NST_HAS_FLAG(map, NST_FLAG_MAP_TRACKED) )
        {
            NST_SET_FLAG(map, NST_FLAG_MAP_TRACKED);
            nst_add_tracked_object((Nst_GGCObj *)map);
        }

        nst_add_tracked_object((Nst_GGCObj *)value);
    }

    resize_map(map, false);

    return true;
}

Nst_Obj *_nst_map_get(Nst_MapObj *map, Nst_Obj *key)
{
    int32_t hash = key->hash;

    if ( hash == -1 )
    {
        hash = nst_hash_obj(key);
        if ( hash == -1 )
            return NULL;
    }

    size_t mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    int32_t i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    if ( curr_node.key != NULL &&
        (curr_node.key == key || AS_BOOL(nst_obj_eq(key, curr_node.key, NULL))) )
    {
        nst_inc_ref(curr_node.value);
        return curr_node.value;
    }

    for ( size_t perturb = (size_t)hash; ; perturb >>= 5 )
    {
        i = (int32_t)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];

        if ( curr_node.key == NULL )
            return NULL;

        if ( curr_node.hash == hash && 
           ( curr_node.key == key || AS_BOOL(nst_obj_eq(key, curr_node.key, NULL)) ) )
        {
            nst_inc_ref(curr_node.value);
            return curr_node.value;
        }
    }
}

Nst_Obj *_nst_map_drop(Nst_MapObj *map, Nst_Obj *key)
{
    int32_t hash = key->hash;

    if ( hash == -1 )
    {
        hash = nst_hash_obj(key);
        if ( hash == -1 )
            return NULL;
    }

    size_t mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    int32_t i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    if ( curr_node.key != NULL &&
        (curr_node.key == key || AS_BOOL(nst_obj_eq(key, curr_node.key, NULL))) )
    {
        nst_dec_ref(curr_node.key);
        nst_dec_ref(curr_node.value);

        nodes[i].hash = -1;
        nodes[i].key = NULL;
        nodes[i].value = NULL;
        map->item_count--;

        if ( curr_node.next_idx != -1 )
            nodes[curr_node.next_idx].prev_idx = curr_node.prev_idx;
        else
            map->tail_idx = curr_node.prev_idx;

        if ( curr_node.prev_idx != -1 )
            nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
        else
            map->head_idx = curr_node.next_idx;

        resize_map(map, true);
        NST_RETURN_TRUE;
    }

    for ( size_t perturb = (size_t)hash; ; perturb >>= 5 )
    {
        i = (int32_t)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];

        if ( curr_node.key == NULL )
            NST_RETURN_FALSE;

        if ( curr_node.hash == hash &&
            (curr_node.key == key || AS_BOOL(nst_obj_eq(key, curr_node.key, NULL))) )
        {
            nst_dec_ref(curr_node.key);
            nst_dec_ref(curr_node.value);
            
            nodes[i & mask].hash = -1;
            nodes[i & mask].key = NULL;
            nodes[i & mask].value = NULL;
            map->item_count--;

            if ( curr_node.next_idx != -1 )
                nodes[curr_node.next_idx].prev_idx = curr_node.prev_idx;
            else
                map->tail_idx = curr_node.prev_idx;

            if ( curr_node.prev_idx != -1 )
                nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
            else
                map->head_idx = curr_node.next_idx;

            resize_map(map, true);
            NST_RETURN_TRUE;
        }
    }
}

void nst_destroy_map(Nst_MapObj *map)
{
    for ( Nst_Int i = _nst_map_get_next_idx(-1, map);
          i != -1;
          i = _nst_map_get_next_idx(i, map) )
    {
        nst_dec_ref(map->nodes[i].key);
        nst_dec_ref(map->nodes[i].value);
    }

    free(map->nodes);
}

void _nst_map_set_str(Nst_MapObj *map, const char *key, Nst_Obj *value)
{
    Nst_Obj *key_obj = nst_new_string_raw(key, false);
    nst_map_set(map, key_obj, value);
    nst_dec_ref(key_obj);
}

Nst_Obj *_nst_map_get_str(Nst_MapObj *map, const char *key)
{
    Nst_Obj *key_obj = nst_new_string_raw(key, false);
    Nst_Obj *value = _nst_map_get(map, key_obj);
    nst_dec_ref(key_obj);
    return value;
}

Nst_Obj *_nst_map_drop_str(Nst_MapObj *map, const char *key)
{
    Nst_Obj *key_obj = nst_new_string_raw(key, false);
    Nst_Obj *value = _nst_map_drop(map, key_obj);
    nst_dec_ref(key_obj);
    return value;
}

void nst_traverse_map(Nst_MapObj *map)
{
    for ( Nst_Int i = _nst_map_get_next_idx(-1, map);
          i != -1;
          i = _nst_map_get_next_idx(i, map) )
        // don't really care if the object is tracked by the garbage collector or not
        // the keys are never tracked since they can only be integers or strings
        NST_SET_FLAG(map->nodes[i].value, NST_FLAG_GGC_REACHABLE);
}

Nst_Int _nst_map_get_next_idx(Nst_Int curr_idx, Nst_MapObj *map)
{
    if ( curr_idx == -1 )
        return map->head_idx;
    else
        return map->nodes[curr_idx].next_idx;
}
