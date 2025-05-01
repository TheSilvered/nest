#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tests.h"

#define CAPTURE_BUF_SIZE 4096

static const char *RED = "\x1b[31m";
static const char *GREEN = "\x1b[32m";
static const char *YELLOW = "\x1b[33m";
static const char *RESET = "\x1b[0m";

static i32 tests_failed = 0;
static i32 cases_failed = 0;

static bool capturing = false;
static char capture_buf[CAPTURE_BUF_SIZE];
static FILE *orig_stdout = NULL;
static FILE *orig_stderr = NULL;
static FILE *capture_file = NULL;

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

void run_test_(Test test, const char *test_name)
{
    Nst_printf("Running %s:", test_name);

    cases_failed = 0;
    TestResult result = test();

    if (capturing)
        capture_output_end(NULL);

    if (Nst_error_occurred()) {
        Nst_error_print();
        Nst_error_clear();
        if (result == TEST_SUCCESS)
            result = TEST_FAILURE;
    }

    switch (result) {
    case TEST_SUCCESS:
        Nst_printf(" %spassed.%s\n", GREEN, RESET);
        break;
    case TEST_FAILURE:
        Nst_printf(
            "\n  %sfailed on %" PRIi32 " checks.%s\n",
            RED, cases_failed, RESET);
        tests_failed += 1;
        break;
    case TEST_NOT_IMPL:
        Nst_printf(" %snot implemented.%s\n", YELLOW, RESET);
        break;
    case TEST_CRITICAL_FAILURE:
        Nst_printf(
            "\n  %sfailed on %" PRIi32 " checks.%s\nExiting.\n",
            YELLOW, cases_failed, RESET);
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
    Nst_printf("\n%s  failure on line %i", RED, line);
    if (Nst_error_occurred()) {
        Nst_printf(" with  error%s\n    ", RESET);
        Nst_error_print();
        Nst_error_clear();
    } else {
        Nst_printf("%s", RESET);
    }
    *result = TEST_FAILURE;
    cases_failed++;
}

static void crit_fail(TestResult *result, int line)
{
    Nst_printf("\n%s  critical failure on line %i%s", RED, line, RESET);
    if (Nst_error_occurred()) {
        Nst_printf(" with error%s\n    ", RESET);
        Nst_error_print();
        Nst_error_clear();
    } else {
        Nst_printf("%s", RESET);
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

bool capture_output_begin(void)
{
    if (capturing) {
        stderr = orig_stderr;
        stdout = orig_stdout;
        fclose(capture_file);
        Nst_fprintf(
            Nst_stdio()->err,
            "capture_output_begin failed, already capturing\n");
        return false;
    }
    orig_stdout = stdout;
    orig_stderr = stderr;
    memset(capture_buf, 0, CAPTURE_BUF_SIZE * sizeof(char));
    capture_file = tmpfile();
    if (capture_file == NULL) {
        Nst_fprintf(
            Nst_stdio()->err,
            "capture_output_begin failed to create temp file\n");
        return false;
    }
    capturing = true;
    stdout = capture_file;
    stderr = capture_file;
    return true;
}

const char *capture_output_end(usize *out_length)
{
    if (!capturing) {
        Nst_fprintf(
            Nst_stdio()->err,
            "capture_output_end failed, capturing not started\n");
        if (out_length != NULL)
            *out_length = 0;
        return NULL;
    }
    stdout = orig_stdout;
    stderr = orig_stderr;
    capturing = false;

    if (fseek(capture_file, 0, SEEK_SET) != 0) {
        fclose(capture_file);
        Nst_fprintf(
            Nst_stdio()->err,
            "capture_output_end failed to seek file\n");
        if (out_length != NULL)
            *out_length = 0;
        return NULL;
    }
    usize bytes_written = fread(
        capture_buf,
        sizeof(char),
        CAPTURE_BUF_SIZE - 1,
        capture_file);
    if (ferror(capture_file)) {
        fclose(capture_file);
        Nst_fprintf(
            Nst_stdio()->err,
            "capture_output_end failed to read file\n");
        if (out_length != NULL)
            *out_length = 0;
        return NULL;
    }
    capture_buf[bytes_written] = '\0';
    if (out_length != NULL)
        *out_length = bytes_written;
    return (const char *)capture_buf;
}
