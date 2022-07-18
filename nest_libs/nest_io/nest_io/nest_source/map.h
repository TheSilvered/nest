#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>
#include "obj.h"
#include "simple_types.h"
#include "str.h"

#define MAP_MIN_SIZE 32
#define AS_MAP(ptr) ((Nst_MapObj *)(ptr))

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
    OBJ_HEAD;
    size_t size;
    size_t item_count;
    size_t mask;
    MapNode *nodes;
}
Nst_MapObj;

Nst_MapObj *new_map();
bool map_set(Nst_MapObj *map, Nst_Obj *key, Nst_Obj *value);
Nst_Obj *map_get(Nst_MapObj *map, Nst_Obj *key);
Nst_Obj *map_drop(Nst_MapObj *map, Nst_Obj *key);

void destroy_map(Nst_MapObj *map);
Nst_int get_next_idx(Nst_int curr_idx, Nst_MapObj *map);

inline void map_set_str(Nst_MapObj *map, const char *key, Nst_Obj *value)
{
    Nst_Obj *key_obj = new_string_raw(key, false);
    map_set(map, key_obj, value);
    dec_ref(key_obj);
}

inline Nst_Obj *map_get_str(Nst_MapObj *map, const char *key)
{
    Nst_Obj *key_obj = new_string_raw(key, false);
    Nst_Obj *value = map_get(map, key_obj);
    dec_ref(key_obj);
    return value;
}

inline Nst_Obj *map_drop_str(Nst_MapObj *map, const char *key)
{
    Nst_Obj *key_obj = new_string_raw(key, false);
    Nst_Obj *value = map_drop(map, key_obj);
    dec_ref(key_obj);
    return value;
}

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MAP_H
