#include <cmath>
#include "nest_math.h"

#define FUNC_COUNT 42
#define COORD_TYPE_ERROR do { \
        NST_SET_RAW_VALUE_ERROR("all coordinates must be of type 'Real' or 'Int'"); \
        return nullptr; \
    } while (0)

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(floor_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ceil_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(round_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(exp_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(log_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(logn_,   2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(log2_,   1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(log10_,  1);
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
    func_list_[idx++] = NST_MAKE_FUNCDECLR(min_seq_,1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(max_seq_,1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(sum_seq_,1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(frexp_,  1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(ldexp_,  2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(map_,    5);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(clamp_,  3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(gcd_,    2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(lcm_,    2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(gcd_seq_,1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(lcm_seq_,1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(abs_,    1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(hypot_,  2);

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

NST_FUNC_SIGN(floor_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_int((Nst_Int)n);
}

NST_FUNC_SIGN(ceil_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_int((Nst_Int)ceil(n));
}

NST_FUNC_SIGN(round_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_int((Nst_Int)round(n));
}

NST_FUNC_SIGN(exp_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(exp(n));
}

NST_FUNC_SIGN(log_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(log(n));
}

NST_FUNC_SIGN(logn_)
{
    Nst_Real n;
    Nst_Real base;

    if ( !nst_extract_arg_values("NN", arg_num, args, err, &n, &base) )
        return nullptr;

    return nst_new_real(log(n) / log(base));
}

NST_FUNC_SIGN(log2_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(log2(n));
}

NST_FUNC_SIGN(log10_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(log10(n));
}

NST_FUNC_SIGN(divmod_)
{
    Nst_Int x;
    Nst_Int y;

    if ( !nst_extract_arg_values("ii", arg_num, args, err, &x, &y) )
        return nullptr;

    Nst_SeqObj *res = SEQ(nst_new_array(2));
    res->objs[0] = nst_new_int(x / y);
    res->objs[1] = nst_new_int(x % y);

    return OBJ(res);
}

NST_FUNC_SIGN(sin_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(sin(n));
}

NST_FUNC_SIGN(cos_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(cos(n));
}

NST_FUNC_SIGN(tan_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(tan(n));
}

NST_FUNC_SIGN(asin_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(asin(n));
}

NST_FUNC_SIGN(acos_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(acos(n));
}

NST_FUNC_SIGN(atan_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(atan(n));
}

NST_FUNC_SIGN(atan2_)
{
    Nst_Real x;
    Nst_Real y;

    if ( !nst_extract_arg_values("NN", arg_num, args, err, &y, &x) )
        return nullptr;

    return nst_new_real(atan2(y, x));
}

NST_FUNC_SIGN(sinh_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(sinh(n));
}

NST_FUNC_SIGN(cosh_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(cosh(n));
}

NST_FUNC_SIGN(tanh_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(tanh(n));
}

NST_FUNC_SIGN(asinh_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(asinh(n));
}

NST_FUNC_SIGN(acosh_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(acosh(n));
}

NST_FUNC_SIGN(atanh_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    return nst_new_real(atanh(n));
}

NST_FUNC_SIGN(dist_2d_)
{
    Nst_SeqObj *p1;
    Nst_SeqObj *p2;

    if ( !nst_extract_arg_values("AA", arg_num, args, err, &p1, &p2) )
        return nullptr;

    if ( p1->len != 2 || p2->len != 2 )
    {
        NST_SET_RAW_VALUE_ERROR("the points must have exactly two values");
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

    if      ( p1x_obj->type == nst_t.Int  ) x1 = (Nst_Real)AS_INT(p1x_obj);
    else if ( p1x_obj->type == nst_t.Real ) x1 = AS_REAL(p1x_obj);
    else COORD_TYPE_ERROR;

    if      ( p1y_obj->type == nst_t.Int  ) y1 = (Nst_Real)AS_INT(p1y_obj);
    else if ( p1y_obj->type == nst_t.Real ) y1 = AS_REAL(p1y_obj);
    else COORD_TYPE_ERROR;

    if      ( p2x_obj->type == nst_t.Int  ) x2 = (Nst_Real)AS_INT(p2x_obj);
    else if ( p2x_obj->type == nst_t.Real ) x2 = AS_REAL(p2x_obj);
    else COORD_TYPE_ERROR;

    if      ( p2y_obj->type == nst_t.Int  ) y2 = (Nst_Real)AS_INT(p2y_obj);
    else if ( p2y_obj->type == nst_t.Real ) y2 = AS_REAL(p2y_obj);
    else COORD_TYPE_ERROR;

    // sqrt((x1 - x2)^2 + (y1 - y2)^2)
    Nst_Real c2 = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);
    return nst_new_real(sqrt(c2));
}

NST_FUNC_SIGN(dist_3d_)
{
    Nst_SeqObj *p1;
    Nst_SeqObj *p2;

    if ( !nst_extract_arg_values("AA", arg_num, args, err, &p1, &p2) )
        return nullptr;

    if ( p1->len != 3 || p2->len != 3 )
    {
        NST_SET_RAW_VALUE_ERROR("the points must have exactly three values");
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

    if      ( p1x_obj->type == nst_t.Int  ) x1 = (Nst_Real)AS_INT(p1x_obj);
    else if ( p1x_obj->type == nst_t.Real ) x1 = AS_REAL(p1x_obj);
    else COORD_TYPE_ERROR;

    if      ( p1y_obj->type == nst_t.Int  ) y1 = (Nst_Real)AS_INT(p1y_obj);
    else if ( p1y_obj->type == nst_t.Real ) y1 = AS_REAL(p1y_obj);
    else COORD_TYPE_ERROR;

    if      ( p1z_obj->type == nst_t.Int  ) z1 = (Nst_Real)AS_INT(p1z_obj);
    else if ( p1z_obj->type == nst_t.Real ) z1 = AS_REAL(p1z_obj);
    else COORD_TYPE_ERROR;

    if      ( p2x_obj->type == nst_t.Int  ) x2 = (Nst_Real)AS_INT(p2x_obj);
    else if ( p2x_obj->type == nst_t.Real ) x2 = AS_REAL(p2x_obj);
    else COORD_TYPE_ERROR;

    if      ( p2y_obj->type == nst_t.Int  ) y2 = (Nst_Real)AS_INT(p2y_obj);
    else if ( p2y_obj->type == nst_t.Real ) y2 = AS_REAL(p2y_obj);
    else COORD_TYPE_ERROR;

    if      ( p2z_obj->type == nst_t.Int  ) z2 = (Nst_Real)AS_INT(p2z_obj);
    else if ( p2z_obj->type == nst_t.Real ) z2 = AS_REAL(p2z_obj);
    else COORD_TYPE_ERROR;

    // sqrt((x1 - x2)^2 + (y1 - y2)^2 + (z1 - z2)^2)
    Nst_Real d2 = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) + (z1 - z2)*(z1 - z2);
    return nst_new_real(sqrt(d2));
}

NST_FUNC_SIGN(dist_nd_)
{
    Nst_SeqObj *p1;
    Nst_SeqObj *p2;

    if ( !nst_extract_arg_values("AA", arg_num, args, err, &p1, &p2) )
        return nullptr;

    if ( p1->len != p2->len )
    {
        NST_SET_RAW_VALUE_ERROR("the points must have the same number of values");
        return nullptr;
    }

    Nst_Obj **objs1 = p1->objs;
    Nst_Obj **objs2 = p2->objs;
    size_t len = p1->len;

    Nst_Obj *obj1;
    Nst_Obj *obj2;
    Nst_Real t1 = 0;
    Nst_Real t2 = 0;
    Nst_Real tot = 0;

    for ( size_t i = 0; i < len; i++ )
    {
        obj1 = objs1[i];
        obj2 = objs2[i];

        if      ( obj1->type == nst_t.Int  ) t1 = (Nst_Real)AS_INT(obj1);
        else if ( obj1->type == nst_t.Real ) t1 = AS_REAL(obj1);
        else COORD_TYPE_ERROR;

        if      ( obj2->type == nst_t.Int  ) t2 = (Nst_Real)AS_INT(obj2);
        else if ( obj2->type == nst_t.Real ) t2 = AS_REAL(obj2);
        else COORD_TYPE_ERROR;

        tot += (t1 - t2) * (t1 - t2);
    }

    return nst_new_real(sqrt(tot));
}

NST_FUNC_SIGN(deg_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;
    // 57.29577951308232 = 180 / PI
    return nst_new_real(n * 57.29577951308232);
}

NST_FUNC_SIGN(rad_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;
    // 0.017453292519943295 = PI / 180
    return nst_new_real(n * 0.017453292519943295);
}

NST_FUNC_SIGN(min_)
{
    Nst_Real a;
    Nst_Real b;

    if ( !nst_extract_arg_values("NN", arg_num, args, err, &a, &b))
        return nullptr;

    if ( b < a )
        return nst_inc_ref(args[1]);
    return nst_inc_ref(args[0]);
}

NST_FUNC_SIGN(max_)
{
    Nst_Real a;
    Nst_Real b;

    if ( !nst_extract_arg_values("NN", arg_num, args, err, &a, &b))
        return nullptr;

    if ( b > a )
        return nst_inc_ref(args[1]);
    return nst_inc_ref(args[0]);
}

NST_FUNC_SIGN(min_seq_)
{
    Nst_SeqObj *seq;

    if ( !nst_extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    size_t seq_len = seq->len;

    if ( seq_len == 0 )
    {
        NST_SET_RAW_VALUE_ERROR("sequence length is zero");
        return nullptr;
    }

    Nst_Obj *min_obj = seq->objs[0];

    for ( size_t i = 1; i < seq_len; i++)
    {
        Nst_Obj *res = nst_obj_lt(seq->objs[i], min_obj, err);
        if ( res == nullptr )
            return nullptr;

        if ( res == nst_c.b_true )
            min_obj = seq->objs[i];
        nst_dec_ref(res);
    }

    return nst_inc_ref(min_obj);
}

NST_FUNC_SIGN(max_seq_)
{
    Nst_SeqObj *seq;

    if ( !nst_extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    size_t seq_len = seq->len;

    if ( seq_len == 0 )
    {
        NST_SET_RAW_VALUE_ERROR("the sequence has a length of zero");
        return nullptr;
    }

    Nst_Obj *max_obj = seq->objs[0];

    for ( size_t i = 1; i < seq_len; i++ )
    {
        Nst_Obj *res = nst_obj_gt(seq->objs[i], max_obj, err);
        if ( res == nullptr )
            return nullptr;

        if ( res == nst_c.b_true )
            max_obj = seq->objs[i];
        nst_dec_ref(res);
    }

    return nst_inc_ref(max_obj);
}

NST_FUNC_SIGN(sum_seq_)
{
    Nst_SeqObj *seq;

    if ( !nst_extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    if ( seq->len == 0 )
        NST_RETURN_ZERO;

    Nst_Obj *tot = nst_inc_ref(nst_c.Byte_0);
    Nst_Obj *new_tot = nullptr;

    for (size_t i = 0, n = seq->len; i < n; i++)
    {
        new_tot = nst_obj_add(tot, seq->objs[i], err);
        nst_dec_ref(tot);

        if ( new_tot == nullptr )
            return nullptr;
        else
            tot = new_tot;
    }

    return tot;
}

NST_FUNC_SIGN(frexp_)
{
    Nst_Real n;

    if ( !nst_extract_arg_values("r", arg_num, args, err, &n) )
        return nullptr;

    int num;
    Nst_SeqObj *arr = SEQ(nst_new_array(2));
    arr->objs[0] = nst_new_real(frexp(n, &num));
    arr->objs[1] = nst_new_int(num);

    return OBJ(arr);
}

NST_FUNC_SIGN(ldexp_)
{
    Nst_Real m;
    Nst_Int e;

    if ( !nst_extract_arg_values("ri", arg_num, args, err, &m, &e) )
        return nullptr;

    return nst_new_real(ldexp(m, (int)e));
}

NST_FUNC_SIGN(map_)
{
    Nst_Real val;
    Nst_Real min1;
    Nst_Real max1;
    Nst_Real min2;
    Nst_Real max2;

    if ( !nst_extract_arg_values("NNNNN", arg_num, args, err, &val, &min1, &max1, &min2, &max2) )
        return nullptr;

    if ( min1 == max1 )
        return nst_new_real(min2);

    return nst_new_real((val - min1) / (max1 - min1) * (max2 - min2) + min2);
}

NST_FUNC_SIGN(clamp_)
{
    Nst_Real val;
    Nst_Real min;
    Nst_Real max;

    if ( !nst_extract_arg_values("NNN", arg_num, args, err, &val, &min, &max) )
        return nullptr;

    return nst_new_real(min > val ? min : max < val ? max : val);
}

template <typename T>

static inline T gcd_int(T a, T b)
{
    if ( a == 0 )
        return b;
    else if ( b == 0 )
        return a;

    if ( a < 0 )
        a *= -1;
    if ( b < 0 )
        b *= -1;

    T res = 1;
    while ( true )
    {
        if ( a == b )
            return res * a;
        else if ( !(a & 1) && !(b & 1) )
        {
            a >>= 1;
            b >>= 1;
            res <<= 1;
        }
        else if ( !(a & 1) )
            a >>= 1;
        else if ( !(b & 1) )
            b >>= 1;
        else if ( a > b )
            a -= b;
        else
            b -= a;
    }
}

static inline Nst_Real gcd_real(Nst_Real a, Nst_Real b)
{
    if ( a == 0.0 )
        return b;
    else if ( b == 0.0 )
        return a;

    if ( a < 0.0 )
        a *= -1.0;
    if ( b < 0.0 )
        b *= -1.0;

    while ( true )
    {
        if ( a == b )
            return a;
        else if ( a > b )
            a -= b;
        else
            b -= a;
    }
}

NST_FUNC_SIGN(gcd_)
{
    Nst_Real n1;
    Nst_Real n2;

    if ( !nst_extract_arg_values("NN", arg_num, args, err, &n1, &n2) )
        return nullptr;

    if ( args[0]->type == nst_t.Real || args[1]->type == nst_t.Real )
        return nst_new_real(gcd_real(n1, n2));
    else if ( args[0]->type == nst_t.Int || args[1]->type == nst_t.Int )
    {
        Nst_Int n1_int;
        Nst_Int n2_int;

        if ( args[0]->type == nst_t.Int )
            n1_int = AS_INT(args[0]);
        else
            n1_int = (Nst_Int)AS_BYTE(args[0]);

        if ( args[1]->type == nst_t.Int )
            n2_int = AS_INT(args[1]);
        else
            n2_int = (Nst_Int)AS_BYTE(args[1]);

        return nst_new_int(gcd_int<Nst_Int>(n1_int, n2_int));
    }
    else
    {
        Nst_Byte n1_byte = AS_BYTE(args[0]);
        Nst_Byte n2_byte = AS_BYTE(args[1]);
        return nst_new_byte(gcd_int<Nst_Byte>(n1_byte, n2_byte));
    }
}

NST_FUNC_SIGN(lcm_)
{
    Nst_Real n1;
    Nst_Real n2;

    if ( !nst_extract_arg_values("NN", arg_num, args, err, &n1, &n2) )
        return nullptr;

    Nst_Obj *numerator = nst_obj_mul(args[0], args[1], nullptr);

    if ( (numerator->type == nst_t.Int  && AS_INT(numerator)  == 0)   ||
         (numerator->type == nst_t.Real && AS_REAL(numerator) == 0.0) ||
         (numerator->type == nst_t.Byte && AS_BYTE(numerator) == 0) )
        return numerator;

    Nst_Obj *denominator = gcd_(2, args, err);
    if ( denominator == nullptr )
        return nullptr;

    Nst_Obj *result = nst_obj_div(numerator, denominator, err);
    nst_dec_ref(numerator);
    nst_dec_ref(denominator);

    return result;
}

NST_FUNC_SIGN(gcd_seq_)
{
    Nst_SeqObj *seq;
    if ( !nst_extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    if ( seq->len == 0 )
        NST_RETURN_ZERO;

    Nst_Obj **objs = seq->objs;

    if ( objs[0]->type != nst_t.Int &&
         objs[0]->type != nst_t.Real &&
         objs[0]->type != nst_t.Byte )
    {
        NST_SET_TYPE_ERROR(_nst_format_error(
            "expected type 'Byte', 'Int' or 'Real', but object at index %zi is of type '%s'",
            "us",
            0, TYPE_NAME(objs[0])
        ));
        return nullptr;
    }

    Nst_Obj *prev = nst_inc_ref(objs[0]);
    Nst_Obj *curr = nullptr;
    Nst_Obj *gcd_args[2] = { prev, nullptr };

    for ( size_t i = 1, n = seq->len; i < n; i++ )
    {
        if ( objs[i]->type != nst_t.Int &&
             objs[i]->type != nst_t.Real &&
             objs[i]->type != nst_t.Byte )
        {
            NST_SET_TYPE_ERROR(_nst_format_error(
                "expected type 'Byte', 'Int' or 'Real', but object at index %zi is of type '%s'",
                "us",
                i, TYPE_NAME(objs[i])
            ));
            nst_dec_ref(prev);
            return nullptr;
        }
        gcd_args[0] = prev;
        gcd_args[1] = objs[i];
        curr = gcd_(2, gcd_args, err);
        nst_dec_ref(prev);

        if ( curr == nullptr )
            return nullptr;

        prev = curr;
    }

    return prev;
}

NST_FUNC_SIGN(lcm_seq_)
{
    Nst_SeqObj *seq;
    if ( !nst_extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    if ( seq->len == 0 )
        NST_RETURN_ZERO;

    Nst_Obj **objs = seq->objs;

    if ( objs[0]->type != nst_t.Int &&
        objs[0]->type != nst_t.Real &&
        objs[0]->type != nst_t.Byte )
    {
        NST_SET_TYPE_ERROR(_nst_format_error(
            "expected type 'Byte', 'Int' or 'Real', but object at index %zi is of type '%s'",
            "us",
            0, TYPE_NAME(objs[0])
        ));
        return nullptr;
    }

    Nst_Obj *prev = nst_inc_ref(objs[0]);
    Nst_Obj *curr = nullptr;
    Nst_Obj *lcm_args[2] = { prev, nullptr };

    for ( size_t i = 1, n = seq->len; i < n; i++ )
    {
        if ( objs[i]->type != nst_t.Int &&
            objs[i]->type != nst_t.Real &&
            objs[i]->type != nst_t.Byte )
        {
            NST_SET_TYPE_ERROR(_nst_format_error(
                "expected type 'Byte', 'Int' or 'Real', but object at index %zi is of type '%s'",
                "us",
                i, TYPE_NAME(objs[i])
            ));
            nst_dec_ref(prev);
            return nullptr;
        }
        lcm_args[0] = prev;
        lcm_args[1] = objs[i];
        curr = lcm_(2, lcm_args, err);
        nst_dec_ref(prev);

        if ( curr == nullptr )
            return nullptr;

        prev = curr;
    }

    return prev;
}

NST_FUNC_SIGN(abs_)
{
    Nst_Real n;
    if ( !nst_extract_arg_values("N", arg_num, args, err, &n) )
        return nullptr;

    if ( n >= 0 )
        return nst_inc_ref(args[0]);
    return nst_obj_mul(args[0], nst_c.Int_neg1, err);
}

NST_FUNC_SIGN(hypot_)
{
    Nst_Real c1;
    Nst_Real c2;

    if ( !nst_extract_arg_values("NN", arg_num, args, err, &c1, &c2) )
        return nullptr;

    return nst_new_real(sqrt(c1 * c1 + c2 * c2));
}
