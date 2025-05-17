#include "tests.h"

#define SV(str) Nst_sv_new_c(str)

TestResult test_sv_new(void)
{
    ENTER_TEST;

    Nst_StrView sv = Nst_sv_new((u8 *)"Hello!", 6);
    fail_if(sv.len != 6);
    fail_if(str_neq(sv.value, "Hello!"));

    sv = Nst_sv_new(NULL, 0);
    fail_if(sv.len != 0);
    fail_if(sv.value != NULL);

    EXIT_TEST;
}

TestResult test_sv_new_c(void)
{
    ENTER_TEST;

    Nst_StrView sv = Nst_sv_new_c("Hello!\xc3\xa8");
    fail_if(sv.len != 8);
    fail_if(str_neq(sv.value, "Hello!\xc3\xa8"));

    sv = Nst_sv_new_c("ab\0c");
    fail_if(sv.len != 2);
    fail_if(str_neq(sv.value, "ab"));

    sv = Nst_sv_new_c(NULL);
    fail_if(sv.len != 0);
    fail_if(sv.value != NULL);

    EXIT_TEST;
}

TestResult test_sv_from_str(void)
{
    ENTER_TEST;

    Nst_Obj *str = Nst_str_new_c("Hello!\xc3\xa8");
    if (!fail_if(str == NULL)) {
        Nst_StrView sv = Nst_sv_from_str(str);
        fail_if(sv.len != 8);
        fail_if(str_neq(sv.value, "Hello!\xc3\xa8"));
        Nst_dec_ref(str);
    }

    EXIT_TEST;
}

TestResult test_sv_from_sb(void)
{
    ENTER_TEST;

    Nst_StrBuilder sb = { 0 };
    if (!fail_if(!Nst_sb_init(&sb, 10))) {
        Nst_sb_push_c(&sb, "Hello!");
        Nst_StrView sv = Nst_sv_from_sb(&sb);
        fail_if(sv.len != 6);
        fail_if(sv.value != sb.value);
        Nst_sb_destroy(&sb);
    }

    EXIT_TEST;
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
    ENTER_TEST;

    i64 num;
    Nst_StrView rest;

    fail_if(Nst_sv_parse_int(SV("0"), 1, 0, NULL, NULL));
    fail_if(Nst_sv_parse_int(SV("0"), 37, 0, NULL, NULL));
    fail_if(Nst_sv_parse_int(SV(""), 0, 0, NULL, NULL));
    fail_if(Nst_sv_parse_int(SV("   "), 0, 0, NULL, NULL));
    fail_if(Nst_sv_parse_int(SV(" \t\v"), 0, 0, NULL, NULL));
    fail_if(Nst_sv_parse_int(SV("+"), 0, 0, NULL, NULL));
    fail_if(Nst_sv_parse_int(SV("-"), 0, 0, NULL, NULL));
    fail_if(Nst_sv_parse_int(SV(" +"), 0, 0, NULL, NULL));
    fail_if(Nst_sv_parse_int(SV(" -"), 0, 0, NULL, NULL));

    if (!fail_if(!Nst_sv_parse_int(SV("0"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("1"), 0, 0, &num, &rest))) {
        fail_if(num != 1);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("35"), 0, 0, &num, &rest))) {
        fail_if(num != 35);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("9223372036854775807"), 0, 0, &num, &rest))) {
        fail_if(num != 9223372036854775807);
        fail_if(rest.len != 0);
    }
    fail_if(Nst_sv_parse_int(SV("9223372036854775808"), 0, 0, &num, &rest));

    if (!fail_if(!Nst_sv_parse_int(SV("+0"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("-0"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("+1"), 0, 0, &num, &rest))) {
        fail_if(num != 1);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("-1"), 0, 0, &num, &rest))) {
        fail_if(num != -1);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("+35"), 0, 0, &num, &rest))) {
        fail_if(num != 35);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("-35"), 0, 0, &num, &rest))) {
        fail_if(num != -35);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("+9223372036854775807"), 0, 0, &num, &rest))) {
        fail_if(num != 9223372036854775807);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("-9223372036854775807"), 0, 0, &num, &rest))) {
        fail_if(num != -9223372036854775807);
        fail_if(rest.len != 0);
    }
    if (!fail_if(!Nst_sv_parse_int(SV("-9223372036854775808"), 0, 0, &num, &rest))) {
        fail_if(num != -9223372036854775807 - 1);
        fail_if(rest.len != 0);
    }
    fail_if(Nst_sv_parse_int(SV("-9223372036854775809"), 0, 0, &num, &rest));

    if (!fail_if(!Nst_sv_parse_int(SV("0smth"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 4);
        fail_if(str_neq(rest.value, "smth"));
    }
    if (!fail_if(!Nst_sv_parse_int(SV("14 other"), 0, 0, &num, &rest))) {
        fail_if(num != 14);
        fail_if(rest.len != 5);
        fail_if(str_neq(rest.value, "other"));
    }
    if (!fail_if(!Nst_sv_parse_int(SV("0b"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 1);
        fail_if(str_neq(rest.value, "b"));
    }
    if (!fail_if(!Nst_sv_parse_int(SV("0B"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 1);
        fail_if(str_neq(rest.value, "B"));
    }
    if (!fail_if(!Nst_sv_parse_int(SV("0o"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 1);
        fail_if(str_neq(rest.value, "o"));
    }
    if (!fail_if(!Nst_sv_parse_int(SV("0O"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 1);
        fail_if(str_neq(rest.value, "O"));
    }
    if (!fail_if(!Nst_sv_parse_int(SV("0x"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 1);
        fail_if(str_neq(rest.value, "x"));
    }
    if (!fail_if(!Nst_sv_parse_int(SV("0X"), 0, 0, &num, &rest))) {
        fail_if(num != 0);
        fail_if(rest.len != 1);
        fail_if(str_neq(rest.value, "X"));
    }

    EXIT_TEST;
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
