#include <errno.h>
#include <math.h>
#include <assert.h>
#include "mem.h"
#include "map.h"
#include "hash.h"
#include "lib_import.h"
#include "obj_ops.h"
#include "global_consts.h"
#include "format.h"
#include "string.h"

Nst_Obj *nst_map_new()
{
    Nst_MapObj *map = nst_obj_alloc(
        Nst_MapObj,
        nst_t.Map,
        _nst_map_destroy);
    if ( map == NULL )
    {
        return NULL;
    }

    map->item_count = 0;
    map->nodes = nst_calloc_c(_NST_MAP_MIN_SIZE, Nst_MapNode, NULL);

    if ( map->nodes == NULL )
    {
        nst_free(map);
        return NULL;
    }

    map->mask = _NST_MAP_MIN_SIZE - 1;
    map->size = _NST_MAP_MIN_SIZE;
    map->head_idx = -1;
    map->tail_idx = -1;

    NST_GGC_OBJ_INIT(map, _nst_map_traverse, _nst_map_track);

    return OBJ(map);
}

static i32 set_clean(Nst_MapObj *map,
                     i32         hash,
                     Nst_Obj    *key,
                     Nst_Obj    *value,
                     i32         prev_idx)
{
    assert(key != NULL);

    usize mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    i32 i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    // every new key must end in an empty node because there are no duplicates
    for ( usize perturb = (usize)hash;
          curr_node.key != NULL;
          perturb >>= 5 )
    {
        i = (i32)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];
    }

    (nodes + (i & mask))->hash = hash;
    (nodes + (i & mask))->key = key;
    (nodes + (i & mask))->value = value;
    (nodes + (i & mask))->prev_idx = prev_idx;
    (nodes + (i & mask))->next_idx = -1;
    return i & mask;
}

static bool are_eq(Nst_Obj *ob1, Nst_Obj *ob2)
{
    if ( ob1 == ob2 )
    {
        return true;
    }

    if ( nst_obj_eq(ob1, ob2) == nst_c.Bool_true )
    {
        nst_dec_ref(nst_c.Bool_true);
        return true;
    }
    else
    {
        nst_dec_ref(nst_c.Bool_false);
        return false;
    }
}

bool _nst_map_resize(Nst_MapObj *map, bool force_item_reset)
{
    usize old_size = map->size;
    Nst_MapNode *old_nodes = map->nodes;
    usize size;
    if ( old_size - map->item_count < old_size >> 2 )
    {
        size = old_size << 1;
    }
    else if ( old_size > _NST_MAP_MIN_SIZE && old_size >> 2 >= map->item_count )
    {
        size = old_size >> 1;
    }
    else if ( force_item_reset )
    {
        size = old_size; // resize_map only resets the items
    }
    else
    {
        return true;
    }

    map->nodes = nst_calloc_c(size, Nst_MapNode, NULL);
    if ( map->nodes == NULL )
    {
        map->nodes = old_nodes;
        return old_size > size;
    }
    map->mask = size - 1;
    map->size = size;

    i32 prev_idx = -1;
    i32 new_idx = 0;

    for ( i32 i = map->head_idx; i != -1; i = old_nodes[i].next_idx )
    {
        new_idx = set_clean(
            map,
            old_nodes[i].hash,
            old_nodes[i].key,
            old_nodes[i].value,
            prev_idx);

        if ( prev_idx != -1 )
        {
            map->nodes[prev_idx].next_idx = new_idx;
        }
        else
        {
            map->head_idx = new_idx;
        }
        prev_idx = new_idx;
    }
    map->tail_idx = prev_idx;

    nst_free(old_nodes);
    return true;
}

bool _nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value)
{
    i32 hash = key->hash;

    if ( hash == -1 )
    {
        hash = nst_obj_hash(key);
        if ( hash == -1 )
        {
            nst_set_value_error(nst_sprintf(
                _NST_EM_UNHASHABLE_TYPE,
                TYPE_NAME(key)));
            return false;
        }
    }

    if ( !_nst_map_resize(map, false) )
    {
        return false;
    }

    usize mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    usize i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    for ( usize perturb = (usize)hash;
          curr_node.key != NULL && curr_node.key != key;
          perturb >>= 5 )
    {
        if ( curr_node.hash == hash && are_eq(key, curr_node.key) )
        {
            break;
        }

        i = (i32)((i * 5) + 1 + perturb);
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
            {
                nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
            }
            else
            {
                map->head_idx = curr_node.next_idx;
            }

            nodes[map->tail_idx].next_idx = (i32)(i & mask);
            (nodes + (i & mask))->prev_idx = map->tail_idx;
            map->tail_idx = (i32)(i & mask);
            (nodes + (i & mask))->next_idx = -1;
        }
    }
    else
    {
        map->item_count++;

        // if it's the first node inserted
        if ( map->head_idx == -1 )
        {
            map->head_idx = (i32)(i & mask);
            (nodes + (i & mask))->prev_idx = -1;
        }
        else
        {
            nodes[map->tail_idx].next_idx = (i32)(i & mask);
            (nodes + (i & mask))->prev_idx = map->tail_idx;
        }
        map->tail_idx = (i32)(i & mask);
        (nodes + (i & mask))->next_idx = -1;
    }

    (nodes + (i & mask))->hash = hash;
    (nodes + (i & mask))->key = key;
    (nodes + (i & mask))->value = value;

    if ( NST_OBJ_IS_TRACKED(map) &&
         NST_FLAG_HAS(value, NST_FLAG_GGC_IS_SUPPORTED) )
    {
        nst_ggc_track_obj((Nst_GGCObj*)value);
    }

    return true;
}

Nst_Obj *_nst_map_get(Nst_MapObj *map, Nst_Obj *key)
{
    i32 hash = key->hash;

    if ( hash == -1 )
    {
        hash = nst_obj_hash(key);
        if ( hash == -1 )
        {
            return NULL;
        }
    }

    usize mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    i32 i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    if ( curr_node.key == NULL )
    {
        return NULL;
    }

    if ( curr_node.hash == hash && are_eq(key, curr_node.key) )
    {
        nst_inc_ref(curr_node.value);
        return curr_node.value;
    }

    for ( usize perturb = (usize)hash; ; perturb >>= 5 )
    {
        i = (i32)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];

        if ( curr_node.key == NULL )
        {
            return NULL;
        }

        if ( curr_node.hash == hash && are_eq(key, curr_node.key) )
        {
            nst_inc_ref(curr_node.value);
            return curr_node.value;
        }
    }
}

Nst_Obj *_nst_map_drop(Nst_MapObj *map, Nst_Obj *key)
{
    i32 hash = key->hash;

    if ( hash == -1 )
    {
        hash = nst_obj_hash(key);
        if ( hash == -1 )
        {
            return NULL;
        }
    }

    usize mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    i32 i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    if ( curr_node.key == NULL )
    {
        NST_RETURN_FALSE;
    }

    if ( curr_node.hash == hash && are_eq(key, curr_node.key) )
    {
        nst_dec_ref(curr_node.key);
        nst_dec_ref(curr_node.value);

        nodes[i].hash = -1;
        nodes[i].key = NULL;
        nodes[i].value = NULL;
        map->item_count--;

        if ( curr_node.next_idx != -1 )
        {
            nodes[curr_node.next_idx].prev_idx = curr_node.prev_idx;
        }
        else
        {
            map->tail_idx = curr_node.prev_idx;
        }

        if ( curr_node.prev_idx != -1 )
        {
            nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
        }
        else
        {
            map->head_idx = curr_node.next_idx;
        }

        _nst_map_resize(map, true);
        NST_RETURN_TRUE;
    }

    for ( usize perturb = (usize)hash; ; perturb >>= 5 )
    {
        i = (i32)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];

        if ( curr_node.key == NULL )
        {
            NST_RETURN_FALSE;
        }

        if ( curr_node.hash == hash && are_eq(key, curr_node.key) )
        {
            nst_dec_ref(curr_node.key);
            nst_dec_ref(curr_node.value);

            nodes[i & mask].hash = -1;
            nodes[i & mask].key = NULL;
            nodes[i & mask].value = NULL;
            map->item_count--;

            if ( curr_node.next_idx != -1 )
            {
                nodes[curr_node.next_idx].prev_idx = curr_node.prev_idx;
            }
            else
            {
                map->tail_idx = curr_node.prev_idx;
            }

            if ( curr_node.prev_idx != -1 )
            {
                nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
            }
            else
            {
                map->head_idx = curr_node.next_idx;
            }

            _nst_map_resize(map, true);
            NST_RETURN_TRUE;
        }
    }
}

void _nst_map_destroy(Nst_MapObj *map)
{
    for ( i32 i = nst_map_get_next_idx(-1, map);
          i != -1;
          i = nst_map_get_next_idx(i, map) )
    {
        nst_dec_ref(map->nodes[i].key);
        nst_dec_ref(map->nodes[i].value);
    }

    nst_free(map->nodes);
}

bool _nst_map_set_str(Nst_MapObj *map, const i8 *key, Nst_Obj *value)
{
    Nst_Obj *key_obj = nst_string_new_c_raw(key, false);
    if ( key_obj == NULL )
    {
        return false;
    }
    bool res = nst_map_set(map, key_obj, value);
    nst_dec_ref(key_obj);
    return res;
}

Nst_Obj *_nst_map_get_str(Nst_MapObj *map, const i8 *key)
{
    Nst_StrObj key_obj;
    key_obj.value = (i8 *)key;
    key_obj.len = strlen(key);
    key_obj.hash = -1;
    key_obj.type = nst_t.Str;

    Nst_Obj *value = nst_map_get(map, &key_obj);
    return value;
}

Nst_Obj *_nst_map_drop_str(Nst_MapObj *map, const i8 *key)
{
    Nst_StrObj key_obj;
    key_obj.value = (i8 *)key;
    key_obj.len = strlen(key);
    key_obj.hash = -1;
    key_obj.type = nst_t.Str;
    Nst_Obj *value = nst_map_drop(map, &key_obj);

    return value;
}

void _nst_map_traverse(Nst_MapObj *map)
{
    for ( i32 i = nst_map_get_next_idx(-1, map);
          i != -1;
          i = nst_map_get_next_idx(i, map) )
    {
        // don't really care if the object is tracked by the garbage collector
        // or not, keys shouldn't be tracked but for good mesure the flag is
        // added reguardless
        NST_FLAG_SET(map->nodes[i].key,   NST_FLAG_GGC_REACHABLE);
        NST_FLAG_SET(map->nodes[i].value, NST_FLAG_GGC_REACHABLE);
    }
}

void _nst_map_track(Nst_MapObj *map)
{
    for ( i32 i = nst_map_get_next_idx(-1, map);
          i != -1;
          i = nst_map_get_next_idx(i, map) )
    {
        if ( NST_FLAG_HAS(map->nodes[i].value, NST_FLAG_GGC_IS_SUPPORTED) )
        {
            nst_ggc_track_obj((Nst_GGCObj*)(map->nodes[i].value));
        }
    }
}

i32 _nst_map_get_next_idx(i32 curr_idx, Nst_MapObj *map)
{
    if ( curr_idx == -1 )
    {
        return map->head_idx;
    }
    else
    {
        return map->nodes[curr_idx].next_idx;
    }
}

i32 _nst_map_get_prev_idx(i32 curr_idx, Nst_MapObj *map)
{
    if ( curr_idx == -1 )
    {
        return map->tail_idx;
    }
    else
    {
        return map->nodes[curr_idx].prev_idx;
    }
}
