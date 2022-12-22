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

    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(random_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rand_int_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rand_perc_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(choice_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(shuffle_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rand_seed_, 1);
    
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
    return nst_new_int(rand_num());
}

NST_FUNC_SIGN(rand_int_)
{
    Nst_Int min;
    Nst_Int max;

    if ( !nst_extract_arg_values("ii", arg_num, args, err, &min, &max) )
        return nullptr;

    if ( min > max )
    {
        NST_SET_RAW_VALUE_ERROR("'min' is greater than 'max'");
        return nullptr;
    }

    return nst_new_int(rand_range(min, max));
}

NST_FUNC_SIGN(rand_perc_)
{
    return nst_new_real((long double)uint64_t(rand_num()) / (long double)ULLONG_MAX);
}

NST_FUNC_SIGN(choice_)
{
    Nst_Obj *seq;

    if ( !nst_extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    Nst_Obj *val = nst_get_value_seq(SEQ(seq), rand_num() % SEQ(seq)->len);
    nst_dec_ref(seq);
    return val;
}

NST_FUNC_SIGN(shuffle_)
{
    Nst_SeqObj *seq;

    if ( !nst_extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

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

NST_FUNC_SIGN(rand_seed_)
{
    Nst_Int seed;

    if ( !nst_extract_arg_values("i", arg_num, args, err, &seed) )
        return nullptr;

    rand_num.seed(seed);

    NST_RETURN_NULL;
}
