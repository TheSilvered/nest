#include <cmath>
#include "vec_lib.h"

static Nst_Declr obj_list_[] = {
    Nst_CONSTDECLR(Vec2),
    Nst_FUNCDECLR(vec2_, 2),
    Nst_FUNCDECLR(vec2_to_str_, 1),
    Nst_FUNCDECLR(x_, 1),
    Nst_FUNCDECLR(y_, 1),
    Nst_FUNCDECLR(len_, 1),
    Nst_FUNCDECLR(dot_, 2),
    Nst_FUNCDECLR(cross_, 2),
    Nst_FUNCDECLR(normalized_, 1),
};
static Nst_TypeObj *t_Vec2;

bool lib_init()
{
    t_Vec2 = Nst_type_new("Vec2");
    if (t_Vec2 == nullptr)
        return nullptr;
    return lib_init_;
}

void lib_quit()
{
    Nst_dec_ref(t_Vec2);
}

Nst_Obj *Vec2()
{
    return Nst_inc_ref(t_Vec2);
}

Nst_Obj *vec2_new(f64 x, f64 y)
{
    Vec2Obj *vec2 = Nst_obj_alloc(Vec2Obj, t_Vec2);
    if (vec2 == nullptr)
        return nullptr;

    vec2->x = x;
    vec2->y = y;

    return OBJ(vec2);
}

Nst_Obj *NstC vec2_(usize arg_num, Nst_Obj **args)
{
    f64 x, y;
    if (!Nst_extract_args("NN", arg_num, args, &x, &y))
        return nullptr;
    return vec2_new(x, y);
}

Nst_Obj *NstC vec2_to_str_(usize arg_num, Nst_Obj **args)
{
    Vec2Obj *vec2;
    if (!Nst_extract_args("#", arg_num, args, t_Vec2, &vec2))
        return nullptr;
    return Nst_sprintf("<Vec2 x=%lg y=%lg>", vec2->x, vec2->y);
}

Nst_Obj *NstC x_(usize arg_num, Nst_Obj **args)
{
    Vec2Obj *vec2;
    if (!Nst_extract_args("#", arg_num, args, t_Vec2, &vec2))
        return nullptr;
    return Nst_real_new(vec2->x);
}

Nst_Obj *NstC y_(usize arg_num, Nst_Obj **args)
{
    Vec2Obj *vec2;
    if (!Nst_extract_args("#", arg_num, args, t_Vec2, &vec2))
        return nullptr;
    return Nst_real_new(vec2->y);
}

Nst_Obj *NstC len_(usize arg_num, Nst_Obj **args)
{
    Vec2Obj *vec2;
    if (!Nst_extract_args("#", arg_num, args, t_Vec2, &vec2))
        return nullptr;
    f64 x = vec2->x;
    f64 y = vec2->y;
    return Nst_real_new(sqrt(x*x + y*y));
}

Nst_Obj *NstC dot_(usize arg_num, Nst_Obj **args)
{
    Vec2Obj *u;
    Vec2Obj *v;
    if (!Nst_extract_args("##", arg_num, args, t_Vec2, &u, t_Vec2, &v))
        return nullptr;

    return Nst_real_new(u->x * v->x + u->y * v->y);
}

Nst_Obj *NstC cross_(usize arg_num, Nst_Obj **args)
{
    Vec2Obj *u;
    Vec2Obj *v;
    if (!Nst_extract_args("##", arg_num, args, t_Vec2, &u, t_Vec2, &v))
        return nullptr;

    return Nst_real_new(u->x * v->y - v->x * u->y);
}

Nst_Obj *NstC normalized_(usize arg_num, Nst_Obj **args)
{
    Vec2Obj *vec2;
    if (!Nst_extract_args("#", arg_num, args, t_Vec2, &vec2))
        return nullptr;
    f64 x = vec2->x;
    f64 y = vec2->y;
    f64 len = sqrt(x*x + y*y);
    return vec2_new(x / len, y / len);
}
