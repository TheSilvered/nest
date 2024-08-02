#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "test_nest.h"

#ifndef Nst_WIN
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define ENTER_TEST TestResult test_result__ = TEST_SUCCESS
#define EXIT_TEST return test_result__

const i8 *RED = "\x1b[31m";
const i8 *GREEN = "\x1b[32m";
const i8 *YELLOW = "\x1b[33m";
const i8 *RESET = "\x1b[0m";

void test_init()
{
    if (!Nst_supports_color()) {
        RED = "";
        GREEN = "";
        YELLOW = "";
    }
}

void run_test(Test test, const i8 *test_name)
{
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
            "%sTest '%s' failed.%s\n",
            RED, test_name, RESET);
        break;
    case TEST_NOT_IMPL:
        Nst_printf(
            "%sTest '%s' not implemented.%s\n",
            YELLOW, test_name, RESET);
        break;
    case TEST_CRITICAL_FAILURE:
        Nst_printf(
            "%sTest '%s' failed.%s Stopping execution...\n",
            YELLOW, test_name, RESET);
        Nst_quit();
        exit(1);
    }
}

static void fail(TestResult *result, int line)
{
    Nst_printf("%s  Failure on line %i%s\n", RED, line, RESET);
    *result = TEST_FAILURE;
}

static void crit_fail(TestResult *result, int line)
{
    Nst_printf("%s  Critical failure on line %i%s\n", RED, line, RESET);
    *result = TEST_CRITICAL_FAILURE;
}

#define fail_if(cond) fail_if_((cond), &test_result__, __LINE__)
static bool fail_if_(bool cond, TestResult *result, int line)
{
    if (cond)
        fail(result, line);
    return cond;
}

#define crit_fail_if(cond)                                                    \
    if (crit_fail_if_((cond), &test_result__, __LINE__))                      \
        goto failure
static bool crit_fail_if_(bool cond, TestResult *result, int line)
{
    if (cond)
        crit_fail(result, line);
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

// argv_parser.h

TestResult test_parse_args()
{
    return TEST_NOT_IMPL;
}

#ifdef Nst_WIN

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
    fail_if(str_neq(str, "He"));
    str = Nst_fmt("{s:.0}", 0, NULL, "Hello");
    fail_if(str_neq(str, ""));
    str = Nst_fmt("{s:.2}", 0, NULL, "😀🎺");
    fail_if(str_neq(str, "😀🎺"));
    str = Nst_fmt("{s:.1}", 0, NULL, "😀🎺");
    fail_if(str_neq(str, "😀"));
    str = Nst_fmt("{s:.0}", 0, NULL, "😀🎺");
    fail_if(str_neq(str, ""));
    str = Nst_fmt("{s:.*}", 0, NULL, "Hello", 3);
    fail_if(str_neq(str, "Hel"));

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
    fail_if(str_neq(str, "Hel      "));
    str = Nst_fmt("{s:*.3}", 0, NULL, "Hello", 9);
    fail_if(str_neq(str, "Hel      "));
    str = Nst_fmt("{s:9.*}", 0, NULL, "Hello", 3);
    fail_if(str_neq(str, "Hel      "));
    str = Nst_fmt("{s:*.*}", 0, NULL, "Hello", 9, 3);
    fail_if(str_neq(str, "Hel      "));

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
    str = Nst_fmt("{i:+}", 0, NULL, 10);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{l:+}", 0, NULL, 10l);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{L:+}", 0, NULL, 10ll);
    fail_if(str_neq(str, "+10"));
    str = Nst_fmt("{i: }", 0, NULL, 10);
    fail_if(str_neq(str, " 10"));
    str = Nst_fmt("{l: }", 0, NULL, 10l);
    fail_if(str_neq(str, " 10"));
    str = Nst_fmt("{L: }", 0, NULL, 10ll);
    fail_if(str_neq(str, " 10"));
    str = Nst_fmt("{i}", 0, NULL, -10);
    fail_if(str_neq(str, "-10"));
    str = Nst_fmt("{l}", 0, NULL, -10l);
    fail_if(str_neq(str, "-10"));
    str = Nst_fmt("{L}", 0, NULL, -10ll);
    fail_if(str_neq(str, "-10"));

    str = Nst_fmt("{i:bp}", 0, NULL, 10u);
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
    str = Nst_fmt("{i:o}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:o}", 0, NULL, 0l);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:o}", 0, NULL, 0ll);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l}", 0, NULL, 0l);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L}", 0, NULL, 0ll);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i:x}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:x}", 0, NULL, 0l);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:x}", 0, NULL, 0ll);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{i:X}", 0, NULL, 0);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{l:X}", 0, NULL, 0l);
    fail_if(str_neq(str, "0"));
    str = Nst_fmt("{L:X}", 0, NULL, 0ll);
    fail_if(str_neq(str, "0"));

    str = Nst_fmt("{i:b}", 0, NULL, 123456789);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{l:b}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{L:b}", 0, NULL, 123456789ll);
    fail_if(str_neq(str, "111010110111100110100010101"));
    str = Nst_fmt("{i:o}", 0, NULL, 123456789);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{l:o}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{L:o}", 0, NULL, 123456789ll);
    fail_if(str_neq(str, "726746425"));
    str = Nst_fmt("{i}", 0, NULL, 123456789);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{l}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{L}", 0, NULL, 123456789ll);
    fail_if(str_neq(str, "123456789"));
    str = Nst_fmt("{i:x}", 0, NULL, 123456789);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{l:x}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{L:x}", 0, NULL, 123456789ll);
    fail_if(str_neq(str, "75bcd15"));
    str = Nst_fmt("{i:X}", 0, NULL, 123456789);
    fail_if(str_neq(str, "75BCD15"));
    str = Nst_fmt("{l:X}", 0, NULL, 123456789l);
    fail_if(str_neq(str, "75BCD15"));
    str = Nst_fmt("{L:X}", 0, NULL, 123456789ll);
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

TestResult test_string_copy()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_repr()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_get()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_next_ch()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_new_c_raw()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_new_c()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_new()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_parse_int()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_parse_byte()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_parse_real()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_compare()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_find()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_rfind()
{
    return TEST_NOT_IMPL;
}
