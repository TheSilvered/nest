#include "nest_math.h"
#include <cmath>

#define FUNC_COUNT 27
#define COORD_TYPE_ERROR do { \
        SET_VALUE_ERROR("all coordinates must be of type 'Real' or 'Int'"); \
        return nullptr; \
    } while (0)

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(floor_,  1, "floor");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(ceil_,   1, "ceil");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(round_,  1, "round");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(exp_,    1, "exp");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(log_,    1, "log");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(logn_,   2, "logn");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(log2_,   1, "log2");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(log10_,  1, "log10");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(divmod_, 2, "divmod");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(sin_,    1, "sin");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(cos_,    1, "cos");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(tan_,    1, "tan");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(asin_,   1, "asin");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(acos_,   1, "acos");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(atan_,   1, "atan");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(atan2_,  2, "atan2");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(sinh_,   1, "sinh");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(cosh_,   1, "cosh");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(tanh_,   1, "tanh");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(asinh_,  1, "asinh");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(acosh_,  1, "acosh");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(atanh_,  1, "atanh");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(dist_2d_,2, "dist_2d");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(dist_3d_,2, "dist_3d");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(dist_nd_,2, "dist_nd");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(deg_,    1, "deg");
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(rad_,    1, "rad");

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

Nst_Obj *floor_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_int_obj((Nst_int)n);
}

Nst_Obj *ceil_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_int_obj((Nst_int)ceil(n));
}

Nst_Obj *round_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_int_obj((Nst_int)round(n));
}

Nst_Obj *exp_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(exp(n));
}

Nst_Obj *log_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(log(n));
}

Nst_Obj *logn_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;
    Nst_real base;

    if ( !extract_arg_values("NN", arg_num, args, err, &n, &base) )
        return nullptr;

    return new_real_obj(log(n) / log(base));
}

Nst_Obj *log2_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(log2(n));
}

Nst_Obj *log10_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(log10(n));
}

Nst_Obj *divmod_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_int x;
    Nst_int y;

    if ( !extract_arg_values("ii", arg_num, args, err, &x, &y) )
        return nullptr;

    Nst_sequence *res = new_array_empty(2);
    res->objs[0] = new_int_obj(x / y);
    res->objs[1] = new_int_obj(x % y);

    return new_arr_obj(res);
}

Nst_Obj *sin_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(sin(n));
}

Nst_Obj *cos_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(cos(n));
}

Nst_Obj *tan_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(tan(n));
}

Nst_Obj *asin_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(asin(n));
}

Nst_Obj *acos_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(acos(n));
}

Nst_Obj *atan_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(atan(n));
}

Nst_Obj *atan2_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real x;
    Nst_real y;

    if ( !extract_arg_values("NN", arg_num, args, err, &y, &x) )
        return nullptr;

    return new_real_obj(atan2(y, x));
}

Nst_Obj *sinh_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(sinh(n));
}

Nst_Obj *cosh_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(cosh(n));
}

Nst_Obj *tanh_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(tanh(n));
}

Nst_Obj *asinh_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(asinh(n));
}

Nst_Obj *acosh_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(acosh(n));
}

Nst_Obj *atanh_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return new_real_obj(atanh(n));
}

Nst_Obj *dist_2d_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_sequence *p1;
    Nst_sequence *p2;

    if ( !extract_arg_values("AA", arg_num, args, err, &p1, &p2) )
        return nullptr;

    if ( p1->len != 2 || p2->len != 2 )
    {
        SET_VALUE_ERROR("the points must have exactly two values");
        return nullptr;
    }

    Nst_real x1;
    Nst_real y1;
    Nst_real x2;
    Nst_real y2;

    Nst_Obj *p1x_obj = p1->objs[0];
    Nst_Obj *p1y_obj = p1->objs[1];
    Nst_Obj *p2x_obj = p2->objs[0];
    Nst_Obj *p2y_obj = p2->objs[1];

    if      ( p1x_obj->type == nst_t_int  ) x1 = (Nst_real)AS_INT(p1x_obj);
    else if ( p1x_obj->type == nst_t_real ) x1 = AS_REAL(p1x_obj);
    else COORD_TYPE_ERROR;

    if      ( p1y_obj->type == nst_t_int  ) y1 = (Nst_real)AS_INT(p1y_obj);
    else if ( p1y_obj->type == nst_t_real ) y1 = AS_REAL(p1y_obj);
    else COORD_TYPE_ERROR;

    if      ( p2x_obj->type == nst_t_int  ) x2 = (Nst_real)AS_INT(p2x_obj);
    else if ( p2x_obj->type == nst_t_real ) x2 = AS_REAL(p2x_obj);
    else COORD_TYPE_ERROR;

    if      ( p2y_obj->type == nst_t_int  ) y2 = (Nst_real)AS_INT(p2y_obj);
    else if ( p2y_obj->type == nst_t_real ) y2 = AS_REAL(p2y_obj);
    else COORD_TYPE_ERROR;

    // sqrt((x1 - x2)^2 + (y1 - y2)^2)
    Nst_real c2 = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);
    return new_real_obj(sqrt(c2));
}

Nst_Obj *dist_3d_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_sequence *p1;
    Nst_sequence *p2;

    if ( !extract_arg_values("AA", arg_num, args, err, &p1, &p2) )
        return nullptr;

    if ( p1->len != 3 || p2->len != 3 )
    {
        SET_VALUE_ERROR("the points must have exactly three values");
        return nullptr;
    }

    Nst_real x1;
    Nst_real y1;
    Nst_real z1;
    Nst_real x2;
    Nst_real y2;
    Nst_real z2;

    Nst_Obj *p1x_obj = p1->objs[0];
    Nst_Obj *p1y_obj = p1->objs[1];
    Nst_Obj *p1z_obj = p1->objs[2];

    Nst_Obj *p2x_obj = p2->objs[0];
    Nst_Obj *p2y_obj = p2->objs[1];
    Nst_Obj *p2z_obj = p2->objs[2];

    if      ( p1x_obj->type == nst_t_int  ) x1 = (Nst_real)AS_INT(p1x_obj);
    else if ( p1x_obj->type == nst_t_real ) x1 = AS_REAL(p1x_obj);
    else COORD_TYPE_ERROR;

    if      ( p1y_obj->type == nst_t_int  ) y1 = (Nst_real)AS_INT(p1y_obj);
    else if ( p1y_obj->type == nst_t_real ) y1 = AS_REAL(p1y_obj);
    else COORD_TYPE_ERROR;

    if      ( p1z_obj->type == nst_t_int  ) z1 = (Nst_real)AS_INT(p1z_obj);
    else if ( p1z_obj->type == nst_t_real ) z1 = AS_REAL(p1z_obj);
    else COORD_TYPE_ERROR;

    if      ( p2x_obj->type == nst_t_int  ) x2 = (Nst_real)AS_INT(p2x_obj);
    else if ( p2x_obj->type == nst_t_real ) x2 = AS_REAL(p2x_obj);
    else COORD_TYPE_ERROR;

    if      ( p2y_obj->type == nst_t_int  ) y2 = (Nst_real)AS_INT(p2y_obj);
    else if ( p2y_obj->type == nst_t_real ) y2 = AS_REAL(p2y_obj);
    else COORD_TYPE_ERROR;

    if      ( p2z_obj->type == nst_t_int  ) z2 = (Nst_real)AS_INT(p2z_obj);
    else if ( p2z_obj->type == nst_t_real ) z2 = AS_REAL(p2z_obj);
    else COORD_TYPE_ERROR;

    // sqrt((x1 - x2)^2 + (y1 - y2)^2 + (z1 - z2)^2)
    Nst_real d2 = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) + (z1 - z2)*(z1 - z2);
    return new_real_obj(sqrt(d2));
}

Nst_Obj *dist_nd_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_sequence *p1;
    Nst_sequence *p2;

    if ( !extract_arg_values("AA", arg_num, args, err, &p1, &p2) )
        return nullptr;

    if ( p1->len != p2->len )
    {
        SET_VALUE_ERROR("the points must have the same number of values");
        return nullptr;
    }

    Nst_Obj **objs1 = p1->objs;
    Nst_Obj **objs2 = p2->objs;
    size_t len = p1->len;

    Nst_Obj *obj1;
    Nst_Obj *obj2;
    Nst_real t1 = 0;
    Nst_real t2 = 0;
    Nst_real tot = 0;

    for ( size_t i = 0; i < len; i++ )
    {
        obj1 = objs1[i];
        obj2 = objs2[i];

        if      ( obj1->type == nst_t_int  ) t1 = (Nst_real)AS_INT(obj1);
        else if ( obj1->type == nst_t_real ) t1 = AS_REAL(obj1);
        else COORD_TYPE_ERROR;

        if      ( obj2->type == nst_t_int  ) t2 = (Nst_real)AS_INT(obj2);
        else if ( obj2->type == nst_t_real ) t2 = AS_REAL(obj2);
        else COORD_TYPE_ERROR;

        tot += (t1 - t2) * (t1 - t2);
    }

    return new_real_obj(sqrt(tot));
}

Nst_Obj *deg_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;
    // 57.29577951308232 = 180 / PI
    return new_real_obj(n * 57.29577951308232);
}

Nst_Obj *rad_(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real n;

    if ( !extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;
    // 0.017453292519943295 = PI / 180
    return new_real_obj(n * 0.017453292519943295);
}
