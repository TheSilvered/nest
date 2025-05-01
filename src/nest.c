#include "nest.h"

#ifdef Nst_MSVC

#include <windows.h>

int wmain(int argc, wchar_t **wargv)
{
    _Nst_console_mode_init();

    char **argv;

    if (!_Nst_wargv_to_argv(argc, wargv, &argv))
        return -1;

#else

#include <string.h>

int main(int argc, char **argv)
{

#endif

#ifdef _DEBUG
    puts("**USING DEBUG BUILD - " Nst_VERSION "**");
    for (usize i = 0, n = strlen(Nst_VERSION) + 24; i < n; i++)
        putc('-', stdout);
    putc('\n', stdout);
    fflush(stdout);
#endif

    Nst_CLArgs cl_args;
    Nst_cl_args_init(&cl_args, argc, argv);
    i32 parse_result = Nst_cl_args_parse(&cl_args);
    if (parse_result == -1)
        return -1;
    else if (parse_result == 1)
        return 0;

    if (!Nst_init()) {
        fprintf(stderr, "Failed to initialize Nest.");
        return -1;
    }

    Nst_Program prog;
    Nst_ExecutionKind ek = Nst_prog_init(&prog, cl_args);
    i32 result;

    if (ek == Nst_EK_ERROR) {
        Nst_error_print();
        Nst_error_clear();
        result = 1;
    } else if (ek == Nst_EK_INFO)
        result = 0;
    else
        result = Nst_run(&prog);

    if (Nst_error_occurred())
        Nst_error_print();

    Nst_prog_destroy(&prog);
    Nst_quit();

#ifdef Nst_MSVC
    Nst_free(argv);
#endif // !Nst_MSVC

    Nst_log_alloc_count();
    Nst_log_alloc_info();

    return result;
}
