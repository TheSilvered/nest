#include "nest_time.h"
#include <chrono>
#include <ctime>
#include "framework.h"

#define FUNC_COUNT 18

using namespace std::chrono;

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = MAKE_FUNCDECLR(_time, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_time_ns, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_high_res_time, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_high_res_time_ns, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_year_day, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_week_day, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_day, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_month, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_year, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_date, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_seconds, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_minutes, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_hours, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_clock_time, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_gmt_clock_time, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_clock_datetime, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(_gmt_clock_datetime, 0);
    func_list_[idx++] = MAKE_FUNCDECLR(sleep, 1);

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

static Nst_int get_year_day_c(tm *t)
{
    Nst_int y = t->tm_year + 1900;
    Nst_int m = t->tm_mon;
    Nst_int d = t->tm_mday;

    int days_per_moth[] = { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

    Nst_int day_of_the_year = days_per_moth[m] + d;
    // If it's a leap year and it's at least March
    if ( ((y % 4 == 0 && y % 100 != 0) || (y % 400)) && m > 2 )
        day_of_the_year += 1;

    return day_of_the_year;
}

static void add_date(Nst_map *map, tm *(*time_func)(const time_t*))
{
    time_t raw_time;
    time(&raw_time);
    tm *t = time_func(&raw_time);

    Nst_Obj *day_obj = new_int_obj(t->tm_mday);
    Nst_Obj *weekday_obj = new_int_obj(t->tm_wday);
    Nst_Obj *yearday_obj = new_int_obj(get_year_day_c(t));
    Nst_Obj *month_obj = new_int_obj(t->tm_mon);
    Nst_Obj *year_obj = new_int_obj(t->tm_year + 1900);

    map_set_str(map, "day", day_obj);
    map_set_str(map, "weekday", weekday_obj);
    map_set_str(map, "yearday", yearday_obj);
    map_set_str(map, "month", month_obj);
    map_set_str(map, "year", month_obj);

    dec_ref(day_obj);
    dec_ref(weekday_obj);
    dec_ref(yearday_obj);
    dec_ref(month_obj);
    dec_ref(year_obj);
}

static void add_time(Nst_map *map, tm *(*time_func)(const time_t *))
{
    time_t raw_time;
    time(&raw_time);
    tm *t = time_func(&raw_time);

    Nst_Obj *seconds = new_int_obj(t->tm_sec);
    Nst_Obj *minutes = new_int_obj(t->tm_min);
    Nst_Obj *hours = new_int_obj(t->tm_hour);

    map_set_str(map, "seconds", seconds);
    map_set_str(map, "minutes", minutes);
    map_set_str(map, "hours", hours);

    dec_ref(seconds);
    dec_ref(minutes);
    dec_ref(hours);
}

Nst_Obj *_time(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return new_real_obj(duration<Nst_real>(
        system_clock::now().time_since_epoch()).count()
    );
}

Nst_Obj *_time_ns(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return new_int_obj(Nst_int(duration_cast<nanoseconds>(
        system_clock::now().time_since_epoch()).count()
    ));
}

Nst_Obj *_high_res_time(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return new_real_obj(duration<Nst_real>(
        high_resolution_clock::now().time_since_epoch()).count()
    );
}

Nst_Obj *_high_res_time_ns(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return new_int_obj(Nst_int(duration_cast<nanoseconds>(
        high_resolution_clock::now().time_since_epoch()).count()
    ));
}

Nst_Obj *_year_day(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    return new_int_obj(get_year_day_c(t));
}

Nst_Obj *_week_day(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    year_month_day ymd{ std::chrono::floor<days>(system_clock::now()) };
    return new_int_obj(weekday{ ymd }.c_encoding());
}

Nst_Obj *_day(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    year_month_day ymd{ std::chrono::floor<days>(system_clock::now()) };
    return new_int_obj((unsigned int)ymd.day());
}

Nst_Obj *_month(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    year_month_day ymd{ std::chrono::floor<days>(system_clock::now()) };
    return new_int_obj((unsigned int)ymd.month());
}

Nst_Obj *_year(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    year_month_day ymd{ std::chrono::floor<days>(system_clock::now()) };
    return new_int_obj((int)ymd.year());
}

Nst_Obj *_date(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_map *map = new_map();
    add_date(map, localtime);

    return make_obj(map, nst_t_map, (void (*)(void *))destroy_map);
}

Nst_Obj *_seconds(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);
    
    return new_int_obj(t->tm_sec);
}

Nst_Obj *_minutes(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);

    return new_int_obj(t->tm_min);
}

Nst_Obj *_hours(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    time_t raw_time;
    time(&raw_time);
    tm *t = localtime(&raw_time);

    return new_int_obj(t->tm_hour);
}

Nst_Obj *_clock_time(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_map *map = new_map();

    add_time(map, localtime);

    return make_obj(map, nst_t_map, (void (*)(void *))destroy_map);
}

Nst_Obj *_gmt_clock_time(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_map *map = new_map();

    add_time(map, gmtime);

    return make_obj(map, nst_t_map, (void (*)(void *))destroy_map);
}

Nst_Obj *_clock_datetime(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_map *map = new_map();

    add_date(map, localtime);
    add_time(map, localtime);

    return make_obj(map, nst_t_map, (void (*)(void *))destroy_map);
}

Nst_Obj *_gmt_clock_datetime(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_map *map = new_map();

    add_date(map, gmtime);
    add_time(map, gmtime);

    return make_obj(map, nst_t_map, (void (*)(void *))destroy_map);
}

Nst_Obj *sleep(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_real time;

    if ( !extract_arg_values("R", arg_num, args, err, &time) )
        return nullptr;

    Sleep(DWORD(time * 1000));

    inc_ref(nst_null);
    return nst_null;
}
