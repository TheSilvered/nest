#include "tests.h"

TestResult test_str_new_c(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_new(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_new_allocated(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_new_len(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_from_sv(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_from_sb(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_copy(void)
{
    TEST_ENTER;

    Nst_Obj *str = Nst_str_new_c("h\xc3\xa8llo\xf0\x9f\x98\x8a");
    test_assert_or_exit(str != NULL, {});
    Nst_Obj *copy = Nst_str_copy(str);
    test_assert_or_exit(str != NULL, Nst_dec_ref(str));

    test_assert(Nst_str_len(str) == Nst_str_len(copy));
    test_assert(Nst_str_char_len(str) == Nst_str_char_len(copy));
    test_assert(ref_obj_to_bool(Nst_obj_eq(str, copy)));

    Nst_dec_ref(str);
    Nst_dec_ref(copy);

    TEST_EXIT;
}

TestResult test_str_repr(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_get(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_get_obj(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_next(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_next_obj(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_next_utf32(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_next_utf8(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_parse_int(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_parse_byte(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_parse_real(void)
{
    return TEST_NOT_IMPL;
}

TestResult test_str_compare(void)
{
    return TEST_NOT_IMPL;
}
