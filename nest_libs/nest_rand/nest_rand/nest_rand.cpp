#include <random>
#include <chrono>
#include "nest_rand.h"

#define SET_TYPE_ERROR(msg) \
    err->name = (char *)"Type Error"; \
    err->message = (char *)msg;

#define SET_VALUE_ERROR(msg) \
    err->name = (char *)"Value Error"; \
    err->message = (char *)msg;


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
        duration_cast<milliseconds>(
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
    Nst_Obj *min_obj = args[0];
    Nst_Obj *max_obj = args[1];

    if ( min_obj->type != nst_t_int || max_obj->type != nst_t_int )
    {
        SET_TYPE_ERROR("expected 'Int'");
        return nullptr;
    }

    Nst_int min = AS_INT(min_obj);
    Nst_int max = AS_INT(max_obj);

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
    Nst_Obj *seq_obj = args[0];

    if ( seq_obj->type != nst_t_arr || seq_obj->type != nst_t_vect )
    {
        SET_TYPE_ERROR("expected 'Array' or 'Vector'");
        return nullptr;
    }

    Nst_sequence *seq = AS_SEQ(seq_obj);
    return get_value_seq(seq, rand() % seq->len);
}

Nst_Obj *shuffle(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *seq_obj = args[0];

    if ( seq_obj->type != nst_t_arr || seq_obj->type != nst_t_vect )
    {
        SET_TYPE_ERROR("expected 'Array' or 'Vector'");
        return nullptr;
    }

    Nst_sequence *seq = AS_SEQ(seq_obj);
    size_t seq_len = seq->len;
    Nst_Obj **objs = seq->objs;

    for ( size_t i = 0; i - 1 < seq_len; i++ )
    {
        size_t idx = size_t(rand_range(i + 1, seq_len));
        Nst_Obj *obj = seq->objs[i];
        seq->objs[i] = seq->objs[idx];
        seq->objs[idx] = obj;
    }

    inc_ref(nst_null);
    return nst_null;
}

Nst_Obj *rand_seed(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_Obj *seed = args[0];

    if ( seed->type != nst_t_int )
    {
        SET_TYPE_ERROR("expected 'Int'");
        return nullptr;
    }

    srand(AS_INT(seed));

    inc_ref(nst_null);
    return nst_null;
}

Nst_Obj *_get_rand_max(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return new_int_obj(RAND_MAX);
}
