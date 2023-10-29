#ifndef NEST_TIME_H
#define NEST_TIME_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();

Nst_FUNC_SIGN(time_);
Nst_FUNC_SIGN(time_ns_);
Nst_FUNC_SIGN(high_res_time_);
Nst_FUNC_SIGN(high_res_time_ns_);
Nst_FUNC_SIGN(monotonic_time_);
Nst_FUNC_SIGN(monotonic_time_ns_);
Nst_FUNC_SIGN(year_day_);
Nst_FUNC_SIGN(week_day_);
Nst_FUNC_SIGN(day_);
Nst_FUNC_SIGN(month_);
Nst_FUNC_SIGN(year_);
Nst_FUNC_SIGN(date_);
Nst_FUNC_SIGN(gmt_date_);
Nst_FUNC_SIGN(seconds_);
Nst_FUNC_SIGN(minutes_);
Nst_FUNC_SIGN(hours_);
Nst_FUNC_SIGN(clock_time_);
Nst_FUNC_SIGN(gmt_clock_time_);
Nst_FUNC_SIGN(clock_datetime_);
Nst_FUNC_SIGN(gmt_clock_datetime_);
Nst_FUNC_SIGN(sleep_);
Nst_FUNC_SIGN(sleep_ms_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_TIME_H