#ifndef NEST_TIME_H
#define NEST_TIME_H

#include "../../../src/nest.h"

#if defined(_WIN32) || defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_FuncDeclr *get_func_ptrs();
EXPORT NST_INIT_LIB_OBJ_FUNC;

NST_FUNC_SIGN(time_);
NST_FUNC_SIGN(time_ns_);
NST_FUNC_SIGN(high_res_time_);
NST_FUNC_SIGN(high_res_time_ns_);
NST_FUNC_SIGN(year_day_);
NST_FUNC_SIGN(week_day_);
NST_FUNC_SIGN(day_);
NST_FUNC_SIGN(month_);
NST_FUNC_SIGN(year_);
NST_FUNC_SIGN(date_);
NST_FUNC_SIGN(seconds_);
NST_FUNC_SIGN(minutes_);
NST_FUNC_SIGN(hours_);
NST_FUNC_SIGN(clock_time_);
NST_FUNC_SIGN(gmt_clock_time_);
NST_FUNC_SIGN(clock_datetime_);
NST_FUNC_SIGN(gmt_clock_datetime_);
NST_FUNC_SIGN(sleep_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_TIME_H