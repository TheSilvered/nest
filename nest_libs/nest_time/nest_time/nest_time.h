#ifndef NEST_TIME_H
#define NEST_TIME_H

#include "../../../nest/nest_include.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

__declspec(dllexport) bool lib_init();
__declspec(dllexport) Nst_FuncDeclr *get_func_ptrs();
__declspec(dllexport) NST_INIT_LIB_OBJ_FUNC;

Nst_Obj *_time(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_time_ns(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_high_res_time(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_high_res_time_ns(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_year_day(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_week_day(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_day(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_month(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_year(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_date(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_seconds(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_minutes(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_hours(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_clock_time(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_gmt_clock_time(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_clock_datetime(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *_gmt_clock_datetime(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
Nst_Obj *sleep(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_TIME_H