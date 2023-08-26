#include <random>
#include <chrono>
#include <climits>
#include "nest_rand.h"

#define FUNC_COUNT 6

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static std::mt19937_64 rand_num;

static inline i64 rand_range(i64 min, i64 max)
{
    return rand_num() % (max - min) + min;
}

bool lib_init()
{
    using namespace std::chrono;
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(random_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(rand_int_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(rand_perc_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(choice_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(shuffle_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(seed_, 1);

#if __LINE__ - FUNC_COUNT != 24
#error
#endif

    rand_num.seed(duration_cast<nanoseconds>(
        system_clock::now().time_since_epoch()).count());

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

Nst_FUNC_SIGN(random_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_int_new(rand_num());
}

Nst_FUNC_SIGN(rand_int_)
{
    i64 min;
    i64 max;

    Nst_DEF_EXTRACT("i i", &min, &max);

    if (min > max) {
        Nst_set_value_error_c("'min' is greater than 'max'");
        return nullptr;
    }

    return Nst_int_new(rand_range(min, max));
}

Nst_FUNC_SIGN(rand_perc_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    u64 num = u64(rand_range(LLONG_MIN, LLONG_MAX - 1));
    return Nst_real_new(f64(num) / f64(ULLONG_MAX));
}

Nst_FUNC_SIGN(choice_)
{
    Nst_Obj *seq;

    Nst_DEF_EXTRACT("S", &seq);

    Nst_Obj *val = Nst_seq_get(SEQ(seq), rand_num() % SEQ(seq)->len);
    Nst_dec_ref(seq);
    return val;
}

Nst_FUNC_SIGN(shuffle_)
{
    Nst_SeqObj *seq;

    Nst_DEF_EXTRACT("A", &seq);

    usize seq_len = seq->len;
    Nst_Obj **objs = seq->objs;

    for (usize i = 0; i + 1 < seq_len; i++) {
        usize idx = usize(rand_range(i, seq_len));
        Nst_Obj *obj = objs[i];
        objs[i] = objs[idx];
        objs[idx] = obj;
    }

    return Nst_inc_ref(seq);
}

Nst_FUNC_SIGN(seed_)
{
    i64 seed;
    Nst_DEF_EXTRACT("i", &seed);
    rand_num.seed(seed);
    Nst_RETURN_NULL;
}
