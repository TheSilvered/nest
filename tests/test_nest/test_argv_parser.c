#include "tests.h"

typedef char *argv_t[];

#ifdef __cplusplus
#define ARGS(...) sizeof(argv_t { "nest", ## __VA_ARGS__ }) / sizeof(char *), argv_t { "nest", ## __VA_ARGS__ }
#else
#define ARGS(...) sizeof((argv_t){ "nest", ## __VA_ARGS__ }) / sizeof(char *), (argv_t){ "nest", ## __VA_ARGS__ }
#endif // !__cplusplus

static bool str_starts_with(const char *str1, const char *str2)
{
    if (str1 == NULL)
        return false;
    while (*str1 == *str2 && *str2) {
        str1++;
        str2++;
    }
    return *str2 == 0;
}

TestResult test_cl_args_parse(void)
{
    TEST_ENTER;

    Nst_CLArgs args;

    Nst_cl_args_init(&args, ARGS("file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(str_eq((u8 *)args.filename, "file.nest"));
    test_assert(args.args_start == 2);

    Nst_cl_args_init(&args, ARGS());
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-h"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "USAGE: nest [options]"));
        test_assert(result == 1);
    }

    Nst_cl_args_init(&args, ARGS("-?"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "USAGE: nest [options]"));
        test_assert(result == 1);
    }

    Nst_cl_args_init(&args, ARGS("--help"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "USAGE: nest [options]"));
        test_assert(result == 1);
    }

    Nst_cl_args_init(&args, ARGS("-V"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Using Nest version:"));
        test_assert(result == 1);
    }

    Nst_cl_args_init(&args, ARGS("--version"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Using Nest version:"));
        test_assert(result == 1);
    }

    Nst_cl_args_init(&args, ARGS("-t", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_tokens);

    Nst_cl_args_init(&args, ARGS("-t"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--tokens", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_tokens);

    Nst_cl_args_init(&args, ARGS("--tokens"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-a", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_ast);

    Nst_cl_args_init(&args, ARGS("-a"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--ast", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_ast);

    Nst_cl_args_init(&args, ARGS("--ast"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-i", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_instructions);

    Nst_cl_args_init(&args, ARGS("-i"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--instructions", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_instructions);

    Nst_cl_args_init(&args, ARGS("--instructions"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-b", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_bytecode);

    Nst_cl_args_init(&args, ARGS("-b"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--bytecode", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_bytecode);

    Nst_cl_args_init(&args, ARGS("--bytecode"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-f", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.force_execution);

    Nst_cl_args_init(&args, ARGS("-f"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--force-execution", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.force_execution);

    Nst_cl_args_init(&args, ARGS("--force-execution"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-D", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.no_default);

    Nst_cl_args_init(&args, ARGS("-D"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--no-default", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.no_default);

    Nst_cl_args_init(&args, ARGS("--no-default"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-bf", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_bytecode);
    test_assert(args.force_execution);

    Nst_cl_args_init(&args, ARGS("-bf"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-taibfD", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.print_tokens);
    test_assert(args.print_ast);
    test_assert(args.print_instructions);
    test_assert(args.print_bytecode);
    test_assert(args.force_execution);
    test_assert(args.no_default);

    Nst_cl_args_init(&args, ARGS("-taibfD"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    _Nst_supports_color_override(true);

    Nst_cl_args_init(&args, ARGS("-m", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(!Nst_supports_color());

    Nst_cl_args_init(&args, ARGS("-m"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    _Nst_supports_color_override(true);

    Nst_cl_args_init(&args, ARGS("--monochrome", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(!Nst_supports_color());

    Nst_cl_args_init(&args, ARGS("--monochrome"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "No file provided"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-x", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid option: -x"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-txf", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid option: -x"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--idk", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid option: --idk"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-e=utf8", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.encoding == Nst_EID_UTF8);

    Nst_cl_args_init(&args, ARGS("--encoding=utf8", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.encoding == Nst_EID_UTF8);

    Nst_cl_args_init(&args, ARGS("-e", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid usage of the option: -e"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-e=", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid usage of the option: -e"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-fe=utf8", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid usage of the option: -e"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--encoding", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid usage of the option: --encoding"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--encoding=", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid usage of the option: --encoding"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-e=none", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Unknown encoding none"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--encoding=none", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Unknown encoding none"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-O0", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.opt_level == 0);

    Nst_cl_args_init(&args, ARGS("-O1", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.opt_level == 1);

    Nst_cl_args_init(&args, ARGS("-O2", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.opt_level == 2);

    Nst_cl_args_init(&args, ARGS("-O3", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(args.opt_level == 3);

    Nst_cl_args_init(&args, ARGS("-fO0", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid option: -O"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-O10", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid option: -O10"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-O5", "file.nest"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid option: -O5"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-c", ">>> 1"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(str_eq((u8 *)args.command, ">>> 1"));
    test_assert(args.args_start == 3);

    Nst_cl_args_init(&args, ARGS("-fc", ">>> 1"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid usage of the option: -c"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-fc", ">>> 1"));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid usage of the option: -c"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("--", "-t"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(str_eq((u8 *)args.filename, "-t"));
    test_assert(args.args_start == 3);

    Nst_cl_args_init(&args, ARGS("-a-", ""));
    if (test_capture_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = test_capture_end(NULL);
        test_assert(str_starts_with(msg, "Invalid option: --"));
        test_assert(result == -1);
    }

    Nst_cl_args_init(&args, ARGS("-i", "-O3", "--no-default", "file.nest"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(str_eq((u8 *)args.filename, "file.nest"));
    test_assert(args.args_start == 5);

    Nst_cl_args_init(&args, ARGS("-i", "-O3", "--no-default", "-c", ">>> 1"));
    test_assert(Nst_cl_args_parse(&args) == 0);
    test_assert(str_eq((u8 *)args.command, ">>> 1"));
    test_assert(args.args_start == 6);

    TEST_EXIT;
}

#ifdef Nst_MSVC

typedef wchar_t *wargv_t[];

#ifdef __cplusplus
#define WARGS(...) sizeof(wargv_t { __VA_ARGS__ }) / sizeof(wchar_t *), wargv_t { __VA_ARGS__ }
#else
#define WARGS(...) sizeof((wargv_t){ __VA_ARGS__ }) / sizeof(wchar_t *), (wargv_t){ __VA_ARGS__ }
#endif // !__cplusplus

TestResult test_wargv_to_argv(void)
{
    TEST_ENTER;

    char **argv = NULL;
    test_assert(_Nst_wargv_to_argv(WARGS(L"arg"), &argv));
    test_with(argv != NULL) {
        test_assert(str_eq((u8 *)argv[0], "arg"));
        Nst_free(argv);
    }

    test_assert(_Nst_wargv_to_argv(WARGS(L"\xe8"), &argv));
    test_with(argv != NULL) {
        test_assert(str_eq((u8 *)argv[0], "\xc3\xa8"));
        Nst_free(argv);
    }

    test_assert(_Nst_wargv_to_argv(WARGS(L"\x4e16\x754c"), &argv));
    test_with(argv != NULL) {
        test_assert(str_eq((u8 *)argv[0], "\xe4\xb8\x96\xe7\x95\x8c"));
        Nst_free(argv);
    }

    test_assert(_Nst_wargv_to_argv(WARGS(L"\xd83d\xde00"), &argv));
    test_with(argv != NULL) {
        test_assert(str_eq((u8 *)argv[0], "\xF0\x9F\x98\x80"));
        Nst_free(argv);
    }

    test_assert(_Nst_wargv_to_argv(WARGS(
        L"\xd83d\xdc68\xd83c\xdffc\x200d\xd83d\xdc69\xd83c\xdffe\x200d\xd83d"
        L"\xdc67\xd83c\xdffb\x200d\xd83d\xdc66\xd83c\xdffd"), &argv));
    test_with(argv != NULL) {
        test_assert(str_eq(
            (u8 *)argv[0],
            "\xf0\x9f\x91\xa8\xf0\x9f\x8f\xbc\xe2\x80\x8d\xf0\x9f\x91\xa9\xf0"
            "\x9f\x8f\xbe\xe2\x80\x8d\xf0\x9f\x91\xa7\xf0\x9f\x8f\xbb\xe2\x80"
            "\x8d\xf0\x9f\x91\xa6\xf0\x9f\x8f\xbd"));
        Nst_free(argv);
    }

    TEST_EXIT;
}

#endif
