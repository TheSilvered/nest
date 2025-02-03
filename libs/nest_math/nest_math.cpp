#include <cmath>
#include "nest_math.h"

using std::fmin;
using std::fmax;
using std::isnan;
using std::isinf;

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(floor_,  1),
    Nst_FUNCDECLR(ceil_,   1),
    Nst_FUNCDECLR(round_,  1),
    Nst_FUNCDECLR(exp_,    1),
    Nst_FUNCDECLR(ln_,     1),
    Nst_FUNCDECLR(log_,    2),
    Nst_FUNCDECLR(divmod_, 2),
    Nst_FUNCDECLR(sin_,    1),
    Nst_FUNCDECLR(cos_,    1),
    Nst_FUNCDECLR(tan_,    1),
    Nst_FUNCDECLR(asin_,   1),
    Nst_FUNCDECLR(acos_,   1),
    Nst_FUNCDECLR(atan_,   1),
    Nst_FUNCDECLR(atan2_,  2),
    Nst_FUNCDECLR(sinh_,   1),
    Nst_FUNCDECLR(cosh_,   1),
    Nst_FUNCDECLR(tanh_,   1),
    Nst_FUNCDECLR(asinh_,  1),
    Nst_FUNCDECLR(acosh_,  1),
    Nst_FUNCDECLR(atanh_,  1),
    Nst_FUNCDECLR(dist_2d_,2),
    Nst_FUNCDECLR(dist_3d_,2),
    Nst_FUNCDECLR(dist_nd_,2),
    Nst_FUNCDECLR(deg_,    1),
    Nst_FUNCDECLR(rad_,    1),
    Nst_FUNCDECLR(min_,    2),
    Nst_FUNCDECLR(max_,    2),
    Nst_FUNCDECLR(sum_,    1),
    Nst_FUNCDECLR(frexp_,  1),
    Nst_FUNCDECLR(ldexp_,  2),
    Nst_FUNCDECLR(map_,    5),
    Nst_FUNCDECLR(clamp_,  3),
    Nst_FUNCDECLR(gcd_,    2),
    Nst_FUNCDECLR(lcm_,    2),
    Nst_FUNCDECLR(abs_,    1),
    Nst_FUNCDECLR(hypot_,  2),
    Nst_FUNCDECLR(is_nan_, 1),
    Nst_FUNCDECLR(is_inf_, 1),
    Nst_CONSTDECLR(INF_),
    Nst_CONSTDECLR(NAN_),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    return obj_list_;
}

Nst_Obj *NstC floor_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_int_new((i64)n);
}

Nst_Obj *NstC ceil_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_int_new((i64)ceil(n));
}

Nst_Obj *NstC round_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_int_new((i64)round(n));
}

Nst_Obj *NstC exp_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(exp(n));
}

Nst_Obj *NstC ln_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(log(n));
}

Nst_Obj *NstC log_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    Nst_Obj *base_obj;
    if (!Nst_extract_args("N ?N", arg_num, args, &n, &base_obj))
        return nullptr;

    if (base_obj == Nst_null())
        return Nst_real_new(log10(n));
    f64 base = AS_REAL(base_obj);

    if (base == 2.0)
        return Nst_real_new(log2(n));
    else
        return Nst_real_new(log(n) / log(base));
}

Nst_Obj *NstC divmod_(usize arg_num, Nst_Obj **args)
{
    i64 x;
    i64 y;

    if (!Nst_extract_args("i i", arg_num, args, &x, &y))
        return nullptr;

    return Nst_array_create_c("II", x / y, x % y);
}

Nst_Obj *NstC sin_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(sin(n));
}

Nst_Obj *NstC cos_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(cos(n));
}

Nst_Obj *NstC tan_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(tan(n));
}

Nst_Obj *NstC asin_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(asin(n));
}

Nst_Obj *NstC acos_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(acos(n));
}

Nst_Obj *NstC atan_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(atan(n));
}

Nst_Obj *NstC atan2_(usize arg_num, Nst_Obj **args)
{
    f64 x;
    f64 y;
    if (!Nst_extract_args("N N", arg_num, args, &y, &x))
        return nullptr;
    return Nst_real_new(atan2(y, x));
}

Nst_Obj *NstC sinh_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(sinh(n));
}

Nst_Obj *NstC cosh_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(cosh(n));
}

Nst_Obj *NstC tanh_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(tanh(n));
}

Nst_Obj *NstC asinh_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(asinh(n));
}

Nst_Obj *NstC acosh_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(acosh(n));
}

Nst_Obj *NstC atanh_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    return Nst_real_new(atanh(n));
}

Nst_Obj *NstC dist_2d_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *p1;
    Nst_Obj *p2;

    if (!Nst_extract_args("A.N A.N", arg_num, args, &p1, &p2))
        return nullptr;

    if (Nst_seq_len(p1) != 2 || Nst_seq_len(p2) != 2) {
        Nst_set_value_error_c("the points must have exactly two values");
        return nullptr;
    }

    f64 x1 = Nst_number_to_f64(Nst_seq_getnf(p1, 0));
    f64 y1 = Nst_number_to_f64(Nst_seq_getnf(p1, 1));
    f64 x2 = Nst_number_to_f64(Nst_seq_getnf(p2, 0));
    f64 y2 = Nst_number_to_f64(Nst_seq_getnf(p2, 1));

    // sqrt((x1 - x2)^2 + (y1 - y2)^2)
    f64 c2 = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);
    return Nst_real_new(sqrt(c2));
}

Nst_Obj *NstC dist_3d_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *p1;
    Nst_Obj *p2;

    if (!Nst_extract_args("A.N A.N", arg_num, args, &p1, &p2))
        return nullptr;

    if (Nst_seq_len(p1) != 3 || Nst_seq_len(p2) != 3) {
        Nst_set_value_error_c("the points must have exactly three values");
        return nullptr;
    }

    f64 x1 = Nst_number_to_f64(Nst_seq_getnf(p1, 0));
    f64 y1 = Nst_number_to_f64(Nst_seq_getnf(p1, 1));
    f64 z1 = Nst_number_to_f64(Nst_seq_getnf(p1, 2));
    f64 x2 = Nst_number_to_f64(Nst_seq_getnf(p2, 0));
    f64 y2 = Nst_number_to_f64(Nst_seq_getnf(p2, 1));
    f64 z2 = Nst_number_to_f64(Nst_seq_getnf(p2, 2));

    // sqrt((x1 - x2)^2 + (y1 - y2)^2 + (z1 - z2)^2)
    f64 d2 = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) + (z1 - z2)*(z1 - z2);
    return Nst_real_new(sqrt(d2));
}

Nst_Obj *NstC dist_nd_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *p1;
    Nst_Obj *p2;

    if (!Nst_extract_args("A.N A.N", arg_num, args, &p1, &p2))
        return nullptr;

    if (Nst_seq_len(p1) != Nst_seq_len(p2)) {
        Nst_set_value_error_c(
            "the points must have the same number of values");
        return nullptr;
    }

    Nst_Obj **objs1 = _Nst_seq_objs(p1);
    Nst_Obj **objs2 = _Nst_seq_objs(p2);

    f64 tot = 0;

    for (usize i = 0, len = Nst_seq_len(p1); i < len; i++) {
        f64 t1 = Nst_number_to_f64(objs1[i]);
        f64 t2 = Nst_number_to_f64(objs2[i]);

        tot += (t1 - t2) * (t1 - t2);
    }

    return Nst_real_new(sqrt(tot));
}

Nst_Obj *NstC deg_(usize arg_num, Nst_Obj **args)
{
    f64 n;

    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    // 57.29577951308232 ~ 180 / PI
    return Nst_real_new(n * 57.29577951308232);
}

Nst_Obj *NstC rad_(usize arg_num, Nst_Obj **args)
{
    f64 n;

    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;
    // 0.017453292519943295 ~ PI / 180
    return Nst_real_new(n * 0.017453292519943295);
}

Nst_Obj *NstC min_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *ob_a;
    Nst_Obj *ob_b;
    if (!Nst_extract_args("o o", arg_num, args, &ob_a, &ob_b))
        return nullptr;

    Nst_Obj *res;
    if (ob_b == Nst_null() && (Nst_T(ob_a, Array) || Nst_T(ob_a, Vector))) {
        usize seq_len = Nst_seq_len(ob_a);

        if (seq_len == 0) {
            Nst_set_value_error_c("sequence length is zero");
            return nullptr;
        }

        Nst_Obj *min_obj = Nst_seq_getnf(ob_a, 0);

        for (usize i = 1; i < seq_len; i++) {
            Nst_Obj *curr_obj = Nst_seq_getnf(ob_a, i);
            res = Nst_obj_lt(curr_obj, min_obj);
            if (res == nullptr)
                return nullptr;

            if (res == Nst_true())
                min_obj = curr_obj;
            Nst_dec_ref(res);
        }

        return Nst_inc_ref(min_obj);
    }

    res = Nst_obj_lt(ob_b, ob_a);
    if (res == nullptr)
        return nullptr;

    if (res == Nst_true()) {
        Nst_dec_ref(res);
        return Nst_inc_ref(ob_b);
    }
    Nst_dec_ref(res);
    return Nst_inc_ref(ob_a);
}

Nst_Obj *NstC max_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *ob_a;
    Nst_Obj *ob_b;
    if (!Nst_extract_args("o o", arg_num, args, &ob_a, &ob_b))
        return nullptr;

    Nst_Obj *res;
    if (ob_b == Nst_null() && (Nst_T(ob_a, Array) || Nst_T(ob_a, Vector))) {
        usize seq_len = Nst_seq_len(ob_a);

        if (seq_len == 0) {
            Nst_set_value_error_c("sequence length is zero");
            return nullptr;
        }

        Nst_Obj *max_obj = Nst_seq_getnf(ob_a, 0);

        for (usize i = 1; i < seq_len; i++) {
            Nst_Obj *curr_obj = Nst_seq_getnf(ob_a, i);
            res = Nst_obj_gt(curr_obj, max_obj);
            if (res == nullptr)
                return nullptr;

            if (res == Nst_true())
                max_obj = curr_obj;
            Nst_dec_ref(res);
        }

        return Nst_inc_ref(max_obj);
    }

    res = Nst_obj_gt(ob_b, ob_a);
    if (res == nullptr)
        return nullptr;

    if (res == Nst_true()) {
        Nst_dec_ref(res);
        return Nst_inc_ref(ob_b);
    }
    Nst_dec_ref(res);
    return Nst_inc_ref(ob_a);
}

Nst_Obj *NstC sum_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;

    if (!Nst_extract_args("A", arg_num, args, &seq))
        return nullptr;

    if (Nst_seq_len(seq) == 0)
        Nst_RETURN_ZERO;

    Nst_Obj *tot = Nst_inc_ref(Nst_const()->Byte_0);
    Nst_Obj *new_tot = nullptr;

    for (usize i = 0, n = Nst_seq_len(seq); i < n; i++) {
        new_tot = Nst_obj_add(tot, Nst_seq_getnf(seq, i));
        Nst_dec_ref(tot);

        if (new_tot == nullptr)
            return nullptr;
        else
            tot = new_tot;
    }

    return tot;
}

Nst_Obj *NstC frexp_(usize arg_num, Nst_Obj **args)
{
    f64 n;

    if (!Nst_extract_args("r", arg_num, args, &n))
        return nullptr;

    int num;
    f64 exp = frexp(n, &num);

    return Nst_array_create_c("fi", exp, (i32)num);
}

Nst_Obj *NstC ldexp_(usize arg_num, Nst_Obj **args)
{
    f64 m;
    i64 e;

    if (!Nst_extract_args("ri", arg_num, args, &m, &e))
        return nullptr;

    return Nst_real_new(ldexp(m, (i32)e));
}

Nst_Obj *NstC map_(usize arg_num, Nst_Obj **args)
{
    f64 val;
    f64 min1;
    f64 max1;
    f64 min2;
    f64 max2;

    if (!Nst_extract_args(
            "N N N N N",
            arg_num, args,
            &val, &min1, &max1, &min2, &max2))
    {
        return nullptr;
    }

    if (min1 == max1)
        return Nst_real_new(min2);

    return Nst_real_new((val - min1) / (max1 - min1) * (max2 - min2) + min2);
}

Nst_Obj *NstC clamp_(usize arg_num, Nst_Obj **args)
{
    f64 val;
    f64 min_val;
    f64 max_val;

    if (!Nst_extract_args("N N N", arg_num, args, &val, &min_val, &max_val))
        return nullptr;

    return Nst_real_new(fmax(fmin(val, max_val), min_val));
}

static inline i64 gcd_int(i64 a, i64 b)
{
    if (a == 0)
        return b;
    else if (b == 0)
        return a;

    if (a < 0)
        a *= -1;
    if (b < 0)
        b *= -1;

    i64 res = 1;
    while (true) {
        if (a == b)
            return res * a;
        else if (!(a & 1) && !(b & 1)) {
            a >>= 1;
            b >>= 1;
            res <<= 1;
        } else if (!(a & 1))
            a >>= 1;
        else if (!(b & 1))
            b >>= 1;
        else if (a > b)
            a -= b;
        else
            b -= a;
    }
}

static inline f64 gcd_real(f64 a, f64 b)
{
    if (a == 0.0)
        return b;
    else if (b == 0.0)
        return a;

    if (a < 0.0)
        a *= -1.0;
    if (b < 0.0)
        b *= -1.0;

    while (true) {
        if (a == b)
            return a;
        else if (a > b)
            a -= b;
        else
            b -= a;
    }
}

Nst_Obj *gcd_or_lcm_seq(Nst_Obj *seq, Nst_NestCallable func)
{
    if (Nst_seq_len(seq) == 0)
        Nst_RETURN_ZERO;

    Nst_Obj **objs = _Nst_seq_objs(seq);
    Nst_Obj *prev = Nst_inc_ref(objs[0]);
    Nst_Obj *curr = nullptr;
    Nst_Obj *gcd_args[2] = { prev, nullptr };

    for (usize i = 1, n = Nst_seq_len(seq); i < n; i++) {
        gcd_args[0] = prev;
        gcd_args[1] = objs[i];
        curr = func(2, gcd_args);
        Nst_dec_ref(prev);

        if (curr == nullptr)
            return nullptr;

        prev = curr;
    }

    return prev;
}

Nst_Obj *NstC gcd_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *ob1;
    Nst_Obj *ob2;

    if (!Nst_extract_args("i|r|B|A.N i|r|B|n", arg_num, args, &ob1, &ob2))
        return nullptr;

    if (Nst_T(ob1, Array) || Nst_T(ob1, Vector)) {
        if (ob2 == Nst_null())
            return gcd_or_lcm_seq(ob1, gcd_);
        else {
            Nst_set_type_errorf(
                "the two objects must a sequence and null or two numbers,"
                " got '%s' and '%s'",
                TYPE_NAME(ob1), TYPE_NAME(ob2));
            return nullptr;
        }
    }

    if (ob2 == Nst_null()) {
        Nst_set_type_errorf(
            "the two objects must a sequence and null or two numbers,"
            " got '%s' and '%s'",
            TYPE_NAME(ob1), TYPE_NAME(ob2));
        return nullptr;
    }

    if (Nst_T(ob1, Real) || Nst_T(ob2, Real)) {
        f64 n1 = Nst_number_to_f64(ob1);
        f64 n2 = Nst_number_to_f64(ob2);
        return Nst_real_new(gcd_real(n1, n2));
    } else if (Nst_T(ob1, Int) || Nst_T(ob2, Int)) {
        i64 n1_int = Nst_number_to_i64(ob1);
        i64 n2_int = Nst_number_to_i64(ob2);
        return Nst_int_new(gcd_int(n1_int, n2_int));
    } else {
        u8 n1_byte = AS_BYTE(ob1);
        u8 n2_byte = AS_BYTE(ob2);
        return Nst_byte_new((u8)gcd_int((i64)n1_byte, (i64)n2_byte));
    }
}

Nst_Obj *NstC lcm_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *ob1;
    Nst_Obj *ob2;

    if (!Nst_extract_args("i|r|B|A.N i|r|B|n", arg_num, args, &ob1, &ob2))
        return nullptr;

    if (Nst_T(ob1, Array) || Nst_T(ob1, Vector)) {
        if (ob2 == Nst_null())
            return gcd_or_lcm_seq(ob1, lcm_);
        else {
            Nst_set_type_errorf(
                "the two objects must a sequence and null or two numbers,"
                " got '%s' and '%s'",
                TYPE_NAME(ob1), TYPE_NAME(ob2));
            return nullptr;
        }
    }

    if (ob2 == Nst_null()) {
        Nst_set_type_errorf(
            "the two objects must a sequence and null or two numbers,"
            " got '%s' and '%s'",
            TYPE_NAME(ob1), TYPE_NAME(ob2));
        return nullptr;
    }

    Nst_Obj *numerator = Nst_obj_mul(ob1, ob2);

    if (Nst_number_to_f64(numerator) == 0)
        return numerator;

    Nst_Obj *denominator = gcd_(2, args);
    if (denominator == nullptr)
        return nullptr;

    Nst_Obj *result = Nst_obj_div(numerator, denominator);
    Nst_dec_ref(numerator);
    Nst_dec_ref(denominator);

    return result;
}

Nst_Obj *NstC abs_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("N", arg_num, args, &n))
        return nullptr;

    if (n >= 0)
        return Nst_inc_ref(args[0]);
    return Nst_obj_mul(args[0], Nst_const()->Int_neg1);
}

Nst_Obj *NstC hypot_(usize arg_num, Nst_Obj **args)
{
    f64 c1;
    f64 c2;

    if (!Nst_extract_args("N N", arg_num, args, &c1, &c2))
        return nullptr;

    return Nst_real_new(sqrt(c1 * c1 + c2 * c2));
}

Nst_Obj *NstC is_nan_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("r", arg_num, args, &n))
        return nullptr;
    Nst_RETURN_BOOL(isnan(n));
}

Nst_Obj *NstC is_inf_(usize arg_num, Nst_Obj **args)
{
    f64 n;
    if (!Nst_extract_args("r", arg_num, args, &n))
        return nullptr;
    Nst_RETURN_BOOL(isinf(n));
}

Nst_Obj *NstC INF_()
{
    return Nst_inc_ref(Nst_const()->Real_inf);
}

Nst_Obj *NstC NAN_()
{
    return Nst_inc_ref(Nst_const()->Real_nan);
}
