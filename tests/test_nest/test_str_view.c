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
    TEST_ENTER;

    Nst_StrView sv = Nst_sv_new_c("aè😊あ");
    u32 ch;
    isize i = Nst_sv_next(sv, -1, &ch);
    test_assert(ch == 'a');
    test_assert(i >= 0);
    i = Nst_sv_next(sv, i, &ch);
    test_assert(ch == 0xe8);
    test_assert(i >= 0);
    i = Nst_sv_next(sv, i, &ch);
    test_assert(ch == 0x1F60A);
    test_assert(i >= 0);
    i = Nst_sv_next(sv, i, &ch);
    test_assert(ch == 0x3042);
    test_assert(i >= 0);
    i = Nst_sv_next(sv, i, &ch);
    test_assert(i < 0);

    TEST_EXIT;
}

TestResult test_sv_prev(void)
{
    TEST_ENTER;

    Nst_StrView sv = Nst_sv_new_c("aè😊あ");
    u32 ch;
    isize i = Nst_sv_prev(sv, -1, &ch);
    test_assert(ch == 0x3042);
    test_assert(i >= 0);
    i = Nst_sv_prev(sv, i, &ch);
    test_assert(ch == 0x1F60A);
    test_assert(i >= 0);
    i = Nst_sv_prev(sv, i, &ch);
    test_assert(ch == 0xe8);
    test_assert(i >= 0);
    i = Nst_sv_prev(sv, i, &ch);
    test_assert(ch == 'a');
    test_assert(i >= 0);
    i = Nst_sv_prev(sv, i, &ch);
    test_assert(i < 0);

    TEST_EXIT;
}

TestResult test_sv_parse_int(void)
{
    TEST_ENTER;

    i64 num;
    Nst_StrView rest;

    // Only whitespace
    test_assert(!Nst_sv_parse_int(SV(""), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("   "), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV(" \t\v"), 0, 0, 0, NULL, NULL));

    // Only the sign
    test_assert(!Nst_sv_parse_int(SV("+"), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("-"), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV(" +"), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV(" -"), 0, 0, 0, NULL, NULL));

    // Only a 0
    test_with(Nst_sv_parse_int(SV("0"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("  0\t"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "\t"));
    }
    test_with(Nst_sv_parse_int(SV("+0"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-0"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("  -0 \t"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, " \t"));
    }
    test_with(Nst_sv_parse_int(SV("0smth"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 4);
        test_assert(str_eq(rest.value, "smth"));
    }
    test_with(Nst_sv_parse_int(SV("\v +0  smth"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 6);
        test_assert(str_eq(rest.value, "  smth"));
    }

    // Possible prefixes
    test_with(Nst_sv_parse_int(SV("0b"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "b"));
    }
    test_with(Nst_sv_parse_int(SV(" \t+0b  "), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 3);
        test_assert(str_eq(rest.value, "b  "));
    }
    test_with(Nst_sv_parse_int(SV("0B"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "B"));
    }
    test_with(Nst_sv_parse_int(SV(" \t -0B\v"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, "B\v"));
    }
    test_with(Nst_sv_parse_int(SV("0o  "), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 3);
        test_assert(str_eq(rest.value, "o  "));
    }
    test_with(Nst_sv_parse_int(SV("0O"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "O"));
    }
    test_with(Nst_sv_parse_int(SV("0x"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "x"));
    }
    test_with(Nst_sv_parse_int(SV("0X"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "X"));
    }

    // Regular numbers
    test_with(Nst_sv_parse_int(SV("1"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("35"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 35);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("001"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("0035"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 35);
        test_assert(rest.len == 0);
    }

    // With a sign
    test_with(Nst_sv_parse_int(SV("+1"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-1"), 0, 0, 0, &num, &rest)) {
        test_assert(num == -1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("+35"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 35);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-35"), 0, 0, 0, &num, &rest)) {
        test_assert(num == -35);
        test_assert(rest.len == 0);
    }

    // Check limits (overflow not allowed)
    test_with(Nst_sv_parse_int(SV("9223372036854775807"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 9223372036854775807);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_int(SV("9223372036854775808"), 0, 0, 0, &num, &rest));

    test_with(Nst_sv_parse_int(SV("+9223372036854775807"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 9223372036854775807);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-9223372036854775807"), 0, 0, 0, &num, &rest)) {
        test_assert(num == -9223372036854775807);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("-9223372036854775808"), 0, 0, 0, &num, &rest)) {
        test_assert(num == -9223372036854775807 - 1);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_int(SV("-9223372036854775809"), 0, 0, 0, &num, &rest));

    // Remaining part (no full match required)
    test_with(Nst_sv_parse_int(SV("14 other"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 14);
        test_assert(rest.len == 6);
        test_assert(str_eq(rest.value, " other"));
    }

    // Invalid bases
    test_assert(!Nst_sv_parse_int(SV("0"), 1, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0"), 37, 0, 0, NULL, NULL));

    // Binary
    test_with(Nst_sv_parse_int(SV("10"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 2);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("11012"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 13);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "2"));
    }
    test_with(Nst_sv_parse_int(SV("0b"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "b"));
    }
    test_with(Nst_sv_parse_int(SV("0B"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "B"));
    }
    test_with(Nst_sv_parse_int(SV("0b111"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 7);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("0B123"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, "23"));
    }
    test_with(Nst_sv_parse_int(SV("0b111"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 7);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("0B123"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, "23"));
    }

    // Octal
    test_with(Nst_sv_parse_int(SV("10"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 8);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("789"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 7);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, "89"));
    }
    test_with(Nst_sv_parse_int(SV("0o"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "o"));
    }
    test_with(Nst_sv_parse_int(SV("0O"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "O"));
    }
    test_with(Nst_sv_parse_int(SV("0o178"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 15);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "8"));
    }
    test_with(Nst_sv_parse_int(SV("0O123"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 83);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("0o178"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 15);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "8"));
    }
    test_with(Nst_sv_parse_int(SV("0O123"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 83);
        test_assert(rest.len == 0);
    }

    // Hexadecimal
    test_with(Nst_sv_parse_int(SV("10"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 16);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("abj"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 171);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "j"));
    }
    test_with(Nst_sv_parse_int(SV("0x"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "x"));
    }
    test_with(Nst_sv_parse_int(SV("0X"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "X"));
    }
    test_with(Nst_sv_parse_int(SV("0x123efg"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 74735);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "g"));
    }
    test_with(Nst_sv_parse_int(SV("0X12"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 18);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("0x123efg"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 74735);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "g"));
    }
    test_with(Nst_sv_parse_int(SV("0X12"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 18);
        test_assert(rest.len == 0);
    }

    // Other bases
    test_with(Nst_sv_parse_int(SV("10Z"), 30, 0, 0, &num, &rest)) {
        test_assert(num == 30);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "Z"));
    }
    test_with(Nst_sv_parse_int(SV("3i"), 20, 0, 0, &num, &rest)) {
        test_assert(num == 78);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_int(SV("infinity"), 36, 0, 0, &num, &rest)) {
        test_assert(num == 1461559270678);
        test_assert(rest.len == 0);
    }

    // Disabled separator
    test_with(Nst_sv_parse_int(SV("12_345"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 12);
        test_assert(rest.len == 4);
        test_assert(str_eq(rest.value, "_345"));
    }

    // Normal separator
    test_with(Nst_sv_parse_int(SV("12_345"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 12345);
        test_assert(rest.len == 0);
    }

    test_with(Nst_sv_parse_int(SV("1_2_3_4_5"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 12345);
        test_assert(rest.len == 0);
    }

    test_with(Nst_sv_parse_int(SV("1_2_3__4_5"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 123);
        test_assert(rest.len == 5);
        test_assert(str_eq(rest.value, "__4_5"));
    }

    // Unicode separator (U+1F60A = 😊)
    test_with(Nst_sv_parse_int(SV("1😊2😊3😊4😊5"), 0, 0, 0x1F60A, &num, &rest)) {
        test_assert(num == 12345);
        test_assert(rest.len == 0);
    }

    // Trailing separator
    test_with(Nst_sv_parse_int(SV("12_345_"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 12345);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "_"));
    }

    // Separator after prefix
    test_with(Nst_sv_parse_int(SV("0x_10"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 4);
        test_assert(str_eq(rest.value, "x_10"));
    }

    // Full match
    test_assert(Nst_sv_parse_int(SV("  0\t"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_int(SV("+0"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_int(SV("-0"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_int(SV("  -0 \t"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0smth"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("\v +0  smth"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0b"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV(" \t+0b  "), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0B"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV(" \t -0B\v"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0o"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0O"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0x"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0X"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_int(SV("1"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_int(SV("1  "), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_int(SV("35"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_int(SV(" 001  "), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_int(SV("  0035 "), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("14 other"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_int(SV("0b"), 2, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_int(SV("0b111"), 2, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));

    TEST_EXIT;
}

TestResult test_sv_parse_byte(void)
{
    TEST_ENTER;

    u8 num;
    Nst_StrView rest;

    // Only whitespace
    test_assert(!Nst_sv_parse_byte(SV(""), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("   "), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV(" \t\v"), 0, 0, 0, NULL, NULL));

    // Only the sign
    test_assert(!Nst_sv_parse_byte(SV("+"), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("-"), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV(" +"), 0, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV(" -"), 0, 0, 0, NULL, NULL));

    // Only a 0
    test_with(Nst_sv_parse_byte(SV("0"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("  0\t"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "\t"));
    }
    test_with(Nst_sv_parse_byte(SV("+0"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("-0"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("  -0 \t"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, " \t"));
    }
    test_with(Nst_sv_parse_byte(SV("0smth"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 4);
        test_assert(str_eq(rest.value, "smth"));
    }
    test_with(Nst_sv_parse_byte(SV("\v +0  smth"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 6);
        test_assert(str_eq(rest.value, "  smth"));
    }

    // Possible prefixes
    test_with(Nst_sv_parse_byte(SV("0b"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV(" \t+0b  "), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, "  "));
    }
    test_with(Nst_sv_parse_byte(SV("0B"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV(" \t -0B\v"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "\v"));
    }
    test_with(Nst_sv_parse_byte(SV("0o  "), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 3);
        test_assert(str_eq(rest.value, "o  "));
    }
    test_with(Nst_sv_parse_byte(SV("0O"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "O"));
    }
    test_with(Nst_sv_parse_byte(SV("0x"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "x"));
    }
    test_with(Nst_sv_parse_byte(SV("0X"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "X"));
    }

    // Regular numbers
    test_with(Nst_sv_parse_byte(SV("1"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("35"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 35);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("001"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("0035"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 35);
        test_assert(rest.len == 0);
    }

    // With a sign
    test_with(Nst_sv_parse_byte(SV("+1"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_byte(SV("-1"), 0, 0, 0, NULL, NULL));
    test_with(Nst_sv_parse_byte(SV("+35"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 35);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_byte(SV("-35"), 0, 0, 0, NULL, NULL));

    // Check limits (overflow not allowed)
    test_with(Nst_sv_parse_byte(SV("255"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 255);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_byte(SV("256"), 0, 0, 0, &num, &rest));

    test_with(Nst_sv_parse_byte(SV("+255"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 255);
        test_assert(rest.len == 0);
    }

    // Remaining part (no full match required)
    test_with(Nst_sv_parse_byte(SV("14 other"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 14);
        test_assert(rest.len == 6);
        test_assert(str_eq(rest.value, " other"));
    }

    // Invalid bases
    test_assert(!Nst_sv_parse_byte(SV("0"), 1, 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0"), 37, 0, 0, NULL, NULL));

    // Binary
    test_with(Nst_sv_parse_byte(SV("10"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 2);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("11012"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 13);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "2"));
    }
    test_with(Nst_sv_parse_byte(SV("0b"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("0B"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("0b111"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 7);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("0B123"), 2, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, "23"));
    }
    test_with(Nst_sv_parse_byte(SV("0b111"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 7);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("0B123"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 1);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, "23"));
    }

    // Octal
    test_with(Nst_sv_parse_byte(SV("10"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 8);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("789"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 7);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, "89"));
    }
    test_with(Nst_sv_parse_byte(SV("0o"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "o"));
    }
    test_with(Nst_sv_parse_byte(SV("0O"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "O"));
    }
    test_with(Nst_sv_parse_byte(SV("0o178"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 15);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "8"));
    }
    test_with(Nst_sv_parse_byte(SV("0O123"), 8, 0, 0, &num, &rest)) {
        test_assert(num == 83);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("0o178"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 15);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "8"));
    }
    test_with(Nst_sv_parse_byte(SV("0O123"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 83);
        test_assert(rest.len == 0);
    }

    // Hexadecimal
    test_with(Nst_sv_parse_byte(SV("10"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 16);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("abj"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 171);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "j"));
    }
    test_with(Nst_sv_parse_byte(SV("0x"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "x"));
    }
    test_with(Nst_sv_parse_byte(SV("0X"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "X"));
    }
    test_with(Nst_sv_parse_byte(SV("0xefg"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 239);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "g"));
    }
    test_with(Nst_sv_parse_byte(SV("0X12"), 16, 0, 0, &num, &rest)) {
        test_assert(num == 18);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("0xefg"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 239);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "g"));
    }
    test_with(Nst_sv_parse_byte(SV("0X12"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 18);
        test_assert(rest.len == 0);
    }

    // Other bases
    test_with(Nst_sv_parse_byte(SV("10Z"), 30, 0, 0, &num, &rest)) {
        test_assert(num == 30);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "Z"));
    }
    test_with(Nst_sv_parse_byte(SV("3i"), 20, 0, 0, &num, &rest)) {
        test_assert(num == 78);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_byte(SV("1z"), 36, 0, 0, &num, &rest)) {
        test_assert(num == 71);
        test_assert(rest.len == 0);
    }

    // Disabled separator
    test_with(Nst_sv_parse_byte(SV("12_345"), 0, 0, 0, &num, &rest)) {
        test_assert(num == 12);
        test_assert(rest.len == 4);
        test_assert(str_eq(rest.value, "_345"));
    }

    // Normal separator
    test_with(Nst_sv_parse_byte(SV("1_23"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 123);
        test_assert(rest.len == 0);
    }

    test_with(Nst_sv_parse_byte(SV("1_2_3"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 123);
        test_assert(rest.len == 0);
    }

    test_with(Nst_sv_parse_byte(SV("1_2_3__4_5"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 123);
        test_assert(rest.len == 5);
        test_assert(str_eq(rest.value, "__4_5"));
    }

    // Unicode separator (U+1F60A = 😊)
    test_with(Nst_sv_parse_byte(SV("1😊2😊3"), 0, 0, 0x1F60A, &num, &rest)) {
        test_assert(num == 123);
        test_assert(rest.len == 0);
    }

    // Trailing separator
    test_with(Nst_sv_parse_byte(SV("1_23_"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 123);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "_"));
    }

    // Separator after prefix
    test_with(Nst_sv_parse_byte(SV("0x_10"), 0, 0, '_', &num, &rest)) {
        test_assert(num == 0);
        test_assert(rest.len == 4);
        test_assert(str_eq(rest.value, "x_10"));
    }

    // Full match
    test_assert(Nst_sv_parse_byte(SV("  0\t"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("+0"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("-0"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("  -0 \t"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0smth"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("\v +0  smth"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0o"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0O"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0x"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0X"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("1"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("1  "), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("35"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV(" 001  "), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("  0035 "), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("14 other"), 0, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("0b111"), 2, Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));

    // Required suffix
    test_assert(Nst_sv_parse_byte(SV("0"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("j"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("0b"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("10"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("10b"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0ob"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("0o7b"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("0o7b"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0h"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("0h0"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0x0"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_byte(SV("0x"), 0, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));
    test_assert(Nst_sv_parse_byte(SV("0"), 13, Nst_SVFLAG_CHAR_BYTE, 0, NULL, NULL));

    TEST_EXIT;
}

TestResult test_sv_parse_real(void)
{
    TEST_ENTER;

    f64 num;
    Nst_StrView rest;

    test_assert(!Nst_sv_parse_real(SV(""), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_real(SV(" "), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_real(SV("+"), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_real(SV("-"), 0, 0, NULL, NULL));
    test_assert(!Nst_sv_parse_real(SV("."), 0, 0, NULL, NULL));

    test_with(Nst_sv_parse_real(SV("1"), 0, 0, &num, &rest)) {
        test_assert(num == 1.0);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_real(SV("1"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest));

    test_with(Nst_sv_parse_real(SV("1.0"), 0, 0, &num, &rest)) {
        test_assert(num == 1.0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1.0"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest)) {
        test_assert(num == 1.0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1.00"), 0, 0, &num, &rest)) {
        test_assert(num == 1.0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1.00"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest)) {
        test_assert(num == 1.0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1.2"), 0, 0, &num, &rest)) {
        test_assert(num == 1.2);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1.2"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest)) {
        test_assert(num == 1.2);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV(".1"), 0, 0, &num, &rest)) {
        test_assert(num == 0.1);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_real(SV(".1"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest));
    test_with(Nst_sv_parse_real(SV("0.1"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest)) {
        test_assert(num == 0.1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1e2"), 0, 0, &num, &rest)) {
        test_assert(num == 100.0);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_real(SV("1e2"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest));
    test_with(Nst_sv_parse_real(SV("1.0e2"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest)) {
        test_assert(num == 100.0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1e+2"), 0, 0, &num, &rest)) {
        test_assert(num == 100.0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1.0e+2"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest)) {
        test_assert(num == 100.0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1e+2"), 0, 0, &num, &rest)) {
        test_assert(num == 100.0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1.0e-2"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest)) {
        test_assert(num == 0.01);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV(".1e2"), 0, 0, &num, &rest)) {
        test_assert(num == 10.0);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_real(SV(".1e2"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest));
    test_with(Nst_sv_parse_real(SV("0.1e2"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest)) {
        test_assert(num == 10.0);
        test_assert(rest.len == 0);
    }
    test_with(!Nst_sv_parse_real(SV("e1"), 0, 0, &num, &rest)) {
        test_assert(num == 0.0);
        test_assert(rest.len == 2);
        test_assert(str_eq(rest.value, "e1"));
    }
    test_with(!Nst_sv_parse_real(SV(".e1"), 0, 0, &num, &rest)) {
        test_assert(num == 0.0);
        test_assert(rest.len == 3);
        test_assert(str_eq(rest.value, ".e1"));
    }
    test_with(Nst_sv_parse_real(SV("1.e2"), 0, 0, &num, &rest)) {
        test_assert(num == 100.0);
        test_assert(rest.len == 0);
    }
    test_assert(!Nst_sv_parse_real(SV("1.e2"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest));
    test_with(Nst_sv_parse_real(SV("1.2e3"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest)) {
        test_assert(num == 1200.0);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1_23.1"), Nst_SVFLAG_STRICT_REAL, '_', &num, &rest)) {
        test_assert(num == 123.1);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1__23.1"), 0, '_', &num, &rest)) {
        test_assert(num == 1.0);
        test_assert(rest.len == 6);
        test_assert(str_eq(rest.value, "__23.1"));
    }
    test_assert(!Nst_sv_parse_real(SV("1__23.1"), Nst_SVFLAG_STRICT_REAL, 0, &num, &rest));
    test_with(Nst_sv_parse_real(SV("1.0e2_3"), Nst_SVFLAG_STRICT_REAL, '_', &num, &rest)) {
        test_assert(num == 1e23);
        test_assert(rest.len == 0);
    }
    test_with(Nst_sv_parse_real(SV("1.0e2_3_"), Nst_SVFLAG_STRICT_REAL, '_', &num, &rest)) {
        test_assert(num == 1e23);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "_"));
    }
    test_with(Nst_sv_parse_real(SV("1_2_"), 0, '_', &num, &rest)) {
        test_assert(num == 12.0);
        test_assert(rest.len == 1);
        test_assert(str_eq(rest.value, "_"));
    }

    // A string long enough to cause a memory allocation in the function to
    // remove the separators
    const char *long_str = "1_2.0000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000";

    test_with(Nst_sv_parse_real(SV(long_str), 0, '_', &num, &rest)) {
        test_assert(num == 12.0);
        test_assert(rest.len == 0);
    }

    test_with(Nst_sv_parse_real(SV("1"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1.0"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1.00"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1.2"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV(".1"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1e2"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1.0e2"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1e+2"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1.0e+2"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1e-2"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1.0e-2"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV(".1e2"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("0.1e2"), Nst_SVFLAG_FULL_MATCH, 0, NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1_23.1"), Nst_SVFLAG_FULL_MATCH, '_', NULL, NULL));
    test_with(!Nst_sv_parse_real(SV("1__23.1"), Nst_SVFLAG_FULL_MATCH, '_', NULL, NULL));
    test_with(Nst_sv_parse_real(SV("1.0e2_3"), Nst_SVFLAG_FULL_MATCH, '_', NULL, NULL));
    test_with(!Nst_sv_parse_real(SV("1.0e2_3_"), Nst_SVFLAG_FULL_MATCH, '_', NULL, NULL));

    TEST_EXIT;
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
