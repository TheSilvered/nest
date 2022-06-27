#ifndef NEST_TIME_H
#define NEST_TIME_H

#include "nest_source/nest_include.h"

#ifdef NESTTIME_EXPORTS
#define NEST_TIME_API __declspec(dllexport)
#else
#define NEST_TIME_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NEST_TIME_API bool lib_init();
NEST_TIME_API FuncDeclr *get_func_ptrs();
NEST_TIME_API INIT_LIB_OBJ_FUNC;

Nst_Obj *_time(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_time_ns(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_high_res_time(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_high_res_time_ns(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_year_day(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_week_day(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_day(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_month(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_year(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_date(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_seconds(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_minutes(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_hours(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_clock_time(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_gmt_clock_time(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_clock_datetime(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *_gmt_clock_datetime(size_t arg_num, Nst_Obj **args, OpErr *err);
Nst_Obj *sleep(size_t arg_num, Nst_Obj **args, OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_TIME_H