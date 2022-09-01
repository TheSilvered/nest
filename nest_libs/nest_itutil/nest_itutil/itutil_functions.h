#ifndef ITUTIL_FUNCTIONS_H
#define ITUTIL_FUNCTIONS_H

#include "../../../nest/nest_include.h"

Nst_Obj *count_start(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *count_advance(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *count_is_done(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *count_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

Nst_Obj *cycle_start(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *cycle_advance(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *cycle_is_done(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *cycle_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

Nst_Obj *repeat_start(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *repeat_advance(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *repeat_is_done(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *repeat_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

Nst_Obj *chain_start(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *chain_advance(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *chain_is_done(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *chain_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

Nst_Obj *zip_start(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *zip_advance(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *zip_is_done(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *zip_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

Nst_Obj *enumerate_start(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *enumerate_advance(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *enumerate_is_done(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *enumerate_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

Nst_Obj *kvi_start(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *kvi_advance(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *kvi_is_done(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *keys_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *values_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *items_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

Nst_Obj *reversed_start(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *reversed_advance(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *reversed_is_done(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *reversed_get_val(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

#endif // !ITUTIL_FUNCTIONS_H
