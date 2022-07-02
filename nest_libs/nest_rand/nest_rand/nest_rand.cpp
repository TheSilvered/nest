#include <random>
#include <chrono>
#include "nest_rand.h"

#define FUNC_COUNT 7

static FuncDeclr *func_list_;
static bool lib_init_ = false;

static inline Nst_int rand_range(Nst_int min, Nst_int max)
{
    return rand() % (max - min) + min;
}

bool lib_init()
{
    using namespace std::chrono;

    if ( (func_list_ = new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = MAKE_FUNCDECLR(random, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(rand_int, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(rand_perc, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(choice, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(shuffle, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(rand_seed, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(_get_rand_max, 0);

    srand(unsigned int(
        duration_cast<nanoseconds>(
            system_clock::now().time_since_epoch()
        ).count())
    );

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

Nst_Obj *random(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return new_int_obj(rand());
}

Nst_Obj *rand_int(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_int min;
    Nst_int max;

    if ( !extract_arg_values("ii", arg_num, args, err, &min, &max) )
        return nullptr;

    if ( min > max )
    {
        SET_VALUE_ERROR("'min' is greater than 'max'");
        return nullptr;
    }

    return new_int_obj(rand_range(min, max));
}

Nst_Obj *rand_perc(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return new_real_obj(double(rand()) / double(RAND_MAX));
}

Nst_Obj *choice(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *seq;

    if ( !extract_arg_values("S", arg_num, args, err, &seq) )
        return nullptr;

    Nst_Obj *val = get_value_seq(AS_SEQ(seq), rand() % AS_SEQ(seq)->len);
    dec_ref(seq);
    return val;
}

Nst_Obj *shuffle(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_sequence *seq;

    if ( !extract_arg_values("A", arg_num, args, err, &seq) )
        return nullptr;

    size_t seq_len = seq->len;
    Nst_Obj **objs = seq->objs;

    for ( size_t i = 0; i + 1 < seq_len; i++ )
    {
        size_t idx = size_t(rand_range(i, seq_len));
        Nst_Obj *obj = seq->objs[i];
        seq->objs[i] = seq->objs[idx];
        seq->objs[idx] = obj;
    }

    return inc_ref(nst_null);
}

Nst_Obj *rand_seed(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_int seed;

    if ( !extract_arg_values("i", arg_num, args, err, &seed) )
        return nullptr;

    srand((unsigned int)seed);

    return inc_ref(nst_null);
}

Nst_Obj *_get_rand_max(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return new_int_obj(RAND_MAX);
}
