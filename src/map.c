#include <errno.h>
#include <math.h>
#include "mem.h"
#include "map.h"
#include "hash.h"
#include "lib_import.h"
#include "obj_ops.h"
#include "global_consts.h"
#include "format.h"
#include "string.h"

/**
 * @param hash: the hash of the key contained in the node
 * @param key: the key of the node
 * @param value: the value of the node
 * @param next_idx: the index of the next node inside the map
 * @param prev_idx: the index of the previous node inside the map
 */
NstEXP typedef struct _Nst_MapNode {
    i32 hash;
    Nst_Obj *key;
    Nst_Obj *value;
    i32 next_idx;
    i32 prev_idx;
} Nst_MapNode;

/**
 * @param cap: the current capacity of the nodes array
 * @param len: the number of nodes inside the map
 * @param mask: the mask applied to the hash when inserting new nodes
 * @param nodes: the array of nodes of the map
 * @param head_idx: the first node in the map
 * @param tail_idx: the last node in the map
 */
NstEXP typedef struct _Nst_MapObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    usize cap;
    usize len;
    usize mask;
    Nst_MapNode *nodes;
    i32 head_idx;
    i32 tail_idx;
} Nst_MapObj;

#define MAP(ptr) ((Nst_MapObj *)(ptr))

static bool resize_map(Nst_MapObj *map, bool force_item_reset);

Nst_Obj *Nst_map_new(void)
{
    Nst_MapObj *map = Nst_obj_alloc(Nst_MapObj, Nst_t.Map);
    if (map == NULL)
        return NULL;

    Nst_GGC_OBJ_INIT(map);

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

    return OBJ(map);
}

static i32 set_clean(Nst_MapObj *map, i32 hash, Nst_Obj *key, Nst_Obj *value,
                     i32 prev_idx)
{
    Nst_assert(key != NULL);

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

static bool resize_map(Nst_MapObj *map, bool force_item_reset)
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

bool Nst_map_set(Nst_Obj *map, Nst_Obj *key, Nst_Obj *value)
{
    Nst_assert(map->type == Nst_t.Map);
    i32 hash = key->hash;

    if (hash == -1) {
        hash = Nst_obj_hash(key);
        if (hash == -1) {
            Nst_set_value_errorf(
                _Nst_EM_UNHASHABLE_TYPE,
                TYPE_NAME(key));
            return false;
        }
    }

    if (!resize_map(MAP(map), false))
        return false;

    usize mask = MAP(map)->mask;
    Nst_MapNode *nodes = MAP(map)->nodes;
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
        MAP(map)->len++;

        // if it's the first node inserted
        if (MAP(map)->head_idx == -1) {
            MAP(map)->head_idx = (i32)(i & mask);
            (nodes + (i & mask))->prev_idx = -1;
        } else {
            nodes[MAP(map)->tail_idx].next_idx = (i32)(i & mask);
            (nodes + (i & mask))->prev_idx = MAP(map)->tail_idx;
        }
        MAP(map)->tail_idx = (i32)(i & mask);
        (nodes + (i & mask))->next_idx = -1;
    }

    (nodes + (i & mask))->hash = hash;
    (nodes + (i & mask))->key = key;
    (nodes + (i & mask))->value = value;

    return true;
}

Nst_Obj *Nst_map_get(Nst_Obj *map, Nst_Obj *key)
{
    Nst_assert(map->type == Nst_t.Map);
    i32 hash = key->hash;

    if (hash == -1) {
        hash = Nst_obj_hash(key);
        if (hash == -1)
            return NULL;
    }

    usize mask = MAP(map)->mask;
    Nst_MapNode *nodes = MAP(map)->nodes;
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
}

Nst_Obj *Nst_map_drop(Nst_Obj *map, Nst_Obj *key)
{
    Nst_assert(map->type == Nst_t.Map);
    i32 hash = key->hash;

    if (hash == -1) {
        hash = Nst_obj_hash(key);
        if (hash == -1)
            return NULL;
    }

    usize mask = MAP(map)->mask;
    Nst_MapNode *nodes = MAP(map)->nodes;
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
        MAP(map)->len--;

        if (curr_node.next_idx != -1)
            nodes[curr_node.next_idx].prev_idx = curr_node.prev_idx;
        else
            MAP(map)->tail_idx = curr_node.prev_idx;

        if (curr_node.prev_idx != -1)
            nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
        else
            MAP(map)->head_idx = curr_node.next_idx;

        resize_map(MAP(map), true);
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
            MAP(map)->len--;

            if (curr_node.next_idx != -1)
                nodes[curr_node.next_idx].prev_idx = curr_node.prev_idx;
            else
                MAP(map)->tail_idx = curr_node.prev_idx;

            if (curr_node.prev_idx != -1)
                nodes[curr_node.prev_idx].next_idx = curr_node.next_idx;
            else
                MAP(map)->head_idx = curr_node.next_idx;

            resize_map(MAP(map), true);
            return node_value;
        }
    }
}

Nst_Obj *Nst_map_copy(Nst_Obj *map)
{
    Nst_assert(map->type == Nst_t.Map);
    Nst_MapObj *new_map = MAP(Nst_map_new());
    if (new_map == NULL)
        return NULL;

    new_map->len      = MAP(map)->len;
    new_map->mask     = MAP(map)->mask;
    new_map->cap      = MAP(map)->cap;
    new_map->head_idx = MAP(map)->head_idx;
    new_map->tail_idx = MAP(map)->tail_idx;

    Nst_MapNode *new_nodes = Nst_crealloc_c(
        new_map->nodes,
        new_map->cap,
        Nst_MapNode,
        _Nst_MAP_MIN_SIZE,
        NULL);

    if (new_nodes == NULL) {
        Nst_dec_ref(new_map);
        return NULL;
    }

    new_map->nodes = new_nodes;
    Nst_MapNode *old_nodes = MAP(map)->nodes;

    for (usize i = 0, n = MAP(map)->cap; i < n; i++) {
        if (old_nodes[i].key == NULL)
            continue;
        new_nodes[i].hash = old_nodes[i].hash;
        new_nodes[i].key = Nst_inc_ref(old_nodes[i].key);
        new_nodes[i].value = Nst_inc_ref(old_nodes[i].value);
        new_nodes[i].next_idx = old_nodes[i].next_idx;
        new_nodes[i].prev_idx = old_nodes[i].prev_idx;
    }

    return OBJ(new_map);
}

void _Nst_map_destroy(Nst_Obj *map)
{
    Nst_assert(map->type == Nst_t.Map);
    Nst_Obj *key;
    Nst_Obj *val;
    for (isize i = Nst_map_next(-1, map, &key, &val);
         i != -1;
         i = Nst_map_next(i, map, &key, &val))
    {
        Nst_dec_ref(key);
        Nst_dec_ref(val);
    }

    Nst_free(MAP(map)->nodes);
}

bool Nst_map_set_str(Nst_Obj *map, const i8 *key, Nst_Obj *value)
{
    Nst_assert(map->type == Nst_t.Map);
    Nst_Obj *key_obj = Nst_str_new_c_raw(key, false);
    if (key_obj == NULL)
        return false;
    bool res = Nst_map_set(map, key_obj, value);
    Nst_dec_ref(key_obj);
    return res;
}

Nst_Obj *Nst_map_get_str(Nst_Obj *map, const i8 *key)
{
    Nst_assert(map->type == Nst_t.Map);
    Nst_StrObj key_obj;
    key_obj.value = (i8 *)key;
    key_obj.len = strlen(key);
    key_obj.hash = -1;
    key_obj.type = Nst_t.Str;

    Nst_Obj *value = Nst_map_get(map, OBJ(&key_obj));
    return value;
}

Nst_Obj *Nst_map_drop_str(Nst_Obj *map, const i8 *key)
{
    Nst_assert(map->type == Nst_t.Map);
    Nst_StrObj key_obj;
    key_obj.value = (i8 *)key;
    key_obj.len = strlen(key);
    key_obj.hash = -1;
    key_obj.type = Nst_t.Str;
    Nst_Obj *value = Nst_map_drop(map, OBJ(&key_obj));

    return value;
}

void _Nst_map_traverse(Nst_Obj *map)
{
    Nst_assert(map->type == Nst_t.Map);
    Nst_Obj *key;
    Nst_Obj *val;
    for (isize i = Nst_map_next(-1, map, &key, &val);
         i != -1;
         i = Nst_map_next(i, map, &key, &val))
    {
        Nst_ggc_obj_reachable(key);
        Nst_ggc_obj_reachable(val);
    }
}

isize Nst_map_next(isize idx, Nst_Obj *map, Nst_Obj **out_key,
                   Nst_Obj **out_val)
{
    Nst_assert(map->type == Nst_t.Map);
    isize new_idx = idx == -1
        ? MAP(map)->head_idx
        : MAP(map)->nodes[idx].next_idx;
    if (out_key != NULL)
        *out_key = MAP(map)->nodes[new_idx].key;
    if (out_val != NULL)
        *out_val = MAP(map)->nodes[new_idx].value;
    return new_idx;
}

isize Nst_map_prev(isize idx, Nst_Obj *map, Nst_Obj **out_key,
                   Nst_Obj **out_val)
{
    Nst_assert(map->type == Nst_t.Map);
    isize new_idx = idx == -1
        ? MAP(map)->tail_idx
        : MAP(map)->nodes[idx].prev_idx;
    if (out_key != NULL)
        *out_key = MAP(map)->nodes[new_idx].key;
    if (out_val != NULL)
        *out_val = MAP(map)->nodes[new_idx].value;
    return new_idx;
}

usize Nst_map_len(Nst_Obj *map)
{
    Nst_assert(map->type == Nst_t.Map);
    return MAP(map)->len;
}

usize Nst_map_cap(Nst_Obj *map)
{
    Nst_assert(map->type == Nst_t.Map);
    return MAP(map)->cap;
}
