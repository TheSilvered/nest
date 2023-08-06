#include <chrono>
#include <ctime>
#include "nest_time.h"

#ifdef Nst_WIN
#include "../dll/framework.h"
#else
#include <unistd.h>
#endif

#define FUNC_COUNT 21

using namespace std::chrono;

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(time_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(time_ns_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(high_res_time_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(high_res_time_ns_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(monotonic_time_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(monotonic_time_ns_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(year_day_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(week_day_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(day_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(month_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(year_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(date_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(seconds_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(minutes_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(hours_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(clock_time_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(gmt_clock_time_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(clock_datetime_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(gmt_clock_datetime_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(sleep_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(sleep_ms_, 1);

#if __LINE__ - FUNC_COUNT != 24
#error
#endif

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
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
    if ( ((y % 4 == 0 && y % 100 != 0) || (y % 400)) && m > 2 )
    {
        day_of_the_year += 1;
    }

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
    Nst_Obj *month_obj = Nst_int_new(t->tm_mon);
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

Nst_FUNC_SIGN(time_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_real_new(duration<f64>(
        system_clock::now().time_since_epoch()).count());
}

Nst_FUNC_SIGN(time_ns_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_int_new(i64(duration_cast<nanoseconds>(
        system_clock::now().time_since_epoch()).count()));
}

Nst_FUNC_SIGN(high_res_time_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_real_new(duration<f64>(
        high_resolution_clock::now().time_since_epoch()).count());
}

Nst_FUNC_SIGN(high_res_time_ns_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_int_new(i64(duration_cast<nanoseconds>(
        high_resolution_clock::now().time_since_epoch()).count()));
}

Nst_FUNC_SIGN(monotonic_time_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_real_new(duration<f64>(
        steady_clock::now().time_since_epoch()).count());
}

Nst_FUNC_SIGN(monotonic_time_ns_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_int_new(i64(duration_cast<nanoseconds>(
        steady_clock::now().time_since_epoch()).count()));
}

Nst_FUNC_SIGN(year_day_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(get_year_day_c(t));
}

Nst_FUNC_SIGN(week_day_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_wday);
}

Nst_FUNC_SIGN(day_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_mday);
}

Nst_FUNC_SIGN(month_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_mon);
}

Nst_FUNC_SIGN(year_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_year + 1900);
}

Nst_FUNC_SIGN(date_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_date(map, localtime);
    return OBJ(map);
}

Nst_FUNC_SIGN(seconds_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_sec);
}

Nst_FUNC_SIGN(minutes_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_min);
}

Nst_FUNC_SIGN(hours_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return Nst_int_new(t->tm_hour);
}

Nst_FUNC_SIGN(clock_time_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_time(map, localtime);
    return OBJ(map);
}

Nst_FUNC_SIGN(gmt_clock_time_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_time(map, gmtime);
    return OBJ(map);
}

Nst_FUNC_SIGN(clock_datetime_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_date(map, localtime);
    add_time(map, localtime);
    return OBJ(map);
}

Nst_FUNC_SIGN(gmt_clock_datetime_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_MapObj *map = MAP(Nst_map_new());
    add_date(map, gmtime);
    add_time(map, gmtime);
    return OBJ(map);
}

Nst_FUNC_SIGN(sleep_)
{
    f64 time;

    Nst_DEF_EXTRACT("N", &time);

#ifdef Nst_WIN
    Sleep(DWORD(time * 1000));
#else
    usleep(useconds_t(time * 1000000));
#endif
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(sleep_ms_)
{
    i64 time;

    Nst_DEF_EXTRACT("l", &time);

#ifdef Nst_WIN
    Sleep(DWORD(time));
#else
    usleep(useconds_t(time * 1000));
#endif
    Nst_RETURN_NULL;
}
