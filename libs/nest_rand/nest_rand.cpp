#include <random>
#include <chrono>
#include <climits>
#include "nest_rand.h"

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(random_, 0),
    Nst_FUNCDECLR(rand_int_, 2),
    Nst_FUNCDECLR(rand_perc_, 0),
    Nst_FUNCDECLR(choice_, 1),
    Nst_FUNCDECLR(shuffle_, 2),
    Nst_FUNCDECLR(seed_, 1),
    Nst_DECLR_END
};

static std::mt19937_64 rand_num;

static inline i64 rand_range(i64 min, i64 max)
{
    return rand_num() % (max - min) + min;
}

Nst_Declr *lib_init()
{
    using namespace std::chrono;

    rand_num.seed(duration_cast<nanoseconds>(
        system_clock::now().time_since_epoch()).count());

    return obj_list_;
}

Nst_Obj *NstC random_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_int_new(rand_num());
}

Nst_Obj *NstC rand_int_(usize arg_num, Nst_Obj **args)
{
    i64 min;
    i64 max;

    if (!Nst_extract_args("i i", arg_num, args, &min, &max))
        return nullptr;

    if (min > max) {
        Nst_set_value_error_c("'min' is greater than 'max'");
        return nullptr;
    }

    return Nst_int_new(rand_range(min, max));
}

Nst_Obj *NstC rand_perc_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    u64 num = u64(rand_range(LLONG_MIN, LLONG_MAX - 1));
    return Nst_real_new(f64(num) / f64(ULLONG_MAX));
}

Nst_Obj *NstC choice_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *seq;

    if (!Nst_extract_args("S", arg_num, args, &seq))
        return nullptr;

    Nst_Obj *val = Nst_seq_get(SEQ(seq), rand_num() % SEQ(seq)->len);
    Nst_dec_ref(seq);
    return val;
}

Nst_Obj *NstC shuffle_(usize arg_num, Nst_Obj **args)
{
    Nst_SeqObj *seq;
    bool new_seq;

    if (!Nst_extract_args("A:o y", arg_num, args, &seq, &new_seq))
        return nullptr;

    if (new_seq) {
        Nst_dec_ref(seq);
        seq = SEQ(Nst_seq_copy(seq));
    }

    usize seq_len = seq->len;
    Nst_Obj **objs = seq->objs;

    for (usize i = 0; i + 1 < seq_len; i++) {
        usize idx = usize(rand_range(i, seq_len));
        Nst_Obj *obj = objs[i];
        objs[i] = objs[idx];
        objs[idx] = obj;
    }

    return OBJ(seq);
}

Nst_Obj *NstC seed_(usize arg_num, Nst_Obj **args)
{
    i64 seed;
    if (!Nst_extract_args("i", arg_num, args, &seed))
        return nullptr;
    rand_num.seed(u64(seed));
    Nst_RETURN_NULL;
}
