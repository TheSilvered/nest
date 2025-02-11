#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "test_nest.h"

#if defined(Nst_GCC) || defined(Nst_CLANG)
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-label"
#elif defined(Nst_MSVC)
#pragma warning(disable: 4102)
#endif

#define ENTER_TEST TestResult test_result__ = TEST_SUCCESS
#define EXIT_TEST failure: return test_result__

#define logerr(call) ()

const i8 *RED = "\x1b[31m";
const i8 *GREEN = "\x1b[32m";
const i8 *YELLOW = "\x1b[33m";
const i8 *RESET = "\x1b[0m";

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

void run_test(Test test, const i8 *test_name)
{
    cases_failed = 0;
    TestResult result = test();

    if (Nst_error_occurred()) {
        Nst_print_traceback(Nst_error_get());
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
            "%sTest '%s' failed on %li cases.%s\n",
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
            "%sTest '%s' failed on %li cases.%s Stopping execution...\n",
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
        Nst_print_traceback(Nst_error_get());
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
        Nst_print_traceback(Nst_error_get());
        Nst_error_clear();
    } else {
        Nst_printf("%s\n", RESET);
    }
    *result = TEST_CRITICAL_FAILURE;
    cases_failed++;
}

// Pass to fail_if to begin freeing objects (with Nst_dec_ref)
#define F_O ((void *) 2)
// Pass to fail_if to begin freeing pointers (with Nst_free)
#define F_P ((void *) 3)
// Pass to fail_if to begin freeing custom pointers, the first parameter after
// this will be the function used
#define F_C ((void *) 4)

#define F_END ((void *) 1)

#define F_ENDIF(ptr) (void *)((usize)(ptr) * (usize)F_END))

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

// To this function are passed the va_args of fail_if and crit_fail_if
// By default they will free pointers as Nest objects with Nst_dec_ref,
// Pass F_P to begin freeing pointers with Nst_free
// Pass F_O to start freeing objects again
// Pass F_C followed by a function of type void (*)(void *), the pointers
// will then be passed to that function
// Any NULL pointer is automatically skipped
// F_END will end the series even with pointers still in the series, it is
// added automatically after all aruments when calling fail_if or crit_fail_if
// F_ENDIF will stop if the given pointer is NULL.
#define free_args(...) free_args_(0, __VA_ARGS__)

static void free_args_(int dummy, ...)
{
    va_list args;
    va_start(args, dummy);
    free_args_va(args);
}

#define fail_if(cond, ...)                                                    \
    fail_if_((cond), &test_result__, __LINE__, ## __VA_ARGS__, F_END)
static bool fail_if_(bool cond, TestResult *result, int line, ...)
{
    va_list args;
    va_start(args, line);

    if (cond) {
        free_args_va(args);
        fail(result, line);
    }
    return cond;
}

#define crit_fail_if(cond, ...) do {                                          \
    if (crit_fail_if_(                                                        \
            (cond),                                                           \
            &test_result__,                                                   \
            __LINE__, ## __VA_ARGS__,                                         \
            F_END))                                                           \
    {                                                                         \
        goto failure;                                                         \
    }} while (0)
static bool crit_fail_if_(bool cond, TestResult *result, int line, ...)
{
    va_list args;
    va_start(args, line);

    if (cond) {
        free_args_va(args);
        crit_fail(result, line);
    }
    return cond;
}

static bool is_null(void *ptr)
{
    if (ptr == NULL) {
        return true;
    }
    Nst_free(ptr);
    return false;
}

static bool str_neq(i8 *str1, const i8 *str2)
{
    if (str1 == NULL)
        return true;
    if (strcmp((const i8 *)(str1), str2) != 0) {
        Nst_free(str1);
        return true;
    }
    Nst_free(str1);
    return false;
}

// Object to bool, takes a reference from the object
static bool ref_obj_to_bool(Nst_Obj *obj)
{
    bool result = Nst_obj_to_bool(obj);
    Nst_dec_ref(obj);
    return result;
}

// argv_parser.h

TestResult test_parse_args()
{
    return TEST_NOT_IMPL;
}

#ifdef Nst_MSVC

TestResult test_wargv_to_argv()
{
    return TEST_NOT_IMPL;
}

#endif

// encoding.h

TestResult test_check_bytes()
{
    return TEST_NOT_IMPL;
}

TestResult test_to_utf32()
{
    return TEST_NOT_IMPL;
}

TestResult test_from_utf32()
{
    return TEST_NOT_IMPL;
}

TestResult test_utf16_to_utf8()
{
    return TEST_NOT_IMPL;
}

TestResult test_translate_cp()
{
    return TEST_NOT_IMPL;
}

TestResult test_check_string_cp()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_char_len()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_utf8_char_len()
{
    return TEST_NOT_IMPL;
}

TestResult test_char_to_wchar_t()
{
    return TEST_NOT_IMPL;
}

TestResult test_wchar_t_to_char()
{
    return TEST_NOT_IMPL;
}

TestResult test_is_valid_cp()
{
    return TEST_NOT_IMPL;
}

TestResult test_is_non_character()
{
    return TEST_NOT_IMPL;
}

TestResult test_check_bom()
{
    return TEST_NOT_IMPL;
}

TestResult test_detect_encoding()
{
    return TEST_NOT_IMPL;
}

TestResult test_encoding_from_name()
{
    return TEST_NOT_IMPL;
}

TestResult test_single_byte_cp()
{
    return TEST_NOT_IMPL;
}

// error.h

TestResult test_set_error()
{
    return TEST_NOT_IMPL;
}

TestResult test_set_error_c()
{
    return TEST_NOT_IMPL;
}

TestResult test_error_occurred()
{
    return TEST_NOT_IMPL;
}

TestResult test_error_get()
{
    return TEST_NOT_IMPL;
}

TestResult test_error_clear()
{
    return TEST_NOT_IMPL;
}

TestResult test_set_error_internal()
{
    return TEST_NOT_IMPL;
}

// file.h

TestResult test_FILE_read()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_write()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_flush()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_tell()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_seek()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_close()
{
    return TEST_NOT_IMPL;
}

TestResult test_io_result_details()
{
    return TEST_NOT_IMPL;
}

TestResult test_fopen_unicode()
{
    return TEST_NOT_IMPL;
}

// format.h

TestResult test_sprintf()
{
    return TEST_NOT_IMPL;
}

TestResult test_fmt()
{
    ENTER_TEST;
    i8 *str = NULL;
    usize len = 0;

    str = Nst_fmt("Hello", 0, &len);
    fail_if(str_neq(str, "Hello"));
    fail_if(len != 5);
    str = Nst_fmt("", 0, &len);
    fail_if(str_neq(str, ""));
    fail_if(len != 0);
    str = Nst_fmt("{{", 0, &len);
    fail_if(str_neq(str, "{"));
    fail_if(len != 1);
    str = Nst_fmt("}}", 0, &len);
    fail_if(str_neq(str, "}"));
    fail_if(len != 1);
    str = Nst_fmt("{{}}", 0, &len);
    fail_if(str_neq(str, "{}"));
    fail_if(len != 2);

    // Formatting C strings

    str = Nst_fmt("{s}", 0, &len, "Hello");
    fail_if(str_neq(str, "Hello"));
    fail_if(len != 5);
    str = Nst_fmt("{s}", 0, &len, NULL);
    fail_if(str_neq(str, "(null)"));
    fail_if(len != 6);
    str = Nst_fmt("{s}", 0, NULL, "😀🎺");
    fail_if(str_neq(str, "😀🎺"));

    str = Nst_fmt("{s:r}", 0, NULL, "Hello");
    fail_if(str_neq(str, "'Hello'"));
    str = Nst_fmt("{s:r}", 0, NULL, "Hello\x10\a\b\x1b\f\n\r\t\v\\è😀🎺");
    fail_if(str_neq(str, "'Hello\\x10\\a\\b\\e\\f\\n\\r\\t\\v\\\\è😀🎺'"));
    str = Nst_fmt("{s:r}", 0, NULL, "Hello'''");
    fail_if(str_neq(str, "\"Hello'''\""));
    str = Nst_fmt("{s:r}", 0, NULL, "Hello\"\"\"");
    fail_if(str_neq(str, "'Hello\"\"\"'"));
    str = Nst_fmt("{s:r}", 0, NULL, "Hello'\"'");
    fail_if(str_neq(str, "\"Hello'\\\"'\""));
    str = Nst_fmt("{s:r}", 0, NULL, "Hello\"'\"");
    fail_if(str_neq(str, "'Hello\"\\'\"'"));
    str = Nst_fmt("{s:r}", 0, NULL, "\x0e");
    fail_if(str_neq(str, "'\\x0e'"));
    str = Nst_fmt("{s:rP}", 0, NULL, "\x0e");
    fail_if(str_neq(str, "'\\x0E'"));

    str = Nst_fmt("{s:R}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hello"));
    str = Nst_fmt("{s:R}", 0, NULL, "Hello\x10\a\b\x1b\f\n\r\t\v\\è😀🎺");
    fail_if(str_neq(str, "Hello\\x10\\a\\b\\e\\f\\n\\r\\t\\v\\\\è😀🎺"));
    str = Nst_fmt("{s:R}", 0, NULL, "Hello'''");
    fail_if(str_neq(str, "Hello'''"));
    str = Nst_fmt("{s:R}", 0, NULL, "Hello\"\"\"");
    fail_if(str_neq(str, "Hello\"\"\""));
    str = Nst_fmt("{s:R}", 0, NULL, "Hello'\"'");
    fail_if(str_neq(str, "Hello'\"'"));
    str = Nst_fmt("{s:R}", 0, NULL, "Hello\"'\"");
    fail_if(str_neq(str, "Hello\"'\""));
    str = Nst_fmt("{s:R}", 0, NULL, "\x0e");
    fail_if(str_neq(str, "\\x0e"));
    str = Nst_fmt("{s:RP}", 0, NULL, "\x0e");
    fail_if(str_neq(str, "\\x0E"));

    str = Nst_fmt("{s:a}", 0, NULL, "Hello");
    fail_if(str_neq(str, "'Hello'"));
    str = Nst_fmt("{s:a}", 0, NULL, "Hello\x10\a\b\x1b\f\n\r\t\v\\è😀🎺");
    fail_if(str_neq(str, "'Hello\\x10\\a\\b\\e\\f\\n\\r\\t\\v\\\\\\u00e8\\U01f600\\U01f3ba'"));
    str = Nst_fmt("{s:a}", 0, NULL, "Hello'''");
    fail_if(str_neq(str, "\"Hello'''\""));
    str = Nst_fmt("{s:a}", 0, NULL, "Hello\"\"\"");
    fail_if(str_neq(str, "'Hello\"\"\"'"));
    str = Nst_fmt("{s:a}", 0, NULL, "Hello'\"'");
    fail_if(str_neq(str, "\"Hello'\\\"'\""));
    str = Nst_fmt("{s:a}", 0, NULL, "Hello\"'\"");
    fail_if(str_neq(str, "'Hello\"\\'\"'"));
    str = Nst_fmt("{s:a}", 0, NULL, "\x0e");
    fail_if(str_neq(str, "'\\x0e'"));
    str = Nst_fmt("{s:aP}", 0, NULL, "\x0e");
    fail_if(str_neq(str, "'\\x0E'"));

    str = Nst_fmt("{s:A}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hello"));
    str = Nst_fmt("{s:A}", 0, NULL, "Hello\x10\a\b\x1b\f\n\r\t\v\\è😀🎺");
    fail_if(str_neq(str, "Hello\\x10\\a\\b\\e\\f\\n\\r\\t\\v\\\\\\u00e8\\U01f600\\U01f3ba"));
    str = Nst_fmt("{s:A}", 0, NULL, "Hello'''");
    fail_if(str_neq(str, "Hello'''"));
    str = Nst_fmt("{s:A}", 0, NULL, "Hello\"\"\"");
    fail_if(str_neq(str, "Hello\"\"\""));
    str = Nst_fmt("{s:A}", 0, NULL, "Hello'\"'");
    fail_if(str_neq(str, "Hello'\"'"));
    str = Nst_fmt("{s:A}", 0, NULL, "Hello\"'\"");
    fail_if(str_neq(str, "Hello\"'\""));
    str = Nst_fmt("{s:A}", 0, NULL, "\x0e");
    fail_if(str_neq(str, "\\x0e"));
    str = Nst_fmt("{s:AP}", 0, NULL, "\x0e");
    fail_if(str_neq(str, "\\x0E"));

    str = Nst_fmt("{s:.10}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hello"));
    str = Nst_fmt("{s:.5}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hello"));
    str = Nst_fmt("{s:.2}", 0, NULL, "Hello");
    fail_if(str_neq(str, "He..."));
    str = Nst_fmt("{s:.0}", 0, NULL, "Hello");
    fail_if(str_neq(str, "..."));
    str = Nst_fmt("{s:.2}", 0, NULL, "😀🎺");
    fail_if(str_neq(str, "😀🎺"));
    str = Nst_fmt("{s:.1}", 0, NULL, "😀🎺");
    fail_if(str_neq(str, "😀..."));
    str = Nst_fmt("{s:.0}", 0, NULL, "😀🎺");
    fail_if(str_neq(str, "..."));
    str = Nst_fmt("{s:.*}", 0, NULL, "Hello", 3);
    fail_if(str_neq(str, "Hel..."));

    str = Nst_fmt("{s:10}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hello     "));
    str = Nst_fmt("{s:10>}", 0, NULL, "Hello");
    fail_if(str_neq(str, "     Hello"));
    str = Nst_fmt("{s:10^}", 0, NULL, "Hello");
    fail_if(str_neq(str, "  Hello   "));
    str = Nst_fmt("{s:*}", 0, NULL, "Hello", 8);
    fail_if(str_neq(str, "Hello   "));
    str = Nst_fmt("{s:_.10}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hello....."));
    str = Nst_fmt("{s:_.10>}", 0, NULL, "Hello");
    fail_if(str_neq(str, ".....Hello"));
    str = Nst_fmt("{s:_.10^}", 0, NULL, "Hello");
    fail_if(str_neq(str, "..Hello..."));
    str = Nst_fmt("{s:_è10}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Helloèèèèè"));
    str = Nst_fmt("{s:_è10>}", 0, NULL, "Hello");
    fail_if(str_neq(str, "èèèèèHello"));
    str = Nst_fmt("{s:_è10^}", 0, NULL, "Hello");
    fail_if(str_neq(str, "èèHelloèèè"));
    str = Nst_fmt("{s:_😀10}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hello😀😀😀😀😀"));
    str = Nst_fmt("{s:_😀10>}", 0, NULL, "Hello");
    fail_if(str_neq(str, "😀😀😀😀😀Hello"));
    str = Nst_fmt("{s:_😀10^}", 0, NULL, "Hello");
    fail_if(str_neq(str, "😀😀Hello😀😀😀"));

    str = Nst_fmt("{s:6}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "🎺🎺    "));
    str = Nst_fmt("{s:6>}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "    🎺🎺"));
    str = Nst_fmt("{s:6^}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "  🎺🎺  "));
    str = Nst_fmt("{s:_.6}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "🎺🎺...."));
    str = Nst_fmt("{s:_.6>}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "....🎺🎺"));
    str = Nst_fmt("{s:_.6^}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "..🎺🎺.."));
    str = Nst_fmt("{s:_è6}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "🎺🎺èèèè"));
    str = Nst_fmt("{s:_è6>}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "èèèè🎺🎺"));
    str = Nst_fmt("{s:_è6^}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "èè🎺🎺èè"));
    str = Nst_fmt("{s:_😀6}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "🎺🎺😀😀😀😀"));
    str = Nst_fmt("{s:_😀6>}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "😀😀😀😀🎺🎺"));
    str = Nst_fmt("{s:_😀6^}", 0, NULL, "🎺🎺");
    fail_if(str_neq(str, "😀😀🎺🎺😀😀"));

    str = Nst_fmt("{s:9.3}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hel...   "));
    str = Nst_fmt("{s:*.3}", 0, NULL, "Hello", 9);
    fail_if(str_neq(str, "Hel...   "));
    str = Nst_fmt("{s:9.*}", 0, NULL, "Hello", 3);
    fail_if(str_neq(str, "Hel...   "));
    str = Nst_fmt("{s:*.*}", 0, NULL, "Hello", 9, 3);
    fail_if(str_neq(str, "Hel...   "));

    str = Nst_fmt("{s:c3}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hel"));
    str = Nst_fmt("{s:c7}", 0, NULL, "Hello");
    fail_if(str_neq(str, "Hello  "));
    str = Nst_fmt("{s:c3>}", 0, NULL, "Hello");
    fail_if(str_neq(str, "llo"));
    str = Nst_fmt("{s:c7>}", 0, NULL, "Hello");
    fail_if(str_neq(str, "  Hello"));
    str = Nst_fmt("{s:c3^}", 0, NULL, "Hello");
    fail_if(str_neq(str, "ell"));
    str = Nst_fmt("{s:c7^}", 0, NULL, "Hello");
    fail_if(str_neq(str, " Hello "));

    str = Nst_fmt("{s:c3}", 0, NULL, "àèìòù");
    fail_if(str_neq(str, "àèì"));
    str = Nst_fmt("{s:c7}", 0, NULL, "àèìòù");
    fail_if(str_neq(str, "àèìòù  "));
    str = Nst_fmt("{s:c3>}", 0, NULL, "àèìòù");
    fail_if(str_neq(str, "ìòù"));
    str = Nst_fmt("{s:c7>}", 0, NULL, "àèìòù");
    fail_if(str_neq(str, "  àèìòù"));
    str = Nst_fmt("{s:c3^}", 0, NULL, "àèìòù");
    fail_if(str_neq(str, "èìò"));
    str = Nst_fmt("{s:c7^}", 0, NULL, "àèìòù");
    fail_if(str_neq(str, " àèìòù "));

    // Formatting Unsigned Integers

    str = Nst_fmt("{i:u}", 0, NULL, 10u);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{l:u}", 0, NULL, 10ul);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{L:u}", 0, NULL, 10ull);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{i:u+}", 0, NULL, 10u);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{l:u+}", 0, NULL, 10ul);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{L:u+}", 0, NULL, 10ull);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{i:u }", 0, NULL, 10u);
    fail_if(str_neq(str, " 10"));
    str = Nst_fmt("{l:u }", 0, NULL, 10ul);
    fail_if(str_neq(str, " 10"));
    str = Nst_fmt("{L:u }", 0, NULL, 10ull);
    fail_if(str_neq(str, " 10"));

    str = Nst_fmt("{i:ubp}", 0, NULL, 10u);
    fail_if(str_neq(str, "0b1010"));
    str = Nst_fmt("{l:ubP}", 0, NULL, 10ul);
    fail_if(str_neq(str, "0B1010"));
    str = Nst_fmt("{i:ubp+}", 0, NULL, 10u);
    fail_if(str_neq(str, "+0b1010"));
    str = Nst_fmt("{l:ubP+}", 0, NULL, 10ul);
    fail_if(str_neq(str, "+0B1010"));
    str = Nst_fmt("{i:ubp }", 0, NULL, 10u);
    fail_if(str_neq(str, " 0b1010"));
    str = Nst_fmt("{l:ubP }", 0, NULL, 10ul);
    fail_if(str_neq(str, " 0B1010"));
    str = Nst_fmt("{i:uop}", 0, NULL, 10u);
    fail_if(str_neq(str, "0o12"));
    str = Nst_fmt("{l:uoP}", 0, NULL, 10ul);
    fail_if(str_neq(str, "0O12"));
    str = Nst_fmt("{i:up}", 0, NULL, 10u);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{l:uP}", 0, NULL, 10ul);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{i:uxp}", 0, NULL, 10u);
    fail_if(str_neq(str, "0xa"));
    str = Nst_fmt("{l:uxP}", 0, NULL, 10ul);
    fail_if(str_neq(str, "0Xa"));
    str = Nst_fmt("{i:uXp}", 0, NULL, 10u);
    fail_if(str_neq(str, "0xA"));
    str = Nst_fmt("{l:uXP}", 0, NULL, 10ul);
    fail_if(str_neq(str, "0XA"));

    str = Nst_fmt("{i:ub}", 0, NULL, 0u);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:ub}", 0, NULL, 0ul);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:ub}", 0, NULL, 0ull);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i:uo}", 0, NULL, 0u);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:uo}", 0, NULL, 0ul);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:uo}", 0, NULL, 0ull);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i:u}", 0, NULL, 0u);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:u}", 0, NULL, 0ul);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:u}", 0, NULL, 0ull);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i:ux}", 0, NULL, 0u);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:ux}", 0, NULL, 0ul);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:ux}", 0, NULL, 0ull);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i:uX}", 0, NULL, 0u);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:uX}", 0, NULL, 0ul);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:uX}", 0, NULL, 0ull);
    fail_if(str_neq(str, "0"));

    str = Nst_fmt("{i:ub}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{l:ub}", 0, NULL, 123456789ul);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{L:ub}", 0, NULL, 123456789ull);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{i:uo}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{l:uo}", 0, NULL, 123456789ul);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{L:uo}", 0, NULL, 123456789ull);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{i:u}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{l:u}", 0, NULL, 123456789ul);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{L:u}", 0, NULL, 123456789ull);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{i:ux}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{l:ux}", 0, NULL, 123456789ul);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{L:ux}", 0, NULL, 123456789ull);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{i:uX}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "75BCD15"));
    str = Nst_fmt("{l:uX}", 0, NULL, 123456789ul);
    fail_if(str_neq(str, "75BCD15"));
    str = Nst_fmt("{L:uX}", 0, NULL, 123456789ull);
    fail_if(str_neq(str, "75BCD15"));

    str = Nst_fmt("{i:ub}", 0, NULL, 4294967295u);
    fail_if(str_neq(str, "11111111111111111111111111111111"));
    str = Nst_fmt("{l:ub}", 0, NULL, 4294967295ul);
    fail_if(str_neq(str, "11111111111111111111111111111111"));
    str = Nst_fmt("{L:ub}", 0, NULL, 18446744073709551615ull);
    fail_if(str_neq(str, "1111111111111111111111111111111111111111111111111111111111111111"));
    str = Nst_fmt("{i:uo}", 0, NULL, 4294967295u);
    fail_if(str_neq(str, "37777777777"));
    str = Nst_fmt("{l:uo}", 0, NULL, 4294967295ul);
    fail_if(str_neq(str, "37777777777"));
    str = Nst_fmt("{L:uo}", 0, NULL, 18446744073709551615ull);
    fail_if(str_neq(str, "1777777777777777777777"));
    str = Nst_fmt("{i:u}", 0, NULL, 4294967295u);
    fail_if(str_neq(str, "4294967295"));
    str = Nst_fmt("{l:u}", 0, NULL, 4294967295ul);
    fail_if(str_neq(str, "4294967295"));
    str = Nst_fmt("{L:u}", 0, NULL, 18446744073709551615ull);
    fail_if(str_neq(str, "18446744073709551615"));
    str = Nst_fmt("{i:ux}", 0, NULL, 4294967295u);
    fail_if(str_neq(str, "ffffffff"));
    str = Nst_fmt("{l:ux}", 0, NULL, 4294967295ul);
    fail_if(str_neq(str, "ffffffff"));
    str = Nst_fmt("{L:ux}", 0, NULL, 18446744073709551615ull);
    fail_if(str_neq(str, "ffffffffffffffff"));
    str = Nst_fmt("{i:uX}", 0, NULL, 4294967295u);
    fail_if(str_neq(str, "FFFFFFFF"));
    str = Nst_fmt("{l:uX}", 0, NULL, 4294967295ul);
    fail_if(str_neq(str, "FFFFFFFF"));
    str = Nst_fmt("{L:uX}", 0, NULL, 18446744073709551615ull);
    fail_if(str_neq(str, "FFFFFFFFFFFFFFFF"));

    str = Nst_fmt("{i:ub''}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "111'01011011'11001101'00010101"));
    str = Nst_fmt("{i:uo''}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "726'746'425"));
    str = Nst_fmt("{i:u''}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "123'456'789"));
    str = Nst_fmt("{i:ux''}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "75b'cd15"));
    str = Nst_fmt("{i:uX''}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "75B'CD15"));
    str = Nst_fmt("{i:ub'è}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "111è01011011è11001101è00010101"));
    str = Nst_fmt("{i:uo'è}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "726è746è425"));
    str = Nst_fmt("{i:u'è}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "123è456è789"));
    str = Nst_fmt("{i:ux'è}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "75bècd15"));
    str = Nst_fmt("{i:uX'è}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "75BèCD15"));
    str = Nst_fmt("{i:ub'😀}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "111😀01011011😀11001101😀00010101"));
    str = Nst_fmt("{i:uo'😀}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "726😀746😀425"));
    str = Nst_fmt("{i:u'😀}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "123😀456😀789"));
    str = Nst_fmt("{i:ux'😀}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "75b😀cd15"));
    str = Nst_fmt("{i:uX'😀}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "75B😀CD15"));

    str = Nst_fmt("{i:ub'',2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "1'11'01'01'10'11'11'00'11'01'00'01'01'01"));
    str = Nst_fmt("{i:uo'',2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "7'26'74'64'25"));
    str = Nst_fmt("{i:u'',2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "1'23'45'67'89"));
    str = Nst_fmt("{i:ux'',2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "7'5b'cd'15"));
    str = Nst_fmt("{i:uX'',2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "7'5B'CD'15"));
    str = Nst_fmt("{i:ub'è,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "1è11è01è01è10è11è11è00è11è01è00è01è01è01"));
    str = Nst_fmt("{i:uo'è,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "7è26è74è64è25"));
    str = Nst_fmt("{i:u'è,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "1è23è45è67è89"));
    str = Nst_fmt("{i:ux'è,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "7è5bècdè15"));
    str = Nst_fmt("{i:uX'è,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "7è5BèCDè15"));
    str = Nst_fmt("{i:ub'😀,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "1😀11😀01😀01😀10😀11😀11😀00😀11😀01😀00😀01😀01😀01"));
    str = Nst_fmt("{i:uo'😀,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "7😀26😀74😀64😀25"));
    str = Nst_fmt("{i:u'😀,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "1😀23😀45😀67😀89"));
    str = Nst_fmt("{i:ux'😀,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "7😀5b😀cd😀15"));
    str = Nst_fmt("{i:uX'😀,2}", 0, NULL, 123456789u);
    fail_if(str_neq(str, "7😀5B😀CD😀15"));

    str = Nst_fmt("{i:ub.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "  1010"));
    str = Nst_fmt("{i:ub0.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "001010"));
    str = Nst_fmt("{i:ub+.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "+  1010"));
    str = Nst_fmt("{i:ub+0.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "+001010"));
    str = Nst_fmt("{i:ub .6}", 0, NULL, 10u);
    fail_if(str_neq(str, "   1010"));
    str = Nst_fmt("{i:ub 0.6}", 0, NULL, 10u);
    fail_if(str_neq(str, " 001010"));
    str = Nst_fmt("{i:uo.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "    12"));
    str = Nst_fmt("{i:uo0.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "000012"));
    str = Nst_fmt("{i:u.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "    10"));
    str = Nst_fmt("{i:u0.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "000010"));
    str = Nst_fmt("{i:ux.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "     a"));
    str = Nst_fmt("{i:ux0.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "00000a"));
    str = Nst_fmt("{i:uX.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "     A"));
    str = Nst_fmt("{i:uX0.6}", 0, NULL, 10u);
    fail_if(str_neq(str, "00000A"));

    str = Nst_fmt("{i:ub''.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, "   10'10"));
    str = Nst_fmt("{i:ub0''.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, "00'10'10"));
    str = Nst_fmt("{i:ub''+.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, "+   10'10"));
    str = Nst_fmt("{i:ub''+0.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, "+00'10'10"));
    str = Nst_fmt("{i:ub'' .6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, "    10'10"));
    str = Nst_fmt("{i:ub'' 0.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, " 00'10'10"));
    str = Nst_fmt("{i:uo''.6,2}", 0, NULL, 103u);
    fail_if(str_neq(str, "    1'47"));
    str = Nst_fmt("{i:uo''0.6,2}", 0, NULL, 103u);
    fail_if(str_neq(str, "00'01'47"));
    str = Nst_fmt("{i:u''.6,2}", 0, NULL, 103u);
    fail_if(str_neq(str, "    1'03"));
    str = Nst_fmt("{i:u''0.6,2}", 0, NULL, 103u);
    fail_if(str_neq(str, "00'01'03"));
    str = Nst_fmt("{i:ux''.6,2}", 0, NULL, 1744u);
    fail_if(str_neq(str, "    6'd0"));
    str = Nst_fmt("{i:ux''0.6,2}", 0, NULL, 1744u);
    fail_if(str_neq(str, "00'06'd0"));
    str = Nst_fmt("{i:uX''.6,2}", 0, NULL, 1744u);
    fail_if(str_neq(str, "    6'D0"));
    str = Nst_fmt("{i:uX''0.6,2}", 0, NULL, 1744u);
    fail_if(str_neq(str, "00'06'D0"));

    str = Nst_fmt("{i:ubp''.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, "0b   10'10"));
    str = Nst_fmt("{i:ubP0''.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, "0B00'10'10"));
    str = Nst_fmt("{i:ubp''+.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, "+0b   10'10"));
    str = Nst_fmt("{i:ubP''+0.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, "+0B00'10'10"));
    str = Nst_fmt("{i:ubp'' .6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, " 0b   10'10"));
    str = Nst_fmt("{i:ubP'' 0.6,2}", 0, NULL, 10u);
    fail_if(str_neq(str, " 0B00'10'10"));
    str = Nst_fmt("{i:uop''.6,2}", 0, NULL, 103u);
    fail_if(str_neq(str, "0o    1'47"));
    str = Nst_fmt("{i:uoP''0.6,2}", 0, NULL, 103u);
    fail_if(str_neq(str, "0O00'01'47"));
    str = Nst_fmt("{i:up''.6,2}", 0, NULL, 103u);
    fail_if(str_neq(str, "    1'03"));
    str = Nst_fmt("{i:uP''0.6,2}", 0, NULL, 103u);
    fail_if(str_neq(str, "00'01'03"));
    str = Nst_fmt("{i:uxp''.6,2}", 0, NULL, 1744u);
    fail_if(str_neq(str, "0x    6'd0"));
    str = Nst_fmt("{i:uxP''0.6,2}", 0, NULL, 1744u);
    fail_if(str_neq(str, "0X00'06'd0"));
    str = Nst_fmt("{i:uXp''.6,2}", 0, NULL, 1744u);
    fail_if(str_neq(str, "0x    6'D0"));
    str = Nst_fmt("{i:uXP''0.6,2}", 0, NULL, 1744u);
    fail_if(str_neq(str, "0X00'06'D0"));

    // Formatting Signed Integers

    str = Nst_fmt("{i}", 0, NULL, 10);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{l}", 0, NULL, 10l);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{L}", 0, NULL, 10ll);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{z}", 0, NULL, (isize)10);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{i:+}", 0, NULL, 10);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{l:+}", 0, NULL, 10l);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{L:+}", 0, NULL, 10ll);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{z:+}", 0, NULL, (isize)10);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{i: }", 0, NULL, 10);
    fail_if(str_neq(str, " 10"));
    str = Nst_fmt("{l: }", 0, NULL, 10l);
    fail_if(str_neq(str, " 10"));
    str = Nst_fmt("{L: }", 0, NULL, 10ll);
    fail_if(str_neq(str, " 10"));
    str = Nst_fmt("{z: }", 0, NULL, (isize)10);
    fail_if(str_neq(str, " 10"));
    str = Nst_fmt("{i}", 0, NULL, -10);
    fail_if(str_neq(str, "-10"));
    str = Nst_fmt("{l}", 0, NULL, -10l);
    fail_if(str_neq(str, "-10"));
    str = Nst_fmt("{L}", 0, NULL, -10ll);
    fail_if(str_neq(str, "-10"));
    str = Nst_fmt("{z}", 0, NULL, (isize)-10);
    fail_if(str_neq(str, "-10"));

    str = Nst_fmt("{i:bp}", 0, NULL, 10);
    fail_if(str_neq(str, "0b1010"));
    str = Nst_fmt("{l:bP}", 0, NULL, 10l);
    fail_if(str_neq(str, "0B1010"));
    str = Nst_fmt("{i:bp+}", 0, NULL, 10);
    fail_if(str_neq(str, "+0b1010"));
    str = Nst_fmt("{l:bP+}", 0, NULL, 10l);
    fail_if(str_neq(str, "+0B1010"));
    str = Nst_fmt("{i:bp }", 0, NULL, 10);
    fail_if(str_neq(str, " 0b1010"));
    str = Nst_fmt("{l:bP }", 0, NULL, 10l);
    fail_if(str_neq(str, " 0B1010"));
    str = Nst_fmt("{i:op}", 0, NULL, 10);
    fail_if(str_neq(str, "0o12"));
    str = Nst_fmt("{l:oP}", 0, NULL, 10l);
    fail_if(str_neq(str, "0O12"));
    str = Nst_fmt("{i:p}", 0, NULL, 10);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{l:P}", 0, NULL, 10l);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{i:xp}", 0, NULL, 10);
    fail_if(str_neq(str, "0xa"));
    str = Nst_fmt("{l:xP}", 0, NULL, 10l);
    fail_if(str_neq(str, "0Xa"));
    str = Nst_fmt("{i:Xp}", 0, NULL, 10);
    fail_if(str_neq(str, "0xA"));
    str = Nst_fmt("{l:XP}", 0, NULL, 10l);
    fail_if(str_neq(str, "0XA"));

    str = Nst_fmt("{i:bp}", 0, NULL, -10);
    fail_if(str_neq(str, "-0b1010"));
    str = Nst_fmt("{l:bP}", 0, NULL, -10l);
    fail_if(str_neq(str, "-0B1010"));
    str = Nst_fmt("{i:op}", 0, NULL, -10);
    fail_if(str_neq(str, "-0o12"));
    str = Nst_fmt("{l:oP}", 0, NULL, -10l);
    fail_if(str_neq(str, "-0O12"));
    str = Nst_fmt("{i:p}", 0, NULL, -10);
    fail_if(str_neq(str, "-10"));
    str = Nst_fmt("{l:P}", 0, NULL, -10l);
    fail_if(str_neq(str, "-10"));
    str = Nst_fmt("{i:xp}", 0, NULL, -10);
    fail_if(str_neq(str, "-0xa"));
    str = Nst_fmt("{l:xP}", 0, NULL, -10l);
    fail_if(str_neq(str, "-0Xa"));
    str = Nst_fmt("{i:Xp}", 0, NULL, -10);
    fail_if(str_neq(str, "-0xA"));
    str = Nst_fmt("{l:XP}", 0, NULL, -10l);
    fail_if(str_neq(str, "-0XA"));

    str = Nst_fmt("{i:b}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:b}", 0, NULL, 0l);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:b}", 0, NULL, 0ll);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{z:b}", 0, NULL, (isize)0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i:o}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:o}", 0, NULL, 0l);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:o}", 0, NULL, 0ll);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{z:o}", 0, NULL, (isize)0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l}", 0, NULL, 0l);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L}", 0, NULL, 0ll);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{z}", 0, NULL, (isize)0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i:x}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:x}", 0, NULL, 0l);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:x}", 0, NULL, 0ll);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{z:x}", 0, NULL, (isize)0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i:X}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:X}", 0, NULL, 0l);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:X}", 0, NULL, 0ll);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{z:X}", 0, NULL, (isize)0);
    fail_if(str_neq(str, "0"));

    str = Nst_fmt("{i:b}", 0, NULL, 123456789);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{l:b}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{L:b}", 0, NULL, 123456789ll);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{z:b}", 0, NULL, (isize)123456789);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{i:o}", 0, NULL, 123456789);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{l:o}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{L:o}", 0, NULL, 123456789ll);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{z:o}", 0, NULL, (isize)123456789);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{i}", 0, NULL, 123456789);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{l}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{L}", 0, NULL, 123456789ll);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{z}", 0, NULL, (isize)123456789);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{i:x}", 0, NULL, 123456789);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{l:x}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{L:x}", 0, NULL, 123456789ll);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{z:x}", 0, NULL, (isize)123456789);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{i:X}", 0, NULL, 123456789);
    fail_if(str_neq(str, "75BCD15"));
    str = Nst_fmt("{l:X}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "75BCD15"));
    str = Nst_fmt("{L:X}", 0, NULL, 123456789ll);
    fail_if(str_neq(str, "75BCD15"));
    str = Nst_fmt("{z:X}", 0, NULL, (isize)123456789);
    fail_if(str_neq(str, "75BCD15"));

    str = Nst_fmt("{i:b}", 0, NULL, 2147483647);
    fail_if(str_neq(str, "1111111111111111111111111111111"));
    str = Nst_fmt("{l:b}", 0, NULL, 2147483647l);
    fail_if(str_neq(str, "1111111111111111111111111111111"));
    str = Nst_fmt("{L:b}", 0, NULL, 9223372036854775807ll);
    fail_if(str_neq(str, "111111111111111111111111111111111111111111111111111111111111111"));
    str = Nst_fmt("{i:o}", 0, NULL, 2147483647);
    fail_if(str_neq(str, "17777777777"));
    str = Nst_fmt("{l:o}", 0, NULL, 2147483647l);
    fail_if(str_neq(str, "17777777777"));
    str = Nst_fmt("{L:o}", 0, NULL, 9223372036854775807ll);
    fail_if(str_neq(str, "777777777777777777777"));
    str = Nst_fmt("{i}", 0, NULL, 2147483647);
    fail_if(str_neq(str, "2147483647"));
    str = Nst_fmt("{l}", 0, NULL, 2147483647l);
    fail_if(str_neq(str, "2147483647"));
    str = Nst_fmt("{L}", 0, NULL, 9223372036854775807ll);
    fail_if(str_neq(str, "9223372036854775807"));
    str = Nst_fmt("{i:x}", 0, NULL, 2147483647);
    fail_if(str_neq(str, "7fffffff"));
    str = Nst_fmt("{l:x}", 0, NULL, 2147483647l);
    fail_if(str_neq(str, "7fffffff"));
    str = Nst_fmt("{L:x}", 0, NULL, 9223372036854775807ll);
    fail_if(str_neq(str, "7fffffffffffffff"));
    str = Nst_fmt("{i:X}", 0, NULL, 2147483647);
    fail_if(str_neq(str, "7FFFFFFF"));
    str = Nst_fmt("{l:X}", 0, NULL, 2147483647l);
    fail_if(str_neq(str, "7FFFFFFF"));
    str = Nst_fmt("{L:X}", 0, NULL, 9223372036854775807ll);
    fail_if(str_neq(str, "7FFFFFFFFFFFFFFF"));

    str = Nst_fmt("{i:b}", 0, NULL, -2147483647 - 1);
    fail_if(str_neq(str, "-10000000000000000000000000000000"));
    str = Nst_fmt("{l:b}", 0, NULL, -2147483647l - 1l);
    fail_if(str_neq(str, "-10000000000000000000000000000000"));
    str = Nst_fmt("{L:b}", 0, NULL, -9223372036854775807ll - 1);
    fail_if(str_neq(str, "-1000000000000000000000000000000000000000000000000000000000000000"));
    str = Nst_fmt("{i:o}", 0, NULL, -2147483647 - 1);
    fail_if(str_neq(str, "-20000000000"));
    str = Nst_fmt("{l:o}", 0, NULL, -2147483647l - 1l);
    fail_if(str_neq(str, "-20000000000"));
    str = Nst_fmt("{L:o}", 0, NULL, -9223372036854775807ll - 1);
    fail_if(str_neq(str, "-1000000000000000000000"));
    str = Nst_fmt("{i}", 0, NULL, -2147483647 - 1);
    fail_if(str_neq(str, "-2147483648"));
    str = Nst_fmt("{l}", 0, NULL, -2147483647l - 1l);
    fail_if(str_neq(str, "-2147483648"));
    str = Nst_fmt("{L}", 0, NULL, -9223372036854775807ll - 1);
    fail_if(str_neq(str, "-9223372036854775808"));
    str = Nst_fmt("{i:x}", 0, NULL, -2147483647 - 1);
    fail_if(str_neq(str, "-80000000"));
    str = Nst_fmt("{l:x}", 0, NULL, -2147483647l - 1l);
    fail_if(str_neq(str, "-80000000"));
    str = Nst_fmt("{L:x}", 0, NULL, -9223372036854775807ll - 1);
    fail_if(str_neq(str, "-8000000000000000"));
    str = Nst_fmt("{i:X}", 0, NULL, -2147483647 - 1);
    fail_if(str_neq(str, "-80000000"));
    str = Nst_fmt("{l:X}", 0, NULL, -2147483647l - 1l);
    fail_if(str_neq(str, "-80000000"));
    str = Nst_fmt("{L:X}", 0, NULL, -9223372036854775807ll - 1);
    fail_if(str_neq(str, "-8000000000000000"));

    // Formatting bytes

    str = Nst_fmt("{B:b}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{B:o}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{B}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{B:x}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{B:X}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));

    str = Nst_fmt("{B:b}", 0, NULL, 10);
    fail_if(str_neq(str, "1010"));
    str = Nst_fmt("{B:o}", 0, NULL, 10);
    fail_if(str_neq(str, "12"));
    str = Nst_fmt("{B}", 0, NULL, 10);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{B:x}", 0, NULL, 10);
    fail_if(str_neq(str, "a"));
    str = Nst_fmt("{B:X}", 0, NULL, 10);
    fail_if(str_neq(str, "A"));

    str = Nst_fmt("{B:b}", 0, NULL, 255);
    fail_if(str_neq(str, "11111111"));
    str = Nst_fmt("{B:o}", 0, NULL, 255);
    fail_if(str_neq(str, "377"));
    str = Nst_fmt("{B}", 0, NULL, 255);
    fail_if(str_neq(str, "255"));
    str = Nst_fmt("{B:x}", 0, NULL, 255);
    fail_if(str_neq(str, "ff"));
    str = Nst_fmt("{B:X}", 0, NULL, 255);
    fail_if(str_neq(str, "FF"));

    str = Nst_fmt("{B:bp}", 0, NULL, 10);
    fail_if(str_neq(str, "0b1010b"));
    str = Nst_fmt("{B:op}", 0, NULL, 10);
    fail_if(str_neq(str, "0o12b"));
    str = Nst_fmt("{B:p}", 0, NULL, 10);
    fail_if(str_neq(str, "10b"));
    str = Nst_fmt("{B:xp}", 0, NULL, 10);
    fail_if(str_neq(str, "0ha"));
    str = Nst_fmt("{B:Xp}", 0, NULL, 10);
    fail_if(str_neq(str, "0hA"));

    str = Nst_fmt("{B:bP}", 0, NULL, 10);
    fail_if(str_neq(str, "0B1010B"));
    str = Nst_fmt("{B:oP}", 0, NULL, 10);
    fail_if(str_neq(str, "0O12B"));
    str = Nst_fmt("{B:P}", 0, NULL, 10);
    fail_if(str_neq(str, "10B"));
    str = Nst_fmt("{B:xP}", 0, NULL, 10);
    fail_if(str_neq(str, "0Ha"));
    str = Nst_fmt("{B:XP}", 0, NULL, 10);
    fail_if(str_neq(str, "0HA"));

    // Formatting Floating Point Numbers

    str = Nst_fmt("{f}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.23457"));
    str = Nst_fmt("{f}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12.3457"));
    str = Nst_fmt("{f}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "123.457"));
    str = Nst_fmt("{f}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1234.57"));
    str = Nst_fmt("{f}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "12345.7"));
    str = Nst_fmt("{f}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123457"));
    str = Nst_fmt("{f}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1.23457e+06"));
    str = Nst_fmt("{f}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.123457"));
    str = Nst_fmt("{f}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.0123457"));
    str = Nst_fmt("{f}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0.00123457"));
    str = Nst_fmt("{f}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0.000123457"));
    str = Nst_fmt("{f}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "1.23457e-05"));
    str = Nst_fmt("{f}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "1.23457e-06"));
    str = Nst_fmt("{f}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "1.23457e-07"));

    str = Nst_fmt("{f:.2}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.2"));
    str = Nst_fmt("{f:.2}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12"));
    str = Nst_fmt("{f:.2}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "1.2e+02"));
    str = Nst_fmt("{f:.2}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1.2e+03"));
    str = Nst_fmt("{f:.2}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "1.2e+04"));
    str = Nst_fmt("{f:.2}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "1.2e+05"));
    str = Nst_fmt("{f:.2}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1.2e+06"));
    str = Nst_fmt("{f:.2}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.12"));
    str = Nst_fmt("{f:.2}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.012"));
    str = Nst_fmt("{f:.2}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0.0012"));
    str = Nst_fmt("{f:.2}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0.00012"));
    str = Nst_fmt("{f:.2}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "1.2e-05"));
    str = Nst_fmt("{f:.2}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "1.2e-06"));
    str = Nst_fmt("{f:.2}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "1.2e-07"));

    str = Nst_fmt("{f:''.7}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123'456.7"));

    str = Nst_fmt("{f}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1"));
    str = Nst_fmt("{f}", 0, NULL, 10.0);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{f}", 0, NULL, 100.0);
    fail_if(str_neq(str, "100"));
    str = Nst_fmt("{f}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1000"));
    str = Nst_fmt("{f}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "10000"));
    str = Nst_fmt("{f}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "100000"));
    str = Nst_fmt("{f}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1e+06"));
    str = Nst_fmt("{f}", 0, NULL, 0.1);
    fail_if(str_neq(str, "0.1"));
    str = Nst_fmt("{f}", 0, NULL, 0.01);
    fail_if(str_neq(str, "0.01"));
    str = Nst_fmt("{f}", 0, NULL, 0.001);
    fail_if(str_neq(str, "0.001"));
    str = Nst_fmt("{f}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "0.0001"));
    str = Nst_fmt("{f}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "1e-05"));
    str = Nst_fmt("{f}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "1e-06"));
    str = Nst_fmt("{f}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "1e-07"));

    str = Nst_fmt("{f:G}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.23457"));
    str = Nst_fmt("{f:G}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12.3457"));
    str = Nst_fmt("{f:G}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "123.457"));
    str = Nst_fmt("{f:G}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1234.57"));
    str = Nst_fmt("{f:G}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "12345.7"));
    str = Nst_fmt("{f:G}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123456.7"));
    str = Nst_fmt("{f:G}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1.23457e+06"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.123457"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.0123457"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0.00123457"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0.000123457"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "1.23457e-05"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "1.23457e-06"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "1.23457e-07"));

    str = Nst_fmt("{f:G}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1.0"));
    str = Nst_fmt("{f:G}", 0, NULL, 10.0);
    fail_if(str_neq(str, "10.0"));
    str = Nst_fmt("{f:G}", 0, NULL, 100.0);
    fail_if(str_neq(str, "100.0"));
    str = Nst_fmt("{f:G}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1000.0"));
    str = Nst_fmt("{f:G}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "10000.0"));
    str = Nst_fmt("{f:G}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "100000.0"));
    str = Nst_fmt("{f:G}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1.0e+06"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.1);
    fail_if(str_neq(str, "0.1"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.01);
    fail_if(str_neq(str, "0.01"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.001);
    fail_if(str_neq(str, "0.001"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "0.0001"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "1.0e-05"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "1.0e-06"));
    str = Nst_fmt("{f:G}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "1.0e-07"));

    str = Nst_fmt("{f:0}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.23457"));
    str = Nst_fmt("{f:0}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12.3457"));
    str = Nst_fmt("{f:0}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "123.457"));
    str = Nst_fmt("{f:0}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1234.57"));
    str = Nst_fmt("{f:0}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "12345.7"));
    str = Nst_fmt("{f:0}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123457"));
    str = Nst_fmt("{f:0}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1.23457e+06"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.123457"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.0123457"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0.00123457"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0.000123457"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "1.23457e-05"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "1.23457e-06"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "1.23457e-07"));

    str = Nst_fmt("{f:0}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1.00000"));
    str = Nst_fmt("{f:0}", 0, NULL, 10.0);
    fail_if(str_neq(str, "10.0000"));
    str = Nst_fmt("{f:0}", 0, NULL, 100.0);
    fail_if(str_neq(str, "100.000"));
    str = Nst_fmt("{f:0}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1000.00"));
    str = Nst_fmt("{f:0}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "10000.0"));
    str = Nst_fmt("{f:0}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "100000"));
    str = Nst_fmt("{f:0}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1.00000e+06"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.1);
    fail_if(str_neq(str, "0.100000"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.01);
    fail_if(str_neq(str, "0.0100000"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.001);
    fail_if(str_neq(str, "0.00100000"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "0.000100000"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "1.00000e-05"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "1.00000e-06"));
    str = Nst_fmt("{f:0}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "1.00000e-07"));

    str = Nst_fmt("{f:G0}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.23457"));
    str = Nst_fmt("{f:G0}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12.3457"));
    str = Nst_fmt("{f:G0}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "123.457"));
    str = Nst_fmt("{f:G0}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1234.57"));
    str = Nst_fmt("{f:G0}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "12345.7"));
    str = Nst_fmt("{f:G0}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123456.7"));
    str = Nst_fmt("{f:G0}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1.23457e+06"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.123457"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.0123457"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0.00123457"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0.000123457"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "1.23457e-05"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "1.23457e-06"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "1.23457e-07"));

    str = Nst_fmt("{f:G0}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1.00000"));
    str = Nst_fmt("{f:G0}", 0, NULL, 10.0);
    fail_if(str_neq(str, "10.0000"));
    str = Nst_fmt("{f:G0}", 0, NULL, 100.0);
    fail_if(str_neq(str, "100.000"));
    str = Nst_fmt("{f:G0}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1000.00"));
    str = Nst_fmt("{f:G0}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "10000.0"));
    str = Nst_fmt("{f:G0}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "100000.0"));
    str = Nst_fmt("{f:G0}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1.00000e+06"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.1);
    fail_if(str_neq(str, "0.100000"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.01);
    fail_if(str_neq(str, "0.0100000"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.001);
    fail_if(str_neq(str, "0.00100000"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "0.000100000"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "1.00000e-05"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "1.00000e-06"));
    str = Nst_fmt("{f:G0}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "1.00000e-07"));

    str = Nst_fmt("{f:f}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.234567"));
    str = Nst_fmt("{f:f}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12.34567"));
    str = Nst_fmt("{f:f}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "123.4567"));
    str = Nst_fmt("{f:f}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1234.567"));
    str = Nst_fmt("{f:f}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "12345.67"));
    str = Nst_fmt("{f:f}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123456.7"));
    str = Nst_fmt("{f:f}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1234567"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.123457"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.012346"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0.001235"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0.000123"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "0.000012"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "0.000001"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "0"));

    str = Nst_fmt("{f:f.2}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.23"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12.35"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "123.46"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1234.57"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "12345.67"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123456.7"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1234567"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.12"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.01"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{f:f.2}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "0"));

    str = Nst_fmt("{f:f''.1}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123'456.7"));

    str = Nst_fmt("{f:f}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1"));
    str = Nst_fmt("{f:f}", 0, NULL, 10.0);
    fail_if(str_neq(str, "10"));
    str = Nst_fmt("{f:f}", 0, NULL, 100.0);
    fail_if(str_neq(str, "100"));
    str = Nst_fmt("{f:f}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1000"));
    str = Nst_fmt("{f:f}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "10000"));
    str = Nst_fmt("{f:f}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "100000"));
    str = Nst_fmt("{f:f}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1000000"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.1);
    fail_if(str_neq(str, "0.1"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.01);
    fail_if(str_neq(str, "0.01"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.001);
    fail_if(str_neq(str, "0.001"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "0.0001"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "0.00001"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "0.000001"));
    str = Nst_fmt("{f:f}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "0"));

    str = Nst_fmt("{f:F}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.234567"));
    str = Nst_fmt("{f:F}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12.34567"));
    str = Nst_fmt("{f:F}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "123.4567"));
    str = Nst_fmt("{f:F}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1234.567"));
    str = Nst_fmt("{f:F}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "12345.67"));
    str = Nst_fmt("{f:F}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123456.7"));
    str = Nst_fmt("{f:F}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1234567.0"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.123457"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.012346"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0.001235"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0.000123"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "0.000012"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "0.000001"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "0.0"));

    str = Nst_fmt("{f:F}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1.0"));
    str = Nst_fmt("{f:F}", 0, NULL, 10.0);
    fail_if(str_neq(str, "10.0"));
    str = Nst_fmt("{f:F}", 0, NULL, 100.0);
    fail_if(str_neq(str, "100.0"));
    str = Nst_fmt("{f:F}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1000.0"));
    str = Nst_fmt("{f:F}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "10000.0"));
    str = Nst_fmt("{f:F}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "100000.0"));
    str = Nst_fmt("{f:F}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1000000.0"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.1);
    fail_if(str_neq(str, "0.1"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.01);
    fail_if(str_neq(str, "0.01"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.001);
    fail_if(str_neq(str, "0.001"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "0.0001"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "0.00001"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "0.000001"));
    str = Nst_fmt("{f:F}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "0.0"));

    str = Nst_fmt("{f:f0}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.234567"));
    str = Nst_fmt("{f:f0}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12.345670"));
    str = Nst_fmt("{f:f0}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "123.456700"));
    str = Nst_fmt("{f:f0}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1234.567000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "12345.670000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123456.700000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1234567.000000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.123457"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.012346"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0.001235"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0.000123"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "0.000012"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "0.000001"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "0.000000"));

    str = Nst_fmt("{f:f0}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1.000000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 10.0);
    fail_if(str_neq(str, "10.000000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 100.0);
    fail_if(str_neq(str, "100.000000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1000.000000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "10000.000000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "100000.000000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1000000.000000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.1);
    fail_if(str_neq(str, "0.100000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.01);
    fail_if(str_neq(str, "0.010000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.001);
    fail_if(str_neq(str, "0.001000"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "0.000100"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "0.000010"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "0.000001"));
    str = Nst_fmt("{f:f0}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "0.000000"));

    str = Nst_fmt("{f:F0}", 0, NULL, 1.234567);
    fail_if(str_neq(str, "1.234567"));
    str = Nst_fmt("{f:F0}", 0, NULL, 12.34567);
    fail_if(str_neq(str, "12.345670"));
    str = Nst_fmt("{f:F0}", 0, NULL, 123.4567);
    fail_if(str_neq(str, "123.456700"));
    str = Nst_fmt("{f:F0}", 0, NULL, 1234.567);
    fail_if(str_neq(str, "1234.567000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 12345.67);
    fail_if(str_neq(str, "12345.670000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 123456.7);
    fail_if(str_neq(str, "123456.700000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 1234567.0);
    fail_if(str_neq(str, "1234567.000000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.1234567);
    fail_if(str_neq(str, "0.123457"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.01234567);
    fail_if(str_neq(str, "0.012346"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.001234567);
    fail_if(str_neq(str, "0.001235"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.0001234567);
    fail_if(str_neq(str, "0.000123"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.00001234567);
    fail_if(str_neq(str, "0.000012"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.000001234567);
    fail_if(str_neq(str, "0.000001"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.0000001234567);
    fail_if(str_neq(str, "0.000000"));

    str = Nst_fmt("{f:F0}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1.000000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 10.0);
    fail_if(str_neq(str, "10.000000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 100.0);
    fail_if(str_neq(str, "100.000000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1000.000000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "10000.000000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "100000.000000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1000000.000000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.1);
    fail_if(str_neq(str, "0.100000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.01);
    fail_if(str_neq(str, "0.010000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.001);
    fail_if(str_neq(str, "0.001000"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "0.000100"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "0.000010"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "0.000001"));
    str = Nst_fmt("{f:F0}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "0.000000"));

    str = Nst_fmt("{f:e}", 0, NULL, 1.357);
    fail_if(str_neq(str, "1.357e+00"));
    str = Nst_fmt("{f:e}", 0, NULL, 13.57);
    fail_if(str_neq(str, "1.357e+01"));
    str = Nst_fmt("{f:e}", 0, NULL, 135.7);
    fail_if(str_neq(str, "1.357e+02"));
    str = Nst_fmt("{f:e}", 0, NULL, 1357.0);
    fail_if(str_neq(str, "1.357e+03"));
    str = Nst_fmt("{f:e}", 0, NULL, 13570.0);
    fail_if(str_neq(str, "1.357e+04"));
    str = Nst_fmt("{f:e}", 0, NULL, 135700.0);
    fail_if(str_neq(str, "1.357e+05"));
    str = Nst_fmt("{f:e}", 0, NULL, 1357000.0);
    fail_if(str_neq(str, "1.357e+06"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.1357);
    fail_if(str_neq(str, "1.357e-01"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.01357);
    fail_if(str_neq(str, "1.357e-02"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.001357);
    fail_if(str_neq(str, "1.357e-03"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.0001357);
    fail_if(str_neq(str, "1.357e-04"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.00001357);
    fail_if(str_neq(str, "1.357e-05"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.000001357);
    fail_if(str_neq(str, "1.357e-06"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.0000001357);
    fail_if(str_neq(str, "1.357e-07"));

    str = Nst_fmt("{f:e.2}", 0, NULL, 1.357);
    fail_if(str_neq(str, "1.36e+00"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 13.57);
    fail_if(str_neq(str, "1.36e+01"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 135.7);
    fail_if(str_neq(str, "1.36e+02"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 1357.0);
    fail_if(str_neq(str, "1.36e+03"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 13570.0);
    fail_if(str_neq(str, "1.36e+04"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 135700.0);
    fail_if(str_neq(str, "1.36e+05"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 1357000.0);
    fail_if(str_neq(str, "1.36e+06"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 0.1357);
    fail_if(str_neq(str, "1.36e-01"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 0.01357);
    fail_if(str_neq(str, "1.36e-02"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 0.001357);
    fail_if(str_neq(str, "1.36e-03"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 0.0001357);
    fail_if(str_neq(str, "1.36e-04"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 0.00001357);
    fail_if(str_neq(str, "1.36e-05"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 0.000001357);
    fail_if(str_neq(str, "1.36e-06"));
    str = Nst_fmt("{f:e.2}", 0, NULL, 0.0000001357);
    fail_if(str_neq(str, "1.36e-07"));

    str = Nst_fmt("{f:e}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1e+00"));
    str = Nst_fmt("{f:e}", 0, NULL, 10.0);
    fail_if(str_neq(str, "1e+01"));
    str = Nst_fmt("{f:e}", 0, NULL, 100.0);
    fail_if(str_neq(str, "1e+02"));
    str = Nst_fmt("{f:e}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1e+03"));
    str = Nst_fmt("{f:e}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "1e+04"));
    str = Nst_fmt("{f:e}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "1e+05"));
    str = Nst_fmt("{f:e}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1e+06"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.1);
    fail_if(str_neq(str, "1e-01"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.01);
    fail_if(str_neq(str, "1e-02"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.001);
    fail_if(str_neq(str, "1e-03"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "1e-04"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "1e-05"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "1e-06"));
    str = Nst_fmt("{f:e}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "1e-07"));

    str = Nst_fmt("{f:E}", 0, NULL, 1.357);
    fail_if(str_neq(str, "1.357e+00"));
    str = Nst_fmt("{f:E}", 0, NULL, 13.57);
    fail_if(str_neq(str, "1.357e+01"));
    str = Nst_fmt("{f:E}", 0, NULL, 135.7);
    fail_if(str_neq(str, "1.357e+02"));
    str = Nst_fmt("{f:E}", 0, NULL, 1357.0);
    fail_if(str_neq(str, "1.357e+03"));
    str = Nst_fmt("{f:E}", 0, NULL, 13570.0);
    fail_if(str_neq(str, "1.357e+04"));
    str = Nst_fmt("{f:E}", 0, NULL, 135700.0);
    fail_if(str_neq(str, "1.357e+05"));
    str = Nst_fmt("{f:E}", 0, NULL, 1357000.0);
    fail_if(str_neq(str, "1.357e+06"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.1357);
    fail_if(str_neq(str, "1.357e-01"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.01357);
    fail_if(str_neq(str, "1.357e-02"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.001357);
    fail_if(str_neq(str, "1.357e-03"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.0001357);
    fail_if(str_neq(str, "1.357e-04"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.00001357);
    fail_if(str_neq(str, "1.357e-05"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.000001357);
    fail_if(str_neq(str, "1.357e-06"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.0000001357);
    fail_if(str_neq(str, "1.357e-07"));

    str = Nst_fmt("{f:E}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1.0e+00"));
    str = Nst_fmt("{f:E}", 0, NULL, 10.0);
    fail_if(str_neq(str, "1.0e+01"));
    str = Nst_fmt("{f:E}", 0, NULL, 100.0);
    fail_if(str_neq(str, "1.0e+02"));
    str = Nst_fmt("{f:E}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1.0e+03"));
    str = Nst_fmt("{f:E}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "1.0e+04"));
    str = Nst_fmt("{f:E}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "1.0e+05"));
    str = Nst_fmt("{f:E}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1.0e+06"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.1);
    fail_if(str_neq(str, "1.0e-01"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.01);
    fail_if(str_neq(str, "1.0e-02"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.001);
    fail_if(str_neq(str, "1.0e-03"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "1.0e-04"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "1.0e-05"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "1.0e-06"));
    str = Nst_fmt("{f:E}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "1.0e-07"));

    str = Nst_fmt("{f:e0}", 0, NULL, 1.357);
    fail_if(str_neq(str, "1.357000e+00"));
    str = Nst_fmt("{f:e0}", 0, NULL, 13.57);
    fail_if(str_neq(str, "1.357000e+01"));
    str = Nst_fmt("{f:e0}", 0, NULL, 135.7);
    fail_if(str_neq(str, "1.357000e+02"));
    str = Nst_fmt("{f:e0}", 0, NULL, 1357.0);
    fail_if(str_neq(str, "1.357000e+03"));
    str = Nst_fmt("{f:e0}", 0, NULL, 13570.0);
    fail_if(str_neq(str, "1.357000e+04"));
    str = Nst_fmt("{f:e0}", 0, NULL, 135700.0);
    fail_if(str_neq(str, "1.357000e+05"));
    str = Nst_fmt("{f:e0}", 0, NULL, 1357000.0);
    fail_if(str_neq(str, "1.357000e+06"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.1357);
    fail_if(str_neq(str, "1.357000e-01"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.01357);
    fail_if(str_neq(str, "1.357000e-02"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.001357);
    fail_if(str_neq(str, "1.357000e-03"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.0001357);
    fail_if(str_neq(str, "1.357000e-04"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.00001357);
    fail_if(str_neq(str, "1.357000e-05"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.000001357);
    fail_if(str_neq(str, "1.357000e-06"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.0000001357);
    fail_if(str_neq(str, "1.357000e-07"));

    str = Nst_fmt("{f:e0}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1.000000e+00"));
    str = Nst_fmt("{f:e0}", 0, NULL, 10.0);
    fail_if(str_neq(str, "1.000000e+01"));
    str = Nst_fmt("{f:e0}", 0, NULL, 100.0);
    fail_if(str_neq(str, "1.000000e+02"));
    str = Nst_fmt("{f:e0}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1.000000e+03"));
    str = Nst_fmt("{f:e0}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "1.000000e+04"));
    str = Nst_fmt("{f:e0}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "1.000000e+05"));
    str = Nst_fmt("{f:e0}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1.000000e+06"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.1);
    fail_if(str_neq(str, "1.000000e-01"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.01);
    fail_if(str_neq(str, "1.000000e-02"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.001);
    fail_if(str_neq(str, "1.000000e-03"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "1.000000e-04"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "1.000000e-05"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "1.000000e-06"));
    str = Nst_fmt("{f:e0}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "1.000000e-07"));

    str = Nst_fmt("{f:E0}", 0, NULL, 1.357);
    fail_if(str_neq(str, "1.357000e+00"));
    str = Nst_fmt("{f:E0}", 0, NULL, 13.57);
    fail_if(str_neq(str, "1.357000e+01"));
    str = Nst_fmt("{f:E0}", 0, NULL, 135.7);
    fail_if(str_neq(str, "1.357000e+02"));
    str = Nst_fmt("{f:E0}", 0, NULL, 1357.0);
    fail_if(str_neq(str, "1.357000e+03"));
    str = Nst_fmt("{f:E0}", 0, NULL, 13570.0);
    fail_if(str_neq(str, "1.357000e+04"));
    str = Nst_fmt("{f:E0}", 0, NULL, 135700.0);
    fail_if(str_neq(str, "1.357000e+05"));
    str = Nst_fmt("{f:E0}", 0, NULL, 1357000.0);
    fail_if(str_neq(str, "1.357000e+06"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.1357);
    fail_if(str_neq(str, "1.357000e-01"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.01357);
    fail_if(str_neq(str, "1.357000e-02"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.001357);
    fail_if(str_neq(str, "1.357000e-03"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.0001357);
    fail_if(str_neq(str, "1.357000e-04"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.00001357);
    fail_if(str_neq(str, "1.357000e-05"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.000001357);
    fail_if(str_neq(str, "1.357000e-06"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.0000001357);
    fail_if(str_neq(str, "1.357000e-07"));

    str = Nst_fmt("{f:E0}", 0, NULL, 1.0);
    fail_if(str_neq(str, "1.000000e+00"));
    str = Nst_fmt("{f:E0}", 0, NULL, 10.0);
    fail_if(str_neq(str, "1.000000e+01"));
    str = Nst_fmt("{f:E0}", 0, NULL, 100.0);
    fail_if(str_neq(str, "1.000000e+02"));
    str = Nst_fmt("{f:E0}", 0, NULL, 1000.0);
    fail_if(str_neq(str, "1.000000e+03"));
    str = Nst_fmt("{f:E0}", 0, NULL, 10000.0);
    fail_if(str_neq(str, "1.000000e+04"));
    str = Nst_fmt("{f:E0}", 0, NULL, 100000.0);
    fail_if(str_neq(str, "1.000000e+05"));
    str = Nst_fmt("{f:E0}", 0, NULL, 1000000.0);
    fail_if(str_neq(str, "1.000000e+06"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.1);
    fail_if(str_neq(str, "1.000000e-01"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.01);

    fail_if(str_neq(str, "1.000000e-02"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.001);
    fail_if(str_neq(str, "1.000000e-03"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.0001);
    fail_if(str_neq(str, "1.000000e-04"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.00001);
    fail_if(str_neq(str, "1.000000e-05"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.000001);
    fail_if(str_neq(str, "1.000000e-06"));
    str = Nst_fmt("{f:E0}", 0, NULL, 0.0000001);
    fail_if(str_neq(str, "1.000000e-07"));

    str = Nst_fmt("{f}", 0, NULL, INFINITY);
    fail_if(str_neq(str, "Inf"));
    str = Nst_fmt("{f}", 0, NULL, -INFINITY);
    fail_if(str_neq(str, "-Inf"));
    str = Nst_fmt("{f:p}", 0, NULL, INFINITY);
    fail_if(str_neq(str, "inf"));
    str = Nst_fmt("{f:p}", 0, NULL, -INFINITY);
    fail_if(str_neq(str, "-inf"));
    str = Nst_fmt("{f:P}", 0, NULL, INFINITY);
    fail_if(str_neq(str, "INF"));
    str = Nst_fmt("{f:P}", 0, NULL, -INFINITY);
    fail_if(str_neq(str, "-INF"));

    str = Nst_fmt("{f}", 0, NULL, NAN);
    fail_if(str_neq(str, "NaN"));
    str = Nst_fmt("{f}", 0, NULL, -NAN);
    fail_if(str_neq(str, "-NaN"));
    str = Nst_fmt("{f:p}", 0, NULL, NAN);
    fail_if(str_neq(str, "nan"));
    str = Nst_fmt("{f:p}", 0, NULL, -NAN);
    fail_if(str_neq(str, "-nan"));
    str = Nst_fmt("{f:P}", 0, NULL, NAN);
    fail_if(str_neq(str, "NAN"));
    str = Nst_fmt("{f:P}", 0, NULL, -NAN);
    fail_if(str_neq(str, "-NAN"));

    // Formatting Booleans

    str = Nst_fmt("{b}", 0, NULL, true);
    fail_if(str_neq(str, "true"));
    str = Nst_fmt("{b:8}", 0, NULL, true);
    fail_if(str_neq(str, "true    "));
    str = Nst_fmt("{b:8>}", 0, NULL, true);
    fail_if(str_neq(str, "    true"));
    str = Nst_fmt("{b:8^}", 0, NULL, true);
    fail_if(str_neq(str, "  true  "));
    str = Nst_fmt("{b:c2}", 0, NULL, true);
    fail_if(str_neq(str, "tr"));
    str = Nst_fmt("{b:c2>}", 0, NULL, true);
    fail_if(str_neq(str, "ue"));
    str = Nst_fmt("{b:c2^}", 0, NULL, true);
    fail_if(str_neq(str, "ru"));

    str = Nst_fmt("{b}", 0, NULL, false);
    fail_if(str_neq(str, "false"));
    str = Nst_fmt("{b:8}", 0, NULL, false);
    fail_if(str_neq(str, "false   "));
    str = Nst_fmt("{b:8>}", 0, NULL, false);
    fail_if(str_neq(str, "   false"));
    str = Nst_fmt("{b:8^}", 0, NULL, false);
    fail_if(str_neq(str, " false  "));
    str = Nst_fmt("{b:c2}", 0, NULL, false);
    fail_if(str_neq(str, "fa"));
    str = Nst_fmt("{b:c2>}", 0, NULL, false);
    fail_if(str_neq(str, "se"));
    str = Nst_fmt("{b:c2^}", 0, NULL, false);
    fail_if(str_neq(str, "al"));

    str = Nst_fmt("{b:P}", 0, NULL, true);
    fail_if(str_neq(str, "TRUE"));
    str = Nst_fmt("{b:P8}", 0, NULL, true);
    fail_if(str_neq(str, "TRUE    "));
    str = Nst_fmt("{b:P8>}", 0, NULL, true);
    fail_if(str_neq(str, "    TRUE"));
    str = Nst_fmt("{b:P8^}", 0, NULL, true);
    fail_if(str_neq(str, "  TRUE  "));
    str = Nst_fmt("{b:Pc2}", 0, NULL, true);
    fail_if(str_neq(str, "TR"));
    str = Nst_fmt("{b:Pc2>}", 0, NULL, true);
    fail_if(str_neq(str, "UE"));
    str = Nst_fmt("{b:Pc2^}", 0, NULL, true);
    fail_if(str_neq(str, "RU"));

    str = Nst_fmt("{b:P}", 0, NULL, false);
    fail_if(str_neq(str, "FALSE"));
    str = Nst_fmt("{b:P8}", 0, NULL, false);
    fail_if(str_neq(str, "FALSE   "));
    str = Nst_fmt("{b:P8>}", 0, NULL, false);
    fail_if(str_neq(str, "   FALSE"));
    str = Nst_fmt("{b:P8^}", 0, NULL, false);
    fail_if(str_neq(str, " FALSE  "));
    str = Nst_fmt("{b:Pc2}", 0, NULL, false);
    fail_if(str_neq(str, "FA"));
    str = Nst_fmt("{b:Pc2>}", 0, NULL, false);
    fail_if(str_neq(str, "SE"));
    str = Nst_fmt("{b:Pc2^}", 0, NULL, false);
    fail_if(str_neq(str, "AL"));

    // Formatting Pointers

#ifdef _Nst_ARCH_x64

    str = Nst_fmt("{p}", 0, NULL, (void *)0);
    fail_if(str_neq(str, "0x0"));
    str = Nst_fmt("{p:P}", 0, NULL, (void *)0);
    fail_if(str_neq(str, "0X0"));
    str = Nst_fmt("{p:0}", 0, NULL, (void *)0);
    fail_if(str_neq(str, "0x0000000000000000"));
    str = Nst_fmt("{p:0P}", 0, NULL, (void *)0);
    fail_if(str_neq(str, "0X0000000000000000"));

    str = Nst_fmt("{p}", 0, NULL, (void *)0x00007ffe0325c4e4);
    fail_if(str_neq(str, "0x7ffe0325c4e4"));
    str = Nst_fmt("{p:P}", 0, NULL, (void *)0x00007ffe0325c4e4);
    fail_if(str_neq(str, "0X7ffe0325c4e4"));
    str = Nst_fmt("{p:X}", 0, NULL, (void *)0x00007ffe0325c4e4);
    fail_if(str_neq(str, "0x7FFE0325C4E4"));
    str = Nst_fmt("{p:PX}", 0, NULL, (void *)0x00007ffe0325c4e4);
    fail_if(str_neq(str, "0X7FFE0325C4E4"));

    str = Nst_fmt("{p:0}", 0, NULL, (void *)0x00007ffe0325c4e4);
    fail_if(str_neq(str, "0x00007ffe0325c4e4"));
    str = Nst_fmt("{p:0P}", 0, NULL, (void *)0x00007ffe0325c4e4);
    fail_if(str_neq(str, "0X00007ffe0325c4e4"));
    str = Nst_fmt("{p:0X}", 0, NULL, (void *)0x00007ffe0325c4e4);
    fail_if(str_neq(str, "0x00007FFE0325C4E4"));
    str = Nst_fmt("{p:0PX}", 0, NULL, (void *)0x00007ffe0325c4e4);
    fail_if(str_neq(str, "0X00007FFE0325C4E4"));

#else

    str = Nst_fmt("{p}", 0, NULL, (void *)0);
    fail_if(str_neq(str, "0x0"));
    str = Nst_fmt("{p:P}", 0, NULL, (void *)0);
    fail_if(str_neq(str, "0X0"));
    str = Nst_fmt("{p:0}", 0, NULL, (void *)0);
    fail_if(str_neq(str, "0x00000000"));
    str = Nst_fmt("{p:0P}", 0, NULL, (void *)0);
    fail_if(str_neq(str, "0X00000000"));

    str = Nst_fmt("{p}", 0, NULL, (void *)0x0325c4e4);
    fail_if(str_neq(str, "0x325c4e4"));
    str = Nst_fmt("{p:P}", 0, NULL, (void *)0x0325c4e4);
    fail_if(str_neq(str, "0X325c4e4"));
    str = Nst_fmt("{p:X}", 0, NULL, (void *)0x0325c4e4);
    fail_if(str_neq(str, "0x325C4E4"));
    str = Nst_fmt("{p:PX}", 0, NULL, (void *)0x0325c4e4);
    fail_if(str_neq(str, "0X325C4E4"));

    str = Nst_fmt("{p:0}", 0, NULL, (void *)0x0325c4e4);
    fail_if(str_neq(str, "0x0325c4e4"));
    str = Nst_fmt("{p:0P}", 0, NULL, (void *)0x0325c4e4);
    fail_if(str_neq(str, "0X0325c4e4"));
    str = Nst_fmt("{p:0X}", 0, NULL, (void *)0x0325c4e4);
    fail_if(str_neq(str, "0x0325C4E4"));
    str = Nst_fmt("{p:0PX}", 0, NULL, (void *)0x0325c4e4);
    fail_if(str_neq(str, "0X0325C4E4"));

#endif

    // Formatting Characters

    str = Nst_fmt("{c}", 0, NULL, 'a');
    fail_if(str_neq(str, "a"));
    str = Nst_fmt("{c:a}", 0, NULL, '\xa');
    fail_if(str_neq(str, "'\\n'"));
    str = Nst_fmt("{c}", 0, NULL, '\xe8');
    fail_if(str_neq(str, "è"));

    EXIT_TEST;
}

// function.h

TestResult test_func_set_vt()
{
    return TEST_NOT_IMPL;
}

// hash.h

TestResult test_obj_hash()
{
    return TEST_NOT_IMPL;
}

// iter.h

TestResult test_iter_start()
{
    return TEST_NOT_IMPL;
}

TestResult test_iter_get_val()
{
    return TEST_NOT_IMPL;
}

// lib_import.h

TestResult test_extract_args()
{
    return TEST_NOT_IMPL;
}

// llist.h

TestResult test_llist_push()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_push(&llist, (void *)1, false);
    fail_if(llist.len != 1);
    Nst_llist_push(&llist, (void *)2, false);
    fail_if(llist.len != 2);
    Nst_llist_push(&llist, (void *)3, false);
    fail_if(llist.len != 3);

    fail_if(llist.head->value != (void *)3);
    fail_if(llist.head->next->value != (void *)2);
    fail_if(llist.tail->value != (void *)1);
    fail_if(llist.head->next->next != llist.tail);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_append()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    fail_if(llist.len != 1);
    Nst_llist_append(&llist, (void *)2, false);
    fail_if(llist.len != 2);
    Nst_llist_append(&llist, (void *)3, false);
    fail_if(llist.len != 3);

    fail_if(llist.head->value != (void *)1);
    fail_if(llist.head->next->value != (void *)2);
    fail_if(llist.tail->value != (void *)3);
    fail_if(llist.head->next->next != llist.tail);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_insert()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);

    Nst_llist_insert(&llist, (void *)3, false, llist.head);
    fail_if(llist.head->next->value != (void *)3);
    Nst_llist_insert(&llist, (void *)4, false, llist.tail);
    fail_if(llist.tail->value != (void *)4);
    Nst_llist_insert(&llist, (void *)5, false, NULL);
    fail_if(llist.head->value != (void *)5);

    fail_if(llist.len != 5);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_pop()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    fail_if(Nst_llist_pop(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_pop(&llist);
    fail_if(llist.len != 1);
    fail_if(value != (void *)1);
    Nst_llist_pop(&llist);
    fail_if(llist.head != NULL);
    fail_if(llist.tail != NULL);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_peek_front()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    fail_if(Nst_llist_peek_front(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_peek_front(&llist);
    fail_if(llist.len != 2);
    fail_if(value != (void *)1);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_peek_back()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    fail_if(Nst_llist_peek_back(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_peek_back(&llist);
    fail_if(llist.len != 2);
    fail_if(value != (void *)2);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_push_llnode()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = Nst_llnode_new((void *)1, false);
    Nst_LLNode *node2 = Nst_llnode_new((void *)2, false);
    Nst_llist_push_llnode(&llist, node1);
    fail_if(llist.len != 1);
    fail_if(llist.head != node1);
    fail_if(llist.tail != node1);
    Nst_llist_push_llnode(&llist, node2);
    fail_if(llist.len != 2);
    fail_if(llist.head != node2);
    fail_if(llist.tail != node1);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_append_llnode()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = Nst_llnode_new((void *)1, false);
    Nst_LLNode *node2 = Nst_llnode_new((void *)2, false);
    Nst_llist_append_llnode(&llist, node1);
    fail_if(llist.len != 1);
    fail_if(llist.head != node1);
    fail_if(llist.tail != node1);
    Nst_llist_append_llnode(&llist, node2);
    fail_if(llist.len != 2);
    fail_if(llist.head != node1);
    fail_if(llist.tail != node2);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_pop_llnode()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = NULL;
    Nst_LLNode *node2 = NULL;

    fail_if(Nst_llist_pop_llnode(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    node1 = Nst_llist_pop_llnode(&llist);
    fail_if(llist.len != 1);
    fail_if(node1->value != (void *)1);
    node2 = Nst_llist_pop_llnode(&llist);
    fail_if(llist.head != NULL);
    fail_if(llist.tail != NULL);
    fail_if(node2->value != (void *)2);

    if (node1 != NULL)
        Nst_free(node1);
    if (node2 != NULL)
        Nst_free(node2);
    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_empty()
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    Nst_llist_append(&llist, (void *)3, false);
    Nst_llist_empty(&llist, NULL);
    fail_if(llist.len != 0);
    fail_if(llist.head != NULL);
    fail_if(llist.tail != NULL);

    void *alloc_v1 = Nst_raw_malloc(1);
    void *alloc_v2 = Nst_raw_malloc(1);
    void *alloc_v3 = Nst_raw_malloc(1);
    Nst_llist_append(&llist, alloc_v1, true);
    Nst_llist_append(&llist, alloc_v2, true);
    Nst_llist_append(&llist, alloc_v3, true);
    Nst_llist_empty(&llist, Nst_free);
    fail_if(llist.len != 0);
    fail_if(llist.head != NULL);
    fail_if(llist.tail != NULL);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_move_nodes()
{
    ENTER_TEST;
    Nst_LList from, to;
    Nst_llist_init(&from);
    Nst_llist_init(&to);

    Nst_llist_append(&from, (void *)1, false);
    Nst_llist_append(&from, (void *)2, false);
    Nst_llist_append(&from, (void *)3, false);
    Nst_llist_move_nodes(&from, &to);

    fail_if(from.len != 0);
    fail_if(from.head != NULL);
    fail_if(from.tail != NULL);
    fail_if(to.len != 3);
    fail_if(to.head->value != (void *)1);
    fail_if(to.tail->value != (void *)3);
    fail_if(to.head->next->value != (void *)2);

    Nst_llist_empty(&from, NULL);
    Nst_llist_empty(&to, NULL);
    EXIT_TEST;
}

// map.h

TestResult test_map_set()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_get()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_drop()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_copy()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_get_next_idx()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_get_prev_idx()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_set_str()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_get_str()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_drop_str()
{
    return TEST_NOT_IMPL;
}

// mem.h

TestResult test_alloc()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_expand_by()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_expand_to()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_fit()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_append()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_pop()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_at()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_shrink_auto()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_copy()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_expand_by()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_expand_to()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_fit()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_append()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_append_c_str()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_append_str()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_append_char()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_to_string()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_copy()
{
    return TEST_NOT_IMPL;
}

// sequence.h

TestResult test_seq_set()
{
    return TEST_NOT_IMPL;
}

TestResult test_seq_get()
{
    return TEST_NOT_IMPL;
}

TestResult test_seq_copy()
{
    return TEST_NOT_IMPL;
}

TestResult test_vector_append()
{
    return TEST_NOT_IMPL;
}

TestResult test_vector_remove()
{
    return TEST_NOT_IMPL;
}

TestResult test_vector_pop()
{
    return TEST_NOT_IMPL;
}

TestResult test_array_vector_create()
{
    return TEST_NOT_IMPL;
}

TestResult test_array_vector_create_c()
{
    return TEST_NOT_IMPL;
}

// simple_types.h

TestResult test_number_to_u8()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_int()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_i32()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_i64()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_f32()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_f64()
{
    return TEST_NOT_IMPL;
}

TestResult test_obj_to_bool()
{
    return TEST_NOT_IMPL;
}

// str.h

TestResult test_str_copy()
{
    ENTER_TEST;
    Nst_Obj *str = Nst_str_new_c_raw("hèllo\xf0\x9f\x98\x8a", false);
    crit_fail_if(str == NULL);
    Nst_Obj *copy = Nst_str_copy(str);
    crit_fail_if(str == NULL, str);

    fail_if(Nst_str_len(str) != Nst_str_len(copy));
    fail_if(Nst_str_char_len(str) != Nst_str_char_len(copy));
    fail_if(ref_obj_to_bool(Nst_obj_ne(str, copy)));

    Nst_dec_ref(str);
    Nst_dec_ref(copy);
    EXIT_TEST;
}

TestResult test_str_repr()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_get()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_new_c_raw()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_new_c()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_new()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_new_allocated()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_new_len()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_temp()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_next()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_next_obj()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_next_utf32()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_next_utf8()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_parse_int()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_parse_byte()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_parse_real()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_compare()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_find()
{
    return TEST_NOT_IMPL;
}

TestResult test_str_rfind()
{
    return TEST_NOT_IMPL;
}
