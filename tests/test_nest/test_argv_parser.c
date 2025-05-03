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
    ENTER_TEST;

    Nst_CLArgs args;

    Nst_cl_args_init(&args, ARGS("file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(str_neq((u8 *)args.filename, "file.nest"));
    fail_if(args.args_start != 2);

    Nst_cl_args_init(&args, ARGS());
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-h"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "USAGE: nest [options]"));
        fail_if(result != 1);
    }

    Nst_cl_args_init(&args, ARGS("-?"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "USAGE: nest [options]"));
        fail_if(result != 1);
    }

    Nst_cl_args_init(&args, ARGS("--help"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "USAGE: nest [options]"));
        fail_if(result != 1);
    }

    Nst_cl_args_init(&args, ARGS("-V"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Using Nest version:"));
        fail_if(result != 1);
    }

    Nst_cl_args_init(&args, ARGS("--version"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Using Nest version:"));
        fail_if(result != 1);
    }

    Nst_cl_args_init(&args, ARGS("-t", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_tokens);

    Nst_cl_args_init(&args, ARGS("-t"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--tokens", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_tokens);

    Nst_cl_args_init(&args, ARGS("--tokens"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-a", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_ast);

    Nst_cl_args_init(&args, ARGS("-a"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--ast", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_ast);

    Nst_cl_args_init(&args, ARGS("--ast"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-i", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_instructions);

    Nst_cl_args_init(&args, ARGS("-i"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--instructions", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_instructions);

    Nst_cl_args_init(&args, ARGS("--instructions"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-b", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_bytecode);

    Nst_cl_args_init(&args, ARGS("-b"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--bytecode", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_bytecode);

    Nst_cl_args_init(&args, ARGS("--bytecode"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-f", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.force_execution);

    Nst_cl_args_init(&args, ARGS("-f"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--force-execution", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.force_execution);

    Nst_cl_args_init(&args, ARGS("--force-execution"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-D", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.no_default);

    Nst_cl_args_init(&args, ARGS("-D"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--no-default", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.no_default);

    Nst_cl_args_init(&args, ARGS("--no-default"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-bf", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_bytecode);
    fail_if(!args.force_execution);

    Nst_cl_args_init(&args, ARGS("-bf"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-taibfD", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(!args.print_tokens);
    fail_if(!args.print_ast);
    fail_if(!args.print_instructions);
    fail_if(!args.print_bytecode);
    fail_if(!args.force_execution);
    fail_if(!args.no_default);

    Nst_cl_args_init(&args, ARGS("-taibfD"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    _Nst_supports_color_override(true);

    Nst_cl_args_init(&args, ARGS("-m", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(Nst_supports_color());

    Nst_cl_args_init(&args, ARGS("-m"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    _Nst_supports_color_override(true);

    Nst_cl_args_init(&args, ARGS("--monochrome", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(Nst_supports_color());

    Nst_cl_args_init(&args, ARGS("--monochrome"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "No file provided"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-x", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid option: -x"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-txf", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid option: -x"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--idk", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid option: --idk"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-e=utf8", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(args.encoding != Nst_EID_UTF8);

    Nst_cl_args_init(&args, ARGS("--encoding=utf8", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(args.encoding != Nst_EID_UTF8);

    Nst_cl_args_init(&args, ARGS("-e", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid usage of the option: -e"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-e=", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid usage of the option: -e"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-fe=utf8", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid usage of the option: -e"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--encoding", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid usage of the option: --encoding"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--encoding=", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid usage of the option: --encoding"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-e=none", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Unknown encoding none"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--encoding=none", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Unknown encoding none"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-O0", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(args.opt_level != 0);

    Nst_cl_args_init(&args, ARGS("-O1", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(args.opt_level != 1);

    Nst_cl_args_init(&args, ARGS("-O2", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(args.opt_level != 2);

    Nst_cl_args_init(&args, ARGS("-O3", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(args.opt_level != 3);

    Nst_cl_args_init(&args, ARGS("-fO0", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid option: -O"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-O10", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid option: -O10"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-O5", "file.nest"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid option: -O5"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-c", ">>> 1"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(str_neq((u8 *)args.command, ">>> 1"));
    fail_if(args.args_start != 3);

    Nst_cl_args_init(&args, ARGS("-fc", ">>> 1"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid usage of the option: -c"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-fc", ">>> 1"));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid usage of the option: -c"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("--", "-t"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(str_neq((u8 *)args.filename, "-t"));
    fail_if(args.args_start != 3);

    Nst_cl_args_init(&args, ARGS("-a-", ""));
    if (capture_output_begin()) {
        i32 result = Nst_cl_args_parse(&args);
        const char *msg = capture_output_end(NULL);
        fail_if(!str_starts_with(msg, "Invalid option: --"));
        fail_if(result != -1);
    }

    Nst_cl_args_init(&args, ARGS("-i", "-O3", "--no-default", "file.nest"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(str_neq((u8 *)args.filename, "file.nest"));
    fail_if(args.args_start != 5);

    Nst_cl_args_init(&args, ARGS("-i", "-O3", "--no-default", "-c", ">>> 1"));
    fail_if(Nst_cl_args_parse(&args) != 0);
    fail_if(str_neq((u8 *)args.command, ">>> 1"));
    fail_if(args.args_start != 6);

    EXIT_TEST;
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
    ENTER_TEST;

    char **argv;
    fail_if(!_Nst_wargv_to_argv(WARGS(L"arg"), &argv));
    fail_if(argv == NULL);
    if (argv != NULL) {
        fail_if(str_neq((u8 *)argv[0], "arg"));
        Nst_free(argv);
    }

    fail_if(!_Nst_wargv_to_argv(WARGS(L"\xe8"), &argv));
    fail_if(argv == NULL);
    if (argv != NULL) {
        fail_if(str_neq((u8 *)argv[0], "\xc3\xa8"));
        Nst_free(argv);
    }

    fail_if(!_Nst_wargv_to_argv(WARGS(L"\x4e16\x754c"), &argv));
    fail_if(argv == NULL);
    if (argv != NULL) {
        fail_if(str_neq((u8 *)argv[0], "\xe4\xb8\x96\xe7\x95\x8c"));
        Nst_free(argv);
    }

    fail_if(!_Nst_wargv_to_argv(WARGS(L"\xd83d\xde00"), &argv));
    fail_if(argv == NULL);
    if (argv != NULL) {
        fail_if(str_neq((u8 *)argv[0], "\xF0\x9F\x98\x80"));
        Nst_free(argv);
    }

    fail_if(!_Nst_wargv_to_argv(WARGS(
        L"\xd83d\xdc68\xd83c\xdffc\x200d\xd83d\xdc69\xd83c\xdffe\x200d\xd83d"
        L"\xdc67\xd83c\xdffb\x200d\xd83d\xdc66\xd83c\xdffd"), &argv));
    fail_if(argv == NULL);
    if (argv != NULL) {
        fail_if(str_neq(
            (u8 *)argv[0],
            "\xf0\x9f\x91\xa8\xf0\x9f\x8f\xbc\xe2\x80\x8d\xf0\x9f\x91\xa9\xf0"
            "\x9f\x8f\xbe\xe2\x80\x8d\xf0\x9f\x91\xa7\xf0\x9f\x8f\xbb\xe2\x80"
            "\x8d\xf0\x9f\x91\xa6\xf0\x9f\x8f\xbd"));
        Nst_free(argv);
    }

    EXIT_TEST;
}

#endif
