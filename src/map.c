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

Nst_Obj *Nst_map_new(void)
{
    Nst_MapObj *map = Nst_obj_alloc(
        Nst_MapObj,
        Nst_t.Map,
        _Nst_map_destroy);
    if (map == NULL)
        return NULL;

    map->len = 0;
    map->nodes = Nst_calloc_c(_Nst_MAP_MIN_SIZE, Nst_MapNode, NULL);

    if (map->nodes == NULL) {
        Nst_free(map);
        return NULL;
    }

    map->mask = _Nst_MAP_MIN_SIZE - 1;
    map->cap = _Nst_MAP_MIN_SIZE;
    map->head_idx = -1;
    map->tail_idx = -1;

    Nst_GGC_OBJ_INIT(map, _Nst_map_traverse, _Nst_map_track);

    return OBJ(map);
}

static i32 set_clean(Nst_MapObj *map, i32 hash, Nst_Obj *key, Nst_Obj *value,
                     i32 prev_idx)
{
    assert(key != NULL);

    usize mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    i32 i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    // every new key must end in an empty node because there are no duplicates
    for (usize perturb = (usize)hash; curr_node.key != NULL; perturb >>= 5) {
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

bool _Nst_map_resize(Nst_MapObj *map, bool force_item_reset)
{
    usize old_size = map->cap;
    Nst_MapNode *old_nodes = map->nodes;
    usize size;
    if (old_size - map->len < old_size >> 2)
        size = old_size << 1;
    else if (old_size > _Nst_MAP_MIN_SIZE && old_size >> 2 >= map->len)
        size = old_size >> 1;
    else if (force_item_reset)
        size = old_size; // resize_map only resets the items
    else
        return true;

    map->nodes = Nst_calloc_c(size, Nst_MapNode, NULL);
    if (map->nodes == NULL) {
        map->nodes = old_nodes;
        return old_size > size;
    }
    map->mask = size - 1;
    map->cap = size;

    i32 prev_idx = -1;
    i32 new_idx = 0;

    for (i32 i = map->head_idx; i != -1; i = old_nodes[i].next_idx) {
        new_idx = set_clean(
            map,
            old_nodes[i].hash,
            old_nodes[i].key,
            old_nodes[i].value,
            prev_idx);

        if (prev_idx != -1)
            map->nodes[prev_idx].next_idx = new_idx;
        else
            map->head_idx = new_idx;
        prev_idx = new_idx;
    }
    map->tail_idx = prev_idx;

    Nst_free(old_nodes);
    return true;
}

bool _Nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value)
{
    i32 hash = key->hash;

    if (hash == -1) {
        hash = Nst_obj_hash(key);
        if (hash == -1) {
            Nst_set_value_error(Nst_sprintf(
                _Nst_EM_UNHASHABLE_TYPE,
                TYPE_NAME(key)));
            return false;
        }
    }

    if (!_Nst_map_resize(map, false))
        return false;

    usize mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    usize i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    for (usize perturb = (usize)hash;
         curr_node.key != NULL && curr_node.key != key;
         perturb >>= 5)
    {
        if (curr_node.hash == hash && Nst_obj_eq_c(key, curr_node.key))
            break;

        i = (i32)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];
    }

    Nst_inc_ref(key);
    Nst_inc_ref(value);

    if (curr_node.key != NULL) {
        Nst_dec_ref(curr_node.key);
        Nst_dec_ref(curr_node.value);
    } else {
        map->len++;

        // if it's the first node inserted
        if (map->head_idx == -1) {
            map->head_idx = (i32)(i & mask);
            (nodes + (i & mask))->prev_idx = -1;
        } else {
            nodes[map->tail_idx].next_idx = (i32)(i & mask);
            (nodes + (i & mask))->prev_idx = map->tail_idx;
        }
        map->tail_idx = (i32)(i & mask);
        (nodes + (i & mask))->next_idx = -1;
    }

    (nodes + (i & mask))->hash = hash;
    (nodes + (i & mask))->key = key;
    (nodes + (i & mask))->value = value;

    if (Nst_OBJ_IS_TRACKED(map)
        && Nst_FLAG_HAS(value, Nst_FLAG_GGC_IS_SUPPORTED))
    {
        Nst_ggc_track_obj((Nst_GGCObj*)value);
    }

    return true;
}

Nst_Obj *_Nst_map_get(Nst_MapObj *map, Nst_Obj *key)
{
    i32 hash = key->hash;

    if (hash == -1) {
        hash = Nst_obj_hash(key);
        if (hash == -1)
            return NULL;
    }

    usize mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    i32 i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    if (curr_node.key == NULL)
        return NULL;

    if (curr_node.hash == hash && Nst_obj_eq_c(key, curr_node.key)) {
        Nst_inc_ref(curr_node.value);
        return curr_node.value;
    }

    for (usize perturb = (usize)hash; ; perturb >>= 5) {
        i = (i32)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];

        if (curr_node.key == NULL)
            return NULL;

        if (curr_node.hash == hash && Nst_obj_eq_c(key, curr_node.key)) {
            Nst_inc_ref(curr_node.value);
            return curr_node.value;
        }
    }
    return NULL;
}

Nst_Obj *_Nst_map_drop(Nst_MapObj *map, Nst_Obj *key)
{
    i32 hash = key->hash;

    if (hash == -1) {
        hash = Nst_obj_hash(key);
        if (hash == -1)
            return NULL;
    }

    usize mask = map->mask;
    Nst_MapNode *nodes = map->nodes;
    i32 i = hash & mask;
    Nst_MapNode curr_node = nodes[i];

    if (curr_node.key == NULL)
        return NULL;

    if (curr_node.hash == hash && Nst_obj_eq_c(key, curr_node.key)) {
        Nst_dec_ref(curr_node.key);
        Nst_Obj *node_value = curr_node.value;

        nodes[i].hash = -1;
        nodes[i].key = NULL;
        nodes[i].value = NULL;
        map->len--;

        if (curr_node.next_idx != -1)
            nodes[curr_node.next_idx].prev_idx = curr_node.prev_idx;
        else
            map->tail_idx = curr_node.prev_idx;

        if (curr_node.prev_idx != -1)
            nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
        else
            map->head_idx = curr_node.next_idx;

        _Nst_map_resize(map, true);
        return node_value;
    }

    for (usize perturb = (usize)hash; ; perturb >>= 5) {
        i = (i32)((i * 5) + 1 + perturb);
        curr_node = nodes[i & mask];

        if (curr_node.key == NULL)
            return NULL;

        if (curr_node.hash == hash && Nst_obj_eq_c(key, curr_node.key)) {
            Nst_dec_ref(curr_node.key);
            Nst_Obj *node_value = curr_node.value;

            nodes[i & mask].hash = -1;
            nodes[i & mask].key = NULL;
            nodes[i & mask].value = NULL;
            map->len--;

            if (curr_node.next_idx != -1)
                nodes[curr_node.next_idx].prev_idx = curr_node.prev_idx;
            else
                map->tail_idx = curr_node.prev_idx;

            if (curr_node.prev_idx != -1)
                nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
            else
                map->head_idx = curr_node.next_idx;

            _Nst_map_resize(map, true);
            return node_value;
        }
    }
    return NULL;
}

void _Nst_map_destroy(Nst_MapObj *map)
{
    for (i32 i = Nst_map_get_next_idx(-1, map);
         i != -1;
         i = Nst_map_get_next_idx(i, map))
    {
        Nst_dec_ref(map->nodes[i].key);
        Nst_dec_ref(map->nodes[i].value);
    }

    Nst_free(map->nodes);
}

bool _Nst_map_set_str(Nst_MapObj *map, const i8 *key, Nst_Obj *value)
{
    Nst_Obj *key_obj = Nst_string_new_c_raw(key, false);
    if (key_obj == NULL)
        return false;
    bool res = Nst_map_set(map, key_obj, value);
    Nst_dec_ref(key_obj);
    return res;
}

Nst_Obj *_Nst_map_get_str(Nst_MapObj *map, const i8 *key)
{
    Nst_StrObj key_obj;
    key_obj.value = (i8 *)key;
    key_obj.len = strlen(key);
    key_obj.hash = -1;
    key_obj.type = Nst_t.Str;

    Nst_Obj *value = Nst_map_get(map, &key_obj);
    return value;
}

Nst_Obj *_Nst_map_drop_str(Nst_MapObj *map, const i8 *key)
{
    Nst_StrObj key_obj;
    key_obj.value = (i8 *)key;
    key_obj.len = strlen(key);
    key_obj.hash = -1;
    key_obj.type = Nst_t.Str;
    Nst_Obj *value = Nst_map_drop(map, &key_obj);

    return value;
}

void _Nst_map_traverse(Nst_MapObj *map)
{
    for (i32 i = Nst_map_get_next_idx(-1, map);
         i != -1;
         i = Nst_map_get_next_idx(i, map))
    {
        // don't really care if the object is tracked by the garbage collector
        // or not, keys shouldn't be tracked but for good mesure the flag is
        // added reguardless
        Nst_FLAG_SET(map->nodes[i].key,   Nst_FLAG_GGC_REACHABLE);
        Nst_FLAG_SET(map->nodes[i].value, Nst_FLAG_GGC_REACHABLE);
    }
}

void _Nst_map_track(Nst_MapObj *map)
{
    for (i32 i = Nst_map_get_next_idx(-1, map);
         i != -1;
         i = Nst_map_get_next_idx(i, map))
    {
        if (Nst_FLAG_HAS(map->nodes[i].value, Nst_FLAG_GGC_IS_SUPPORTED))
            Nst_ggc_track_obj((Nst_GGCObj*)(map->nodes[i].value));
    }
}

i32 _Nst_map_get_next_idx(i32 curr_idx, Nst_MapObj *map)
{
    if (curr_idx == -1)
        return map->head_idx;
    else
        return map->nodes[curr_idx].next_idx;
}

i32 _Nst_map_get_prev_idx(i32 curr_idx, Nst_MapObj *map)
{
    if (curr_idx == -1)
        return map->tail_idx;
    else
        return map->nodes[curr_idx].prev_idx;
}
