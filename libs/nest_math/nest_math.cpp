#include <cmath>
#include "nest_math.h"

#define FUNC_COUNT 36
#define COORD_TYPE_ERROR do { \
        Nst_set_value_error_c( \
            "all coordinates must be of type 'Real' or 'Int'"); \
        return nullptr; \
    } while (0)

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(floor_,  1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(ceil_,   1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(round_,  1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(exp_,    1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(ln_,     1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(log_,    2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(divmod_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(sin_,    1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(cos_,    1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(tan_,    1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(asin_,   1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(acos_,   1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(atan_,   1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(atan2_,  2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(sinh_,   1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(cosh_,   1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(tanh_,   1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(asinh_,  1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(acosh_,  1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(atanh_,  1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(dist_2d_,2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(dist_3d_,2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(dist_nd_,2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(deg_,    1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(rad_,    1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(min_,    2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(max_,    2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(sum_,    1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(frexp_,  1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(ldexp_,  2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(map_,    5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(clamp_,  3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(gcd_,    2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(lcm_,    2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(abs_,    1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(hypot_,  2);

#if __LINE__ - FUNC_COUNT != 20
#error
#endif

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

Nst_FUNC_SIGN(floor_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_int_new((i64)n);
}

Nst_FUNC_SIGN(ceil_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_int_new((i64)ceil(n));
}

Nst_FUNC_SIGN(round_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_int_new((i64)round(n));
}

Nst_FUNC_SIGN(exp_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(exp(n));
}

Nst_FUNC_SIGN(ln_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(log(n));
}

Nst_FUNC_SIGN(log_)
{
    f64 n;
    Nst_Obj *base_obj;
    Nst_DEF_EXTRACT("N?N", &n, &base_obj);

    if ( base_obj == Nst_null() )
    {
        return Nst_real_new(log10(n));
    }
    f64 base = AS_REAL(base_obj);

    if ( base == 2.0 )
    {
        return Nst_real_new(log2(n));
    }
    else
    {
        return Nst_real_new(log(n) / log(base));
    }
}

Nst_FUNC_SIGN(divmod_)
{
    i64 x;
    i64 y;

    Nst_DEF_EXTRACT("ii", &x, &y);

    return Nst_array_create_c("II", x / y, x % y);
}

Nst_FUNC_SIGN(sin_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(sin(n));
}

Nst_FUNC_SIGN(cos_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(cos(n));
}

Nst_FUNC_SIGN(tan_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(tan(n));
}

Nst_FUNC_SIGN(asin_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(asin(n));
}

Nst_FUNC_SIGN(acos_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(acos(n));
}

Nst_FUNC_SIGN(atan_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(atan(n));
}

Nst_FUNC_SIGN(atan2_)
{
    f64 x;
    f64 y;
    Nst_DEF_EXTRACT("NN", &y, &x);
    return Nst_real_new(atan2(y, x));
}

Nst_FUNC_SIGN(sinh_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(sinh(n));
}

Nst_FUNC_SIGN(cosh_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(cosh(n));
}

Nst_FUNC_SIGN(tanh_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(tanh(n));
}

Nst_FUNC_SIGN(asinh_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(asinh(n));
}

Nst_FUNC_SIGN(acosh_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(acosh(n));
}

Nst_FUNC_SIGN(atanh_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);
    return Nst_real_new(atanh(n));
}

Nst_FUNC_SIGN(dist_2d_)
{
    Nst_SeqObj *p1;
    Nst_SeqObj *p2;

    Nst_DEF_EXTRACT("A.i|rA.i|r", &p1, &p2);

    if ( p1->len != 2 || p2->len != 2 )
    {
        Nst_set_value_error_c("the points must have exactly two values");
        return nullptr;
    }

    f64 x1;
    f64 y1;
    f64 x2;
    f64 y2;

    Nst_Obj *p1x_obj = p1->objs[0];
    Nst_Obj *p1y_obj = p1->objs[1];
    Nst_Obj *p2x_obj = p2->objs[0];
    Nst_Obj *p2y_obj = p2->objs[1];

    Nst_TypeObj *int_type = Nst_type()->Int;

    if ( p1x_obj->type == int_type )
    {
        x1 = (f64)AS_INT(p1x_obj);
    }
    else
    {
        x1 = AS_REAL(p1x_obj);
    }

    if ( p1y_obj->type == int_type )
    {
        y1 = (f64)AS_INT(p1y_obj);
    }
    else
    {
        y1 = AS_REAL(p1y_obj);
    }

    if ( p2x_obj->type == int_type  )
    {
        x2 = (f64)AS_INT(p2x_obj);
    }
    else
    {
        x2 = AS_REAL(p2x_obj);
    }

    if ( p2y_obj->type == int_type  )
    {
        y2 = (f64)AS_INT(p2y_obj);
    }
    else
    {
        y2 = AS_REAL(p2y_obj);
    }

    // sqrt((x1 - x2)^2 + (y1 - y2)^2)
    f64 c2 = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);
    return Nst_real_new(sqrt(c2));
}

Nst_FUNC_SIGN(dist_3d_)
{
    Nst_SeqObj *p1;
    Nst_SeqObj *p2;

    Nst_DEF_EXTRACT("A.i|rA.i|r", &p1, &p2);

    if ( p1->len != 3 || p2->len != 3 )
    {
        Nst_set_value_error_c("the points must have exactly three values");
        return nullptr;
    }

    f64 x1;
    f64 y1;
    f64 z1;
    f64 x2;
    f64 y2;
    f64 z2;

    Nst_Obj *p1x_obj = p1->objs[0];
    Nst_Obj *p1y_obj = p1->objs[1];
    Nst_Obj *p1z_obj = p1->objs[2];

    Nst_Obj *p2x_obj = p2->objs[0];
    Nst_Obj *p2y_obj = p2->objs[1];
    Nst_Obj *p2z_obj = p2->objs[2];

    Nst_TypeObj *int_type = Nst_type()->Int;

    if ( p1x_obj->type == int_type )
    {
        x1 = (f64)AS_INT(p1x_obj);
    }
    else
    {
        x1 = AS_REAL(p1x_obj);
    }

    if ( p1y_obj->type == int_type )
    {
        y1 = (f64)AS_INT(p1y_obj);
    }
    else
    {
        y1 = AS_REAL(p1y_obj);
    }

    if ( p1z_obj->type == int_type )
    {
        z1 = (f64)AS_INT(p1z_obj);
    }
    else
    {
        z1 = AS_REAL(p1z_obj);
    }

    if ( p2x_obj->type == int_type )
    {
        x2 = (f64)AS_INT(p2x_obj);
    }
    else
    {
        x2 = AS_REAL(p2x_obj);
    }

    if ( p2y_obj->type == int_type )
    {
        y2 = (f64)AS_INT(p2y_obj);
    }
    else
    {
        y2 = AS_REAL(p2y_obj);
    }

    if ( p2z_obj->type == int_type )
    {
        z2 = (f64)AS_INT(p2z_obj);
    }
    else
    {
        z2 = AS_REAL(p2z_obj);
    }

    // sqrt((x1 - x2)^2 + (y1 - y2)^2 + (z1 - z2)^2)
    f64 d2 = (x1 - x2)*(x1 - x2)
                + (y1 - y2)*(y1 - y2)
                + (z1 - z2)*(z1 - z2);
    return Nst_real_new(sqrt(d2));
}

Nst_FUNC_SIGN(dist_nd_)
{
    Nst_SeqObj *p1;
    Nst_SeqObj *p2;

    Nst_DEF_EXTRACT("A.i|rA.i|r", &p1, &p2);

    if ( p1->len != p2->len )
    {
        Nst_set_value_error_c(
            "the points must have the same number of values");
        return nullptr;
    }

    Nst_Obj **objs1 = p1->objs;
    Nst_Obj **objs2 = p2->objs;
    usize len = p1->len;

    Nst_Obj *obj1;
    Nst_Obj *obj2;
    f64 t1 = 0;
    f64 t2 = 0;
    f64 tot = 0;

    for ( usize i = 0; i < len; i++ )
    {
        obj1 = objs1[i];
        obj2 = objs2[i];

        if ( obj1->type == Nst_type()->Int )
        {
            t1 = (f64)AS_INT(obj1);
        }
        else
        {
            t1 = AS_REAL(obj1);
        }

        if ( obj2->type == Nst_type()->Int  )
        {
            t2 = (f64)AS_INT(obj2);
        }
        else
        {
            t2 = AS_REAL(obj2);
        }

        tot += (t1 - t2) * (t1 - t2);
    }

    return Nst_real_new(sqrt(tot));
}

Nst_FUNC_SIGN(deg_)
{
    f64 n;

    Nst_DEF_EXTRACT("N", &n);
    // 57.29577951308232 = 180 / PI
    return Nst_real_new(n * 57.29577951308232);
}

Nst_FUNC_SIGN(rad_)
{
    f64 n;

    Nst_DEF_EXTRACT("N", &n);
    // 0.017453292519943295 = PI / 180
    return Nst_real_new(n * 0.017453292519943295);
}

Nst_FUNC_SIGN(min_)
{
    Nst_Obj *ob_a;
    Nst_Obj *ob_b;
    Nst_DEF_EXTRACT("oo", &ob_a, &ob_b);

    Nst_Obj *res;
    if ( ob_b == Nst_null() &&
        (ob_a->type == Nst_type()->Array || ob_a->type == Nst_type()->Vector))
    {
        Nst_SeqObj *seq = SEQ(ob_a);
        usize seq_len = seq->len;

        if ( seq_len == 0 )
        {
            Nst_set_value_error_c("sequence length is zero");
            return nullptr;
        }

        Nst_Obj *min_obj = seq->objs[0];

        for ( usize i = 1; i < seq_len; i++)
        {
            res = Nst_obj_lt(seq->objs[i], min_obj);
            if ( res == nullptr )
            {
                return nullptr;
            }

            if ( res == Nst_true() )
            {
                min_obj = seq->objs[i];
            }
            Nst_dec_ref(res);
        }

        return Nst_inc_ref(min_obj);
    }

    res = Nst_obj_lt(ob_b, ob_a);
    if ( res == nullptr )
    {
        return nullptr;
    }

    if ( res == Nst_true() )
    {
        Nst_dec_ref(res);
        return Nst_inc_ref(ob_b);
    }
    Nst_dec_ref(res);
    return Nst_inc_ref(ob_a);
}

Nst_FUNC_SIGN(max_)
{
    Nst_Obj *ob_a;
    Nst_Obj *ob_b;
    Nst_DEF_EXTRACT("oo", &ob_a, &ob_b);

    Nst_Obj *res;
    if ( ob_b == Nst_null() &&
        (ob_a->type == Nst_type()->Array || ob_a->type == Nst_type()->Vector))
    {
        Nst_SeqObj *seq = SEQ(ob_a);
        usize seq_len = seq->len;

        if ( seq_len == 0 )
        {
            Nst_set_value_error_c("sequence length is zero");
            return nullptr;
        }

        Nst_Obj *max_obj = seq->objs[0];

        for ( usize i = 1; i < seq_len; i++)
        {
            res = Nst_obj_gt(seq->objs[i], max_obj);
            if ( res == nullptr )
            {
                return nullptr;
            }

            if ( res == Nst_true() )
            {
                max_obj = seq->objs[i];
            }
            Nst_dec_ref(res);
        }

        return Nst_inc_ref(max_obj);
    }

    res = Nst_obj_gt(ob_b, ob_a);
    if ( res == nullptr )
    {
        return nullptr;
    }

    if ( res == Nst_true() )
    {
        Nst_dec_ref(res);
        return Nst_inc_ref(ob_b);
    }
    Nst_dec_ref(res);
    return Nst_inc_ref(ob_a);
}

Nst_FUNC_SIGN(sum_)
{
    Nst_SeqObj *seq;

    Nst_DEF_EXTRACT("A", &seq);

    if ( seq->len == 0 )
    {
        Nst_RETURN_ZERO;
    }

    Nst_Obj *tot = Nst_inc_ref(Nst_const()->Byte_0);
    Nst_Obj *new_tot = nullptr;

    for (usize i = 0, n = seq->len; i < n; i++)
    {
        new_tot = Nst_obj_add(tot, seq->objs[i]);
        Nst_dec_ref(tot);

        if ( new_tot == nullptr )
        {
            return nullptr;
        }
        else
        {
            tot = new_tot;
        }
    }

    return tot;
}

Nst_FUNC_SIGN(frexp_)
{
    f64 n;

    Nst_DEF_EXTRACT("r", &n);

    int num;
    Nst_SeqObj *arr = SEQ(Nst_array_new(2));
    arr->objs[0] = Nst_real_new(frexp(n, &num));
    arr->objs[1] = Nst_int_new(num);

    return OBJ(arr);
}

Nst_FUNC_SIGN(ldexp_)
{
    f64 m;
    i64 e;

    Nst_DEF_EXTRACT("ri", &m, &e);

    return Nst_real_new(ldexp(m, (i32)e));
}

Nst_FUNC_SIGN(map_)
{
    f64 val;
    f64 min1;
    f64 max1;
    f64 min2;
    f64 max2;

    Nst_DEF_EXTRACT("NNNNN", &val, &min1, &max1, &min2, &max2);

    if ( min1 == max1 )
    {
        return Nst_real_new(min2);
    }

    return Nst_real_new((val - min1) / (max1 - min1) * (max2 - min2) + min2);
}

Nst_FUNC_SIGN(clamp_)
{
    f64 val;
    f64 min;
    f64 max;

    Nst_DEF_EXTRACT("NNN", &val, &min, &max);

    return Nst_real_new(min > val ? min : max < val ? max : val);
}

template <typename T>

static inline T gcd_int(T a, T b)
{
    if ( a == 0 )
    {
        return b;
    }
    else if ( b == 0 )
    {
        return a;
    }

    if ( a < 0 )
    {
        a *= -1;
    }
    if ( b < 0 )
    {
        b *= -1;
    }

    T res = 1;
    while ( true )
    {
        if ( a == b )
        {
            return res * a;
        }
        else if ( !(a & 1) && !(b & 1) )
        {
            a >>= 1;
            b >>= 1;
            res <<= 1;
        }
        else if ( !(a & 1) )
        {
            a >>= 1;
        }
        else if ( !(b & 1) )
        {
            b >>= 1;
        }
        else if ( a > b )
        {
            a -= b;
        }
        else
        {
            b -= a;
        }
    }
}

static inline f64 gcd_real(f64 a, f64 b)
{
    if ( a == 0.0 )
    {
        return b;
    }
    else if ( b == 0.0 )
    {
        return a;
    }

    if ( a < 0.0 )
    {
        a *= -1.0;
    }
    if ( b < 0.0 )
    {
        b *= -1.0;
    }

    while ( true )
    {
        if ( a == b )
        {
            return a;
        }
        else if ( a > b )
        {
            a -= b;
        }
        else
        {
            b -= a;
        }
    }
}

Nst_Obj *gcd_or_lcm_seq(Nst_SeqObj *seq, Nst_Obj *(*func)(usize arg_num, Nst_Obj **args))
{
    if ( seq->len == 0 )
    {
        Nst_RETURN_ZERO;
    }

    Nst_Obj **objs = seq->objs;
    Nst_Obj *prev = Nst_inc_ref(objs[0]);
    Nst_Obj *curr = nullptr;
    Nst_Obj *gcd_args[2] = { prev, nullptr };

    for ( usize i = 1, n = seq->len; i < n; i++ )
    {
        gcd_args[0] = prev;
        gcd_args[1] = objs[i];
        curr = func(2, gcd_args);
        Nst_dec_ref(prev);

        if ( curr == nullptr )
        {
            return nullptr;
        }

        prev = curr;
    }

    return prev;
}

Nst_FUNC_SIGN(gcd_)
{
    Nst_Obj *ob1;
    Nst_Obj *ob2;

    Nst_DEF_EXTRACT("i|r|B|A.i|r|Bi|r|B|n", &ob1, &ob2);

    Nst_TypeObj *type_real = Nst_type()->Real;
    Nst_TypeObj *type_int  = Nst_type()->Int;

    if ( (ob1->type == Nst_type()->Array || ob1->type == Nst_type()->Vector) )
    {
        if ( ob2 == Nst_null() )
        {
            return gcd_or_lcm_seq(SEQ(ob1), gcd_);
        }
        else
        {
            Nst_set_type_error(Nst_sprintf(
                "the two objects must a sequence and null or two numbers, got '%s' and '%s'",
                TYPE_NAME(ob1), TYPE_NAME(ob2)
            ));
            return nullptr;
        }
    }

    if ( ob2 == Nst_null() )
    {
        Nst_set_type_error(Nst_sprintf(
            "the two objects must a sequence and null or two numbers, got '%s' and '%s'",
            TYPE_NAME(ob1), TYPE_NAME(ob2)
        ));
        return nullptr;
    }

    if ( ob1->type == type_real || ob2->type == type_real )
    {
        f64 n1, n2;
        if ( ob1->type == type_real )
            n1 = AS_REAL(ob1);
        else if ( ob1->type == type_int )
            n1 = (f64)AS_INT(ob1);
        else
            n1 = (f64)AS_BYTE(ob1);

        if ( ob2->type == type_real )
            n2 = AS_REAL(ob2);
        else if ( ob2->type == type_int )
            n2 = (f64)AS_INT(ob2);
        else
            n2 = (f64)AS_BYTE(ob2);

        return Nst_real_new(gcd_real(n1, n2));
    }
    else if ( ob1->type == type_int || ob2->type == type_int )
    {
        i64 n1_int;
        i64 n2_int;

        if ( ob1->type == type_int )
        {
            n1_int = AS_INT(ob1);
        }
        else
        {
            n1_int = (i64)AS_BYTE(ob1);
        }

        if ( ob2->type == type_int )
        {
            n2_int = AS_INT(ob2);
        }
        else
        {
            n2_int = (i64)AS_BYTE(ob2);
        }

        return Nst_int_new(gcd_int(n1_int, n2_int));
    }
    else
    {
        u8 n1_byte = AS_BYTE(ob1);
        u8 n2_byte = AS_BYTE(ob2);
        return Nst_byte_new(gcd_int(n1_byte, n2_byte));
    }
}

Nst_FUNC_SIGN(lcm_)
{
    Nst_Obj *ob1;
    Nst_Obj *ob2;

    Nst_DEF_EXTRACT("i|r|B|A.i|r|Bi|r|B|n", &ob1, &ob2);

    Nst_TypeObj *type_real = Nst_type()->Real;
    Nst_TypeObj *type_int  = Nst_type()->Int;
    Nst_TypeObj *type_byte = Nst_type()->Byte;

    if ( (ob1->type == Nst_type()->Array || ob1->type == Nst_type()->Vector) )
    {
        if ( ob2 == Nst_null() )
        {
            return gcd_or_lcm_seq(SEQ(ob1), lcm_);
        }
        else
        {
            Nst_set_type_error(Nst_sprintf(
                "the two objects must a sequence and null or two numbers, got '%s' and '%s'",
                TYPE_NAME(ob1), TYPE_NAME(ob2)
            ));
            return nullptr;
        }
    }

    if ( ob2 == Nst_null() )
    {
        Nst_set_type_error(Nst_sprintf(
            "the two objects must a sequence and null or two numbers, got '%s' and '%s'",
            TYPE_NAME(ob1), TYPE_NAME(ob2)
        ));
        return nullptr;
    }

    Nst_Obj *numerator = Nst_obj_mul(ob1, ob2);

    if ( (numerator->type == type_int  && AS_INT(numerator)  == 0)   ||
         (numerator->type == type_real && AS_REAL(numerator) == 0.0) ||
         (numerator->type == type_byte && AS_BYTE(numerator) == 0) )
    {
        return numerator;
    }

    Nst_Obj *denominator = gcd_(2, args);
    if ( denominator == nullptr )
    {
        return nullptr;
    }

    Nst_Obj *result = Nst_obj_div(numerator, denominator);
    Nst_dec_ref(numerator);
    Nst_dec_ref(denominator);

    return result;
}

Nst_FUNC_SIGN(abs_)
{
    f64 n;
    Nst_DEF_EXTRACT("N", &n);

    if ( n >= 0 )
    {
        return Nst_inc_ref(args[0]);
    }
    return Nst_obj_mul(args[0], Nst_const()->Int_neg1);
}

Nst_FUNC_SIGN(hypot_)
{
    f64 c1;
    f64 c2;

    Nst_DEF_EXTRACT("NN", &c1, &c2);

    return Nst_real_new(sqrt(c1 * c1 + c2 * c2));
}
