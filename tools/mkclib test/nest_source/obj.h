#ifndef OBJ_H
#define OBJ_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct Nst_Obj
{
    int ref_count;
    void *value;
    char *type_name;
    struct Nst_Obj *type;
    void (*destructor)(void *);
    int32_t hash;
}
Nst_Obj;

Nst_Obj *make_obj(void *value, Nst_Obj *type, void (*destructor)(void *));
Nst_Obj *make_obj_free(void *value, Nst_Obj *type);
Nst_Obj *inc_ref(Nst_Obj *obj);
void dec_ref(Nst_Obj *obj);
void destroy_obj(Nst_Obj *obj);

void init_obj(void);
void del_obj(void);

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
