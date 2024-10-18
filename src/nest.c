#include "nest.h"

#ifdef Nst_MSVC

#include <windows.h>

int wmain(int argc, wchar_t **wargv)
{
    _Nst_console_mode_init();

    i8 **argv;

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
    i32 parse_result = _Nst_cl_args_parse(&cl_args);
    if (parse_result == -1)
        return -1;
    else if (parse_result == 1)
        return 0;

    if (!Nst_init(&cl_args)) {
        fprintf(stderr, "Failed to initialize Nest.");
        return -1;
    }

    Nst_ExecutionState es;
    Nst_SourceText src;
    i32 result = Nst_execute(cl_args, &es, &src);

    if (Nst_error_occurred())
        Nst_print_traceback(Nst_error_get());

    Nst_es_destroy(&es);
    Nst_source_text_destroy(&src);
    Nst_quit();

#ifdef Nst_MSVC
    Nst_free(argv);
#endif // !Nst_MSVC

#ifdef Nst_COUNT_ALLOC
    Nst_log_alloc_count();
#endif

    return result;
}
