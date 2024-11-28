#ifndef NEST_TIME_H
#define NEST_TIME_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();

Nst_Obj *NstC time_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC time_ns_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC high_res_time_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC high_res_time_ns_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC monotonic_time_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC monotonic_time_ns_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC year_day_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC week_day_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC day_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC month_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC year_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC date_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC gmt_date_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC second_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC minute_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC hour_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC clock_time_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC gmt_clock_time_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC clock_datetime_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC gmt_clock_datetime_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC sleep_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC sleep_ms_(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_TIME_H