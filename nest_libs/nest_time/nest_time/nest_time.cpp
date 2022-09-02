#include "nest_time.h"
#include <chrono>
#include <ctime>
#include "framework.h"

#define FUNC_COUNT 18

using namespace std::chrono;

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(_time, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_time_ns, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_high_res_time, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_high_res_time_ns, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_year_day, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_week_day, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_day, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_month, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_year, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_date, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_seconds, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_minutes, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_hours, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_clock_time, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_gmt_clock_time, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_clock_datetime, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_gmt_clock_datetime, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(sleep, 1);

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

static Nst_Int get_year_day_c(tm *t)
{
    Nst_Int y = t->tm_year + 1900;
    Nst_Int m = t->tm_mon;
    Nst_Int d = t->tm_mday;

    int days_per_moth[] = { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

    Nst_Int day_of_the_year = days_per_moth[m] + d;
    // If it's a leap year and it's at least March
    if ( ((y % 4 == 0 && y % 100 != 0) || (y % 400)) && m > 2 )
        day_of_the_year += 1;

    return day_of_the_year;
}

static void add_date(Nst_MapObj *map, tm *(*time_func)(const time_t*))
{
    time_t raw_time;
    time(&raw_time);
    tm *t = time_func(&raw_time);

    Nst_Obj *day_obj = nst_new_int(t->tm_mday);
    Nst_Obj *weekday_obj = nst_new_int(t->tm_wday);
    Nst_Obj *yearday_obj = nst_new_int(get_year_day_c(t));
    Nst_Obj *month_obj = nst_new_int(t->tm_mon);
    Nst_Obj *year_obj = nst_new_int(t->tm_year + 1900);

    nst_map_set_str(map, "day", day_obj);
    nst_map_set_str(map, "week_day", weekday_obj);
    nst_map_set_str(map, "year_day", yearday_obj);
    nst_map_set_str(map, "month", month_obj);
    nst_map_set_str(map, "year", year_obj);

    nst_dec_ref(day_obj);
    nst_dec_ref(weekday_obj);
    nst_dec_ref(yearday_obj);
    nst_dec_ref(month_obj);
    nst_dec_ref(year_obj);
}

static void add_time(Nst_MapObj *map, tm *(*time_func)(const time_t *))
{
    time_t raw_time;
    time(&raw_time);
    tm *t = time_func(&raw_time);

    Nst_Obj *seconds = nst_new_int(t->tm_sec);
    Nst_Obj *minutes = nst_new_int(t->tm_min);
    Nst_Obj *hours = nst_new_int(t->tm_hour);

    nst_map_set_str(map, "seconds", seconds);
    nst_map_set_str(map, "minutes", minutes);
    nst_map_set_str(map, "hours", hours);

    nst_dec_ref(seconds);
    nst_dec_ref(minutes);
    nst_dec_ref(hours);
}

Nst_Obj *_time(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    return nst_new_real(duration<Nst_Real>(
        system_clock::now().time_since_epoch()).count()
    );
}

Nst_Obj *_time_ns(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    return nst_new_int(Nst_Int(duration_cast<nanoseconds>(
        system_clock::now().time_since_epoch()).count()
    ));
}

Nst_Obj *_high_res_time(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    return nst_new_real(duration<Nst_Real>(
        high_resolution_clock::now().time_since_epoch()).count()
    );
}

Nst_Obj *_high_res_time_ns(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    return nst_new_int(Nst_Int(duration_cast<nanoseconds>(
        high_resolution_clock::now().time_since_epoch()).count()
    ));
}

Nst_Obj *_year_day(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return nst_new_int(get_year_day_c(t));
}

Nst_Obj *_week_day(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    year_month_day ymd{ std::chrono::floor<days>(system_clock::now()) };
    return nst_new_int(weekday{ ymd }.c_encoding());
}

Nst_Obj *_day(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    year_month_day ymd{ std::chrono::floor<days>(system_clock::now()) };
    return nst_new_int((unsigned int)ymd.day());
}

Nst_Obj *_month(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    year_month_day ymd{ std::chrono::floor<days>(system_clock::now()) };
    return nst_new_int((unsigned int)ymd.month());
}

Nst_Obj *_year(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    year_month_day ymd{ std::chrono::floor<days>(system_clock::now()) };
    return nst_new_int((int)ymd.year());
}

Nst_Obj *_date(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_MapObj *map = AS_MAP(nst_new_map());
    add_date(map, localtime);
    return (Nst_Obj *)map;
}

Nst_Obj *_seconds(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return nst_new_int(t->tm_sec);
}

Nst_Obj *_minutes(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return nst_new_int(t->tm_min);
}

Nst_Obj *_hours(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return nst_new_int(t->tm_hour);
}

Nst_Obj *_clock_time(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_MapObj *map = AS_MAP(nst_new_map());
    add_time(map, localtime);
    return (Nst_Obj *)map;
}

Nst_Obj *_gmt_clock_time(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_MapObj *map = AS_MAP(nst_new_map());
    add_time(map, gmtime);
    return (Nst_Obj *)map;
}

Nst_Obj *_clock_datetime(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_MapObj *map = AS_MAP(nst_new_map());
    add_date(map, localtime);
    add_time(map, localtime);
    return (Nst_Obj *)map;
}

Nst_Obj *_gmt_clock_datetime(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_MapObj *map = AS_MAP(nst_new_map());
    add_date(map, gmtime);
    add_time(map, gmtime);
    return (Nst_Obj *)map;
}

Nst_Obj *sleep(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)
{
    Nst_Real time;

    if ( !nst_extract_arg_values("N", arg_num, args, err, &time) )
        return nullptr;

    Sleep(DWORD(time * 1000));
    return nst_inc_ref(nst_null);
}
