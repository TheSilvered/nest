#ifndef ITUTIL_FUNCTIONS_H
#define ITUTIL_FUNCTIONS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct CountData {
    i64 idx;
    i64 start, step;
} CountData;

typedef struct CycleData {
    i64 idx;
    Nst_Obj *iterable;
} CycleData;

void destroy_cycle_data(Nst_Obj *obj);

typedef struct RepeatData {
    i64 count;
    i64 max_count;
    Nst_Obj *item;
} RepeatData;

void destroy_repeat_data(Nst_Obj *obj);

typedef struct ChainData {
    Nst_Obj *main_iter;
    Nst_Obj *current_iter;
} ChainData;

void destroy_chain_data(Nst_Obj *obj);

typedef struct ZipData {
    usize count;
    Nst_Obj **iterators;
} ZipData;

void destroy_zip_data(Nst_Obj *obj);

typedef struct EnumerateData {
    i64 idx;
    i64 start, step;
    Nst_Obj *iterator;
    bool invert_order;
} EnumerateData;

void destroy_enumerate_data(Nst_Obj *obj);

typedef struct MapIterData {
    isize idx;
    Nst_Obj *map;
} MapIterData;

void destroy_map_iter_data(Nst_Obj *obj);

typedef struct BatchData {
    Nst_Obj *iterator;
    Nst_Obj *padding_obj;
    usize batch_size;
    bool iter_ended;
} BatchData;

void destroy_batch_data(Nst_Obj *obj);

typedef struct _ItutilFunctions {
    Nst_Obj *count_start;
    Nst_Obj *count_next;
    Nst_Obj *cycle_start;
    Nst_Obj *cycle_str_next;
    Nst_Obj *cycle_seq_next;
    Nst_Obj *cycle_iter_start;
    Nst_Obj *cycle_iter_next;
    Nst_Obj *repeat_start;
    Nst_Obj *repeat_next;
    Nst_Obj *chain_start;
    Nst_Obj *chain_next;
    Nst_Obj *zip_start;
    Nst_Obj *zip_next;
    Nst_Obj *enumerate_start;
    Nst_Obj *enumerate_next;
    Nst_Obj *map_iter_start;
    Nst_Obj *keys_next;
    Nst_Obj *values_next;
    Nst_Obj *batch_start;
    Nst_Obj *batch_next;
} ItutilFunctions;

extern ItutilFunctions itutil_functions;

bool init_itutil_functions();
void free_itutil_functions();

Nst_Obj *NstC count_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC count_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC cycle_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_str_next(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_seq_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC cycle_iter_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC cycle_iter_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC repeat_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC repeat_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC chain_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC chain_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC zip_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC zip_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC enumerate_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC enumerate_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC map_iter_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC keys_next(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC values_next(usize arg_num, Nst_Obj **args);

Nst_Obj *NstC batch_start(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC batch_next(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ITUTIL_FUNCTIONS_H
