#include <stdlib.h>
#include <string.h>
#include "tests.h"

static const char *RED = "\x1b[31m";
static const char *GREEN = "\x1b[32m";
static const char *YELLOW = "\x1b[33m";
static const char *RESET = "\x1b[0m";

static i32 tests_failed = 0;
static i32 cases_failed = 0;

void test_init(void)
{
    if (!Nst_supports_color()) {
        RED = "";
        GREEN = "";
        YELLOW = "";
        RESET = "";
    }
    tests_failed = 0;
}

void run_test(Test test, const char *test_name)
{
    cases_failed = 0;
    TestResult result = test();

    if (Nst_error_occurred()) {
        Nst_tb_print(Nst_error_get());
        Nst_error_clear();
        if (result == TEST_SUCCESS)
            result = TEST_FAILURE;
    }

    switch (result) {
    case TEST_SUCCESS:
        Nst_printf(
            "%sTest '%s' passed.%s\n",
            GREEN, test_name, RESET);
        break;
    case TEST_FAILURE:
        Nst_printf(
            "%sTest '%s' failed on %" PRIi32 " cases.%s\n",
            RED, test_name, cases_failed, RESET);
        tests_failed += 1;
        break;
    case TEST_NOT_IMPL:
        Nst_printf(
            "%sTest '%s' not implemented.%s\n",
            YELLOW, test_name, RESET);
        break;
    case TEST_CRITICAL_FAILURE:
        Nst_printf(
            "%sTest '%s' failed on %" PRIi32 " cases.%s Stopping execution...\n",
            YELLOW, test_name, cases_failed, RESET);
        Nst_quit();
        exit(1);
    }
}

i32 tests_failed_count(void)
{
    return tests_failed;
}

static void fail(TestResult *result, int line)
{
    Nst_printf("%s  Failure on line %i", RED, line);
    if (Nst_error_occurred()) {
        Nst_printf(" with  error%s\n    ", RESET);
        Nst_tb_print(Nst_error_get());
        Nst_error_clear();
    } else {
        Nst_printf("%s\n", RESET);
    }
    *result = TEST_FAILURE;
    cases_failed++;
}

static void crit_fail(TestResult *result, int line)
{
    Nst_printf("%s  Critical failure on line %i%s\n", RED, line, RESET);
    if (Nst_error_occurred()) {
        Nst_printf(" with error%s\n    ", RESET);
        Nst_tb_print(Nst_error_get());
        Nst_error_clear();
    } else {
        Nst_printf("%s\n", RESET);
    }
    *result = TEST_CRITICAL_FAILURE;
    cases_failed++;
}

static void free_args_va(va_list args)
{
    bool free_ptrs = false;
    void (*custom_func)(void *) = NULL;
    for (void *ptr = va_arg(args, void *);
         ptr != F_END;
         ptr = va_arg(args, void *))
    {
        if (ptr == F_O) {
            free_ptrs = false;
            custom_func = NULL;
        } else if (ptr == F_P) {
            free_ptrs = true;
            custom_func = NULL;
        } else if (ptr == F_C) {
            free_ptrs = true;
            custom_func = (void (*)(void *))va_arg(args, void *);
        } else if (!free_ptrs)
            Nst_ndec_ref(ptr);
        else if (custom_func && ptr)
            custom_func(ptr);
        else if (ptr)
            Nst_free(ptr);
    }
}

bool fail_if_(bool cond, TestResult *result, int line, ...)
{
    va_list args;
    va_start(args, line);

    if (cond) {
        free_args_va(args);
        fail(result, line);
    } else
        Nst_error_clear();
    return cond;
}

bool crit_fail_if_(bool cond, TestResult *result, int line, ...)
{
    va_list args;
    va_start(args, line);

    if (cond) {
        free_args_va(args);
        crit_fail(result, line);
    }
    return cond;
}

bool str_neq(u8 *str1, const char *str2)
{
    if (str1 == NULL)
        return true;
    return strcmp((const char *)(str1), str2) != 0;
}

bool ref_obj_to_bool(Nst_Obj *obj)
{
    bool result = Nst_obj_to_bool(obj);
    Nst_dec_ref(obj);
    return result;
}
