#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>
#include "obj.h"
#include "simple_types.h"
#include "str.h"

#define MAP_MIN_SIZE 32
#define AS_MAP(ptr) ((Nst_MapObj *)(ptr))

#define nst_map_set(map, key, value) _nst_map_set(AS_MAP(map), (Nst_Obj *)key, (Nst_Obj *)value)
#define nst_map_get(map, key) _nst_map_get(AS_MAP(map), (Nst_Obj *)key)
#define nst_map_drop(map, key) _nst_map_drop(AS_MAP(map), (Nst_Obj *)key)

#define nst_map_get_next_idx(curr_idx, map) _nst_map_get_next_idx(curr_idx, AS_MAP(map))

#define nst_map_set_str(map, key, value) _nst_map_set_str(AS_MAP(map), key, (Nst_Obj *)value)
#define nst_map_get_str(map, key) _nst_map_get_str(AS_MAP(map), key)
#define nst_map_drop_str(map, key) _nst_map_drop_str(AS_MAP(map), key)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    int32_t hash;
    Nst_Obj *key;
    Nst_Obj *value;
}
Nst_MapNode;

typedef struct
{
    OBJ_HEAD;
    size_t size;
    size_t item_count;
    size_t mask;
    Nst_MapNode *nodes;
}
Nst_MapObj;

Nst_Obj *nst_new_map();
bool _nst_map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value);
Nst_Obj *_nst_map_get(Nst_MapObj *map, Nst_Obj *key);
Nst_Obj *_nst_map_drop(Nst_MapObj *map, Nst_Obj *key);

void nst_destroy_map(Nst_MapObj *map);
Nst_Int _nst_map_get_next_idx(Nst_Int curr_idx, Nst_MapObj *map);

inline void _nst_map_set_str(Nst_MapObj *map, const char *key, Nst_Obj *value)
{
    Nst_Obj *key_obj = nst_new_string_raw(key, false);
    nst_map_set(map, key_obj, value);
    dec_ref(key_obj);
}

inline Nst_Obj *_nst_map_get_str(Nst_MapObj *map, const char *key)
{
    Nst_Obj *key_obj = nst_new_string_raw(key, false);
    Nst_Obj *value = _nst_map_get(map, key_obj);
    dec_ref(key_obj);
    return value;
}

inline Nst_Obj *_nst_map_drop_str(Nst_MapObj *map, const char *key)
{
    Nst_Obj *key_obj = nst_new_string_raw(key, false);
    Nst_Obj *value = _nst_map_drop(map, key_obj);
    dec_ref(key_obj);
    return value;
}

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MAP_H
