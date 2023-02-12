#include <random>
#include <chrono>
#include <climits>
#include "nest_rand.h"

#define FUNC_COUNT 6

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;
static std::mt19937_64 rand_num;

static inline Nst_Int rand_range(Nst_Int min, Nst_Int max)
{
    return rand_num() % (max - min) + min;
}

bool lib_init()
{
    using namespace std::chrono;

    if ( (func_list_ = nst_func_list_new(FUNC_COUNT)) == nullptr )
    {
        return false;
    }

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(random_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rand_int_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rand_perc_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(choice_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(shuffle_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(seed_, 1);

#if __LINE__ - FUNC_COUNT != 29
#error FUNC_COUNT does not match the number of lines
#endif

    rand_num.seed(duration_cast<nanoseconds>(
        system_clock::now().time_since_epoch()
    ).count());

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

NST_FUNC_SIGN(random_)
{
    return nst_int_new(rand_num());
}

NST_FUNC_SIGN(rand_int_)
{
    Nst_Int min;
    Nst_Int max;

    NST_DEF_EXTRACT("ii", &min, &max);

    if ( min > max )
    {
        NST_SET_RAW_VALUE_ERROR("'min' is greater than 'max'");
        return nullptr;
    }

    return nst_int_new(rand_range(min, max));
}

NST_FUNC_SIGN(rand_perc_)
{
    return nst_real_new(
        (long double)uint64_t(rand_num()) / (long double)ULLONG_MAX);
}

NST_FUNC_SIGN(choice_)
{
    Nst_Obj *seq;

    NST_DEF_EXTRACT("S", &seq);

    Nst_Obj *val = nst_seq_get(SEQ(seq), rand_num() % SEQ(seq)->len);
    nst_dec_ref(seq);
    return val;
}

NST_FUNC_SIGN(shuffle_)
{
    Nst_SeqObj *seq;

    NST_DEF_EXTRACT("A", &seq);

    size_t seq_len = seq->len;
    Nst_Obj **objs = seq->objs;

    for ( size_t i = 0; i + 1 < seq_len; i++ )
    {
        size_t idx = size_t(rand_range(i, seq_len));
        Nst_Obj *obj = objs[i];
        objs[i] = objs[idx];
        objs[idx] = obj;
    }

    NST_RETURN_NULL;
}

NST_FUNC_SIGN(seed_)
{
    Nst_Int seed;

    NST_DEF_EXTRACT("i", &seed);

    rand_num.seed(seed);

    NST_RETURN_NULL;
}
