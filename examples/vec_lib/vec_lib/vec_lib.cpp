#include <cmath>
#include "vec_lib.h"

#define FUNC_COUNT 9

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_TypeObj *t_Vec2;

bool lib_init()
{
    usize idx = 0;
    Nst_OpErr err = { nullptr, nullptr };

    t_Vec2 = nst_type_new("Vec2", 4, &err);
    func_list_[idx++] = NST_MAKE_NAMED_OBJDECLR(t_Vec2, "Vec2_");
    func_list_[idx++] = NST_MAKE_FUNCDECLR(vec2_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(vec2_to_str_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(x_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(y_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(len_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dot_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(cross_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(normalized_, 1);

#if __LINE__ - FUNC_COUNT != 18
#error
#endif

    lib_init_ = err.name == nullptr;
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    nst_dec_ref(t_Vec2);
}

Nst_Obj *vec2_new(f64 x, f64 y, Nst_OpErr *err)
{
    Vec2Obj *vec2 = nst_obj_alloc(Vec2Obj, t_Vec2, nullptr, err);
    if ( vec2 == nullptr )
    {
        return nullptr;
    }

    vec2->x = x;
    vec2->y = y;

    return OBJ(vec2);
}

NST_FUNC_SIGN(vec2_)
{
    Nst_Real x, y;
    NST_DEF_EXTRACT("NN", &x, &y);
    return vec2_new(x, y, err);
}

NST_FUNC_SIGN(vec2_to_str_)
{
    Vec2Obj *vec2;
    NST_DEF_EXTRACT("#", t_Vec2, &vec2);
    return nst_sprintf("<Vec2 x=%lg y=%lg>", vec2->x, vec2->y);
}

NST_FUNC_SIGN(x_)
{
    Vec2Obj *vec2;
    NST_DEF_EXTRACT("#", t_Vec2, &vec2);
    return nst_real_new(vec2->x, err);
}

NST_FUNC_SIGN(y_)
{
    Vec2Obj *vec2;
    NST_DEF_EXTRACT("#", t_Vec2, &vec2);
    return nst_real_new(vec2->y, err);
}

NST_FUNC_SIGN(len_)
{
    Vec2Obj *vec2;
    NST_DEF_EXTRACT("#", t_Vec2, &vec2);
    Nst_Real x = vec2->x;
    Nst_Real y = vec2->y;
    return nst_real_new(sqrt(x*x + y*y), err);
}

NST_FUNC_SIGN(dot_)
{
    Vec2Obj *u;
    Vec2Obj *v;
    NST_DEF_EXTRACT("##", t_Vec2, &u, t_Vec2, &v);

    return nst_real_new(u->x * v->x + u->y * v->y, err);
}

NST_FUNC_SIGN(cross_)
{
    Vec2Obj *u;
    Vec2Obj *v;
    NST_DEF_EXTRACT("##", t_Vec2, &u, t_Vec2, &v);

    return nst_real_new(u->x * v->y - v->x * u->y, err);
}

NST_FUNC_SIGN(normalized_)
{
    Vec2Obj *vec2;
    NST_DEF_EXTRACT("#", t_Vec2, &vec2);
    Nst_Real x = vec2->x;
    Nst_Real y = vec2->y;
    f64 len = sqrt(x*x + y*y);
    return vec2_new(x / len, y / len, err);
}
