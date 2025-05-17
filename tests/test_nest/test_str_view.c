#include "tests.h"

#define SV(str) Nst_sv_new_c(str)

TestResult test_sv_new(void)
{
    TEST_ENTER;

    Nst_StrView sv = Nst_sv_new((u8 *)"Hello!", 6);
    test_assert(sv.len == 6);
    test_assert(str_eq(sv.value, "Hello!"));

    sv = Nst_sv_new(NULL, 0);
    test_assert(sv.len == 0);
    test_assert(sv.value == NULL);

    TEST_EXIT;
}

TestResult test_sv_new_c(void)
{
    TEST_ENTER;

    Nst_StrView sv = Nst_sv_new_c("Hello!\xc3\xa8");
    test_assert(sv.len == 8);
    test_assert(str_eq(sv.value, "Hello!\xc3\xa8"));

    sv = Nst_sv_new_c("ab\0c");
    test_assert(sv.len == 2);
    test_assert(str_eq(sv.value, "ab"));

    sv = Nst_sv_new_c(NULL);
    test_assert(sv.len == 0);
    test_assert(sv.value == NULL);

    TEST_EXIT;
}

TestResult test_sv_from_str(void)
{
    TEST_ENTER;

    Nst_Obj *str = Nst_str_new_c("Hello!\xc3\xa8");
    test_with(str != NULL) {
        Nst_StrView sv = Nst_sv_from_str(str);
        test_assert(sv.len == 8);
        test_assert(str_eq(sv.value, "Hello!\xc3\xa8"));
        Nst_dec_ref(str);
    }

    TEST_EXIT;
}

TestResult test_sv_from_sb(void)
{
    TEST_ENTER;

    Nst_StrBuilder sb = { 0 };
    test_with(Nst_sb_init(&sb, 10)) {
        Nst_sb_push_c(&sb, "Hello!");
        Nst_StrView sv = Nst_sv_from_sb(&sb);
        test_assert(sv.len == 6);
        test_assert(sv.value == sb.value);
        Nst_sb_destroy(&sb);
    }

    TEST_EXIT;
}

TestResult test_sv_next(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_sv_prev(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_sv_parse_int(void)
{
    TEST_ENTER;

    i64 num;
    Nst_StrView rest;

    test_assert(!Nst_sv_parse_int(SV("0"), 1, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0"), 37, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV(""), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("   "), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV(" \t\v"), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("+"), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("-"), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV(" +"), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV(" -"), 0, 0, NULL, NULL));

    test_with(Nst_sv_parse_int(SV("0"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("1"), 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("35"), 0, 0, &num, &rest)) {
        test_assert(num == 35);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("9223372036854775807"), 0, 0, &num, &rest)) {
        test_assert(num == 9223372036854775807);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_int(SV("9223372036854775808"), 0, 0, &num, &rest));

    test_with(Nst_sv_parse_int(SV("+0"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-0"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("+1"), 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-1"), 0, 0, &num, &rest)) {
        test_assert(num == -1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("+35"), 0, 0, &num, &rest)) {
        test_assert(num == 35);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-35"), 0, 0, &num, &rest)) {
        test_assert(num == -35);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("+9223372036854775807"), 0, 0, &num, &rest)) {
        test_assert(num == 9223372036854775807);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-9223372036854775807"), 0, 0, &num, &rest)) {
        test_assert(num == -9223372036854775807);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-9223372036854775808"), 0, 0, &num, &rest)) {
        test_assert(num == -9223372036854775807 - 1);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_int(SV("-9223372036854775809"), 0, 0, &num, &rest));

    test_with(Nst_sv_parse_int(SV("0smth"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 4);
        test_assert(str_eq(rest.value, "smth"));
    }
    test_with(Nst_sv_parse_int(SV("14 other"), 0, 0, &num, &rest)) {
        test_assert(num == 14);
        test_assert(rest.len == 5);
        test_assert(str_eq(rest.value, "other"));
    }
    test_with(Nst_sv_parse_int(SV("0b"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "b"));
    }
    test_with(Nst_sv_parse_int(SV("0B"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "B"));
    }
    test_with(Nst_sv_parse_int(SV("0o"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "o"));
    }
    test_with(Nst_sv_parse_int(SV("0O"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "O"));
    }
    test_with(Nst_sv_parse_int(SV("0x"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "x"));
    }
    test_with(Nst_sv_parse_int(SV("0X"), 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "X"));
    }

    TEST_EXIT;
}

TestResult test_sv_parse_byte(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_sv_parse_real(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_sv_compare(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_sv_lfind(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_sv_rfind(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_sv_ltok(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_sv_rtok(void)
{
    return TEST_NOT_IMPL;
}
