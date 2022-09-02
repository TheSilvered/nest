#ifndef OBJ_H
#define OBJ_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define nst_dec_ref(obj) _dec_ref((Nst_Obj *)obj)
#define nst_inc_ref(obj) _inc_ref((Nst_Obj *)obj)
#define nst_destroy_obj(obj) _destroy_obj((Nst_Obj *)obj)

#define NST_OBJ_HEAD \
    int ref_count; \
    char *type_name; \
    struct Nst_Obj *type; \
    void (*destructor)(void *); \
    int32_t hash

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct Nst_Obj
{
    NST_OBJ_HEAD;
}
Nst_Obj;

Nst_Obj *nst_alloc_obj(size_t size, Nst_Obj *type, void (*destructor)(void *));
void _nst_init_obj(void);
void _nst_del_obj(void);

inline void _destroy_obj(Nst_Obj *obj);

inline Nst_Obj *_inc_ref(Nst_Obj *obj)
{
    obj->ref_count++;
    return obj;
}

inline void _dec_ref(Nst_Obj *obj)
{
    obj->ref_count--;
    if ( obj->ref_count <= 0 || obj == obj->type && obj->ref_count == 1 )
        _destroy_obj(obj);
}

inline void _destroy_obj(Nst_Obj *obj)
{
    obj->ref_count = 2147483647;
    if ( obj->destructor != NULL )
        (*obj->destructor)(obj);
    if ( obj != obj->type )
        nst_dec_ref(obj->type);
    free(obj);
}

extern Nst_Obj *nst_t_type;
extern Nst_Obj *nst_t_int;
extern Nst_Obj *nst_t_real;
extern Nst_Obj *nst_t_bool;
extern Nst_Obj *nst_t_null;
extern Nst_Obj *nst_t_str;
extern Nst_Obj *nst_t_arr;
extern Nst_Obj *nst_t_vect;
extern Nst_Obj *nst_t_map;
extern Nst_Obj *nst_t_func;
extern Nst_Obj *nst_t_iter;
extern Nst_Obj *nst_t_byte;
extern Nst_Obj *nst_t_file;

extern Nst_Obj *nst_true;
extern Nst_Obj *nst_false;
extern Nst_Obj *nst_null;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OBJ_H
