#include <string.h>
#include "type.h"
#include "global_consts.h"
#include "mem.h"

static bool check_type_name(const u8 *name)
{
    u8 c = *name++;
    while (c) {
        if (c >= 0x80) {
            Nst_set_value_error_c(_Nst_EM_BAD_TYPE_NAME);
            return false;
        }
        c = *name++;
    }
    return true;
}

Nst_TypeObj *Nst_type_new(const i8 *name, Nst_ObjDstr dstr)
{
    if (!check_type_name((const u8 *)name))
        return NULL;

    Nst_TypeObj *type = Nst_obj_alloc(Nst_TypeObj, Nst_t.Type);
    if (type == NULL)
        return NULL;

    type->p_head = NULL;
    type->p_len = 0;
    type->dstr = dstr;
    type->name = Nst_sv_new_c(name);

    return type;
}

Nst_TypeObj *Nst_cont_type_new(const i8 *name, Nst_ObjDstr dstr,
                               Nst_ObjTrav trav)
{
    if (!check_type_name((const u8 *)name))
        return NULL;

    Nst_ContTypeObj *type = Nst_obj_alloc(Nst_ContTypeObj, Nst_t.Type);
    if (type == NULL)
        return NULL;

    type->p_head = NULL;
    type->p_len = 0;
    type->dstr = dstr;
    type->trav = trav;
    type->name = Nst_sv_new_c(name);

    Nst_SET_FLAG(type, Nst_FLAG_TYPE_IS_CONTAINER);

    return TYPE(type);
}

void _Nst_type_destroy(Nst_TypeObj *type)
{
    for (Nst_Obj *ob = type->p_head; ob != NULL;) {
        Nst_Obj *next_ob = ob->p_next;
        Nst_free(ob);
        ob = next_ob;
    }

    type->p_len = _Nst_P_LEN_MAX + 1;
}
