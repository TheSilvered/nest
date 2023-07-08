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

    func_list_[idx++] = NST_MAKE_FUNCDECLR(floor_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ceil_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(round_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(exp_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ln_,     1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(log_,    2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(divmod_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(sin_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(cos_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(tan_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(asin_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(acos_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(atan_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(atan2_,  2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(sinh_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(cosh_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(tanh_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(asinh_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(acosh_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(atanh_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dist_2d_,2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dist_3d_,2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dist_nd_,2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(deg_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rad_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(min_,    2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(max_,    2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(sum_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(frexp_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ldexp_,  2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(map_,    5);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(clamp_,  3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(gcd_,    2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(lcm_,    2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(abs_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(hypot_,  2);

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

NST_FUNC_SIGN(floor_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_int_new((Nst_Int)n);
}

NST_FUNC_SIGN(ceil_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_int_new((Nst_Int)ceil(n));
}

NST_FUNC_SIGN(round_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_int_new((Nst_Int)round(n));
}

NST_FUNC_SIGN(exp_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(exp(n));
}

NST_FUNC_SIGN(ln_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(log(n));
}

NST_FUNC_SIGN(log_)
{
    Nst_Real n;
    Nst_Obj *base_obj;
    NST_DEF_EXTRACT("N?N", &n, &base_obj);

    if ( base_obj == nst_null() )
    {
        return nst_real_new(log10(n));
    }
    Nst_Real base = AS_REAL(base_obj);

    if ( base == 2.0 )
    {
        return nst_real_new(log2(n));
    }
    else
    {
        return nst_real_new(log(n) / log(base));
    }
}

NST_FUNC_SIGN(divmod_)
{
    Nst_Int x;
    Nst_Int y;

    NST_DEF_EXTRACT("ii", &x, &y);

    return nst_array_create_c("II", x / y, x % y);
}

NST_FUNC_SIGN(sin_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(sin(n));
}

NST_FUNC_SIGN(cos_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(cos(n));
}

NST_FUNC_SIGN(tan_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(tan(n));
}

NST_FUNC_SIGN(asin_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(asin(n));
}

NST_FUNC_SIGN(acos_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(acos(n));
}

NST_FUNC_SIGN(atan_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(atan(n));
}

NST_FUNC_SIGN(atan2_)
{
    Nst_Real x;
    Nst_Real y;
    NST_DEF_EXTRACT("NN", &y, &x);
    return nst_real_new(atan2(y, x));
}

NST_FUNC_SIGN(sinh_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(sinh(n));
}

NST_FUNC_SIGN(cosh_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(cosh(n));
}

NST_FUNC_SIGN(tanh_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(tanh(n));
}

NST_FUNC_SIGN(asinh_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(asinh(n));
}

NST_FUNC_SIGN(acosh_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(acosh(n));
}

NST_FUNC_SIGN(atanh_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);
    return nst_real_new(atanh(n));
}

NST_FUNC_SIGN(dist_2d_)
{
    Nst_SeqObj *p1;
    Nst_SeqObj *p2;

    NST_DEF_EXTRACT("A.i|rA.i|r", &p1, &p2);

    if ( p1->len != 2 || p2->len != 2 )
    {
        Nst_set_value_error_c("the points must have exactly two values");
        return nullptr;
    }

    Nst_Real x1;
    Nst_Real y1;
    Nst_Real x2;
    Nst_Real y2;

    Nst_Obj *p1x_obj = p1->objs[0];
    Nst_Obj *p1y_obj = p1->objs[1];
    Nst_Obj *p2x_obj = p2->objs[0];
    Nst_Obj *p2y_obj = p2->objs[1];

    Nst_TypeObj *int_type = nst_type()->Int;

    if ( p1x_obj->type == int_type )
    {
        x1 = (Nst_Real)AS_INT(p1x_obj);
    }
    else
    {
        x1 = AS_REAL(p1x_obj);
    }

    if ( p1y_obj->type == int_type )
    {
        y1 = (Nst_Real)AS_INT(p1y_obj);
    }
    else
    {
        y1 = AS_REAL(p1y_obj);
    }

    if ( p2x_obj->type == int_type  )
    {
        x2 = (Nst_Real)AS_INT(p2x_obj);
    }
    else
    {
        x2 = AS_REAL(p2x_obj);
    }

    if ( p2y_obj->type == int_type  )
    {
        y2 = (Nst_Real)AS_INT(p2y_obj);
    }
    else
    {
        y2 = AS_REAL(p2y_obj);
    }

    // sqrt((x1 - x2)^2 + (y1 - y2)^2)
    Nst_Real c2 = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);
    return nst_real_new(sqrt(c2));
}

NST_FUNC_SIGN(dist_3d_)
{
    Nst_SeqObj *p1;
    Nst_SeqObj *p2;

    NST_DEF_EXTRACT("A.i|rA.i|r", &p1, &p2);

    if ( p1->len != 3 || p2->len != 3 )
    {
        Nst_set_value_error_c("the points must have exactly three values");
        return nullptr;
    }

    Nst_Real x1;
    Nst_Real y1;
    Nst_Real z1;
    Nst_Real x2;
    Nst_Real y2;
    Nst_Real z2;

    Nst_Obj *p1x_obj = p1->objs[0];
    Nst_Obj *p1y_obj = p1->objs[1];
    Nst_Obj *p1z_obj = p1->objs[2];

    Nst_Obj *p2x_obj = p2->objs[0];
    Nst_Obj *p2y_obj = p2->objs[1];
    Nst_Obj *p2z_obj = p2->objs[2];

    Nst_TypeObj *int_type = nst_type()->Int;

    if ( p1x_obj->type == int_type )
    {
        x1 = (Nst_Real)AS_INT(p1x_obj);
    }
    else
    {
        x1 = AS_REAL(p1x_obj);
    }

    if ( p1y_obj->type == int_type )
    {
        y1 = (Nst_Real)AS_INT(p1y_obj);
    }
    else
    {
        y1 = AS_REAL(p1y_obj);
    }

    if ( p1z_obj->type == int_type )
    {
        z1 = (Nst_Real)AS_INT(p1z_obj);
    }
    else
    {
        z1 = AS_REAL(p1z_obj);
    }

    if ( p2x_obj->type == int_type )
    {
        x2 = (Nst_Real)AS_INT(p2x_obj);
    }
    else
    {
        x2 = AS_REAL(p2x_obj);
    }

    if ( p2y_obj->type == int_type )
    {
        y2 = (Nst_Real)AS_INT(p2y_obj);
    }
    else
    {
        y2 = AS_REAL(p2y_obj);
    }

    if ( p2z_obj->type == int_type )
    {
        z2 = (Nst_Real)AS_INT(p2z_obj);
    }
    else
    {
        z2 = AS_REAL(p2z_obj);
    }

    // sqrt((x1 - x2)^2 + (y1 - y2)^2 + (z1 - z2)^2)
    Nst_Real d2 = (x1 - x2)*(x1 - x2)
                + (y1 - y2)*(y1 - y2)
                + (z1 - z2)*(z1 - z2);
    return nst_real_new(sqrt(d2));
}

NST_FUNC_SIGN(dist_nd_)
{
    Nst_SeqObj *p1;
    Nst_SeqObj *p2;

    NST_DEF_EXTRACT("A.i|rA.i|r", &p1, &p2);

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
    Nst_Real t1 = 0;
    Nst_Real t2 = 0;
    Nst_Real tot = 0;

    for ( usize i = 0; i < len; i++ )
    {
        obj1 = objs1[i];
        obj2 = objs2[i];

        if ( obj1->type == nst_type()->Int )
        {
            t1 = (Nst_Real)AS_INT(obj1);
        }
        else
        {
            t1 = AS_REAL(obj1);
        }

        if ( obj2->type == nst_type()->Int  )
        {
            t2 = (Nst_Real)AS_INT(obj2);
        }
        else
        {
            t2 = AS_REAL(obj2);
        }

        tot += (t1 - t2) * (t1 - t2);
    }

    return nst_real_new(sqrt(tot));
}

NST_FUNC_SIGN(deg_)
{
    Nst_Real n;

    NST_DEF_EXTRACT("N", &n);
    // 57.29577951308232 = 180 / PI
    return nst_real_new(n * 57.29577951308232);
}

NST_FUNC_SIGN(rad_)
{
    Nst_Real n;

    NST_DEF_EXTRACT("N", &n);
    // 0.017453292519943295 = PI / 180
    return nst_real_new(n * 0.017453292519943295);
}

NST_FUNC_SIGN(min_)
{
    Nst_Obj *ob_a;
    Nst_Obj *ob_b;
    NST_DEF_EXTRACT("oo", &ob_a, &ob_b);

    Nst_Obj *res;
    if ( ob_b == nst_null() &&
        (ob_a->type == nst_type()->Array || ob_a->type == nst_type()->Vector))
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
            res = nst_obj_lt(seq->objs[i], min_obj);
            if ( res == nullptr )
            {
                return nullptr;
            }

            if ( res == nst_true() )
            {
                min_obj = seq->objs[i];
            }
            nst_dec_ref(res);
        }

        return nst_inc_ref(min_obj);
    }

    res = nst_obj_lt(ob_b, ob_a);
    if ( res == nullptr )
    {
        return nullptr;
    }

    if ( res == nst_true() )
    {
        nst_dec_ref(res);
        return nst_inc_ref(ob_b);
    }
    nst_dec_ref(res);
    return nst_inc_ref(ob_a);
}

NST_FUNC_SIGN(max_)
{
    Nst_Obj *ob_a;
    Nst_Obj *ob_b;
    NST_DEF_EXTRACT("oo", &ob_a, &ob_b);

    Nst_Obj *res;
    if ( ob_b == nst_null() &&
        (ob_a->type == nst_type()->Array || ob_a->type == nst_type()->Vector))
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
            res = nst_obj_gt(seq->objs[i], max_obj);
            if ( res == nullptr )
            {
                return nullptr;
            }

            if ( res == nst_true() )
            {
                max_obj = seq->objs[i];
            }
            nst_dec_ref(res);
        }

        return nst_inc_ref(max_obj);
    }

    res = nst_obj_gt(ob_b, ob_a);
    if ( res == nullptr )
    {
        return nullptr;
    }

    if ( res == nst_true() )
    {
        nst_dec_ref(res);
        return nst_inc_ref(ob_b);
    }
    nst_dec_ref(res);
    return nst_inc_ref(ob_a);
}

NST_FUNC_SIGN(sum_)
{
    Nst_SeqObj *seq;

    NST_DEF_EXTRACT("A", &seq);

    if ( seq->len == 0 )
    {
        NST_RETURN_ZERO;
    }

    Nst_Obj *tot = nst_inc_ref(nst_const()->Byte_0);
    Nst_Obj *new_tot = nullptr;

    for (usize i = 0, n = seq->len; i < n; i++)
    {
        new_tot = nst_obj_add(tot, seq->objs[i]);
        nst_dec_ref(tot);

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

NST_FUNC_SIGN(frexp_)
{
    Nst_Real n;

    NST_DEF_EXTRACT("r", &n);

    int num;
    Nst_SeqObj *arr = SEQ(nst_array_new(2));
    arr->objs[0] = nst_real_new(frexp(n, &num));
    arr->objs[1] = nst_int_new(num);

    return OBJ(arr);
}

NST_FUNC_SIGN(ldexp_)
{
    Nst_Real m;
    Nst_Int e;

    NST_DEF_EXTRACT("ri", &m, &e);

    return nst_real_new(ldexp(m, (i32)e));
}

NST_FUNC_SIGN(map_)
{
    Nst_Real val;
    Nst_Real min1;
    Nst_Real max1;
    Nst_Real min2;
    Nst_Real max2;

    NST_DEF_EXTRACT("NNNNN", &val, &min1, &max1, &min2, &max2);

    if ( min1 == max1 )
    {
        return nst_real_new(min2);
    }

    return nst_real_new((val - min1) / (max1 - min1) * (max2 - min2) + min2);
}

NST_FUNC_SIGN(clamp_)
{
    Nst_Real val;
    Nst_Real min;
    Nst_Real max;

    NST_DEF_EXTRACT("NNN", &val, &min, &max);

    return nst_real_new(min > val ? min : max < val ? max : val);
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

static inline Nst_Real gcd_real(Nst_Real a, Nst_Real b)
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

Nst_Obj *gcd_or_lcm_seq(Nst_SeqObj *seq, NST_FUNC_SIGN((*func)))
{
    if ( seq->len == 0 )
    {
        NST_RETURN_ZERO;
    }

    Nst_Obj **objs = seq->objs;
    Nst_Obj *prev = nst_inc_ref(objs[0]);
    Nst_Obj *curr = nullptr;
    Nst_Obj *gcd_args[2] = { prev, nullptr };

    for ( usize i = 1, n = seq->len; i < n; i++ )
    {
        gcd_args[0] = prev;
        gcd_args[1] = objs[i];
        curr = func(2, gcd_args);
        nst_dec_ref(prev);

        if ( curr == nullptr )
        {
            return nullptr;
        }

        prev = curr;
    }

    return prev;
}

NST_FUNC_SIGN(gcd_)
{
    Nst_Obj *ob1;
    Nst_Obj *ob2;

    NST_DEF_EXTRACT("i|r|B|A.i|r|Bi|r|B|n", &ob1, &ob2);

    Nst_TypeObj *type_real = nst_type()->Real;
    Nst_TypeObj *type_int  = nst_type()->Int;

    if ( (ob1->type == nst_type()->Array || ob1->type == nst_type()->Vector) )
    {
        if ( ob2 == nst_null() )
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

    if ( ob2 == nst_null() )
    {
        Nst_set_type_error(Nst_sprintf(
            "the two objects must a sequence and null or two numbers, got '%s' and '%s'",
            TYPE_NAME(ob1), TYPE_NAME(ob2)
        ));
        return nullptr;
    }

    if ( ob1->type == type_real || ob2->type == type_real )
    {
        Nst_Real n1, n2;
        if ( ob1->type == type_real )
            n1 = AS_REAL(ob1);
        else if ( ob1->type == type_int )
            n1 = (Nst_Real)AS_INT(ob1);
        else
            n1 = (Nst_Real)AS_BYTE(ob1);

        if ( ob2->type == type_real )
            n2 = AS_REAL(ob2);
        else if ( ob2->type == type_int )
            n2 = (Nst_Real)AS_INT(ob2);
        else
            n2 = (Nst_Real)AS_BYTE(ob2);

        return nst_real_new(gcd_real(n1, n2));
    }
    else if ( ob1->type == type_int || ob2->type == type_int )
    {
        Nst_Int n1_int;
        Nst_Int n2_int;

        if ( ob1->type == type_int )
        {
            n1_int = AS_INT(ob1);
        }
        else
        {
            n1_int = (Nst_Int)AS_BYTE(ob1);
        }

        if ( ob2->type == type_int )
        {
            n2_int = AS_INT(ob2);
        }
        else
        {
            n2_int = (Nst_Int)AS_BYTE(ob2);
        }

        return nst_int_new(gcd_int(n1_int, n2_int));
    }
    else
    {
        Nst_Byte n1_byte = AS_BYTE(ob1);
        Nst_Byte n2_byte = AS_BYTE(ob2);
        return nst_byte_new(gcd_int(n1_byte, n2_byte));
    }
}

NST_FUNC_SIGN(lcm_)
{
    Nst_Obj *ob1;
    Nst_Obj *ob2;

    NST_DEF_EXTRACT("i|r|B|A.i|r|Bi|r|B|n", &ob1, &ob2);

    Nst_TypeObj *type_real = nst_type()->Real;
    Nst_TypeObj *type_int  = nst_type()->Int;
    Nst_TypeObj *type_byte = nst_type()->Byte;

    if ( (ob1->type == nst_type()->Array || ob1->type == nst_type()->Vector) )
    {
        if ( ob2 == nst_null() )
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

    if ( ob2 == nst_null() )
    {
        Nst_set_type_error(Nst_sprintf(
            "the two objects must a sequence and null or two numbers, got '%s' and '%s'",
            TYPE_NAME(ob1), TYPE_NAME(ob2)
        ));
        return nullptr;
    }

    Nst_Obj *numerator = nst_obj_mul(ob1, ob2);

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

    Nst_Obj *result = nst_obj_div(numerator, denominator);
    nst_dec_ref(numerator);
    nst_dec_ref(denominator);

    return result;
}

NST_FUNC_SIGN(abs_)
{
    Nst_Real n;
    NST_DEF_EXTRACT("N", &n);

    if ( n >= 0 )
    {
        return nst_inc_ref(args[0]);
    }
    return nst_obj_mul(args[0], nst_const()->Int_neg1);
}

NST_FUNC_SIGN(hypot_)
{
    Nst_Real c1;
    Nst_Real c2;

    NST_DEF_EXTRACT("NN", &c1, &c2);

    return nst_real_new(sqrt(c1 * c1 + c2 * c2));
}
