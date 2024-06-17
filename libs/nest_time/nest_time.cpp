#include <chrono>
#include <ctime>
#include "nest_time.h"

#ifdef Nst_WIN
#include "../dll/framework.h"
#else
#include <unistd.h>
#endif

using namespace std::chrono;

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(time_, 0),
    Nst_FUNCDECLR(time_ns_, 0),
    Nst_FUNCDECLR(high_res_time_, 0),
    Nst_FUNCDECLR(high_res_time_ns_, 0),
    Nst_FUNCDECLR(monotonic_time_, 0),
    Nst_FUNCDECLR(monotonic_time_ns_, 0),
    Nst_FUNCDECLR(year_day_, 0),
    Nst_FUNCDECLR(week_day_, 0),
    Nst_FUNCDECLR(day_, 0),
    Nst_FUNCDECLR(month_, 0),
    Nst_FUNCDECLR(year_, 0),
    Nst_FUNCDECLR(date_, 0),
    Nst_FUNCDECLR(gmt_date_, 0),
    Nst_FUNCDECLR(seconds_, 0),
    Nst_FUNCDECLR(minutes_, 0),
    Nst_FUNCDECLR(hours_, 0),
    Nst_FUNCDECLR(clock_time_, 0),
    Nst_FUNCDECLR(gmt_clock_time_, 0),
    Nst_FUNCDECLR(clock_datetime_, 0),
    Nst_FUNCDECLR(gmt_clock_datetime_, 0),
    Nst_FUNCDECLR(sleep_, 1),
    Nst_FUNCDECLR(sleep_ms_, 1),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    return obj_list_;
}

static i64 get_year_day_c(tm *t)
{
    i64 y = t->tm_year + 1900;
    i64 m = t->tm_mon;
    i64 d = t->tm_mday;

    i32 days_per_moth[] = {   0,   0,  31,  59,  90, 120, 151,
                            181, 212, 243, 273, 304, 334       };

    i64 day_of_the_year = days_per_moth[m] + d;
    // If it's a leap year and it's at least March
    if (((y % 4 == 0 && y % 100 != 0) || (y % 400)) && m > 2)
        day_of_the_year += 1;

    return day_of_the_year;
}

static void add_date(Nst_MapObj *map, tm *(*time_func)(const time_t*))
{
    time_t raw_time;
    time(&raw_time);
    tm *t = time_func(&raw_time);

    Nst_Obj *day_obj = Nst_int_new(t->tm_mday);
    Nst_Obj *weekday_obj = Nst_int_new(t->tm_wday);
    Nst_Obj *yearday_obj = Nst_int_new(get_year_day_c(t));
    Nst_Obj *month_obj = Nst_int_new(t->tm_mon + 1);
    Nst_Obj *year_obj = Nst_int_new(t->tm_year + 1900);

    Nst_map_set_str(map, "year", year_obj);
    Nst_map_set_str(map, "month", month_obj);
    Nst_map_set_str(map, "year_day", yearday_obj);
    Nst_map_set_str(map, "day", day_obj);
    Nst_map_set_str(map, "week_day", weekday_obj);

    Nst_dec_ref(day_obj);
    Nst_dec_ref(weekday_obj);
    Nst_dec_ref(yearday_obj);
    Nst_dec_ref(month_obj);
    Nst_dec_ref(year_obj);
}

static void add_time(Nst_MapObj *map, tm *(*time_func)(const time_t *))
{
    time_t raw_time;
    time(&raw_time);
    tm *t = time_func(&raw_time);

    Nst_Obj *second = Nst_int_new(t->tm_sec);
    Nst_Obj *minute = Nst_int_new(t->tm_min);
    Nst_Obj *hour = Nst_int_new(t->tm_hour);

    Nst_map_set_str(map, "hour", hour);
    Nst_map_set_str(map, "minute", minute);
    Nst_map_set_str(map, "second", second);

    Nst_dec_ref(second);
    Nst_dec_ref(minute);
    Nst_dec_ref(hour);
}

Nst_Obj *NstC time_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_real_new(duration<f64>(
        system_clock::now().time_since_epoch()).count());
}

Nst_Obj *NstC time_ns_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_int_new(i64(duration_cast<nanoseconds>(
        system_clock::now().time_since_epoch()).count()));
}

Nst_Obj *NstC high_res_time_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_real_new(duration<f64>(
        high_resolution_clock::now().time_since_epoch()).count());
}

Nst_Obj *NstC high_res_time_ns_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_int_new(i64(duration_cast<nanoseconds>(
        high_resolution_clock::now().time_since_epoch()).count()));
}

Nst_Obj *NstC monotonic_time_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_real_new(duration<f64>(
        steady_clock::now().time_since_epoch()).count());
}

Nst_Obj *NstC monotonic_time_ns_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_int_new(i64(duration_cast<nanoseconds>(
        steady_clock::now().time_since_epoch()).count()));
}

Nst_Obj *NstC year_day_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(get_year_day_c(t));
}

Nst_Obj *NstC week_day_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_wday);
}

Nst_Obj *NstC day_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_mday);
}

Nst_Obj *NstC month_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_mon + 1);
}

Nst_Obj *NstC year_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_year + 1900);
}

Nst_Obj *NstC date_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_date(map, localtime);
    return OBJ(map);
}

Nst_Obj *NstC gmt_date_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_date(map, gmtime);
    return OBJ(map);
}

Nst_Obj *NstC seconds_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_sec);
}

Nst_Obj *NstC minutes_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_min);
}

Nst_Obj *NstC hours_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_hour);
}

Nst_Obj *NstC clock_time_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_time(map, localtime);
    return OBJ(map);
}

Nst_Obj *NstC gmt_clock_time_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_time(map, gmtime);
    return OBJ(map);
}

Nst_Obj *NstC clock_datetime_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_date(map, localtime);
    add_time(map, localtime);
    return OBJ(map);
}

Nst_Obj *NstC gmt_clock_datetime_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_date(map, gmtime);
    add_time(map, gmtime);
    return OBJ(map);
}

Nst_Obj *NstC sleep_(usize arg_num, Nst_Obj **args)
{
    f64 time;

    if (!Nst_extract_args("N", arg_num, args, &time))
        return nullptr;

#ifdef Nst_WIN
    Sleep(DWORD(time * 1000));
#else
    usleep(useconds_t(time * 1000000));
#endif
    Nst_RETURN_NULL;
}

Nst_Obj *NstC sleep_ms_(usize arg_num, Nst_Obj **args)
{
    i64 time;

    if (!Nst_extract_args("l", arg_num, args, &time))
        return nullptr;

#ifdef Nst_WIN
    Sleep(DWORD(time));
#else
    usleep(useconds_t(time * 1000));
#endif
    Nst_RETURN_NULL;
}
